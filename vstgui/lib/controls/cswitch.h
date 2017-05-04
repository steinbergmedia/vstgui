// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __cswitch__
#define __cswitch__

#include "ccontrol.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CVerticalSwitch Declaration
//! @brief a vertical switch control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CVerticalSwitch : public CControl, public IMultiBitmapControl
{
public:
	CVerticalSwitch (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CVerticalSwitch (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, int32_t iMaxPositions, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CVerticalSwitch (const CVerticalSwitch& vswitch);

	void draw (CDrawContext*) override;

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	int32_t onKeyDown (VstKeyCode& keyCode) override;

	bool sizeToFit () override;

	void setNumSubPixmaps (int32_t numSubPixmaps) override { IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps); invalid (); }

	CLASS_METHODS(CVerticalSwitch, CControl)
protected:
	~CVerticalSwitch () noexcept override = default;
	CPoint	offset;

private:
	double coef;
};


//-----------------------------------------------------------------------------
// CHorizontalSwitch Declaration
//! @brief a horizontal switch control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CHorizontalSwitch : public CControl, public IMultiBitmapControl
{
public:
	CHorizontalSwitch (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CHorizontalSwitch (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, int32_t iMaxPositions, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	CHorizontalSwitch (const CHorizontalSwitch& hswitch);

	void draw (CDrawContext*) override;

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	int32_t onKeyDown (VstKeyCode& keyCode) override;

	bool sizeToFit () override;

	void setNumSubPixmaps (int32_t numSubPixmaps) override { IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps); invalid (); }

	CLASS_METHODS(CHorizontalSwitch, CControl)
protected:
	~CHorizontalSwitch () noexcept override = default;
	CPoint	offset;

private:
	double coef;
};


//-----------------------------------------------------------------------------
// CRockerSwitch Declaration
//! @brief a switch control with 3 sub bitmaps
/// @ingroup controls
//-----------------------------------------------------------------------------
class CRockerSwitch : public CControl, public IMultiBitmapControl
{
public:
	CRockerSwitch (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset = CPoint (0, 0), const int32_t style = kHorizontal);
	CRockerSwitch (const CRect& size, IControlListener* listener, int32_t tag, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset = CPoint (0, 0), const int32_t style = kHorizontal);
	CRockerSwitch (const CRockerSwitch& rswitch);

	void draw (CDrawContext*) override;
	bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons) override;

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	int32_t onKeyDown (VstKeyCode& keyCode) override;
	int32_t onKeyUp (VstKeyCode& keyCode) override;

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
	float fEntryState;
};

} // namespace

#endif
