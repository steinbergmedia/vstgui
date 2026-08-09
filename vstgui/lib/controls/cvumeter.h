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

	CVuMeter (const CRect& size, const SPtr<CBitmap>& onBitmap, const SPtr<CBitmap>& offBitmap,
			  int32_t nbLed, Style style = Style::kVertical);

	//-----------------------------------------------------------------------------
	/// @name CVuMeter Methods
	//-----------------------------------------------------------------------------
	//@{
	float getDecreaseStepValue () const { return decreaseValue; }
	virtual void setDecreaseStepValue (float value) { decreaseValue = value; }

	SPtr<CBitmap> getOnBitmap () const { return getBackground (); }
	SPtr<CBitmap> getOffBitmap () const { return offBitmap; }
	void setOnBitmap (const SPtr<CBitmap>& bitmap) { setBackground (bitmap); }
	void setOffBitmap (const SPtr<CBitmap>& bitmap);

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

protected:
	VSTGUI_SHAREDPTR_FRIEND (CVuMeter)
	~CVuMeter () noexcept override;

	SPtr<CBitmap> offBitmap;

	int32_t nbLed;
	Style style;
	float decreaseValue;

	CRect rectOn;
	CRect rectOff;
};

} // VSTGUI
