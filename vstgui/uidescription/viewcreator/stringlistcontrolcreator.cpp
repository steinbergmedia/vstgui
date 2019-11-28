// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "stringlistcontrolcreator.h"

#include "../../lib/controls/cstringlist.h"
#include "../../lib/ccolor.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
StringListControlCreator::StringListControlCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr StringListControlCreator::getViewName () const
{
	return kCStringListControl;
}

//------------------------------------------------------------------------
IdStringPtr StringListControlCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr StringListControlCreator::getDisplayName () const
{
	return "String List Control";
}

//------------------------------------------------------------------------
CView* StringListControlCreator::create (const UIAttributes& attributes,
                                         const IUIDescription* description) const
{
	auto control = new CListControl (CRect (0, 0, 100, 200));
	auto drawer = makeOwned<StringListControlDrawer> ();
	control->setDrawer (drawer);
	auto configurator = makeOwned<StaticListControlConfigurator> (12.);
	control->setConfigurator (configurator);
	return control;
}

//------------------------------------------------------------------------
bool StringListControlCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrStyleHover);
	attributeNames.emplace_back (kAttrFont);
	attributeNames.emplace_back (kAttrFontColor);
	attributeNames.emplace_back (kAttrSelectedFontColor);
	attributeNames.emplace_back (kAttrBackColor);
	attributeNames.emplace_back (kAttrSelectedBackColor);
	attributeNames.emplace_back (kAttrHoverColor);
	attributeNames.emplace_back (kAttrLineColor);
	attributeNames.emplace_back (kAttrLineWidth);
	attributeNames.emplace_back (kAttrTextInset);
	attributeNames.emplace_back (kAttrRowHeight);
	attributeNames.emplace_back (kAttrTextAlignment);
	return true;
}

//------------------------------------------------------------------------
auto StringListControlCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrFont)
		return kFontType;
	if (attributeName == kAttrFontColor)
		return kColorType;
	if (attributeName == kAttrSelectedFontColor)
		return kColorType;
	if (attributeName == kAttrBackColor)
		return kColorType;
	if (attributeName == kAttrSelectedBackColor)
		return kColorType;
	if (attributeName == kAttrHoverColor)
		return kColorType;
	if (attributeName == kAttrLineColor)
		return kColorType;
	if (attributeName == kAttrLineWidth)
		return kFloatType;
	if (attributeName == kAttrTextInset)
		return kFloatType;
	if (attributeName == kAttrRowHeight)
		return kFloatType;
	if (attributeName == kAttrStyleHover)
		return kBooleanType;
	if (attributeName == kAttrTextAlignment)
		return kStringType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool StringListControlCreator::apply (CView* view, const UIAttributes& attributes,
                                      const IUIDescription* description) const
{
	auto control = dynamic_cast<CListControl*> (view);
	if (!control)
		return false;
	auto drawer = dynamic_cast<StringListControlDrawer*> (control->getDrawer ());
	auto configurator = dynamic_cast<StaticListControlConfigurator*> (control->getConfigurator ());
	if (!drawer || !configurator)
		return false;

	if (const auto* fontAttr = attributes.getAttributeValue (kAttrFont))
	{
		if (auto font = description->getFont (fontAttr->data ()))
			drawer->setFont (font);
	}
	if (const auto* textAlignmentAttr = attributes.getAttributeValue (kAttrTextAlignment))
	{
		CHoriTxtAlign align = kCenterText;
		if (*textAlignmentAttr == strLeft)
			align = kLeftText;
		else if (*textAlignmentAttr == strRight)
			align = kRightText;
		drawer->setTextAlign (align);
	}
	CColor color;
	if (stringToColor (attributes.getAttributeValue (kAttrFontColor), color, description))
		drawer->setFontColor (color);
	if (stringToColor (attributes.getAttributeValue (kAttrSelectedFontColor), color, description))
		drawer->setSelectedFontColor (color);
	if (stringToColor (attributes.getAttributeValue (kAttrBackColor), color, description))
		drawer->setBackColor (color);
	if (stringToColor (attributes.getAttributeValue (kAttrSelectedBackColor), color, description))
		drawer->setSelectedBackColor (color);
	if (stringToColor (attributes.getAttributeValue (kAttrHoverColor), color, description))
		drawer->setHoverColor (color);
	if (stringToColor (attributes.getAttributeValue (kAttrLineColor), color, description))
		drawer->setLineColor (color);
	double d;
	if (attributes.getDoubleAttribute (kAttrLineWidth, d))
		drawer->setLineWidth (d);
	if (attributes.getDoubleAttribute (kAttrTextInset, d))
		drawer->setTextInset (d);
	if (attributes.getDoubleAttribute (kAttrRowHeight, d))
		configurator->setRowHeight (d);
	bool b;
	if (attributes.getBooleanAttribute (kAttrStyleHover, b))
		configurator->setFlags (CListControlRowDesc::Selectable |
		                        (b ? CListControlRowDesc::Hoverable : 0));

	control->invalid ();
	control->recalculateLayout ();

	return true;
}

//------------------------------------------------------------------------
bool StringListControlCreator::getAttributeValue (CView* view, const string& attributeName,
                                                  string& stringValue,
                                                  const IUIDescription* desc) const
{
	auto control = dynamic_cast<CListControl*> (view);
	if (!control)
		return false;
	auto drawer = dynamic_cast<StringListControlDrawer*> (control->getDrawer ());
	auto configurator = dynamic_cast<StaticListControlConfigurator*> (control->getConfigurator ());
	if (!drawer || !configurator)
		return false;

	if (attributeName == kAttrFont)
	{
		UTF8StringPtr fontName = desc->lookupFontName (drawer->getFont ());
		if (fontName)
		{
			stringValue = fontName;
			return true;
		}
		return false;
	}
	else if (attributeName == kAttrFontColor)
	{
		colorToString (drawer->getFontColor (), stringValue, desc);
		return true;
	}
	else if (attributeName == kAttrSelectedFontColor)
	{
		colorToString (drawer->getSelectedFontColor (), stringValue, desc);
		return true;
	}
	else if (attributeName == kAttrBackColor)
	{
		colorToString (drawer->getBackColor (), stringValue, desc);
		return true;
	}
	else if (attributeName == kAttrSelectedBackColor)
	{
		colorToString (drawer->getSelectedBackColor (), stringValue, desc);
		return true;
	}
	else if (attributeName == kAttrHoverColor)
	{
		colorToString (drawer->getHoverColor (), stringValue, desc);
		return true;
	}
	else if (attributeName == kAttrLineColor)
	{
		colorToString (drawer->getLineColor (), stringValue, desc);
		return true;
	}
	else if (attributeName == kAttrLineWidth)
	{
		stringValue = UIAttributes::doubleToString (drawer->getLineWidth ());
		return true;
	}
	else if (attributeName == kAttrTextInset)
	{
		stringValue = UIAttributes::doubleToString (drawer->getTextInset ());
		return true;
	}
	else if (attributeName == kAttrRowHeight)
	{
		stringValue = UIAttributes::doubleToString (configurator->getRowHeight ());
		return true;
	}
	else if (attributeName == kAttrStyleHover)
	{
		stringValue =
		    UIAttributes::boolToString (configurator->getFlags () & CListControlRowDesc::Hoverable);
		return true;
	}
	else if (attributeName == kAttrTextAlignment)
	{
		CHoriTxtAlign align = drawer->getTextAlign ();
		switch (align)
		{
			case kLeftText: stringValue = strLeft; break;
			case kRightText: stringValue = strRight; break;
			case kCenterText: stringValue = strCenter; break;
		}
		return true;
	}

	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
