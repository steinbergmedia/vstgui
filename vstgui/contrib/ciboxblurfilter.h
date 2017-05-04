// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../lib/cbitmapfilter.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace BitmapFilter {

//------------------------------------------------------------------------
class CIBoxBlurFilter : public BitmapFilter::FilterBase
{
public:
	CIBoxBlurFilter ();

	bool run (bool replace) override;

	static IFilter* CreateFunction (IdStringPtr _name) { return new CIBoxBlurFilter (); }
};

//------------------------------------------------------------------------
} // BitmapFilter
} // VSTGUI
