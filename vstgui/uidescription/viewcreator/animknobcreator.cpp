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
AnimKnobCreator::AnimKnobCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr AnimKnobCreator::getViewName () const
{
	return kCAnimKnob;
}

//------------------------------------------------------------------------
IdStringPtr AnimKnobCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr AnimKnobCreator::getDisplayName () const
{
	return "Animation Knob";
}

//------------------------------------------------------------------------
CView* AnimKnobCreator::create (const UIAttributes& attributes,
                                const IUIDescription* description) const
{
	return new CAnimKnob (CRect (0, 0, 0, 0), nullptr, -1, nullptr);
}

//------------------------------------------------------------------------
bool AnimKnobCreator::apply (CView* view, const UIAttributes& attributes,
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
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	IMultiBitmapControlCreator::apply (view, attributes, description);
#endif
	return KnobBaseCreator::apply (view, attributes, description);
}

//------------------------------------------------------------------------
bool AnimKnobCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrInverseBitmap);
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	IMultiBitmapControlCreator::getAttributeNames (attributeNames);
#endif
	return KnobBaseCreator::getAttributeNames (attributeNames);
}

//------------------------------------------------------------------------
auto AnimKnobCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrInverseBitmap)
		return kBooleanType;
	auto res = KnobBaseCreator::getAttributeType (attributeName);
	if (res != kUnknownType)
		return res;
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	return IMultiBitmapControlCreator::getAttributeType (attributeName);
#else
	return res;
#endif
}

//------------------------------------------------------------------------
bool AnimKnobCreator::getAttributeValue (CView* view, const string& attributeName,
                                         string& stringValue, const IUIDescription* desc) const
{
	auto* animKnob = dynamic_cast<CAnimKnob*> (view);
	if (!animKnob)
		return false;

	if (attributeName == kAttrInverseBitmap)
	{
		stringValue = animKnob->getInverseBitmap () ? strTrue : strFalse;
		return true;
	}
	if (KnobBaseCreator::getAttributeValue (view, attributeName, stringValue, desc))
		return true;
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	return IMultiBitmapControlCreator::getAttributeValue (view, attributeName, stringValue, desc);
#else
	return false;
#endif
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
