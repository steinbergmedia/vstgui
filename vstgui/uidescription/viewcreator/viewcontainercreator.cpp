// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "viewcontainercreator.h"

#include "../../lib/cviewcontainer.h"
#include "../../lib/ccolor.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
CViewContainerCreator::CViewContainerCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr CViewContainerCreator::getViewName () const
{
	return kCViewContainer;
}

//------------------------------------------------------------------------
IdStringPtr CViewContainerCreator::getBaseViewName () const
{
	return kCView;
}

//------------------------------------------------------------------------
UTF8StringPtr CViewContainerCreator::getDisplayName () const
{
	return "View Container";
}

//------------------------------------------------------------------------
CView* CViewContainerCreator::create (const UIAttributes& attributes,
                                      const IUIDescription* description) const
{
	return new CViewContainer (CRect (0, 0, 100, 100));
}

//------------------------------------------------------------------------
bool CViewContainerCreator::apply (CView* view, const UIAttributes& attributes,
                                   const IUIDescription* description) const
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

//------------------------------------------------------------------------
bool CViewContainerCreator::getAttributeNames (std::list<std::string>& attributeNames) const
{
	attributeNames.emplace_back (kAttrBackgroundColor);
	attributeNames.emplace_back (kAttrBackgroundColorDrawStyle);
	return true;
}

//------------------------------------------------------------------------
auto CViewContainerCreator::getAttributeType (const std::string& attributeName) const -> AttrType
{
	if (attributeName == kAttrBackgroundColor)
		return kColorType;
	if (attributeName == kAttrBackgroundColorDrawStyle)
		return kListType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool CViewContainerCreator::getAttributeValue (CView* view, const std::string& attributeName,
                                               std::string& stringValue,
                                               const IUIDescription* desc) const
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

//------------------------------------------------------------------------
bool CViewContainerCreator::getPossibleListValues (const std::string& attributeName,
                                                   std::list<const std::string*>& values) const
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

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
