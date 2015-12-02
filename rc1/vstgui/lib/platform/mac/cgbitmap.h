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

#ifndef __cgbitmap__
#define __cgbitmap__

#include "../iplatformbitmap.h"

#if MAC
#include "../../cpoint.h"

#if TARGET_OS_IPHONE
	#include <CoreGraphics/CoreGraphics.h>
	#include <ImageIO/ImageIO.h>
#else
	#include <ApplicationServices/ApplicationServices.h>
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
class CGBitmap : public IPlatformBitmap
{
public:
	CGBitmap (const CPoint& size);
	CGBitmap (CGImageRef image);
	CGBitmap ();
	~CGBitmap ();
	
	bool load (const CResourceDescription& desc) VSTGUI_OVERRIDE_VMETHOD;
	const CPoint& getSize () const VSTGUI_OVERRIDE_VMETHOD { return size; }
	IPlatformBitmapPixelAccess* lockPixels (bool alphaPremultiplied) VSTGUI_OVERRIDE_VMETHOD;
	void setScaleFactor (double factor) VSTGUI_OVERRIDE_VMETHOD { scaleFactor = factor; }
	double getScaleFactor () const VSTGUI_OVERRIDE_VMETHOD { return scaleFactor; }

	CGImageRef getCGImage ();
	CGContextRef createCGContext ();
	bool loadFromImageSource (CGImageSourceRef source);

	void setDirty () { dirty = true; }
	void* getBits () const { return bits; }
	uint32_t getBytesPerRow () const { return bytesPerRow; }

	CGLayerRef createCGLayer (CGContextRef context);
	CGLayerRef getCGLayer () const { return layer; }
//-----------------------------------------------------------------------------
protected:
	void allocBits ();
	void freeCGImage ();

	CPoint size;
	CGImageRef image;
	CGImageSourceRef imageSource;

	CGLayerRef layer;

	void* bits;
	bool dirty;
	uint32_t bytesPerRow;
	double scaleFactor;
};

} // namespace

#endif // MAC
#endif // __cgbitmap__
