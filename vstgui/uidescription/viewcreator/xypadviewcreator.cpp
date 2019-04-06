// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "xypadviewcreator.h"

#include "../../lib/controls/cxypad.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
CXYPadCreator::CXYPadCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr CXYPadCreator::getViewName () const
{
	return kCXYPad;
}

//------------------------------------------------------------------------
IdStringPtr CXYPadCreator::getBaseViewName () const
{
	return kCParamDisplay;
}

//------------------------------------------------------------------------
UTF8StringPtr CXYPadCreator::getDisplayName () const
{
	return "XY Pad";
}

//------------------------------------------------------------------------
CView* CXYPadCreator::create (const UIAttributes& attributes,
                              const IUIDescription* description) const
{
	return new CXYPad (CRect (0, 0, 60, 60));
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
