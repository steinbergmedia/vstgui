// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../ccolor.h"
#include "ccontrol.h"

namespace VSTGUI {

//------------------------------------------------------------------------
class CSliderBase : public CControl, protected CMouseWheelEditingSupport
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
	enum Style
	{
		kHorizontal = 1 << StyleHorizontal,
		kVertical = 1 << StyleVertical,
		kLeft = 1 << StyleLeft,
		kRight = 1 << StyleRight,
		kTop = 1 << StyleTop,
		kBottom = 1 << StyleBottom,
	};

	CSliderBase (const CRect& size, IControlListener* listener, int32_t tag);
	CSliderBase (const CSliderBase& slider);

	void setOffsetHandle (const CPoint& val);
	CPoint getOffsetHandle () const;

	void setStyle (int32_t style);
	int32_t getStyle () const;
	bool isStyleHorizontal () const;
	bool isStyleRight () const;
	bool isStyleBottom () const;
	bool isInverseStyle () const;

	void setZoomFactor (float val);
	float getZoomFactor () const;

	void setSliderMode (CSliderMode mode);
	CSliderMode getSliderMode () const;
	CSliderMode getEffectiveSliderMode () const;

	static void setGlobalMode (CSliderMode mode);
	static CSliderMode getGlobalMode ();

	// overrides
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;

	bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance,
	              const CButtonState& buttons) override;
	int32_t onKeyDown (VstKeyCode& keyCode) override;

	void setViewSize (const CRect& rect, bool invalid) override;

	static bool kAlwaysUseZoomFactor;

protected:
	~CSliderBase () noexcept;

	CRect calculateHandleRect (float normValue) const;

	// for sub-classes to access private variables:
	void setHandleSizePrivate (CCoord width, CCoord height);
	CPoint getHandleSizePrivate () const;
	CPoint getControlSizePrivate () const;
	void setHandleRangePrivate (CCoord range);
	void setHandleMinPosPrivate (CCoord pos);
	CCoord getHandleMinPosPrivate () const;

private:
	void updateInternalHandleValues ();
	float calculateDelta (const CPoint& where, CRect* handleRect = nullptr) const;
	void doRamping ();

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
// CSlider Declaration
//! @brief a slider control
/// @ingroup controls
//------------------------------------------------------------------------
class CSlider : public CSliderBase
{
private:
public:
	CSlider (const CRect& size, IControlListener* listener, int32_t tag, int32_t iMinPos,
	         int32_t iMaxPos, CBitmap* handle, CBitmap* background,
	         const CPoint& offset = CPoint (0, 0), const int32_t style = kLeft | kHorizontal);
	CSlider (const CRect& rect, IControlListener* listener, int32_t tag, const CPoint& offsetHandle,
	         int32_t rangeHandle, CBitmap* handle, CBitmap* background,
	         const CPoint& offset = CPoint (0, 0), const int32_t style = kLeft | kHorizontal);
	CSlider (const CSlider& slider);

//------------------------------------------------------------------------
	/// @name CSlider Methods
//------------------------------------------------------------------------
	//@{
	VSTGUI_DEPRECATED (
	    /** \deprecated use setBackgroundOffset */
	    virtual void setOffset (const CPoint& val);)
	VSTGUI_DEPRECATED (
	    /** \deprecated use getBackgroundOffset*/
	    virtual CPoint getOffset () const;)

	/** set background draw offset */
	void setBackgroundOffset (const CPoint& offset);
	/** get background draw offset */
	CPoint getBackgroundOffset () const;

	virtual void setHandle (CBitmap* pHandle);
	virtual CBitmap* getHandle () const;
	//@}

//------------------------------------------------------------------------
	/// @name Draw Style Methods
//------------------------------------------------------------------------
	//@{
	enum DrawStyle
	{
		kDrawFrame = 1 << 0,
		kDrawBack = 1 << 1,
		kDrawValue = 1 << 2,
		kDrawValueFromCenter = 1 << 3,
		kDrawInverted = 1 << 4
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
	bool sizeToFit () override;

	CLASS_METHODS (CSlider, CControl)
protected:
	~CSlider () noexcept override;

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
// CVerticalSlider Declaration
//! @brief a vertical slider control
/// @ingroup controls
//------------------------------------------------------------------------
class CVerticalSlider : public CSlider
{
public:
	CVerticalSlider (const CRect& size, IControlListener* listener, int32_t tag, int32_t iMinPos,
	                 int32_t iMaxPos, CBitmap* handle, CBitmap* background,
	                 const CPoint& offset = CPoint (0, 0), const int32_t style = kBottom);
	CVerticalSlider (const CRect& rect, IControlListener* listener, int32_t tag,
	                 const CPoint& offsetHandle, int32_t rangeHandle, CBitmap* handle,
	                 CBitmap* background, const CPoint& offset = CPoint (0, 0),
	                 const int32_t style = kBottom);
	CVerticalSlider (const CVerticalSlider& slider) = default;
};

//------------------------------------------------------------------------
// CHorizontalSlider Declaration
//! @brief a horizontal slider control
/// @ingroup controls
//------------------------------------------------------------------------
class CHorizontalSlider : public CSlider
{
public:
	CHorizontalSlider (const CRect& size, IControlListener* listener, int32_t tag, int32_t iMinPos,
	                   int32_t iMaxPos, CBitmap* handle, CBitmap* background,
	                   const CPoint& offset = CPoint (0, 0), const int32_t style = kRight);
	CHorizontalSlider (const CRect& rect, IControlListener* listener, int32_t tag,
	                   const CPoint& offsetHandle, int32_t rangeHandle, CBitmap* handle,
	                   CBitmap* background, const CPoint& offset = CPoint (0, 0),
	                   const int32_t style = kRight);
	CHorizontalSlider (const CHorizontalSlider& slider) = default;
};

} // VSTGUI
