//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef __macglobals__
#define __macglobals__

#include "../../vstguibase.h"

#if MAC

#include "../../crect.h"
#include "../../cpoint.h"

#if TARGET_OS_IPHONE
	#include <CoreFoundation/CoreFoundation.h>
	#include <CoreGraphics/CoreGraphics.h>
#else
	#include <ApplicationServices/ApplicationServices.h>
#endif

namespace VSTGUI {
struct CColor;

// TODO: This needs to be done a nicer fashion
extern void* gBundleRef;
inline CFBundleRef getBundleRef () { return (CFBundleRef)gBundleRef; }
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

} // namespace

#endif // MAC
#endif // __macglobals__
