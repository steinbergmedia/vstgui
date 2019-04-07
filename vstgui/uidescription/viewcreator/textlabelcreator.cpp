// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "textlabelcreator.h"

#include "../../lib/controls/ctextlabel.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
TextLabelCreator::TextLabelCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr TextLabelCreator::getViewName () const
{
	return kCTextLabel;
}

//------------------------------------------------------------------------
IdStringPtr TextLabelCreator::getBaseViewName () const
{
	return kCParamDisplay;
}

//------------------------------------------------------------------------
UTF8StringPtr TextLabelCreator::getDisplayName () const
{
	return "Label";
}

//------------------------------------------------------------------------
CView* TextLabelCreator::create (const UIAttributes& attributes,
                                 const IUIDescription* description) const
{
	return new CTextLabel (CRect (0, 0, 100, 20));
}

//------------------------------------------------------------------------
bool TextLabelCreator::apply (CView* view, const UIAttributes& attributes,
                              const IUIDescription* description) const
{
	auto* label = dynamic_cast<CTextLabel*> (view);
	if (!label)
		return false;

	const auto* attr = attributes.getAttributeValue (kAttrTitle);
	if (attr)
	{
		auto index = attr->find ("\\n");
		if (index != string::npos)
		{
			auto str = *attr;
			while (index != string::npos)
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

//------------------------------------------------------------------------
bool TextLabelCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrTitle);
	attributeNames.emplace_back (kAttrTruncateMode);
	return true;
}

//------------------------------------------------------------------------
auto TextLabelCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrTitle)
		return kStringType;
	if (attributeName == kAttrTruncateMode)
		return kListType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool TextLabelCreator::getAttributeValue (CView* view, const string& attributeName,
                                          string& stringValue, const IUIDescription* desc) const
{
	auto* label = dynamic_cast<CTextLabel*> (view);
	if (!label)
		return false;
	if (attributeName == kAttrTitle)
	{
		stringValue = label->getText ().getString ();
		auto index = stringValue.find ("\n");
		while (index != string::npos)
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

//------------------------------------------------------------------------
bool TextLabelCreator::getPossibleListValues (const string& attributeName,
                                              ConstStringPtrList& values) const
{
	if (attributeName == kAttrTruncateMode)
	{
		return getStandardAttributeListValues (kAttrTruncateMode, values);
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
