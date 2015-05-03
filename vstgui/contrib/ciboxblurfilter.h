//
//  ciboxblurfilter.h
//  a replacement for the box blur filter on Mac OS X using CoreImage
//
//  Created by Arne Scheffler on 24/04/14.
//  Copyright (c) 2014 Arne Scheffler. All rights reserved.
//

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

	bool run (bool replace) VSTGUI_OVERRIDE_VMETHOD;

	static IFilter* CreateFunction (IdStringPtr _name) { return new CIBoxBlurFilter (); }
};

//------------------------------------------------------------------------
} // BitmapFilter
} // VSTGUI
