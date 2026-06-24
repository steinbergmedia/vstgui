// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "ccontrol.h"
#include "../cbitmap.h"
#include "../ccolor.h"
#include "../clinestyle.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class CKnobBase : public CControl, protected CMouseWheelEditingSupport
{
public:
	//-----------------------------------------------------------------------------
	/// @name CKnobBase Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void  valueToPoint (CPoint& point) const;
	virtual float valueFromPoint (CPoint& point) const;

	virtual void  setStartAngle (float val);
	virtual float getStartAngle () const { return startAngle; }

	virtual void  setRangeAngle (float val);
	virtual float getRangeAngle () const { return rangeAngle; }

	virtual void  setZoomFactor (float val) { zoomFactor = val; }
	virtual float getZoomFactor () const { return zoomFactor; }

	virtual CCoord getInsetValue () const { return inset; }
	virtual void setInsetValue (CCoord val) { inset = val; }

	virtual void setKnobRange (float val) { if (val > 0.f) knobRange = val; }
	virtual float getKnobRange () const { return knobRange; }
	//@}

	// overrides
	void onMouseWheelEvent (MouseWheelEvent& event) override;
	void onKeyboardEvent (KeyboardEvent& event) override;
	void setViewSize (const CRect &rect, bool invalid = true) override;
	bool sizeToFit () override;
	void setMin (float val) override;
	void setMax (float val) override;

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;

protected:
	CKnobBase (const CRect& size, IControlListener* listener, int32_t tag,
			   const SharedPointer<CBitmap>& background);
	void compute ();

	float startAngle, rangeAngle;
	float zoomFactor;
	float knobRange;
	CCoord inset;

private:
	struct MouseEditingState;
	
	MouseEditingState& getMouseEditingState ();
	void clearMouseEditingState ();
};

//-----------------------------------------------------------------------------
// CKnob Declaration
//! @brief a knob control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CKnob : public CKnobBase
{
public:
	enum DrawStyle {
		kLegacyHandleLineDrawing	= 0,
		kHandleCircleDrawing		= 1 << 0,
		kCoronaDrawing				= 1 << 1,
		kCoronaFromCenter			= 1 << 2,
		kCoronaInverted				= 1 << 3,
		kCoronaLineDashDot			= 1 << 4,
		kCoronaOutline				= 1 << 5,
		kCoronaLineCapButt			= 1 << 6,
		kSkipHandleDrawing			= 1 << 7,
	};

	CKnob (const CRect& size, IControlListener* listener, int32_t tag,
		   const SharedPointer<CBitmap>& background, const SharedPointer<CBitmap>& handle,
		   const CPoint& offset = CPoint (0, 0), int32_t drawStyle = kLegacyHandleLineDrawing);

	//-----------------------------------------------------------------------------
	/// @name CKnob Methods
	//-----------------------------------------------------------------------------
	//@{
	int32_t getDrawStyle () const { return drawStyle; }
	virtual void setDrawStyle (int32_t style);
	
	CColor getCoronaColor () const { return coronaColor; }
	virtual void setCoronaColor (CColor color);

	CCoord getCoronaInset () const { return coronaInset; }
	virtual void setCoronaInset (CCoord inset);
	
	CColor getColorShadowHandle () const { return colorShadowHandle; }
	virtual void setColorShadowHandle (CColor color);

	CColor getColorHandle () const { return colorHandle; }
	virtual void setColorHandle (CColor color);

	CCoord getHandleLineWidth () const { return handleLineWidth; }
	virtual void setHandleLineWidth (CCoord width);

	CCoord getCoronaOutlineWidthAdd () const { return coronaOutlineWidthAdd; }
	virtual void setCoronaOutlineWidthAdd (CCoord width);

	const CLineStyle::CoordVector& getCoronaDashDotLengths () const;
	virtual void setCoronaDashDotLengths (const CLineStyle::CoordVector& lengths);

	SharedPointer<CBitmap> getHandleBitmap () const { return pHandle; }
	void setHandleBitmap (const SharedPointer<CBitmap>& bitmap);
	//@}

	// overrides
	void draw (CDrawContext& context) override;
	bool getFocusPath (CGraphicsPath& outPath, CCoord focusLineWidth) override;
	bool drawFocusOnTop () override;

protected:
	VSTGUI_SHAREDPTR_FRIEND (CKnob)
	~CKnob () noexcept override;

	virtual void drawHandle (CDrawContext& context);
	virtual void drawCoronaOutline (CDrawContext& context) const;
	virtual void drawCorona (CDrawContext& context) const;
	virtual void drawHandleAsCircle (CDrawContext& context) const;
	virtual void drawHandleAsLine (CDrawContext& context) const;

	static void addArc (const SharedPointer<CGraphicsPath>& path, const CRect& r, double startAngle,
						double sweepAngle);

	CPoint offset;
	
	int32_t drawStyle;
	CColor colorHandle, colorShadowHandle, coronaColor;
	CCoord handleLineWidth;
	CCoord coronaInset;
	CCoord coronaOutlineWidthAdd;

	CLineStyle coronaLineStyle;
	SharedPointer<CBitmap> pHandle;
};

//-----------------------------------------------------------------------------
// CAnimKnob Declaration
//! @brief a bitmap knob control
/// @ingroup controls uses_multi_frame_bitmaps
//-----------------------------------------------------------------------------
class CAnimKnob : public CKnobBase,
				  public MultiFrameBitmapView<CAnimKnob>
{
public:
	CAnimKnob (const CRect& size, IControlListener* listener, int32_t tag,
			   const SharedPointer<CBitmap>& background);

	//-----------------------------------------------------------------------------
	/// @name CAnimKnob Methods
	//-----------------------------------------------------------------------------
	//@{
	void setInverseBitmap (bool val) { bInverseBitmap = val; }
	bool getInverseBitmap () const { return bInverseBitmap; }
	//@}

	// overrides
	void draw (CDrawContext& context) override;
	bool sizeToFit () override;
	void setBackground (const SharedPointer<CBitmap>& background) override;

protected:
	VSTGUI_SHAREDPTR_FRIEND (CAnimKnob)
	~CAnimKnob () noexcept override = default;
	bool	bInverseBitmap;
};

} // VSTGUI
