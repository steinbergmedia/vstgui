// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "gdkframe.h"
#include "../iplatformtextedit.h"
#include "../iplatformoptionmenu.h"
#include "../iplatformopenglview.h"
#include "../iplatformviewlayer.h"
#include "../../vstkeycode.h"
#include "cairobitmap.h"
#include "cairocontext.h"
#include <chrono>
#include <gtkmm.h>
#include <cairomm/cairomm.h>
#include <unordered_map>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace GDK {
namespace {

using VirtMap = std::unordered_map<guint, uint16_t>;
const VirtMap keyMap = {{GDK_KEY_BackSpace, VKEY_BACK},
						{GDK_KEY_Tab, VKEY_TAB},
						{GDK_KEY_ISO_Left_Tab, VKEY_TAB},
						{GDK_KEY_Clear, VKEY_CLEAR},
						{GDK_KEY_Return, VKEY_RETURN},
						{GDK_KEY_Pause, VKEY_PAUSE},
						{GDK_KEY_Escape, VKEY_ESCAPE},
						{GDK_KEY_space, VKEY_SPACE},
						{GDK_KEY_KP_Next, VKEY_NEXT},
						{GDK_KEY_End, VKEY_END},
						{GDK_KEY_Home, VKEY_HOME},

						{GDK_KEY_Left, VKEY_LEFT},
						{GDK_KEY_Up, VKEY_UP},
						{GDK_KEY_Right, VKEY_RIGHT},
						{GDK_KEY_Down, VKEY_DOWN},
						{GDK_KEY_Page_Up, VKEY_PAGEUP},
						{GDK_KEY_Page_Down, VKEY_PAGEDOWN},
						{GDK_KEY_KP_Page_Up, VKEY_PAGEUP},
						{GDK_KEY_KP_Page_Down, VKEY_PAGEDOWN},

						{GDK_KEY_Select, VKEY_SELECT},
						{GDK_KEY_Print, VKEY_PRINT},
						{GDK_KEY_KP_Enter, VKEY_ENTER},
						{GDK_KEY_Insert, VKEY_INSERT},
						{GDK_KEY_Delete, VKEY_DELETE},
						{GDK_KEY_Help, VKEY_HELP},

						{GDK_KEY_KP_Multiply, VKEY_MULTIPLY},
						{GDK_KEY_KP_Add, VKEY_ADD},
						{GDK_KEY_KP_Separator, VKEY_SEPARATOR},
						{GDK_KEY_KP_Subtract, VKEY_SUBTRACT},
						{GDK_KEY_KP_Decimal, VKEY_DECIMAL},
						{GDK_KEY_KP_Divide, VKEY_DIVIDE},
						{GDK_KEY_F1, VKEY_F1},
						{GDK_KEY_F2, VKEY_F2},
						{GDK_KEY_F3, VKEY_F3},
						{GDK_KEY_F4, VKEY_F4},
						{GDK_KEY_F5, VKEY_F5},
						{GDK_KEY_F6, VKEY_F6},
						{GDK_KEY_F7, VKEY_F7},
						{GDK_KEY_F8, VKEY_F8},
						{GDK_KEY_F9, VKEY_F9},
						{GDK_KEY_F10, VKEY_F10},
						{GDK_KEY_F11, VKEY_F11},
						{GDK_KEY_F12, VKEY_F12},
						{GDK_KEY_Num_Lock, VKEY_NUMLOCK},
						{GDK_KEY_Scroll_Lock, VKEY_SCROLL}, // correct ?
						{GDK_KEY_Shift_L, VKEY_SHIFT},
						{GDK_KEY_Shift_R, VKEY_SHIFT},
						{GDK_KEY_Control_L, VKEY_CONTROL},
						{GDK_KEY_Control_R, VKEY_CONTROL},
						{GDK_KEY_Alt_L, VKEY_ALT},
						{GDK_KEY_Alt_R, VKEY_ALT},
						{GDK_KEY_VoidSymbol, 0}};

//------------------------------------------------------------------------
VstKeyCode keyCodeFromEvent (GdkEventKey* event)
{
	VstKeyCode key{};
	if (event->state & GDK_SHIFT_MASK)
		key.modifier |= MODIFIER_SHIFT;
	if (event->state & GDK_CONTROL_MASK)
		key.modifier |= MODIFIER_CONTROL;
	if (event->state & GDK_MOD1_MASK)
		key.modifier |= MODIFIER_ALTERNATE;
	auto it = keyMap.find (event->keyval);
	if (it != keyMap.end ())
		key.virt = it->second;
	else
		key.character = towlower (gdk_keyval_to_unicode (event->keyval));
	return key;
}

//------------------------------------------------------------------------
CButtonState buttonState (guint b, guint modifiers, GdkEventType eventType = GDK_BUTTON_PRESS)
{
	using namespace Gdk;
	CButtonState buttonState{};
	if (modifiers & GDK_SHIFT_MASK)
		buttonState |= kShift;
	if (modifiers & GDK_CONTROL_MASK)
		buttonState |= kControl;
	if (modifiers & GDK_MOD1_MASK)
		buttonState |= kAlt;
	if (modifiers & GDK_BUTTON1_MASK)
		buttonState |= kLButton;
	if (modifiers & GDK_BUTTON2_MASK)
		buttonState |= kMButton;
	if (modifiers & GDK_BUTTON3_MASK)
		buttonState |= kRButton;
	if (modifiers & GDK_BUTTON4_MASK)
		buttonState |= kButton4;
	if (modifiers & GDK_BUTTON5_MASK)
		buttonState |= kButton5;
	switch (b)
	{
		case 1:
			buttonState |= kLButton;
			break;
		case 2:
			buttonState |= kMButton;
			break;
		case 3:
			buttonState |= kRButton;
			break;
		case 4:
			buttonState |= kButton4;
			break;
		case 5:
			buttonState |= kButton5;
			break;
	}
	if (eventType == GDK_DOUBLE_BUTTON_PRESS)
		buttonState |= kDoubleClick;
	return buttonState;
}

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
Frame::CreateIResourceInputStreamFunc Frame::createResourceInputStreamFunc =
	[](const CResourceDescription&) { return nullptr; };
Frame::CreatePlatformTimerFunc Frame::createPlatformTimerFunc = [](IPlatformTimerCallback*) {
	return nullptr;
};

//------------------------------------------------------------------------
struct Frame::Impl
{
	Glib::RefPtr<Gdk::Window> window;
};

//------------------------------------------------------------------------
Frame* Frame::create (IPlatformFrameCallback* frameCallback,
					  const CRect& size,
					  void* parent,
					  IPlatformFrameConfig* config)
{
	auto frame = new Frame (frameCallback);
	if (frame->init (size, parent, config))
		return frame;
	delete frame;
	return nullptr;
}

//------------------------------------------------------------------------
Frame::Frame (IPlatformFrameCallback* frame) : IPlatformFrame (frame)
{
	impl = std::unique_ptr<Impl> (new Impl);
}

//------------------------------------------------------------------------
Frame::~Frame () noexcept = default;

//------------------------------------------------------------------------
bool Frame::init (const CRect& size, void* parent, IPlatformFrameConfig* config)
{
	auto gdkWindowParent = static_cast<GdkWindow*> (parent);

	GdkWindowAttr attributes{};
	attributes.x = size.left;
	attributes.y = size.top;
	attributes.width = size.getWidth ();
	attributes.height = size.getHeight ();
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gdk_window_get_visual (gdkWindowParent);
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.cursor = nullptr;
	attributes.wmclass_name = nullptr;
	attributes.wmclass_class = nullptr;
	attributes.override_redirect = false;
	attributes.type_hint = GDK_WINDOW_TYPE_HINT_NORMAL;
	attributes.event_mask = GDK_EXPOSURE_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_MOTION_MASK |
							GDK_BUTTON1_MOTION_MASK | GDK_BUTTON2_MOTION_MASK |
							GDK_BUTTON3_MOTION_MASK | GDK_BUTTON_PRESS_MASK |
							GDK_BUTTON_RELEASE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK |
							GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_FOCUS_CHANGE_MASK |
							GDK_STRUCTURE_MASK | GDK_PROPERTY_CHANGE_MASK |
							GDK_VISIBILITY_NOTIFY_MASK | GDK_SCROLL_MASK | GDK_SMOOTH_SCROLL_MASK;

	auto parentWindow = Glib::wrap (gdkWindowParent);
	impl->window =
		Gdk::Window::create (parentWindow, &attributes, GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL);
	if (!impl->window)
		return false;
	impl->window->set_accept_focus (true);
	impl->window->show ();
	return true;
}

//------------------------------------------------------------------------
bool Frame::getGlobalPosition (CPoint& pos) const
{
	assert (impl->window);

	int x, y;
	impl->window->get_origin (x, y);
	pos.x = x;
	pos.y = y;
	return true;
}

//------------------------------------------------------------------------
bool Frame::setSize (const CRect& newSize)
{
	assert (impl->window);

	impl->window->move_resize (newSize.left, newSize.top, newSize.getWidth (),
							   newSize.getHeight ());
	return true;
}

//------------------------------------------------------------------------
bool Frame::getSize (CRect& size) const
{
	assert (impl->window);

	int x, y;
	impl->window->get_position (x, y);
	size.left = x;
	size.top = y;
	size.setWidth (impl->window->get_width ());
	size.setHeight (impl->window->get_height ());
	return true;
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
	return false;
}

//------------------------------------------------------------------------
bool Frame::invalidRect (const CRect& rect)
{
	assert (impl->window);

	impl->window->invalidate_rect (
		Gdk::Rectangle (rect.left, rect.top, rect.getWidth (), rect.getHeight ()), true);
	return true;
}

//------------------------------------------------------------------------
bool Frame::scrollRect (const CRect& src, const CPoint& distance)
{
	return false;
}

//------------------------------------------------------------------------
bool Frame::showTooltip (const CRect& rect, const char* utf8Text)
{
	return false;
}

//------------------------------------------------------------------------
bool Frame::hideTooltip ()
{
	return false;
}

//------------------------------------------------------------------------
void* Frame::getPlatformRepresentation () const
{
	if (impl->window)
		return impl->window->gobj ();
	return nullptr;
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
	return nullptr;
}
#endif // VSTGUI_OPENGL_SUPPORT

//------------------------------------------------------------------------
SharedPointer<IPlatformViewLayer> Frame::createPlatformViewLayer (
	IPlatformViewLayerDelegate* drawDelegate, IPlatformViewLayer* parentLayer)
{
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
void Frame::setClipboard (const SharedPointer<IDataPackage>& data) {}

//------------------------------------------------------------------------
SharedPointer<IDataPackage> Frame::getClipboard ()
{
	return nullptr;
}

//------------------------------------------------------------------------
auto Frame::getPlatformType () const -> PlatformType
{
	return PlatformType::kGdkWindow;
}

//------------------------------------------------------------------------
void Frame::onFrameClosed () {}

//------------------------------------------------------------------------
void Frame::drawDirtyRegions ()
{
	auto updateRegion = gdk_window_get_update_area (impl->window->gobj ());
	if (!updateRegion)
		updateRegion = gdk_window_get_clip_region (impl->window->gobj ());
	if (!updateRegion)
		return;
	CRect size;
	getSize (size);

	auto region = ::Cairo::RefPtr<::Cairo::Region> (new ::Cairo::Region (updateRegion, false));

#if GDK_VERSION_GT(3, 19)
	auto cairoContext = impl->window->begin_draw_frame (region);
	auto context = owned (new Cairo::Context (size, cairoContext->get_cairo_context ()->cobj ()));
#else
	impl->window->begin_paint_region (region);
	auto cairoContext = impl->window->create_cairo_context ();
	auto context = owned (new Cairo::Context (size, cairoContext->cobj ()));
#endif

	auto numRects = region->get_num_rectangles ();
	for (auto i = 0; i < numRects; ++i)
	{
		auto rect = region->get_rectangle (i);
		CRect r;
		r.left = rect.x;
		r.top = rect.y;
		r.setWidth (rect.width);
		r.setHeight (rect.height);
		frame->platformDrawRect (context, r);
	}

#if GDK_VERSION_GT(3, 19)
	impl->window->end_draw_frame (cairoContext);
#else
	impl->window->end_paint ();
#endif
}

//------------------------------------------------------------------------
bool Frame::handleEvent (void* gdkEvent)
{
	auto ev = reinterpret_cast<GdkEvent*> (gdkEvent);
	switch (ev->type)
	{
		case GDK_EXPOSE:
		{
			drawDirtyRegions ();
			return true;
		}
		case GDK_2BUTTON_PRESS:
		case GDK_3BUTTON_PRESS:
		case GDK_BUTTON_PRESS:
		{
			auto event = reinterpret_cast<GdkEventButton*> (ev);
			CPoint where{event->x, event->y};
			auto eventButtonState = buttonState (event->button, event->state, event->type);
			return frame->platformOnMouseDown (where, eventButtonState) == kMouseEventHandled;
		}
		case GDK_BUTTON_RELEASE:
		{
			auto event = reinterpret_cast<GdkEventButton*> (ev);
			CPoint where{event->x, event->y};
			auto eventButtonState = buttonState (event->button, event->state, event->type);
			return frame->platformOnMouseUp (where, eventButtonState) == kMouseEventHandled;
		}
		case GDK_MOTION_NOTIFY:
		{
			auto event = reinterpret_cast<GdkEventMotion*> (ev);
			CPoint where{event->x, event->y};
			auto eventButtonState = buttonState (0, event->state);
			return frame->platformOnMouseMoved (where, eventButtonState) == kMouseEventHandled;
		}
		case GDK_SCROLL:
		{
			auto event = reinterpret_cast<GdkEventScroll*> (ev);
			auto eventButtonState = buttonState (0, event->state);
			CPoint where{event->x, event->y};
			float distance = 0.f;
			CMouseWheelAxis axis = kMouseWheelAxisY;
			switch (event->direction)
			{
				case GDK_SCROLL_SMOOTH:
				{
					bool result = false;
					if (event->delta_x)
					{
						result |=
							frame->platformOnMouseWheel (where, kMouseWheelAxisX, -event->delta_x,
														 eventButtonState) == kMouseEventHandled;
					}
					if (event->delta_y)
					{
						result |=
							frame->platformOnMouseWheel (where, kMouseWheelAxisY, -event->delta_y,
														 eventButtonState) == kMouseEventHandled;
					}
					break;
				}
				case GDK_SCROLL_UP:
				{
					axis = kMouseWheelAxisY;
					distance = 1.f;
					break;
				}
				case GDK_SCROLL_DOWN:
				{
					axis = kMouseWheelAxisY;
					distance = -1.f;
					break;
				}
				case GDK_SCROLL_LEFT:
				{
					axis = kMouseWheelAxisX;
					distance = 1.f;
					break;
				}
				case GDK_SCROLL_RIGHT:
				{
					axis = kMouseWheelAxisX;
					distance = -1.f;
					break;
				}
			}
			return frame->platformOnMouseWheel (where, axis, distance, eventButtonState) ==
				   kMouseEventHandled;
		}
		case GDK_KEY_PRESS:
		{
			auto event = reinterpret_cast<GdkEventKey*> (ev);
			auto keyCode = keyCodeFromEvent (event);
			return frame->platformOnKeyDown (keyCode) != -1;
		}
		case GDK_KEY_RELEASE:
		{
			auto event = reinterpret_cast<GdkEventKey*> (ev);
			auto keyCode = keyCodeFromEvent (event);
			return frame->platformOnKeyUp (keyCode) != -1;
		}
		case GDK_FOCUS_CHANGE:
		{
			auto event = reinterpret_cast<GdkEventFocus*> (ev);
			auto hasFocus = event->in != 0;
			frame->platformOnActivate (hasFocus);
			return true;
		}
		case GDK_LEAVE_NOTIFY:
		{
			auto event = reinterpret_cast<GdkEventCrossing*> (ev);
			CPoint where{event->x, event->y};
			auto eventButtonState = buttonState (0, event->state, event->type);
			frame->platformOnMouseExited (where, eventButtonState);
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------
} // GDK

//------------------------------------------------------------------------
IPlatformFrame* IPlatformFrame::createPlatformFrame (IPlatformFrameCallback* frame,
													 const CRect& size,
													 void* parent,
													 PlatformType parentType,
													 IPlatformFrameConfig* config)
{
	return GDK::Frame::create (frame, size, parent, config);
}

//------------------------------------------------------------------------
uint32_t IPlatformFrame::getTicks ()
{
	using namespace std::chrono;
	return duration_cast<milliseconds> (steady_clock::now ().time_since_epoch ()).count ();
}

//------------------------------------------------------------------------
auto IPlatformResourceInputStream::create (const CResourceDescription& desc) -> Ptr
{
	return GDK::Frame::createResourceInputStreamFunc (desc);
}

//------------------------------------------------------------------------
SharedPointer<IPlatformTimer> IPlatformTimer::create (IPlatformTimerCallback* callback)
{
	return GDK::Frame::createPlatformTimerFunc (callback);
}

//------------------------------------------------------------------------
} // VSTGUI
