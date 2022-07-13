// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "cview.h"
#include "iscalefactorchangedlistener.h"
#include "iexternalview.h"
#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
class CExternalView : public CView,
					  public IScaleFactorChangedListener
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

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // VSTGUI
