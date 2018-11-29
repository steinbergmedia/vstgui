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
private:
	enum StyleEnum
	{
		StyleHorizontal = 0,
		StyleVertical,
		StyleLeft,
		StyleRight,
		StyleTop,
		StyleBottom,
	};
public:
	CSlider (const CRect& size, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const int32_t style = kLeft|kHorizontal);
	CSlider (const CRect& rect, IControlListener* listener, int32_t tag, const CPoint& offsetHandle, int32_t rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const int32_t style = kLeft|kHorizontal);
	CSlider (const CSlider& slider);

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	enum Mode {
		kTouchMode,
		kRelativeTouchMode,
		kFreeClickMode,
		kRampMode
	};
#endif
	//-----------------------------------------------------------------------------
	/// @name CSlider Methods
	//-----------------------------------------------------------------------------
	//@{
	VSTGUI_DEPRECATED (virtual void setDrawTransparentHandle (bool val) {})
	VSTGUI_DEPRECATED (virtual bool getDrawTransparentHandle () const { return true; })
	VSTGUI_DEPRECATED (virtual void setMode (Mode newMode);)
	VSTGUI_DEPRECATED (virtual Mode getMode () const;)
	virtual void setOffsetHandle (const CPoint& val);
	virtual CPoint getOffsetHandle () const;
	virtual void setOffset (const CPoint& val);
	virtual CPoint getOffset () const;

	virtual void setSliderMode (CSliderMode mode);
	CSliderMode getSliderMode () const;
	CSliderMode getEffectiveSliderMode () const;

	static void setGlobalMode (CSliderMode mode);
	static CSliderMode getGlobalMode ();

	enum Style
	{
		kHorizontal = 1 << StyleHorizontal,
		kVertical = 1 << StyleVertical,
		kLeft = 1 << StyleLeft,
		kRight = 1 << StyleRight,
		kTop = 1 << StyleTop,
		kBottom = 1 << StyleBottom,
	};

	virtual void setStyle (int32_t style);
	virtual int32_t getStyle () const;

	virtual void     setHandle (CBitmap* pHandle);
	virtual CBitmap* getHandle () const;

	virtual void  setZoomFactor (float val);
	virtual float getZoomFactor () const;
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
	virtual void setFrameWidth (CCoord width);
	virtual void setFrameColor (CColor color);
	virtual void setBackColor (CColor color);
	virtual void setValueColor (CColor color);

	int32_t getDrawStyle () const;
	CCoord getFrameWidth () const;
	CColor getFrameColor () const;
	CColor getBackColor () const;
	CColor getValueColor () const;
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
	
	CRect calculateHandleRect (float normValue) const;
	void doRamping ();

	// for sub-classes to access private variables:
	void setSliderSize (CCoord width, CCoord height);
	CPoint getSliderSize () const;
	CPoint getControlSize () const;

	struct Impl;
	std::unique_ptr<Impl> impl;

	static CSliderMode globalMode;
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
