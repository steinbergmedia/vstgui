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

#include "macglobals.h"

#if MAC
#include "../../cframe.h"
#include "../iplatformframe.h"
#include "../std_unorderedmap.h"
#include <mach/mach_time.h>

#define USE_MAIN_DISPLAY_COLORSPACE	1

#if !TARGET_OS_IPHONE && MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_7
// forward declare functions Apple removed from OSX SDK 10.11
extern CMError CMGetSystemProfile (CMProfileRef * prof);
extern CMError CMCloseProfile (CMProfileRef prof);
#endif

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

typedef std::unordered_map<CColor, CGColorRef, ColorHash> CGColorMap;

//-----------------------------------------------------------------------------
class CGColorMapImpl
{
public:
	~CGColorMapImpl ()
	{
		for (CGColorMap::const_iterator it = map.begin (), end = map.end (); it != end; ++it)
			CFRelease (it->second);
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
	CGColorMap& colorMap = getColorMap ();
	CGColorMap::const_iterator it = colorMap.find (color);
	if (it != colorMap.end ())
	{
		CGColorRef result = it->second;
		return result;
	}
	const CGFloat components[] = {
		static_cast<CGFloat> (color.red / 255.),
		static_cast<CGFloat> (color.green / 255.),
		static_cast<CGFloat> (color.blue / 255.),
		static_cast<CGFloat> (color.alpha / 255.)
	};
	CGColorRef result = CGColorCreate (GetCGColorSpace (), components);
	colorMap.insert (std::make_pair (color, result));
	return result;
}

//-----------------------------------------------------------------------------
class GenericMacColorSpace
{
public:
	GenericMacColorSpace ()
	{
	#if USE_MAIN_DISPLAY_COLORSPACE
		colorspace = CreateMainDisplayColorSpace ();
	#else
		#if MAC_COCOA
		colorspace = CGColorSpaceCreateWithName (kCGColorSpaceGenericRGB);
		#else
		colorspace = CreateGenericRGBColorSpace ();
		#endif
	#endif
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
	
#if !MAC_COCOA && !USE_MAIN_DISPLAY_COLORSPACE
	//-----------------------------------------------------------------------------
	CMProfileRef OpenGenericProfile(void)
	{
		#define	kGenericRGBProfilePathStr       "/System/Library/ColorSync/Profiles/Generic RGB Profile.icc"

		CMProfileLocation 	loc;
		CMProfileRef cmProfile;
			
		loc.locType = cmPathBasedProfile;
		std::strcpy (loc.u.pathLoc.path, kGenericRGBProfilePathStr);
	
		if (CMOpenProfile (&cmProfile, &loc) != noErr)
			cmProfile = NULL;
		
	    return cmProfile;
	}

	//-----------------------------------------------------------------------------
	static CGColorSpaceRef CreateGenericRGBColorSpace (void)
	{
		CGColorSpaceRef colorspace = 0;
		CMProfileRef genericRGBProfile = OpenGenericProfile ();
	
		if (genericRGBProfile)
		{
			colorspace = CGColorSpaceCreateWithPlatformColorSpace (genericRGBProfile);
			
			// we opened the profile so it is up to us to close it
			CMCloseProfile (genericRGBProfile); 
		}
		if (colorspace == NULL)
			colorspace = CGColorSpaceCreateDeviceRGB ();
		return colorspace;
	}
#endif

#if USE_MAIN_DISPLAY_COLORSPACE
	//-----------------------------------------------------------------------------
	static CGColorSpaceRef CreateMainDisplayColorSpace ()
	{
	#if TARGET_OS_IPHONE
		return CGColorSpaceCreateDeviceRGB ();

	#elif MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7
		ColorSyncProfileRef csProfileRef = ColorSyncProfileCreateWithDisplayID (0);
		if (csProfileRef)
		{
			CGColorSpaceRef colorSpace = CGColorSpaceCreateWithPlatformColorSpace (csProfileRef);
			CFRelease (csProfileRef);
			return colorSpace;
		}
		return 0;
	#else
		CMProfileRef sysprof = NULL;

		// Get the Systems Profile for the main display
		if (CMGetSystemProfile (&sysprof) == noErr)
		{
			// Create a colorspace with the systems profile
			CGColorSpaceRef colorSpace = CGColorSpaceCreateWithPlatformColorSpace (sysprof);

			// Close the profile
			CMCloseProfile (sysprof);
			return colorSpace;
		}
		return 0;
	#endif
	}
#endif

	CGColorSpaceRef colorspace;
};

//-----------------------------------------------------------------------------
CGColorSpaceRef GetCGColorSpace ()
{
	return GenericMacColorSpace::instance ().colorspace;
}

} // namespace

#endif // MAC
