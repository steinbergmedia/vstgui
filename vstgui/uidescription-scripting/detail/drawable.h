// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "viewscriptobject.h"
#include "drawcontextobject.h"
#include "../../lib/cview.h"
#include "../../lib/controls/ccontrol.h"
#include "../../uidescription/iviewcreator.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ScriptingInternal {

//------------------------------------------------------------------------
struct JavaScriptDrawable
{
	void onDraw (CDrawContext* context, const CRect& rect, const CRect& viewSize);

	void setup (ViewScriptObject* object);

private:
	ViewScriptObject* scriptObject {nullptr};
	DrawContextObject drawContext;
};

//------------------------------------------------------------------------
struct JavaScriptDrawableView : CView,
								JavaScriptDrawable
{
	using CView::CView;

	void drawRect (CDrawContext* context, const CRect& rect) override;
};

//------------------------------------------------------------------------
struct JavaScriptDrawableControl : CControl,
								   JavaScriptDrawable
{
	using CControl::CControl;

	void draw (CDrawContext* pContext) override;
	void drawRect (CDrawContext* context, const CRect& rect) override;

	CLASS_METHODS_NOCOPY (JavaScriptDrawableControl, CControl);
};

//------------------------------------------------------------------------
struct JavaScriptDrawableViewCreator : ViewCreatorAdapter
{
	IdStringPtr getViewName () const override;
	IdStringPtr getBaseViewName () const override;
	CView* create (const UIAttributes& attributes,
				   const IUIDescription* description) const override;
};

//------------------------------------------------------------------------
struct JavaScriptDrawableControlCreator : ViewCreatorAdapter
{
	IdStringPtr getViewName () const override;
	IdStringPtr getBaseViewName () const override;
	CView* create (const UIAttributes& attributes,
				   const IUIDescription* description) const override;
};

//------------------------------------------------------------------------
} // ScriptingInternal
} // VSTGUI
