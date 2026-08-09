// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "ccontrol.h"
#include "../cbitmap.h"
#include <algorithm>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class CSwitchBase : public CControl,
					public MultiFrameBitmapView<CSwitchBase>
{
public:
	void setInverseBitmap (bool state);
	bool getInverseBitmap () const { return inverseBitmap; }

protected:
	CSwitchBase (const CRect& size, IControlListener* listener, int32_t tag,
				 const SPtr<CBitmap>& background);
	~CSwitchBase () noexcept override = default;

	void draw (CDrawContext&) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	bool sizeToFit () override;

	double getCoef () const { return coef; }
	int32_t normalizedToIndex (float norm) const;
	float indexToNormalized (int32_t index) const;

	virtual double calculateCoef () const = 0;
	virtual float calcNormFromPoint (const CPoint& where) const = 0;

private:
	double coef;
	float mouseStartValue;
	bool inverseBitmap{false};
};

//-----------------------------------------------------------------------------
// CVerticalSwitch Declaration
//! @brief a vertical switch control
/// @ingroup controls uses_multi_frame_bitmaps
//-----------------------------------------------------------------------------
class CVerticalSwitch : public CSwitchBase
{
public:
	CVerticalSwitch (const CRect& size, IControlListener* listener, int32_t tag,
					 const SPtr<CBitmap>& background);

	void onKeyboardEvent (KeyboardEvent& event) override;

protected:
	VSTGUI_SHAREDPTR_FRIEND (CVerticalSwitch)
	~CVerticalSwitch () noexcept override = default;

	double calculateCoef () const override;
	float calcNormFromPoint (const CPoint& where) const override;
};


//-----------------------------------------------------------------------------
// CHorizontalSwitch Declaration
//! @brief a horizontal switch control
/// @ingroup controls uses_multi_frame_bitmaps
//-----------------------------------------------------------------------------
class CHorizontalSwitch : public CSwitchBase
{
public:
	CHorizontalSwitch (const CRect& size, IControlListener* listener, int32_t tag,
					   const SPtr<CBitmap>& background);

	void onKeyboardEvent (KeyboardEvent& event) override;

protected:
	VSTGUI_SHAREDPTR_FRIEND (CHorizontalSwitch)
	~CHorizontalSwitch () noexcept override = default;

	double calculateCoef () const override;
	float calcNormFromPoint (const CPoint& where) const override;
};

//-----------------------------------------------------------------------------
// CRockerSwitch Declaration
//! @brief a switch control with 3 sub bitmaps
/// @ingroup controls use_multi_frame_bitmaps
//-----------------------------------------------------------------------------
class CRockerSwitch : public CControl,
					  public MultiFrameBitmapView<CRockerSwitch>
{
private:
	enum StyleEnum
	{
		StyleHorizontal = 0,
		StyleVertical,
	};
public:
	enum Style
	{
		kHorizontal = 1 << StyleHorizontal,
		kVertical = 1 << StyleVertical,
	};

	CRockerSwitch (const CRect& size, IControlListener* listener, int32_t tag,
				   const SPtr<CBitmap>& background, const int32_t style = kHorizontal);

	void draw (CDrawContext&) override;
	void onMouseWheelEvent (MouseWheelEvent& event) override;

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	void onKeyboardEvent (KeyboardEvent& event) override;

	bool sizeToFit () override;

protected:
	VSTGUI_SHAREDPTR_FRIEND (CRockerSwitch)
	~CRockerSwitch () noexcept override;

	int32_t	style;

	SPtr<CVSTGUITimer> resetValueTimer;

private:
	float mouseStartValue;
};

} // VSTGUI
