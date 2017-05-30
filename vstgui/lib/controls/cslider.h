// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
	void draw (CDrawContext*) override;

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;

	bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons) override;
	int32_t onKeyDown (VstKeyCode& keyCode) override;

	bool sizeToFit () override;

	static bool kAlwaysUseZoomFactor;

	CLASS_METHODS(CSlider, CControl)
protected:
	~CSlider () noexcept override;
	void setViewSize (const CRect& rect, bool invalid) override;
	
	float calculateDelta (const CPoint& where, CRect* handleRect = nullptr) const;
	
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
	CVerticalSlider (const CVerticalSlider& slider) = default;
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
	CHorizontalSlider (const CHorizontalSlider& slider) = default;
};

} // namespace

#endif