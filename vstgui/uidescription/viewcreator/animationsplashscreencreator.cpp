// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "animationsplashscreencreator.h"

#include "../../lib/controls/csplashscreen.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
CAnimationSplashScreenCreator::CAnimationSplashScreenCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr CAnimationSplashScreenCreator::getViewName () const
{
	return kCAnimationSplashScreen;
}

//------------------------------------------------------------------------
IdStringPtr CAnimationSplashScreenCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr CAnimationSplashScreenCreator::getDisplayName () const
{
	return "Animation Splash Screen";
}

//------------------------------------------------------------------------
CView* CAnimationSplashScreenCreator::create (const UIAttributes& attributes,
                                              const IUIDescription* description) const
{
	return new CAnimationSplashScreen (CRect (0, 0, 0, 0), -1, nullptr, nullptr);
}

//------------------------------------------------------------------------
bool CAnimationSplashScreenCreator::apply (CView* view, const UIAttributes& attributes,
                                           const IUIDescription* description) const
{
	auto* splashScreen = dynamic_cast<CAnimationSplashScreen*> (view);
	if (!splashScreen)
		return false;

	CBitmap* bitmap;
	if (stringToBitmap (attributes.getAttributeValue (kAttrSplashBitmap), bitmap, description))
		splashScreen->setSplashBitmap (bitmap);

	CPoint p;
	if (attributes.getPointAttribute (kAttrSplashOrigin, p))
	{
		CRect size = splashScreen->getSplashRect ();
		size.originize ();
		size.offset (p.x, p.y);
		splashScreen->setSplashRect (size);
	}
	if (attributes.getPointAttribute (kAttrSplashSize, p))
	{
		CRect size = splashScreen->getSplashRect ();
		size.setWidth (p.x);
		size.setHeight (p.y);
		splashScreen->setSplashRect (size);
	}
	int32_t value;
	if (attributes.getIntegerAttribute (kAttrAnimationIndex, value))
		splashScreen->setAnimationIndex (static_cast<uint32_t> (value));
	if (attributes.getIntegerAttribute (kAttrAnimationTime, value))
		splashScreen->setAnimationTime (static_cast<uint32_t> (value));

	return true;
}

//------------------------------------------------------------------------
bool CAnimationSplashScreenCreator::getAttributeNames (std::list<std::string>& attributeNames) const
{
	attributeNames.emplace_back (kAttrSplashBitmap);
	attributeNames.emplace_back (kAttrSplashOrigin);
	attributeNames.emplace_back (kAttrSplashSize);
	attributeNames.emplace_back (kAttrAnimationIndex);
	attributeNames.emplace_back (kAttrAnimationTime);
	return true;
}

//------------------------------------------------------------------------
auto CAnimationSplashScreenCreator::getAttributeType (const std::string& attributeName) const
    -> AttrType
{
	if (attributeName == kAttrSplashBitmap)
		return kBitmapType;
	if (attributeName == kAttrSplashOrigin)
		return kRectType;
	if (attributeName == kAttrSplashSize)
		return kRectType;
	if (attributeName == kAttrAnimationIndex)
		return kIntegerType;
	if (attributeName == kAttrAnimationTime)
		return kIntegerType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool CAnimationSplashScreenCreator::getAttributeValue (CView* view,
                                                       const std::string& attributeName,
                                                       std::string& stringValue,
                                                       const IUIDescription* desc) const
{
	auto* splashScreen = dynamic_cast<CAnimationSplashScreen*> (view);
	if (!splashScreen)
		return false;

	if (attributeName == kAttrSplashBitmap)
	{
		CBitmap* bitmap = splashScreen->getSplashBitmap ();
		if (bitmap)
			bitmapToString (bitmap, stringValue, desc);
		else
			stringValue = "";
		return true;
	}
	else if (attributeName == kAttrSplashOrigin)
	{
		stringValue = UIAttributes::pointToString (splashScreen->getSplashRect ().getTopLeft ());
		return true;
	}
	else if (attributeName == kAttrSplashSize)
	{
		stringValue = UIAttributes::pointToString (splashScreen->getSplashRect ().getSize ());
		return true;
	}
	else if (attributeName == kAttrAnimationIndex)
	{
		stringValue = UIAttributes::integerToString (splashScreen->getAnimationIndex ());
		return true;
	}
	else if (attributeName == kAttrAnimationTime)
	{
		stringValue = UIAttributes::integerToString (splashScreen->getAnimationTime ());
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
