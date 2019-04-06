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
	bool getAttributeNames (std::list<std::string>& attributeNames) const
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
#include "viewcreator/checkboxcreator.h"
#include "viewcreator/controlcreator.h"
#include "viewcreator/gradientviewcreator.h"
#include "viewcreator/layeredviewcontainercreator.h"
#include "viewcreator/onoffbuttoncreator.h"
#include "viewcreator/optionmenucreator.h"
#include "viewcreator/paramdisplaycreator.h"
#include "viewcreator/rowcolumnviewcreator.h"
#include "viewcreator/scrollviewcreator.h"
#include "viewcreator/segmentbuttoncreator.h"
#include "viewcreator/shadowviewcontainercreator.h"
#include "viewcreator/sliderviewcreator.h"
#include "viewcreator/splitviewcreator.h"
#include "viewcreator/uiviewswitchcontainercreator.h"
#include "viewcreator/viewcontainercreator.h"
#include "viewcreator/viewcreator.h"
#include "viewcreator/vumetercreator.h"
#include "viewcreator/xypadviewcreator.h"

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
		sprintf (strBuffer, "#%02x%02x%02x%02x", red, green, blue, alpha);
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
class CTextLabelCreator : public ViewCreatorAdapter
{
public:
	CTextLabelCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCTextLabel; }
	IdStringPtr getBaseViewName () const override { return kCParamDisplay; }
	UTF8StringPtr getDisplayName () const override { return "Label"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CTextLabel (CRect (0, 0, 100, 20)); }
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* label = dynamic_cast<CTextLabel*> (view);
		if (!label)
			return false;

		const std::string* attr = attributes.getAttributeValue (kAttrTitle);
		if (attr)
		{
			auto index = attr->find ("\\n");
			if (index != std::string::npos)
			{
				auto str = *attr;
				while (index != std::string::npos)
				{
					str.replace (index, 2, "\n");
					index = str.find ("\\n");
				}
				label->setText (UTF8String (std::move (str)));
			}
			else
				label->setText (UTF8String (*attr));
		}
		attr = attributes.getAttributeValue (kAttrTruncateMode);
		if (attr)
		{
			if (*attr == strHead)
				label->setTextTruncateMode (CTextLabel::kTruncateHead);
			else if (*attr == strTail)
				label->setTextTruncateMode (CTextLabel::kTruncateTail);
			else
				label->setTextTruncateMode (CTextLabel::kTruncateNone);
		}

		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrTitle);
		attributeNames.emplace_back (kAttrTruncateMode);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrTitle) return kStringType;
		if (attributeName == kAttrTruncateMode) return kListType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* label = dynamic_cast<CTextLabel*> (view);
		if (!label)
			return false;
		if (attributeName == kAttrTitle)
		{
			stringValue = label->getText ().getString ();
			auto index = stringValue.find ("\n");
			while (index != std::string::npos)
			{
				stringValue.replace (index, 1, "\\n");
				index = stringValue.find ("\n");
			}
			return true;
		}
		else if (attributeName == kAttrTruncateMode)
		{
			switch (label->getTextTruncateMode ())
			{
				case CTextLabel::kTruncateHead: stringValue = strHead; break;
				case CTextLabel::kTruncateTail: stringValue = strTail; break;
				case CTextLabel::kTruncateNone: stringValue = ""; break;
			}
			return true;
		}
		return false;
	}
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const override
	{
		if (attributeName == kAttrTruncateMode)
		{
			return getStandardAttributeListValues (kAttrTruncateMode, values);
		}
		return false;
	}

};
CTextLabelCreator __gCTextLabelCreator;

//-----------------------------------------------------------------------------
class CMultiLineTextLabelCreator : public ViewCreatorAdapter
{
public:
	std::string kClip = "clip";
	std::string kTruncate = "truncate";
	std::string kWrap = "wrap";

	CMultiLineTextLabelCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCMultiLineTextLabel; }
	IdStringPtr getBaseViewName () const override { return kCTextLabel; }
	UTF8StringPtr getDisplayName () const override { return "Multiline Label"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CMultiLineTextLabel (CRect (0, 0, 100, 20)); }
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto label = dynamic_cast<CMultiLineTextLabel*> (view);
		if (!label)
			return false;
		
		auto attr = attributes.getAttributeValue (kAttrLineLayout);
		if (attr)
		{
			if (*attr == kTruncate)
				label->setLineLayout (CMultiLineTextLabel::LineLayout::truncate);
			else if (*attr == kWrap)
				label->setLineLayout (CMultiLineTextLabel::LineLayout::wrap);
			else
				label->setLineLayout (CMultiLineTextLabel::LineLayout::clip);
		}
		bool autoHeight;
		if (attributes.getBooleanAttribute (kAttrAutoHeight, autoHeight))
			label->setAutoHeight (autoHeight);
		
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrLineLayout);
		attributeNames.emplace_back (kAttrAutoHeight);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrLineLayout) return kListType;
		if (attributeName == kAttrAutoHeight) return kBooleanType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto label = dynamic_cast<CMultiLineTextLabel*> (view);
		if (!label)
			return false;
		if (attributeName == kAttrLineLayout)
		{
			switch (label->getLineLayout ())
			{
				case CMultiLineTextLabel::LineLayout::truncate: stringValue = kTruncate; break;
				case CMultiLineTextLabel::LineLayout::wrap: stringValue = kWrap; break;
				case CMultiLineTextLabel::LineLayout::clip: stringValue = kClip; break;
			}
			return true;
		}
		else if (attributeName == kAttrAutoHeight)
		{
			stringValue = label->getAutoHeight () ? strTrue : strFalse;
			return true;
		}

		return false;
	}
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const override
	{
		if (attributeName == kAttrLineLayout)
		{
			values.emplace_back (&kClip);
			values.emplace_back (&kTruncate);
			values.emplace_back (&kWrap);
			return true;
		}
		return false;
	}

};
CMultiLineTextLabelCreator __gCMultiLineTextLabelCreator;

//-----------------------------------------------------------------------------
class CTextEditCreator : public ViewCreatorAdapter
{
public:
	CTextEditCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCTextEdit; }
	IdStringPtr getBaseViewName () const override { return kCTextLabel; }
	UTF8StringPtr getDisplayName () const override { return "Text Edit"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CTextEdit (CRect (0, 0, 100, 20), nullptr, -1); }
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* label = dynamic_cast<CTextEdit*> (view);
		if (!label)
			return false;

		bool b;
		if (attributes.getBooleanAttribute (kAttrSecureStyle, b))
			label->setSecureStyle (b);
		if (attributes.getBooleanAttribute (kAttrImmediateTextChange, b))
			label->setImmediateTextChange (b);

		int32_t style = label->getStyle ();
		applyStyleMask (attributes.getAttributeValue (kAttrStyleDoubleClick), CTextEdit::kDoubleClickStyle, style);
		label->setStyle (style);

		if (auto placeholder = attributes.getAttributeValue (kAttrPlaceholderTitle))
			label->setPlaceholderString (placeholder->c_str ());

		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrSecureStyle);
		attributeNames.emplace_back (kAttrImmediateTextChange);
		attributeNames.emplace_back (kAttrStyleDoubleClick);
		attributeNames.emplace_back (kAttrPlaceholderTitle);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrSecureStyle) return kBooleanType;
		if (attributeName == kAttrImmediateTextChange) return kBooleanType;
		if (attributeName == kAttrStyleDoubleClick) return kBooleanType;
		if (attributeName == kAttrPlaceholderTitle) return kStringType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* label = dynamic_cast<CTextEdit*> (view);
		if (!label)
			return false;
		if (attributeName == kAttrSecureStyle)
		{
			stringValue = label->getSecureStyle () ? strTrue : strFalse;
			return true;
		}
		if (attributeName == kAttrImmediateTextChange)
		{
			stringValue = label->getImmediateTextChange () ? strTrue : strFalse;
			return true;
		}
		if (attributeName == kAttrStyleDoubleClick)
		{
			stringValue = label->getStyle () & CTextEdit::kDoubleClickStyle ? strTrue : strFalse;
			return true;
		}
		if (attributeName == kAttrPlaceholderTitle)
		{
			stringValue = label->getPlaceholderString ().getString ();
			return true;
		}
		
		return false;
	}

};
CTextEditCreator __gCTextEditCreator;

//------------------------------------------------------------------------
class CSearchTextEditCreator : public ViewCreatorAdapter
{
public:
	CSearchTextEditCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCSearchTextEdit; }
	IdStringPtr getBaseViewName () const override { return kCTextEdit; }
	UTF8StringPtr getDisplayName () const override { return "Search Text Edit"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override
	{
		return new CSearchTextEdit (CRect (0, 0, 100, 20), nullptr, -1);
	}
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto ste = dynamic_cast<CSearchTextEdit*>(view);
		if (!ste)
			return false;
		CPoint p;
		if (attributes.getPointAttribute (kAttrClearMarkInset, p))
			ste->setClearMarkInset (p);
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrClearMarkInset);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrClearMarkInset)
			return kPointType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto ste = dynamic_cast<CSearchTextEdit*>(view);
		if (!ste)
			return false;
		if (attributeName == kAttrClearMarkInset)
		{
			stringValue = UIAttributes::pointToString (ste->getClearMarkInset ());
			return true;
		}
		return false;
	}
};
CSearchTextEditCreator __gCSearchTextEditCreator;

//-----------------------------------------------------------------------------
class CTextButtonCreator : public ViewCreatorAdapter
{
public:
	std::array<std::string, 4> positionsStrings = {
	    {strLeft, "center above text", "center below text", strRight}};

	CTextButtonCreator () { UIViewFactory::registerViewCreator (*this); vstgui_assert (positionsStrings.size () == CDrawMethods::kIconRight + 1); }
	IdStringPtr getViewName () const override { return kCTextButton; }
	IdStringPtr getBaseViewName () const override { return kCControl; }
	UTF8StringPtr getDisplayName () const override { return "Text Button"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override
	{
		CTextButton* button = new CTextButton (CRect (0, 0, 100, 20), nullptr, -1, "");
		if (!description->lookupGradientName (button->getGradient ()))
			addGradientToUIDescription (description, button->getGradient (), "Default TextButton Gradient");
		if (!description->lookupGradientName (button->getGradientHighlighted ()))
			addGradientToUIDescription (description, button->getGradientHighlighted (), "Default TextButton Gradient Highlighted");
		return button;
	}
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* button = dynamic_cast<CTextButton*> (view);
		if (!button)
			return false;

		const std::string* attr = attributes.getAttributeValue (kAttrTitle);
		if (attr)
			button->setTitle (attr->c_str ());

		attr = attributes.getAttributeValue (kAttrFont);
		if (attr)
		{
			CFontRef font = description->getFont (attr->c_str ());
			if (font)
			{
				button->setFont (font);
			}
		}

		CColor color;
		if (stringToColor (attributes.getAttributeValue (kAttrTextColor), color, description))
			button->setTextColor (color);
		if (stringToColor (attributes.getAttributeValue (kAttrTextColorHighlighted), color, description))
			button->setTextColorHighlighted (color);
		if (stringToColor (attributes.getAttributeValue (kAttrFrameColor), color, description))
			button->setFrameColor (color);
		if (stringToColor (attributes.getAttributeValue (kAttrFrameColorHighlighted), color, description))
			button->setFrameColorHighlighted (color);

		double d;
		if (attributes.getDoubleAttribute (kAttrFrameWidth, d))
			button->setFrameWidth (d);
		if (attributes.getDoubleAttribute (kAttrRoundRadius, d))
			button->setRoundRadius (d);
		if (attributes.getDoubleAttribute (kAttrIconTextMargin, d))
			button->setTextMargin (d);
		
		attr = attributes.getAttributeValue (kAttrKickStyle);
		if (attr)
		{
			button->setStyle (*attr == strTrue ? CTextButton::kKickStyle : CTextButton::kOnOffStyle);
		}

		CBitmap* bitmap;
		if (stringToBitmap (attributes.getAttributeValue (kAttrIcon), bitmap, description))
			button->setIcon (bitmap);
		if (stringToBitmap (attributes.getAttributeValue (kAttrIconHighlighted), bitmap, description))
			button->setIconHighlighted (bitmap);

		attr = attributes.getAttributeValue (kAttrIconPosition);
		if (attr)
		{
			auto it = std::find (positionsStrings.begin (), positionsStrings.end (), *attr);
			if (it != positionsStrings.end ())
			{
				auto pos = std::distance (positionsStrings.begin (), it);
				button->setIconPosition (static_cast<CDrawMethods::IconPosition> (pos));
			}
		}
		attr = attributes.getAttributeValue (kAttrTextAlignment);
		if (attr)
		{
			CHoriTxtAlign align = kCenterText;
			if (*attr == strLeft)
				align = kLeftText;
			else if (*attr == strRight)
				align = kRightText;
			button->setTextAlignment (align);
		}
		const std::string* gradientName = attributes.getAttributeValue (kAttrGradient);
		if (gradientName)
			button->setGradient (description->getGradient (gradientName->c_str ()));
		const std::string* gradientHighlightedName = attributes.getAttributeValue (kAttrGradientHighlighted);
		if (gradientHighlightedName)
			button->setGradientHighlighted (description->getGradient (gradientHighlightedName->c_str ()));

		if (gradientName == nullptr && gradientHighlightedName == nullptr)
		{
			bool hasOldGradient = true;
			CColor startColor, highlightedStartColor, endColor, highlightedEndColor;
			if (!stringToColor (attributes.getAttributeValue (kAttrGradientStartColor), startColor, description))
				hasOldGradient = false;
			if (hasOldGradient && !stringToColor (attributes.getAttributeValue (kAttrGradientStartColorHighlighted), highlightedStartColor, description))
				hasOldGradient = false;
			if (hasOldGradient && !stringToColor (attributes.getAttributeValue (kAttrGradientEndColor), endColor, description))
				hasOldGradient = false;
			if (hasOldGradient && !stringToColor (attributes.getAttributeValue (kAttrGradientEndColorHighlighted), highlightedEndColor, description))
				hasOldGradient = false;
			if (hasOldGradient)
			{
				SharedPointer<CGradient> gradient = owned (CGradient::create (0, 1, startColor, endColor));
				button->setGradient (gradient);
				addGradientToUIDescription (description, gradient, "TextButton");
				gradient = owned (CGradient::create (0, 1, highlightedStartColor, highlightedEndColor));
				button->setGradientHighlighted (gradient);
				addGradientToUIDescription (description, gradient, "TextButton Highlighted");
			}
		}

		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrKickStyle);
		attributeNames.emplace_back (kAttrTitle);
		attributeNames.emplace_back (kAttrFont);
		attributeNames.emplace_back (kAttrTextColor);
		attributeNames.emplace_back (kAttrTextColorHighlighted);
		attributeNames.emplace_back (kAttrGradient);
		attributeNames.emplace_back (kAttrGradientHighlighted);
		attributeNames.emplace_back (kAttrFrameColor);
		attributeNames.emplace_back (kAttrFrameColorHighlighted);
		attributeNames.emplace_back (kAttrRoundRadius);
		attributeNames.emplace_back (kAttrFrameWidth);
		attributeNames.emplace_back (kAttrIconTextMargin);
		attributeNames.emplace_back (kAttrTextAlignment);
		attributeNames.emplace_back (kAttrIcon);
		attributeNames.emplace_back (kAttrIconHighlighted);
		attributeNames.emplace_back (kAttrIconPosition);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrTitle) return kStringType;
		if (attributeName == kAttrFont) return kFontType;
		if (attributeName == kAttrTextColor) return kColorType;
		if (attributeName == kAttrTextColorHighlighted) return kColorType;
		if (attributeName == kAttrGradient) return kGradientType;
		if (attributeName == kAttrGradientHighlighted) return kGradientType;
		if (attributeName == kAttrFrameColor) return kColorType;
		if (attributeName == kAttrFrameColorHighlighted) return kColorType;
		if (attributeName == kAttrFrameWidth) return kFloatType;
		if (attributeName == kAttrRoundRadius) return kFloatType;
		if (attributeName == kAttrKickStyle) return kBooleanType;
		if (attributeName == kAttrIcon) return kBitmapType;
		if (attributeName == kAttrIconHighlighted) return kBitmapType;
		if (attributeName == kAttrIconPosition) return kListType;
		if (attributeName == kAttrIconTextMargin) return kFloatType;
		if (attributeName == kAttrTextAlignment) return kStringType;
		return kUnknownType;
	}
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const override
	{
		if (attributeName == kAttrIconPosition)
		{
			for (const auto& s : positionsStrings)
				values.emplace_back (&s);
			return true;
		}
		return false;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* button = dynamic_cast<CTextButton*> (view);
		if (!button)
			return false;
		if (attributeName == kAttrTitle)
		{
			stringValue = button->getTitle ().getString ();
			return true;
		}
		else if (attributeName == kAttrFont)
		{
			UTF8StringPtr fontName = desc->lookupFontName (button->getFont ());
			if (fontName)
			{
				stringValue = fontName;
				return true;
			}
			return false;
		}
		else if (attributeName == kAttrTextColor)
		{
			colorToString (button->getTextColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == kAttrTextColorHighlighted)
		{
			colorToString (button->getTextColorHighlighted (), stringValue, desc);
			return true;
		}
		else if (attributeName == kAttrFrameColor)
		{
			colorToString (button->getFrameColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == kAttrFrameColorHighlighted)
		{
			colorToString (button->getFrameColorHighlighted (), stringValue, desc);
			return true;
		}
		else if (attributeName == kAttrFrameWidth)
		{
			stringValue = UIAttributes::doubleToString (button->getFrameWidth ());
			return true;
		}
		else if (attributeName == kAttrRoundRadius)
		{
			stringValue = UIAttributes::doubleToString (button->getRoundRadius ());
			return true;
		}
		else if (attributeName == kAttrKickStyle)
		{
			stringValue = button->getStyle() == CTextButton::kKickStyle ? strTrue : strFalse;
			return true;
		}
		else if (attributeName == kAttrIcon)
		{
			CBitmap* bitmap = button->getIcon ();
			if (bitmap)
			{
				return bitmapToString (bitmap, stringValue, desc);
			}
		}
		else if (attributeName == kAttrIconHighlighted)
		{
			CBitmap* bitmap = button->getIconHighlighted ();
			if (bitmap)
			{
				return bitmapToString (bitmap, stringValue, desc);
			}
		}
		else if (attributeName == kAttrIconPosition)
		{
			auto pos = button->getIconPosition ();
			vstgui_assert (pos < positionsStrings.size ());
			stringValue = positionsStrings[pos];
			return true;
		}
		else if (attributeName == kAttrIconTextMargin)
		{
			stringValue = UIAttributes::doubleToString (button->getTextMargin ());
			return true;
		}
		else if (attributeName == kAttrTextAlignment)
		{
			CHoriTxtAlign align = button->getTextAlignment ();
			switch (align)
			{
				case kLeftText: stringValue = strLeft; break;
				case kRightText: stringValue = strRight; break;
				case kCenterText: stringValue = strCenter; break;
			}
			return true;
		}
		else if (attributeName == kAttrGradient)
		{
			CGradient* gradient = button->getGradient ();
			UTF8StringPtr gradientName = gradient ? desc->lookupGradientName (gradient) : nullptr;
			stringValue = gradientName ? gradientName : "";
			return true;
		}
		else if (attributeName == kAttrGradientHighlighted)
		{
			CGradient* gradient = button->getGradientHighlighted ();
			UTF8StringPtr gradientName = gradient ? desc->lookupGradientName (gradient) : nullptr;
			stringValue = gradientName ? gradientName : "";
			return true;
		}
		return false;
	}

};
CTextButtonCreator __gCTextButtonCreator;

//-----------------------------------------------------------------------------
class CKnobBaseCreator : public ViewCreatorAdapter
{
public:
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* knob = dynamic_cast<CKnobBase*> (view);
		if (!knob)
			return false;

		double d;
		if (attributes.getDoubleAttribute (kAttrAngleStart, d))
		{
			// convert from degree
			d = d / 180.f * static_cast<float> (Constants::pi);
			knob->setStartAngle (static_cast<float>(d));
		}
		if (attributes.getDoubleAttribute (kAttrAngleRange, d))
		{
			// convert from degree
			d = d / 180.f * static_cast<float> (Constants::pi);
			knob->setRangeAngle (static_cast<float>(d));
		}
		if (attributes.getDoubleAttribute (kAttrValueInset, d))
			knob->setInsetValue (d);
		if (attributes.getDoubleAttribute (kAttrZoomFactor, d))
			knob->setZoomFactor (static_cast<float>(d));

		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrAngleStart);
		attributeNames.emplace_back (kAttrAngleRange);
		attributeNames.emplace_back (kAttrValueInset);
		attributeNames.emplace_back (kAttrZoomFactor);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrAngleStart) return kFloatType;
		if (attributeName == kAttrAngleRange) return kFloatType;
		if (attributeName == kAttrValueInset) return kFloatType;
		if (attributeName == kAttrZoomFactor) return kFloatType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* knob = dynamic_cast<CKnobBase*> (view);
		if (!knob)
			return false;

		if (attributeName == kAttrAngleStart)
		{
			stringValue = UIAttributes::doubleToString ((knob->getStartAngle () / Constants::pi * 180.), 5);
			return true;
		}
		if (attributeName == kAttrAngleRange)
		{
			stringValue = UIAttributes::doubleToString ((knob->getRangeAngle () / Constants::pi * 180.), 5);
			return true;
		}
		if (attributeName == kAttrValueInset)
		{
			stringValue = UIAttributes::doubleToString (knob->getInsetValue ());
			return true;
		}
		if (attributeName == kAttrZoomFactor)
		{
			stringValue = UIAttributes::doubleToString (knob->getZoomFactor ());
			return true;
		}
		return false;
	}
};

//-----------------------------------------------------------------------------
class CKnobCreator : public CKnobBaseCreator
{
public:
	CKnobCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCKnob; }
	IdStringPtr getBaseViewName () const override { return kCControl; }
	UTF8StringPtr getDisplayName () const override { return "Knob"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto knob = new CKnob (CRect (0, 0, 70, 70), nullptr, -1, nullptr, nullptr);
		knob->setDrawStyle (CKnob::kCoronaDrawing | CKnob::kCoronaOutline |
		                    CKnob::kCoronaLineDashDot | CKnob::kCoronaLineCapButt |
		                    CKnob::kSkipHandleDrawing);
		knob->setCoronaColor (kRedCColor);
		knob->setColorShadowHandle (kBlackCColor);
		knob->setHandleLineWidth (8.);
		knob->setCoronaInset (12);
		knob->setCoronaOutlineWidthAdd (2.);
		knob->setCoronaDashDotLengths ({1.26, 0.1});
		knob->setValue (1.f);
		return knob;
	}
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* knob = dynamic_cast<CKnob*> (view);
		if (!knob)
			return false;

		double d;
		if (attributes.getDoubleAttribute (kAttrCoronaInset, d))
			knob->setCoronaInset (d);
		if (attributes.getDoubleAttribute (kAttrHandleLineWidth, d))
			knob->setHandleLineWidth (d);
		if (attributes.getDoubleAttribute (kAttrCoronaOutlineWidthAdd, d))
			knob->setCoronaOutlineWidthAdd (d);

		CColor color;
		if (stringToColor (attributes.getAttributeValue (kAttrCoronaColor), color, description))
			knob->setCoronaColor (color);
		if (stringToColor (attributes.getAttributeValue (kAttrHandleShadowColor), color, description))
			knob->setColorShadowHandle (color);
		if (stringToColor (attributes.getAttributeValue (kAttrHandleColor), color, description))
			knob->setColorHandle (color);

		UIAttributes::StringArray dashLengthsStrings;
		if (attributes.getStringArrayAttribute (kAttrCoronaDashDotLengths, dashLengthsStrings))
		{
			CLineStyle::CoordVector lengths;
			for (auto& str : dashLengthsStrings)
			{
				double value;
				if (UIAttributes::stringToDouble (str, value))
				{
					lengths.emplace_back (value);
				}
			}
			knob->setCoronaDashDotLengths (lengths);
		}

		CBitmap* bitmap;
		if (stringToBitmap (attributes.getAttributeValue (kAttrHandleBitmap), bitmap, description))
			knob->setHandleBitmap (bitmap);

		int32_t drawStyle = knob->getDrawStyle ();
		applyStyleMask (attributes.getAttributeValue (kAttrCircleDrawing), CKnob::kHandleCircleDrawing, drawStyle);
		applyStyleMask (attributes.getAttributeValue (kAttrCoronaDrawing), CKnob::kCoronaDrawing, drawStyle);
		applyStyleMask (attributes.getAttributeValue (kAttrCoronaFromCenter), CKnob::kCoronaFromCenter, drawStyle);
		applyStyleMask (attributes.getAttributeValue (kAttrCoronaInverted), CKnob::kCoronaInverted, drawStyle);
		applyStyleMask (attributes.getAttributeValue (kAttrCoronaDashDot), CKnob::kCoronaLineDashDot, drawStyle);
		applyStyleMask (attributes.getAttributeValue (kAttrCoronaOutline), CKnob::kCoronaOutline, drawStyle);
		applyStyleMask (attributes.getAttributeValue (kAttrCoronaLineCapButt), CKnob::kCoronaLineCapButt, drawStyle);
		applyStyleMask (attributes.getAttributeValue (kAttrSkipHandleDrawing), CKnob::kSkipHandleDrawing, drawStyle);
		knob->setDrawStyle (drawStyle);
		return CKnobBaseCreator::apply (view, attributes, description);
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrCircleDrawing);
		attributeNames.emplace_back (kAttrCoronaDrawing);
		attributeNames.emplace_back (kAttrCoronaOutline);
		attributeNames.emplace_back (kAttrCoronaFromCenter);
		attributeNames.emplace_back (kAttrCoronaInverted);
		attributeNames.emplace_back (kAttrCoronaDashDot);
		attributeNames.emplace_back (kAttrCoronaLineCapButt);
		attributeNames.emplace_back (kAttrSkipHandleDrawing);
		attributeNames.emplace_back (kAttrCoronaInset);
		attributeNames.emplace_back (kAttrCoronaColor);
		attributeNames.emplace_back (kAttrHandleShadowColor);
		attributeNames.emplace_back (kAttrHandleColor);
		attributeNames.emplace_back (kAttrHandleLineWidth);
		attributeNames.emplace_back (kAttrCoronaOutlineWidthAdd);
		attributeNames.emplace_back (kAttrCoronaDashDotLengths);
		attributeNames.emplace_back (kAttrHandleBitmap);
		return CKnobBaseCreator::getAttributeNames (attributeNames);
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrCircleDrawing) return kBooleanType;
		if (attributeName == kAttrCoronaDrawing) return kBooleanType;
		if (attributeName == kAttrCoronaOutline) return kBooleanType;
		if (attributeName == kAttrCoronaFromCenter) return kBooleanType;
		if (attributeName == kAttrCoronaInverted) return kBooleanType;
		if (attributeName == kAttrCoronaDashDot) return kBooleanType;
		if (attributeName == kAttrCoronaLineCapButt) return kBooleanType;
		if (attributeName == kAttrSkipHandleDrawing) return kBooleanType;
		if (attributeName == kAttrCoronaInset) return kFloatType;
		if (attributeName == kAttrCoronaColor) return kColorType;
		if (attributeName == kAttrHandleShadowColor) return kColorType;
		if (attributeName == kAttrHandleColor) return kColorType;
		if (attributeName == kAttrHandleLineWidth) return kFloatType;
		if (attributeName == kAttrCoronaOutlineWidthAdd) return kFloatType;
		if (attributeName == kAttrCoronaDashDotLengths) return kStringType;
		if (attributeName == kAttrHandleBitmap) return kBitmapType;
		return CKnobBaseCreator::getAttributeType (attributeName);
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* knob = dynamic_cast<CKnob*> (view);
		if (!knob)
			return false;

		if (attributeName == kAttrCoronaInset)
		{
			stringValue = UIAttributes::doubleToString (knob->getCoronaInset ());
			return true;
		}
		if (attributeName == kAttrHandleLineWidth)
		{
			stringValue = UIAttributes::doubleToString (knob->getHandleLineWidth ());
			return true;
		}
		if (attributeName == kAttrCoronaOutlineWidthAdd)
		{
			stringValue = UIAttributes::doubleToString (knob->getCoronaOutlineWidthAdd ());
			return true;
		}
		if (attributeName == kAttrCoronaColor)
		{
			colorToString (knob->getCoronaColor (), stringValue, desc);
			return true;
		}
		if (attributeName == kAttrHandleShadowColor)
		{
			colorToString (knob->getColorShadowHandle (), stringValue, desc);
			return true;
		}
		if (attributeName == kAttrHandleColor)
		{
			colorToString (knob->getColorHandle (), stringValue, desc);
			return true;
		}
		if (attributeName == kAttrHandleBitmap)
		{
			CBitmap* bitmap = knob->getHandleBitmap ();
			if (bitmap)
			{
				return bitmapToString (bitmap, stringValue, desc);
			}
		}
		if (attributeName == kAttrCircleDrawing)
		{
			if (knob->getDrawStyle () & CKnob::kHandleCircleDrawing)
				stringValue = strTrue;
			else
				stringValue = strFalse;
			return true;
		}
		if (attributeName == kAttrCoronaDrawing)
		{
			if (knob->getDrawStyle () & CKnob::kCoronaDrawing)
				stringValue = strTrue;
			else
				stringValue = strFalse;
			return true;
		}
		if (attributeName == kAttrCoronaFromCenter)
		{
			if (knob->getDrawStyle () & CKnob::kCoronaFromCenter)
				stringValue = strTrue;
			else
				stringValue = strFalse;
			return true;
		}
		if (attributeName == kAttrCoronaInverted)
		{
			if (knob->getDrawStyle () & CKnob::kCoronaInverted)
				stringValue = strTrue;
			else
				stringValue = strFalse;
			return true;
		}
		if (attributeName == kAttrCoronaDashDot)
		{
			if (knob->getDrawStyle () & CKnob::kCoronaLineDashDot)
				stringValue = strTrue;
			else
				stringValue = strFalse;
			return true;
		}
		if (attributeName == kAttrCoronaOutline)
		{
			if (knob->getDrawStyle () & CKnob::kCoronaOutline)
				stringValue = strTrue;
			else
				stringValue = strFalse;
			return true;
		}
		if (attributeName == kAttrCoronaLineCapButt)
		{
			if (knob->getDrawStyle () & CKnob::kCoronaLineCapButt)
				stringValue = strTrue;
			else
				stringValue = strFalse;
			return true;
		}
		if (attributeName == kAttrSkipHandleDrawing)
		{
			if (knob->getDrawStyle () & CKnob::kSkipHandleDrawing)
				stringValue = strTrue;
			else
				stringValue = strFalse;
			return true;
		}
		if (attributeName == kAttrCoronaDashDotLengths)
		{
			const auto& lengths = knob->getCoronaDashDotLengths ();
			UIAttributes::StringArray lengthStrings;
			for (auto value : lengths)
			{
				lengthStrings.emplace_back (UIAttributes::doubleToString (value));
			}
			stringValue = UIAttributes::stringArrayToString (lengthStrings);
			return true;
		}
		return CKnobBaseCreator::getAttributeValue (view, attributeName, stringValue, desc);
	}

};
CKnobCreator __CKnobCreator;

//-----------------------------------------------------------------------------
class IMultiBitmapControlCreator
{
public:
	static bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description)
	{
		auto* multiBitmapControl = dynamic_cast<IMultiBitmapControl*> (view);
		if (!multiBitmapControl)
			return false;

		int32_t value;
		if (attributes.getIntegerAttribute (kAttrHeightOfOneImage, value))
			multiBitmapControl->setHeightOfOneImage (value);
		else
			multiBitmapControl->autoComputeHeightOfOneImage ();

		if (attributes.getIntegerAttribute (kAttrSubPixmaps, value))
			multiBitmapControl->setNumSubPixmaps (value);
		return true;
	}
	static bool getAttributeNames (std::list<std::string>& attributeNames)
	{
		attributeNames.emplace_back (kAttrHeightOfOneImage);
		attributeNames.emplace_back (kAttrSubPixmaps);
		return true;
	}
	static IViewCreator::AttrType getAttributeType (const std::string& attributeName)
	{
		if (attributeName == kAttrHeightOfOneImage) return IViewCreator::AttrType::kIntegerType;
		if (attributeName == kAttrSubPixmaps) return IViewCreator::AttrType::kIntegerType;
		return IViewCreator::AttrType::kUnknownType;
	}
	static bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc)
	{
		auto* multiBitmapControl = dynamic_cast<IMultiBitmapControl*> (view);
		if (!multiBitmapControl)
			return false;

		if (attributeName == kAttrHeightOfOneImage)
		{
			stringValue = UIAttributes::integerToString (static_cast<int32_t> (multiBitmapControl->getHeightOfOneImage ()));
			return true;
		}
		if (attributeName == kAttrSubPixmaps)
		{
			stringValue = UIAttributes::integerToString (multiBitmapControl->getNumSubPixmaps ());
			return true;
		}
		return false;
	}

};

//-----------------------------------------------------------------------------
class MultiBitmapControlCreator : public ViewCreatorAdapter
{
public:
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		return IMultiBitmapControlCreator::getAttributeNames (attributeNames);
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		return IMultiBitmapControlCreator::getAttributeType (attributeName);
	}
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		return IMultiBitmapControlCreator::apply (view, attributes, description);
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		return IMultiBitmapControlCreator::getAttributeValue (view, attributeName, stringValue, desc);
	}
};

//-----------------------------------------------------------------------------
class CAnimKnobCreator : public CKnobBaseCreator
{
public:
	CAnimKnobCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCAnimKnob; }
	IdStringPtr getBaseViewName () const override { return kCControl; }
	UTF8StringPtr getDisplayName () const override { return "Animation Knob"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override
	{
		return new CAnimKnob (CRect (0, 0, 0, 0), nullptr, -1, nullptr);
	}

	bool apply (CView* view, const UIAttributes& attributes,
	            const IUIDescription* description) const override
	{
		auto* animKnob = dynamic_cast<CAnimKnob*> (view);
		if (!animKnob)
			return false;

		bool b;
		if (attributes.getBooleanAttribute (kAttrInverseBitmap, b))
		{
			animKnob->setInverseBitmap (b);
		}
		IMultiBitmapControlCreator::apply (view, attributes, description);
		return CKnobBaseCreator::apply (view, attributes, description);
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrInverseBitmap);
		IMultiBitmapControlCreator::getAttributeNames (attributeNames);
		return CKnobBaseCreator::getAttributeNames (attributeNames);
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrInverseBitmap)
			return kBooleanType;
		auto res = CKnobBaseCreator::getAttributeType (attributeName);
		if (res != kUnknownType)
			return res;
		return IMultiBitmapControlCreator::getAttributeType (attributeName);
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue,
	                        const IUIDescription* desc) const override
	{
		auto* animKnob = dynamic_cast<CAnimKnob*> (view);
		if (!animKnob)
			return false;

		if (attributeName == kAttrInverseBitmap)
		{
			stringValue = animKnob->getInverseBitmap () ? strTrue : strFalse;
			return true;
		}
		if (CKnobBaseCreator::getAttributeValue (view, attributeName, stringValue, desc))
			return true;
		return IMultiBitmapControlCreator::getAttributeValue (view, attributeName, stringValue,
		                                                      desc);
	}
};
CAnimKnobCreator __gCAnimKnobCreator;

//-----------------------------------------------------------------------------
class CSwitchBaseCreator : public ViewCreatorAdapter
{
public:
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrInverseBitmap);
		return IMultiBitmapControlCreator::getAttributeNames (attributeNames);
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrInverseBitmap) return kBooleanType;
		return IMultiBitmapControlCreator::getAttributeType (attributeName);
	}
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto control = dynamic_cast<CSwitchBase*> (view);
		if (!control)
			return false;

		bool b;
		if (attributes.getBooleanAttribute (kAttrInverseBitmap, b))
		{
			control->setInverseBitmap (b);
		}

		return IMultiBitmapControlCreator::apply (view, attributes, description);
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto control = dynamic_cast<CSwitchBase*> (view);
		if (!control)
			return false;

		if (attributeName == kAttrInverseBitmap)
		{
			stringValue = control->getInverseBitmap() ? strTrue : strFalse;
			return true;
		}
		return IMultiBitmapControlCreator::getAttributeValue (view, attributeName, stringValue, desc);
	}
};

//-----------------------------------------------------------------------------
class CVerticalSwitchCreator : public CSwitchBaseCreator
{
public:
	CVerticalSwitchCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCVerticalSwitch; }
	IdStringPtr getBaseViewName () const override { return kCControl; }
	UTF8StringPtr getDisplayName () const override { return "Vertical Switch"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CVerticalSwitch (CRect (0, 0, 0, 0), nullptr, -1, nullptr); }
};
CVerticalSwitchCreator __gCVerticalSwitchCreator;

//-----------------------------------------------------------------------------
class CHorizontalSwitchCreator : public CSwitchBaseCreator
{
public:
	CHorizontalSwitchCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCHorizontalSwitch; }
	IdStringPtr getBaseViewName () const override { return kCControl; }
	UTF8StringPtr getDisplayName () const override { return "Horizontal Switch"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CHorizontalSwitch (CRect (0, 0, 0, 0), nullptr, -1, nullptr); }
};
CHorizontalSwitchCreator __gCHorizontalSwitchCreator;

//-----------------------------------------------------------------------------
class CRockerSwitchCreator : public MultiBitmapControlCreator
{
public:
	CRockerSwitchCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCRockerSwitch; }
	IdStringPtr getBaseViewName () const override { return kCControl; }
	UTF8StringPtr getDisplayName () const override { return "Rocker Switch"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CRockerSwitch (CRect (0, 0, 0, 0), nullptr, -1, nullptr); }
};
CRockerSwitchCreator __gCRockerSwitchCreator;

//-----------------------------------------------------------------------------
class CMovieBitmapCreator : public MultiBitmapControlCreator
{
public:
	CMovieBitmapCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCMovieBitmap; }
	IdStringPtr getBaseViewName () const override { return kCControl; }
	UTF8StringPtr getDisplayName () const override { return "Movie Bitmap"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CMovieBitmap (CRect (0, 0, 0, 0), nullptr, -1, nullptr); }
};
CMovieBitmapCreator __gCMovieBitmapCreator;

//-----------------------------------------------------------------------------
class CMovieButtonCreator : public MultiBitmapControlCreator
{
public:
	CMovieButtonCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCMovieButton; }
	IdStringPtr getBaseViewName () const override { return kCControl; }
	UTF8StringPtr getDisplayName () const override { return "Movie Button"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CMovieButton (CRect (0, 0, 0, 0), nullptr, -1, nullptr); }
};
CMovieButtonCreator __gCMovieButtonCreator;

//-----------------------------------------------------------------------------
class CKickButtonCreator : public MultiBitmapControlCreator
{
public:
	CKickButtonCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCKickButton; }
	IdStringPtr getBaseViewName () const override { return kCControl; }
	UTF8StringPtr getDisplayName () const override { return "Kick Button"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CKickButton (CRect (0, 0, 0, 0), nullptr, -1, nullptr); }
};
CKickButtonCreator __gCKickButtonCreator;


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
CAnimationSplashScreenCreator __gCAnimationSplashScreenCreator;
CCheckBoxCreator __gCCheckBoxCreator;
CControlCreator __gCControlCreator;
CGradientViewCreator __gCGradientViewCreator;
CLayeredViewContainerCreator __CLayeredViewContainerCreator;
COnOffButtonCreator __gCOnOffButtonCreator;
COptionMenuCreator __gCOptionMenuCreator;
CParamDisplayCreator __gCParamDisplayCreator;
CRowColumnViewCreator __CRowColumnViewCreator;
CScrollViewCreator __CScrollViewCreator;
CSegmentButtonCreator __gCSegmentButtonCreator;
CShadowViewContainerCreator __gCShadowViewContainerCreator;
CSliderCreator __gCSliderCreator;
CSplitViewCreator __gCSplitViewCreator;
UIViewSwitchContainerCreator __gUIViewSwitchContainerCreator;
CViewContainerCreator __CViewContainerCreator;
CViewCreator __gCViewCreator;
CVuMeterCreator __gCVuMeterCreator;
CXYPadCreator __gCXYPadCreator;


//------------------------------------------------------------------------
}} // VSTGUI

/**
@endcond ignore
*/
