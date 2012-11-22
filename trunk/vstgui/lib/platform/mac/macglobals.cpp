//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2011, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
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
#include <mach/mach_time.h>

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
class GenericMacColorSpace
{
public:
	GenericMacColorSpace ()
	{
		#if MAC_COCOA
		rgbColorspace = CGColorSpaceCreateWithName (kCGColorSpaceGenericRGB);
		#else
		CreateGenericRGBColorSpace ();
		#endif
		CreateMainDisplayColorSpace ();
	}
	
	~GenericMacColorSpace ()
	{
		CGColorSpaceRelease (rgbColorspace);
		CGColorSpaceRelease (mainDisplayColorSpace);
	}

	static GenericMacColorSpace& instance ()
	{
		static GenericMacColorSpace gInstance;
		return gInstance;
	}
	
	#if !MAC_COCOA
	//-----------------------------------------------------------------------------
	CMProfileRef OpenGenericProfile(void)
	{
		#define	kGenericRGBProfilePathStr       "/System/Library/ColorSync/Profiles/Generic RGB Profile.icc"

		CMProfileLocation 	loc;
		CMProfileRef cmProfile;
			
		loc.locType = cmPathBasedProfile;
		strcpy (loc.u.pathLoc.path, kGenericRGBProfilePathStr);
	
		if (CMOpenProfile (&cmProfile, &loc) != noErr)
			cmProfile = NULL;
		
	    return cmProfile;
	}

	//-----------------------------------------------------------------------------
	void CreateGenericRGBColorSpace(void)
	{
		CMProfileRef genericRGBProfile = OpenGenericProfile ();
	
		if (genericRGBProfile)
		{
			rgbColorspace = CGColorSpaceCreateWithPlatformColorSpace (genericRGBProfile);
			
			// we opened the profile so it is up to us to close it
			CMCloseProfile (genericRGBProfile); 
		}
		if (rgbColorspace == NULL)
			rgbColorspace = CGColorSpaceCreateDeviceRGB ();
	}
	#endif

	void CreateMainDisplayColorSpace ()
	{
		// TODO: Replace with ColorSyncProfileRef
		CMProfileRef sysprof = NULL;

		// Get the Systems Profile for the main display
		if (CMGetSystemProfile (&sysprof) == noErr)
		{
			// Create a colorspace with the systems profile
			mainDisplayColorSpace = CGColorSpaceCreateWithPlatformColorSpace (sysprof);

			// Close the profile
			CMCloseProfile (sysprof);
		}
	}

	CGColorSpaceRef rgbColorspace;
	CGColorSpaceRef mainDisplayColorSpace;
};

//-----------------------------------------------------------------------------
CGColorSpaceRef GetGenericRGBColorSpace ()
{
	return GenericMacColorSpace::instance ().rgbColorspace;
}

//-----------------------------------------------------------------------------
CGColorSpaceRef GetMainDisplayColorSpace ()
{
	return GenericMacColorSpace::instance ().mainDisplayColorSpace;
}

} // namespace

#endif // MAC
