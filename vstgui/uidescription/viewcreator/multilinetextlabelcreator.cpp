// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "multilinetextlabelcreator.h"

#include "../../lib/controls/ctextlabel.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"
#include <array>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
auto MultiLineTextLabelCreator::lineLayoutStrings () -> LineLayoutStrings&
{
	static LineLayoutStrings strings = {"clip", "truncate", "wrap"};
	return strings;
}

//------------------------------------------------------------------------
MultiLineTextLabelCreator::MultiLineTextLabelCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr MultiLineTextLabelCreator::getViewName () const
{
	return kCMultiLineTextLabel;
}

//------------------------------------------------------------------------
IdStringPtr MultiLineTextLabelCreator::getBaseViewName () const
{
	return kCTextLabel;
}

//------------------------------------------------------------------------
UTF8StringPtr MultiLineTextLabelCreator::getDisplayName () const
{
	return "Multiline Label";
}

//------------------------------------------------------------------------
CView* MultiLineTextLabelCreator::create (const UIAttributes& attributes,
                                          const IUIDescription* description) const
{
	return new CMultiLineTextLabel (CRect (0, 0, 100, 20));
}

//------------------------------------------------------------------------
bool MultiLineTextLabelCreator::apply (CView* view, const UIAttributes& attributes,
                                       const IUIDescription* description) const
{
	using LineLayout = CMultiLineTextLabel::LineLayout;

	auto label = dynamic_cast<CMultiLineTextLabel*> (view);
	if (!label)
		return false;

	auto attr = attributes.getAttributeValue (kAttrLineLayout);
	if (attr)
	{
		for (auto index = 0u; index <= static_cast<size_t> (LineLayout::wrap); ++index)
		{
			if (*attr == lineLayoutStrings ()[index])
				label->setLineLayout (static_cast<LineLayout> (index));
		}
	}
	bool autoHeight;
	if (attributes.getBooleanAttribute (kAttrAutoHeight, autoHeight))
		label->setAutoHeight (autoHeight);
	bool verticalCentered;
	if (attributes.getBooleanAttribute (kAttrVerticalCentered, verticalCentered))
		label->setVerticalCentered (verticalCentered);

	return true;
}

//------------------------------------------------------------------------
bool MultiLineTextLabelCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrLineLayout);
	attributeNames.emplace_back (kAttrAutoHeight);
	attributeNames.emplace_back (kAttrVerticalCentered);
	return true;
}

//------------------------------------------------------------------------
auto MultiLineTextLabelCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrLineLayout)
		return kListType;
	if (attributeName == kAttrAutoHeight)
		return kBooleanType;
	if (attributeName == kAttrVerticalCentered)
		return kBooleanType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool MultiLineTextLabelCreator::getAttributeValue (CView* view, const string& attributeName,
                                                   string& stringValue,
                                                   const IUIDescription* desc) const
{
	auto label = dynamic_cast<CMultiLineTextLabel*> (view);
	if (!label)
		return false;
	if (attributeName == kAttrLineLayout)
	{
		stringValue = lineLayoutStrings ()[static_cast<size_t> (label->getLineLayout ())];
		return true;
	}
	else if (attributeName == kAttrAutoHeight)
	{
		stringValue = label->getAutoHeight () ? strTrue : strFalse;
		return true;
	}
	else if (attributeName == kAttrVerticalCentered)
	{
		stringValue = label->getVerticalCentered () ? strTrue : strFalse;
		return true;
	}

	return false;
}

//------------------------------------------------------------------------
bool MultiLineTextLabelCreator::getPossibleListValues (const string& attributeName,
                                                       ConstStringPtrList& values) const
{
	if (attributeName == kAttrLineLayout)
	{
		for (auto& str : lineLayoutStrings ())
			values.emplace_back (&str);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
