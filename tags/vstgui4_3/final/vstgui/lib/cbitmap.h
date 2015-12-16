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

#ifndef __cbitmap__
#define __cbitmap__

#include "vstguifwd.h"
#include "cpoint.h"
#include "crect.h"
#include "cresourcedescription.h"
#include <vector>

namespace VSTGUI {

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

	IPlatformBitmap* getPlatformBitmap () const;
	void setPlatformBitmap (IPlatformBitmap* bitmap);

	bool addBitmap (IPlatformBitmap* platformBitmap);
	IPlatformBitmap* getBestPlatformBitmapForScaleFactor (double scaleFactor) const;
	//@}

//-----------------------------------------------------------------------------
	CLASS_METHODS_NOCOPY(CBitmap, CBaseObject)
protected:
	CBitmap ();

	CResourceDescription resourceDesc;
	typedef SharedPointer<IPlatformBitmap> BitmapPointer;
	typedef std::vector<BitmapPointer> BitmapVector;
	BitmapVector bitmaps;
};

//-----------------------------------------------------------------------------
struct CNinePartTiledDescription
{
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
	
	CCoord left;
	CCoord top;
	CCoord right;
	CCoord bottom;
	
	CNinePartTiledDescription (CCoord left = 0, CCoord top = 0, CCoord right = 0, CCoord bottom = 0)
	: left (left), top (top), right (right), bottom (bottom) {}

	//-----------------------------------------------------------------------------
	inline void calcRects (const CRect& inBitmapRect, CRect outRect[kPartCount]) const
	{
		// Center
		CRect myCenter = outRect[kPartCenter]	(inBitmapRect.left		+ left,
												 inBitmapRect.top		+ top,
												 inBitmapRect.right		- right,
												 inBitmapRect.bottom	- bottom);
		
		// Edges
		outRect[kPartTop]			(myCenter.left,		inBitmapRect.top,	myCenter.right,		myCenter.top);
		outRect[kPartLeft]			(inBitmapRect.left,	myCenter.top,		myCenter.left,		myCenter.bottom);
		outRect[kPartRight]			(myCenter.right,	myCenter.top,		inBitmapRect.right,	myCenter.bottom);
		outRect[kPartBottom]		(myCenter.left,		myCenter.bottom,	myCenter.right,		inBitmapRect.bottom);
		
		// Corners
		outRect[kPartTopLeft]		(inBitmapRect.left,	inBitmapRect.top,	myCenter.left,		myCenter.top);
		outRect[kPartTopRight]		(myCenter.right,	inBitmapRect.top,	inBitmapRect.right,	myCenter.top);
		outRect[kPartBottomLeft]	(inBitmapRect.left,	myCenter.bottom,	myCenter.left,		inBitmapRect.bottom);
		outRect[kPartBottomRight]	(myCenter.right,	myCenter.bottom,	inBitmapRect.right,	inBitmapRect.bottom);
	}
	
};

//-----------------------------------------------------------------------------
// CNinePartTiledBitmap Declaration
/// @brief a nine-part tiled bitmap
/// @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class CNinePartTiledBitmap : public CBitmap
{
public:
	CNinePartTiledBitmap (const CResourceDescription& desc, const CNinePartTiledDescription& offsets);
	CNinePartTiledBitmap (IPlatformBitmap* platformBitmap, const CNinePartTiledDescription& offsets);
	~CNinePartTiledBitmap ();
	
	//-----------------------------------------------------------------------------
	/// @name Part Offsets
	//-----------------------------------------------------------------------------
	//@{
	void setPartOffsets (const CNinePartTiledDescription& partOffsets) { offsets = partOffsets; }
	const CNinePartTiledDescription& getPartOffsets () const { return offsets; }
	//@}

	virtual void draw (CDrawContext* context, const CRect& rect, const CPoint& offset = CPoint (0, 0), float alpha = 1.f) VSTGUI_OVERRIDE_VMETHOD;

//-----------------------------------------------------------------------------
	CLASS_METHODS_NOCOPY(CNinePartTiledBitmap, CBitmap)
protected:
	CNinePartTiledDescription offsets;
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
	inline bool setPosition (uint32_t x, uint32_t y);		///< set current position
	inline uint32_t getX () const { return x; }			///< return current x position
	inline uint32_t getY () const { return y; }			///< return current y position
	virtual void getColor (CColor& c) const = 0;	///< get color of current pixel
	virtual void setColor (const CColor& c) = 0;	///< set color of current pixel

	inline void getValue (uint32_t& value);			///< get native color value
	inline void setValue (uint32_t value);			///< set native color value

	inline uint32_t getBitmapWidth () const { return maxX+1; }
	inline uint32_t getBitmapHeight () const { return maxY+1; }

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
inline bool CBitmapPixelAccess::setPosition (uint32_t _x, uint32_t _y)
{
	if (_x > maxX || _y > maxY)
		return false;
	x = _x;
	y = _y;
	currentPos = address + y * bytesPerRow + x * 4;
	return true;
}

//------------------------------------------------------------------------
inline void CBitmapPixelAccess::getValue (uint32_t& value)
{
	value = *(uint32_t*) (currentPos);
}

//------------------------------------------------------------------------
inline void CBitmapPixelAccess::setValue (uint32_t value)
{
	*(uint32_t*) (currentPos) = value;
}

} // namespace VSTGUI

#endif
