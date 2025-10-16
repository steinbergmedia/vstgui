// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE
// Originally written and contributed to VSTGUI by PreSonus Software Ltd.

#include "waylandframe.h"
// #include "waylanddragging.h"
#include "waylandutils.h"
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
#include "linuxfactory.h"
#include "cairographicscontext.h"
#include "waylandplatform.h"
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <cairo/cairo.h>

#ifdef None
#undef None
#endif

#include "../../events.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Wayland {

//------------------------------------------------------------------------
struct RedrawTimerHandler : ITimerHandler,
							NonAtomicReferenceCounted
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
	DrawHandler (ChildWindow& window)
	: childWindow (window), windowSurface (nullptr), device (nullptr)
	{
		onSizeChanged (window.getSize ());
	}

	void onSizeChanged (const CPoint& size)
	{
		drawContext = nullptr;
		device.reset ();
		windowSurface.reset ();

		void* buffer = childWindow.getBuffer ();
		if (buffer == nullptr)
			return;

		auto s = cairo_image_surface_create_for_data (
			static_cast<unsigned char*> (buffer), CAIRO_FORMAT_ARGB32, childWindow.getSize ().x,
			childWindow.getSize ().y, childWindow.getBufferStride ());
		if (cairo_surface_status (s) != CAIRO_STATUS_SUCCESS)
			return;

		windowSurface.assign (s);
		device =
			getPlatformFactory ().asLinuxFactory ()->getCairoGraphicsDeviceFactory ().addDevice (
				cairo_surface_get_device (s));
		auto cairoDevice = std::static_pointer_cast<CairoGraphicsDevice> (device);
		drawContext = std::make_shared<CairoGraphicsDeviceContext> (*cairoDevice, windowSurface);
	}

	bool draw (const CInvalidRectList& dirtyRects, IPlatformFrameCallback* frame)
	{
		if (drawContext == nullptr)
			onSizeChanged (childWindow.getSize ());

		if (drawContext == nullptr)
			return false;

		CRect copyRect;
		drawContext->beginDraw ();
		frame->platformDrawRects (drawContext, 1, dirtyRects.data ());
		for (auto rect : dirtyRects)
		{
			if (copyRect.isEmpty ())
				copyRect = rect;
			else
				copyRect.unite (rect);
		}
		drawContext->endDraw ();
		cairo_surface_flush (windowSurface);

		childWindow.commit (copyRect);

		return true;
	}

private:
	ChildWindow& childWindow;
	Cairo::SurfaceHandle windowSurface;
	PlatformGraphicsDevicePtr device;
	std::shared_ptr<CairoGraphicsDeviceContext> drawContext;
};

//------------------------------------------------------------------------
struct Frame::Impl
{
	using RectList = CInvalidRectList;

	ChildWindow window;
	DrawHandler drawHandler;
	// TODO: DoubleClickDetector doubleClickDetector;
	IPlatformFrameCallback* frame;
	std::unique_ptr<GenericOptionMenuTheme> genericOptionMenuTheme;
	SharedPointer<RedrawTimerHandler> redrawTimer;
	RectList dirtyRects;
	CCursorType currentCursor {kCursorDefault};
	uint32_t pointerGrabed {0};
	// TODO: WaylandDragAndDropHandler dndHandler;

	//------------------------------------------------------------------------
	Impl (IWaylandFrame* waylandFrame, CPoint size, IPlatformFrameCallback* frame)
	: window (waylandFrame, size)
	, drawHandler (window)
	, frame (frame) //, dndHandler (&window, frame)
	{
		window.setFrame (frame);
	}

	//------------------------------------------------------------------------
	~Impl () noexcept {}

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
		// TODO
	}

	//------------------------------------------------------------------------
	void redraw ()
	{
		if (drawHandler.draw (dirtyRects, frame))
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
};

//------------------------------------------------------------------------
Frame::Frame (IPlatformFrameCallback* frame, const CRect& size, IPlatformFrameConfig* config)
: IPlatformFrame (frame)
{
	auto cfg = dynamic_cast<FrameConfig*> (config);
	if (cfg && cfg->runLoop)
	{
		if (auto f = getPlatformFactory ().asLinuxFactory ())
		{
			if (f->getRunLoop () == nullptr)
				f->setRunLoop (cfg->runLoop);
		}
		RunLoop::init (cfg->waylandHost);
	}

	impl = std::unique_ptr<Impl> (
		new Impl (cfg ? cfg->waylandFrame : nullptr, {size.getWidth (), size.getHeight ()}, frame));

	frame->platformOnActivate (true);
}

//------------------------------------------------------------------------
Frame::~Frame ()
{
	impl.reset ();
	RunLoop::exit ();
}

//------------------------------------------------------------------------
void Frame::optionMenuPopupStarted () {}

//------------------------------------------------------------------------
void Frame::optionMenuPopupStopped () {}

//------------------------------------------------------------------------
bool Frame::getGlobalPosition (CPoint& pos) const { return false; }

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
	// TODO
	return false;
}

//------------------------------------------------------------------------
bool Frame::getCurrentMouseButtons (CButtonState& buttons) const
{
	// TODO
	return false;
}

//------------------------------------------------------------------------
bool Frame::getCurrentModifiers (Modifiers& modifiers) const
{
	// TODO
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
#warning TODO: Implementation
	return nullptr;
	// return reinterpret_cast<void*> (getX11WindowID ());
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
	auto optionMenu =
		makeOwned<GenericOptionMenu> (cFrame, MouseEventButtonState (MouseButton::Left), theme);
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
PlatformType Frame::getPlatformType () const { return PlatformType::kWaylandSurfaceID; }

//------------------------------------------------------------------------
Optional<UTF8String> Frame::convertCurrentKeyEventToText ()
{
	// TODO: return RunLoop::instance ().convertCurrentKeyEventToText ();
	return Optional<UTF8String> ();
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
} // Wayland
} // VSTGUI
