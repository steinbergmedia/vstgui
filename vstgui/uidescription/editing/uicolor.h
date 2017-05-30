// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uicolor__
#define __uicolor__

#include "../../lib/vstguibase.h"

#if VSTGUI_LIVE_EDITING

#include "../../lib/ccolor.h"
#include "../../lib/idependency.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIColor : protected CColor, public CBaseObject, public IDependency
{
public:
	UIColor () : CColor (kTransparentCColor), hue (0), saturation (0), lightness (0) {}

	UIColor& operator= (const CColor& c);
	const CColor& base () const { return *this; }

	double getRed () const { return r; }
	double getGreen () const { return g; }
	double getBlue () const { return b; }
	double getAlpha () const { return alpha; }
	
	double getHue () const { return hue; }
	double getSaturation () const { return saturation; }
	double getLightness () const { return lightness; }
	
	void setHue (double h);
	void setSaturation (double s);
	void setLightness (double l);

	void setRed (double nr);
	void setGreen (double ng);
	void setBlue (double nb);
	void setAlpha (double na);

	void beginEdit ();
	void endEdit ();
	
	static IdStringPtr kMsgChanged;
	static IdStringPtr kMsgEditChange;
	static IdStringPtr kMsgBeginEditing;
	static IdStringPtr kMsgEndEditing;
private:
	enum HSLUpdateDirection
	{
		kFrom,
		kTo
	};

	void updateHSL (HSLUpdateDirection direction);
	
	double hue, saturation, lightness;
	double r, g, b;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uicolor__
