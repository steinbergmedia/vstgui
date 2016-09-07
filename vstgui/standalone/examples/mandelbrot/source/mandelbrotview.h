#pragma once

#include "vstgui/lib/cview.h"
#include <functional>

//------------------------------------------------------------------------
namespace Mandelbrot {

//------------------------------------------------------------------------
struct View : public VSTGUI::CView
{
	using CRect = VSTGUI::CRect;
	using CPoint = VSTGUI::CPoint;
	using CMouseEventResult = VSTGUI::CMouseEventResult;
	using CButtonState = VSTGUI::CButtonState;
	using CDrawContext = VSTGUI::CDrawContext;
	using ChangedFunc = std::function<void (CRect box)>;

	View (const ChangedFunc& func);

	int32_t onKeyDown (VstKeyCode& keyCode) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	void draw (CDrawContext* context) override;

private:
	CRect box;
	ChangedFunc changed;
};

//------------------------------------------------------------------------
} // Mandelbrot
