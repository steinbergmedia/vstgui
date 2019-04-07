// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "moviebuttoncreator.h"

#include "../../lib/controls/cmoviebutton.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
MovieButtonCreator::MovieButtonCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr MovieButtonCreator::getViewName () const
{
	return kCMovieButton;
}

//------------------------------------------------------------------------
IdStringPtr MovieButtonCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr MovieButtonCreator::getDisplayName () const
{
	return "Movie Button";
}

//------------------------------------------------------------------------
CView* MovieButtonCreator::create (const UIAttributes& attributes,
                                   const IUIDescription* description) const
{
	return new CMovieButton (CRect (0, 0, 0, 0), nullptr, -1, nullptr);
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
