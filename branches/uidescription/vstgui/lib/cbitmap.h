//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2008, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __cbitmap__
#define __cbitmap__

#include "vstguibase.h"
#include "cpoint.h"
#include "crect.h"
#include "ccolor.h"

#if VSTGUI_USES_COREGRAPHICS
#include <ApplicationServices/ApplicationServices.h>
#endif

#if WINDOWS
	#include <windows.h>

	#if GDIPLUS
	#include <objidl.h>
	#include <gdiplus.h>
	#endif
#endif // WINDOWS

BEGIN_NAMESPACE_VSTGUI
class CDrawContext;

//-----------------------------------------------------------------------------
// CResourceDescription Declaration
//! \brief Describes a resource by name or by ID
/// \nosubgrouping
//-----------------------------------------------------------------------------
class CResourceDescription
{
public:
	enum { kIntegerType, kStringType, kUnknownType };

	CResourceDescription () : type (kUnknownType) { u.name = 0; }
	CResourceDescription (int id) : type (kIntegerType) { u.id = id; }
	CResourceDescription (const char* name) : type (kStringType) { u.name = name; }

	CResourceDescription& operator= (int id) { u.id = id; type = kIntegerType; return *this; }
	CResourceDescription& operator= (const CResourceDescription& desc) { type = desc.type; u.id = desc.u.id; u.name = desc.u.name; return *this; }

	int type;
	union {
		int id;
		const char* name;
	} u;
};

//-----------------------------------------------------------------------------
// CBitmap Declaration
//! \brief Encapsulates various platform depended kinds of bitmaps
/// \nosubgrouping
//-----------------------------------------------------------------------------
class CBitmap : public CBaseObject
{
public:
	//-----------------------------------------------------------------------------
	/// \name Constructors
	//-----------------------------------------------------------------------------
	//@{
	CBitmap (const CResourceDescription& desc);				///< Create a pixmap from a resource identifier.
	CBitmap (CCoord width, CCoord height);					///< Create a pixmap with a given size.
	//@}
	virtual ~CBitmap ();

	//-----------------------------------------------------------------------------
	/// \name CBitmap Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void draw (CDrawContext *pContext, CRect &rect, const CPoint &offset = CPoint (0, 0));	///< Draw the pixmap using a given rect as output position and a given offset of its source pixmap.
	virtual void drawTransparent (CDrawContext *pContext, CRect &rect, const CPoint &offset = CPoint (0, 0));
	virtual void drawAlphaBlend  (CDrawContext *pContext, CRect &rect, const CPoint &offset = CPoint (0, 0), unsigned char alpha = 128);	///< Same as CBitmap::draw except that it uses the alpha value to draw the bitmap alpha blended.

	inline CCoord getWidth () const { return width; }		///< get the width of the image
	inline CCoord getHeight () const { return height; }		///< get the height of the image

	bool isLoaded () const;									///< check if image is loaded
	void *getHandle () const;								///< get a platform image object. Normally you don't need this
	
	void setTransparentColor (const CColor color);						///< set the color of the image which should not be drawn. Works only once on some implementations/platforms.
	CColor getTransparentColor () const { return transparentCColor; }	///< get the current transparent color

	void setNoAlpha (bool state) { noAlpha = state; }
	bool getNoAlpha () const { return noAlpha; }

	const CResourceDescription& getResourceDescription () const { return resourceDesc; }
	//@}

#if VSTGUI_USES_COREGRAPHICS
	CBitmap (CGImageRef cgImage);							///< Create a pixmap from a CGImage
	virtual CGImageRef createCGImage (bool transparent = false);
	virtual void setBitsDirty ();
#endif // VSTGUI_USES_COREGRAPHICS

#if GDIPLUS
	CBitmap (Gdiplus::Bitmap* platformBitmap);							///< Create a pixmap from a Gdiplus Bitmap. 
	Gdiplus::Bitmap* getBitmap ();
#endif // GDIPLUS

	//-------------------------------------------
protected:
	CBitmap ();

	virtual void dispose ();
	virtual bool loadFromResource (const CResourceDescription& resourceDesc);
	virtual bool loadFromPath (const void* platformPath);	// load from a platform path. On Windows it's a C string and on Mac OS X its a CFURLRef.

	CResourceDescription resourceDesc;
	CCoord width;
	CCoord height;

	CColor transparentCColor;
	bool noAlpha;

	void *pHandle;
	void *pMask;
#if WINDOWS
	#if GDIPLUS
	Gdiplus::Bitmap	*pBitmap;
	void* bits;
	#endif // GDIPLUS
#endif // WINDOWS

#if VSTGUI_USES_COREGRAPHICS
	void* cgImage;
#endif // VSTGUI_USES_COREGRAPHICS

};

END_NAMESPACE_VSTGUI

#endif
