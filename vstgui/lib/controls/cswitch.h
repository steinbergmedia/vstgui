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
#if VSTGUI_ENABLE_DEPRECATED_METHODS
,
					public IMultiBitmapControl
#endif
{
public:
	void setInverseBitmap (bool state);
	bool getInverseBitmap () const { return inverseBitmap; }

protected:
	CSwitchBase (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background);
	CSwitchBase (const CSwitchBase& other);
	~CSwitchBase () noexcept override = default;

	void draw (CDrawContext*) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	bool sizeToFit () override;

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	CSwitchBase (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps,
				 CCoord heightOfOneImage, int32_t iMaxPositions, CBitmap* background,
				 const CPoint& offset = CPoint (0, 0));
	void setNumSubPixmaps (int32_t numSubPixmaps) override
	{
		IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps);
		invalid ();
	}
	const CPoint& getOffset () const { return offset; }
#endif

	double getCoef () const { return coef; }
	int32_t normalizedToIndex (float norm) const;
	float indexToNormalized (int32_t index) const;

	virtual double calculateCoef () const = 0;
	virtual float calcNormFromPoint (const CPoint& where) const = 0;

	VSTGUI_DEPRECATED_MSG (static bool useLegacyIndexCalculation;
						   , "Use CMultiFrameBitmap::normalizedValueToFrameIndex() instead")

private:
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	CPoint offset {};
#endif
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
					 CBitmap* background);
	CVerticalSwitch (const CVerticalSwitch& vswitch);

	void onKeyboardEvent (KeyboardEvent& event) override;

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	CVerticalSwitch (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps,
					 CCoord heightOfOneImage, int32_t iMaxPositions, CBitmap* background,
					 const CPoint& offset = CPoint (0, 0));
#endif

	CLASS_METHODS(CVerticalSwitch, CControl)
protected:
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
					   CBitmap* background);
	CHorizontalSwitch (const CHorizontalSwitch& hswitch);

	void onKeyboardEvent (KeyboardEvent& event) override;

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	CHorizontalSwitch (const CRect& size, IControlListener* listener, int32_t tag,
					   int32_t subPixmaps, CCoord heightOfOneImage, int32_t iMaxPositions,
					   CBitmap* background, const CPoint& offset = CPoint (0, 0));
#endif

	CLASS_METHODS(CHorizontalSwitch, CControl)
protected:
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
#if VSTGUI_ENABLE_DEPRECATED_METHODS
,
					  public IMultiBitmapControl
#endif
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

	CRockerSwitch (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background,
				   const int32_t style = kHorizontal);
	CRockerSwitch (const CRockerSwitch& rswitch);

	void draw (CDrawContext*) override;
	void onMouseWheelEvent (MouseWheelEvent& event) override;

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	void onKeyboardEvent (KeyboardEvent& event) override;

	bool sizeToFit () override;

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	CRockerSwitch (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background,
				   const CPoint& offset, const int32_t style = kHorizontal);
	CRockerSwitch (const CRect& size, IControlListener* listener, int32_t tag,
				   CCoord heightOfOneImage, CBitmap* background,
				   const CPoint& offset = CPoint (0, 0), const int32_t style = kHorizontal);
	void setNumSubPixmaps (int32_t numSubPixmaps) override { IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps); invalid (); }
#endif

	CLASS_METHODS(CRockerSwitch, CControl)
protected:
	~CRockerSwitch () noexcept override;

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	CPoint offset {};
#endif
	int32_t	style;

	CVSTGUITimer* resetValueTimer;
private:
	float mouseStartValue;
};

} // VSTGUI
