// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cexternalview.h"
#include "cframe.h"
#include "platform/iplatformframe.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
struct CExternalViewBaseImpl
{
private:
	using ExternalViewPtr = std::shared_ptr<ExternalView::IView>;

	ExternalViewPtr view;
	bool isAttached {false};

	static ExternalView::IntRect fromCRect (const CRect& r)
	{
		ExternalView::IntRect res;
		res.origin.x = static_cast<int64_t> (std::ceil (r.left));
		res.origin.y = static_cast<int64_t> (std::ceil (r.top));
		res.size.width = static_cast<int64_t> (std::floor (r.getWidth ()));
		res.size.height = static_cast<int64_t> (std::floor (r.getHeight ()));
		return res;
	}

	static CRect calculateSize (CViewContainer* parent, CRect newSize)
	{
		CFrame* frame = parent ? parent->getFrame () : nullptr;
		while (parent && parent != frame)
		{
			CRect parentSize = parent->getViewSize ();
			parent->getTransform ().transform (newSize);
			newSize.offset (parentSize.left, parentSize.top);
			newSize.bound (parentSize);
			parent = static_cast<CViewContainer*> (parent->getParentView ());
		}
		if (frame)
			frame->getTransform ().transform (newSize);
		return newSize;
	}

public:
	CExternalViewBaseImpl (const ExternalViewPtr& v) : view (v) {}

	void updateSize (CViewContainer* parent, CRect localSize, CRect globalSize)
	{
		if (!view)
			return;
		localSize = calculateSize (parent, localSize);
		view->setViewSize (fromCRect (globalSize), fromCRect (localSize));
	}

	void attach (CFrame* frame)
	{
		if (!frame || !view)
			return;
		auto platformFrame = frame->getPlatformFrame ();
		if (!platformFrame)
			return;
		ExternalView::PlatformViewType viewType = {};
		switch (platformFrame->getPlatformType ())
		{
			case PlatformType::kHWND:
				viewType = ExternalView::PlatformViewType::HWND;
				break;
			case PlatformType::kNSView:
				viewType = ExternalView::PlatformViewType::NSView;
				break;
			default:
				return;
		}
		if (!view->platformViewTypeSupported (viewType))
			return;
		isAttached = view->attach (platformFrame->getPlatformRepresentation (), viewType);
	}
	void remove ()
	{
		if (view && isAttached)
		{
			view->remove ();
			isAttached = false;
		}
	}
	void scaleFactorChanged (double scaleFactor)
	{
		if (view)
			view->setContentScaleFactor (scaleFactor);
	}
	void takeFocus ()
	{
		if (view)
			view->takeFocus ();
	}
	void looseFocus ()
	{
		if (view)
			view->looseFocus ();
	}
	void enableMouse (bool state)
	{
		if (view)
			view->setMouseEnabled (state);
	}

	ExternalView::IView* getView () const { return view.get (); }
};

//------------------------------------------------------------------------
struct CExternalView::Impl : CExternalViewBaseImpl
{
	using CExternalViewBaseImpl::CExternalViewBaseImpl;
};

//------------------------------------------------------------------------
CExternalView::CExternalView (const CRect& r, const ExternalViewPtr& view) : CView (r)
{
	impl = std::make_unique<Impl> (view);
	impl->getView ()->setTookFocusCallback ([this] () {
		if (auto frame = getFrame ())
			frame->setFocusView (this);
	});
}

//------------------------------------------------------------------------
CExternalView::~CExternalView () noexcept { impl->getView ()->setTookFocusCallback (nullptr); }

//------------------------------------------------------------------------
bool CExternalView::attached (CView* parent)
{
	if (CView::attached (parent))
	{
		auto frame = parent->getFrame ();
		impl->updateSize (parent->asViewContainer (), getViewSize (),
						  translateToGlobal (getViewSize ()));
		impl->scaleFactorChanged (frame->getScaleFactor ());
		impl->attach (frame);
		frame->registerScaleFactorChangedListener (this);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool CExternalView::removed (CView* parent)
{
	if (auto frame = parent->getFrame ())
	{
		frame->unregisterScaleFactorChangedListener (this);
	}
	impl->remove ();
	return CView::removed (parent);
}

//------------------------------------------------------------------------
void CExternalView::takeFocus () { impl->takeFocus (); }

//------------------------------------------------------------------------
void CExternalView::looseFocus () { impl->looseFocus (); }

//------------------------------------------------------------------------
void CExternalView::setViewSize (const CRect& rect, bool invalid)
{
	CView::setViewSize (rect, invalid);
	impl->updateSize (getParentView () ? getParentView ()->asViewContainer () : nullptr,
					  getViewSize (), translateToGlobal (getViewSize ()));
}

//------------------------------------------------------------------------
void CExternalView::parentSizeChanged ()
{
	impl->updateSize (getParentView () ? getParentView ()->asViewContainer () : nullptr,
					  getViewSize (), translateToGlobal (getViewSize ()));
}

//------------------------------------------------------------------------
void CExternalView::onScaleFactorChanged (CFrame* frame, double newScaleFactor)
{
	impl->scaleFactorChanged (newScaleFactor);
}

//------------------------------------------------------------------------
void CExternalView::setMouseEnabled (bool enable)
{
	impl->enableMouse (enable);
	CView::setMouseEnabled (enable);
}

//------------------------------------------------------------------------
ExternalView::IView* CExternalView::getExternalView () const { return impl->getView (); }

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
struct CExternalControl::Impl : CExternalViewBaseImpl
{
	using CExternalViewBaseImpl::CExternalViewBaseImpl;
};

//------------------------------------------------------------------------
CExternalControl::CExternalControl (const CRect& r, const ExternalControlPtr& view)
: CControl (r, nullptr)
{
	impl = std::make_unique<Impl> (view);
	impl->getView ()->setTookFocusCallback ([this] () {
		if (auto frame = getFrame ())
			frame->setFocusView (this);
	});
	auto control = dynamic_cast<ExternalView::IControlViewExtension*> (impl->getView ());
	vstgui_assert (
		control, "Please provide an object that inherits from ExternalView::IControlViewExtension");
	control->setEditCallbacks ({
		[this] () { beginEdit (); },
		[this] (double newValue) {
			auto old = getValue ();
			setValueNormalized (static_cast<float> (newValue));
			if (old != getValue ())
				valueChanged ();
		},
		[this] () { endEdit (); },
	});
	setWantsFocus (true);
}

//------------------------------------------------------------------------
CExternalControl::~CExternalControl () noexcept
{
	impl->getView ()->setTookFocusCallback (nullptr);
	auto control = dynamic_cast<ExternalView::IControlViewExtension*> (impl->getView ());
	control->setEditCallbacks ({});
}

//------------------------------------------------------------------------
void CExternalControl::setValue (float val)
{
	CControl::setValue (val);
	auto control = dynamic_cast<ExternalView::IControlViewExtension*> (impl->getView ());
	control->setValue (getValueNormalized ());
}

//------------------------------------------------------------------------
bool CExternalControl::attached (CView* parent)
{
	if (CControl::attached (parent))
	{
		auto frame = parent->getFrame ();
		impl->updateSize (parent->asViewContainer (), getViewSize (),
						  translateToGlobal (getViewSize ()));
		impl->scaleFactorChanged (frame->getScaleFactor ());
		impl->attach (frame);
		frame->registerScaleFactorChangedListener (this);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool CExternalControl::removed (CView* parent)
{
	if (auto frame = parent->getFrame ())
	{
		frame->unregisterScaleFactorChangedListener (this);
	}
	impl->remove ();
	return CControl::removed (parent);
}

//------------------------------------------------------------------------
void CExternalControl::takeFocus () { impl->takeFocus (); }

//------------------------------------------------------------------------
void CExternalControl::looseFocus () { impl->looseFocus (); }

//------------------------------------------------------------------------
void CExternalControl::setViewSize (const CRect& rect, bool invalid)
{
	CControl::setViewSize (rect, invalid);
	impl->updateSize (getParentView () ? getParentView ()->asViewContainer () : nullptr,
					  getViewSize (), translateToGlobal (getViewSize ()));
}

//------------------------------------------------------------------------
void CExternalControl::parentSizeChanged ()
{
	impl->updateSize (getParentView () ? getParentView ()->asViewContainer () : nullptr,
					  getViewSize (), translateToGlobal (getViewSize ()));
}

//------------------------------------------------------------------------
void CExternalControl::onScaleFactorChanged (CFrame* frame, double newScaleFactor)
{
	impl->scaleFactorChanged (newScaleFactor);
}

//------------------------------------------------------------------------
void CExternalControl::setMouseEnabled (bool enable)
{
	impl->enableMouse (enable);
	CControl::setMouseEnabled (enable);
}

//------------------------------------------------------------------------
ExternalView::IView* CExternalControl::getExternalView () const { return impl->getView (); }

//------------------------------------------------------------------------
bool CExternalControl::getFocusPath (CGraphicsPath& outPath) { return true; }

//------------------------------------------------------------------------
} // VSTGUI
