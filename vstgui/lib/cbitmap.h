// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cpoint.h"
#include "crect.h"
#include "cresourcedescription.h"
#include "platform/iplatformbitmap.h"
#include <vector>

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CBitmap Declaration
//! @brief Encapsulates various platform depended kinds of bitmaps
//-----------------------------------------------------------------------------
class CBitmap : public AtomicReferenceCounted
{
public:
	/** Create an image from a resource identifier */
	explicit CBitmap (const CResourceDescription& desc);
	/** Create an image with a given size */
	CBitmap (CCoord width, CCoord height);
	/** Create an image with a given size and scale factor */
	CBitmap (CPoint size, double scaleFactor = 1.);
	explicit CBitmap (const PlatformBitmapPtr& platformBitmap);
	~CBitmap () noexcept override = default;

	//-----------------------------------------------------------------------------
	/// @name CBitmap Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void draw (CDrawContext* context, const CRect& rect, const CPoint& offset = CPoint (0, 0), float alpha = 1.f);

	/** get the width of the image */
	CCoord getWidth () const;
	/** get the height of the image */
	CCoord getHeight () const;
	/** get size of image */
	CPoint getSize () const;

	/** check if image is loaded */
	bool isLoaded () const { return getPlatformBitmap () ? true : false; }

	const CResourceDescription& getResourceDescription () const { return resourceDesc; }

	PlatformBitmapPtr getPlatformBitmap () const;
	void setPlatformBitmap (const PlatformBitmapPtr& bitmap);

	bool addBitmap (const PlatformBitmapPtr& platformBitmap);
	PlatformBitmapPtr getBestPlatformBitmapForScaleFactor (double scaleFactor) const;
	//@}

//-----------------------------------------------------------------------------
protected:
	CBitmap ();

	CResourceDescription resourceDesc;
	using BitmapVector = std::vector<PlatformBitmapPtr>;
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
	
	CCoord left {0.};
	CCoord top {0.};
	CCoord right {0.};
	CCoord bottom {0.};

	CNinePartTiledDescription () = default;
	CNinePartTiledDescription (CCoord left, CCoord top, CCoord right, CCoord bottom)
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
	CNinePartTiledBitmap (const PlatformBitmapPtr& platformBitmap, const CNinePartTiledDescription& offsets);
	~CNinePartTiledBitmap () noexcept override = default;
	
	//-----------------------------------------------------------------------------
	/// @name Part Offsets
	//-----------------------------------------------------------------------------
	//@{
	void setPartOffsets (const CNinePartTiledDescription& partOffsets) { offsets = partOffsets; }
	const CNinePartTiledDescription& getPartOffsets () const { return offsets; }
	//@}

	void draw (CDrawContext* context, const CRect& rect, const CPoint& offset = CPoint (0, 0), float alpha = 1.f) override;

//-----------------------------------------------------------------------------
protected:
	CNinePartTiledDescription offsets;
};

//------------------------------------------------------------------------
// CBitmapPixelAccess
/// @brief direct pixel access to a CBitmap
/// @ingroup new_in_4_0
//------------------------------------------------------------------------
class CBitmapPixelAccess : public AtomicReferenceCounted
{
public:
	/** advance position */
	inline bool operator++ ();
	/** set current position */
	inline bool setPosition (uint32_t x, uint32_t y);
	/** return current x position */
	inline uint32_t getX () const { return x; }
	/** return current y position */
	inline uint32_t getY () const { return y; }
	/** get color of current pixel */
	virtual void getColor (CColor& c) const = 0;
	/** set color of current pixel */
	virtual void setColor (const CColor& c) = 0;

	/** get native color value */
	inline void getValue (uint32_t& value);
	/** set native color value */
	inline void setValue (uint32_t value);

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
	~CBitmapPixelAccess () noexcept override = default;
	void init (CBitmap* bitmap, IPlatformBitmapPixelAccess* pixelAccess);

	CBitmap* bitmap;
	SharedPointer<IPlatformBitmapPixelAccess> pixelAccess;
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

} // VSTGUI
