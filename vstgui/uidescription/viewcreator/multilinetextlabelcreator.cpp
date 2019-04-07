// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "multilinetextlabelcreator.h"

#include "../../lib/controls/ctextlabel.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
CMultiLineTextLabelCreator::CMultiLineTextLabelCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr CMultiLineTextLabelCreator::getViewName () const
{
	return kCMultiLineTextLabel;
}

//------------------------------------------------------------------------
IdStringPtr CMultiLineTextLabelCreator::getBaseViewName () const
{
	return kCTextLabel;
}

//------------------------------------------------------------------------
UTF8StringPtr CMultiLineTextLabelCreator::getDisplayName () const
{
	return "Multiline Label";
}

//------------------------------------------------------------------------
CView* CMultiLineTextLabelCreator::create (const UIAttributes& attributes,
                                           const IUIDescription* description) const
{
	return new CMultiLineTextLabel (CRect (0, 0, 100, 20));
}

//------------------------------------------------------------------------
bool CMultiLineTextLabelCreator::apply (CView* view, const UIAttributes& attributes,
                                        const IUIDescription* description) const
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

//------------------------------------------------------------------------
bool CMultiLineTextLabelCreator::getAttributeNames (std::list<std::string>& attributeNames) const
{
	attributeNames.emplace_back (kAttrLineLayout);
	attributeNames.emplace_back (kAttrAutoHeight);
	return true;
}

//------------------------------------------------------------------------
auto CMultiLineTextLabelCreator::getAttributeType (const std::string& attributeName) const
    -> AttrType
{
	if (attributeName == kAttrLineLayout)
		return kListType;
	if (attributeName == kAttrAutoHeight)
		return kBooleanType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool CMultiLineTextLabelCreator::getAttributeValue (CView* view, const std::string& attributeName,
                                                    std::string& stringValue,
                                                    const IUIDescription* desc) const
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

//------------------------------------------------------------------------
bool CMultiLineTextLabelCreator::getPossibleListValues (const std::string& attributeName,
                                                        std::list<const std::string*>& values) const
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

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
