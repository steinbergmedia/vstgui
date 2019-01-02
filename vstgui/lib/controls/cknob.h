// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "ccontrol.h"
#include "../ccolor.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class CKnobBase : public CControl
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
	//@}

	// overrides
	bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons) override;
	int32_t onKeyDown (VstKeyCode& keyCode) override;
	void setViewSize (const CRect &rect, bool invalid = true) override;
	bool sizeToFit () override;
	void setMin (float val) override;
	void setMax (float val) override;

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	
	CLASS_METHODS_VIRTUAL(CKnobBase, CControl)
protected:
	CKnobBase (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background);
	CKnobBase (const CKnobBase& knob);
	void compute ();

	float startAngle, rangeAngle;
	float zoomFactor;
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
	
	CKnob (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, CBitmap* handle, const CPoint& offset = CPoint (0, 0), int32_t drawStyle = kLegacyHandleLineDrawing);
	CKnob (const CKnob& knob);

	//-----------------------------------------------------------------------------
	/// @name CKnob Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual int32_t getDrawStyle () const { return drawStyle; }
	virtual void setDrawStyle (int32_t style);
	
	virtual CColor getCoronaColor () const { return coronaColor; }
	virtual void setCoronaColor (CColor color);

	virtual CCoord getCoronaInset () const { return coronaInset; }
	virtual void setCoronaInset (CCoord inset);
	
	virtual CColor getColorShadowHandle () const { return colorShadowHandle; }
	virtual void setColorShadowHandle (CColor color);

	virtual CColor getColorHandle () const { return colorHandle; }
	virtual void setColorHandle (CColor color);

	virtual CCoord getHandleLineWidth () const { return handleLineWidth; }
	virtual void setHandleLineWidth (CCoord width);

	virtual CCoord getCoronaOutlineWidthAdd () const { return coronaOutlineWidthAdd; }
	virtual void setCoronaOutlineWidthAdd (CCoord width);

	virtual CBitmap* getHandleBitmap () const { return pHandle; }
	virtual void setHandleBitmap (CBitmap* bitmap);
	//@}

	// overrides
	void draw (CDrawContext* pContext) override;
	bool getFocusPath (CGraphicsPath& outPath) override;
	bool drawFocusOnTop () override;

	CLASS_METHODS(CKnob, CKnobBase)
protected:
	~CKnob () noexcept override;

	virtual void drawHandle (CDrawContext* pContext);
	virtual void drawCoronaOutline (CDrawContext* pContext) const;
	virtual void drawCorona (CDrawContext* pContext) const;
	virtual void drawHandleAsCircle (CDrawContext* pContext) const;
	virtual void drawHandleAsLine (CDrawContext* pContext) const;

	void addArc (CGraphicsPath* path, const CRect& r, double startAngle, double sweepAngle) const;

	CPoint offset;
	
	int32_t drawStyle;
	CColor colorHandle, colorShadowHandle, coronaColor;
	CCoord handleLineWidth;
	CCoord coronaInset;
	CCoord coronaOutlineWidthAdd;

	CBitmap* pHandle;
};

//-----------------------------------------------------------------------------
// CAnimKnob Declaration
//! @brief a bitmap knob control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CAnimKnob : public CKnobBase, public IMultiBitmapControl
{
public:
	CAnimKnob (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CAnimKnob (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CAnimKnob (const CAnimKnob& knob);

	//-----------------------------------------------------------------------------
	/// @name CAnimKnob Methods
	//-----------------------------------------------------------------------------
	//@{
	void setInverseBitmap (bool val) { bInverseBitmap = val; }
	bool getInverseBitmap () const { return bInverseBitmap; }
	//@}

	// overrides
	void draw (CDrawContext* pContext) override;
	bool sizeToFit () override;
	void setHeightOfOneImage (const CCoord& height) override;
	void setBackground (CBitmap *background) override;
	void setNumSubPixmaps (int32_t numSubPixmaps) override { IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps); invalid (); }

	CLASS_METHODS(CAnimKnob, CKnobBase)
protected:
	~CAnimKnob () noexcept override = default;
	bool	bInverseBitmap;
};

} // VSTGUI
