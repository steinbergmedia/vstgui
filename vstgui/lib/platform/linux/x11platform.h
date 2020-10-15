// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../vstguifwd.h"
#include "x11frame.h"
#include <atomic>
#include <memory>

struct xcb_connection_t;	  // forward declaration
struct xcb_key_press_event_t; // forward declaration
struct xcb_button_press_event_t;
struct xcb_motion_notify_event_t;
struct xcb_enter_notify_event_t;
struct xcb_focus_in_event_t;
struct xcb_expose_event_t;
struct xcb_map_notify_event_t;
struct xcb_property_notify_event_t;
struct xcb_selection_notify_event_t;
struct xcb_client_message_event_t;
using xcb_window_t = uint32_t;

//------------------------------------------------------------------------
namespace VSTGUI {
extern void* soHandle; // shared library handle

//------------------------------------------------------------------------
namespace X11 {

class Frame;
class Timer;

//------------------------------------------------------------------------
struct IFrameEventHandler
{
	virtual void onEvent (xcb_map_notify_event_t& event) = 0;
	virtual void onEvent (xcb_key_press_event_t& event) = 0;
	virtual void onEvent (xcb_button_press_event_t& event) = 0;
	virtual void onEvent (xcb_motion_notify_event_t& event) = 0;
	virtual void onEvent (xcb_enter_notify_event_t& event) = 0;
	virtual void onEvent (xcb_focus_in_event_t& event) = 0;
	virtual void onEvent (xcb_expose_event_t& event) = 0;
	virtual void onEvent (xcb_property_notify_event_t& event) = 0;
	virtual void onEvent (xcb_selection_notify_event_t& event) = 0;
	virtual void onEvent (xcb_client_message_event_t& event, xcb_window_t proxyId = 0) = 0;
};

//------------------------------------------------------------------------
class Platform
{
public:
	~Platform ();

	static Platform& getInstance ();
	static uint64_t getCurrentTimeMs ();

	std::string getPath ();

private:
	Platform ();

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
struct RunLoop
{
	static void init (const SharedPointer<IRunLoop>& runLoop);
	static void exit ();
	static const SharedPointer<IRunLoop> get ();

	xcb_connection_t* getXcbConnection () const;

	void registerWindowEventHandler (uint32_t windowId, IFrameEventHandler* handler);
	void unregisterWindowEventHandler (uint32_t windowId);

	uint32_t getCursorID (CCursorType cursor);
	VstKeyCode getCurrentKeyEvent () const;
	Optional<UTF8String> convertCurrentKeyEventToText () const;

	static RunLoop& instance ();

private:
	RunLoop ();
	~RunLoop () noexcept;

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // X11
} // VSTGUI
