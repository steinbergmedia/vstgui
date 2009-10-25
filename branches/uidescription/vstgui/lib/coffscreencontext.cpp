//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2008, Steinberg Media Technologies, All Rights Reserved
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

#include "coffscreencontext.h"
#include "cframe.h"
#include "cbitmap.h"
#include "win32support.h"

BEGIN_NAMESPACE_VSTGUI

#if VSTGUI_USES_COREGRAPHICS
	static CGContextRef createOffscreenBitmap (long width, long height, void** bits);
	#ifndef CGFLOAT_DEFINED
		#define CGFLOAT_DEFINED
		typedef float CGFloat;
	#endif
#endif

//-----------------------------------------------------------------------------
// COffscreenContext Implementation
//-----------------------------------------------------------------------------
COffscreenContext::COffscreenContext (CDrawContext* pContext, CBitmap* pBitmapBg, bool drawInBitmap)
: CDrawContext (pContext->pFrame, NULL, NULL)
, pBitmap (0)
, pBitmapBg (pBitmapBg)
, height (20)
, width (20)
, bDrawInBitmap (drawInBitmap)
{
	if (pBitmapBg)
	{
		height = pBitmapBg->getHeight ();
		width  = pBitmapBg->getWidth ();
		
		clipRect (0, 0, width, height);
	}

	#if DEBUG
		gNbCOffscreenContext++;
		gBitmapAllocation += (long)height * (long)width;
	#endif // DEBUG
		
	bDestroyPixmap = false;
	
#if WINDOWS
	#if GDIPLUS	
	pSystemContext = CreateCompatibleDC ((HDC)pContext->getSystemContext ());
	
	if (drawInBitmap)
	{
		// get GDI+ Bitmap from the CBitmap
		pGraphics = new Gdiplus::Graphics (pBitmapBg->getBitmap());  // our Context to draw on the bitmap

	}
	else // create bitmap if no bitmap handle exists
	{
		bDestroyPixmap = false;	// was true, but since we attach it to a CBitmap from VSTGUI that destroys it.
		pWindow = CreateCompatibleBitmap ((HDC)pContext->getSystemContext (), (int)width, (int)height);
		Gdiplus::Bitmap* myOffscreenBitmap = Gdiplus::Bitmap::FromHBITMAP ((HBITMAP)pWindow, NULL);
		pGraphics = new Gdiplus::Graphics (myOffscreenBitmap);  // our Context to draw on the bitmap
		// CHECK_GDIPLUS_STATUS("OffScreenContext: from OffscreenBitmap",pGraphics);
		pBitmap = new CBitmap (myOffscreenBitmap);	// the VSTGUI Bitmap Object
	}
	
	if (pGraphics)
	{
		//pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeLowQuality);
		pGraphics->SetPageUnit (Gdiplus::UnitPixel);
		//debug("graphics resolution x=%3.0f y=%3.0f\n",pGraphics->GetDpiX(),pGraphics->GetDpiY());

	}
	
	#else
	if (pOldBrush)
		SelectObject ((HDC)getSystemContext (), pOldBrush);
	if (pOldPen)
		SelectObject ((HDC)getSystemContext (), pOldPen);
	if (pOldFont)
		SelectObject ((HDC)getSystemContext (), pOldFont);
	pOldBrush = pOldPen = pOldFont = 0;

	pSystemContext = CreateCompatibleDC ((HDC)pContext->getSystemContext ());
	
	if (drawInBitmap)
		pWindow = pBitmapBg->getHandle ();
	else // create bitmap if no bitmap handle exists
	{
		bDestroyPixmap = true;
		pWindow = CreateCompatibleBitmap ((HDC)pContext->getSystemContext (), width, height);
	}
	oldBitmap = SelectObject ((HDC)pSystemContext, pWindow);
	#endif // GDIPLUS

#elif VSTGUI_USES_COREGRAPHICS
	offscreenBitmap = 0;
	if (drawInBitmap)
		offscreenBitmap = pBitmapBg->getHandle ();
	if (offscreenBitmap == 0)
		bDestroyPixmap = true;
	gCGContext = createOffscreenBitmap (width, height, &offscreenBitmap);

        
#endif

	if (!drawInBitmap)
	{
		// draw bitmap to Offscreen
		CRect r (0, 0, width, height);
		if (pBitmapBg)
			pBitmapBg->draw (this, r);
		else
		{
			setFillColor (kBlackCColor);
			drawRect (r, kDrawFilled);
		}
	}
}

//-----------------------------------------------------------------------------
COffscreenContext::COffscreenContext (CBitmap* inBitmap)
: CDrawContext (0, 0, 0)
, pBitmap (0)
, pBitmapBg (inBitmap)
, width (inBitmap->getWidth ())
, height (inBitmap->getHeight ())
, bDrawInBitmap (true)
, bDestroyPixmap (false)
, backgroundColor (kTransparentCColor)
{
	clipRect (0, 0, (CCoord)width, (CCoord)height);
	
#if VSTGUI_USES_COREGRAPHICS
	offscreenBitmap = pBitmapBg->getHandle ();
	if (offscreenBitmap != 0)
	{
		gCGContext = createOffscreenBitmap (width, height, &offscreenBitmap);
	}
	else
	{
		#if DEBUG
		DebugPrint ("Drawing into a loaded Bitmap not supported. Please use the CBitmap (widht, height) constructor if you want to draw into it.\n");
		#endif
	}

#else
	#if GDIPLUS
	pGraphics = new Gdiplus::Graphics (pBitmapBg->getBitmap ());
	pGraphics->SetInterpolationMode (Gdiplus::InterpolationModeLowQuality);
	pGraphics->SetPageUnit(Gdiplus::UnitPixel);
	#else
	// TODO: Windows GDI implementation
	#endif
#endif

}

//-----------------------------------------------------------------------------
COffscreenContext::COffscreenContext (CFrame* pFrame, long width, long height, const CColor backgroundColor)
: CDrawContext (pFrame, NULL, NULL)
, pBitmap (0)
, pBitmapBg (0)
, height ((CCoord)height)
, width ((CCoord)width)
, bDrawInBitmap (false)
, backgroundColor (backgroundColor)
{
	clipRect (0, 0, (CCoord)width, (CCoord)height);

	#if DEBUG
	gNbCOffscreenContext++;
	gBitmapAllocation += height * width;
	#endif

	bDestroyPixmap = true;

#if WINDOWS
	#if GDIPLUS
	pBitmap = new CBitmap ((CCoord)width, (CCoord)height);
	pGraphics = new Gdiplus::Graphics (pBitmap->getBitmap ());
	pGraphics->SetInterpolationMode (Gdiplus::InterpolationModeLowQuality);
	pGraphics->SetPageUnit(Gdiplus::UnitPixel);
	#else
	void* SystemWindow = pFrame ? pFrame->getSystemWindow () : 0;
	void* SystemContext = GetDC ((HWND)SystemWindow);
	
	pSystemContext = CreateCompatibleDC ((HDC)SystemContext);
	#if DEBUG
	gNbDC++;
	#endif
	pWindow = CreateCompatibleBitmap ((HDC)SystemContext, width, height);

	oldBitmap = SelectObject ((HDC)pSystemContext, pWindow);
	ReleaseDC ((HWND)SystemWindow, (HDC)SystemContext);

	CRect r (0, 0, width, height);
	setFillColor (backgroundColor);
	setFrameColor (backgroundColor);
	drawRect (r, kDrawFilledAndStroked);
	#endif

#elif VSTGUI_USES_COREGRAPHICS
	offscreenBitmap = 0;
	gCGContext = createOffscreenBitmap (width, height, &offscreenBitmap);

	CRect r (0, 0, width, height);
	setFillColor (backgroundColor);
	setFrameColor (backgroundColor);
	drawRect (r, kDrawFilledAndStroked);

#endif
}

//-----------------------------------------------------------------------------
COffscreenContext::~COffscreenContext ()
{
	#if DEBUG
	gNbCOffscreenContext--;
	gBitmapAllocation -= (long)height * (long)width;
	#endif

	if (pBitmap)
		pBitmap->forget ();

#if WINDOWS
	#if GDIPLUS
	#else
	if (pSystemContext)
	{
		DeleteDC ((HDC)pSystemContext);
		pSystemContext = 0;
		#if DEBUG
		gNbDC--;
		#endif
	}
	if (bDestroyPixmap && pWindow)
		DeleteObject (pWindow);
	#endif

#elif VSTGUI_USES_COREGRAPHICS
	if (gCGContext)
	{
		CGContextRestoreGState (gCGContext);
		CGContextRelease (gCGContext);
	}
	gCGContext = 0;
	if (offscreenBitmap && bDestroyPixmap)
		free (offscreenBitmap);
        
#endif
}

//-----------------------------------------------------------------------------
void COffscreenContext::resetClipRect ()
{
	setClipRect (CRect (0, 0, width, height));
}

//-----------------------------------------------------------------------------
void COffscreenContext::copyTo (CDrawContext* pContext, CRect& srcRect, CPoint destOffset)
{
#if WINDOWS
	#if GDIPLUS
	// GDIPLUS todo
	#else
	BitBlt ((HDC)pSystemContext,
			destOffset.h,
			destOffset.v,
			srcRect.width (),
			srcRect.height (),
			(HDC)pContext->getSystemContext (),
			srcRect.left + pContext->offset.h,
			srcRect.top + pContext->offset.v,
			SRCCOPY);
	#endif
	
#elif MAC
	// not supported
#endif
}

//-----------------------------------------------------------------------------
void COffscreenContext::copyFrom (CDrawContext* pContext, CRect destRect, CPoint srcOffset)
{
#if WINDOWS
	#if GDIPLUS
	if (pBitmap)
	{
		pBitmap->draw (pContext, destRect, srcOffset);
	}
	#else
	BitBlt ((HDC)pContext->getSystemContext (),  // hdcDest
					destRect.left + pContext->offset.h, // xDest
					destRect.top + pContext->offset.v,  // yDest
					destRect.right - destRect.left,     // xWidth,
					destRect.bottom - destRect.top,     // yHeight
          
					(HDC)pSystemContext,                // hdcSrc
					srcOffset.h,                        // xSrc
					srcOffset.v,                        // ySrc
					SRCCOPY);                           // dwROP
	#endif

#elif VSTGUI_USES_COREGRAPHICS
	if (!gCGContext)
		return;
	CGContextRef context = pContext->beginCGContext ();
	if (context)
	{
		CGImageRef image = 0;
		#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
		image = getCGImage ();
		#endif
		if (image == 0)
		{
			size_t pixRowBytes = CGBitmapContextGetBytesPerRow (gCGContext);
			short pixDepth = CGBitmapContextGetBitsPerPixel (gCGContext);
			size_t size = pixRowBytes * CGBitmapContextGetHeight (gCGContext);
			CGDataProviderRef provider = CGDataProviderCreateWithData (NULL, CGBitmapContextGetData (gCGContext), size, NULL);
			CGImageAlphaInfo alphaInfo = CGBitmapContextGetAlphaInfo (gCGContext);
			image = CGImageCreate (CGBitmapContextGetWidth (gCGContext), CGBitmapContextGetHeight (gCGContext), 8 , pixDepth, pixRowBytes, GetGenericRGBColorSpace (), alphaInfo, provider, NULL, 0, kCGRenderingIntentDefault);
			CGDataProviderRelease (provider);
		}
		if (image)
		{
			CGRect dest;
			dest.origin.x = destRect.left - srcOffset.h + pContext->offset.h;
			dest.origin.y = (destRect.top + pContext->offset.v) * -1 - (getHeight () - srcOffset.v);
			dest.size.width = getWidth ();
			dest.size.height = getHeight ();
			
			CGRect clipRect;
			clipRect.origin.x = destRect.left + pContext->offset.h;
		    clipRect.origin.y = (destRect.top + pContext->offset.v) * -1  - destRect.height ();
		    clipRect.size.width = destRect.width (); 
		    clipRect.size.height = destRect.height ();
			
			CGContextClipToRect (context, clipRect);

			CGContextDrawImage (context, dest, image);
			
			CGImageRelease (image);
		}
		
		pContext->releaseCGContext (context);
	}

#endif
}

//-----------------------------------------------------------------------------
#if VSTGUI_USES_COREGRAPHICS
//-----------------------------------------------------------------------------
CGImageRef COffscreenContext::getCGImage () const
{
	if (gCGContext)
		return CGBitmapContextCreateImage (gCGContext);
	return 0;
}

//-----------------------------------------------------------------------------
void COffscreenContext::releaseCGContext (CGContextRef context)
{
	if (bDrawInBitmap && pBitmapBg)
		pBitmapBg->setBitsDirty ();
	CDrawContext::releaseCGContext (context);
}

//-----------------------------------------------------------------------------
CGContextRef createOffscreenBitmap (long width, long height, void** bits)
{
	CGContextRef    context = NULL; 
	int             bitmapByteCount; 
	int             bitmapBytesPerRow; 

	// each pixel is represented by four bytes 
	// (8 bits each of alpha, R, G, B) 
	bitmapBytesPerRow   = width * 4; 
	bitmapByteCount     = bitmapBytesPerRow * height; 

	// create the bitmap 
	if (*bits == 0)
		*bits = malloc (bitmapByteCount);
	if (*bits != NULL)
	{
		memset (*bits, 0, bitmapByteCount);
		// create the context 
		context = CGBitmapContextCreate (*bits,
		width, 
		height, 
		8,              // bits per component 
		bitmapBytesPerRow, 
		GetGenericRGBColorSpace (), 
		kCGImageAlphaPremultipliedFirst);

		if (context)
		{
			CGContextSaveGState (context);
			CGContextSetShouldAntialias (context, false);
			CGContextTranslateCTM (context, 0, (CGFloat)height);
			CGContextSetFillColorSpace (context, GetGenericRGBColorSpace ());
			CGContextSetStrokeColorSpace (context, GetGenericRGBColorSpace ()); 
			CGAffineTransform cgCTM = CGAffineTransformMake (1.0, 0.0, 0.0, -1.0, 0.0, 0.0);
			CGContextSetTextMatrix (context, cgCTM);
			CGContextScaleCTM (context, 1, -1);
			CGContextClipToRect (context, CGRectMake (0, 0, width, height));
			CGContextSaveGState (context);
			CGRect r = CGRectMake (0, 0, width, height);
			CGContextClearRect (context, r);
		}
	}
	return context;
}
#endif // VSTGUI_USES_COREGRAPHICS

END_NAMESPACE_VSTGUI
