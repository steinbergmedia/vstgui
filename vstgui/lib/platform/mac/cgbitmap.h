// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
	explicit CGBitmap (const CPoint& size);
	explicit CGBitmap (CGImageRef image);
	CGBitmap ();
	~CGBitmap () noexcept override;
	
	bool load (const CResourceDescription& desc) override;
	const CPoint& getSize () const override { return size; }
	SharedPointer<IPlatformBitmapPixelAccess> lockPixels (bool alphaPremultiplied) override;
	void setScaleFactor (double factor) override { scaleFactor = factor; }
	double getScaleFactor () const override { return scaleFactor; }

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
