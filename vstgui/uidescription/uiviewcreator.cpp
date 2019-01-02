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
#include "../vstgui.h"
#include <sstream>
#include <array>

namespace VSTGUI {
namespace UIViewCreator {

static constexpr auto strTrue = "true";
static constexpr auto strFalse = "false";
static constexpr auto strHorizontal = "horizontal";
static constexpr auto strVertical = "vertical";

static constexpr auto strNone = "none";
static constexpr auto strHead = "head";
static constexpr auto strTail = "tail";

static constexpr auto strLeft = "left";
static constexpr auto strRight = "right";
static constexpr auto strCenter = "center";

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
static void applyStyleMask (const std::string* value, int32_t mask, int32_t& style)
{
	if (value)
	{
		setBit (style, mask, *value == strTrue);
	}
}

//------------------------------------------------------------------------
static void addGradientToUIDescription (const IUIDescription* description, CGradient* gradient, UTF8StringPtr baseName)
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

static bool getStandardAttributeListValues (const std::string& attributeName, std::list<const std::string*>& values);

#if VSTGUI_LIVE_EDITING
//-----------------------------------------------------------------------------
class LiveEditingCView : public CView
{
public:
	LiveEditingCView (const CRect& r) : CView (r) {}
	void draw (CDrawContext* context) override
	{
		if (getDrawBackground ())
		{
			CView::draw (context);
			return;
		}
		context->setLineWidth (1.);
		context->setLineStyle (kLineSolid);
		context->setDrawMode (kAliasing);
		context->setFrameColor ({200, 200, 200, 100});
		context->setFillColor ({200, 200, 200, 100});
		constexpr auto width = 5.;
		CRect viewSize = getViewSize ();
		auto r = viewSize;
		r.setSize ({width,width});
		uint32_t row = 0u;
		while (r.top < viewSize.bottom)
		{
			uint32_t column = (row % 2) ? 0u : 1u;
			while (r.left < viewSize.right)
			{
				if (column % 2)
					context->drawRect (r, kDrawFilled);
				r.offset (width, 0);
				++column;
			}
			r.left = viewSize.left;
			r.right = r.left + width;
			r.offset (0, width);
			++row;
		}
		context->drawRect (viewSize, kDrawStroked);
		setDirty (false);
	}
};
using SimpleCView = LiveEditingCView;
#else
using SimpleCView = CView;
#endif

//-----------------------------------------------------------------------------
class CViewCreator : public ViewCreatorAdapter
{
public:
	CViewCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCView; }
	IdStringPtr getBaseViewName () const override { return nullptr; }
	UTF8StringPtr getDisplayName () const override { return "View"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new SimpleCView (CRect (0, 0, 50, 50)); }
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		CPoint p;
		CRect size;
		if (attributes.getPointAttribute (kAttrOrigin, p))
		{
			CRect origViewSize = view->getViewSize ();
			size.setTopLeft (p);
			size.setWidth (origViewSize.getWidth ());
			size.setHeight (origViewSize.getHeight ());
			view->setViewSize (size, false);
			view->setMouseableArea (size);
		}
		if (attributes.getPointAttribute (kAttrSize, p))
		{
			size = view->getViewSize ();
			size.setSize (p);
			view->setViewSize (size, false);
			view->setMouseableArea (size);
		}

		CBitmap* bitmap;
		if (stringToBitmap (attributes.getAttributeValue (kAttrBitmap), bitmap, description))
			view->setBackground (bitmap);
		if (stringToBitmap (attributes.getAttributeValue (kAttrDisabledBitmap), bitmap, description))
			view->setDisabledBackground (bitmap);
		
		bool b;
		if (attributes.getBooleanAttribute (kAttrTransparent, b))
			view->setTransparency (b);
		if (attributes.getBooleanAttribute (kAttrMouseEnabled, b))
			view->setMouseEnabled (b);
		if (attributes.hasAttribute (kAttrWantsFocus) && attributes.getBooleanAttribute (kAttrWantsFocus, b))
			view->setWantsFocus (b);

		const std::string* autosizeAttr = attributes.getAttributeValue (kAttrAutosize);
		if (autosizeAttr)
		{
			int32_t autosize = kAutosizeNone;
			if (autosizeAttr->find ("left") != std::string::npos)
				autosize |= kAutosizeLeft;
			if (autosizeAttr->find ("top") != std::string::npos)
				autosize |= kAutosizeTop;
			if (autosizeAttr->find ("right") != std::string::npos)
				autosize |= kAutosizeRight;
			if (autosizeAttr->find ("bottom") != std::string::npos)
				autosize |= kAutosizeBottom;
			if (autosizeAttr->find ("row") != std::string::npos)
				autosize |= kAutosizeRow;
			if (autosizeAttr->find ("column") != std::string::npos)
				autosize |= kAutosizeColumn;
			view->setAutosizeFlags (autosize);
		}
		const std::string* tooltipAttr = attributes.getAttributeValue (kAttrTooltip);
		if (tooltipAttr)
		{
			if (!tooltipAttr->empty ())
				view->setTooltipText (tooltipAttr->data ());
			else
				view->setTooltipText (nullptr);
		}

		const std::string* customViewAttr = attributes.getAttributeValue (kAttrCustomViewName);
		if (customViewAttr)
			view->setAttribute ('uicv', static_cast<uint32_t> (customViewAttr->size () + 1), customViewAttr->c_str ());

		const std::string* subControllerAttr = attributes.getAttributeValue (kAttrSubController);
		if (subControllerAttr)
			view->setAttribute ('uisc', static_cast<uint32_t> (subControllerAttr->size () + 1), subControllerAttr->c_str ());

		double opacity;
		if (attributes.getDoubleAttribute (kAttrOpacity, opacity))
			view->setAlphaValue (static_cast<float>(opacity));

		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrOrigin);
		attributeNames.emplace_back (kAttrSize);
		attributeNames.emplace_back (kAttrOpacity);
		attributeNames.emplace_back (kAttrTransparent);
		attributeNames.emplace_back (kAttrMouseEnabled);
		attributeNames.emplace_back (kAttrWantsFocus);
		attributeNames.emplace_back (kAttrBitmap);
		attributeNames.emplace_back (kAttrDisabledBitmap);
		attributeNames.emplace_back (kAttrAutosize);
		attributeNames.emplace_back (kAttrTooltip);
		attributeNames.emplace_back (kAttrCustomViewName);
		attributeNames.emplace_back (kAttrSubController);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrOrigin) return kPointType;
		else if (attributeName == kAttrSize) return kPointType;
		else if (attributeName == kAttrOpacity) return kFloatType;
		else if (attributeName == kAttrTransparent) return kBooleanType;
		else if (attributeName == kAttrMouseEnabled) return kBooleanType;
		else if (attributeName == kAttrWantsFocus) return kBooleanType;
		else if (attributeName == kAttrBitmap) return kBitmapType;
		else if (attributeName == kAttrDisabledBitmap) return kBitmapType;
		else if (attributeName == kAttrAutosize) return kStringType;
		else if (attributeName == kAttrTooltip) return kStringType;
		else if (attributeName == kAttrCustomViewName) return kStringType;
		else if (attributeName == kAttrSubController) return kStringType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		if (attributeName == kAttrOrigin)
		{
			stringValue = UIAttributes::pointToString (view->getViewSize ().getTopLeft ());
			return true;
		}
		else if (attributeName == kAttrSize)
		{
			stringValue = UIAttributes::pointToString (view->getViewSize ().getSize ());
			return true;
		}
		else if (attributeName == kAttrOpacity)
		{
			stringValue = UIAttributes::doubleToString (view->getAlphaValue ());
			return true;
		}
		else if (attributeName == kAttrTransparent)
		{
			stringValue = view->getTransparency () ? strTrue : strFalse;
			return true;
		}
		else if (attributeName == kAttrMouseEnabled)
		{
			stringValue = view->getMouseEnabled () ? strTrue : strFalse;
			return true;
		}
		else if (attributeName == kAttrWantsFocus)
		{
			stringValue = view->wantsFocus () ? strTrue : strFalse;
			return true;
		}
		else if (attributeName == kAttrBitmap)
		{
			CBitmap* bitmap = view->getBackground ();
			if (bitmap)
				bitmapToString (bitmap, stringValue, desc);
			else
				stringValue = "";
			return true;
		}
		else if (attributeName == kAttrDisabledBitmap)
		{
			CBitmap* bitmap = view->getDisabledBackground ();
			if (bitmap)
				bitmapToString (bitmap, stringValue, desc);
			else
				stringValue = "";
			return true;
		}
		else if (attributeName == kAttrAutosize)
		{
			std::stringstream stream;
			int32_t autosize = view->getAutosizeFlags ();
			if (autosize == 0)
				return true;
			if (autosize & kAutosizeLeft)
				stream << "left ";
			if (autosize & kAutosizeRight)
				stream << "right ";
			if (autosize & kAutosizeTop)
				stream << "top ";
			if (autosize & kAutosizeBottom)
				stream << "bottom ";
			if (autosize & kAutosizeRow)
				stream << "row ";
			if (autosize & kAutosizeColumn)
				stream << "column ";
			stringValue = stream.str ();
			return true;
		}
		else if (attributeName == kAttrTooltip)
		{
			return getViewAttributeString (view, kCViewTooltipAttribute, stringValue);
		}
		else if (attributeName == kAttrCustomViewName)
		{
			return getViewAttributeString (view, 'uicv', stringValue);
		}
		else if (attributeName == kAttrSubController)
		{
			return getViewAttributeString (view, 'uisc', stringValue);
		}
		return false;
	}
	bool getAttributeValueRange (const std::string& attributeName, double& minValue, double &maxValue) const override
	{
		if (attributeName == kAttrOpacity)
		{
			minValue = 0.;
			maxValue = 1.;
			return true;
		}
		return false;
	}
private:
	static bool getViewAttributeString (CView* view, const CViewAttributeID attrID, std::string& value)
	{
		uint32_t attrSize = 0;
		if (view->getAttributeSize (attrID, attrSize))
		{
			char* cstr = new char [attrSize+1];
			if (view->getAttribute (attrID, attrSize, cstr, attrSize))
				value = cstr;
			else
				value = "";
			delete [] cstr;
			return true;
		}
		return false;
	}
};
CViewCreator __gCViewCreator;

//-----------------------------------------------------------------------------
class CViewContainerCreator : public ViewCreatorAdapter
{
public:
	std::string kStroked = "stroked";
	std::string kFilledAndStroked = "filled and stroked";
	std::string kFilled = "filled";

	CViewContainerCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCViewContainer; }
	IdStringPtr getBaseViewName () const override { return kCView; }
	UTF8StringPtr getDisplayName () const override { return "View Container"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CViewContainer (CRect (0, 0, 100, 100)); }
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		CViewContainer* viewContainer = view->asViewContainer ();
		if (viewContainer == nullptr)
			return false;
		CColor backColor;
		if (stringToColor (attributes.getAttributeValue (kAttrBackgroundColor), backColor, description))
			viewContainer->setBackgroundColor (backColor);
		const std::string* attr = attributes.getAttributeValue (kAttrBackgroundColorDrawStyle);
		if (attr)
		{
			CDrawStyle drawStyle = kDrawFilledAndStroked;
			if (*attr == kStroked)
			{
				drawStyle = kDrawStroked;
			}
			else if (*attr == kFilled)
			{
				drawStyle = kDrawFilled;
			}
			viewContainer->setBackgroundColorDrawStyle (drawStyle);
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrBackgroundColor);
		attributeNames.emplace_back (kAttrBackgroundColorDrawStyle);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrBackgroundColor) return kColorType;
		if (attributeName == kAttrBackgroundColorDrawStyle) return kListType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		CViewContainer* vc = view->asViewContainer ();
		if (vc == nullptr)
			return false;
		if (attributeName == kAttrBackgroundColor)
		{
			colorToString (vc->getBackgroundColor (), stringValue, desc);
			return true;
		}
		if (attributeName == kAttrBackgroundColorDrawStyle)
		{
			switch (vc->getBackgroundColorDrawStyle ())
			{
				case kDrawStroked: stringValue = kStroked; break;
				case kDrawFilledAndStroked: stringValue = kFilledAndStroked; break;
				case kDrawFilled: stringValue = kFilled; break;
			}
			return true;
		}
		return false;
	}

	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const override
	{
		if (attributeName == kAttrBackgroundColorDrawStyle)
		{
			values.emplace_back (&kStroked);
			values.emplace_back (&kFilledAndStroked);
			values.emplace_back (&kFilled);
			return true;
		}
		return false;
	}

};
CViewContainerCreator __CViewContainerCreator;

//-----------------------------------------------------------------------------
class CLayeredViewContainerCreator : public ViewCreatorAdapter
{
public:
	CLayeredViewContainerCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCLayeredViewContainer; }
	IdStringPtr getBaseViewName () const override { return kCViewContainer; }
	UTF8StringPtr getDisplayName () const override { return "Layered View Container"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CLayeredViewContainer (CRect (0, 0, 100, 100)); }
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* lvc = dynamic_cast<CLayeredViewContainer*>(view);
		if (lvc == nullptr)
			return false;
		int32_t zIndex;
		if (attributes.getIntegerAttribute (kAttrZIndex, zIndex))
			lvc->setZIndex (static_cast<uint32_t>(zIndex));
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrZIndex);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrZIndex) return kIntegerType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* lvc = dynamic_cast<CLayeredViewContainer*>(view);
		if (lvc == nullptr)
			return false;
		if (attributeName == kAttrZIndex)
		{
			stringValue = UIAttributes::integerToString (static_cast<int32_t>(lvc->getZIndex ()));
			return true;
		}
		return false;
	}
};
CLayeredViewContainerCreator __CLayeredViewContainerCreator;

//-----------------------------------------------------------------------------
class CRowColumnViewCreator : public ViewCreatorAdapter
{
public:
	std::string kLeftTop = "left-top";
	std::string kStretch = "stretch";
	std::string kCenter = strCenter;
	std::string kRightBottom = "right-bottom";

	CRowColumnViewCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCRowColumnView; }
	IdStringPtr getBaseViewName () const override { return kCViewContainer; }
	UTF8StringPtr getDisplayName () const override { return "Row Column View Container"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CRowColumnView (CRect (0, 0, 100, 100)); }
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* rcv = dynamic_cast<CRowColumnView*> (view);
		if (rcv == nullptr)
			return false;
		const std::string* attr = attributes.getAttributeValue (kAttrRowStyle);
		if (attr)
			rcv->setStyle (*attr == strTrue ? CRowColumnView::kRowStyle : CRowColumnView::kColumnStyle);
		attr = attributes.getAttributeValue (kAttrSpacing);
		if (attr)
		{
			CCoord spacing = UTF8StringView (attr->c_str ()).toDouble ();
			rcv->setSpacing (spacing);
		}
		CRect margin;
		if (attributes.getRectAttribute (kAttrMargin, margin))
			rcv->setMargin (margin);
		attr = attributes.getAttributeValue (kAttrAnimateViewResizing);
		if (attr)
			rcv->setAnimateViewResizing (*attr == strTrue ? true : false);
		attr = attributes.getAttributeValue (kAttrHideClippedSubviews);
		if (attr)
			rcv->setHideClippedSubviews (*attr == strTrue ? true : false);
		attr = attributes.getAttributeValue (kAttrEqualSizeLayout);
		if (attr)
		{
			if (*attr == kStretch)
				rcv->setLayoutStyle (CRowColumnView::kStretchEqualy);
			else if (*attr == kCenter)
				rcv->setLayoutStyle (CRowColumnView::kCenterEqualy);
			else if (*attr == kRightBottom)
				rcv->setLayoutStyle (CRowColumnView::kRightBottomEqualy);
			else
				rcv->setLayoutStyle (CRowColumnView::kLeftTopEqualy);
		}
		attr = attributes.getAttributeValue (kAttrViewResizeAnimationTime);
		if (attr)
		{
			uint32_t time = (uint32_t)strtol (attr->c_str(), nullptr, 10);
			rcv->setViewResizeAnimationTime (time);
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrRowStyle);
		attributeNames.emplace_back (kAttrSpacing);
		attributeNames.emplace_back (kAttrMargin);
		attributeNames.emplace_back (kAttrEqualSizeLayout);
		attributeNames.emplace_back (kAttrHideClippedSubviews);
		attributeNames.emplace_back (kAttrAnimateViewResizing);
		attributeNames.emplace_back (kAttrViewResizeAnimationTime);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrRowStyle) return kBooleanType;
		if (attributeName == kAttrSpacing) return kIntegerType;
		if (attributeName == kAttrMargin) return kRectType;
		if (attributeName == kAttrEqualSizeLayout) return kListType;
		if (attributeName == kAttrHideClippedSubviews) return kBooleanType;
		if (attributeName == kAttrAnimateViewResizing) return kBooleanType;
		if (attributeName == kAttrViewResizeAnimationTime) return kIntegerType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* rcv = dynamic_cast<CRowColumnView*> (view);
		if (rcv == nullptr)
			return false;
		if (attributeName == kAttrRowStyle)
		{
			stringValue = rcv->getStyle () == CRowColumnView::kRowStyle ? strTrue : strFalse;
			return true;
		}
		if (attributeName == kAttrAnimateViewResizing)
		{
			stringValue = rcv->isAnimateViewResizing () ? strTrue : strFalse;
			return true;
		}
		if (attributeName == kAttrHideClippedSubviews)
		{
			stringValue = rcv->hideClippedSubviews () ? strTrue : strFalse;
			return true;
		}
		if (attributeName == kAttrSpacing)
		{
			stringValue = UIAttributes::integerToString (static_cast<int32_t> (rcv->getSpacing ()));
			return true;
		}
		if (attributeName == kAttrViewResizeAnimationTime)
		{
			stringValue = UIAttributes::integerToString (static_cast<int32_t> (rcv->getViewResizeAnimationTime ()));
			return true;
		}
		if (attributeName == kAttrMargin)
		{
			stringValue = UIAttributes::rectToString (rcv->getMargin ());
			return true;
		}
		if (attributeName == kAttrEqualSizeLayout)
		{
			switch (rcv->getLayoutStyle ())
			{
				case CRowColumnView::kLeftTopEqualy: stringValue = kLeftTop; break;
				case CRowColumnView::kStretchEqualy: stringValue = kStretch; break;
				case CRowColumnView::kCenterEqualy: stringValue = kCenter; break;
				case CRowColumnView::kRightBottomEqualy: stringValue = kRightBottom; break;
			}
			return true;
		}
		return false;
	}
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const override
	{
		if (attributeName == kAttrEqualSizeLayout)
		{
			values.emplace_back (&kLeftTop);
			values.emplace_back (&kStretch);
			values.emplace_back (&kCenter);
			values.emplace_back (&kRightBottom);
			return true;
		}
		return false;
	}

};
CRowColumnViewCreator __CRowColumnViewCreator;

//-----------------------------------------------------------------------------
class CScrollViewCreator : public ViewCreatorAdapter
{
public:
	CScrollViewCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCScrollView; }
	IdStringPtr getBaseViewName () const override { return kCViewContainer; }
	UTF8StringPtr getDisplayName () const override { return "Scroll View"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CScrollView (CRect (0, 0, 100, 100), CRect (0, 0, 200, 200), CScrollView::kHorizontalScrollbar|CScrollView::kVerticalScrollbar); }
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* scrollView = dynamic_cast<CScrollView*> (view);
		if (scrollView == nullptr)
			return false;
		
		CPoint p;
		if (attributes.getPointAttribute (kAttrContainerSize, p))
		{
			CRect r;
			r.setSize (p);
			scrollView->setContainerSize (r);
		}

		int32_t style = scrollView->getStyle ();
		applyStyleMask (attributes.getAttributeValue (kAttrHorizontalScrollbar), CScrollView::kHorizontalScrollbar, style);
		applyStyleMask (attributes.getAttributeValue (kAttrVerticalScrollbar), CScrollView::kVerticalScrollbar, style);
		applyStyleMask (attributes.getAttributeValue (kAttrAutoDragScrolling), CScrollView::kAutoDragScrolling, style);
		const std::string* attr = attributes.getAttributeValue (kAttrBordered);
		if (attr)
		{
			setBit (style, CScrollView::kDontDrawFrame, *attr != strTrue);
		}
		applyStyleMask (attributes.getAttributeValue (kAttrOverlayScrollbars), CScrollView::kOverlayScrollbars, style);
		applyStyleMask (attributes.getAttributeValue (kAttrFollowFocusView), CScrollView::kFollowFocusView, style);
		applyStyleMask (attributes.getAttributeValue (kAttrAutoHideScrollbars), CScrollView::kAutoHideScrollbars, style);
		scrollView->setStyle (style);
		CColor color;
		CScrollbar* vscrollbar = scrollView->getVerticalScrollbar ();
		CScrollbar* hscrollbar = scrollView->getHorizontalScrollbar ();
		if (stringToColor (attributes.getAttributeValue (kAttrScrollbarBackgroundColor), color, description))
		{
			if (vscrollbar) vscrollbar->setBackgroundColor (color);
			if (hscrollbar) hscrollbar->setBackgroundColor (color);
		}
		if (stringToColor (attributes.getAttributeValue (kAttrScrollbarFrameColor), color, description))
		{
			if (vscrollbar) vscrollbar->setFrameColor (color);
			if (hscrollbar) hscrollbar->setFrameColor (color);
		}
		if (stringToColor (attributes.getAttributeValue (kAttrScrollbarScrollerColor), color, description))
		{
			if (vscrollbar) vscrollbar->setScrollerColor (color);
			if (hscrollbar) hscrollbar->setScrollerColor (color);
		}
		CCoord width;
		if (attributes.getDoubleAttribute (kAttrScrollbarWidth, width))
			scrollView->setScrollbarWidth (width);
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrContainerSize);
		attributeNames.emplace_back (kAttrScrollbarBackgroundColor);
		attributeNames.emplace_back (kAttrScrollbarFrameColor);
		attributeNames.emplace_back (kAttrScrollbarScrollerColor);
		attributeNames.emplace_back (kAttrHorizontalScrollbar);
		attributeNames.emplace_back (kAttrVerticalScrollbar);
		attributeNames.emplace_back (kAttrAutoHideScrollbars);
		attributeNames.emplace_back (kAttrAutoDragScrolling);
		attributeNames.emplace_back (kAttrOverlayScrollbars);
		attributeNames.emplace_back (kAttrScrollbarWidth);
		attributeNames.emplace_back (kAttrBordered);
		attributeNames.emplace_back (kAttrFollowFocusView);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrContainerSize) return kPointType;
		if (attributeName == kAttrScrollbarBackgroundColor) return kColorType;
		if (attributeName == kAttrScrollbarFrameColor) return kColorType;
		if (attributeName == kAttrScrollbarScrollerColor) return kColorType;
		if (attributeName == kAttrHorizontalScrollbar) return kBooleanType;
		if (attributeName == kAttrVerticalScrollbar) return kBooleanType;
		if (attributeName == kAttrAutoHideScrollbars) return kBooleanType;
		if (attributeName == kAttrAutoDragScrolling) return kBooleanType;
		if (attributeName == kAttrOverlayScrollbars) return kBooleanType;
		if (attributeName == kAttrScrollbarWidth) return kIntegerType;
		if (attributeName == kAttrBordered) return kBooleanType;
		if (attributeName == kAttrFollowFocusView) return kBooleanType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* sc = dynamic_cast<CScrollView*> (view);
		if (sc == nullptr)
			return false;
		if (attributeName == kAttrContainerSize)
		{
			stringValue = UIAttributes::pointToString (sc->getContainerSize ().getSize ());
			return true;
		}
		if (attributeName == kAttrScrollbarWidth)
		{
			stringValue = UIAttributes::doubleToString (sc->getScrollbarWidth ());
			return true;
		}
		CScrollbar* scrollbar = sc->getVerticalScrollbar ();
		if (!scrollbar)
			scrollbar = sc->getHorizontalScrollbar ();
		if (scrollbar)
		{
			if (attributeName == kAttrScrollbarBackgroundColor)
			{
				colorToString (scrollbar->getBackgroundColor (), stringValue, desc);
				return true;
			}
			if (attributeName == kAttrScrollbarFrameColor)
			{
				colorToString (scrollbar->getFrameColor (), stringValue, desc);
				return true;
			}
			if (attributeName == kAttrScrollbarScrollerColor)
			{
				colorToString (scrollbar->getScrollerColor (), stringValue, desc);
				return true;
			}
		}
		if (attributeName == kAttrHorizontalScrollbar)
		{
			stringValue = sc->getStyle () & CScrollView::kHorizontalScrollbar ? strTrue : strFalse;
			return true;
		}
		if (attributeName == kAttrVerticalScrollbar)
		{
			stringValue = sc->getStyle () & CScrollView::kVerticalScrollbar ? strTrue : strFalse;
			return true;
		}
		if (attributeName == kAttrAutoHideScrollbars)
		{
			stringValue = sc->getStyle () & CScrollView::kAutoHideScrollbars ? strTrue : strFalse;
			return true;
		}
		if (attributeName == kAttrAutoDragScrolling)
		{
			stringValue = sc->getStyle () & CScrollView::kAutoDragScrolling ? strTrue : strFalse;
			return true;
		}
		if (attributeName == kAttrBordered)
		{
			stringValue = sc->getStyle () & CScrollView::kDontDrawFrame ? strFalse : strTrue;
			return true;
		}
		if (attributeName == kAttrOverlayScrollbars)
		{
			stringValue = sc->getStyle () & CScrollView::kOverlayScrollbars ? strTrue : strFalse;
			return true;
		}
		if (attributeName == kAttrFollowFocusView)
		{
			stringValue = sc->getStyle () & CScrollView::kFollowFocusView ? strTrue : strFalse;
			return true;
		}
		return false;
	}

};
CScrollViewCreator __CScrollViewCreator;

//-----------------------------------------------------------------------------
class CControlCreator : public ViewCreatorAdapter
{
protected:
	class DummyControl : public CControl
	{
	public:
		DummyControl () : CControl (CRect (0, 0, 40, 40), nullptr, -1) {}
		void draw (CDrawContext* pContext) override { CView::draw (pContext); }
		
		CLASS_METHODS(DummyControl, CControl)
	};
public:
	CControlCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCControl; }
	IdStringPtr getBaseViewName () const override { return kCView; }
	UTF8StringPtr getDisplayName () const override { return "Control"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new DummyControl (); }
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* control = dynamic_cast<CControl*> (view);
		if (control == nullptr)
			return false;
		
		double value;
		if (attributes.getDoubleAttribute (kAttrDefaultValue, value))
			control->setDefaultValue (static_cast<float> (value));
		if (attributes.getDoubleAttribute (kAttrMinValue, value))
			control->setMin (static_cast<float> (value));
		if (attributes.getDoubleAttribute (kAttrMaxValue, value))
			control->setMax (static_cast<float> (value));
		if (attributes.getDoubleAttribute (kAttrWheelIncValue, value))
			control->setWheelInc (static_cast<float> (value));

		const std::string* controlTagAttr = attributes.getAttributeValue (kAttrControlTag);
		if (controlTagAttr)
		{
			if (controlTagAttr->length () == 0)
			{
				control->setTag (-1);
				control->setListener (nullptr);
			}
			else
			{
				int32_t tag = description->getTagForName (controlTagAttr->c_str ());
				if (tag != -1)
				{
					control->setListener (description->getControlListener (controlTagAttr->c_str ()));
					control->setTag (tag);
				}
				else
				{
					char* endPtr = nullptr;
					tag = (int32_t)strtol (controlTagAttr->c_str (), &endPtr, 10);
					if (endPtr != controlTagAttr->c_str ())
					{
						control->setListener (description->getControlListener (controlTagAttr->c_str ()));
						control->setTag (tag);
					}
				}
			}
		}
		
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrControlTag);
		attributeNames.emplace_back (kAttrDefaultValue);
		attributeNames.emplace_back (kAttrMinValue);
		attributeNames.emplace_back (kAttrMaxValue);
		attributeNames.emplace_back (kAttrWheelIncValue);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrControlTag) return kTagType;
		else if (attributeName == kAttrDefaultValue) return kFloatType;
		else if (attributeName == kAttrMinValue) return kFloatType;
		else if (attributeName == kAttrMaxValue) return kFloatType;
		else if (attributeName == kAttrWheelIncValue) return kFloatType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* control = dynamic_cast<CControl*> (view);
		if (control == nullptr)
			return false;
		if (attributeName == kAttrControlTag)
		{
			if (control->getTag () != -1)
			{
				UTF8StringPtr controlTag = desc->lookupControlTagName (control->getTag ());
				if (controlTag)
				{
					stringValue = controlTag;
					return true;
				}
			}
		}
		else if (attributeName == kAttrDefaultValue)
		{
			stringValue = UIAttributes::doubleToString (control->getDefaultValue ());
			return true;
		}
		else if (attributeName == kAttrMinValue)
		{
			stringValue = UIAttributes::doubleToString (control->getMin ());
			return true;
		}
		else if (attributeName == kAttrMaxValue)
		{
			stringValue = UIAttributes::doubleToString (control->getMax ());
			return true;
		}
		else if (attributeName == kAttrWheelIncValue)
		{
			stringValue = UIAttributes::doubleToString (control->getWheelInc (), 5);
			return true;
		}
		return false;
	}

};
CControlCreator __gCControlCreator;

//-----------------------------------------------------------------------------
class COnOffButtonCreator : public ViewCreatorAdapter
{
public:
	COnOffButtonCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCOnOffButton; }
	IdStringPtr getBaseViewName () const override { return kCControl; }
	UTF8StringPtr getDisplayName () const override { return "OnOff Button"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new COnOffButton (CRect (0, 0, 20, 20), nullptr, -1, nullptr); }
};
COnOffButtonCreator __gCOnOffButtonCreator;

//-----------------------------------------------------------------------------
class CCheckBoxCreator : public ViewCreatorAdapter
{
public:
	CCheckBoxCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCCheckBox; }
	IdStringPtr getBaseViewName () const override { return kCControl; }
	UTF8StringPtr getDisplayName () const override { return "Checkbox"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CCheckBox (CRect (0, 0, 100, 20), nullptr, -1, "Title"); }
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* checkbox = dynamic_cast<CCheckBox*> (view);
		if (!checkbox)
			return false;
		
		const std::string* attr = attributes.getAttributeValue (kAttrTitle);
		if (attr)
			checkbox->setTitle (attr->c_str ());

		attr = attributes.getAttributeValue (kAttrFont);
		if (attr)
		{
			CFontRef font = description->getFont (attr->c_str ());
			if (font)
			{
				checkbox->setFont (font);
			}
		}

		CColor color;
		if (stringToColor (attributes.getAttributeValue (kAttrFontColor), color, description))
			checkbox->setFontColor (color);

		if (stringToColor (attributes.getAttributeValue (kAttrBoxframeColor), color, description))
			checkbox->setBoxFrameColor (color);

		if (stringToColor (attributes.getAttributeValue (kAttrBoxfillColor), color, description))
			checkbox->setBoxFillColor (color);

		if (stringToColor (attributes.getAttributeValue (kAttrCheckmarkColor), color, description))
			checkbox->setCheckMarkColor (color);

		int32_t style = checkbox->getStyle ();
		applyStyleMask (attributes.getAttributeValue (kAttrDrawCrossbox), CCheckBox::kDrawCrossBox, style);
		applyStyleMask (attributes.getAttributeValue (kAttrAutosizeToFit), CCheckBox::kAutoSizeToFit, style);
		checkbox->setStyle (style);

		double dv;
		if (attributes.getDoubleAttribute (kAttrFrameWidth, dv))
			checkbox->setFrameWidth (dv);
		if (attributes.getDoubleAttribute (kAttrRoundRectRadius, dv))
			checkbox->setRoundRectRadius (dv);
		return true;
	}

	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrTitle);
		attributeNames.emplace_back (kAttrFont);
		attributeNames.emplace_back (kAttrFontColor);
		attributeNames.emplace_back (kAttrBoxframeColor);
		attributeNames.emplace_back (kAttrBoxfillColor);
		attributeNames.emplace_back (kAttrCheckmarkColor);
		attributeNames.emplace_back (kAttrFrameWidth);
		attributeNames.emplace_back (kAttrRoundRectRadius);
		attributeNames.emplace_back (kAttrAutosizeToFit);
		attributeNames.emplace_back (kAttrDrawCrossbox);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrTitle) return kStringType;
		else if (attributeName == kAttrFont) return kFontType;
		else if (attributeName == kAttrFontColor) return kColorType;
		else if (attributeName == kAttrBoxframeColor) return kColorType;
		else if (attributeName == kAttrBoxfillColor) return kColorType;
		else if (attributeName == kAttrCheckmarkColor) return kColorType;
		else if (attributeName == kAttrFrameWidth) return kFloatType;
		else if (attributeName == kAttrRoundRectRadius) return kFloatType;
		else if (attributeName == kAttrAutosizeToFit) return kBooleanType;
		else if (attributeName == kAttrDrawCrossbox) return kBooleanType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* checkbox = dynamic_cast<CCheckBox*> (view);
		if (!checkbox)
			return false;
		
		if (attributeName == kAttrTitle)
		{
			stringValue = checkbox->getTitle ().getString ();
			return true;
		}
		else if (attributeName == kAttrFont)
		{
			UTF8StringPtr fontName = desc->lookupFontName (checkbox->getFont ());
			if (fontName)
			{
				stringValue = fontName;
				return true;
			}
			return false;
		}
		else if (attributeName == kAttrFontColor)
		{
			colorToString (checkbox->getFontColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == kAttrBoxframeColor)
		{
			colorToString (checkbox->getBoxFrameColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == kAttrBoxfillColor)
		{
			colorToString (checkbox->getBoxFillColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == kAttrCheckmarkColor)
		{
			colorToString (checkbox->getCheckMarkColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == kAttrAutosizeToFit)
		{
			if (checkbox->getStyle () & CCheckBox::kAutoSizeToFit)
				stringValue = strTrue;
			else
				stringValue = strFalse;
			return true;
		}
		else if (attributeName == kAttrDrawCrossbox)
		{
			if (checkbox->getStyle () & CCheckBox::kDrawCrossBox)
				stringValue = strTrue;
			else
				stringValue = strFalse;
			return true;
		}
		else if (attributeName == kAttrFrameWidth)
		{
			stringValue = UIAttributes::doubleToString (checkbox->getFrameWidth ());
			return true;
		}
		else if (attributeName == kAttrRoundRectRadius)
		{
			stringValue = UIAttributes::doubleToString (checkbox->getRoundRectRadius ());
			return true;
		}
		return false;
	}

};
CCheckBoxCreator __gCCheckBoxCreator;

//-----------------------------------------------------------------------------
class CParamDisplayCreator : public ViewCreatorAdapter
{
public:
	CParamDisplayCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCParamDisplay; }
	IdStringPtr getBaseViewName () const override { return kCControl; }
	UTF8StringPtr getDisplayName () const override { return "Parameter Display"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CParamDisplay (CRect (0, 0, 100, 20)); }
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* display = dynamic_cast<CParamDisplay*> (view);
		if (!display)
			return false;
		
		const std::string* fontAttr = attributes.getAttributeValue (kAttrFont);
		if (fontAttr)
		{
			CFontRef font = description->getFont (fontAttr->c_str ());
			if (font)
			{
				display->setFont (font);
			}
		}

		CColor color;
		if (stringToColor (attributes.getAttributeValue (kAttrFontColor), color, description))
			display->setFontColor (color);
		if (stringToColor (attributes.getAttributeValue (kAttrBackColor), color, description))
			display->setBackColor (color);
		if (stringToColor (attributes.getAttributeValue (kAttrFrameColor), color, description))
			display->setFrameColor (color);
		if (stringToColor (attributes.getAttributeValue (kAttrShadowColor), color, description))
			display->setShadowColor (color);

		CPoint p;
		if (attributes.getPointAttribute (kAttrTextInset, p))
			display->setTextInset (p);
		if (attributes.getPointAttribute (kAttrTextShadowOffset, p))
			display->setShadowTextOffset (p);
		if (attributes.getPointAttribute (kAttrBackgroundOffset, p))
			display->setBackOffset (p);
		bool b;
		if (attributes.getBooleanAttribute(kAttrFontAntialias, b))
			display->setAntialias (b);

		const std::string* textAlignmentAttr = attributes.getAttributeValue (kAttrTextAlignment);
		if (textAlignmentAttr)
		{
			CHoriTxtAlign align = kCenterText;
			if (*textAlignmentAttr == strLeft)
				align = kLeftText;
			else if (*textAlignmentAttr == strRight)
				align = kRightText;
			display->setHoriAlign (align);
		}
		double d;
		if (attributes.getDoubleAttribute(kAttrRoundRectRadius, d))
			display->setRoundRectRadius (d);
		if (attributes.getDoubleAttribute(kAttrFrameWidth, d))
			display->setFrameWidth (d);
		if (attributes.getDoubleAttribute (kAttrTextRotation, d))
			display->setTextRotation (d);

		int32_t style = display->getStyle ();
		applyStyleMask (attributes.getAttributeValue (kAttrStyle3DIn), CParamDisplay::k3DIn, style);
		applyStyleMask (attributes.getAttributeValue (kAttrStyle3DOut), CParamDisplay::k3DOut, style);
		applyStyleMask (attributes.getAttributeValue (kAttrStyleNoFrame), CParamDisplay::kNoFrame, style);
		applyStyleMask (attributes.getAttributeValue (kAttrStyleNoDraw), CParamDisplay::kNoDrawStyle, style);
		applyStyleMask (attributes.getAttributeValue (kAttrStyleNoText), CParamDisplay::kNoTextStyle, style);
		applyStyleMask (attributes.getAttributeValue (kAttrStyleShadowText), CParamDisplay::kShadowText, style);
		applyStyleMask (attributes.getAttributeValue (kAttrStyleRoundRect), CParamDisplay::kRoundRectStyle, style);
		display->setStyle (style);

		const std::string* precisionAttr = attributes.getAttributeValue (kAttrValuePrecision);
		if (precisionAttr)
		{
			uint8_t precision = (uint8_t)strtol (precisionAttr->c_str (), nullptr, 10);
			display->setPrecision (precision);
		}

		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrFont);
		attributeNames.emplace_back (kAttrFontColor);
		attributeNames.emplace_back (kAttrBackColor);
		attributeNames.emplace_back (kAttrFrameColor);
		attributeNames.emplace_back (kAttrShadowColor);
		attributeNames.emplace_back (kAttrRoundRectRadius);
		attributeNames.emplace_back (kAttrFrameWidth);
		attributeNames.emplace_back (kAttrTextAlignment);
		attributeNames.emplace_back (kAttrTextInset);
		attributeNames.emplace_back (kAttrTextShadowOffset);
		attributeNames.emplace_back (kAttrValuePrecision);
		attributeNames.emplace_back (kAttrBackgroundOffset);
		attributeNames.emplace_back (kAttrFontAntialias);
		attributeNames.emplace_back (kAttrStyle3DIn);
		attributeNames.emplace_back (kAttrStyle3DOut);
		attributeNames.emplace_back (kAttrStyleNoFrame);
		attributeNames.emplace_back (kAttrStyleNoText);
		attributeNames.emplace_back (kAttrStyleNoDraw);
		attributeNames.emplace_back (kAttrStyleShadowText);
		attributeNames.emplace_back (kAttrStyleRoundRect);
		attributeNames.emplace_back (kAttrTextRotation);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrFont) return kFontType;
		else if (attributeName == kAttrFontColor) return kColorType;
		else if (attributeName == kAttrBackColor) return kColorType;
		else if (attributeName == kAttrFrameColor) return kColorType;
		else if (attributeName == kAttrShadowColor) return kColorType;
		else if (attributeName == kAttrFontAntialias) return kBooleanType;
		else if (attributeName == kAttrStyle3DIn)return kBooleanType;
		else if (attributeName == kAttrStyle3DOut) return kBooleanType;
		else if (attributeName == kAttrStyleNoFrame) return kBooleanType;
		else if (attributeName == kAttrStyleNoText) return kBooleanType;
		else if (attributeName == kAttrStyleNoDraw) return kBooleanType;
		else if (attributeName == kAttrStyleShadowText) return kBooleanType;
		else if (attributeName == kAttrStyleRoundRect) return kBooleanType;
		else if (attributeName == kAttrRoundRectRadius) return kFloatType;
		else if (attributeName == kAttrFrameWidth) return kFloatType;
		else if (attributeName == kAttrTextAlignment) return kStringType;
		else if (attributeName == kAttrTextInset) return kPointType;
		else if (attributeName == kAttrTextShadowOffset) return kPointType;
		else if (attributeName == kAttrValuePrecision) return kIntegerType;
		else if (attributeName == kAttrTextRotation) return kFloatType;
		else if (attributeName == kAttrBackgroundOffset) return kPointType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* pd = dynamic_cast<CParamDisplay*> (view);
		if (pd == nullptr)
			return false;
		if (attributeName == kAttrFont)
		{
			UTF8StringPtr fontName = desc->lookupFontName (pd->getFont ());
			if (fontName)
			{
				stringValue = fontName;
				return true;
			}
			return false;
		}
		else if (attributeName == kAttrFontColor)
		{
			colorToString (pd->getFontColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == kAttrBackColor)
		{
			colorToString (pd->getBackColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == kAttrFrameColor)
		{
			colorToString (pd->getFrameColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == kAttrShadowColor)
		{
			colorToString (pd->getShadowColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == kAttrTextInset)
		{
			stringValue = UIAttributes::pointToString (pd->getTextInset ());
			return true;
		}
		else if (attributeName == kAttrTextShadowOffset)
		{
			stringValue = UIAttributes::pointToString (pd->getShadowTextOffset ());
			return true;
		}
		else if (attributeName == kAttrFontAntialias)
		{
			stringValue = pd->getAntialias () ? strTrue : strFalse;
			return true;
		}
		else if (attributeName == kAttrStyle3DIn)
		{
			stringValue = pd->getStyle () & CParamDisplay::k3DIn ? strTrue : strFalse;
			return true;
		}
		else if (attributeName == kAttrStyle3DOut)
		{
			stringValue = pd->getStyle () & CParamDisplay::k3DOut ? strTrue : strFalse;
			return true;
		}
		else if (attributeName == kAttrStyleNoFrame)
		{
			stringValue = pd->getStyle () & CParamDisplay::kNoFrame ? strTrue : strFalse;
			return true;
		}
		else if (attributeName == kAttrStyleNoText)
		{
			stringValue = pd->getStyle () & CParamDisplay::kNoTextStyle ? strTrue : strFalse;
			return true;
		}
		else if (attributeName == kAttrStyleNoDraw)
		{
			stringValue = pd->getStyle () & CParamDisplay::kNoDrawStyle ? strTrue : strFalse;
			return true;
		}
		else if (attributeName == kAttrStyleShadowText)
		{
			stringValue = pd->getStyle () & CParamDisplay::kShadowText ? strTrue : strFalse;
			return true;
		}
		else if (attributeName == kAttrStyleRoundRect)
		{
			stringValue = pd->getStyle () & CParamDisplay::kRoundRectStyle ? strTrue : strFalse;
			return true;
		}
		else if (attributeName == kAttrRoundRectRadius)
		{
			stringValue = UIAttributes::doubleToString (pd->getRoundRectRadius ());
			return true;
		}
		else if (attributeName == kAttrFrameWidth)
		{
			stringValue = UIAttributes::doubleToString (pd->getFrameWidth ());
			return true;
		}
		else if (attributeName == kAttrTextAlignment)
		{
			CHoriTxtAlign align = pd->getHoriAlign ();
			switch (align)
			{
				case kLeftText: stringValue = strLeft; break;
				case kRightText: stringValue = strRight; break;
				case kCenterText: stringValue = strCenter; break;
			}
			return true;
		}
		else if (attributeName == kAttrValuePrecision)
		{
			stringValue = UIAttributes::integerToString (static_cast<int32_t> (pd->getPrecision ()));
			return true;
		}
		else if (attributeName == kAttrTextRotation)
		{
			stringValue = UIAttributes::doubleToString (pd->getTextRotation ());
			return true;
		}
		else if (attributeName == kAttrBackgroundOffset)
		{
			stringValue = UIAttributes::pointToString (pd->getBackOffset ());
			return true;
		}
		return false;
	}
	bool getAttributeValueRange (const std::string& attributeName, double& minValue, double &maxValue) const override
	{
		if (attributeName == kAttrTextRotation)
		{
			minValue = 0.;
			maxValue = 360.;
			return true;
		}
		return false;
	}

};
CParamDisplayCreator __gCParamDisplayCreator;

//-----------------------------------------------------------------------------
class COptionMenuCreator : public ViewCreatorAdapter
{
public:
	COptionMenuCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCOptionMenu; }
	IdStringPtr getBaseViewName () const override { return kCParamDisplay; }
	UTF8StringPtr getDisplayName () const override { return "Option Menu"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new COptionMenu (CRect (0, 0, 100, 20), nullptr, -1); }
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* menu = dynamic_cast<COptionMenu*> (view);
		if (!menu)
			return false;

		int32_t style = menu->getStyle ();
		applyStyleMask (attributes.getAttributeValue (kAttrMenuPopupStyle), COptionMenu::kPopupStyle, style);
		applyStyleMask (attributes.getAttributeValue (kAttrMenuCheckStyle), COptionMenu::kCheckStyle, style);
		menu->setStyle (style);

		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrMenuPopupStyle);
		attributeNames.emplace_back (kAttrMenuCheckStyle);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrMenuPopupStyle) return kBooleanType;
		if (attributeName == kAttrMenuCheckStyle) return kBooleanType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* menu = dynamic_cast<COptionMenu*> (view);
		if (!menu)
			return false;
		if (attributeName == kAttrMenuPopupStyle)
		{
			stringValue = (menu->getStyle () & COptionMenu::kPopupStyle) ? strTrue : strFalse;
			return true;
		}
		if (attributeName == kAttrMenuCheckStyle)
		{
			stringValue = (menu->getStyle () & COptionMenu::kCheckStyle) ? strTrue : strFalse;
			return true;
		}
		return false;
	}

};
COptionMenuCreator __gCOptionMenuCreator;

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
class CSegmentButtonCreator : public ViewCreatorAdapter
{
public:
	std::string SelectionModeSingle = "Single";
	std::string SelectionModeMultiple = "Multiple";

	CSegmentButtonCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCSegmentButton; }
	IdStringPtr getBaseViewName () const override { return kCControl; }
	UTF8StringPtr getDisplayName () const override { return "Segment Button"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override
	{
		CSegmentButton* button = new CSegmentButton (CRect (0, 0, 200, 20));
		updateSegmentCount (button, 4);
		return button;
	}
	void updateSegmentCount (CSegmentButton* button, uint32_t numSegments) const
	{
		if (button->getSegments ().size () != numSegments)
		{
			button->removeAllSegments ();
			for (uint32_t i = 0; i < numSegments; i++)
			{
				std::stringstream str;
				str << "Segment ";
				str << i + 1;
				CSegmentButton::Segment seg;
				seg.name = str.str ().c_str ();
				button->addSegment (std::move (seg));
			}
		}
	}
	void updateSegments (CSegmentButton* button, const UIAttributes::StringArray& names) const
	{
		button->removeAllSegments ();
		for (const auto& name : names)
		{
			CSegmentButton::Segment segment;
			segment.name = name.c_str ();
			button->addSegment (std::move (segment));
		}
	}
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* button = dynamic_cast<CSegmentButton*> (view);
		if (!button)
			return false;

		const std::string* attr = attributes.getAttributeValue (kAttrFont);
		if (attr)
		{
			CFontRef font = description->getFont (attr->c_str ());
			if (font)
			{
				button->setFont (font);
			}
		}

		attr = attributes.getAttributeValue (kAttrStyle);
		if (attr)
			button->setStyle (*attr == strHorizontal ? CSegmentButton::Style::kHorizontal : CSegmentButton::Style::kVertical);

		CColor color;
		if (stringToColor (attributes.getAttributeValue (kAttrTextColor), color, description))
			button->setTextColor (color);
		if (stringToColor (attributes.getAttributeValue (kAttrTextColorHighlighted), color, description))
			button->setTextColorHighlighted (color);
		if (stringToColor (attributes.getAttributeValue (kAttrFrameColor), color, description))
			button->setFrameColor (color);

		double d;
		if (attributes.getDoubleAttribute (kAttrFrameWidth, d))
			button->setFrameWidth (d);
		if (attributes.getDoubleAttribute (kAttrRoundRadius, d))
			button->setRoundRadius (d);
		if (attributes.getDoubleAttribute (kAttrIconTextMargin, d))
			button->setTextMargin (d);
		
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

		UIAttributes::StringArray segmentNames;
		if (attributes.getStringArrayAttribute (kAttrSegmentNames, segmentNames))
			updateSegments (button, segmentNames);

		attr = attributes.getAttributeValue (kAttrTruncateMode);
		if (attr)
		{
			if (*attr == strHead)
				button->setTextTruncateMode (CDrawMethods::kTextTruncateHead);
			else if (*attr == strTail)
				button->setTextTruncateMode (CDrawMethods::kTextTruncateTail);
			else
				button->setTextTruncateMode (CDrawMethods::kTextTruncateNone);
		}
		attr = attributes.getAttributeValue (kAttrSelectionMode);
		if (attr)
		{
			if (*attr == SelectionModeSingle)
				button->setSelectionMode (CSegmentButton::SelectionMode::kSingle);
			else if (*attr == SelectionModeMultiple)
				button->setSelectionMode (CSegmentButton::SelectionMode::kMultiple);
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrStyle);
		attributeNames.emplace_back (kAttrSelectionMode);
		attributeNames.emplace_back (kAttrSegmentNames);
		attributeNames.emplace_back (kAttrFont);
		attributeNames.emplace_back (kAttrTextColor);
		attributeNames.emplace_back (kAttrTextColorHighlighted);
		attributeNames.emplace_back (kAttrGradient);
		attributeNames.emplace_back (kAttrGradientHighlighted);
		attributeNames.emplace_back (kAttrFrameColor);
		attributeNames.emplace_back (kAttrRoundRadius);
		attributeNames.emplace_back (kAttrFrameWidth);
		attributeNames.emplace_back (kAttrIconTextMargin);
		attributeNames.emplace_back (kAttrTextAlignment);
		attributeNames.emplace_back (kAttrTruncateMode);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrStyle) return kListType;
		if (attributeName == kAttrSelectionMode) return kListType;
		if (attributeName == kAttrSegmentNames) return kStringType;
		if (attributeName == kAttrFont) return kFontType;
		if (attributeName == kAttrTextColor) return kColorType;
		if (attributeName == kAttrTextColorHighlighted) return kColorType;
		if (attributeName == kAttrGradient) return kGradientType;
		if (attributeName == kAttrGradientHighlighted) return kGradientType;
		if (attributeName == kAttrFrameColor) return kColorType;
		if (attributeName == kAttrFrameWidth) return kFloatType;
		if (attributeName == kAttrRoundRadius) return kFloatType;
		if (attributeName == kAttrIconTextMargin) return kFloatType;
		if (attributeName == kAttrTextAlignment) return kStringType;
		if (attributeName == kAttrTruncateMode) return kListType;
		return kUnknownType;
	}
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const override
	{
		if (attributeName == kAttrStyle)
		{
			return getStandardAttributeListValues (kAttrOrientation, values);
		}
		else if (attributeName == kAttrSelectionMode)
		{
			values.push_back (&SelectionModeSingle);
			values.push_back (&SelectionModeMultiple);
			return true;
		}
		else if (attributeName == kAttrTruncateMode)
		{
			return getStandardAttributeListValues (kAttrTruncateMode, values);
		}
		return false;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* button = dynamic_cast<CSegmentButton*> (view);
		if (!button)
			return false;
		if (attributeName == kAttrFont)
		{
			UTF8StringPtr fontName = desc->lookupFontName (button->getFont ());
			if (fontName)
			{
				stringValue = fontName;
				return true;
			}
			return false;
		}
		else if (attributeName == kAttrSegmentNames)
		{
			const CSegmentButton::Segments& segments = button->getSegments ();
			UIAttributes::StringArray stringArray;
			for (const auto& segment : segments)
				stringArray.emplace_back (segment.name.getString ());
			stringValue = UIAttributes::stringArrayToString (stringArray);
			return true;
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
		else if (attributeName == kAttrStyle)
		{
			stringValue = button->getStyle () == CSegmentButton::Style::kHorizontal ? strHorizontal :
			                                                                          strVertical;
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
			if (gradient)
			{
				UTF8StringPtr gradientName = desc->lookupGradientName (gradient);
				stringValue = gradientName ? gradientName : "";
			}
			return true;
		}
		else if (attributeName == kAttrGradientHighlighted)
		{
			CGradient* gradient = button->getGradientHighlighted ();
			if (gradient)
			{
				UTF8StringPtr gradientName = desc->lookupGradientName (gradient);
				stringValue = gradientName ? gradientName : "";
			}
			return true;
		}
		else if (attributeName == kAttrTruncateMode)
		{
			switch (button->getTextTruncateMode ())
			{
				case CDrawMethods::kTextTruncateHead: stringValue = strHead; break;
				case CDrawMethods::kTextTruncateTail: stringValue = strTail; break;
				case CDrawMethods::kTextTruncateNone: stringValue = ""; break;
			}
			return true;
		}
		else if (attributeName == kAttrSelectionMode)
		{
			switch (button->getSelectionMode ())
			{
				case CSegmentButton::SelectionMode::kSingle:
				{
					stringValue = SelectionModeSingle;
					break;
				}
				case CSegmentButton::SelectionMode::kMultiple:
				{
					stringValue = SelectionModeMultiple;
					break;
				}
			}
			return true;
		}
		return false;
	}

};
CSegmentButtonCreator __gCSegmentButtonCreator;

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
		auto* knob = dynamic_cast<CKnob*> (view);
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
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CKnob (CRect (0, 0, 0, 0), nullptr, -1, nullptr, nullptr); }
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
class CSliderCreator : public ViewCreatorAdapter
{
public:
	std::string kTouch = "touch";
	std::string kRelativeTouch = "relative touch";
	std::string kFreeClick = "free click";
	std::string kRamp = "ramp";
	std::string kUseGlobal = "use global";

	CSliderCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCSlider; }
	IdStringPtr getBaseViewName () const override { return kCControl; }
	UTF8StringPtr getDisplayName () const override { return "Slider"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CSlider (CRect (0, 0, 0, 0), nullptr, -1, 0, 0, nullptr, nullptr); }
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* slider = dynamic_cast<CSlider*> (view);
		if (!slider)
			return false;

		// support old attribute name and convert it
		const std::string* freeClickAttr = attributes.getAttributeValue ("free-click");
		if (freeClickAttr)
		{
			slider->setSliderMode (*freeClickAttr == strTrue ? CSliderMode::FreeClick : CSliderMode::Touch);
		}

		const std::string* modeAttr = attributes.getAttributeValue (kAttrMode);
		if (modeAttr)
		{
			if (*modeAttr == kTouch)
				slider->setSliderMode (CSliderMode::Touch);
			else if (*modeAttr == kRelativeTouch)
				slider->setSliderMode (CSliderMode::RelativeTouch);
			else if (*modeAttr == kFreeClick)
				slider->setSliderMode (CSliderMode::FreeClick);
			else if (*modeAttr == kRamp)
				slider->setSliderMode (CSliderMode::Ramp);
			else if (*modeAttr == kUseGlobal)
				slider->setSliderMode (CSliderMode::UseGlobal);
		}
		CBitmap* bitmap;
		if (stringToBitmap (attributes.getAttributeValue (kAttrHandleBitmap), bitmap, description))
			slider->setHandle (bitmap);

		CPoint p;
		if (attributes.getPointAttribute (kAttrHandleOffset, p))
			slider->setOffsetHandle (p);
		if (attributes.getPointAttribute (kAttrBitmapOffset, p))
			slider->setOffset (p);

		double d;
		if (attributes.getDoubleAttribute (kAttrZoomFactor, d))
			slider->setZoomFactor (static_cast<float>(d));

		const std::string* orientationAttr = attributes.getAttributeValue (kAttrOrientation);
		if (orientationAttr)
		{
			int32_t style = slider->getStyle ();
			if (*orientationAttr == strVertical)
			{
				setBit (style, CSlider::kHorizontal, false);
				setBit (style, CSlider::kVertical, true);
			}
			else
			{
				setBit (style, CSlider::kVertical, false);
				setBit (style, CSlider::kHorizontal, true);
			}
			slider->setStyle (style);
		}
		const std::string* reverseOrientationAttr = attributes.getAttributeValue (kAttrReverseOrientation);
		if (reverseOrientationAttr)
		{
			int32_t style = slider->getStyle ();
			if (*reverseOrientationAttr == strTrue)
			{
				if (style & CSlider::kVertical)
				{
					setBit (style, CSlider::kBottom, false);
					setBit (style, CSlider::kTop, true);
				}
				else if (style & CSlider::kHorizontal)
				{
					setBit (style, CSlider::kLeft, false);
					setBit (style, CSlider::kRight, true);
				}
			}
			else
			{
				if (style & CSlider::kVertical)
				{
					setBit (style, CSlider::kTop, false);
					setBit (style, CSlider::kBottom, true);
				}
				else if (style & CSlider::kHorizontal)
				{
					setBit (style, CSlider::kRight, false);
					setBit (style, CSlider::kLeft, true);
				}
			}
			slider->setStyle (style);
		}

		int32_t drawStyle = slider->getDrawStyle ();
		applyStyleMask (attributes.getAttributeValue (kAttrDrawFrame), CSlider::kDrawFrame, drawStyle);
		applyStyleMask (attributes.getAttributeValue (kAttrDrawBack), CSlider::kDrawBack, drawStyle);
		applyStyleMask (attributes.getAttributeValue (kAttrDrawValue), CSlider::kDrawValue, drawStyle);
		applyStyleMask (attributes.getAttributeValue (kAttrDrawValueFromCenter), CSlider::kDrawValueFromCenter, drawStyle);
		applyStyleMask (attributes.getAttributeValue (kAttrDrawValueInverted), CSlider::kDrawInverted, drawStyle);
		slider->setDrawStyle (drawStyle);

		CCoord lineWidth;
		if (attributes.getDoubleAttribute(kAttrFrameWidth, lineWidth))
			slider->setFrameWidth (lineWidth);

		CColor color;
		if (stringToColor (attributes.getAttributeValue (kAttrDrawFrameColor), color, description))
			slider->setFrameColor (color);
		if (stringToColor (attributes.getAttributeValue (kAttrDrawBackColor), color, description))
			slider->setBackColor (color);
		if (stringToColor (attributes.getAttributeValue (kAttrDrawValueColor), color, description))
			slider->setValueColor (color);
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrMode);
		attributeNames.emplace_back (kAttrHandleBitmap);
		attributeNames.emplace_back (kAttrHandleOffset);
		attributeNames.emplace_back (kAttrBitmapOffset);
		attributeNames.emplace_back (kAttrZoomFactor);
		attributeNames.emplace_back (kAttrOrientation);
		attributeNames.emplace_back (kAttrReverseOrientation);
		attributeNames.emplace_back (kAttrDrawFrame);
		attributeNames.emplace_back (kAttrDrawBack);
		attributeNames.emplace_back (kAttrDrawValue);
		attributeNames.emplace_back (kAttrDrawValueFromCenter);
		attributeNames.emplace_back (kAttrDrawValueInverted);
		attributeNames.emplace_back (kAttrFrameWidth);
		attributeNames.emplace_back (kAttrDrawFrameColor);
		attributeNames.emplace_back (kAttrDrawBackColor);
		attributeNames.emplace_back (kAttrDrawValueColor);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrMode) return kListType;
		if (attributeName == kAttrHandleBitmap) return kBitmapType;
		if (attributeName == kAttrHandleOffset) return kPointType;
		if (attributeName == kAttrBitmapOffset) return kPointType;
		if (attributeName == kAttrZoomFactor) return kFloatType;
		if (attributeName == kAttrOrientation) return kListType;
		if (attributeName == kAttrReverseOrientation) return kBooleanType;
		if (attributeName == kAttrDrawFrame) return kBooleanType;
		if (attributeName == kAttrDrawBack) return kBooleanType;
		if (attributeName == kAttrDrawValue) return kBooleanType;
		if (attributeName == kAttrDrawValueFromCenter) return kBooleanType;
		if (attributeName == kAttrDrawValueInverted) return kBooleanType;
		if (attributeName == kAttrFrameWidth) return kFloatType;
		if (attributeName == kAttrDrawFrameColor) return kColorType;
		if (attributeName == kAttrDrawBackColor) return kColorType;
		if (attributeName == kAttrDrawValueColor) return kColorType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* slider = dynamic_cast<CSlider*> (view);
		if (!slider)
			return false;
		if (attributeName == kAttrMode)
		{
			switch (slider->getSliderMode ())
			{
				case CSliderMode::Touch:
					stringValue = kTouch; break;
				case CSliderMode::RelativeTouch:
					stringValue = kRelativeTouch; break;
				case CSliderMode::FreeClick:
					stringValue = kFreeClick; break;
				case CSliderMode::Ramp:
					stringValue = kRamp; break;
				case CSliderMode::UseGlobal:
					stringValue = kUseGlobal; break;
			}
			return true;
		}
		else if (attributeName == kAttrHandleBitmap)
		{
			CBitmap* bitmap = slider->getHandle ();
			if (bitmap)
			{
				bitmapToString (bitmap, stringValue, desc);
			}
			return true;
		}
		else if (attributeName == kAttrHandleOffset)
		{
			stringValue = UIAttributes::pointToString (slider->getOffsetHandle ());
			return true;
		}
		else if (attributeName == kAttrBitmapOffset)
		{
			stringValue = UIAttributes::pointToString (slider->getOffset ());
			return true;
		}
		else if (attributeName == kAttrZoomFactor)
		{
			stringValue = UIAttributes::doubleToString (slider->getZoomFactor ());
			return true;
		}
		else if (attributeName == kAttrOrientation)
		{
			if (slider->getStyle () & CSlider::kVertical)
				stringValue = strVertical;
			else
				stringValue = strHorizontal;
			return true;
		}
		else if (attributeName == kAttrReverseOrientation)
		{
			int32_t style = slider->getStyle ();
			stringValue = strFalse;
			if (((style & CSlider::kVertical) && (style & CSlider::kTop)) || ((style & CSlider::kHorizontal) && (style & CSlider::kRight)))
				stringValue = strTrue;
			else
				stringValue = strFalse;
			return true;
		}
		else if (attributeName == kAttrDrawFrame)
		{
			if (slider->getDrawStyle () & CSlider::kDrawFrame)
				stringValue = strTrue;
			else
				stringValue = strFalse;
			return true;
		}
		else if (attributeName == kAttrDrawBack)
		{
			if (slider->getDrawStyle () & CSlider::kDrawBack)
				stringValue = strTrue;
			else
				stringValue = strFalse;
			return true;
		}
		else if (attributeName == kAttrDrawValue)
		{
			if (slider->getDrawStyle () & CSlider::kDrawValue)
				stringValue = strTrue;
			else
				stringValue = strFalse;
			return true;
		}
		else if (attributeName == kAttrDrawValueFromCenter)
		{
			if (slider->getDrawStyle () & CSlider::kDrawValueFromCenter)
				stringValue = strTrue;
			else
				stringValue = strFalse;
			return true;
		}
		else if (attributeName == kAttrDrawValueInverted)
		{
			if (slider->getDrawStyle () & CSlider::kDrawInverted)
				stringValue = strTrue;
			else
				stringValue = strFalse;
			return true;
		}
		else if (attributeName == kAttrDrawFrameColor)
		{
			colorToString (slider->getFrameColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == kAttrDrawBackColor)
		{
			colorToString (slider->getBackColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == kAttrDrawValueColor)
		{
			colorToString (slider->getValueColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == kAttrFrameWidth)
		{
			stringValue = UIAttributes::doubleToString (slider->getFrameWidth ());
			return true;
		}

		return false;
	}
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const override
	{
		if (attributeName == kAttrOrientation)
		{
			return getStandardAttributeListValues (kAttrOrientation, values);
		}
		if (attributeName == kAttrMode)
		{
			values.emplace_back (&kTouch);
			values.emplace_back (&kRelativeTouch);
			values.emplace_back (&kFreeClick);
			values.emplace_back (&kRamp);
			values.emplace_back (&kUseGlobal);
			return true;
		}
		return false;
	}

};
CSliderCreator __gCSliderCreator;

//-----------------------------------------------------------------------------
class CVuMeterCreator : public ViewCreatorAdapter
{
public:
	CVuMeterCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCVuMeter; }
	IdStringPtr getBaseViewName () const override { return kCControl; }
	UTF8StringPtr getDisplayName () const override { return "VU Meter"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CVuMeter (CRect (0, 0, 0, 0), nullptr, nullptr, 100); }
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* vuMeter = dynamic_cast<CVuMeter*> (view);
		if (!vuMeter)
			return false;

		CBitmap* bitmap;
		if (stringToBitmap (attributes.getAttributeValue (kAttrOffBitmap), bitmap, description))
			vuMeter->setOffBitmap (bitmap);

		const std::string* attr = attributes.getAttributeValue (kAttrOrientation);
		if (attr)
			vuMeter->setStyle (*attr == strVertical ? CVuMeter::kVertical : CVuMeter::kHorizontal);
		
		int32_t numLed;
		if (attributes.getIntegerAttribute(kAttrNumLed, numLed))
			vuMeter->setNbLed (numLed);

		double value;
		if (attributes.getDoubleAttribute(kAttrDecreaseStepValue, value))
			vuMeter->setDecreaseStepValue (static_cast<float>(value));
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrOffBitmap);
		attributeNames.emplace_back (kAttrNumLed);
		attributeNames.emplace_back (kAttrOrientation);
		attributeNames.emplace_back (kAttrDecreaseStepValue);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrOffBitmap) return kBitmapType;
		if (attributeName == kAttrNumLed) return kIntegerType;
		if (attributeName == kAttrOrientation) return kListType;
		if (attributeName == kAttrDecreaseStepValue) return kFloatType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* vuMeter = dynamic_cast<CVuMeter*> (view);
		if (!vuMeter)
			return false;
		if (attributeName == kAttrOffBitmap)
		{
			CBitmap* bitmap = vuMeter->getOffBitmap ();
			if (bitmap)
			{
				bitmapToString (bitmap, stringValue, desc);
			}
			return true;
		}
		else if (attributeName == kAttrOrientation)
		{
			if (vuMeter->getStyle () & CVuMeter::kVertical)
				stringValue = strVertical;
			else
				stringValue = strHorizontal;
			return true;
		}
		else if (attributeName == kAttrNumLed)
		{
			stringValue = UIAttributes::integerToString (vuMeter->getNbLed ());
			return true;
		}
		else if (attributeName == kAttrDecreaseStepValue)
		{
			stringValue = UIAttributes::doubleToString (vuMeter->getDecreaseStepValue ());
			return true;
		}
		return false;
	}
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const override
	{
		if (attributeName == kAttrOrientation)
		{
			return getStandardAttributeListValues (kAttrOrientation, values);
		}
		return false;
	}

};
CVuMeterCreator __gCVuMeterCreator;

//-----------------------------------------------------------------------------
class CAnimationSplashScreenCreator : public ViewCreatorAdapter
{
public:
	CAnimationSplashScreenCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCAnimationSplashScreen; }
	IdStringPtr getBaseViewName () const override { return kCControl; }
	UTF8StringPtr getDisplayName () const override { return "Animation Splash Screen"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CAnimationSplashScreen (CRect (0, 0, 0, 0), -1, nullptr, nullptr); }
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* splashScreen = dynamic_cast<CAnimationSplashScreen*> (view);
		if (!splashScreen)
			return false;

		CBitmap* bitmap;
		if (stringToBitmap (attributes.getAttributeValue (kAttrSplashBitmap), bitmap, description))
			splashScreen->setSplashBitmap (bitmap);

		CPoint p;
		if (attributes.getPointAttribute (kAttrSplashOrigin, p))
		{
			CRect size = splashScreen->getSplashRect ();
			size.originize ();
			size.offset (p.x, p.y);
			splashScreen->setSplashRect (size);
		}
		if (attributes.getPointAttribute (kAttrSplashSize, p))
		{
			CRect size = splashScreen->getSplashRect ();
			size.setWidth (p.x);
			size.setHeight (p.y);
			splashScreen->setSplashRect (size);
		}
		int32_t value;
		if (attributes.getIntegerAttribute (kAttrAnimationIndex, value))
			splashScreen->setAnimationIndex (static_cast<uint32_t> (value));
		if (attributes.getIntegerAttribute (kAttrAnimationTime, value))
			splashScreen->setAnimationTime (static_cast<uint32_t> (value));

		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrSplashBitmap);
		attributeNames.emplace_back (kAttrSplashOrigin);
		attributeNames.emplace_back (kAttrSplashSize);
		attributeNames.emplace_back (kAttrAnimationIndex);
		attributeNames.emplace_back (kAttrAnimationTime);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrSplashBitmap) return kBitmapType;
		if (attributeName == kAttrSplashOrigin) return kRectType;
		if (attributeName == kAttrSplashSize) return kRectType;
		if (attributeName == kAttrAnimationIndex) return kIntegerType;
		if (attributeName == kAttrAnimationTime) return kIntegerType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* splashScreen = dynamic_cast<CAnimationSplashScreen*> (view);
		if (!splashScreen)
			return false;

		if (attributeName == kAttrSplashBitmap)
		{
			CBitmap* bitmap = splashScreen->getSplashBitmap ();
			if (bitmap)
				bitmapToString (bitmap, stringValue, desc);
			else
				stringValue = "";
			return true;
		}
		else if (attributeName == kAttrSplashOrigin)
		{
			stringValue = UIAttributes::pointToString (splashScreen->getSplashRect ().getTopLeft ());
			return true;
		}
		else if (attributeName == kAttrSplashSize)
		{
			stringValue = UIAttributes::pointToString (splashScreen->getSplashRect ().getSize ());
			return true;
		}
		else if (attributeName == kAttrAnimationIndex)
		{
			stringValue = UIAttributes::integerToString (splashScreen->getAnimationIndex ());
			return true;
		}
		else if (attributeName == kAttrAnimationTime)
		{
			stringValue = UIAttributes::integerToString (splashScreen->getAnimationTime ());
			return true;
		}
		return false;
	}

};
CAnimationSplashScreenCreator __gCAnimationSplashScreenCreator;

//-----------------------------------------------------------------------------
class UIViewSwitchContainerCreator : public ViewCreatorAdapter
{
public:
	std::string kLinear = "linear";
	std::string kEasyIn = "easy-in";
	std::string kEasyOut = "easy-out";
	std::string kEasyInOut = "easy-in-out";
	std::string kEasy = "easy";

	std::string kFadeInOut = "fade";
	std::string kMoveInOut = "move";
	std::string kPushInOut = "push";

	UIViewSwitchContainerCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kUIViewSwitchContainer; }
	IdStringPtr getBaseViewName () const override { return kCViewContainer; }
	UTF8StringPtr getDisplayName () const override { return "View Switch Container"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override
	{
		UIViewSwitchContainer* vsc = new UIViewSwitchContainer (CRect (0, 0, 100, 100));
		new UIDescriptionViewSwitchController (vsc, description, description->getController ());
		return vsc;
	}

	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* viewSwitch = dynamic_cast<UIViewSwitchContainer*> (view);
		if (!viewSwitch)
			return false;
		const std::string* attr = attributes.getAttributeValue (kAttrTemplateNames);
		if (attr)
		{
			auto* controller = dynamic_cast<UIDescriptionViewSwitchController*> (viewSwitch->getController ());
			if (controller)
			{
				controller->setTemplateNames (attr->c_str ());
			}
		}
		attr = attributes.getAttributeValue (kAttrTemplateSwitchControl);
		if (attr)
		{
			auto* controller = dynamic_cast<UIDescriptionViewSwitchController*> (viewSwitch->getController ());
			if (controller)
			{
				int32_t tag = description->getTagForName (attr->c_str ());
				controller->setSwitchControlTag (tag);
			}
		}
		attr = attributes.getAttributeValue (kAttrAnimationStyle);
		if (attr)
		{
			UIViewSwitchContainer::AnimationStyle style = UIViewSwitchContainer::kFadeInOut;
			if (*attr == kMoveInOut)
				style = UIViewSwitchContainer::kMoveInOut;
			else if (*attr == kPushInOut)
				style = UIViewSwitchContainer::kPushInOut;
			viewSwitch->setAnimationStyle (style);
		}
		
		attr = attributes.getAttributeValue (kAttrAnimationTimingFunction);
		if (attr)
		{
			UIViewSwitchContainer::TimingFunction tf = UIViewSwitchContainer::kLinear;
			if (*attr == kEasyIn)
				tf = UIViewSwitchContainer::kEasyIn;
			else if (*attr == kEasyOut)
				tf = UIViewSwitchContainer::kEasyOut;
			else if (*attr == kEasyInOut)
				tf = UIViewSwitchContainer::kEasyInOut;
			else if (*attr == kEasy)
				tf = UIViewSwitchContainer::kEasy;
			viewSwitch->setTimingFunction (tf);
		}
		
		int32_t animationTime;
		if (attributes.getIntegerAttribute (kAttrAnimationTime, animationTime))
		{
			viewSwitch->setAnimationTime (static_cast<uint32_t> (animationTime));
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrTemplateNames);
		attributeNames.emplace_back (kAttrTemplateSwitchControl);
		attributeNames.emplace_back (kAttrAnimationStyle);
		attributeNames.emplace_back (kAttrAnimationTimingFunction);
		attributeNames.emplace_back (kAttrAnimationTime);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrTemplateNames) return kStringType;
		if (attributeName == kAttrTemplateSwitchControl) return kTagType;
		if (attributeName == kAttrAnimationStyle) return kListType;
		if (attributeName == kAttrAnimationTimingFunction) return kListType;
		if (attributeName == kAttrAnimationTime) return kIntegerType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* viewSwitch = dynamic_cast<UIViewSwitchContainer*> (view);
		if (!viewSwitch)
			return false;
		if (attributeName == kAttrTemplateNames)
		{
			auto* controller = dynamic_cast<UIDescriptionViewSwitchController*> (viewSwitch->getController ());
			if (controller)
			{
				controller->getTemplateNames (stringValue);
				return true;
			}
		}
		else if (attributeName == kAttrTemplateSwitchControl)
		{
			auto* controller = dynamic_cast<UIDescriptionViewSwitchController*> (viewSwitch->getController ());
			if (controller)
			{
				UTF8StringPtr controlTag = desc->lookupControlTagName (controller->getSwitchControlTag ());
				if (controlTag)
				{
					stringValue = controlTag;
					return true;
				}
				return true;
			}
		}
		else if (attributeName == kAttrAnimationTime)
		{
			stringValue = UIAttributes::integerToString (static_cast<int32_t> (viewSwitch->getAnimationTime ()));
			return true;
		}
		else if (attributeName == kAttrAnimationStyle)
		{
			switch (viewSwitch->getAnimationStyle ())
			{
				case UIViewSwitchContainer::kFadeInOut:
				{
					stringValue = kFadeInOut;
					return true;
				}
				case UIViewSwitchContainer::kMoveInOut:
				{
					stringValue = kMoveInOut;
					return true;
				}
				case UIViewSwitchContainer::kPushInOut:
				{
					stringValue = kPushInOut;
					return true;
				}
			}
		}
		else if (attributeName == kAttrAnimationTimingFunction)
		{
			switch (viewSwitch->getTimingFunction ())
			{
				case UIViewSwitchContainer::kLinear:
				{
					stringValue = kLinear;
					return true;
				}
				case UIViewSwitchContainer::kEasyIn:
				{
					stringValue = kEasyIn;
					return true;
				}
				case UIViewSwitchContainer::kEasyOut:
				{
					stringValue = kEasyOut;
					return true;
				}
				case UIViewSwitchContainer::kEasyInOut:
				{
					stringValue = kEasyInOut;
					return true;
				}
				case UIViewSwitchContainer::kEasy:
				{
					stringValue = kEasy;
					return true;
				}
			}
		}
		return false;
	}
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const override
	{
		if (attributeName == kAttrAnimationStyle)
		{
			values.emplace_back (&kFadeInOut);
			values.emplace_back (&kMoveInOut);
			values.emplace_back (&kPushInOut);
			return true;
		}
		if (attributeName == kAttrAnimationTimingFunction)
		{
			values.emplace_back (&kLinear);
			values.emplace_back (&kEasyIn);
			values.emplace_back (&kEasyOut);
			values.emplace_back (&kEasyInOut);
			values.emplace_back (&kEasy);
			return true;
		}
		return false;
	}
};
UIViewSwitchContainerCreator __gUIViewSwitchContainerCreator;

//-----------------------------------------------------------------------------
class CSplitViewCreator : public ViewCreatorAdapter
{
public:
	std::string kFirst = "first";
	std::string kSecond = "second";
	std::string kLast = "last";
	std::string kAll = "all";


	CSplitViewCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCSplitView; }
	IdStringPtr getBaseViewName () const override { return kCViewContainer; }
	UTF8StringPtr getDisplayName () const override { return "Split View"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CSplitView (CRect (0, 0, 100, 100)); }

	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* splitView = dynamic_cast<CSplitView*> (view);
		if (!splitView)
			return false;

		int32_t width;
		if (attributes.getIntegerAttribute (kAttrSeparatorWidth, width))
			splitView->setSeparatorWidth (width);
		const std::string* attr = attributes.getAttributeValue (kAttrOrientation);
		if (attr)
		{
			if (*attr == strHorizontal)
			{
				splitView->setStyle (CSplitView::kHorizontal);
			}
			else
			{
				splitView->setStyle (CSplitView::kVertical);
			}
		}
		attr = attributes.getAttributeValue (kAttrResizeMethod);
		if (attr)
		{
			if (*attr == kFirst)
			{
				splitView->setResizeMethod (CSplitView::kResizeFirstView);
			}
			if (*attr == kSecond)
			{
				splitView->setResizeMethod (CSplitView::kResizeSecondView);
			}
			else if (*attr == kLast)
			{
				splitView->setResizeMethod (CSplitView::kResizeLastView);
			}
			else if (*attr == kAll)
			{
				splitView->setResizeMethod (CSplitView::kResizeAllViews);
			}
		}

		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrOrientation);
		attributeNames.emplace_back (kAttrResizeMethod);
		attributeNames.emplace_back (kAttrSeparatorWidth);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrOrientation) return kListType;
		if (attributeName == kAttrResizeMethod) return kListType;
		if (attributeName == kAttrSeparatorWidth) return kIntegerType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* splitView = dynamic_cast<CSplitView*> (view);
		if (!splitView)
			return false;
		if (attributeName == kAttrSeparatorWidth)
		{
			stringValue = UIAttributes::integerToString (static_cast<int32_t> (splitView->getSeparatorWidth ()));
			return true;
		}
		if (attributeName == kAttrOrientation)
		{
			stringValue = splitView->getStyle () == CSplitView::kHorizontal ? strHorizontal : strVertical;
			return true;
		}
		if (attributeName == kAttrResizeMethod)
		{
			switch (splitView->getResizeMethod ())
			{
				case CSplitView::kResizeFirstView:
				{
					stringValue = kFirst;
					return true;
				}
				case CSplitView::kResizeSecondView:
				{
					stringValue = kSecond;
					return true;
				}
				case CSplitView::kResizeLastView:
				{
					stringValue = kLast;
					return true;
				}
				case CSplitView::kResizeAllViews:
				{
					stringValue = kAll;
					return true;
				}
			}
		}
		return false;
	}
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const override
	{
		if (attributeName == kAttrOrientation)
		{
			return getStandardAttributeListValues (kAttrOrientation, values);
		}
		else if (attributeName == kAttrResizeMethod)
		{
			values.emplace_back (&kFirst);
			values.emplace_back (&kSecond);
			values.emplace_back (&kLast);
			values.emplace_back (&kAll);
			return true;
		}
		return false;
	}

};
CSplitViewCreator __gCSplitViewCreator;

//-----------------------------------------------------------------------------
class CShadowViewContainerCreator : public ViewCreatorAdapter
{
public:
	CShadowViewContainerCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCShadowViewContainer; }
	IdStringPtr getBaseViewName () const override { return kCViewContainer; }
	UTF8StringPtr getDisplayName () const override { return "Shadow View Container"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override
	{
		return new CShadowViewContainer (CRect (0, 0, 200, 200));
	}

	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* shadowView = dynamic_cast<CShadowViewContainer*> (view);
		if (!shadowView)
			return false;
		double d;
		if (attributes.getDoubleAttribute (kAttrShadowIntensity, d))
			shadowView->setShadowIntensity (static_cast<float> (d));
		if (attributes.getDoubleAttribute (kAttrShadowBlurSize, d))
			shadowView->setShadowBlurSize (d);
		CPoint p;
		if (attributes.getPointAttribute (kAttrShadowOffset, p))
			shadowView->setShadowOffset (p);
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrShadowIntensity);
		attributeNames.emplace_back (kAttrShadowOffset);
		attributeNames.emplace_back (kAttrShadowBlurSize);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrShadowIntensity) return kFloatType;
		if (attributeName == kAttrShadowOffset) return kPointType;
		if (attributeName == kAttrShadowBlurSize) return kFloatType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* shadowView = dynamic_cast<CShadowViewContainer*> (view);
		if (!shadowView)
			return false;
		if (attributeName == kAttrShadowIntensity)
		{
			stringValue = UIAttributes::doubleToString (shadowView->getShadowIntensity ());
			return true;
		}
		else if (attributeName == kAttrShadowBlurSize)
		{
			stringValue = UIAttributes::doubleToString (shadowView->getShadowBlurSize ());
			return true;
		}
		else if (attributeName == kAttrShadowOffset)
		{
			stringValue = UIAttributes::pointToString (shadowView->getShadowOffset ());
			return true;
		}
		return false;
	}
	bool getAttributeValueRange (const std::string& attributeName, double& minValue, double &maxValue) const override
	{
		if (attributeName == kAttrShadowBlurSize)
		{
			minValue = 0.8;
			maxValue = 20;
			return true;
		}
		else if (attributeName == kAttrShadowIntensity)
		{
			minValue = 0;
			maxValue = 1;
			return true;
		}
		return false;
	}
};
CShadowViewContainerCreator __gCShadowViewContainerCreator;

//-----------------------------------------------------------------------------
class CGradientViewCreator : public ViewCreatorAdapter
{
public:
	std::string kLinear = "linear";
	std::string kRadial = "radial";

	CGradientViewCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCGradientView; }
	IdStringPtr getBaseViewName () const override { return kCView; }
	UTF8StringPtr getDisplayName () const override { return "Gradient View"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override
	{
		CGradientView* gradientView = new CGradientView (CRect (0, 0, 100, 100));
		if (description)
		{
			std::list<const std::string*> gradients;
			description->collectGradientNames (gradients);
			if (!gradients.empty ())
			{
				gradientView->setGradient (description->getGradient (gradients.front ()->c_str ()));
			}
		}
		return gradientView;
	}
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
	{
		auto* gv = dynamic_cast<CGradientView*> (view);
		if (gv == nullptr)
			return false;
		CColor color;
		if (stringToColor (attributes.getAttributeValue (kAttrFrameColor), color, description))
			gv->setFrameColor (color);

		double d;
		if (attributes.getDoubleAttribute (kAttrGradientAngle, d))
			gv->setGradientAngle (d);
		if (attributes.getDoubleAttribute (kAttrRoundRectRadius, d))
			gv->setRoundRectRadius (d);
		if (attributes.getDoubleAttribute (kAttrFrameWidth, d))
			gv->setFrameWidth (d);

		bool b;
		if (attributes.getBooleanAttribute (kAttrDrawAntialiased, b))
			gv->setDrawAntialiased (b);

		const std::string* attr = attributes.getAttributeValue (kAttrGradientStyle);
		if (attr)
		{
			if (*attr == kRadial)
				gv->setGradientStyle(CGradientView::kRadialGradient);
			else
				gv->setGradientStyle(CGradientView::kLinearGradient);
		}
		CPoint p;
		if (attributes.getPointAttribute (kAttrRadialCenter, p))
			gv->setRadialCenter (p);
		if (attributes.getDoubleAttribute (kAttrRadialRadius, d))
			gv->setRadialRadius (d);
		
		attr = attributes.getAttributeValue (kAttrGradient);
		if (attr)
		{
			CGradient* gradient = description->getGradient (attr->c_str ());
			gv->setGradient (gradient);
		}
		else
		{ // support old version
			bool hasOldGradient = true;
			CColor startColor, endColor;
			if (!stringToColor (attributes.getAttributeValue (kAttrGradientStartColor), startColor, description))
				hasOldGradient = false;
			if (hasOldGradient && !stringToColor (attributes.getAttributeValue (kAttrGradientEndColor), endColor, description))
				hasOldGradient = false;
			double startOffset = 0.0, endOffset = 1.0;
			if (hasOldGradient && !attributes.getDoubleAttribute (kAttrGradientStartColorOffset, startOffset))
				hasOldGradient = false;
			if (hasOldGradient && !attributes.getDoubleAttribute (kAttrGradientEndColorOffset, endOffset))
				hasOldGradient = false;
			if (hasOldGradient)
			{
				SharedPointer<CGradient> gradient = owned (CGradient::create (startOffset, 1. - endOffset, startColor, endColor));
				gv->setGradient (gradient);
				addGradientToUIDescription (description, gradient, "GradientView");
			}
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrGradientStyle);
		attributeNames.emplace_back (kAttrGradient);
		attributeNames.emplace_back (kAttrGradientAngle);
		attributeNames.emplace_back (kAttrRadialCenter);
		attributeNames.emplace_back (kAttrRadialRadius);
		attributeNames.emplace_back (kAttrFrameColor);
		attributeNames.emplace_back (kAttrRoundRectRadius);
		attributeNames.emplace_back (kAttrFrameWidth);
		attributeNames.emplace_back (kAttrDrawAntialiased);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrGradientStyle) return kListType;
		if (attributeName == kAttrGradient) return kGradientType;
		if (attributeName == kAttrFrameColor) return kColorType;
		if (attributeName == kAttrGradientAngle) return kFloatType;
		if (attributeName == kAttrRoundRectRadius) return kFloatType;
		if (attributeName == kAttrFrameWidth) return kFloatType;
		if (attributeName == kAttrDrawAntialiased) return kBooleanType;
		if (attributeName == kAttrRadialCenter) return kPointType;
		if (attributeName == kAttrRadialRadius) return kFloatType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override
	{
		auto* gv = dynamic_cast<CGradientView*> (view);
		if (gv == nullptr)
			return false;
		if (attributeName == kAttrFrameColor)
		{
			colorToString (gv->getFrameColor (), stringValue, desc);
			return true;
		}
		if (attributeName == kAttrGradientAngle)
		{
			stringValue = UIAttributes::doubleToString (gv->getGradientAngle ());
			return true;
		}
		if (attributeName == kAttrRoundRectRadius)
		{
			stringValue = UIAttributes::doubleToString (gv->getRoundRectRadius ());
			return true;
		}
		if (attributeName == kAttrFrameWidth)
		{
			stringValue = UIAttributes::doubleToString (gv->getFrameWidth ());
			return true;
		}
		if (attributeName == kAttrDrawAntialiased)
		{
			stringValue = gv->getDrawAntialised () ? strTrue : strFalse;
			return true;
		}
		if (attributeName == kAttrGradientStyle)
		{
			stringValue = gv->getGradientStyle () == CGradientView::kLinearGradient ? kLinear : kRadial;
			return true;
		}
		if (attributeName == kAttrRadialRadius)
		{
			stringValue = UIAttributes::doubleToString (gv->getRadialRadius ());
			return true;
		}
		if (attributeName == kAttrRadialCenter)
		{
			stringValue = UIAttributes::pointToString (gv->getRadialCenter ());
			return true;
		}
		if (attributeName == kAttrGradient)
		{
			CGradient* gradient = gv->getGradient ();
			UTF8StringPtr gradientName = gradient ? desc->lookupGradientName (gradient) : nullptr;
			stringValue = gradientName ? gradientName : "";
			return true;
		}
		return false;
	}
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const override
	{
		if (attributeName == kAttrGradientStyle)
		{
			values.emplace_back (&kLinear);
			values.emplace_back (&kRadial);
			return true;
		}
		return false;
	}
	bool getAttributeValueRange (const std::string& attributeName, double& minValue, double &maxValue) const override
	{
		if (attributeName == kAttrGradientAngle)
		{
			minValue = 0.;
			maxValue = 360.;
			return true;
		}
		return false;
	}

};
CGradientViewCreator __gCGradientViewCreator;

//-----------------------------------------------------------------------------
class CXYPadCreator : public ViewCreatorAdapter
{
public:
	CXYPadCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return kCXYPad; }
	IdStringPtr getBaseViewName () const override { return kCParamDisplay; }
	UTF8StringPtr getDisplayName () const override { return "XY Pad"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override { return new CXYPad (CRect (0, 0, 60, 60)); }
};
CXYPadCreator __gCXYPadCreator;

//-----------------------------------------------------------------------------
static bool getStandardAttributeListValues (const std::string& attributeName, std::list<const std::string*>& values)
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
}} // VSTGUI

/**
@endcond ignore
*/
