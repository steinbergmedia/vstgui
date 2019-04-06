// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "layeredviewcontainercreator.h"

#include "../../lib/clayeredviewcontainer.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//-----------------------------------------------------------------------------
CLayeredViewContainerCreator::CLayeredViewContainerCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr CLayeredViewContainerCreator::getViewName () const
{
	return kCLayeredViewContainer;
}

//------------------------------------------------------------------------
IdStringPtr CLayeredViewContainerCreator::getBaseViewName () const
{
	return kCViewContainer;
}

//------------------------------------------------------------------------
UTF8StringPtr CLayeredViewContainerCreator::getDisplayName () const
{
	return "Layered View Container";
}

//------------------------------------------------------------------------
CView* CLayeredViewContainerCreator::create (const UIAttributes& attributes,
                                             const IUIDescription* description) const
{
	return new CLayeredViewContainer (CRect (0, 0, 100, 100));
}

//------------------------------------------------------------------------
bool CLayeredViewContainerCreator::apply (CView* view, const UIAttributes& attributes,
                                          const IUIDescription* description) const
{
	auto* lvc = dynamic_cast<CLayeredViewContainer*> (view);
	if (lvc == nullptr)
		return false;
	int32_t zIndex;
	if (attributes.getIntegerAttribute (kAttrZIndex, zIndex))
		lvc->setZIndex (static_cast<uint32_t> (zIndex));
	return true;
}

//------------------------------------------------------------------------
bool CLayeredViewContainerCreator::getAttributeNames (std::list<std::string>& attributeNames) const
{
	attributeNames.emplace_back (kAttrZIndex);
	return true;
}

//------------------------------------------------------------------------
auto CLayeredViewContainerCreator::getAttributeType (const std::string& attributeName) const
    -> AttrType
{
	if (attributeName == kAttrZIndex)
		return kIntegerType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool CLayeredViewContainerCreator::getAttributeValue (CView* view, const std::string& attributeName,
                                                      std::string& stringValue,
                                                      const IUIDescription* desc) const
{
	auto* lvc = dynamic_cast<CLayeredViewContainer*> (view);
	if (lvc == nullptr)
		return false;
	if (attributeName == kAttrZIndex)
	{
		stringValue = UIAttributes::integerToString (static_cast<int32_t> (lvc->getZIndex ()));
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
