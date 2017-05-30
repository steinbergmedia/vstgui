// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstgui/lib/cview.h"
#include "vstgui/lib/ifocusdrawing.h"
#include <functional>

//------------------------------------------------------------------------
namespace Mandelbrot {

//------------------------------------------------------------------------
struct View : public VSTGUI::CView, public VSTGUI::IFocusDrawing
{
	using CRect = VSTGUI::CRect;
	using CPoint = VSTGUI::CPoint;
	using CMouseEventResult = VSTGUI::CMouseEventResult;
	using CButtonState = VSTGUI::CButtonState;
	using CDrawContext = VSTGUI::CDrawContext;
	using CGraphicsPath = VSTGUI::CGraphicsPath;
	using ChangedFunc = std::function<void (CRect box)>;

	View (ChangedFunc&& func);

	int32_t onKeyDown (VstKeyCode& keyCode) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	void draw (CDrawContext* context) override;

	bool drawFocusOnTop () override { return false; }
	bool getFocusPath (CGraphicsPath& outPath) override { return false; }

private:
	CRect box;
	ChangedFunc changed;
};

//------------------------------------------------------------------------
} // Mandelbrot
