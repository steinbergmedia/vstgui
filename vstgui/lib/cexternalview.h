// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "cview.h"
#include "controls/ccontrol.h"
#include "iscalefactorchangedlistener.h"
#include "iexternalview.h"
#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
/** View to embed non CView views into VSTGUI
 *
 *	This view is the umbrella for views from other view systems (like HWND child windows or
 *	NSViews).
 *	The actual implementation for the external view must be done via ExternalView::IView
 *
 *	@ingroup new_in_4_13
 */
class CExternalView : public CView,
					  public IScaleFactorChangedListener,
					  public ExternalView::IViewEmbedder
{
public:
	using ExternalViewPtr = std::shared_ptr<ExternalView::IView>;

	CExternalView (const CRect& r, const ExternalViewPtr& view);
	~CExternalView () noexcept;

	bool attached (CView* parent) override;
	bool removed (CView* parent) override;
	void takeFocus () override;
	void looseFocus () override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	void parentSizeChanged () override;
	void onScaleFactorChanged (CFrame* frame, double newScaleFactor) override;
	void setMouseEnabled (bool enable = true) override;

	ExternalView::IView* getExternalView () const override;

private:

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
class CExternalControl : public CControl,
						 public IScaleFactorChangedListener,
						 public ExternalView::IViewEmbedder
{
public:
	using ExternalControlPtr = std::shared_ptr<ExternalView::IView>;

	CExternalControl (const CRect& r, const ExternalControlPtr& control);
	~CExternalControl () noexcept;

	void setValue (float val) override;

	bool attached (CView* parent) override;
	bool removed (CView* parent) override;
	void takeFocus () override;
	void looseFocus () override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	void parentSizeChanged () override;
	void onScaleFactorChanged (CFrame* frame, double newScaleFactor) override;
	void setMouseEnabled (bool enable = true) override;

	ExternalView::IView* getExternalView () const override;

	CLASS_METHODS_NOCOPY (CExternalControl, CControl)
private:
	void draw (CDrawContext* pContext) override {}
	bool getFocusPath (CGraphicsPath& outPath) override;

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // VSTGUI
