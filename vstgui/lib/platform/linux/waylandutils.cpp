// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE
// Originally written and contributed to VSTGUI by PreSonus Software Ltd.

#include "waylandutils.h"
#include "../../cstring.h"

#include <wayland-client.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <linux/input.h>
#include <xkbcommon/xkbcommon.h>
#include <sys/mman.h>

namespace VSTGUI {
namespace Wayland {
namespace {

using VirtMap = std::unordered_map<xkb_keysym_t, VirtualKey>;
const VirtMap keyMap = {{XKB_KEY_BackSpace, VirtualKey::Back},
						{XKB_KEY_Tab, VirtualKey::Tab},
						{XKB_KEY_Clear, VirtualKey::Clear},
						{XKB_KEY_Return, VirtualKey::Return},
						{XKB_KEY_Pause, VirtualKey::Pause},
						{XKB_KEY_Escape, VirtualKey::Escape},
						{XKB_KEY_space, VirtualKey::Space},
						{XKB_KEY_End, VirtualKey::End},
						{XKB_KEY_Home, VirtualKey::Home},

						{XKB_KEY_Left, VirtualKey::Left},
						{XKB_KEY_Up, VirtualKey::Up},
						{XKB_KEY_Right, VirtualKey::Right},
						{XKB_KEY_Down, VirtualKey::Down},
						{XKB_KEY_Page_Up, VirtualKey::PageUp},
						{XKB_KEY_Page_Down, VirtualKey::PageDown},

						{XKB_KEY_Select, VirtualKey::Select},
						{XKB_KEY_Print, VirtualKey::Print},
						{XKB_KEY_KP_Enter, VirtualKey::Enter},
						{XKB_KEY_Insert, VirtualKey::Insert},
						{XKB_KEY_Delete, VirtualKey::Delete},
						{XKB_KEY_Help, VirtualKey::Help},
						// Numpads ???
						{XKB_KEY_KP_Multiply, VirtualKey::Multiply},
						{XKB_KEY_KP_Add, VirtualKey::Add},
						{XKB_KEY_KP_Separator, VirtualKey::Separator},
						{XKB_KEY_KP_Subtract, VirtualKey::Subtract},
						{XKB_KEY_KP_Decimal, VirtualKey::Decimal},
						{XKB_KEY_KP_Divide, VirtualKey::Divide},
						{XKB_KEY_F1, VirtualKey::F1},
						{XKB_KEY_F2, VirtualKey::F2},
						{XKB_KEY_F3, VirtualKey::F3},
						{XKB_KEY_F4, VirtualKey::F4},
						{XKB_KEY_F5, VirtualKey::F5},
						{XKB_KEY_F6, VirtualKey::F6},
						{XKB_KEY_F7, VirtualKey::F7},
						{XKB_KEY_F8, VirtualKey::F8},
						{XKB_KEY_F9, VirtualKey::F9},
						{XKB_KEY_F10, VirtualKey::F10},
						{XKB_KEY_F11, VirtualKey::F11},
						{XKB_KEY_F12, VirtualKey::F12},
						{XKB_KEY_Num_Lock, VirtualKey::NumLock},
						{XKB_KEY_Scroll_Lock, VirtualKey::Scroll}, // correct ?
#if 0
						{XKB_KEY_Shift_L, VirtualKey::SHIFT},
						{XKB_KEY_Shift_R, VirtualKey::SHIFT},
						{XKB_KEY_Control_L, VirtualKey::CONTROL},
						{XKB_KEY_Control_R, VirtualKey::CONTROL},
						{XKB_KEY_Alt_L, VirtualKey::ALT},
						{XKB_KEY_Alt_R, VirtualKey::ALT},
#endif
						{XKB_KEY_VoidSymbol, VirtualKey::None}};
const VirtMap shiftKeyMap = {{XKB_KEY_KP_Page_Up, VirtualKey::PageUp},
							 {XKB_KEY_KP_Page_Down, VirtualKey::PageDown},
							 {XKB_KEY_KP_Home, VirtualKey::Home},
							 {XKB_KEY_KP_End, VirtualKey::End}};

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
// InputHandler
//------------------------------------------------------------------------
class InputHandler : public wl_pointer_listener
{
public:
	static InputHandler& instance ()
	{
		static InputHandler inputHandler;
		return inputHandler;
	}

	static constexpr int kWheelClick = 120;

	void addWindow (ChildWindow* window);
	void removeWindow (ChildWindow* window);

	// wl_pointer_listener
	static void onPointerEnter (void* data, wl_pointer* pointer, uint32_t serial,
								wl_surface* surface, wl_fixed_t x, wl_fixed_t y);
	static void onPointerLeave (void* data, wl_pointer* pointer, uint32_t serial,
								wl_surface* surface);
	static void onPointerMotion (void* data, wl_pointer* pointer, uint32_t time, wl_fixed_t x,
								 wl_fixed_t y);
	static void onPointerButton (void* data, wl_pointer* pointer, uint32_t serial, uint32_t time,
								 uint32_t button, uint32_t state);
	static void onPointerAxis (void* data, wl_pointer* wl_pointer, uint32_t time, uint32_t axis,
							   wl_fixed_t value);
	static void onPointerAxisSource (void* data, wl_pointer* pointer, uint32_t axisSource);
	static void onPointerAxisStop (void* data, wl_pointer* pointer, uint32_t time, uint32_t axis);
	static void onPointerAxisDiscrete (void* data, wl_pointer* pointer, uint32_t axis,
									   int32_t discrete);
	static void onPointerAxis120 (void* data, wl_pointer* pointer, uint32_t axis, int32_t discrete);
	static void onPointerFrame (void* data, wl_pointer* pointer);
	static void onAxisRelativeDirection (void* data, wl_pointer* wl_pointer, uint32_t axis,
									   uint32_t direction);

private:
	struct PointerEvent
	{
		uint32_t eventMask {0};
		wl_fixed_t x {0};
		wl_fixed_t y {0};
		uint32_t button {0};
		uint32_t state {0};
		uint32_t time {0};
		uint32_t serial {0};
		struct Axis
		{
			wl_fixed_t value {0};
			int32_t discrete {0};
			int32_t direction {0};
			bool valid {false};
		} axes[2];
		uint32_t axisSource {0};
		uint32_t buttonState {0};
		wl_surface* focus {nullptr};
		wl_surface* previousFocus {nullptr};
	};

	// https://wayland-book.com/seat/example.html#rigging-up-pointer-events
	enum PointerEventMask
	{
		kPointerEnter = 1 << 0,
		kPointerLeave = 1 << 1,
		kPointerMotion = 1 << 2,
		kPointerButton = 1 << 3,
		kPointerAxis = 1 << 4,
		kPointerAxisSource = 1 << 5,
		kPointerAxisStop = 1 << 6,
		kPointerAxisDiscrete = 1 << 7,
		kPointerAxis120 = 1 << 8,
		kPointerAxisRelativeDirection = 1 << 9,
	};

	wl_pointer* pointer = nullptr;
	PointerEvent pointerEvent;
	bool initialized;
	std::vector<ChildWindow*> windows;

	InputHandler ();
	void initialize ();
	void terminate ();
	void dispatch (const PointerEvent& event);
};

//------------------------------------------------------------------------
// InputHandler
//------------------------------------------------------------------------
InputHandler::InputHandler () : pointer (nullptr), initialized (false)
{
	enter = onPointerEnter;
	leave = onPointerLeave;
	motion = onPointerMotion;
	button = onPointerButton;
	axis = onPointerAxis;
	axis_source = onPointerAxisSource;
	axis_stop = onPointerAxisStop;
	axis_discrete = onPointerAxisDiscrete;
#ifdef WL_POINTER_AXIS_VALUE120_SINCE_VERSION
	axis_value120 = onPointerAxis120;
#endif
#ifdef WL_POINTER_AXIS_RELATIVE_DIRECTION_SINCE_VERSION
	axis_relative_direction = onAxisRelativeDirection;
#endif
	frame = onPointerFrame;
}

//------------------------------------------------------------------------
void InputHandler::initialize ()
{
	wl_seat* seat = RunLoop::getClientContext ().getSeat ();
	if (seat == nullptr)
		return;

	// FIXME: this property might change when seat capabilities change
	if (RunLoop::hasPointerInput ())
	{
		pointer = wl_seat_get_pointer (seat);
		if (pointer)
			wl_pointer_add_listener (pointer, this, this);
	}

	initialized = true;
}

//------------------------------------------------------------------------
void InputHandler::terminate ()
{
	if (pointer)
		wl_pointer_destroy (pointer);
	pointer = nullptr;

	initialized = false;
}

//------------------------------------------------------------------------
void InputHandler::addWindow (ChildWindow* window)
{
	windows.push_back (window);
	if (!initialized)
		initialize ();
}

//------------------------------------------------------------------------
void InputHandler::removeWindow (ChildWindow* window)
{
	windows.erase (std::remove (windows.begin (), windows.end (), window));
	if (windows.empty ())
		terminate ();
}

//------------------------------------------------------------------------
void InputHandler::onPointerEnter (void* data, wl_pointer* pointer, uint32_t serial,
								   wl_surface* surface, wl_fixed_t x, wl_fixed_t y)
{
	InputHandler* This = static_cast<InputHandler*> (data);

	This->pointerEvent.eventMask |= kPointerEnter;
	This->pointerEvent.x = x;
	This->pointerEvent.y = y;
	This->pointerEvent.focus = surface;

	// TODO: remember the serial
}

//------------------------------------------------------------------------
void InputHandler::onPointerLeave (void* data, wl_pointer* pointer, uint32_t serial,
								   wl_surface* surface)
{
	InputHandler* This = static_cast<InputHandler*> (data);

	This->pointerEvent.eventMask |= kPointerLeave;
	This->pointerEvent.previousFocus = surface;
	This->pointerEvent.focus = nullptr;
	This->pointerEvent.buttonState = 0;
}

//------------------------------------------------------------------------
void InputHandler::onPointerMotion (void* data, wl_pointer* pointer, uint32_t time, wl_fixed_t x,
									wl_fixed_t y)
{
	InputHandler* This = static_cast<InputHandler*> (data);

	This->pointerEvent.eventMask |= kPointerMotion;
	This->pointerEvent.time = time;
	This->pointerEvent.x = x;
	This->pointerEvent.y = y;
}

//------------------------------------------------------------------------
void InputHandler::onPointerButton (void* data, wl_pointer* pointer, uint32_t serial, uint32_t time,
									uint32_t button, uint32_t state)
{
	InputHandler* This = static_cast<InputHandler*> (data);

	This->pointerEvent.eventMask |= kPointerButton;
	This->pointerEvent.time = time;
	This->pointerEvent.button = button;
	This->pointerEvent.state = state;

	MouseButton flag = MouseButton::None;
	switch (button)
	{
		case BTN_LEFT:
			flag = MouseButton::Left;
			break;
		case BTN_MIDDLE:
			flag = MouseButton::Middle;
			break;
		case BTN_RIGHT:
			flag = MouseButton::Right;
			break;
	}
	if (state == WL_POINTER_BUTTON_STATE_PRESSED)
	{
		This->pointerEvent.serial = serial;
		This->pointerEvent.buttonState |= (uint32_t)flag;
	}
	else
		This->pointerEvent.buttonState &= ~(uint32_t)flag;
}

//------------------------------------------------------------------------
void InputHandler::onPointerAxis (void* data, wl_pointer* wl_pointer, uint32_t time, uint32_t axis,
								  wl_fixed_t value)
{
	/**
	 *	axis: 
	 *	WL_POINTER_AXIS_HORIZONTAL_SCROLL - horizontal scrolling
	 *	WL_POINTER_AXIS_VERTICAL_SCROLL - vertical scrolling
	 */

	InputHandler* This = static_cast<InputHandler*> (data);

	if (axis >= 2)
		return;

	This->pointerEvent.eventMask |= kPointerAxis;
	This->pointerEvent.time = time;
	This->pointerEvent.axes[axis].value += value;
	This->pointerEvent.axes[axis].valid = true;
}

//------------------------------------------------------------------------
void InputHandler::onPointerAxisSource (void* data, wl_pointer* pointer, uint32_t axisSource)
{
	InputHandler* This = static_cast<InputHandler*> (data);

	/**
	 *	axisSource: 
	 * 	WL_POINTER_AXIS_SOURCE_WHEEL - a scroll wheel
	 * 	WL_POINTER_AXIS_SOURCE_FINGER - a finger on a touchpad
	 * 	WL_POINTER_AXIS_SOURCE_CONTINUOUS - ?
	 * 	WL_POINTER_AXIS_SOURCE_WHEEL_TILT - tilting a rocker to the side
	 */

	This->pointerEvent.eventMask |= kPointerAxisSource;
	This->pointerEvent.axisSource = axisSource;
}

//------------------------------------------------------------------------
void InputHandler::onPointerAxisStop (void* data, wl_pointer* pointer, uint32_t time, uint32_t axis)
{
	InputHandler* This = static_cast<InputHandler*> (data);

	This->pointerEvent.eventMask |= kPointerAxisStop;
	This->pointerEvent.time = time;
	This->pointerEvent.axes[axis].valid = true;
}

//------------------------------------------------------------------------
void InputHandler::onPointerAxisDiscrete (void* data, wl_pointer* pointer, uint32_t axis,
										  int32_t discrete)
{
	InputHandler* This = static_cast<InputHandler*> (data);

	This->pointerEvent.eventMask |= kPointerAxisDiscrete;

	onPointerAxis120 (data, pointer, axis, discrete * kWheelClick);
}

//------------------------------------------------------------------------
void InputHandler::onPointerAxis120 (void* data, wl_pointer* pointer, uint32_t axis,
									 int32_t discrete)
{
	InputHandler* This = static_cast<InputHandler*> (data);
	if (axis >= 2)
		return;

	This->pointerEvent.eventMask |= kPointerAxis120;
	This->pointerEvent.axes[axis].discrete += discrete;
	This->pointerEvent.axes[axis].valid = true;
}

//------------------------------------------------------------------------
void InputHandler::onAxisRelativeDirection (void* data, wl_pointer* wl_pointer, uint32_t axis,
											uint32_t direction)
{
	InputHandler* This = static_cast<InputHandler*> (data);
	if (axis >= 2)
		return;

	This->pointerEvent.eventMask |= kPointerAxisRelativeDirection;
	This->pointerEvent.axes[axis].direction = direction;
	This->pointerEvent.axes[axis].valid = true;
}

//------------------------------------------------------------------------
void InputHandler::onPointerFrame (void* data, wl_pointer* pointer)
{
	// TODO: maybe collect events first and dispatch them later to avoid delays inside Wayland
	// dispatch maybe move input handling to a separate queue see
	// https://gitlab.freedesktop.org/wayland/wayland/-/issues/159
	//     https://bugreports.qt.io/browse/QTBUG-66997
	//     https://bugs.kde.org/show_bug.cgi?id=392376
	//     https://gitlab.gnome.org/GNOME/mutter/-/issues/2234

	InputHandler* This = static_cast<InputHandler*> (data);

	PointerEvent event (This->pointerEvent);

	This->pointerEvent.eventMask = 0;
	This->pointerEvent.previousFocus = nullptr;
	for (int i = 0; i < 2; i++)
	{
		This->pointerEvent.axes[i].valid = false;
		This->pointerEvent.axes[i].value = 0;
		This->pointerEvent.axes[i].discrete = 0;
	}
	This->pointerEvent.time = 0;

	This->dispatch (event);
}

//------------------------------------------------------------------------
void InputHandler::dispatch (const PointerEvent& event)
{
	CPoint position (wl_fixed_to_int (event.x), wl_fixed_to_int (event.y));

	for (ChildWindow* window : windows)
	{
		IPlatformFrameCallback* frame = window->getFrame ();
		if (frame == nullptr)
			continue;

		// TODO: wheel, axis, enter, leave, ...

		if ((event.eventMask & InputHandler::kPointerEnter) &&
		event.focus == window->getSurface ())
		{
			MouseEnterEvent enterEvent;
			enterEvent.mousePosition = position;
			enterEvent.buttonState.add ((MouseButton)event.buttonState);
			enterEvent.timestamp = event.time;

			frame->platformOnEvent (enterEvent);
		}

		// FIXME: when dragging / manipulating a control, we might also need to forward mouse move
		// events to a surface that is not currently in focus
		if ((event.eventMask & InputHandler::kPointerButton) &&
			(window->getSurface () == event.focus || window->getSurface () == event.previousFocus))
		{
			if (event.state == WL_POINTER_BUTTON_STATE_RELEASED)
			{
				MouseUpEvent upEvent;
				upEvent.mousePosition = position;
				upEvent.buttonState.add ((MouseButton)event.buttonState);
				upEvent.timestamp = event.time;
				frame->platformOnEvent (upEvent);
			}
			else
			{
				MouseDownEvent downEvent;
				downEvent.mousePosition = position;
				downEvent.buttonState.add ((MouseButton)event.buttonState);
				downEvent.timestamp = event.time;
				frame->platformOnEvent (downEvent);
			}
		}

		if ((event.eventMask & InputHandler::kPointerMotion) &&
			event.focus == window->getSurface ())
		{
			MouseMoveEvent moveEvent;
			moveEvent.mousePosition = position;
			moveEvent.buttonState.add ((MouseButton)event.buttonState);
			moveEvent.timestamp = event.time;
			frame->platformOnEvent (moveEvent);
		}

		if ((event.eventMask & InputHandler::kPointerLeave) &&
			event.previousFocus == window->getSurface ())
		{
			MouseExitEvent exitEvent;
			exitEvent.mousePosition = position;
			exitEvent.buttonState.add ((MouseButton)event.buttonState);
			exitEvent.timestamp = event.time;

			frame->platformOnEvent (exitEvent);
		}

		if ((event.eventMask & InputHandler::kPointerAxis) &&
			event.focus == window->getSurface ())
		{
			if ((event.axisSource == WL_POINTER_AXIS_SOURCE_WHEEL) && 
				(event.eventMask & InputHandler::kPointerAxis120))
			{
				MouseWheelEvent wheelEvent;
				wheelEvent.mousePosition = position;
				wheelEvent.deltaX = event.axes[0].discrete / kWheelClick;
				wheelEvent.deltaY = event.axes[1].discrete / kWheelClick;
				wheelEvent.timestamp = event.time;

				frame->platformOnEvent (wheelEvent);
			}
		}
	}
}

//------------------------------------------------------------------------
// KeyboardHandler
//------------------------------------------------------------------------
struct KeyboardHandler : public wl_keyboard_listener
{
	static KeyboardHandler& instance ()
	{
		static KeyboardHandler keyboardHandler;
		return keyboardHandler;
	}

	void addWindow (ChildWindow* window);
	void removeWindow (ChildWindow* window);

	// wl_keyboard_listener
	static void onKeyboardKeymap (void* data, struct wl_keyboard* wl_keyboard, uint32_t format,
								  int32_t fd, uint32_t size);
	static void onKeyboardEnter (void* data, struct wl_keyboard* wl_keyboard, uint32_t serial,
								 struct wl_surface* surface, struct wl_array* keys);
	static void onKeyboardLeave (void* data, struct wl_keyboard* wl_keyboard, uint32_t serial,
								 struct wl_surface* surface);
	static void onKeyboardKey (void* data, struct wl_keyboard* wl_keyboard, uint32_t serial,
							   uint32_t time, uint32_t key, uint32_t state);
	static void onKeyboardModifiers (void* data, struct wl_keyboard* wl_keyboard, uint32_t serial,
									 uint32_t mods_depressed, uint32_t mods_latched,
									 uint32_t mods_locked, uint32_t group);
	static void onKeyboardRepeatInfo (void* data, struct wl_keyboard* wl_keyboard, int32_t rate,
									  int32_t delay);

	struct KeyboardInputEvent
	{
		uint32_t time {0};
		uint32_t state {0};
		uint32_t key {0};
		uint32_t eventMask {0};
		uint32_t virt {0};

		wl_surface* focus {nullptr};
		wl_surface* previousFocus {nullptr};
	};

	wl_keyboard* keyboard = nullptr;
	KeyboardInputEvent keyboardEvent;
	bool initialized;
	std::vector<ChildWindow*> windows;

	enum KeyboardEventMask
	{
		kKeyboardKeymap = 1 << 0,
		kKeyboardEnter = 1 << 1,
		kKeyboardLeave = 1 << 2,
		kKeyboardKey = 1 << 3,
		kKeyboardModifiers = 1 << 4,
		kKeyboardRepeatInfo = 1 << 5,
	};

	KeyboardHandler ();
	void initialize ();
	void terminate ();
	void dispatch (const KeyboardInputEvent& event);

	struct xkb_keymap* skeymap = nullptr;
	struct xkb_context* xkb_context = nullptr;
	struct xkb_state* xkb_state = nullptr;
};

//------------------------------------------------------------------------

} // namespace Wayland
} // namespace VSTGUI

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Wayland {

//------------------------------------------------------------------------
// ChildWindow
//------------------------------------------------------------------------
ChildWindow::ChildWindow (IWaylandFrame* waylandFrame, CPoint size)
: size (size)
, waylandFrame (shared (waylandFrame))
, data (MAP_FAILED)
, buffer (nullptr)
, pool (nullptr)
, surface (nullptr)
, subSurface (nullptr)
, allocatedSize (0)
, byteSize (0)
, fd (-1)
{
	enter = onEnter;
	leave = onLeave;
	preferred_buffer_scale = onPreferredBufferScale;
	preferred_buffer_transform = onPreferredBufferTransform;
}

//------------------------------------------------------------------------
ChildWindow::~ChildWindow () noexcept
{
	destroyBuffer ();
	terminate ();
}

//------------------------------------------------------------------------
void ChildWindow::updateScaleFactor (ChildWindow* self, int32_t factor)
{
	if (!self)
		return;

	self->getFrame ()->platformScaleFactorChanged (factor);
	wl_surface_set_buffer_scale (self->surface, factor);
	wl_surface_commit (self->surface);
}

//------------------------------------------------------------------------
void ChildWindow::onEnter (void *data, wl_surface *surface, wl_output *output)
{
	auto self = reinterpret_cast<ChildWindow*> (data);
	if (surface != self->surface)
		return;

#if WL_SURFACE_PREFERRED_BUFFER_SCALE_SINCE_VERSION
	// 'preferred_buffer_scale' is handling the scale factor
#else
	const auto& client = RunLoop::getClientContext ();
	const auto count = client.countOutputs ();
	for (auto i = 0; i < count; ++i)
	{
		const auto& wlOutput = client.getOutput (i);
		if (wlOutput.handle != output)
			continue;

		updateScaleFactor (self, wlOutput.scaleFactor);
	}
#endif
}

//------------------------------------------------------------------------
void ChildWindow::onLeave (void *data, wl_surface *wl_surface, wl_output *output)
{

}

//------------------------------------------------------------------------
void ChildWindow::onPreferredBufferScale (void *data, wl_surface *wl_surface, int32_t factor)
{
	auto self = reinterpret_cast<ChildWindow*> (data);
	if (wl_surface != self->surface)
		return;

	updateScaleFactor (self, factor);
}

//------------------------------------------------------------------------
void ChildWindow::onPreferredBufferTransform (void *data, wl_surface *wl_surface, uint32_t transform)
{

}
//------------------------------------------------------------------------
void ChildWindow::setFrame (IPlatformFrameCallback* frame) { frameCallback = frame; }

//------------------------------------------------------------------------
IPlatformFrameCallback* ChildWindow::getFrame () const { return frameCallback; }

//------------------------------------------------------------------------
void ChildWindow::commit (const CRect& rect)
{
	if (surface == nullptr)
		return;

	wl_surface_damage_buffer (surface, rect.left, rect.top, rect.getWidth (), rect.getHeight ());
	wl_surface_attach (surface, buffer, 0, 0);
	wl_surface_commit (surface);

	RunLoop::flush ();
}

//------------------------------------------------------------------------
void ChildWindow::setSize (const CRect& rect)
{
	if (size != rect.getSize ())
	{
		if (buffer != nullptr)
			wl_buffer_destroy (buffer);
		buffer = nullptr;
		// TODO: Is setting data here to MAP_FAILED right?
		// We don't call unmap here!
		// This results in the Live Editor not redrawing when 'close' it.
		//data = MAP_FAILED;
	}
	size = rect.getSize ();
}

//------------------------------------------------------------------------
const CPoint& ChildWindow::getSize () const { return size; }

//------------------------------------------------------------------------
void* ChildWindow::getBuffer () const
{
	if (buffer == nullptr)
		const_cast<ChildWindow*> (this)->createBuffer ();
	return (data != MAP_FAILED) ? data : nullptr;
}

//------------------------------------------------------------------------
int ChildWindow::getBufferStride () const
{
	return cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, size.x);
}

//------------------------------------------------------------------------
wl_surface* ChildWindow::getSurface () const { return surface; }

//------------------------------------------------------------------------
void ChildWindow::createBuffer ()
{
	if (!initialized)
		initialize ();
	if (!initialized)
		return;

	wl_shm* shm = RunLoop::getClientContext ().getSharedMemory ();
	if (shm == nullptr)
		return;

	int stride = getBufferStride ();
	int byteSize = stride * size.y;

	if (byteSize > allocatedSize)
	{
		int newSize = byteSize * 1.5;

		if (pool)
			wl_shm_pool_destroy (pool);
		pool = nullptr;

		if (fd >= 0)
			::close (fd);
		fd = -1;

		if (data != MAP_FAILED)
		{
			::munmap (data, allocatedSize);
			data = MAP_FAILED;
		}

		for (int i = 0; i < 100 && fd < 0; i++)
		{
			UTF8String name = "/vstgui_wl_buffer-" + std::to_string (::rand ());
			fd = ::shm_open (name, O_RDWR | O_CREAT | O_EXCL, 0600);
			if (fd >= 0)
			{
				::shm_unlink (name);
				break;
			}
		}

		int result = 0;
		do
		{
			result = ::ftruncate (fd, newSize);
		} while (result < 0 && errno == EINTR);

		if (result < 0)
		{
			::close (fd);
			return;
		}

		data = ::mmap (NULL, newSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (data == MAP_FAILED)
		{
			::close (fd);
			return;
		}

		allocatedSize = newSize;

		pool = wl_shm_create_pool (shm, fd, allocatedSize);
	}

	buffer = wl_shm_pool_create_buffer (pool, 0, size.x, size.y, stride, WL_SHM_FORMAT_ARGB8888);
	if (surface && buffer)
		wl_surface_attach (surface, buffer, 0, 0);

	// TODO add a buffer listener, check if a buffer has been released before rendering into the
	// same buffer again use multiple buffers as a swapchain
}

//------------------------------------------------------------------------
void ChildWindow::destroyBuffer ()
{
	if (buffer)
		wl_buffer_destroy (buffer);
	buffer = nullptr;

	if (pool)
		wl_shm_pool_destroy (pool);
	pool = nullptr;

	if (data != MAP_FAILED)
		::munmap (data, byteSize);
	data = MAP_FAILED;
}

//------------------------------------------------------------------------
void ChildWindow::initialize ()
{
	wl_display* display = RunLoop::getDisplay ();
	wl_compositor* compositor = RunLoop::getClientContext ().getCompositor ();
	wl_subcompositor* subCompositor = RunLoop::getClientContext ().getSubCompositor ();
	wl_seat* seat = RunLoop::getClientContext ().getSeat ();

	// FIXME: better wait for all required globals to be available before creating any ChildWindow's
	if (display == nullptr || compositor == nullptr || subCompositor == nullptr || seat == nullptr)
		return;

	wl_surface* parent = waylandFrame->getWaylandSurface (display);
	if (parent == nullptr)
		return;

	surface = wl_compositor_create_surface (compositor);
	if (surface == nullptr)
		return;

	wl_surface_add_listener (surface, this, this);

	subSurface = wl_subcompositor_get_subsurface (subCompositor, surface, parent);
	if (subSurface == nullptr)
	{
		wl_surface_destroy (surface);
		surface = nullptr;
		return;
	}

	wl_subsurface_set_desync (subSurface);

	InputHandler::instance ().addWindow (this);
	KeyboardHandler::instance ().addWindow (this);

	initialized = true;
}

//------------------------------------------------------------------------
void ChildWindow::terminate ()
{
	if (!initialized)
		return;

	KeyboardHandler::instance ().removeWindow (this);
	InputHandler::instance ().removeWindow (this);

	if (subSurface)
		wl_subsurface_destroy (subSurface);
	subSurface = nullptr;

	if (surface)
		wl_surface_destroy (surface);
	surface = nullptr;

	RunLoop::flush ();

	initialized = false;
}

//------------------------------------------------------------------------
// KeyboardHandler
//------------------------------------------------------------------------
KeyboardHandler::KeyboardHandler () : keyboard (nullptr), initialized (false)
{
	keymap = onKeyboardKeymap;
	enter = onKeyboardEnter;
	leave = onKeyboardLeave;
	key = onKeyboardKey;
	modifiers = onKeyboardModifiers;
	repeat_info = onKeyboardRepeatInfo;
}

//------------------------------------------------------------------------
void KeyboardHandler::initialize ()
{
	wl_seat* seat = RunLoop::getClientContext ().getSeat ();
	if (seat == nullptr)
		return;

	// FIXME: this property might change when seat capabilities change
	if (RunLoop::hasKeyboardInput ())
	{
		keyboard = wl_seat_get_keyboard (seat);
		if (keyboard)
			wl_keyboard_add_listener (keyboard, this, this);
	}

	initialized = true;
}

//------------------------------------------------------------------------
void KeyboardHandler::terminate ()
{
	if (keyboard)
		wl_keyboard_destroy (keyboard);
	keyboard = nullptr;

	initialized = false;
}

//------------------------------------------------------------------------
void KeyboardHandler::addWindow (ChildWindow* window)
{
	windows.push_back (window);
	if (!initialized)
		initialize ();
}

//------------------------------------------------------------------------
void KeyboardHandler::removeWindow (ChildWindow* window)
{
	windows.erase (std::remove (windows.begin (), windows.end (), window));
	if (windows.empty ())
		terminate ();
}

//------------------------------------------------------------------------
void KeyboardHandler::dispatch (const KeyboardInputEvent& event)
{
	for (ChildWindow* window : windows)
	{
		IPlatformFrameCallback* frame = window->getFrame ();
		if (frame == nullptr)
			continue;

		switch (event.eventMask)
		{
			case KeyboardEventMask::kKeyboardKey:
			{
				VirtMap::const_iterator it;
				bool ksymMapped = false;
				if (!ksymMapped && false) // && keyEvent.modifiers.has (ModifierKey::Shift))
				{
					it = shiftKeyMap.find (event.virt);
					ksymMapped = it != shiftKeyMap.end ();
				}
				if (!ksymMapped)
				{
					it = keyMap.find (event.virt);
					ksymMapped = it != keyMap.end ();
				}

				// KeyboardInputEvent -> VSTGUI::Event
				VSTGUI::KeyboardEvent kev;
				kev.character = event.key;
				if (ksymMapped)
					kev.virt = it->second;

				frame->platformOnEvent (kev);
				break;
			}

			case KeyboardEventMask::kKeyboardEnter:
			case KeyboardEventMask::kKeyboardLeave:
			{
				bool activate = event.eventMask == KeyboardEventMask::kKeyboardEnter;
				frame->platformOnActivate (activate);
				break;
			}

			default:
				break;
		}
	}
}

//------------------------------------------------------------------------
void KeyboardHandler::onKeyboardKeymap (void* data, struct wl_keyboard* wl_keyboard,
										uint32_t format, int32_t fd, uint32_t size)
{
	auto* This = static_cast<KeyboardHandler*> (data);

	This->xkb_context = xkb_context_new (XKB_CONTEXT_NO_FLAGS);
	char* keymap_string = (char*)mmap (NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	xkb_keymap_unref (This->skeymap);
	This->skeymap = xkb_keymap_new_from_string (
		This->xkb_context, keymap_string, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
	munmap (keymap_string, size);
	close (fd);
	xkb_state_unref (This->xkb_state);
	This->xkb_state = xkb_state_new (This->skeymap);

	This->keyboardEvent.eventMask = KeyboardEventMask::kKeyboardKeymap;
}

//------------------------------------------------------------------------
void KeyboardHandler::onKeyboardEnter (void* data, struct wl_keyboard* wl_keyboard, uint32_t serial,
									   struct wl_surface* surface, struct wl_array* keys)
{
	auto* This = static_cast<KeyboardHandler*> (data);

	KeyboardInputEvent event;
	event.eventMask = KeyboardEventMask::kKeyboardEnter;

	This->dispatch (event);
}

//------------------------------------------------------------------------
void KeyboardHandler::onKeyboardLeave (void* data, struct wl_keyboard* wl_keyboard, uint32_t serial,
									   struct wl_surface* surface)
{
	KeyboardInputEvent event;
	event.eventMask = KeyboardEventMask::kKeyboardLeave;

	auto* This = static_cast<KeyboardHandler*> (data);
	This->dispatch (event);
}

//------------------------------------------------------------------------
static char running = 1;
void KeyboardHandler::onKeyboardKey (void* data, struct wl_keyboard* wl_keyboard, uint32_t serial,
									 uint32_t time, uint32_t key, uint32_t state)
{
	auto* This = static_cast<KeyboardHandler*> (data);

	if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
	{
		xkb_keysym_t keysym = xkb_state_key_get_one_sym (This->xkb_state, key + 8);
		// xkb_state_update_key (xkb_state, event.detail, isKeyDown ? XKB_KEY_DOWN : XKB_KEY_UP);
		uint32_t utf32 = xkb_keysym_to_utf32 (keysym);

		if (utf32)
		{
			if (utf32 >= 0x21 && utf32 <= 0x7E)
			{
				printf ("the key %c was pressed\n", (char)utf32);
				if (utf32 == 'q')
					running = 0;
			}
			else
			{
				printf ("the key U+%04X was pressed\n", utf32);
			}
		}
		else
		{
			char name[64];
			xkb_keysym_get_name (keysym, name, 64);
			printf ("the key %s was pressed\n", name);
		}
		This->keyboardEvent.eventMask = KeyboardEventMask::kKeyboardKey;
		This->keyboardEvent.key = utf32;
		This->keyboardEvent.virt = keysym;

		This->dispatch (This->keyboardEvent);
	}
	else if (state == WL_KEYBOARD_KEY_STATE_RELEASED)
	{
		// todo: do we need this???
	}
}

//------------------------------------------------------------------------
void KeyboardHandler::onKeyboardModifiers (void* data, struct wl_keyboard* wl_keyboard,
										   uint32_t serial, uint32_t mods_depressed,
										   uint32_t mods_latched, uint32_t mods_locked,
										   uint32_t group)
{
	auto* This = static_cast<KeyboardHandler*> (data);
	// todo: what does this do??? Is it necessary? We assume it does! ;)
	xkb_state_update_mask (This->xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
}

//------------------------------------------------------------------------
void KeyboardHandler::onKeyboardRepeatInfo (void* data, struct wl_keyboard* wl_keyboard,
											int32_t rate, int32_t delay)
{
	auto* This = static_cast<KeyboardHandler*> (data);
	bool x = true;
}

//------------------------------------------------------------------------
} // Wayland
} // VSTGUI
