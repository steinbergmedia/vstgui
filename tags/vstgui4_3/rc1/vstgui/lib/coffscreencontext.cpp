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

#include "coffscreencontext.h"
#include "cframe.h"
#include "cbitmap.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
COffscreenContext::COffscreenContext (CBitmap* bitmap)
: CDrawContext (CRect (0, 0, bitmap->getWidth (), bitmap->getHeight ()))
, bitmap (bitmap)
{
	bitmap->remember ();
}

//-----------------------------------------------------------------------------
COffscreenContext::COffscreenContext (const CRect& surfaceRect)
: CDrawContext (surfaceRect)
, bitmap (0)
{
}

//-----------------------------------------------------------------------------
COffscreenContext::~COffscreenContext ()
{
	if (bitmap)
		bitmap->forget ();
}

//-----------------------------------------------------------------------------
void COffscreenContext::copyFrom (CDrawContext *pContext, CRect destRect, CPoint srcOffset)
{
	if (bitmap)
		bitmap->draw (pContext, destRect, srcOffset);
}

//-----------------------------------------------------------------------------
COffscreenContext* COffscreenContext::create (CFrame* frame, CCoord width, CCoord height, double scaleFactor)
{
	if (width >= 1. && height >= 1.)
	{
		IPlatformFrame* pf = frame ? frame->getPlatformFrame () : 0;
		if (pf)
			return pf->createOffscreenContext (width, height, scaleFactor);
	}
	return 0;
}

//-----------------------------------------------------------------------------
CCoord COffscreenContext::getWidth () const
{
	return bitmap ? bitmap->getWidth () : 0.;
}

//-----------------------------------------------------------------------------
CCoord COffscreenContext::getHeight () const
{
	return bitmap ? bitmap->getHeight () : 0.;
}

} // namespace
