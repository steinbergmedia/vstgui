// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "viewcontainercreator.h"

#include "../../lib/ccolor.h"
#include "../../lib/cviewcontainer.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"
#include <array>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
auto ViewContainerCreator::backgroundColorDrawStyleStrings () -> BackgroundColorDrawStyleStrings&
{
	static BackgroundColorDrawStyleStrings strings = {"stroked", "filled", "filled and stroked"};
	return strings;
}

//------------------------------------------------------------------------
ViewContainerCreator::ViewContainerCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr ViewContainerCreator::getViewName () const
{
	return kCViewContainer;
}

//------------------------------------------------------------------------
IdStringPtr ViewContainerCreator::getBaseViewName () const
{
	return kCView;
}

//------------------------------------------------------------------------
UTF8StringPtr ViewContainerCreator::getDisplayName () const
{
	return "View Container";
}

//------------------------------------------------------------------------
CView* ViewContainerCreator::create (const UIAttributes& attributes,
                                     const IUIDescription* description) const
{
	return new CViewContainer (CRect (0, 0, 100, 100));
}

//------------------------------------------------------------------------
bool ViewContainerCreator::apply (CView* view, const UIAttributes& attributes,
                                  const IUIDescription* description) const
{
	CViewContainer* viewContainer = view->asViewContainer ();
	if (viewContainer == nullptr)
		return false;
	CColor backColor;
	if (stringToColor (attributes.getAttributeValue (kAttrBackgroundColor), backColor, description))
		viewContainer->setBackgroundColor (backColor);
	const auto* attr = attributes.getAttributeValue (kAttrBackgroundColorDrawStyle);
	if (attr)
	{
		for (auto index = 0u; index <= kDrawFilledAndStroked; ++index)
		{
			if (*attr == backgroundColorDrawStyleStrings ()[index])
			{
				viewContainer->setBackgroundColorDrawStyle (static_cast<CDrawStyle> (index));
				break;
			}
		}
	}
	return true;
}

//------------------------------------------------------------------------
bool ViewContainerCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrBackgroundColor);
	attributeNames.emplace_back (kAttrBackgroundColorDrawStyle);
	return true;
}

//------------------------------------------------------------------------
auto ViewContainerCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrBackgroundColor)
		return kColorType;
	if (attributeName == kAttrBackgroundColorDrawStyle)
		return kListType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool ViewContainerCreator::getAttributeValue (CView* view, const string& attributeName,
                                              string& stringValue, const IUIDescription* desc) const
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
		stringValue = backgroundColorDrawStyleStrings ()[vc->getBackgroundColorDrawStyle ()];
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool ViewContainerCreator::getPossibleListValues (const string& attributeName,
                                                  ConstStringPtrList& values) const
{
	if (attributeName == kAttrBackgroundColorDrawStyle)
	{
		for (auto& str : backgroundColorDrawStyleStrings ())
			values.emplace_back (&str);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
