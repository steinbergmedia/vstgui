//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
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

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CSlider Declaration
//! @brief a slider control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CSlider : public CControl
{
public:
	CSlider (const CRect& size, CControlListener* listener, long tag, long iMinPos, long iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const long style = kLeft|kHorizontal);
	CSlider (const CRect& rect, CControlListener* listener, long tag, const CPoint& offsetHandle, long rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const long style = kLeft|kHorizontal);
	CSlider (const CSlider& slider);
  
	//-----------------------------------------------------------------------------
	/// @name CSlider Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setDrawTransparentHandle (bool val) { bDrawTransparentEnabled = val; }
	virtual bool getDrawTransparentHandle () const { return bDrawTransparentEnabled; }
	virtual void setFreeClick (bool val) { bFreeClick = val; }
	virtual bool getFreeClick () const { return bFreeClick; }
	virtual void setOffsetHandle (const CPoint& val);
	virtual CPoint getOffsetHandle () const { return offsetHandle; }
	virtual void setOffset (const CPoint& val) { offset = val; }
	virtual CPoint getOffset () const { return offset; }

	virtual void setStyle (long style);
	virtual long getStyle () const { return style; }

	virtual void     setHandle (CBitmap* pHandle);
	virtual CBitmap* getHandle () const { return pHandle; }

	virtual void  setZoomFactor (float val) { zoomFactor = val; }
	virtual float getZoomFactor () const { return zoomFactor; }
	//@}

	// overrides
	virtual void draw (CDrawContext*);

	virtual CMouseEventResult onMouseDown (CPoint& where, const long& buttons);
	virtual CMouseEventResult onMouseUp (CPoint& where, const long& buttons);
	virtual CMouseEventResult onMouseMoved (CPoint& where, const long& buttons);

	virtual bool onWheel (const CPoint& where, const float& distance, const long& buttons);
	virtual long onKeyDown (VstKeyCode& keyCode);

	virtual bool sizeToFit ();

	CLASS_METHODS(CSlider, CControl)
protected:
	~CSlider ();
	void setViewSize (CRect& rect, bool invalid);
	
	CPoint   offset; 
	CPoint   offsetHandle;

	CBitmap* pHandle;

	long	style;

	CCoord	widthOfSlider;
	CCoord	heightOfSlider;
	CCoord	rangeHandle;
	CCoord	minTmp;
	CCoord	maxTmp;
	CCoord	minPos;
	CCoord	widthControl;
	CCoord	heightControl;
	float	zoomFactor;

	bool     bDrawTransparentEnabled;
	bool     bFreeClick;

private:
	CCoord   delta;
	float    oldVal;
	long     oldButton; 
};

//-----------------------------------------------------------------------------
// CVerticalSlider Declaration
//! @brief a vertical slider control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CVerticalSlider : public CSlider
{
public:
	CVerticalSlider (const CRect& size, CControlListener* listener, long tag, long iMinPos, long iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const long style = kBottom);
	CVerticalSlider (const CRect& rect, CControlListener* listener, long tag, const CPoint& offsetHandle, long rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const long style = kBottom);
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
	CHorizontalSlider (const CRect& size, CControlListener* listener, long tag, long iMinPos, long iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const long style = kRight);
	CHorizontalSlider (const CRect& rect, CControlListener* listener, long tag, const CPoint& offsetHandle, long rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const long style = kRight);
	CHorizontalSlider (const CHorizontalSlider& slider);
};

} // namespace

#endif