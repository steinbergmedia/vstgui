// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "gdkframe.h"
#include "../iplatformtextedit.h"
#include "../iplatformoptionmenu.h"
#include "../iplatformopenglview.h"
#include "../iplatformviewlayer.h"
#include "../iplatformtimer.h"
#include "cairobitmap.h"
#include "cairocontext.h"
#include <chrono>
#include <gtkmm.h>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace GDK {

//------------------------------------------------------------------------
Frame::CreateIResourceInputStreamFunc Frame::createResourceInputStreamFunc =
	[](const CResourceDescription& desc) { return nullptr; };

//------------------------------------------------------------------------
struct Frame::Impl
{
	Glib::RefPtr<Gdk::Window> window;
};

//------------------------------------------------------------------------
Frame::Frame (IPlatformFrameCallback* frame,
			  const CRect& size,
			  void* parent,
			  IPlatformFrameConfig* config)
	: IPlatformFrame (frame)
{
	impl = std::unique_ptr<Impl> (new Impl);

	GdkWindowAttr attributes{};
	attributes.x = size.left;
	attributes.y = size.top;
	attributes.width = size.getWidth ();
	attributes.height = size.getHeight ();
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = nullptr;
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.cursor = nullptr;
	attributes.wmclass_name = nullptr;
	attributes.wmclass_class = nullptr;
	attributes.override_redirect = false;
	attributes.type_hint = GDK_WINDOW_TYPE_HINT_NORMAL;

	impl->window =
		Gdk::Window::create (Glib::wrap (static_cast<GdkWindow*> (parent)), &attributes, 0);
}

//------------------------------------------------------------------------
Frame::~Frame () noexcept = default;

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
} // GDK

//------------------------------------------------------------------------
IPlatformFrame* IPlatformFrame::createPlatformFrame (IPlatformFrameCallback* frame,
													 const CRect& size,
													 void* parent,
													 PlatformType parentType,
													 IPlatformFrameConfig* config)
{
	return new GDK::Frame (frame, size, parent, config);
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
	return nullptr;
}

//------------------------------------------------------------------------
} // VSTGUI
