// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "autoanimationcreator.h"

#include "../../lib/controls/cautoanimation.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
AutoAnimationCreator::AutoAnimationCreator () { UIViewFactory::registerViewCreator (*this); }

//------------------------------------------------------------------------
IdStringPtr AutoAnimationCreator::getViewName () const { return kCAutoAnimation; }

//------------------------------------------------------------------------
IdStringPtr AutoAnimationCreator::getBaseViewName () const { return kCControl; }

//------------------------------------------------------------------------
UTF8StringPtr AutoAnimationCreator::getDisplayName () const { return "Auto Animation"; }

//------------------------------------------------------------------------
CView* AutoAnimationCreator::create (const UIAttributes& attributes,
									 const IUIDescription* description) const
{
	return new CAutoAnimation (CRect (0, 0, 0, 0), nullptr, -1, nullptr);
}

//------------------------------------------------------------------------
bool AutoAnimationCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrAnimationTime);
	attributeNames.emplace_back (kAttrBitmapOffset);
	return true;
}

//------------------------------------------------------------------------
auto AutoAnimationCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrAnimationTime)
		return kIntegerType;
	if (attributeName == kAttrBitmapOffset)
		return kPointType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool AutoAnimationCreator::apply (CView* view, const UIAttributes& attributes,
								  const IUIDescription* description) const
{
	auto autoAnimation = dynamic_cast<CAutoAnimation*> (view);
	if (autoAnimation == nullptr)
		return false;
	int32_t value;
	if (attributes.getIntegerAttribute (kAttrAnimationTime, value))
		autoAnimation->setAnimationTime (static_cast<uint32_t> (value));
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	CPoint point;
	if (attributes.getPointAttribute (kAttrBitmapOffset, point))
		autoAnimation->setBitmapOffset (point);
#endif
	return true;
}

//------------------------------------------------------------------------
bool AutoAnimationCreator::getAttributeValue (CView* view, const string& attributeName,
											  string& stringValue, const IUIDescription* desc) const
{
	auto autoAnimation = dynamic_cast<CAutoAnimation*> (view);
	if (autoAnimation == nullptr)
		return false;
	if (attributeName == kAttrAnimationTime)
	{
		stringValue = UIAttributes::integerToString (
			static_cast<int32_t> (autoAnimation->getAnimationTime ()));
		return true;
	}
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	if (attributeName == kAttrBitmapOffset)
	{
		stringValue = UIAttributes::pointToString (autoAnimation->getBitmapOffset ());
		return true;
	}
#endif
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
