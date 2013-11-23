//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __cbitmap__
#define __cbitmap__

#include "vstguibase.h"
#include "cpoint.h"
#include "crect.h"
#include "ccolor.h"

#include "platform/iplatformbitmap.h"

namespace VSTGUI {
class CDrawContext;
class IPlatformBitmap;
class IPlatformBitmapPixelAccess;

//-----------------------------------------------------------------------------
// CResourceDescription Declaration
//! @brief Describes a resource by name or by ID
//-----------------------------------------------------------------------------
class CResourceDescription
{
public:
	enum { kIntegerType, kStringType, kUnknownType };

	CResourceDescription () : type (kUnknownType) { u.name = 0; }
	CResourceDescription (int32_t id) : type (kIntegerType) { u.id = id; }
	CResourceDescription (UTF8StringPtr name) : type (kStringType) { u.name = name; }

	CResourceDescription& operator= (int32_t id) { u.id = id; type = kIntegerType; return *this; }
	CResourceDescription& operator= (const CResourceDescription& desc) { type = desc.type; u.id = desc.u.id; u.name = desc.u.name; return *this; }

	int32_t type;
	union {
		int32_t id;
		UTF8StringPtr name;
	} u;
};

//-----------------------------------------------------------------------------
// CBitmap Declaration
//! @brief Encapsulates various platform depended kinds of bitmaps
//-----------------------------------------------------------------------------
class CBitmap : public CBaseObject
{
public:
	CBitmap (const CResourceDescription& desc);				///< Create a pixmap from a resource identifier.
	CBitmap (CCoord width, CCoord height);					///< Create a pixmap with a given size.
	CBitmap (IPlatformBitmap* platformBitmap);
	~CBitmap ();

	//-----------------------------------------------------------------------------
	/// @name CBitmap Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void draw (CDrawContext* context, const CRect& rect, const CPoint& offset = CPoint (0, 0), float alpha = 1.f);

	CCoord getWidth () const;		///< get the width of the image
	CCoord getHeight () const;		///< get the height of the image

	bool isLoaded () const { return getPlatformBitmap () ? true : false; }	///< check if image is loaded

	const CResourceDescription& getResourceDescription () const { return resourceDesc; }

	IPlatformBitmap* getPlatformBitmap () const { return platformBitmap; }
	void setPlatformBitmap (IPlatformBitmap* bitmap);
	//@}

//-----------------------------------------------------------------------------
	CLASS_METHODS_NOCOPY(CBitmap, CBaseObject)
protected:
	CBitmap ();

	CResourceDescription resourceDesc;
	IPlatformBitmap* platformBitmap;
};

//-----------------------------------------------------------------------------
// CNinePartTiledBitmap Declaration
/// @brief a nine-part tiled bitmap
/// @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class CNinePartTiledBitmap : public CBitmap
{
public:
	struct PartOffsets
	{
		CCoord left;
		CCoord top;
		CCoord right;
		CCoord bottom;
		
		PartOffsets (CCoord left = 0, CCoord top = 0, CCoord right = 0, CCoord bottom = 0)
		: left (left), top (top), right (right), bottom (bottom) {}
	};

	CNinePartTiledBitmap (const CResourceDescription& desc, const PartOffsets& offsets);
	CNinePartTiledBitmap (IPlatformBitmap* platformBitmap, const PartOffsets& offsets);
	~CNinePartTiledBitmap ();
	
	//-----------------------------------------------------------------------------
	/// @name Part Offsets
	//-----------------------------------------------------------------------------
	//@{
	void setPartOffsets (const PartOffsets& partOffsets) { offsets = partOffsets; }
	const PartOffsets& getPartOffsets () const { return offsets; }
	//@}

	virtual void draw (CDrawContext* context, const CRect& rect, const CPoint& offset = CPoint (0, 0), float alpha = 1.f) VSTGUI_OVERRIDE_VMETHOD;

//-----------------------------------------------------------------------------
	CLASS_METHODS_NOCOPY(CNinePartTiledBitmap, CBitmap)
protected:
	virtual void drawParts (CDrawContext* pContext, const CRect& rect, float alpha = 1.f);
	virtual void calcPartRects(const CRect& inBitmapRect, const PartOffsets& inPartOffset, CRect* outRect);
	virtual void drawPart (CDrawContext* inContext, const CRect& inSourceRect, const CRect& inDestRect, float inAlpha = 1.f);

	enum
	{
		kPartTopLeft,
		kPartTop,
		kPartTopRight,
		kPartLeft,
		kPartCenter,
		kPartRight,
		kPartBottomLeft,
		kPartBottom,
		kPartBottomRight,
		kPartCount
	};

	PartOffsets offsets;
};

//------------------------------------------------------------------------
// CBitmapPixelAccess
/// @brief direct pixel access to a CBitmap
/// @ingroup new_in_4_0
//------------------------------------------------------------------------
class CBitmapPixelAccess : public CBaseObject
{
public:
	inline bool operator++ ();								///< advance position
	inline bool operator++ (int) { return ++(*this); }	///< advance position
	inline bool operator-- ();								///< decrease position
	inline bool operator-- (int) { return --(*this); }	///< decrease position
	inline bool setPosition (uint32_t x, uint32_t y);		///< set current position
	inline uint32_t getX () const { return x; }			///< return current x position
	inline uint32_t getY () const { return y; }			///< return current y position
	virtual void getColor (CColor& c) const = 0;	///< get color of current pixel
	virtual void setColor (const CColor& c) = 0;	///< set color of current pixel

	inline uint32_t getBitmapWidth () const { return (uint32_t)bitmap->getWidth (); }
	inline uint32_t getBitmapHeight () const { return (uint32_t)bitmap->getHeight (); }

	inline IPlatformBitmapPixelAccess* getPlatformBitmapPixelAccess () const { return pixelAccess; }
	/** create an accessor.
		can return 0 if platform implementation does not support this.
		result needs to be forgotten before the CBitmap reflects the change to the pixels */
	static CBitmapPixelAccess* create (CBitmap* bitmap, bool alphaPremultiplied = true);
//-----------------------------------------------------------------------------
protected:
	CBitmapPixelAccess ();
	~CBitmapPixelAccess ();
	void init (CBitmap* bitmap, IPlatformBitmapPixelAccess* pixelAccess);

	CBitmap* bitmap;
	IPlatformBitmapPixelAccess* pixelAccess;
	uint8_t* currentPos;
	uint8_t* address;
	uint32_t bytesPerRow;
	uint32_t maxX;
	uint32_t maxY;
	uint32_t x;
	uint32_t y;
};

//------------------------------------------------------------------------
inline bool CBitmapPixelAccess::operator++ ()
{
	if (x < maxX)
	{
		x++;
		currentPos += 4;
		return true;
	}
	else if (y < maxY)
	{
		y++;
		x = 0;
		currentPos = address + y * bytesPerRow;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
inline bool CBitmapPixelAccess::operator-- ()
{
	if (x > 0 && y > 0)
	{
		x--;
		currentPos -= 4;
		return true;
	}
	else if (y > 0)
	{
		y--;
		x = 0;
		currentPos = address + y * bytesPerRow;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
inline bool CBitmapPixelAccess::setPosition (uint32_t _x, uint32_t _y)
{
	if (_x > maxX || _y > maxY)
		return false;
	x = _x;
	y = _y;
	currentPos = address + y * bytesPerRow + x * 4;
	return true;
}

} // namespace VSTGUI

#endif
