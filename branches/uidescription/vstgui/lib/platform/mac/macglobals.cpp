/*
 *  macglobals.cpp
 *  VST3PlugIns
 *
 *  Created by Arne Scheffler on 11/6/09.
 *  Copyright 2009 Arne Scheffler. All rights reserved.
 *
 */

#include "macglobals.h"

#if MAC

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
class GenericMacColorSpace
{
public:
	GenericMacColorSpace ()
	{
		#if MAC_COCOA
		colorspace = CGColorSpaceCreateWithName (kCGColorSpaceGenericRGB);
		#else
		CreateGenericRGBColorSpace ();
		#endif
	}
	
	~GenericMacColorSpace () { CGColorSpaceRelease (colorspace); }

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
			colorspace = CGColorSpaceCreateWithPlatformColorSpace (genericRGBProfile);
			
			// we opened the profile so it is up to us to close it
			CMCloseProfile (genericRGBProfile); 
		}
		if (colorspace == NULL)
			colorspace = CGColorSpaceCreateDeviceRGB ();
	}
	#endif

	CGColorSpaceRef colorspace;
};

//-----------------------------------------------------------------------------
CGColorSpaceRef GetGenericRGBColorSpace ()
{
	static GenericMacColorSpace gGenericMacColorSpace;
	return gGenericMacColorSpace.colorspace;
}

END_NAMESPACE_VSTGUI

#endif // MAC
