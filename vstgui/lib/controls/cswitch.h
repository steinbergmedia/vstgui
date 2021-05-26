// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "ccontrol.h"
#include <algorithm>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class CSwitchBase : public CControl, public IMultiBitmapControl
{
public:
	void setInverseBitmap (bool state);
	bool getInverseBitmap () const { return inverseBitmap; }

protected:
	CSwitchBase (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background,
	             const CPoint& offset = CPoint (0, 0));
	CSwitchBase (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps,
	             CCoord heightOfOneImage, int32_t iMaxPositions, CBitmap* background,
	             const CPoint& offset = CPoint (0, 0));
	CSwitchBase (const CSwitchBase& other);
	~CSwitchBase () noexcept override = default;

	void draw (CDrawContext*) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	bool sizeToFit () override;

	void setNumSubPixmaps (int32_t numSubPixmaps) override
	{
		IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps);
		invalid ();
	}
	const CPoint& getOffset () const { return offset; }

	double getCoef () const { return coef; }
	int32_t normalizedToIndex (float norm) const
	{
		if (useLegacyIndexCalculation)
			return static_cast<int32_t> (norm * (getNumSubPixmaps () - 1) + 0.5f);
		return std::min<int32_t> (getNumSubPixmaps () - 1,
		                          static_cast<int32_t> (norm * getNumSubPixmaps ()));
	}

	float indexToNormalized (int32_t index) const
	{
		return static_cast<float> (index) / static_cast<float> (getNumSubPixmaps () - 1);
	}

	virtual double calculateCoef () const = 0;
	virtual float calcNormFromPoint (const CPoint& where) const = 0;

	static bool useLegacyIndexCalculation;

private:
	CPoint offset;
	double coef;
	float mouseStartValue;
	bool inverseBitmap{false};
};

//-----------------------------------------------------------------------------
// CVerticalSwitch Declaration
//! @brief a vertical switch control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CVerticalSwitch : public CSwitchBase
{
public:
	CVerticalSwitch (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CVerticalSwitch (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, int32_t iMaxPositions, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CVerticalSwitch (const CVerticalSwitch& vswitch);

	void onKeyboardEvent (KeyboardEvent& event) override;

	CLASS_METHODS(CVerticalSwitch, CControl)
protected:
	~CVerticalSwitch () noexcept override = default;

	double calculateCoef () const override;
	float calcNormFromPoint (const CPoint& where) const override;
};


//-----------------------------------------------------------------------------
// CHorizontalSwitch Declaration
//! @brief a horizontal switch control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CHorizontalSwitch : public CSwitchBase
{
public:
	CHorizontalSwitch (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CHorizontalSwitch (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, int32_t iMaxPositions, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CHorizontalSwitch (const CHorizontalSwitch& hswitch);

	void onKeyboardEvent (KeyboardEvent& event) override;

	CLASS_METHODS(CHorizontalSwitch, CControl)
protected:
	~CHorizontalSwitch () noexcept override = default;

	double calculateCoef () const override;
	float calcNormFromPoint (const CPoint& where) const override;
};


//-----------------------------------------------------------------------------
// CRockerSwitch Declaration
//! @brief a switch control with 3 sub bitmaps
/// @ingroup controls
//-----------------------------------------------------------------------------
class CRockerSwitch : public CControl, public IMultiBitmapControl
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

	CRockerSwitch (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset = CPoint (0, 0), const int32_t style = kHorizontal);
	CRockerSwitch (const CRect& size, IControlListener* listener, int32_t tag, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset = CPoint (0, 0), const int32_t style = kHorizontal);
	CRockerSwitch (const CRockerSwitch& rswitch);

	void draw (CDrawContext*) override;
	void onMouseWheelEvent (MouseWheelEvent& event) override;

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	void onKeyboardEvent (KeyboardEvent& event) override;

	bool sizeToFit () override;

	void setNumSubPixmaps (int32_t numSubPixmaps) override { IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps); invalid (); }

	CLASS_METHODS(CRockerSwitch, CControl)
protected:
	~CRockerSwitch () noexcept override;

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;

	CPoint	offset;
	int32_t	style;

	CVSTGUITimer* resetValueTimer;
private:
	float mouseStartValue;
};

} // VSTGUI
