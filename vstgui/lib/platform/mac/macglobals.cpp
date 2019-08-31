// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "macglobals.h"

#if MAC
#include "../../cframe.h"
#include "../../ccolor.h"
#include "../iplatformframe.h"
#include "../common/fileresourceinputstream.h"
#include "../std_unorderedmap.h"
#include <mach/mach_time.h>

#define USE_MAIN_DISPLAY_COLORSPACE	1

namespace VSTGUI {

//-----------------------------------------------------------------------------
uint32_t IPlatformFrame::getTicks ()
{
	static struct mach_timebase_info timebaseInfo;
	static bool initialized = false;
	if (!initialized)
	{
		initialized = true;
		mach_timebase_info (&timebaseInfo);
	}
	uint64_t absTime = mach_absolute_time ();
	double d = (absTime / timebaseInfo.denom) * timebaseInfo.numer;	// nano seconds
	return static_cast<uint32_t> (d / 1000000);
}

//-----------------------------------------------------------------------------
struct ColorHash
{
	size_t operator () (const CColor& c) const
	{
		size_t v1 = static_cast<size_t> (c.red | (c.green << 8) | (c.blue << 16) | (c.alpha << 24));
		return v1;
	}
};

using CGColorMap = std::unordered_map<CColor, CGColorRef, ColorHash>;

//-----------------------------------------------------------------------------
class CGColorMapImpl
{
public:
	~CGColorMapImpl ()
	{
		for (auto& it : map)
			CFRelease (it.second);
	}
	
	CGColorMap map;
};

//-----------------------------------------------------------------------------
static CGColorMap& getColorMap ()
{
	static CGColorMapImpl colorMap;
	return colorMap.map;
}

//-----------------------------------------------------------------------------
CGColorRef getCGColor (const CColor& color)
{
	auto& colorMap = getColorMap ();
	auto it = colorMap.find (color);
	if (it != colorMap.end ())
	{
		CGColorRef result = it->second;
		return result;
	}
	const CGFloat components[] = {color.normRed<CGFloat> (), color.normGreen<CGFloat> (),
	                              color.normBlue<CGFloat> (), color.normAlpha<CGFloat> ()};
	CGColorRef result = CGColorCreate (GetCGColorSpace (), components);
	colorMap.emplace (color, result);
	return result;
}

//-----------------------------------------------------------------------------
class GenericMacColorSpace
{
public:
	GenericMacColorSpace ()
	{
		colorspace = CreateMainDisplayColorSpace ();
	}
	
	~GenericMacColorSpace ()
	{
		CGColorSpaceRelease (colorspace);
	}

	static GenericMacColorSpace& instance ()
	{
		static GenericMacColorSpace gInstance;
		return gInstance;
	}
	
	//-----------------------------------------------------------------------------
	static CGColorSpaceRef CreateMainDisplayColorSpace ()
	{
	#if TARGET_OS_IPHONE
		return CGColorSpaceCreateDeviceRGB ();

	#else
		ColorSyncProfileRef csProfileRef = ColorSyncProfileCreateWithDisplayID (0);
		if (csProfileRef)
		{
			CGColorSpaceRef colorSpace = CGColorSpaceCreateWithPlatformColorSpace (csProfileRef);
			CFRelease (csProfileRef);
			return colorSpace;
		}
		return nullptr;
	#endif
	}

	CGColorSpaceRef colorspace;
};

//-----------------------------------------------------------------------------
CGColorSpaceRef GetCGColorSpace ()
{
	return GenericMacColorSpace::instance ().colorspace;
}

//-----------------------------------------------------------------------------
auto IPlatformResourceInputStream::create (const CResourceDescription& desc) -> Ptr
{
	if (desc.type == CResourceDescription::kIntegerType)
		return nullptr;
	if (auto bundle = getBundleRef ())
	{
		Ptr result;
		CFStringRef cfStr = CFStringCreateWithCString (nullptr, desc.u.name, kCFStringEncodingUTF8);
		if (cfStr)
		{
			CFURLRef url = CFBundleCopyResourceURL (getBundleRef (), cfStr, nullptr, nullptr);
			if (url)
			{
				char filePath[PATH_MAX];
				if (CFURLGetFileSystemRepresentation (url, true, (UInt8*)filePath, PATH_MAX))
				{
					result = FileResourceInputStream::create (filePath);
				}
				CFRelease (url);
			}
			CFRelease (cfStr);
		}
		return result;
	}
	return nullptr;
}

} // VSTGUI

#endif // MAC
