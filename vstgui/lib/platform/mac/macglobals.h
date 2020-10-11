// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../vstguibase.h"

#if MAC

#include "../../crect.h"
#include "../../cpoint.h"
#include "macfactory.h"

#if TARGET_OS_IPHONE
	#include <CoreFoundation/CoreFoundation.h>
	#include <CoreGraphics/CoreGraphics.h>
#else
	#include <ApplicationServices/ApplicationServices.h>
#endif

namespace VSTGUI {
struct CColor;

inline CFBundleRef getBundleRef ()
{
	if (auto mf = getPlatformFactory ().asMacFactory ())
	{
		return mf->getBundle ();
	}
	return nullptr;
}
extern CGColorSpaceRef GetCGColorSpace ();
extern CGColorRef getCGColor (const CColor& color);

//-----------------------------------------------------------------------------
inline CRect CRectFromCGRect (const CGRect& r)
{
	return CRect (CPoint (r.origin.x, r.origin.y), CPoint (r.size.width, r.size.height));
}

//-----------------------------------------------------------------------------
inline CGRect CGRectFromCRect (const CRect& r)
{
	return CGRectMake (static_cast<CGFloat> (r.left), static_cast<CGFloat> (r.top), static_cast<CGFloat> (r.getWidth ()), static_cast<CGFloat> (r.getHeight ()));
}

//-----------------------------------------------------------------------------
inline CPoint CPointFromCGPoint (const CGPoint& p)
{
	return CPoint (p.x, p.y);
}

//-----------------------------------------------------------------------------
inline CGPoint CGPointFromCPoint (const CPoint& p)
{
	return CGPointMake (static_cast<CGFloat> (p.x), static_cast<CGFloat> (p.y));
}

//-----------------------------------------------------------------------------
inline CGSize CGSizeFromCPoint (const CPoint& p)
{
	return CGSizeMake (static_cast<CGFloat> (p.x), static_cast<CGFloat> (p.y));
}

} // VSTGUI

#endif // MAC
