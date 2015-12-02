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

#ifndef __coffscreencontext__
#define __coffscreencontext__

#include "vstguifwd.h"
#include "cdrawcontext.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// COffscreenContext Declaration
//! @brief A draw context using a bitmap as it's back buffer
/*! @class COffscreenContext
There are two usage scenarios :
@section offscreen_usage1 Drawing into a bitmap and then push the contents into another draw context

@code
COffscreenContext* offscreen = COffscreenContext::create (frame, 100, 100);
if (offscreen)
{
	offscreen->beginDraw ();
	// ... 
	// draw into offscreen
	// ...
	offscreen->endDraw ();
	offscreen->copyFrom (otherContext, destRect);
	offscreen->forget ();
}
@endcode

@section offscreen_usage2 Drawing static content into a bitmap and reuse the bitmap for drawing

@code
if (cachedBitmap == 0)
{
	COffscreenContext* offscreen = COffscreenContext::create (frame, 100, 100);
	if (offscreen)
	{
		offscreen->beginDraw ();
		// ... 
		// draw into offscreen
		// ...
		offscreen->endDraw ();
		cachedBitmap = offscreen->getBitmap ();
		if (cachedBitmap)
			cachedBitmap->remember ();
		offscreen->forget ();
	}
}
if (cachedBitmap)
{
	// ...
}

@endcode

 */
//-----------------------------------------------------------------------------
class COffscreenContext : public CDrawContext
{
public:
	static COffscreenContext* create (CFrame* frame, CCoord width, CCoord height, double scaleFactor = 1.);

	//-----------------------------------------------------------------------------
	/// @name COffscreenContext Methods
	//-----------------------------------------------------------------------------
	//@{
	void copyFrom (CDrawContext *pContext, CRect destRect, CPoint srcOffset = CPoint (0, 0));	///< copy from offscreen to pContext

	CCoord getWidth () const;
	CCoord getHeight () const;
	//@}

	CBitmap* getBitmap () const { return bitmap; }

protected:
	COffscreenContext (CBitmap* bitmap);
	COffscreenContext (const CRect& surfaceRect);
	~COffscreenContext ();

	CBitmap* bitmap;
};

} // namespace

#endif // __coffscreencontext__
