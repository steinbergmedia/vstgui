// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "multibitmapcontrolcreator.h"
#include "switchcreators.h"

#include "../../lib/controls/cswitch.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
bool SwitchBaseCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrInverseBitmap);
	return IMultiBitmapControlCreator::getAttributeNames (attributeNames);
}

//------------------------------------------------------------------------
auto SwitchBaseCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrInverseBitmap)
		return kBooleanType;
	return IMultiBitmapControlCreator::getAttributeType (attributeName);
}

//------------------------------------------------------------------------
bool SwitchBaseCreator::apply (CView* view, const UIAttributes& attributes,
                               const IUIDescription* description) const
{
	auto control = dynamic_cast<CSwitchBase*> (view);
	if (!control)
		return false;

	bool b;
	if (attributes.getBooleanAttribute (kAttrInverseBitmap, b))
	{
		control->setInverseBitmap (b);
	}

	return IMultiBitmapControlCreator::apply (view, attributes, description);
}

//------------------------------------------------------------------------
bool SwitchBaseCreator::getAttributeValue (CView* view, const string& attributeName,
                                           string& stringValue, const IUIDescription* desc) const
{
	auto control = dynamic_cast<CSwitchBase*> (view);
	if (!control)
		return false;

	if (attributeName == kAttrInverseBitmap)
	{
		stringValue = control->getInverseBitmap () ? strTrue : strFalse;
		return true;
	}
	return IMultiBitmapControlCreator::getAttributeValue (view, attributeName, stringValue, desc);
}

//------------------------------------------------------------------------
VerticalSwitchCreator::VerticalSwitchCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr VerticalSwitchCreator::getViewName () const
{
	return kCVerticalSwitch;
}

//------------------------------------------------------------------------
IdStringPtr VerticalSwitchCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr VerticalSwitchCreator::getDisplayName () const
{
	return "Vertical Switch";
}

//------------------------------------------------------------------------
CView* VerticalSwitchCreator::create (const UIAttributes& attributes,
                                      const IUIDescription* description) const
{
	return new CVerticalSwitch (CRect (0, 0, 0, 0), nullptr, -1, nullptr);
}

//------------------------------------------------------------------------
HorizontalSwitchCreator::HorizontalSwitchCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr HorizontalSwitchCreator::getViewName () const
{
	return kCHorizontalSwitch;
}

//------------------------------------------------------------------------
IdStringPtr HorizontalSwitchCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr HorizontalSwitchCreator::getDisplayName () const
{
	return "Horizontal Switch";
}

//------------------------------------------------------------------------
CView* HorizontalSwitchCreator::create (const UIAttributes& attributes,
                                        const IUIDescription* description) const
{
	return new CHorizontalSwitch (CRect (0, 0, 0, 0), nullptr, -1, nullptr);
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
