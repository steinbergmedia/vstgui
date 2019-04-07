// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "multibitmapcontrolcreator.h"

#include "../../lib/controls/ccontrol.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
bool IMultiBitmapControlCreator::apply (CView* view, const UIAttributes& attributes,
                                        const IUIDescription* description)
{
	auto* multiBitmapControl = dynamic_cast<IMultiBitmapControl*> (view);
	if (!multiBitmapControl)
		return false;

	int32_t value;
	if (attributes.getIntegerAttribute (kAttrHeightOfOneImage, value))
		multiBitmapControl->setHeightOfOneImage (value);
	else
		multiBitmapControl->autoComputeHeightOfOneImage ();

	if (attributes.getIntegerAttribute (kAttrSubPixmaps, value))
		multiBitmapControl->setNumSubPixmaps (value);
	return true;
}

//------------------------------------------------------------------------
bool IMultiBitmapControlCreator::getAttributeNames (StringList& attributeNames)
{
	attributeNames.emplace_back (kAttrHeightOfOneImage);
	attributeNames.emplace_back (kAttrSubPixmaps);
	return true;
}

//------------------------------------------------------------------------
IViewCreator::AttrType IMultiBitmapControlCreator::getAttributeType (const string& attributeName)
{
	if (attributeName == kAttrHeightOfOneImage)
		return IViewCreator::AttrType::kIntegerType;
	if (attributeName == kAttrSubPixmaps)
		return IViewCreator::AttrType::kIntegerType;
	return IViewCreator::AttrType::kUnknownType;
}

//------------------------------------------------------------------------
bool IMultiBitmapControlCreator::getAttributeValue (CView* view, const string& attributeName,
                                                    string& stringValue, const IUIDescription* desc)
{
	auto* multiBitmapControl = dynamic_cast<IMultiBitmapControl*> (view);
	if (!multiBitmapControl)
		return false;

	if (attributeName == kAttrHeightOfOneImage)
	{
		stringValue = UIAttributes::integerToString (
		    static_cast<int32_t> (multiBitmapControl->getHeightOfOneImage ()));
		return true;
	}
	if (attributeName == kAttrSubPixmaps)
	{
		stringValue = UIAttributes::integerToString (multiBitmapControl->getNumSubPixmaps ());
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
