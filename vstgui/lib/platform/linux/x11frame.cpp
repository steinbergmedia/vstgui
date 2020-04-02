// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "x11frame.h"
#include "../../cbuttonstate.h"
#include "../../cframe.h"
#include "../../crect.h"
#include "../../dragging.h"
#include "../../vstkeycode.h"
#include "../../cinvalidrectlist.h"
#include "../iplatformopenglview.h"
#include "../iplatformviewlayer.h"
#include "../iplatformtextedit.h"
#include "../iplatformoptionmenu.h"
#include "../common/fileresourceinputstream.h"
#include "../common/generictextedit.h"
#include "../common/genericoptionmenu.h"
#include "cairobitmap.h"
#include "cairocontext.h"
#include "x11platform.h"
#include "x11utils.h"
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_util.h>
#include <cairo/cairo-xcb.h>

#ifdef None
#undef None
#endif

//------------------------------------------------------------------------
namespace VSTGUI {
namespace X11 {
namespace {

//------------------------------------------------------------------------
std::string getAtomName (xcb_atom_t atom)
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
inline CButtonState translateMouseButtons (xcb_button_t value)
{
	switch (value)
	{
		case 1:
			return kLButton;
		case 2:
			return kMButton;
		case 3:
			return kRButton;
	}
	return 0;
}

//------------------------------------------------------------------------
inline CButtonState translateMouseButtons (int state)
{
	CButtonState buttons = 0;
	if (state & XCB_BUTTON_MASK_1)
		buttons |= kLButton;
	if (state & XCB_BUTTON_MASK_2)
		buttons |= kRButton;
	if (state & XCB_BUTTON_MASK_3)
		buttons |= kMButton;
	return buttons;
}

//------------------------------------------------------------------------
inline uint32_t translateModifiers (int state)
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
} // anonymous

//------------------------------------------------------------------------
struct RedrawTimerHandler
: ITimerHandler
, NonAtomicReferenceCounted
{
	using RedrawCallback = std::function<void ()>;

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
struct DrawHandler
{
	DrawHandler (const ChildWindow& window)
	{
		auto s = cairo_xcb_surface_create (RunLoop::instance ().getXcbConnection (),
										   window.getID (), window.getVisual (),
										   window.getSize ().x, window.getSize ().y);
		windowSurface.assign (s);
		onSizeChanged (window.getSize ());
		device = cairo_device_reference (cairo_surface_get_device (s));
	}

	~DrawHandler ()
	{
		cairo_device_finish (device);
		cairo_device_destroy (device);
	}

	void onSizeChanged (const CPoint& size)
	{
		cairo_xcb_surface_set_size (windowSurface, size.x, size.y);
		backBuffer = Cairo::SurfaceHandle (cairo_surface_create_similar (
			windowSurface, CAIRO_CONTENT_COLOR_ALPHA, size.x, size.y));
		CRect r;
		r.setSize (size);
		drawContext = makeOwned<Cairo::Context> (r, backBuffer);
	}

	template<typename RectList, typename Proc>
	void draw (const RectList& dirtyRects, Proc proc)
	{
		CRect copyRect;
		drawContext->beginDraw ();
		for (auto rect : dirtyRects)
		{
			drawContext->setClipRect (rect);
			drawContext->saveGlobalState ();
			proc (drawContext, rect);
			drawContext->restoreGlobalState ();
			if (copyRect.isEmpty ())
				copyRect = rect;
			else
				copyRect.unite (rect);
		}
		drawContext->endDraw ();
		blitBackbufferToWindow (copyRect);
		xcb_flush (RunLoop::instance ().getXcbConnection ());
	}

private:
	cairo_device_t* device = nullptr;
	Cairo::SurfaceHandle windowSurface;
	Cairo::SurfaceHandle backBuffer;
	SharedPointer<Cairo::Context> drawContext;

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
};

//------------------------------------------------------------------------
struct DoubleClickDetector
{
	void onMouseDown (CPoint where, CButtonState& buttons, xcb_timestamp_t time)
	{
		switch (state)
		{
			case State::MouseDown:
			case State::Uninitialized:
			{
				state = State::MouseDown;
				firstClickState = buttons;
				firstClickTime = time;
				point = where;
				break;
			}
			case State::MouseUp:
			{
				if (timeInside (time) && pointInside (where))
				{
					buttons |= kDoubleClick;
				}
				state = State::Uninitialized;
				break;
			}
		}
	}

	void onMouseUp (CPoint where, CButtonState buttons, xcb_timestamp_t time)
	{
		if (state == State::MouseDown && pointInside (where))
			state = State::MouseUp;
		else
			state = State::Uninitialized;
	}

	void onMouseMove (CPoint where, CButtonState buttons, xcb_timestamp_t time)
	{
		if (!pointInside (where))
			state = State::Uninitialized;
	}

private:
	bool timeInside (xcb_timestamp_t time)
	{
		constexpr xcb_timestamp_t threshold = 250; // in milliseconds
		return (time - firstClickTime) < threshold;
	}

	bool pointInside (CPoint p) const
	{
		CRect r;
		r.setTopLeft (point);
		r.setBottomRight (point);
		r.inset (-5, -5);
		return r.pointInside (p);
	}

	enum class State
	{
		Uninitialized,
		MouseDown,
		MouseUp,
	};

	State state {State::Uninitialized};
	CPoint point;
	CButtonState firstClickState;
	xcb_timestamp_t firstClickTime {0};
};

//------------------------------------------------------------------------
struct Frame::Impl : IFrameEventHandler
{
	using RectList = CInvalidRectList;

	ChildWindow window;
	DrawHandler drawHandler;
	DoubleClickDetector doubleClickDetector;
	IPlatformFrameCallback* frame;
	std::unique_ptr<GenericOptionMenuTheme> genericOptionMenuTheme;
	SharedPointer<RedrawTimerHandler> redrawTimer;
	RectList dirtyRects;
	CCursorType currentCursor {kCursorDefault};
	uint32_t pointerGrabed {0};

	//------------------------------------------------------------------------
	Impl (::Window parent, CPoint size, IPlatformFrameCallback* frame)
	: window (parent, size), drawHandler (window), frame (frame)
	{
		RunLoop::instance ().registerWindowEventHandler (window.getID (), this);
	}

	//------------------------------------------------------------------------
	~Impl () noexcept { RunLoop::instance ().unregisterWindowEventHandler (window.getID ()); }

	//------------------------------------------------------------------------
	void setSize (const CRect& size)
	{
		window.setSize (size);
		drawHandler.onSizeChanged (size.getSize ());
		dirtyRects.clear ();
		dirtyRects.add (size);
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
	void redraw ()
	{
		drawHandler.draw (dirtyRects, [&] (CDrawContext* context, const CRect& rect) {
			frame->platformDrawRect (context, rect);
		});
		dirtyRects.clear ();
	}

	//------------------------------------------------------------------------
	void invalidRect (CRect r)
	{
		dirtyRects.add (r);
		if (redrawTimer)
			return;
		redrawTimer = makeOwned<RedrawTimerHandler> (16, [this] () {
			if (dirtyRects.data ().empty ())
				return;
			redraw ();
		});
	}

	//------------------------------------------------------------------------
	void grabPointer ()
	{
		if (++pointerGrabed > 1)
			return;

		auto xcb = RunLoop::instance ().getXcbConnection ();
		auto cookie =
			xcb_grab_pointer (xcb, false, window.getID (),
							  (XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
							   XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_ENTER_WINDOW |
							   XCB_EVENT_MASK_LEAVE_WINDOW | XCB_EVENT_MASK_POINTER_MOTION),
							  XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_WINDOW_NONE,
							  XCB_CURSOR_NONE, XCB_TIME_CURRENT_TIME);
		if (auto reply = xcb_grab_pointer_reply (xcb, cookie, nullptr))
		{
			if (reply->status != XCB_GRAB_STATUS_SUCCESS)
				pointerGrabed = 0;
			free (reply);
		}
	}

	//------------------------------------------------------------------------
	void ungrabPointer ()
	{
		if (pointerGrabed == 0)
			return;
		if (--pointerGrabed > 0)
			return;
		vstgui_assert (pointerGrabed == 0);
		auto xcb = RunLoop::instance ().getXcbConnection ();
		xcb_ungrab_pointer (xcb, XCB_TIME_CURRENT_TIME);
	}

	//------------------------------------------------------------------------
	void onEvent (xcb_map_notify_event_t& event) override {}

	//------------------------------------------------------------------------
	void onEvent (xcb_key_press_event_t& event) override
	{
		auto type = (event.response_type & ~0x80);
		auto keyCode = RunLoop::instance ().getCurrentKeyEvent ();
		if (type == XCB_KEY_PRESS)
		{
			frame->platformOnKeyDown (keyCode);
		}
		else
		{
			frame->platformOnKeyUp (keyCode);
		}
	}

	//------------------------------------------------------------------------
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
				doubleClickDetector.onMouseDown (where, buttons, event.time);
				auto result = frame->platformOnMouseDown (where, buttons);
				grabPointer ();
				if (result != kMouseEventNotHandled)
				{
					auto xcb = RunLoop::instance ().getXcbConnection ();
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
				doubleClickDetector.onMouseUp (where, buttons, event.time);
				frame->platformOnMouseUp (where, buttons);
				ungrabPointer ();
			}
		}
	}

	//------------------------------------------------------------------------
	void onEvent (xcb_motion_notify_event_t& event) override
	{
		CPoint where (event.event_x, event.event_y);
		auto buttons = translateMouseButtons (event.state);
		buttons |= translateModifiers (event.state);
		doubleClickDetector.onMouseMove (where, buttons, event.time);
		frame->platformOnMouseMoved (where, buttons);
		// make sure we get more motion events
		auto xcb = RunLoop::instance ().getXcbConnection ();
		xcb_get_motion_events (xcb, window.getID (), event.time, event.time + 10000000);
	}

	//------------------------------------------------------------------------
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

	//------------------------------------------------------------------------
	void onEvent (xcb_focus_in_event_t& event) override {}

	//------------------------------------------------------------------------
	void onEvent (xcb_expose_event_t& event) override
	{
		CRect r;
		r.setTopLeft (CPoint (event.x, event.y));
		r.setSize (CPoint (event.width, event.height));
		invalidRect (r);
	}

	//------------------------------------------------------------------------
	void onEvent (xcb_property_notify_event_t& event) override
	{
#if 1 // needed for Reaper
		if (Atoms::xEmbedInfo.valid () && event.atom == Atoms::xEmbedInfo ())
		{
			auto xcb = RunLoop::instance ().getXcbConnection ();
			xcb_map_window (xcb, window.getID ());
		}
#endif
	}

	//------------------------------------------------------------------------
	void onEvent (xcb_client_message_event_t& event) override
	{
		if (Atoms::xEmbed.valid () && event.type == Atoms::xEmbed ())
		{
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
Frame::Frame (IPlatformFrameCallback* frame, const CRect& size, uint32_t parent,
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
void Frame::optionMenuPopupStarted ()
{
	impl->grabPointer ();
}

//------------------------------------------------------------------------
void Frame::optionMenuPopupStopped ()
{
	impl->ungrabPointer ();
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
	size.setSize (impl->window.getSize ());
	return true;
}

//------------------------------------------------------------------------
bool Frame::getCurrentMousePosition (CPoint& mousePosition) const
{
	xcb_query_pointer_cookie_t cookie =
		xcb_query_pointer (RunLoop::instance ().getXcbConnection (), getX11WindowID ());
	xcb_query_pointer_reply_t* reply =
		xcb_query_pointer_reply (RunLoop::instance ().getXcbConnection (), cookie, nullptr);
	if (!reply)
		return false;

	mousePosition.x = reply->win_x;
	mousePosition.y = reply->win_y;
	return true;
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
	return reinterpret_cast<void*> (getX11WindowID ());
}

//------------------------------------------------------------------------
uint32_t Frame::getX11WindowID () const
{
	return impl->window.getID ();
}

//------------------------------------------------------------------------
SharedPointer<IPlatformTextEdit> Frame::createPlatformTextEdit (IPlatformTextEditCallback* textEdit)
{
	return makeOwned<GenericTextEdit> (textEdit);
}

//------------------------------------------------------------------------
SharedPointer<IPlatformOptionMenu> Frame::createPlatformOptionMenu ()
{
	auto cFrame = dynamic_cast<CFrame*> (frame);
	GenericOptionMenuTheme theme;
	if (impl->genericOptionMenuTheme)
		theme = *impl->genericOptionMenuTheme.get ();
	auto optionMenu = makeOwned<GenericOptionMenu> (cFrame, 0, theme);
	optionMenu->setListener (this);
	return optionMenu;
}

#if VSTGUI_OPENGL_SUPPORT
//------------------------------------------------------------------------
SharedPointer<IPlatformOpenGLView> Frame::createPlatformOpenGLView ()
{
#warning TODO: Implementation
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
SharedPointer<COffscreenContext> Frame::createOffscreenContext (CCoord width, CCoord height,
																double scaleFactor)
{
	CPoint size (width * scaleFactor, height * scaleFactor);
	auto bitmap = new Cairo::Bitmap (size);
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
void Frame::setClipboard (const SharedPointer<IDataPackage>& data) {
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
	return PlatformType::kX11EmbedWindowID;
}

//------------------------------------------------------------------------
Optional<UTF8String> Frame::convertCurrentKeyEventToText ()
{
	return RunLoop::instance ().convertCurrentKeyEventToText ();
}

//------------------------------------------------------------------------
bool Frame::setupGenericOptionMenu (bool use, GenericOptionMenuTheme* theme)
{
	if (theme)
		impl->genericOptionMenuTheme =
			std::unique_ptr<GenericOptionMenuTheme> (new GenericOptionMenuTheme (*theme));
	else
		impl->genericOptionMenuTheme = nullptr;
	return true;
}

//------------------------------------------------------------------------
Frame::CreateIResourceInputStreamFunc Frame::createResourceInputStreamFunc =
	[] (const CResourceDescription& desc) -> PlatformResourceInputStreamPtr {
	if (desc.type != CResourceDescription::kStringType)
		return nullptr;
	auto path = Platform::getInstance ().getPath ();
	path += "/Contents/Resources/";
	path += desc.u.name;
	return FileResourceInputStream::create (path);
};

//------------------------------------------------------------------------
const UTF8String& Frame::getResourcePath ()
{
	if (!userDefinedResourcePath.empty ())
	{
		return userDefinedResourcePath;
	}
	else
	{
		static UTF8String path = Platform::getInstance ().getPath () + "/Contents/Resources/";
		return path;
	}
}

UTF8String Frame::userDefinedResourcePath;

//------------------------------------------------------------------------
} // X11
} // VSTGUI
