// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "cparamdisplay.h"
#include "../cbitmap.h"
#include <cmath>

namespace VSTGUI {

//------------------------------------------------------------------------
class CXYPad : public CParamDisplay, protected CMouseWheelEditingSupport
{
public:
	explicit CXYPad (const CRect& size = CRect (0, 0, 0, 0));

	void setStopTrackingOnMouseExit (bool state) { stopTrackingOnMouseExit = state; }
	bool getStopTrackingOnMouseExit () const { return stopTrackingOnMouseExit; }

	void setHandleBitmap (CBitmap* bitmap);
	CBitmap* getHandleBitmap () const;

	void draw (CDrawContext* context) override;
	void drawBack (CDrawContext* pContext, CBitmap* newBack = nullptr) override;

	void onMouseDownEvent (MouseDownEvent& event) override;
	void onMouseUpEvent (MouseUpEvent& event) override;
	void onMouseMoveEvent (MouseMoveEvent& event) override;
	void onMouseCancelEvent (MouseCancelEvent& event) override;
	void onMouseWheelEvent (MouseWheelEvent& event) override;
	void onKeyboardEvent (KeyboardEvent& event) override;

	/** set default value so that x and y default to val */
	void setDefaultValue (float val) override;
	/** set default value for x and y */
	void setDefaultValues (float x, float y);

	static float calculateValue (float x, float y)
	{
		x = std::floor (x * 1000.f + 0.5f) * 0.001f;
		y = std::floor (y * 1000.f + 0.5f) * 0.0000001f;
		return x + y;
	}
	
	static void calculateXY (float value, float& x, float& y)
	{
		x = std::floor (value * 1000.f + 0.5f) * 0.001f;
		y = std::floor ((value - x)  * 10000000.f + 0.5f) * 0.001f;
	}
	
protected:
	void onMouseMove (MouseDownUpMoveEvent& event);

	void setMin (float val) override { }
	void setMax (float val) override { }

	void boundValues (float& x, float& y);
	
	float mouseStartValue;
	CPoint mouseChangeStartPoint;
	CPoint lastMouseChangePoint;
	bool stopTrackingOnMouseExit;
	SharedPointer<CBitmap> handle;
};

} // VSTGUI
