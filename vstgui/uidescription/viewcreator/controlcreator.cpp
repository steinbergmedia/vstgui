// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "controlcreator.h"

#include "../../lib/controls/ccontrol.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
struct CControlCreatorDummyControl : CControl
{
	CControlCreatorDummyControl () : CControl (CRect (0, 0, 40, 40), nullptr, -1) {}
	void draw (CDrawContext* pContext) override { CView::draw (pContext); }

	CLASS_METHODS (CControlCreatorDummyControl, CControl)
};

//------------------------------------------------------------------------
ControlCreator::ControlCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr ControlCreator::getViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
IdStringPtr ControlCreator::getBaseViewName () const
{
	return kCView;
}

//------------------------------------------------------------------------
UTF8StringPtr ControlCreator::getDisplayName () const
{
	return "Control";
}

//------------------------------------------------------------------------
CView* ControlCreator::create (const UIAttributes& attributes,
                               const IUIDescription* description) const
{
	return new CControlCreatorDummyControl ();
}

//------------------------------------------------------------------------
bool ControlCreator::apply (CView* view, const UIAttributes& attributes,
                            const IUIDescription* description) const
{
	auto* control = dynamic_cast<CControl*> (view);
	if (control == nullptr)
		return false;

	double value;
	if (attributes.getDoubleAttribute (kAttrDefaultValue, value))
		control->setDefaultValue (static_cast<float> (value));
	if (attributes.getDoubleAttribute (kAttrMinValue, value))
		control->setMin (static_cast<float> (value));
	if (attributes.getDoubleAttribute (kAttrMaxValue, value))
		control->setMax (static_cast<float> (value));
	if (attributes.getDoubleAttribute (kAttrWheelIncValue, value))
		control->setWheelInc (static_cast<float> (value));

	const auto* controlTagAttr = attributes.getAttributeValue (kAttrControlTag);
	if (controlTagAttr)
	{
		if (controlTagAttr->length () == 0)
		{
			control->setTag (-1);
			control->setListener (nullptr);
		}
		else
		{
			int32_t tag = description->getTagForName (controlTagAttr->c_str ());
			if (tag != -1)
			{
				control->setListener (description->getControlListener (controlTagAttr->c_str ()));
				control->setTag (tag);
			}
			else
			{
				char* endPtr = nullptr;
				tag = (int32_t)strtol (controlTagAttr->c_str (), &endPtr, 10);
				if (endPtr != controlTagAttr->c_str ())
				{
					control->setListener (
					    description->getControlListener (controlTagAttr->c_str ()));
					control->setTag (tag);
				}
				else
				{
					control->setTag (-1);
				}
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------
bool ControlCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrControlTag);
	attributeNames.emplace_back (kAttrDefaultValue);
	attributeNames.emplace_back (kAttrMinValue);
	attributeNames.emplace_back (kAttrMaxValue);
	attributeNames.emplace_back (kAttrWheelIncValue);
	return true;
}

//------------------------------------------------------------------------
auto ControlCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrControlTag)
		return kTagType;
	else if (attributeName == kAttrDefaultValue)
		return kFloatType;
	else if (attributeName == kAttrMinValue)
		return kFloatType;
	else if (attributeName == kAttrMaxValue)
		return kFloatType;
	else if (attributeName == kAttrWheelIncValue)
		return kFloatType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool ControlCreator::getAttributeValue (CView* view, const string& attributeName,
                                        string& stringValue, const IUIDescription* desc) const
{
	auto* control = dynamic_cast<CControl*> (view);
	if (control == nullptr)
		return false;
	if (attributeName == kAttrControlTag)
	{
		if (control->getTag () != -1)
		{
			UTF8StringPtr controlTag = desc->lookupControlTagName (control->getTag ());
			if (controlTag)
			{
				stringValue = controlTag;
				return true;
			}
		}
	}
	else if (attributeName == kAttrDefaultValue)
	{
		stringValue = UIAttributes::doubleToString (control->getDefaultValue ());
		return true;
	}
	else if (attributeName == kAttrMinValue)
	{
		stringValue = UIAttributes::doubleToString (control->getMin ());
		return true;
	}
	else if (attributeName == kAttrMaxValue)
	{
		stringValue = UIAttributes::doubleToString (control->getMax ());
		return true;
	}
	else if (attributeName == kAttrWheelIncValue)
	{
		stringValue = UIAttributes::doubleToString (control->getWheelInc (), 5);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
