//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
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

#include "../cbitmap.h"

namespace VSTGUI {
class CResourceDescription;
class IBitmapReaderCreator;

//-----------------------------------------------------------------------------
class IPlatformBitmap : public CBaseObject
{
public:
	static IPlatformBitmap* create (CPoint* size = 0); // if size pointer is not zero, create a bitmap which can be used as a draw surface

	virtual bool load (const CResourceDescription& desc) = 0;
	virtual const CPoint& getSize () const = 0;

	static IBitmapReaderCreator* gCustomBitmapReaderCreator; // overrides default platform specific bitmap loading, see below
};

//------------------------------------------------------------------------------------
class IBitmapReader : public CBaseObject
{
public:
	virtual int readBytes (void* buffer, int numBytes) = 0;
	virtual int getNumBytes () = 0;
	virtual void rewind () = 0;
};

//------------------------------------------------------------------------------------
class IBitmapReaderCreator
{
public:
	virtual IBitmapReader* createBitmapReader (const CResourceDescription& desc) = 0;
};

} // namespace

/// @endcond

#endif // __iplatformbitmap__