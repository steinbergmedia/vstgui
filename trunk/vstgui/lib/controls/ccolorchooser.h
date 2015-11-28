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

#ifndef __ccolorchooser__
#define __ccolorchooser__

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
};

//-----------------------------------------------------------------------------
struct CColorChooserUISettings
{
	CFontRef font;
	CColor fontColor;
	CColor checkerBoardColor1;
	CColor checkerBoardColor2;
	CPoint margin;

	bool checkerBoardBack;
	
	CColorChooserUISettings ();
};

///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class CColorChooser : public CViewContainer, public IControlListener, public IDependency
{
public:
	CColorChooser (IColorChooserDelegate* delegate = 0, const CColor& initialColor = kTransparentCColor, const CColorChooserUISettings& settings = CColorChooserUISettings ());
	~CColorChooser ();

	void setColor (const CColor& newColor);
	
	static IdStringPtr kMsgBeginColorChange;
	static IdStringPtr kMsgEndColorChange;
//-----------------------------------------------------------------------------
protected:
	void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
	void controlBeginEdit (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
	void controlEndEdit (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
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

} // namespace

#endif
