// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "xypadcreator.h"

#include "../../lib/controls/cxypad.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiviewfactory.h"

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
} // UIViewCreator
} // VSTGUI
