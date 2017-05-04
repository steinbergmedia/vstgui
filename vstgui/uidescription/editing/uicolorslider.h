// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uicolorslider__
#define __uicolorslider__

#include "../../lib/vstguibase.h"

#if VSTGUI_LIVE_EDITING

#include "../../lib/controls/cslider.h"

namespace VSTGUI {
class UIColor;

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class UIColorSlider : public CSlider
{
public:
	enum {
		kHue,
		kSaturation,
		kLightness,
		kRed,
		kGreen,
		kBlue,
		kAlpha
	};
	UIColorSlider (UIColor* color, int32_t style);
	~UIColorSlider () override;

protected:
	void draw (CDrawContext* context) override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;
	void updateBackground (CDrawContext* context);
	void updateHandle (CDrawContext* context);

	SharedPointer<UIColor> color;
	int32_t style;
};


} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uicolorslider__
