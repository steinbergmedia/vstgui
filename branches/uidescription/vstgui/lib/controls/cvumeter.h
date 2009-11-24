//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2009, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __cvumeter__
#define __cvumeter__

#include "ccontrol.h"

namespace VSTGUI {
class COffscreenContext;

//-----------------------------------------------------------------------------
// CVuMeter Declaration
//!
/// @ingroup controls
//-----------------------------------------------------------------------------
class CVuMeter : public CControl
{
public:
	CVuMeter (const CRect& size, CBitmap* onBitmap, CBitmap* offBitmap, long nbLed, long style = kVertical);
	CVuMeter (const CVuMeter& vuMeter);
  
	//-----------------------------------------------------------------------------
	/// @name CVuMeter Methods
	//-----------------------------------------------------------------------------
	//@{
	float getDecreaseStepValue () const { return decreaseValue; }
	virtual void setDecreaseStepValue (float value) { decreaseValue = value; }

	CBitmap* getOnBitmap () const { return getBackground (); }
	CBitmap* getOffBitmap () const { return offBitmap; }
	void setOnBitmap (CBitmap* bitmap) { setBackground (bitmap); }
	void setOffBitmap (CBitmap* bitmap);
	
	long getNbLed () const { return nbLed; }
	void setNbLed (long nb) { nbLed = nb; invalid (); }
	
	void setStyle (long newStyle) { style = newStyle; invalid (); }
	long getStyle () const { return style; }

	void setUseOffscreen (bool val = true);
	bool getUseOffscreen () const { return bUseOffscreen; }
	//@}


	virtual bool attached (CView* parent);
	virtual bool removed (CView* parent);
	virtual void draw (CDrawContext* pContext);
	virtual void setDirty (const bool val = true);
	
	CLASS_METHODS(CVuMeter, CControl)
protected:
	~CVuMeter ();	
	CBitmap* onBitmap;
	CBitmap* offBitmap;
	COffscreenContext* pOScreen;
	
	long     nbLed;
	long     style;
	float    decreaseValue;
	bool	 bUseOffscreen;

	CRect    rectOn;
	CRect    rectOff;
};

} // namespace

#endif
