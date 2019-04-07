// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "multibitmapcontrolcreator.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
struct MovieBitmapCreator : MultiBitmapControlCreator
{
	MovieBitmapCreator ();
	IdStringPtr getViewName () const override;
	IdStringPtr getBaseViewName () const override;
	UTF8StringPtr getDisplayName () const override;
	CView* create (const UIAttributes& attributes,
	               const IUIDescription* description) const override;
};

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
