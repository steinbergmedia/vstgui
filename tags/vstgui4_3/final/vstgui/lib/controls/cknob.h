//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef __cknob__
#define __cknob__

#include "ccontrol.h"
#include "../ccolor.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CKnob Declaration
//! @brief a knob control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CKnob : public CControl
{
public:
	enum DrawStyle {
		kLegacyHandleLineDrawing	= 0,
		kHandleCircleDrawing		= 1 << 0,
		kCoronaDrawing				= 1 << 1,
		kCoronaFromCenter			= 1 << 2,
		kCoronaInverted				= 1 << 3,
		kCoronaLineDashDot			= 1 << 4,
		kCoronaOutline				= 1 << 5
	};
	
	CKnob (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, CBitmap* handle, const CPoint& offset = CPoint (0, 0), int32_t drawStyle = kLegacyHandleLineDrawing);
	CKnob (const CKnob& knob);

	//-----------------------------------------------------------------------------
	/// @name CKnob Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void  setStartAngle (float val);
	virtual float getStartAngle () const { return startAngle; }

	virtual void  setRangeAngle (float val);
	virtual float getRangeAngle () const { return rangeAngle; }

	virtual void  valueToPoint (CPoint& point) const;
	virtual float valueFromPoint (CPoint& point) const;

	virtual CCoord getInsetValue () const { return inset; }
	virtual void setInsetValue (CCoord val) { inset = val; }

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

	virtual CBitmap* getHandleBitmap () const { return pHandle; }
	virtual void setHandleBitmap (CBitmap* bitmap);

	virtual void  setZoomFactor (float val) { zoomFactor = val; }
	virtual float getZoomFactor () const { return zoomFactor; }
	//@}

	// overrides
	virtual void draw (CDrawContext* pContext) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual int32_t onKeyDown (VstKeyCode& keyCode) VSTGUI_OVERRIDE_VMETHOD;
	virtual void setViewSize (const CRect &rect, bool invalid = true) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool sizeToFit () VSTGUI_OVERRIDE_VMETHOD;
	virtual void setMin (float val) VSTGUI_OVERRIDE_VMETHOD;
	virtual void setMax (float val) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool getFocusPath (CGraphicsPath& outPath) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool drawFocusOnTop () VSTGUI_OVERRIDE_VMETHOD;

	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;

	CLASS_METHODS(CKnob, CControl)
protected:
	~CKnob ();
	virtual void drawHandle (CDrawContext* pContext);
	virtual void drawCoronaOutline (CDrawContext* pContext) const;
	virtual void drawCorona (CDrawContext* pContext) const;
	virtual void drawHandleAsCircle (CDrawContext* pContext) const;
	virtual void drawHandleAsLine (CDrawContext* pContext) const;
	void compute ();
	void addArc (CGraphicsPath* path, const CRect& r, double startAngle, double sweepAngle) const;

	CPoint offset;
	
	int32_t drawStyle;
	CColor colorHandle, colorShadowHandle, coronaColor;
	CCoord handleLineWidth;
	CCoord inset;
	CCoord coronaInset;

	CBitmap* pHandle;
	float startAngle, rangeAngle;
	float zoomFactor;

private:
	CPoint firstPoint;
	CPoint lastPoint;
	float  startValue;
	float  fEntryState;
	float  range;
	float  coef;
	CButtonState   oldButton;
	bool   modeLinear;
	
};

//-----------------------------------------------------------------------------
// CAnimKnob Declaration
//! @brief a bitmap knob control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CAnimKnob : public CKnob, public IMultiBitmapControl
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
	//@}

	// overrides
	virtual void draw (CDrawContext* pContext) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool sizeToFit () VSTGUI_OVERRIDE_VMETHOD;
	void setHeightOfOneImage (const CCoord& height) VSTGUI_OVERRIDE_VMETHOD;
	void setBackground (CBitmap *background) VSTGUI_OVERRIDE_VMETHOD;
	void setNumSubPixmaps (int32_t numSubPixmaps) VSTGUI_OVERRIDE_VMETHOD { IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps); invalid (); }

	CLASS_METHODS(CAnimKnob, CKnob)
protected:
	~CAnimKnob ();
	bool	bInverseBitmap;
};

} // namespace

#endif
