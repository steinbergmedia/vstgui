// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "copenglview.h"
#include "platform/iplatformframe.h"
#include "cframe.h"
#include <cassert>

#if VSTGUI_OPENGL_SUPPORT

namespace VSTGUI {

//-----------------------------------------------------------------------------
COpenGLView::COpenGLView (const CRect& size)
: CView (size)
{
}

//-----------------------------------------------------------------------------
COpenGLView::~COpenGLView () noexcept
{
	vstgui_assert (platformOpenGLView == nullptr);
}

//-----------------------------------------------------------------------------
void COpenGLView::updatePlatformOpenGLViewSize ()
{
	if (platformOpenGLView)
	{
		CRect visibleSize (getVisibleViewSize ());
		CPoint offset;
		localToFrame (offset);
		visibleSize.offset (offset.x, offset.y);

		CGraphicsTransform tm;
		tm.scale (scaleFactor, scaleFactor);
		tm.transform (visibleSize);

		platformOpenGLView->viewSizeChanged (visibleSize);
		platformOpenGLView->invalidRect (getViewSize ());
	}
}

//-----------------------------------------------------------------------------
bool COpenGLView::createPlatformOpenGLView ()
{
	vstgui_assert (platformOpenGLView == nullptr);
	IPlatformFrame* platformFrame = getFrame ()->getPlatformFrame ();
	platformOpenGLView = platformFrame ? platformFrame->createPlatformOpenGLView () : nullptr;
	if (platformOpenGLView)
	{
		if (platformOpenGLView->init (this, getPixelFormat ()))
		{
			platformOpenGLView->makeContextCurrent ();
			updatePlatformOpenGLViewSize ();
			platformOpenGLViewCreated ();
			platformOpenGLViewSizeChanged ();
			getFrame ()->registerScaleFactorChangedListeneer (this);
			return true;
		}
		platformOpenGLView = nullptr;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool COpenGLView::destroyPlatformOpenGLView ()
{
	if (platformOpenGLView)
	{
		getFrame ()->unregisterScaleFactorChangedListeneer (this);
		platformOpenGLViewWillDestroy ();
		platformOpenGLView->remove ();
		platformOpenGLView = nullptr;
		return true;
	}
	return false;
}

// CView
//-----------------------------------------------------------------------------
void COpenGLView::setViewSize (const CRect& rect, bool invalid)
{
	CView::setViewSize (rect, invalid);
	updatePlatformOpenGLViewSize ();
	if (platformOpenGLView)
	{
		platformOpenGLView->lockContext ();
		platformOpenGLView->makeContextCurrent ();
		platformOpenGLViewSizeChanged ();
		platformOpenGLView->unlockContext ();
	}
}

//-----------------------------------------------------------------------------
void COpenGLView::parentSizeChanged ()
{
	CView::parentSizeChanged ();
	updatePlatformOpenGLViewSize ();
	if (platformOpenGLView)
	{
		platformOpenGLView->lockContext ();
		platformOpenGLView->makeContextCurrent ();
		platformOpenGLViewSizeChanged ();
		platformOpenGLView->unlockContext ();
	}
}

//-----------------------------------------------------------------------------
void COpenGLView::reshape ()
{
	if (platformOpenGLView)
	{
		platformOpenGLView->lockContext ();
		platformOpenGLView->makeContextCurrent ();
		platformOpenGLViewSizeChanged ();
		platformOpenGLView->unlockContext ();
	}
}

//-----------------------------------------------------------------------------
bool COpenGLView::attached (CView* parent)
{
	if (CView::attached (parent))
	{
		return createPlatformOpenGLView ();
	}
	return false;
}

//-----------------------------------------------------------------------------
bool COpenGLView::removed (CView* parent)
{
	destroyPlatformOpenGLView ();
	return CView::removed (parent);
}

//-----------------------------------------------------------------------------
void COpenGLView::setVisible (bool state)
{
	if (state != isVisible ())
	{
		CView::setVisible (state);
		if (isAttached ())
		{
			if (state && platformOpenGLView)
			{
				createPlatformOpenGLView ();
			}
			else if (state == false && platformOpenGLView != nullptr)
			{
				destroyPlatformOpenGLView ();
			}
		}
	}
}

//-----------------------------------------------------------------------------
void COpenGLView::invalidRect (const CRect& rect)
{
	if (platformOpenGLView)
	{
		CRect r (rect);
		r.offset (-getViewSize ().left, -getViewSize ().top);

		CGraphicsTransform tm;
		tm.scale (scaleFactor, scaleFactor);
		tm.transform (r);

		platformOpenGLView->invalidRect (r);
	}
	else
	{
		CView::invalidRect (rect);
	}
}

//-----------------------------------------------------------------------------
void COpenGLView::onScaleFactorChanged (CFrame* frame, double newScaleFactor)
{
	scaleFactor = frame->getZoom ();
	updatePlatformOpenGLViewSize ();
}

} // VSTGUI

#endif // VSTGUI_OPENGL_SUPPORT
