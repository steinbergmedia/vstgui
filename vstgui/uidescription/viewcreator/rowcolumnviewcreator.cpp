// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "rowcolumnviewcreator.h"

#include "../../lib/crowcolumnview.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"
#include <array>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
auto RowColumnViewCreator::layoutStrings () -> LayoutStrings&
{
	static LayoutStrings strings = {"left-top", "center", "right-bottom", "stretch"};
	return strings;
}

//------------------------------------------------------------------------
RowColumnViewCreator::RowColumnViewCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr RowColumnViewCreator::getViewName () const
{
	return kCRowColumnView;
}

//------------------------------------------------------------------------
IdStringPtr RowColumnViewCreator::getBaseViewName () const
{
	return kCViewContainer;
}

//------------------------------------------------------------------------
UTF8StringPtr RowColumnViewCreator::getDisplayName () const
{
	return "Row Column View Container";
}

//------------------------------------------------------------------------
CView* RowColumnViewCreator::create (const UIAttributes& attributes,
                                     const IUIDescription* description) const
{
	return new CRowColumnView (CRect (0, 0, 100, 100));
}

//------------------------------------------------------------------------
bool RowColumnViewCreator::apply (CView* view, const UIAttributes& attributes,
                                  const IUIDescription* description) const
{
	auto* rcv = dynamic_cast<CRowColumnView*> (view);
	if (rcv == nullptr)
		return false;
	const auto* attr = attributes.getAttributeValue (kAttrRowStyle);
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
		for (auto index = 0u; index <= CRowColumnView::kStretchEqualy; ++index)
		{
			if (*attr == layoutStrings ()[index])
			{
				rcv->setLayoutStyle (static_cast<CRowColumnView::LayoutStyle> (index));
				break;
			}
		}
	}
	attr = attributes.getAttributeValue (kAttrViewResizeAnimationTime);
	if (attr)
	{
		uint32_t time = (uint32_t)strtol (attr->c_str (), nullptr, 10);
		rcv->setViewResizeAnimationTime (time);
	}
	return true;
}

//------------------------------------------------------------------------
bool RowColumnViewCreator::getAttributeNames (StringList& attributeNames) const
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

//------------------------------------------------------------------------
auto RowColumnViewCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrRowStyle)
		return kBooleanType;
	if (attributeName == kAttrSpacing)
		return kIntegerType;
	if (attributeName == kAttrMargin)
		return kRectType;
	if (attributeName == kAttrEqualSizeLayout)
		return kListType;
	if (attributeName == kAttrHideClippedSubviews)
		return kBooleanType;
	if (attributeName == kAttrAnimateViewResizing)
		return kBooleanType;
	if (attributeName == kAttrViewResizeAnimationTime)
		return kIntegerType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool RowColumnViewCreator::getAttributeValue (CView* view, const string& attributeName,
                                              string& stringValue, const IUIDescription* desc) const
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
		stringValue = UIAttributes::integerToString (
		    static_cast<int32_t> (rcv->getViewResizeAnimationTime ()));
		return true;
	}
	if (attributeName == kAttrMargin)
	{
		stringValue = UIAttributes::rectToString (rcv->getMargin ());
		return true;
	}
	if (attributeName == kAttrEqualSizeLayout)
	{
		stringValue = layoutStrings ()[rcv->getLayoutStyle ()];
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool RowColumnViewCreator::getPossibleListValues (const string& attributeName,
                                                  ConstStringPtrList& values) const
{
	if (attributeName == kAttrEqualSizeLayout)
	{
		for (auto& str : layoutStrings ())
			values.emplace_back (&str);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
