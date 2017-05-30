// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __iplatformbitmap__
#define __iplatformbitmap__

/// @cond ignore

#include "../vstguifwd.h"
#include <vector>

namespace VSTGUI {
using PNGBitmapBuffer = std::vector<uint8_t>;
class IPlatformBitmapPixelAccess;

//-----------------------------------------------------------------------------
class IPlatformBitmap : public AtomicReferenceCounted
{
public:
	static SharedPointer<IPlatformBitmap> create (CPoint* size = nullptr); ///< if size pointer is not zero, create a bitmap which can be used as a draw surface
	static SharedPointer<IPlatformBitmap> createFromPath (UTF8StringPtr absolutePath); ///< create a bitmap from an absolute path

	/** Create a platform bitmap from memory */
	static SharedPointer<IPlatformBitmap> createFromMemory (const void* ptr, uint32_t memSize);

	/** Create a memory representation of the platform bitmap in PNG format. */
	static PNGBitmapBuffer createMemoryPNGRepresentation (const SharedPointer<IPlatformBitmap>& bitmap);

	virtual bool load (const CResourceDescription& desc) = 0;
	virtual const CPoint& getSize () const = 0;

	virtual SharedPointer<IPlatformBitmapPixelAccess> lockPixels (bool alphaPremultiplied) = 0;
	
	virtual void setScaleFactor (double factor) = 0;
	virtual double getScaleFactor () const = 0;
};

//------------------------------------------------------------------------------------
class IPlatformBitmapPixelAccess : public AtomicReferenceCounted
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
