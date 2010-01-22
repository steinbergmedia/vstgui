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

#ifndef __csplashscreen__
#define __csplashscreen__

#include "ccontrol.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CSplashScreen Declaration
//!
/// @ingroup views
//-----------------------------------------------------------------------------
class CSplashScreen : public CControl, public CControlListener
{
public:
	CSplashScreen (const CRect& size, CControlListener* listener, long tag, CBitmap* background, const CRect& toDisplay, const CPoint& offset = CPoint (0, 0));
	CSplashScreen (const CRect& size, CControlListener* listener, long tag, CView* splashView);
	CSplashScreen (const CSplashScreen& splashScreen);
  
	virtual void draw (CDrawContext*);
	virtual bool hitTest (const CPoint& where, const long buttons = -1);

	//-----------------------------------------------------------------------------
	/// @name CSplashScreen Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void unSplash ();

	virtual void setDisplayArea (const CRect& rect)  { toDisplay = rect; }				///< set the area in which the splash will be displayed
	virtual CRect& getDisplayArea (CRect& rect) const { rect = toDisplay; return rect; }	///< get the area in which the splash will be displayed
	//@}

	virtual CMouseEventResult onMouseDown (CPoint& where, const long& buttons);

	CLASS_METHODS(CSplashScreen, CControl)
protected:
	~CSplashScreen ();	
	void valueChanged (CControl *pControl);

	CRect	toDisplay;
	CRect	keepSize;
	CPoint	offset;
	CView* modalView;
};

} // namespace

#endif