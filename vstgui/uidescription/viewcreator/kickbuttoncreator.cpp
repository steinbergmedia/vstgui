// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "kickbuttoncreator.h"

#include "../../lib/controls/cbuttons.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
KickButtonCreator::KickButtonCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr KickButtonCreator::getViewName () const
{
	return kCKickButton;
}

//------------------------------------------------------------------------
IdStringPtr KickButtonCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr KickButtonCreator::getDisplayName () const
{
	return "Kick Button";
}

//------------------------------------------------------------------------
CView* KickButtonCreator::create (const UIAttributes& attributes,
                                  const IUIDescription* description) const
{
	return new CKickButton (CRect (0, 0, 0, 0), nullptr, -1, nullptr);
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
