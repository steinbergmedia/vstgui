// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "onoffbuttoncreator.h"

#include "../../lib/controls/cbuttons.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
OnOffButtonCreator::OnOffButtonCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr OnOffButtonCreator::getViewName () const
{
	return kCOnOffButton;
}

//------------------------------------------------------------------------
IdStringPtr OnOffButtonCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr OnOffButtonCreator::getDisplayName () const
{
	return "OnOff Button";
}

//------------------------------------------------------------------------
CView* OnOffButtonCreator::create (const UIAttributes& attributes,
                                   const IUIDescription* description) const
{
	return new COnOffButton (CRect (0, 0, 20, 20), nullptr, -1, nullptr);
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
