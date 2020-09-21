// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

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
	static PlatformBitmapPtr create (CPoint* size);
	static PlatformBitmapPtr createFromPath (UTF8StringPtr absolutePath);
	static PlatformBitmapPtr createFromMemory (const void* ptr, uint32_t memSize);
	static PNGBitmapBuffer createMemoryPNGRepresentation (const PlatformBitmapPtr& bitmap);

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
	CGImageRef image {nullptr};
	CGImageSourceRef imageSource {nullptr};

	CGLayerRef layer {nullptr};

	CGDataProviderRef bitsDataProvider {nullptr};

	void* bits {nullptr};
	bool dirty {false};
	uint32_t bytesPerRow {0};
	double scaleFactor {1.};
};

} // VSTGUI

#endif // MAC
