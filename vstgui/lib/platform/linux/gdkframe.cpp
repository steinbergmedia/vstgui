// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "gdkframe.h"
#include "../iplatformtextedit.h"
#include "../iplatformoptionmenu.h"
#include "../iplatformopenglview.h"
#include "../iplatformviewlayer.h"
#include "cairobitmap.h"
#include "cairocontext.h"
#include <chrono>
#include <gtkmm.h>
#include <cairomm/cairomm.h>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace GDK {

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
	attributes.event_mask = GDK_ALL_EVENTS_MASK;

	auto parentWindow = Glib::wrap (gdkWindowParent);
	impl->window =
		Gdk::Window::create (parentWindow, &attributes, GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL);
	if (!impl->window)
		return false;
	Gdk::RGBA color;
	color.set_rgba (1., 0., 0.);
	impl->window->set_background (color);

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
	auto region = ::Cairo::RefPtr< ::Cairo::Region>(new ::Cairo::Region(updateRegion, false));
	auto cairoContext = impl->window->begin_draw_frame (region);

	CRect size;
	getSize (size);

	auto context = owned (new Cairo::Context (size, cairoContext->get_cairo_context ()->cobj ()));

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

	impl->window->end_draw_frame (cairoContext);
}

//------------------------------------------------------------------------
void Frame::handleEvent (void* gdkEvent)
{
	auto ev = reinterpret_cast<GdkEvent*> (gdkEvent);
	switch (ev->type)
	{
		case GDK_EXPOSE:
		{
			drawDirtyRegions ();
			break;
		}
	}
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
