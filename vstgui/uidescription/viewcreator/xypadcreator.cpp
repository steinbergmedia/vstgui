// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "xypadcreator.h"

#include "../../lib/controls/cxypad.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiviewfactory.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
XYPadCreator::XYPadCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr XYPadCreator::getViewName () const
{
	return kCXYPad;
}

//------------------------------------------------------------------------
IdStringPtr XYPadCreator::getBaseViewName () const
{
	return kCParamDisplay;
}

//------------------------------------------------------------------------
UTF8StringPtr XYPadCreator::getDisplayName () const
{
	return "XY Pad";
}

//------------------------------------------------------------------------
CView* XYPadCreator::create (const UIAttributes& attributes,
                             const IUIDescription* description) const
{
	return new CXYPad (CRect (0, 0, 60, 60));
}

//------------------------------------------------------------------------
bool XYPadCreator::apply (CView* view, const UIAttributes& attributes,
						  const IUIDescription* description) const
{
	auto pad = dynamic_cast<CXYPad*> (view);
	if (!pad)
		return false;

	CBitmap* bitmap;
	if (stringToBitmap (attributes.getAttributeValue (kAttrHandleBitmap), bitmap, description))
		pad->setHandleBitmap (bitmap);

	return true;
}

//------------------------------------------------------------------------
bool XYPadCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrHandleBitmap);
	return true;
}

//------------------------------------------------------------------------
auto XYPadCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrHandleBitmap)
		return kBitmapType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool XYPadCreator::getAttributeValue (CView* view, const string& attributeName, string& stringValue,
									  const IUIDescription* desc) const
{
	auto pad = dynamic_cast<CXYPad*> (view);
	if (!pad)
		return false;
	if (attributeName == kAttrHandleBitmap)
	{
		if (auto bitmap = pad->getHandleBitmap ())
		{
			return bitmapToString (bitmap, stringValue, desc);
		}
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
