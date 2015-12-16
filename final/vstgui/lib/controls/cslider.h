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

#ifndef __cslider__
#define __cslider__

#include "ccontrol.h"
#include "../ccolor.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CSlider Declaration
//! @brief a slider control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CSlider : public CControl
{
public:
	CSlider (const CRect& size, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const int32_t style = kLeft|kHorizontal);
	CSlider (const CRect& rect, IControlListener* listener, int32_t tag, const CPoint& offsetHandle, int32_t rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const int32_t style = kLeft|kHorizontal);
	CSlider (const CSlider& slider);

	enum Mode {
		kTouchMode,
		kRelativeTouchMode,
		kFreeClickMode
	};
	//-----------------------------------------------------------------------------
	/// @name CSlider Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setDrawTransparentHandle (bool val) { bDrawTransparentEnabled = val; }
	virtual bool getDrawTransparentHandle () const { return bDrawTransparentEnabled; }
	virtual void setMode (Mode newMode) { mode = newMode; }
	virtual Mode getMode () const { return mode; }
	virtual void setOffsetHandle (const CPoint& val);
	virtual CPoint getOffsetHandle () const { return offsetHandle; }
	virtual void setOffset (const CPoint& val) { offset = val; }
	virtual CPoint getOffset () const { return offset; }

	virtual void setStyle (int32_t style);
	virtual int32_t getStyle () const { return style; }

	virtual void     setHandle (CBitmap* pHandle);
	virtual CBitmap* getHandle () const { return pHandle; }

	virtual void  setZoomFactor (float val) { zoomFactor = val; }
	virtual float getZoomFactor () const { return zoomFactor; }

	VSTGUI_DEPRECATED(virtual void setFreeClick (bool val) { setMode (val ? kFreeClickMode : kTouchMode); })
	VSTGUI_DEPRECATED(virtual bool getFreeClick () const { return getMode () == kFreeClickMode; })
	//@}

	//-----------------------------------------------------------------------------
	/// @name Draw Style Methods
	//-----------------------------------------------------------------------------
	//@{
	enum DrawStyle {
		kDrawFrame				= 1 << 0,
		kDrawBack				= 1 << 1,
		kDrawValue				= 1 << 2,
		kDrawValueFromCenter	= 1 << 3,
		kDrawInverted			= 1 << 4
	};

	virtual void setDrawStyle (int32_t style);
	virtual void setFrameColor (CColor color);
	virtual void setBackColor (CColor color);
	virtual void setValueColor (CColor color);

	int32_t getDrawStyle () const { return drawStyle; }
	CColor getFrameColor () const { return frameColor; }
	CColor getBackColor () const { return backColor; }
	CColor getValueColor () const { return valueColor; }
	//@}

	// overrides
	virtual void draw (CDrawContext*) VSTGUI_OVERRIDE_VMETHOD;

	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseCancel () VSTGUI_OVERRIDE_VMETHOD;

	virtual bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual int32_t onKeyDown (VstKeyCode& keyCode) VSTGUI_OVERRIDE_VMETHOD;

	virtual bool sizeToFit () VSTGUI_OVERRIDE_VMETHOD;

	static bool kAlwaysUseZoomFactor;

	CLASS_METHODS(CSlider, CControl)
protected:
	~CSlider ();
	void setViewSize (const CRect& rect, bool invalid) VSTGUI_OVERRIDE_VMETHOD;
	
	float calculateDelta (const CPoint& where, CRect* handleRect = 0) const;
	
	CPoint	offset;
	CPoint	offsetHandle;

	CBitmap* pHandle;

	int32_t	style;
	Mode mode;

	CCoord	widthOfSlider;
	CCoord	heightOfSlider;
	CCoord	rangeHandle;
	CCoord	minTmp;
	CCoord	maxTmp;
	CCoord	minPos;
	CCoord	widthControl;
	CCoord	heightControl;
	float	zoomFactor;

	bool	bDrawTransparentEnabled;

	int32_t	drawStyle;
	CColor  frameColor;
	CColor  backColor;
	CColor  valueColor;
private:
	CCoord	delta;
	float	oldVal;
	float	startVal;
	CButtonState oldButton;
	CPoint mouseStartPoint;
};

//-----------------------------------------------------------------------------
// CVerticalSlider Declaration
//! @brief a vertical slider control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CVerticalSlider : public CSlider
{
public:
	CVerticalSlider (const CRect& size, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const int32_t style = kBottom);
	CVerticalSlider (const CRect& rect, IControlListener* listener, int32_t tag, const CPoint& offsetHandle, int32_t rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const int32_t style = kBottom);
	CVerticalSlider (const CVerticalSlider& slider);
};

//-----------------------------------------------------------------------------
// CHorizontalSlider Declaration
//! @brief a horizontal slider control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CHorizontalSlider : public CSlider
{
public:
	CHorizontalSlider (const CRect& size, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const int32_t style = kRight);
	CHorizontalSlider (const CRect& rect, IControlListener* listener, int32_t tag, const CPoint& offsetHandle, int32_t rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const int32_t style = kRight);
	CHorizontalSlider (const CHorizontalSlider& slider);
};

} // namespace

#endif