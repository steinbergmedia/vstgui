// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

/**
@page uidescription_attributes UI XML Attribute Definitions

@section viewclasses View Classes and their attributes
- @ref cview @n
- @ref cviewcontainer @n
- @ref ccontrol @n
- @ref conoffbutton @n
- @ref cparamdisplay @n
- @ref ctextlabel @n
- @ref ctextedit @n
- @ref ctextbutton @n
- @ref csegmentbutton @n
- @ref cknob @n
- @ref cverticalswitch @n
- @ref chorizontalswitch @n
- @ref crockerswitch @n
- @ref cmoviebitmap @n
- @ref ckickbutton @n
- @ref cslider @n
- @ref cvumeter @n
- @ref coptionmenu @n
- @ref uiviewswitchcontainer @n

@section cview CView
Declaration:
@verbatim <view class="CView" /> @endverbatim

Attributes:
- \b origin [Point]
- \b size [Point]
- \b transparent [true/false]
- \b bitmap [bitmap name]
- \b autosize [combination of <i>left</i>, <i>top</i>, <i>right</i>, <i>bottom</i>, <i>row</i>, or <i>column</i> see VSTGUI::CViewAutosizing]
- \b tooltip [tooltip text]

@section cviewcontainer CViewContainer
Declaration:
@verbatim <view class="CViewContainer" /> @endverbatim

Inherites attributes from @ref cview @n

Attributes:
- \b background-color [color]

@section ccontrol CControl
A CControl is an abstract class and can not be created directly

Inherites attributes from @ref cview @n

Attributes:
- \b control-tag [tag name]
- \b default-value [float]
- \b min-value [float]
- \b max-value [float]
- \b wheel-inc-value [float]
- \b background-offset [Point]

@section conoffbutton COnOffButton
Declaration:
@verbatim <view class="COnOffButton" /> @endverbatim

Inherites attributes from @ref ccontrol @n

@section cparamdisplay CParamDisplay
Declaration:
@verbatim <view class="CParamDisplay" /> @endverbatim

Inherites attributes from @ref ccontrol @n

Attributes:
- \b font [font name]
- \b font-color [color]
- \b back-color [color]
- \b frame-color [color]
- \b frame-width [float]
- \b round-rect-radius [float]
- \b shadow-color [color]
- \b font-antialias [true/false]
- \b style-3D-in [true/false]
- \b style-3D-out [true/false]
- \b style-no-frame [true/false]
- \b style-no-text [true/false]
- \b style-no-draw [true/false]
- \b style-round-rect [true/false]
- \b style-shadow-text [true/false]
- \b text-alignment [left/center/right]
- \b text-rotation [float]
- \b value-precision [integer]

@section ctextlabel CTextLabel
Declaration:
@verbatim <view class="CTextLabel" /> @endverbatim

Inherites attributes from @ref cparamdisplay @n

Attributes:
- \b title [string]

@section ctextedit CTextEdit
Declaration:
@verbatim <view class="CTextEdit" /> @endverbatim

Inherites attributes from @ref cparamdisplay @n

Attributes:
- \b title [string]

@section cknob CKnob
Declaration:
@verbatim <view class="CKnob" /> @endverbatim

Inherites attributes from @ref ccontrol @n

Attributes:
- \b angle-start [float]
- \b angle-range [float]
- \b value-inset [int]
- \b zoom-factor [float]
- \b handle-shadow-color [color]
- \b handle-color [color]
- \b handle-bitmap [bitmap name]

@section canimknob CAnimKnob
Declaration:
@verbatim <view class="CAnimKnob" /> @endverbatim

Inherites attributes from @ref cknob @n

Attributes:
- \b height-of-one-image [int]

@section cverticalswitch CVerticalSwitch
Declaration:
@verbatim <view class="CVerticalSwitch" /> @endverbatim

Inherites attributes from @ref ccontrol @n

Attributes:
- \b height-of-one-image [int]

@section chorizontalswitch CHorizontalSwitch
Declaration:
@verbatim <view class="CHorizontalSwitch" /> @endverbatim

Inherites attributes from @ref ccontrol @n

Attributes:
- \b height-of-one-image [int]

@section crockerswitch CRockerSwitch
Declaration:
@verbatim <view class="CRockerSwitch" /> @endverbatim

Inherites attributes from @ref ccontrol @n

Attributes:
- \b height-of-one-image [int]

@section cmoviebitmap CMovieBitmap
Declaration:
@verbatim <view class="CMovieBitmap" /> @endverbatim

Inherites attributes from @ref ccontrol @n

Attributes:
- \b height-of-one-image [int]

@section cmoviebutton CMovieButton
Declaration:
@verbatim <view class="CMovieButton" /> @endverbatim

Inherites attributes from @ref ccontrol @n

Attributes:
- \b height-of-one-image [int]

@section ckickbutton CKickButton
Declaration:
@verbatim <view class="CKickButton" /> @endverbatim

Inherites attributes from @ref ccontrol @n

Attributes:
- \b height-of-one-image [int]

@section ctextbutton CTextButton
Declaration:
@verbatim <view class="CTextButton" /> @endverbatim

Inherits attributes from @ref ccontrol @n

New style (named) gradients are supported by:
- \b gradient
- \b gradient-highlighted

Old style (parametric) gradients are supported by:
- \b gradient-start-color
- \b gradient-end-color
- \b gradient-start-color-hightlighted
- \b gradient-end-color-highlighted

When named gradients are used, the parametric gradient information is discarded.

Attributes:
- \b title [string]
- \b font [string]
- \b text-alignment [left/right/center]
- \b text-color [color string]
- \b text-color-highlighted [color string]
- \b frame-color [color string]
- \b frame-color-highlighted [color string]
- \b frame-width [float]
- \b round-radius [float]
- \b icon-text-margin [float]
- \b kick-style [true/false]
- \b icon [string]
- \b icon-highlighted [string]
- \b icon-position [left/right/center above text/center below text]
- \b gradient [string]
- \b gradient-highlighted [string]
- \b gradient-start-color [color string]
- \b graident-end-color [color string]
- \b gradient-start-color-hightlighted [color string]
- \b gradient-end-color-highlighted [color string]

@section csegmentbutton CSegmentButton
Declaration:
@verbatim <view class="CSegmentButton" /> @endverbatim

Inherits attributes from @ref ccontrol @n

note: a string array is a comma seperated string: "one,two,three".

Attributes:
- \b font [string]
- \b style [horizontal/vertical]
- \b text-alignment [left/right/center]
- \b text-color [color string]
- \b text-color-highlighted [color string]
- \b frame-color [color string]
- \b frame-color-highlighted [color string]
- \b frame-width [float]
- \b round-radius [float]
- \b icon-text-margin [float]
- \b gradient [string]
- \b gradient-highlighted [string]
- \b segment-names [string array]
- \b truncate-mode [head/tail/none]

@section cslider CSlider
Declaration:
@verbatim <view class="CSlider" /> @endverbatim

Inherites attributes from @ref ccontrol @n

Attributes:
- \b transparent-handle [true/false]
- \b free-click [true/false]
- \b handle-bitmap [bitmap name]
- \b handle-offset [Point]
- \b mode [touch/relative touch/free click]
- \b draw-frame [true/false]
- \b draw-back [true/false]
- \b draw-value [true/false]
- \b draw-value-inverted [true/false]
- \b draw-value-from-center [true/false]
- \b draw-back-color [color string]
- \b draw-value-color [color string]
- \b bitmap-offset [Point]
- \b zoom-factor [float]
- \b orientation [vertical/horizontal]
- \b reverse-orientation [true/false]

@section coptionmenu COptionMenu
Declaration:
@verbatim <view class="COptionMenu" /> @endverbatim

Inherites attributes from @ref cparamdisplay @n

Attributes:
- \b menu-popup-style [true/false]
- \b menu-check-style [true/false]

@section cvumeter CVuMeter
Declaration:
@verbatim <view class="CVuMeter" /> @endverbatim

Inherites attributes from @ref cviewcontainer @n

Attributes:
- \b off-bitmap [bitmap name]
- \b num-led [integer]
- \b orientation [vertical/horizontal]
- \b decrease-step-value [float]

@section uiviewswitchcontainer UIViewSwitchContainer
Declaration:
@verbatim <view class="UIViewSwitchContainer" /> @endverbatim

- \b template-names [string array]
- \b template-switch-control [tag name]
- \b animation-style [fade/move/push]
- \b animation-time [integer]

@cond ignore
*/

/*
class CViewCreator : public ViewCreatorAdapter
{
public:
	IdStringPtr getViewName () const { return "CView"; }
	IdStringPtr getBaseViewName () const { return 0; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CView (CRect (0, 0, 0, 0)); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		auto* control = dynamic_cast<CControl*> (view);
		if (control == 0)
			return false;
		return true;
	}
	bool getAttributeNames (StringList& attributeNames) const
	{
		attributeNames.emplace_back ("empty");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "empty") return kUnknownType;
		else if (attributeName == "empty2") return kUnknownType;
		return kUnknownType;
	}

};
*/

#include "iviewfactory.h"
#include "iviewcreator.h"
#include "detail/uiviewcreatorattributes.h"
#include "uiviewcreator.h"
#include "uiviewfactory.h"
#include "uiviewswitchcontainer.h"
#include "uiattributes.h"
#include "uidescription.h"

#include "viewcreator/animationsplashscreencreator.h"
#include "viewcreator/animknobcreator.h"
#include "viewcreator/autoanimationcreator.h"
#include "viewcreator/checkboxcreator.h"
#include "viewcreator/controlcreator.h"
#include "viewcreator/gradientviewcreator.h"
#include "viewcreator/kickbuttoncreator.h"
#include "viewcreator/knobcreator.h"
#include "viewcreator/layeredviewcontainercreator.h"
#include "viewcreator/moviebitmapcreator.h"
#include "viewcreator/moviebuttoncreator.h"
#include "viewcreator/multibitmapcontrolcreator.h"
#include "viewcreator/multilinetextlabelcreator.h"
#include "viewcreator/onoffbuttoncreator.h"
#include "viewcreator/optionmenucreator.h"
#include "viewcreator/paramdisplaycreator.h"
#include "viewcreator/rockerswitchcreator.h"
#include "viewcreator/rowcolumnviewcreator.h"
#include "viewcreator/scrollviewcreator.h"
#include "viewcreator/searchtexteditcreator.h"
#include "viewcreator/segmentbuttoncreator.h"
#include "viewcreator/shadowviewcontainercreator.h"
#include "viewcreator/sliderviewcreator.h"
#include "viewcreator/splitviewcreator.h"
#include "viewcreator/stringlistcontrolcreator.h"
#include "viewcreator/switchcreators.h"
#include "viewcreator/textbuttoncreator.h"
#include "viewcreator/texteditcreator.h"
#include "viewcreator/textlabelcreator.h"
#include "viewcreator/uiviewswitchcontainercreator.h"
#include "viewcreator/viewcontainercreator.h"
#include "viewcreator/viewcreator.h"
#include "viewcreator/vumetercreator.h"
#include "viewcreator/xypadcreator.h"

#include "../vstgui.h"
#include <sstream>
#include <array>

namespace VSTGUI {
namespace UIViewCreator {

#if VSTGUI_ENABLED_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
bool parseSize (const std::string& str, CPoint& point)
{
	return UIAttributes::stringToPoint (str, point);
}

//-----------------------------------------------------------------------------
bool pointToString (const CPoint& p, std::string& string)
{
	string = UIAttributes::pointToString (p);
	return true;
}
#endif

//-----------------------------------------------------------------------------
bool bitmapToString (CBitmap* bitmap, std::string& string, const IUIDescription* desc)
{
	UTF8StringPtr bitmapName = desc->lookupBitmapName (bitmap);
	if (bitmapName)
		string = bitmapName;
	else
	{
		const CResourceDescription& res = bitmap->getResourceDescription ();
		if (res.type == CResourceDescription::kStringType)
			string = res.u.name;
		else
			string = UIAttributes::integerToString (res.u.id);
	}
	return true;
}

//-----------------------------------------------------------------------------
bool colorToString (const CColor& color, std::string& string, const IUIDescription* desc)
{
	UTF8StringPtr colorName = desc ? desc->lookupColorName (color) : nullptr;
	if (colorName)
		string = colorName;
	else
	{
		uint8_t red = color.red;
		uint8_t green = color.green;
		uint8_t blue = color.blue;
		uint8_t alpha = color.alpha;
		char strBuffer[10];
		snprintf (strBuffer, 10, "#%02x%02x%02x%02x", red, green, blue, alpha);
		string = strBuffer;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool stringToColor (const std::string* value, CColor& color, const IUIDescription* desc)
{
	if (value && *value == "")
	{
		color = kTransparentCColor;
		return true;
	}
	return value ? desc->getColor (value->c_str (), color) : false;
}

//-----------------------------------------------------------------------------
bool stringToBitmap (const std::string* value, CBitmap*& bitmap, const IUIDescription* desc)
{
	if (value)
	{
		if (*value == "")
			bitmap = nullptr;
		else
			bitmap = desc->getBitmap (value->c_str ());
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void applyStyleMask (const std::string* value, int32_t mask, int32_t& style)
{
	if (value)
	{
		setBit (style, mask, *value == strTrue);
	}
}

//------------------------------------------------------------------------
void addGradientToUIDescription (const IUIDescription* description, CGradient* gradient, UTF8StringPtr baseName)
{
	if (!description->lookupGradientName (gradient))
	{
		auto* uiDesc = dynamic_cast<UIDescription*>(const_cast<IUIDescription*> (description));
		if (uiDesc)
		{
			uint32_t index = 0;
			std::stringstream str;
			do {
				index++;
				str.str ("");
				str << baseName;
				if (index > 1)
				{
					str << " ";
					str << index;
				}
			} while (description->getGradient (str.str ().c_str ()) != nullptr);
			uiDesc->changeGradient (str.str ().c_str (), gradient);
		}
	}
}

//-----------------------------------------------------------------------------
bool getStandardAttributeListValues (const std::string& attributeName, std::list<const std::string*>& values)
{
	if (attributeName == kAttrOrientation)
	{
		static std::string kHorizontal = strHorizontal;
		static std::string kVertical = strVertical;
		
		values.emplace_back (&kHorizontal);
		values.emplace_back (&kVertical);
		return true;
	}
	else if (attributeName == kAttrTruncateMode)
	{
		static std::string kNone = strNone;
		static std::string kHead = strHead;
		static std::string kTail = strTail;
		
		values.emplace_back (&kNone);
		values.emplace_back (&kHead);
		values.emplace_back (&kTail);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
AnimationSplashScreenCreator __gAnimationSplashScreenCreator;
AnimKnobCreator __gAnimKnobCreator;
AutoAnimationCreator __gAutoAnimationCreator;
CheckBoxCreator __gCheckBoxCreator;
ControlCreator __gControlCreator;
GradientViewCreator __gGradientViewCreator;
KickButtonCreator __gKickButtonCreator;
KnobCreator __gKnobCreator;
LayeredViewContainerCreator __gLayeredViewContainerCreator;
MovieBitmapCreator __gMovieBitmapCreator;
MovieButtonCreator __gMovieButtonCreator;
MultiLineTextLabelCreator __gMultiLineTextLabelCreator;
OnOffButtonCreator __gOnOffButtonCreator;
OptionMenuCreator __gOptionMenuCreator;
ParamDisplayCreator __gParamDisplayCreator;
RowColumnViewCreator __gRowColumnViewCreator;
ScrollViewCreator __gScrollViewCreator;
SearchTextEditCreator __gSearchTextEditCreator;
SegmentButtonCreator __gSegmentButtonCreator;
ShadowViewContainerCreator __gShadowViewContainerCreator;
SliderCreator __gSliderCreator;
SplitViewCreator __gSplitViewCreator;
StringListControlCreator __gStringListControlCreator;
TextButtonCreator __gTextButtonCreator;
TextEditCreator __gTextEditCreator;
TextLabelCreator __gTextLabelCreator;
UIViewSwitchContainerCreator __gUIViewSwitchContainerCreator;
ViewContainerCreator __gViewContainerCreator;
ViewCreator __gViewCreator;
VuMeterCreator __gVuMeterCreator;
XYPadCreator __gCXYPadCreator;
VerticalSwitchCreator __gVerticalSwitchCreator;
HorizontalSwitchCreator __gHorizontalSwitchCreator;
RockerSwitchCreator __gRockerSwitchCreator;

//------------------------------------------------------------------------
}} // VSTGUI

/**
@endcond ignore
*/
