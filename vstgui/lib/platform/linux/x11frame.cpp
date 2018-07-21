// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "x11frame.h"
#include "../../cbuttonstate.h"
#include "../../crect.h"
#include "../../dragging.h"
#include "../../vstkeycode.h"
#include "../../optional.h"
#include "../iplatformopenglview.h"
#include "../iplatformviewlayer.h"
#include "../iplatformtextedit.h"
#include "../iplatformoptionmenu.h"
#include "../common/fileresourceinputstream.h"
#include "cairobitmap.h"
#include "cairocontext.h"
#include "x11platform.h"
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_util.h>
#include <cairo/cairo-xcb.h>

#ifdef None
#	undef None
#endif

//------------------------------------------------------------------------
namespace VSTGUI {
namespace X11 {

//------------------------------------------------------------------------
static std::string getAtomName (xcb_atom_t atom)
{
	std::string name;
	auto xcb = RunLoop::instance ().getXcbConnection ();
	auto cookie = xcb_get_atom_name (xcb, atom);
	if (auto reply = xcb_get_atom_name_reply (xcb, cookie, nullptr))
	{
		auto length = xcb_get_atom_name_name_length (reply);
		name = xcb_get_atom_name_name (reply);
		free (reply);
	}
	return name;
}

//------------------------------------------------------------------------
static uint32_t translateMouseButtons (xcb_button_t value)
{
	switch (value)
	{
		case 1:
			return kLButton;
		case 2:
			return kMButton;
		case 3:
			return kRButton;
		case 4:
			return kButton4;
		case 5:
			return kButton5;
	}
	return 0;
}

//------------------------------------------------------------------------
static uint32_t translateMouseButtons (int state)
{
	uint32_t buttons = 0;
	if (state & XCB_BUTTON_MASK_1)
		buttons |= kLButton;
	if (state & XCB_BUTTON_MASK_2)
		buttons |= kRButton;
	if (state & XCB_BUTTON_MASK_3)
		buttons |= kMButton;
	if (state & XCB_BUTTON_MASK_4)
		buttons |= kButton4;
	if (state & XCB_BUTTON_MASK_5)
		buttons |= kButton5;
	return buttons;
}

//------------------------------------------------------------------------
static uint32_t translateModifiers (int state)
{
	uint32_t buttons = 0;
	if (state & XCB_MOD_MASK_CONTROL)
		buttons |= kControl;
	if (state & XCB_MOD_MASK_SHIFT)
		buttons |= kShift;
	if (state & (XCB_MOD_MASK_1 | XCB_MOD_MASK_5))
		buttons |= kAlt;
	return buttons;
}

//------------------------------------------------------------------------
static xcb_visualtype_t* getVisualType (const xcb_screen_t* screen)
{
	auto depth_iter = xcb_screen_allowed_depths_iterator (screen);
	for (; depth_iter.rem; xcb_depth_next (&depth_iter))
	{
		xcb_visualtype_iterator_t visual_iter;

		visual_iter = xcb_depth_visuals_iterator (depth_iter.data);
		for (; visual_iter.rem; xcb_visualtype_next (&visual_iter))
		{
			if (screen->root_visual == visual_iter.data->visual_id)
			{
				return visual_iter.data;
			}
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------
struct Atom
{
	Atom (const char* name) : name (name) {}

	bool valid () const
	{
		create ();
		return value ? true : false;
	}
	xcb_atom_t operator() () const
	{
		create ();
		return *value;
	}

private:
	void create () const
	{
		if (value)
			return;
		auto connection = RunLoop::instance ().getXcbConnection ();
		auto cookie = xcb_intern_atom (connection, 0, name.size (), name.data ());
		if (auto reply = xcb_intern_atom_reply (connection, cookie, nullptr))
		{
			value = Optional<xcb_atom_t> (reply->atom);
			free (reply);
		}
	}

	std::string name;
	mutable Optional<xcb_atom_t> value;
};

static Atom xEmbedInfoAtom ("_XEMBED_INFO");
static Atom xEmbedAtom ("_XEMBED");

//------------------------------------------------------------------------
struct XEmbedInfo
{
	uint32_t version{1};
	uint32_t flags{0};
};

//------------------------------------------------------------------------
struct SimpleWindow
{
	SimpleWindow (::Window parentId, CPoint size) : size (size)
	{
		auto connection = RunLoop::instance ().getXcbConnection ();
		auto setup = xcb_get_setup (connection);
		auto iter = xcb_setup_roots_iterator (setup);
		auto screen = iter.data;
		visual = getVisualType (screen);
#if 0
		parentId = screen->root;
#endif
		uint32_t paramMask = XCB_CW_BACK_PIXMAP | XCB_CW_BACKING_STORE | XCB_CW_EVENT_MASK;
		xcb_params_cw_t params{};
		params.back_pixel = XCB_BACK_PIXMAP_NONE;
		params.backing_store = XCB_BACKING_STORE_WHEN_MAPPED;
		params.event_mask = XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
							XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
							XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
							XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_POINTER_MOTION_HINT |
							XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_EXPOSURE |
							XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_EXPOSURE;

		xcb_aux_create_window (connection, XCB_COPY_FROM_PARENT, getID (), parentId, 0, 0, size.x,
							   size.y, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, XCB_COPY_FROM_PARENT,
							   paramMask, &params);

		// setup XEMBED
		if (xEmbedInfoAtom.valid ())
		{
			XEmbedInfo info;
			xcb_change_property (connection, XCB_PROP_MODE_REPLACE, getID (), xEmbedInfoAtom (),
								 xEmbedInfoAtom (), 32, 2, &info);
		}

		xcb_flush (connection);
	}

	~SimpleWindow () noexcept {}

	xcb_window_t getID () const { return id; }
	xcb_visualtype_t* getVisual () const { return visual; }

	void setSize (const CRect& rect)
	{
		size = rect.getSize ();
		auto connection = RunLoop::instance ().getXcbConnection ();
		uint16_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
						XCB_CONFIG_WINDOW_HEIGHT;
		uint32_t values[] = {static_cast<uint32_t> (rect.left), static_cast<uint32_t> (rect.top),
							 static_cast<uint32_t> (rect.getWidth ()),
							 static_cast<uint32_t> (rect.getHeight ())};
		xcb_configure_window (connection, getID (), mask, values);
		xcb_flush (connection);
	}

	const CPoint& getSize () const { return size; }

private:
	xcb_window_t id{xcb_generate_id (RunLoop::instance ().getXcbConnection ())};
	CPoint size;
	xcb_visualtype_t* visual{nullptr};
};

//------------------------------------------------------------------------
struct RedrawTimerHandler
	: ITimerHandler
	, NonAtomicReferenceCounted
{
	using RedrawCallback = std::function<void()>;

	RedrawTimerHandler (uint64_t delay, RedrawCallback&& redrawCallback)
		: redrawCallback (std::move (redrawCallback))
	{
		RunLoop::instance ().get ()->registerTimer (delay, this);
	}
	~RedrawTimerHandler () noexcept { RunLoop::instance ().get ()->unregisterTimer (this); }

	void onTimer () override
	{
		SharedPointer<RedrawTimerHandler> Self (this);
		Self->redrawCallback ();
	}

	RedrawCallback redrawCallback;
};

//------------------------------------------------------------------------
struct Frame::Impl : IFrameEventHandler
{
	using RectList = std::vector<CRect>;

	SimpleWindow window;
	Cairo::SurfaceHandle windowSurface;
	Cairo::SurfaceHandle backBuffer;
	IPlatformFrameCallback* frame;
	SharedPointer<RedrawTimerHandler> redrawTimer;
	SharedPointer<Cairo::Context> drawContext;
	RectList dirtyRects;
	CCursorType currentCursor{kCursorDefault};
	bool pointerGrabed{false};

	Impl (::Window parent, CPoint size, IPlatformFrameCallback* frame)
		: window (parent, size), frame (frame)
	{
		RunLoop::instance ().registerWindowEventHandler (window.getID (), this);
	}

	~Impl () noexcept { RunLoop::instance ().unregisterWindowEventHandler (window.getID ()); }

	void setSize (const CRect& size)
	{
		window.setSize (size);
		if (windowSurface)
		{
			cairo_xcb_surface_set_size (windowSurface, size.getWidth (), size.getHeight ());
			backBuffer = Cairo::SurfaceHandle (
				cairo_surface_create_similar (windowSurface, CAIRO_CONTENT_COLOR_ALPHA,
											  window.getSize ().x, window.getSize ().y));
			drawContext = makeOwned<Cairo::Context> (size, backBuffer);
			dirtyRects.clear ();
			dirtyRects.push_back (size);
			redraw ();
		}
	}

	//------------------------------------------------------------------------
	void setCursor (CCursorType cursor)
	{
		if (currentCursor == cursor)
			return;
		currentCursor = cursor;
		setCursorInternal (cursor);
	}

	//------------------------------------------------------------------------
	void setCursorInternal (CCursorType cursor)
	{
		auto xcb = RunLoop::instance ().getXcbConnection ();
		xcb_params_cw_t params;
		params.cursor = RunLoop::instance ().getCursorID (cursor);
		xcb_aux_change_window_attributes (xcb, window.getID (), XCB_CW_CURSOR, &params);
		xcb_aux_sync (xcb);
		xcb_flush (xcb);
	}

	//------------------------------------------------------------------------
	void blitBackbufferToWindow (const CRect& rect)
	{
		Cairo::ContextHandle windowContext (cairo_create (windowSurface));
		cairo_rectangle (windowContext, rect.left, rect.top, rect.getWidth (), rect.getHeight ());
		cairo_clip (windowContext);
		cairo_set_source_surface (windowContext, backBuffer, 0, 0);
		cairo_rectangle (windowContext, rect.left, rect.top, rect.getWidth (), rect.getHeight ());
		cairo_fill (windowContext);
		cairo_surface_flush (windowSurface);
	}

	void redraw ()
	{
		prepareDrawContext ();

		CRect copyRect;
		drawContext->beginDraw ();
		for (auto rect : dirtyRects)
		{
			drawContext->setClipRect (rect);
			drawContext->saveGlobalState ();
			frame->platformDrawRect (drawContext, rect);
			drawContext->restoreGlobalState ();
			if (copyRect.isEmpty ())
				copyRect = rect;
			else
				copyRect.unite (rect);
		}
		drawContext->endDraw ();
		blitBackbufferToWindow (copyRect);

		xcb_flush (RunLoop::instance ().getXcbConnection ());
		dirtyRects.clear ();
	}

	void invalidRect (CRect r)
	{
		dirtyRects.emplace_back (r);
		if (redrawTimer)
			return;
		redrawTimer = makeOwned<RedrawTimerHandler> (16, [this]() {
			if (dirtyRects.empty ())
				return;
			redraw ();
		});
	}

	void prepareDrawContext ()
	{
		if (!windowSurface)
		{
			auto s = cairo_xcb_surface_create (RunLoop::instance ().getXcbConnection (),
											   window.getID (), window.getVisual (),
											   window.getSize ().x, window.getSize ().y);
			windowSurface.assign (s);
			backBuffer = Cairo::SurfaceHandle (
				cairo_surface_create_similar (windowSurface, CAIRO_CONTENT_COLOR_ALPHA,
											  window.getSize ().x, window.getSize ().y));
			drawContext = makeOwned<Cairo::Context> (CRect ({}, window.getSize ()), backBuffer);
		}
	}

	void onEvent (xcb_map_notify_event_t& event) override {}
	void onEvent (xcb_key_press_event_t& event) override
	{
		auto type = (event.response_type & ~0x80);
		auto keyCode = RunLoop::instance ().makeKeyCode (event.detail, event.state);
		if (type == XCB_KEY_PRESS)
		{
			frame->platformOnKeyDown (keyCode);
		}
		else
		{
			frame->platformOnKeyUp (keyCode);
		}
	}
	void onEvent (xcb_button_press_event_t& event) override
	{
		CPoint where (event.event_x, event.event_y);
		if ((event.response_type & ~0x80) == XCB_BUTTON_PRESS) // mouse down or wheel
		{
			if (event.detail >= 4 && event.detail <= 7) // mouse wheel
			{
				auto buttons = translateModifiers (event.state);
				switch (event.detail)
				{
					case 4: // up
					{
						frame->platformOnMouseWheel (where, kMouseWheelAxisY, 1, buttons);
						break;
					}
					case 5: // down
					{
						frame->platformOnMouseWheel (where, kMouseWheelAxisY, -1, buttons);
						break;
					}
					case 6: // left
					{
						frame->platformOnMouseWheel (where, kMouseWheelAxisX, -1, buttons);
						break;
					}
					case 7: // right
					{
						frame->platformOnMouseWheel (where, kMouseWheelAxisX, 1, buttons);
						break;
					}
				}
			}
			else // mouse down
			{
				auto buttons = translateMouseButtons (event.detail);
				buttons |= translateModifiers (event.state);
				auto result = frame->platformOnMouseDown (where, buttons);
				if (result == kMouseEventHandled)
				{
					// grab the pointer
					auto xcb = RunLoop::instance ().getXcbConnection ();
					auto cookie = xcb_grab_pointer (
						xcb, false, window.getID (),
						(XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
						 XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_ENTER_WINDOW |
						 XCB_EVENT_MASK_LEAVE_WINDOW | XCB_EVENT_MASK_POINTER_MOTION),
						XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_WINDOW_NONE, XCB_CURSOR_NONE,
						XCB_TIME_CURRENT_TIME);
					if (auto reply = xcb_grab_pointer_reply (xcb, cookie, nullptr))
					{
						if (reply->status == XCB_GRAB_STATUS_SUCCESS)
							pointerGrabed = true;
						free (reply);
					}
					xcb_set_input_focus (xcb, XCB_INPUT_FOCUS_PARENT, window.getID (),
										 XCB_CURRENT_TIME);
				}
			}
		}
		else // mouse up
		{
			if (event.detail >= 4 && event.detail <= 7) // mouse wheel
			{
			}
			else
			{
				auto buttons = translateMouseButtons (event.detail);
				buttons |= translateModifiers (event.state);
				frame->platformOnMouseUp (where, buttons);
				if (pointerGrabed)
				{
					auto xcb = RunLoop::instance ().getXcbConnection ();
					xcb_ungrab_pointer (xcb, XCB_TIME_CURRENT_TIME);
					pointerGrabed = false;
				}
			}
		}
	}
	void onEvent (xcb_motion_notify_event_t& event) override
	{
		CPoint where (event.event_x, event.event_y);
		auto buttons = translateMouseButtons (event.state);
		frame->platformOnMouseMoved (where, buttons);
		// make sure we get more motion events
		auto xcb = RunLoop::instance ().getXcbConnection ();
		xcb_get_motion_events (xcb, window.getID (), event.time, event.time + 100);
	}
	void onEvent (xcb_enter_notify_event_t& event) override
	{
		if ((event.response_type & ~0x80) == XCB_LEAVE_NOTIFY)
		{
			CPoint where (event.event_x, event.event_y);
			auto buttons = translateMouseButtons (event.state);
			buttons |= translateModifiers (event.state);
			frame->platformOnMouseExited (where, buttons);
			setCursorInternal (kCursorDefault);
		}
		else
		{
			setCursorInternal (currentCursor);
		}
	}
	void onEvent (xcb_focus_in_event_t& event) override {}
	void onEvent (xcb_expose_event_t& event) override
	{
		CRect r;
		r.setTopLeft (CPoint (event.x, event.y));
		r.setSize (CPoint (event.width, event.height));
		invalidRect (r);
	}
	void onEvent (xcb_property_notify_event_t& event) override
	{
#if 1 // needed for Reaper
		if (xEmbedInfoAtom.valid () && event.atom == xEmbedInfoAtom ())
		{
			auto xcb = RunLoop::instance ().getXcbConnection ();
			xcb_map_window (xcb, window.getID ());
		}
#endif
	}
	void onEvent (xcb_client_message_event_t& event) override
	{
		if (xEmbedAtom.valid () && event.type == xEmbedAtom ())
		{
			/* XEMBED messages */
			enum class XEMBED
			{
				EMBEDDED_NOTIFY = 0,
				WINDOW_ACTIVATE = 1,
				WINDOW_DEACTIVATE = 2,
				REQUEST_FOCUS = 3,
				FOCUS_IN = 4,
				FOCUS_OUT = 5,
				FOCUS_NEXT = 6,
				FOCUS_PREV = 7,
				/* 8-9 were used for GRAB_KEY/UNGRAB_KEY */
				MODALITY_ON = 10,
				MODALITY_OFF = 11,
				REGISTER_ACCELERATOR = 12,
				UNREGISTER_ACCELERATOR = 13,
				ACTIVATE_ACCELERATOR = 14,
			};
			switch (static_cast<XEMBED> (event.data.data32[1]))
			{
				case XEMBED::EMBEDDED_NOTIFY:
				{
					auto xcb = RunLoop::instance ().getXcbConnection ();
					xcb_map_window (xcb, window.getID ());
					break;
				}
				case XEMBED::WINDOW_ACTIVATE:
				{
					frame->platformOnWindowActivate (true);
					break;
				}
				case XEMBED::WINDOW_DEACTIVATE:
				{
					frame->platformOnWindowActivate (false);
					break;
				}
				case XEMBED::FOCUS_IN:
				{
					frame->platformOnActivate (true);
					break;
				}
				case XEMBED::FOCUS_OUT:
				{
					frame->platformOnActivate (false);
					break;
				}
				case XEMBED::FOCUS_NEXT:
				{
					// we could send a tab keycode here...
					break;
				}
				case XEMBED::FOCUS_PREV:
				{
					// we could send a shift-tab keycode here...
					break;
				}
				case XEMBED::MODALITY_ON:
				case XEMBED::MODALITY_OFF:
				case XEMBED::REGISTER_ACCELERATOR:
				case XEMBED::UNREGISTER_ACCELERATOR:
				case XEMBED::ACTIVATE_ACCELERATOR:
				case XEMBED::REQUEST_FOCUS:
					break;
			}
		}
	}
};

//------------------------------------------------------------------------
Frame::Frame (IPlatformFrameCallback* frame,
			  const CRect& size,
			  uint32_t parent,
			  IPlatformFrameConfig* config)
	: IPlatformFrame (frame)
{
	auto cfg = dynamic_cast<FrameConfig*> (config);
	if (cfg && cfg->runLoop)
	{
		RunLoop::init (cfg->runLoop);
	}

	impl = std::unique_ptr<Impl> (new Impl (parent, {size.getWidth (), size.getHeight ()}, frame));

	frame->platformOnActivate (true);
}

//------------------------------------------------------------------------
Frame::~Frame ()
{
	impl.reset ();
	RunLoop::exit ();
}

//------------------------------------------------------------------------
bool Frame::getGlobalPosition (CPoint& pos) const
{
	return false;
}

//------------------------------------------------------------------------
bool Frame::setSize (const CRect& newSize)
{
	vstgui_assert (impl);
	impl->setSize (newSize);
	return true;
}

//------------------------------------------------------------------------
bool Frame::getSize (CRect& size) const
{
	return false;
}

//------------------------------------------------------------------------
bool Frame::getCurrentMousePosition (CPoint& mousePosition) const
{
	return false;
}

//------------------------------------------------------------------------
bool Frame::getCurrentMouseButtons (CButtonState& buttons) const
{
	return false;
}

//------------------------------------------------------------------------
bool Frame::setMouseCursor (CCursorType type)
{
	impl->setCursor (type);
	return true;
}

//------------------------------------------------------------------------
bool Frame::invalidRect (const CRect& rect)
{
	impl->invalidRect (rect);
	return true;
}

//------------------------------------------------------------------------
bool Frame::scrollRect (const CRect& src, const CPoint& distance)
{
	(void)src;
	(void)distance;
	return false;
}

//------------------------------------------------------------------------
bool Frame::showTooltip (const CRect& rect, const char* utf8Text)
{
#warning TODO: Implementation
	return false;
}

//------------------------------------------------------------------------
bool Frame::hideTooltip ()
{
#warning TODO: Implementation
	return false;
}

//------------------------------------------------------------------------
void* Frame::getPlatformRepresentation () const
{
	return reinterpret_cast<void*> (impl->window.getID ());
}

//------------------------------------------------------------------------
SharedPointer<IPlatformTextEdit> Frame::createPlatformTextEdit (IPlatformTextEditCallback* textEdit)
{
	return nullptr;
}

//------------------------------------------------------------------------
SharedPointer<IPlatformOptionMenu> Frame::createPlatformOptionMenu ()
{
	return nullptr;
}

#if VSTGUI_OPENGL_SUPPORT
//------------------------------------------------------------------------
SharedPointer<IPlatformOpenGLView> Frame::createPlatformOpenGLView ()
{
#	warning TODO: Implementation
	return nullptr;
}
#endif

//------------------------------------------------------------------------
SharedPointer<IPlatformViewLayer> Frame::createPlatformViewLayer (
	IPlatformViewLayerDelegate* drawDelegate, IPlatformViewLayer* parentLayer)
{
	// optional
	return nullptr;
}

//------------------------------------------------------------------------
SharedPointer<COffscreenContext> Frame::createOffscreenContext (CCoord width,
																CCoord height,
																double scaleFactor)
{
	CPoint size (width * scaleFactor, height * scaleFactor);
	auto bitmap = new Cairo::Bitmap (&size);
	bitmap->setScaleFactor (scaleFactor);
	auto context = owned (new Cairo::Context (bitmap));
	bitmap->forget ();
	if (context->valid ())
		return context;
	return nullptr;
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
DragResult Frame::doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap)
{
	return kDragError;
}
#endif

//------------------------------------------------------------------------
bool Frame::doDrag (const DragDescription& dragDescription,
					const SharedPointer<IDragCallback>& callback)
{
	return false;
}

//------------------------------------------------------------------------
void Frame::setClipboard (const SharedPointer<IDataPackage>& data){
#warning TODO: Implementation
}

//------------------------------------------------------------------------
SharedPointer<IDataPackage> Frame::getClipboard ()
{
#warning TODO: Implementation
	return nullptr;
}

//------------------------------------------------------------------------
PlatformType Frame::getPlatformType () const
{
	return kX11EmbedWindowID;
}

//------------------------------------------------------------------------
Frame::CreateIResourceInputStreamFunc Frame::createResourceInputStreamFunc =
	[](const CResourceDescription& desc) -> IPlatformResourceInputStream::Ptr {
	if (desc.type != CResourceDescription::kStringType)
		return nullptr;
	auto path = Platform::getInstance ().getPath ();
	path += "/Contents/Resources/";
	path += desc.u.name;
	return FileResourceInputStream::create (path);
};

//------------------------------------------------------------------------
} // X11

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
IPlatformFrame* IPlatformFrame::createPlatformFrame (IPlatformFrameCallback* frame,
													 const CRect& size,
													 void* parent,
													 PlatformType parentType,
													 IPlatformFrameConfig* config)
{
	if (parentType == kDefaultNative || parentType == kX11EmbedWindowID)
	{
		auto x11Parent = reinterpret_cast<XID> (parent);
		return new X11::Frame (frame, size, x11Parent, config);
	}
	return nullptr;
}

//------------------------------------------------------------------------
uint32_t IPlatformFrame::getTicks ()
{
	return static_cast<uint32_t> (X11::Platform::getCurrentTimeMs ());
}

//------------------------------------------------------------------------
auto IPlatformResourceInputStream::create (const CResourceDescription& desc) -> Ptr
{
	return X11::Frame::createResourceInputStreamFunc (desc);
}

//------------------------------------------------------------------------
} // VSTGUI
