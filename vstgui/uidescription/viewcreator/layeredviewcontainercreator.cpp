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
LayeredViewContainerCreator::LayeredViewContainerCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr LayeredViewContainerCreator::getViewName () const
{
	return kCLayeredViewContainer;
}

//------------------------------------------------------------------------
IdStringPtr LayeredViewContainerCreator::getBaseViewName () const
{
	return kCViewContainer;
}

//------------------------------------------------------------------------
UTF8StringPtr LayeredViewContainerCreator::getDisplayName () const
{
	return "Layered View Container";
}

//------------------------------------------------------------------------
CView* LayeredViewContainerCreator::create (const UIAttributes& attributes,
                                            const IUIDescription* description) const
{
	return new CLayeredViewContainer (CRect (0, 0, 100, 100));
}

//------------------------------------------------------------------------
bool LayeredViewContainerCreator::apply (CView* view, const UIAttributes& attributes,
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
bool LayeredViewContainerCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrZIndex);
	return true;
}

//------------------------------------------------------------------------
auto LayeredViewContainerCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrZIndex)
		return kIntegerType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool LayeredViewContainerCreator::getAttributeValue (CView* view, const string& attributeName,
                                                     string& stringValue,
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
