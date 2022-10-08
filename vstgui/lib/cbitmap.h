// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cpoint.h"
#include "crect.h"
#include "cresourcedescription.h"
#include "pixelbuffer.h"
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
	using BitmapVector = std::vector<PlatformBitmapPtr>;
	using const_iterator = BitmapVector::const_iterator;

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

	const_iterator begin () const { return bitmaps.begin (); }
	const_iterator end () const { return bitmaps.end (); }
	//@}

//-----------------------------------------------------------------------------
protected:
	CBitmap ();

	CResourceDescription resourceDesc;
	BitmapVector bitmaps;
};

//-----------------------------------------------------------------------------
/** Description for a multi frame bitmap
 *
 *	@ingroup new_in_4_12
 */
struct CMultiFrameBitmapDescription
{
	/** size of one frame */
	CPoint frameSize {};
	/** number of total frames */
	uint16_t numFrames {};
	/** number of frames per row */
	uint16_t framesPerRow {1};
};

//-----------------------------------------------------------------------------
/** Multi frame bitmap
 *
 *	A bitmap describing multiple frames ordered in rows and columns
 *
 *	The index order is columns and then rows:
 *
 *	1.Row: 1 -> 2 -> 3
 *	2.Row: 4 -> 5 -> 6
 *	...
 *
 *	@ingroup new_in_4_12
 */
class CMultiFrameBitmap : public CBitmap
{
public:
	using CBitmap::CBitmap;

	CMultiFrameBitmap (const CResourceDescription& desc,
					   CMultiFrameBitmapDescription multiFrameDesc);

	/** set the multi frame description
	 *
	 *	@param frameSize size of one frame
	 *	@param frameCount number of total frames
	 *	@param framesPerRow number of frames per row
	 *	@return true if bitmap is big enough for the description
	 */
	bool setMultiFrameDesc (CMultiFrameBitmapDescription desc);
	/** get the mult frame description */
	CMultiFrameBitmapDescription getMultiFrameDesc () const;
	/** get the frame size */
	CPoint getFrameSize () const;
	/** get the number of frames */
	uint16_t getNumFrames () const;
	/** get the number of frames per row */
	uint16_t getNumFramesPerRow () const;
	/** calculate the rect for one frame */
	CRect calcFrameRect (uint32_t frameIndex) const;
	/** draw one frame at the position in the context */
	void drawFrame (CDrawContext* context, uint16_t frameIndex, CPoint pos);

private:
	CMultiFrameBitmapDescription description;
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
/** Convert between Platform Pixel Accessor pixel format and PixelBuffer format */
template<typename T1, typename T2,
		 typename std::enable_if<
			 std::is_same<PixelBuffer::Format, T2>::value ||
			 std::is_same<IPlatformBitmapPixelAccess::PixelFormat, T2>::value>::type* = nullptr>
inline T1 convert (T2 format)
{
	using PixelFormat = IPlatformBitmapPixelAccess::PixelFormat;
	using Format = PixelBuffer::Format;
	static_assert (std::is_same<Format, T1>::value || std::is_same<PixelFormat, T1>::value,
				   "Unexpected Format");
	static_assert (!std::is_same<T1, T2>::value, "Unexpected Format");
	static_assert (static_cast<int32_t> (Format::ARGB) == PixelFormat::kARGB, "Format Mismatch");
	static_assert (static_cast<int32_t> (Format::ABGR) == PixelFormat::kABGR, "Format Mismatch");
	static_assert (static_cast<int32_t> (Format::RGBA) == PixelFormat::kRGBA, "Format Mismatch");
	static_assert (static_cast<int32_t> (Format::BGRA) == PixelFormat::kBGRA, "Format Mismatch");
	return static_cast<T1> (format);
}

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
