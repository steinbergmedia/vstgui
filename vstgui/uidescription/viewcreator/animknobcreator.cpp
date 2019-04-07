// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "animknobcreator.h"
#include "multibitmapcontrolcreator.h"

#include "../../lib/controls/cknob.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
CAnimKnobCreator::CAnimKnobCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr CAnimKnobCreator::getViewName () const
{
	return kCAnimKnob;
}

//------------------------------------------------------------------------
IdStringPtr CAnimKnobCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr CAnimKnobCreator::getDisplayName () const
{
	return "Animation Knob";
}

//------------------------------------------------------------------------
CView* CAnimKnobCreator::create (const UIAttributes& attributes,
                                 const IUIDescription* description) const
{
	return new CAnimKnob (CRect (0, 0, 0, 0), nullptr, -1, nullptr);
}

//------------------------------------------------------------------------
bool CAnimKnobCreator::apply (CView* view, const UIAttributes& attributes,
                              const IUIDescription* description) const
{
	auto* animKnob = dynamic_cast<CAnimKnob*> (view);
	if (!animKnob)
		return false;

	bool b;
	if (attributes.getBooleanAttribute (kAttrInverseBitmap, b))
	{
		animKnob->setInverseBitmap (b);
	}
	IMultiBitmapControlCreator::apply (view, attributes, description);
	return CKnobBaseCreator::apply (view, attributes, description);
}

//------------------------------------------------------------------------
bool CAnimKnobCreator::getAttributeNames (std::list<std::string>& attributeNames) const
{
	attributeNames.emplace_back (kAttrInverseBitmap);
	IMultiBitmapControlCreator::getAttributeNames (attributeNames);
	return CKnobBaseCreator::getAttributeNames (attributeNames);
}

//------------------------------------------------------------------------
auto CAnimKnobCreator::getAttributeType (const std::string& attributeName) const -> AttrType
{
	if (attributeName == kAttrInverseBitmap)
		return kBooleanType;
	auto res = CKnobBaseCreator::getAttributeType (attributeName);
	if (res != kUnknownType)
		return res;
	return IMultiBitmapControlCreator::getAttributeType (attributeName);
}

//------------------------------------------------------------------------
bool CAnimKnobCreator::getAttributeValue (CView* view, const std::string& attributeName,
                                          std::string& stringValue,
                                          const IUIDescription* desc) const
{
	auto* animKnob = dynamic_cast<CAnimKnob*> (view);
	if (!animKnob)
		return false;

	if (attributeName == kAttrInverseBitmap)
	{
		stringValue = animKnob->getInverseBitmap () ? strTrue : strFalse;
		return true;
	}
	if (CKnobBaseCreator::getAttributeValue (view, attributeName, stringValue, desc))
		return true;
	return IMultiBitmapControlCreator::getAttributeValue (view, attributeName, stringValue, desc);
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
