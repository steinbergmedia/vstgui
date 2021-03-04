// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguifwd.h"
#include "../cviewcontainer.h"
#include "icontrollistener.h"
#include "ctextedit.h"

namespace VSTGUI {
/// @cond ignore
namespace CColorChooserInternal {
class ColorView;
}
/// @endcond

///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class IColorChooserDelegate
{
public:
	virtual void colorChanged (CColorChooser* chooser, const CColor& color) = 0;
	virtual void onBeginColorChange (CColorChooser* chooser) = 0;
	virtual void onEndColorChange (CColorChooser* chooser) = 0;
};

//-----------------------------------------------------------------------------
struct CColorChooserUISettings
{
	CFontRef font {kNormalFont};
	CColor fontColor {kWhiteCColor};
	CColor checkerBoardColor1 {kWhiteCColor};
	CColor checkerBoardColor2 {kBlackCColor};
	CPoint margin {5, 5};

	bool checkerBoardBack {true};
};

///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class CColorChooser : public CViewContainer, public IControlListener
{
public:
	CColorChooser (IColorChooserDelegate* delegate = nullptr, const CColor& initialColor = kTransparentCColor, const CColorChooserUISettings& settings = CColorChooserUISettings ());
	~CColorChooser () noexcept override = default;

	void setColor (const CColor& newColor);
//-----------------------------------------------------------------------------
protected:
	void valueChanged (CControl* pControl) override;
	void controlBeginEdit (CControl* pControl) override;
	void controlEndEdit (CControl* pControl) override;
	void updateState ();

	/// @cond ignore

	IColorChooserDelegate* delegate;
	CColor color;
	
	CSlider* redSlider;
	CSlider* greenSlider;
	CSlider* blueSlider;
	CSlider* hueSlider;
	CSlider* saturationSlider;
	CSlider* brightnessSlider;
	CSlider* alphaSlider;
	CTextEdit* editFields[8];
	CColorChooserInternal::ColorView* colorView;

	//-----------------------------------------------------------------------------
	enum {
		kRedTag = 10000,
		kGreenTag,
		kBlueTag,
		kHueTag,
		kSaturationTag,
		kBrightnessTag,
		kAlphaTag,
		kColorTag
	};

	//-----------------------------------------------------------------------------
	static bool convertNormalized (UTF8StringPtr string, float& output, CTextEdit::StringToValueUserData* userData);
	static bool convertColorValue (UTF8StringPtr string, float& output, CTextEdit::StringToValueUserData* userData);
	static bool convertAngle (UTF8StringPtr string, float& output, CTextEdit::StringToValueUserData* userData);
	static bool convertNormalizedToString (float value, char string[256], CParamDisplay::ValueToStringUserData* userData);
	static bool convertColorValueToString (float value, char string[256], CParamDisplay::ValueToStringUserData* userData);
	static bool convertAngleToString (float value, char string[256], CParamDisplay::ValueToStringUserData* userData);
	/// @endcond

};

} // VSTGUI
