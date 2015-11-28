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

#ifndef __iplatformbitmap__
#define __iplatformbitmap__

/// @cond ignore

#include "../vstguifwd.h"

namespace VSTGUI {
class IPlatformBitmapPixelAccess;

//-----------------------------------------------------------------------------
class IPlatformBitmap : public CBaseObject
{
public:
	static IPlatformBitmap* create (CPoint* size = 0); ///< if size pointer is not zero, create a bitmap which can be used as a draw surface
	static IPlatformBitmap* createFromPath (UTF8StringPtr absolutePath); ///< create a bitmap from an absolute path

	/** Create a platform bitmap from memory */
	static IPlatformBitmap* createFromMemory (const void* ptr, uint32_t memSize);

	/** Create a memory representation of the platform bitmap in PNG format. The memory could be used by createFromMemory.
		Caller needs to free the memory in ptr */
	static bool createMemoryPNGRepresentation (IPlatformBitmap* bitmap, void** ptr, uint32_t& size);

	virtual bool load (const CResourceDescription& desc) = 0;
	virtual const CPoint& getSize () const = 0;

	virtual IPlatformBitmapPixelAccess* lockPixels (bool alphaPremultiplied) = 0;	// you need to forget the result after use.
	
	virtual void setScaleFactor (double factor) = 0;
	virtual double getScaleFactor () const = 0;
};

//------------------------------------------------------------------------------------
class IPlatformBitmapPixelAccess : public CBaseObject
{
public:
	enum PixelFormat {
		kARGB,
		kRGBA,
		kABGR,
		kBGRA
	};
	
	virtual uint8_t* getAddress () const = 0;
	virtual uint32_t getBytesPerRow () const = 0;
	virtual PixelFormat getPixelFormat () const = 0;
};

} // namespace

/// @endcond

#endif // __iplatformbitmap__
