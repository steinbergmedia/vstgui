// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "macglobals.h"

#if MAC
#include "../../cframe.h"
#include "../../ccolor.h"
#include "macfactory.h"
#include "../iplatformframe.h"
#include "../common/fileresourceinputstream.h"
#include "../std_unorderedmap.h"

namespace VSTGUI {

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
		ColorSyncProfileRef csProfileRef = ColorSyncProfileCreateWithDisplayID (CGMainDisplayID ());
		if (csProfileRef)
		{
			CGColorSpaceRef colorSpace = {};
		#if defined(MAC_OS_VERSION_12_0) && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_VERSION_12_0)
			if (__builtin_available (macOS 12.0, *))
			{
				colorSpace = CGColorSpaceCreateWithColorSyncProfile (csProfileRef, nullptr);
			}
		#if (MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_VERSION_12_0)
			else
			{
				colorSpace = CGColorSpaceCreateWithPlatformColorSpace (csProfileRef);
			}
		#endif
		#else
			colorSpace = CGColorSpaceCreateWithPlatformColorSpace (csProfileRef);
		#endif
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

} // VSTGUI

#endif // MAC
