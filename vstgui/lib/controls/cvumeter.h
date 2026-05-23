// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../enumbitset.h"
#include "../cbitmap.h"
#include "ccontrol.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CVuMeter Declaration
//!
/// @ingroup controls
//-----------------------------------------------------------------------------
class CVuMeter : public CControl
{
public:
	enum class Style : int32_t
	{
		kHorizontal,
		kVertical,
	};

	CVuMeter (const CRect& size, const SharedPointer<CBitmap>& onBitmap,
			  const SharedPointer<CBitmap>& offBitmap, int32_t nbLed,
			  Style style = Style::kVertical);
	CVuMeter (const CVuMeter& vuMeter);
  
	//-----------------------------------------------------------------------------
	/// @name CVuMeter Methods
	//-----------------------------------------------------------------------------
	//@{
	float getDecreaseStepValue () const { return decreaseValue; }
	virtual void setDecreaseStepValue (float value) { decreaseValue = value; }

	SharedPointer<CBitmap> getOnBitmap () const { return getBackground (); }
	SharedPointer<CBitmap> getOffBitmap () const { return offBitmap; }
	void setOnBitmap (const SharedPointer<CBitmap>& bitmap) { setBackground (bitmap); }
	void setOffBitmap (const SharedPointer<CBitmap>& bitmap);

	int32_t getNbLed () const { return nbLed; }
	void setNbLed (int32_t nb) { nbLed = nb; invalid (); }

	void setStyle (Style newStyle)
	{
		style = newStyle;
		invalid ();
	}
	Style getStyle () const { return style; }
	//@}

	// overrides
	void draw (CDrawContext& context) override;
	void setViewSize (const CRect& newSize, bool invalid = true) override;
	bool sizeToFit () override;
	void onIdle () override;
	
	CLASS_METHODS(CVuMeter, CControl)
protected:
	VSTGUI_SHAREDPTR_FRIEND (CVuMeter)
	~CVuMeter () noexcept override;

	SharedPointer<CBitmap> offBitmap;

	int32_t nbLed;
	Style style;
	float decreaseValue;

	CRect rectOn;
	CRect rectOff;
};

} // VSTGUI
