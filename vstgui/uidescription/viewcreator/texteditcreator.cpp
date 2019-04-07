// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "texteditcreator.h"

#include "../../lib/controls/ctextedit.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
TextEditCreator::TextEditCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr TextEditCreator::getViewName () const
{
	return kCTextEdit;
}

//------------------------------------------------------------------------
IdStringPtr TextEditCreator::getBaseViewName () const
{
	return kCTextLabel;
}

//------------------------------------------------------------------------
UTF8StringPtr TextEditCreator::getDisplayName () const
{
	return "Text Edit";
}

//------------------------------------------------------------------------
CView* TextEditCreator::create (const UIAttributes& attributes,
                                const IUIDescription* description) const
{
	return new CTextEdit (CRect (0, 0, 100, 20), nullptr, -1);
}

//------------------------------------------------------------------------
bool TextEditCreator::apply (CView* view, const UIAttributes& attributes,
                             const IUIDescription* description) const
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
	applyStyleMask (attributes.getAttributeValue (kAttrStyleDoubleClick),
	                CTextEdit::kDoubleClickStyle, style);
	label->setStyle (style);

	if (auto placeholder = attributes.getAttributeValue (kAttrPlaceholderTitle))
		label->setPlaceholderString (placeholder->c_str ());

	return true;
}

//------------------------------------------------------------------------
bool TextEditCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrSecureStyle);
	attributeNames.emplace_back (kAttrImmediateTextChange);
	attributeNames.emplace_back (kAttrStyleDoubleClick);
	attributeNames.emplace_back (kAttrPlaceholderTitle);
	return true;
}

//------------------------------------------------------------------------
auto TextEditCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrSecureStyle)
		return kBooleanType;
	if (attributeName == kAttrImmediateTextChange)
		return kBooleanType;
	if (attributeName == kAttrStyleDoubleClick)
		return kBooleanType;
	if (attributeName == kAttrPlaceholderTitle)
		return kStringType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool TextEditCreator::getAttributeValue (CView* view, const string& attributeName,
                                         string& stringValue, const IUIDescription* desc) const
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

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
