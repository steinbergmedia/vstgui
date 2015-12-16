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

	virtual void draw (CDrawContext*) VSTGUI_OVERRIDE_VMETHOD;

	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual int32_t onKeyDown (VstKeyCode& keyCode) VSTGUI_OVERRIDE_VMETHOD;

	virtual bool sizeToFit () VSTGUI_OVERRIDE_VMETHOD;

	void setNumSubPixmaps (int32_t numSubPixmaps) VSTGUI_OVERRIDE_VMETHOD { IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps); invalid (); }

	CLASS_METHODS(CVerticalSwitch, CControl)
protected:
	~CVerticalSwitch ();
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

	virtual void draw (CDrawContext*) VSTGUI_OVERRIDE_VMETHOD;

	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual int32_t onKeyDown (VstKeyCode& keyCode) VSTGUI_OVERRIDE_VMETHOD;

	virtual bool sizeToFit () VSTGUI_OVERRIDE_VMETHOD;

	void setNumSubPixmaps (int32_t numSubPixmaps) VSTGUI_OVERRIDE_VMETHOD { IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps); invalid (); }

	CLASS_METHODS(CHorizontalSwitch, CControl)
protected:
	~CHorizontalSwitch ();
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

	virtual void draw (CDrawContext*) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;

	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual int32_t onKeyDown (VstKeyCode& keyCode) VSTGUI_OVERRIDE_VMETHOD;
	virtual int32_t onKeyUp (VstKeyCode& keyCode) VSTGUI_OVERRIDE_VMETHOD;

	virtual bool sizeToFit () VSTGUI_OVERRIDE_VMETHOD;

	void setNumSubPixmaps (int32_t numSubPixmaps) VSTGUI_OVERRIDE_VMETHOD { IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps); invalid (); }

	CLASS_METHODS(CRockerSwitch, CControl)
protected:
	~CRockerSwitch ();

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;

	CPoint	offset;
	int32_t	style;

	CVSTGUITimer* resetValueTimer;
private:
	float fEntryState;
};

} // namespace

#endif
