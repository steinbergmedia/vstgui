// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "moviebitmapcreator.h"

#include "../../lib/controls/cmoviebitmap.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
MovieBitmapCreator::MovieBitmapCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr MovieBitmapCreator::getViewName () const
{
	return kCMovieBitmap;
}

//------------------------------------------------------------------------
IdStringPtr MovieBitmapCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr MovieBitmapCreator::getDisplayName () const
{
	return "Movie Bitmap";
}

//------------------------------------------------------------------------
CView* MovieBitmapCreator::create (const UIAttributes& attributes,
                                   const IUIDescription* description) const
{
	return new CMovieBitmap (CRect (0, 0, 0, 0), nullptr, -1, nullptr);
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
