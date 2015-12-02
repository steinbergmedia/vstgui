//
//  ciboxblurfilter.mm
//  a replacement for the box blur filter on Mac OS X using CoreImage
//
//  Created by Arne Scheffler on 24/04/14.
//  Copyright (c) 2014 Arne Scheffler. All rights reserved.
//

#import "ciboxblurfilter.h"
#import "../lib/platform/mac/cgbitmap.h"
#import "../lib/cbitmap.h"
#import <QuartzCore/QuartzCore.h>

namespace VSTGUI {
namespace BitmapFilter {

//------------------------------------------------------------------------
__attribute__((__constructor__)) static void registerFilter ()
{
	Factory::getInstance ().registerFilter (Standard::kBoxBlur, CIBoxBlurFilter::CreateFunction);
}

//------------------------------------------------------------------------
CIBoxBlurFilter::CIBoxBlurFilter () : FilterBase ("A Box Blur Filter using CoreImage")
{
	registerProperty (Standard::Property::kInputBitmap,
					  BitmapFilter::Property (BitmapFilter::Property::kObject));
	registerProperty (Standard::Property::kRadius,
					  BitmapFilter::Property (static_cast<int32_t> (2)));
}

//------------------------------------------------------------------------
bool CIBoxBlurFilter::run (bool replace)
{
	CBitmap* inputBitmap = getInputBitmap ();
	int32_t radius = static_cast<int32_t> (
		static_cast<double> (getProperty (Standard::Property::kRadius).getInteger ()) *
		inputBitmap->getPlatformBitmap ()->getScaleFactor ());
	if (inputBitmap == nullptr)
		return false;

	CGBitmap* cgBitmap = dynamic_cast<CGBitmap*> (inputBitmap->getPlatformBitmap ());
	if (cgBitmap == nullptr)
		return false;

	CIImage* inputImage = [[[CIImage alloc] initWithCGImage:cgBitmap->getCGImage ()] autorelease];
	if (inputImage == nil)
		return false;

	CIFilter* filter = [CIFilter filterWithName:@"CIBoxBlur"];

	NSMutableDictionary* values = [[NSMutableDictionary new] autorelease];
	[values setObject:@(radius) forKey:@"inputRadius"];
	[values setObject:inputImage forKey:@"inputImage"];
	[filter setValuesForKeysWithDictionary:values];
	CIImage* outputImage = [filter valueForKey:@"outputImage"];
	if (outputImage == nil)
		return false;

	SharedPointer<CGBitmap> outputBitmap = owned (new CGBitmap (cgBitmap->getSize ()));
	CGContextRef cgContext = outputBitmap->createCGContext ();
	if (cgContext == nullptr)
		return false;
	CGContextScaleCTM (cgContext, 1, -1);
	CIContext* context = [CIContext contextWithCGContext:cgContext options:nil];
	if (context == nil)
		return false;
	[context drawImage:outputImage
			   atPoint:CGPointMake (0, -cgBitmap->getSize ().y)
			  fromRect:CGRectMake (0, 0, cgBitmap->getSize ().x, cgBitmap->getSize ().y)];
	CFRelease (cgContext);

	outputBitmap->setScaleFactor (cgBitmap->getScaleFactor ());
	if (replace)
	{
		inputBitmap->setPlatformBitmap (outputBitmap);
		return true;
	}
	return registerProperty (Standard::Property::kOutputBitmap,
							 BitmapFilter::Property (owned (new CBitmap (outputBitmap))));
}

//------------------------------------------------------------------------
} // BitmapFilter
} // VSTGUI
