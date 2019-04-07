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
CMovieButtonCreator::CMovieButtonCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr CMovieButtonCreator::getViewName () const
{
	return kCMovieButton;
}

//------------------------------------------------------------------------
IdStringPtr CMovieButtonCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr CMovieButtonCreator::getDisplayName () const
{
	return "Movie Button";
}

//------------------------------------------------------------------------
CView* CMovieButtonCreator::create (const UIAttributes& attributes,
                                    const IUIDescription* description) const
{
	return new CMovieButton (CRect (0, 0, 0, 0), nullptr, -1, nullptr);
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
