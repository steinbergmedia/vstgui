// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "ccontrol.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CVuMeter Declaration
//!
/// @ingroup controls
//-----------------------------------------------------------------------------
class CVuMeter : public CControl
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

	CVuMeter (const CRect& size, CBitmap* onBitmap, CBitmap* offBitmap, int32_t nbLed, int32_t style = kVertical);
	CVuMeter (const CVuMeter& vuMeter);
  
	//-----------------------------------------------------------------------------
	/// @name CVuMeter Methods
	//-----------------------------------------------------------------------------
	//@{
	float getDecreaseStepValue () const { return decreaseValue; }
	virtual void setDecreaseStepValue (float value) { decreaseValue = value; }

	virtual CBitmap* getOnBitmap () const { return getBackground (); }
	virtual CBitmap* getOffBitmap () const { return offBitmap; }
	virtual void setOnBitmap (CBitmap* bitmap) { setBackground (bitmap); }
	virtual void setOffBitmap (CBitmap* bitmap);
	
	int32_t getNbLed () const { return nbLed; }
	void setNbLed (int32_t nb) { nbLed = nb; invalid (); }
	
	void setStyle (int32_t newStyle) { style = newStyle; invalid (); }
	int32_t getStyle () const { return style; }
	//@}


	// overrides
	void setDirty (bool state) override;
	void draw (CDrawContext* pContext) override;
	void setViewSize (const CRect& newSize, bool invalid = true) override;
	bool sizeToFit () override;
	void onIdle () override;
	
	CLASS_METHODS(CVuMeter, CControl)
protected:
	~CVuMeter () noexcept override;	

	CBitmap* offBitmap;
	
	int32_t     nbLed;
	int32_t     style;
	float    decreaseValue;

	CRect    rectOn;
	CRect    rectOff;
};

} // VSTGUI
