// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "x11frame.h"
#include "../../cbuttonstate.h"
#include "../../crect.h"
#include "../../dragging.h"
#include "../../vstkeycode.h"
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

#ifdef None
#	undef None
#endif

//------------------------------------------------------------------------
namespace VSTGUI {
namespace X11 {

//------------------------------------------------------------------------
struct SimpleWindow
{
	SimpleWindow (::Window parentId, CPoint size)
	{
		auto connection = RunLoop::instance ().getXcbConnection ();
		// const xcb_setup_t* setup = xcb_get_setup (connection);
		// xcb_screen_iterator_t iter = xcb_setup_roots_iterator (setup);
		// xcb_screen_t* screen = iter.data;

		const uint32_t valueList[] = {
			XCB_EVENT_MASK_BUTTON_PRESS,   XCB_EVENT_MASK_BUTTON_RELEASE,
			XCB_EVENT_MASK_ENTER_WINDOW,   XCB_EVENT_MASK_LEAVE_WINDOW,
			XCB_EVENT_MASK_POINTER_MOTION, XCB_EVENT_MASK_POINTER_MOTION_HINT,
			XCB_EVENT_MASK_BUTTON_MOTION,  XCB_EVENT_MASK_EXPOSURE,
			XCB_EVENT_MASK_PROPERTY_CHANGE};
		const uint32_t valueMask = XCB_CW_EVENT_MASK;

		xcb_create_window (connection, XCB_COPY_FROM_PARENT, getID (), parentId, 0, 0, size.x,
						   size.y, 10, XCB_WINDOW_CLASS_INPUT_OUTPUT, XCB_COPY_FROM_PARENT,
						   valueMask, valueList);

		static std::string xEmbedInfoStr = "_XEMBED_INFO";
		auto cookie = xcb_intern_atom (connection, 0, xEmbedInfoStr.size (), xEmbedInfoStr.data ());
		auto xEmbedAtom = xcb_intern_atom_reply (connection, cookie, nullptr)->atom;

		// setup XEMBED
		uint32_t data[2] = {1, 0};
		xcb_change_property (connection, XCB_PROP_MODE_REPLACE, getID (), xEmbedAtom, xEmbedAtom,
							 32, 2, data);

		xcb_flush (connection);
	}

	~SimpleWindow () noexcept {}

	xcb_window_t getID () const { return id; }

private:
	xcb_window_t id{xcb_generate_id (RunLoop::instance ().getXcbConnection ())};
};

//------------------------------------------------------------------------
struct Frame::Impl : IFrameEventHandler
{
	SimpleWindow window;
	bool handlingEvents{false};

	Impl (::Window parent, CPoint size) : window (parent, size)
	{
		RunLoop::instance ().registerWindowEventHandler (window.getID (), this);
	}

	~Impl () noexcept { RunLoop::instance ().unregisterWindowEventHandler (window.getID ()); }

	void onEvent (xcb_map_notify_event_t& event) override {}
	void onEvent (xcb_key_press_event_t& event) override {}
	void onEvent (xcb_button_press_event_t& event) override {}
	void onEvent (xcb_motion_notify_event_t& event) override {}
	void onEvent (xcb_enter_notify_event_t& event) override {}
	void onEvent (xcb_focus_in_event_t& event) override {}
	void onEvent (xcb_expose_event_t& event) override {}
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
		//		RunLoop::get ()->registerEventHandler (XConnectionNumber (xDisplay), this);
	}

	impl = std::unique_ptr<Impl> (new Impl (parent, {size.getWidth (), size.getHeight ()}));

	frame->platformOnActivate (true);

#if 0 // DEBUG
	auto id = impl->plug.get_id ();
	std::cout << "PlugID: " << std::hex << id << std::endl;

	Gdk::Event::set_show_events (true);
#endif
}

//------------------------------------------------------------------------
Frame::~Frame ()
{
	if (auto runLoop = RunLoop::get ())
	{
		//		runLoop->unregisterEventHandler (this);
	}
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
	return false;
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
	return false;
}

//------------------------------------------------------------------------
bool Frame::invalidRect (const CRect& rect)
{
	return false;
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
void Frame::onEvent () {}

//------------------------------------------------------------------------
Frame::CreateIResourceInputStreamFunc Frame::createResourceInputStreamFunc =
	[](const CResourceDescription& desc) -> IPlatformResourceInputStream::Ptr {
	if (desc.type != CResourceDescription::kStringType)
		return nullptr;
	auto path = Platform::getInstance ().getPath ();
	path += "/";
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
