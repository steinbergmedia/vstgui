// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "optionmenucreator.h"

#include "../../lib/controls/coptionmenu.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
COptionMenuCreator::COptionMenuCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr COptionMenuCreator::getViewName () const
{
	return kCOptionMenu;
}

//------------------------------------------------------------------------
IdStringPtr COptionMenuCreator::getBaseViewName () const
{
	return kCParamDisplay;
}

//------------------------------------------------------------------------
UTF8StringPtr COptionMenuCreator::getDisplayName () const
{
	return "Option Menu";
}

//------------------------------------------------------------------------
CView* COptionMenuCreator::create (const UIAttributes& attributes,
                                   const IUIDescription* description) const
{
	return new COptionMenu (CRect (0, 0, 100, 20), nullptr, -1);
}

//------------------------------------------------------------------------
bool COptionMenuCreator::apply (CView* view, const UIAttributes& attributes,
                                const IUIDescription* description) const
{
	auto* menu = dynamic_cast<COptionMenu*> (view);
	if (!menu)
		return false;

	int32_t style = menu->getStyle ();
	applyStyleMask (attributes.getAttributeValue (kAttrMenuPopupStyle), COptionMenu::kPopupStyle,
	                style);
	applyStyleMask (attributes.getAttributeValue (kAttrMenuCheckStyle), COptionMenu::kCheckStyle,
	                style);
	menu->setStyle (style);

	return true;
}

//------------------------------------------------------------------------
bool COptionMenuCreator::getAttributeNames (std::list<std::string>& attributeNames) const
{
	attributeNames.emplace_back (kAttrMenuPopupStyle);
	attributeNames.emplace_back (kAttrMenuCheckStyle);
	return true;
}

//------------------------------------------------------------------------
auto COptionMenuCreator::getAttributeType (const std::string& attributeName) const -> AttrType
{
	if (attributeName == kAttrMenuPopupStyle)
		return kBooleanType;
	if (attributeName == kAttrMenuCheckStyle)
		return kBooleanType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool COptionMenuCreator::getAttributeValue (CView* view, const std::string& attributeName,
                                            std::string& stringValue,
                                            const IUIDescription* desc) const
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

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
