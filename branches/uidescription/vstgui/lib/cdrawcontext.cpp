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

#include "cdrawcontext.h"
#include "cframe.h"
#include "win32support.h"
#include <cmath>

#if GDIPLUS
	#pragma comment( lib, "Gdiplus" )
#endif // GDIPLUS

BEGIN_NAMESPACE_VSTGUI

#if DEBUG
	long gNbCDrawContext = 0;
#endif // DEBUG

#if VSTGUI_USES_COREGRAPHICS
	static inline void QuartzSetLineDash (CGContextRef context, CLineStyle style, CCoord lineWidth);
	static inline void QuartzSetupClip (CGContextRef context, const CRect clipRect);
	static inline double radians (double degrees) { return degrees * M_PI / 180; }
	static void addOvalToPath (CGContextRef c, CPoint center, CGFloat a, CGFloat b, CGFloat start_angle, CGFloat end_angle);

	#if MAC_CARBON
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // we know that we use deprecated functions from Carbon, so we don't want to be warned
	extern bool isWindowComposited (WindowRef window);
	#endif // MAC_CARBON
	#ifndef CGFLOAT_DEFINED
		#define CGFLOAT_DEFINED
		typedef float CGFloat;
	#endif // CGFLOAT_DEFINED

#endif // VSTGUI_USES_COREGRAPHICS

//-----------------------------------------------------------------------------
// CDrawContext Implementation
//-----------------------------------------------------------------------------
/**
 * CDrawContext constructor.
 * @param inFrame the parent CFrame
 * @param inSystemContext the platform system context, can be NULL
 * @param inWindow the platform window object
 */
CDrawContext::CDrawContext (CFrame* inFrame, void* inSystemContext, void* inWindow)
: pSystemContext (inSystemContext)
, pWindow (inWindow)
, pFrame (inFrame)
, font (0)
, frameWidth (0)
, lineStyle (kLineOnOffDash)
, drawMode (kAntialias)
, globalAlpha (1.f)
#if WINDOWS
	#if GDIPLUS
	, pGraphics (0)
	, pPen (0)
	, pBrush (0)
	, pFontBrush (0)
	#else
	, pBrush (0)
	, pPen (0)
	, pOldBrush (0)
	, pOldFont (0)
	, pOldPen (0)
	#endif
#endif
{
	#if DEBUG
	gNbCDrawContext++;
	#endif

	// initialize values
	if (pFrame)
		pFrame->getViewSize (clipRect);
	else
		clipRect (0, 0, 1000, 1000);

	const CColor notInitalized = {0, 0, 0, 0};
	frameColor = notInitalized;
	fillColor  = notInitalized;
	fontColor  = notInitalized;

	// offsets use by offscreen
	offset (0, 0);
	offsetScreen (0, 0);

#if WINDOWS
	#if GDIPLUS
	if (inSystemContext || pWindow)
	{
		HDC hdc = inSystemContext ? (HDC)inSystemContext : GetDC ((HWND)pWindow);
		pGraphics = new Gdiplus::Graphics (hdc);
		pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeLowQuality); // in timo's code this was commented out, why ?
		pGraphics->SetPageUnit(Gdiplus::UnitPixel);
		pGraphics->SetPixelOffsetMode (Gdiplus::PixelOffsetModeNone);
	}
	pPen = new Gdiplus::Pen (Gdiplus::Color (0, 0, 0), 1);
	pBrush = new Gdiplus::SolidBrush (Gdiplus::Color (0, 0, 0));
	pFontBrush = new Gdiplus::SolidBrush (Gdiplus::Color (0, 0, 0));
	#else
	pHDC = 0;
	if (!pSystemContext && pWindow)
		pSystemContext = pHDC = GetDC ((HWND)pWindow);

	if (pSystemContext)
	{
		pOldBrush  = GetCurrentObject ((HDC)pSystemContext, OBJ_BRUSH);
		pOldPen    = GetCurrentObject ((HDC)pSystemContext, OBJ_PEN);
		pOldFont   = GetCurrentObject ((HDC)pSystemContext, OBJ_FONT);
		SetBkMode ((HDC)pSystemContext, TRANSPARENT);
	}
	iPenStyle = PS_SOLID;
	#endif

	// get position 
	if (pWindow)
	{
		RECT  rctTempWnd;
		GetWindowRect ((HWND)pWindow, &rctTempWnd);
		offsetScreen.h = (CCoord)rctTempWnd.left;
		offsetScreen.v = (CCoord)rctTempWnd.top;
	}
#endif // WINDOWS

#if VSTGUI_USES_COREGRAPHICS
	gCGContext = (CGContextRef)pSystemContext;
	if (pSystemContext)
	{
		CGContextSaveGState (gCGContext);
		CGContextSetShouldAntialias (gCGContext, false);
		CGContextSetFillColorSpace (gCGContext, GetGenericRGBColorSpace ());
		CGContextSetStrokeColorSpace (gCGContext, GetGenericRGBColorSpace ()); 
		CGContextSaveGState (gCGContext);
		CGAffineTransform cgCTM = CGAffineTransformMake (1.0, 0.0, 0.0, -1.0, 0.0, 0.0);
		CGContextSetTextMatrix (gCGContext, cgCTM);
	}
#endif // VSTGUI_USES_COREGRAPHICS

#if MAC_CARBON
	if (pFrame && pFrame->getPlatformControl ())
	{
		if (pFrame && (pSystemContext || pWindow))
		{
			HIRect bounds;
			HIViewGetFrame ((HIViewRef)pFrame->getPlatformControl (), &bounds);
			if (pWindow || !pSystemContext)
			{
				if (isWindowComposited ((WindowRef)pWindow))
				{
					HIViewRef contentView;
					HIViewFindByID (HIViewGetRoot ((WindowRef)pWindow), kHIViewWindowContentID, &contentView);
					if (HIViewGetSuperview ((HIViewRef)pFrame->getPlatformControl ()) != contentView)
						HIViewConvertRect (&bounds, (HIViewRef)pFrame->getPlatformControl (), contentView);
					bounds.origin.x += pFrame->hiScrollOffset.x;
					bounds.origin.y += pFrame->hiScrollOffset.y;
				}
			}
			offsetScreen.x = (CCoord)bounds.origin.x;
			offsetScreen.y = (CCoord)bounds.origin.y;
			clipRect (0, 0, (CCoord)bounds.size.width, (CCoord)bounds.size.height);
			clipRect.offset (pFrame->hiScrollOffset.x, pFrame->hiScrollOffset.y);
		}
		if (!pSystemContext && pWindow)
		{
			GrafPtr port = GetWindowPort ((WindowRef)pWindow);
			OSStatus err = QDBeginCGContext (port, &gCGContext);
			if (err == noErr)
			{
				CGContextSaveGState (gCGContext);
				SyncCGContextOriginWithPort (gCGContext, port);
				Rect rect;
				GetPortBounds (port, &rect);
				CGContextTranslateCTM (gCGContext, 0, rect.bottom - rect.top);
				CGContextTranslateCTM (gCGContext, offsetScreen.x, -offsetScreen.y);
				CGContextTranslateCTM (gCGContext, -pFrame->hiScrollOffset.x, pFrame->hiScrollOffset.y);
				CGContextSetShouldAntialias (gCGContext, false);
				CGContextSetFillColorSpace (gCGContext, GetGenericRGBColorSpace ());
				CGContextSetStrokeColorSpace (gCGContext, GetGenericRGBColorSpace ());
				CGContextScaleCTM (gCGContext, 1, -1);
				QuartzSetupClip (gCGContext, clipRect);
				CGContextSaveGState (gCGContext);
				setClipRect (clipRect);
				CGAffineTransform cgCTM = CGAffineTransformMake (1.0, 0.0, 0.0, -1.0, 0.0, 0.0);
				CGContextSetTextMatrix (gCGContext, cgCTM);
			}
		}
	}
	
#endif // MAC_CARBON

	// set the default values
	setFrameColor (kWhiteCColor);
	setLineStyle (kLineSolid);
	setLineWidth (1);
	setFillColor (kBlackCColor);
	setFontColor (kWhiteCColor);
	setFont (kSystemFont);
	setDrawMode (kCopyMode);
}

//-----------------------------------------------------------------------------
CDrawContext::~CDrawContext ()
{
	#if DEBUG
	gNbCDrawContext--;
	#endif

	if (font)
		font->forget ();
#if WINDOWS
	#if GDIPLUS
	if (pFontBrush)
		delete pFontBrush;
	if (pBrush)
		delete pBrush;
	if (pPen)
		delete pPen;
	if (pGraphics)
		delete pGraphics;
	#else
	if (pOldBrush)
		SelectObject ((HDC)pSystemContext, pOldBrush);
	if (pOldPen)
		SelectObject ((HDC)pSystemContext, pOldPen);
	if (pOldFont)
		SelectObject ((HDC)pSystemContext, pOldFont);
	
	if (pBrush)
		DeleteObject (pBrush);
	if (pPen)
		DeleteObject (pPen);
  
	if (pHDC)
	{
		ReleaseDC ((HWND)pWindow, pHDC);
		#if DEBUG
		gNbDC--;
		#endif
	}
	#endif
#elif VSTGUI_USES_COREGRAPHICS
	if (gCGContext)
	{
		CGContextRestoreGState (gCGContext); // restore the original state
		CGContextRestoreGState (gCGContext); // we need to do it twice !!!
		CGContextSynchronize (gCGContext);
		#if MAC_CARBON
		if (!pSystemContext && pWindow)
			QDEndCGContext (GetWindowPort ((WindowRef)pWindow), &gCGContext);
		#endif
	}
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::setGlobalAlpha (float newAlpha)
{
	if (newAlpha == globalAlpha)
		return;

#if WINDOWS && GDIPLUS
	const CColor notInitalized = {0, 0, 0, 0};
	CColor color (frameColor);
	frameColor = notInitalized;
	setFrameColor (color);
	color = fillColor;
	fillColor = notInitalized;
	setFillColor (color);
	color = fontColor;
	fontColor = notInitalized;
	setFontColor (fontColor);

#elif VSTGUI_USES_COREGRAPHICS
	if (gCGContext)
		CGContextSetAlpha (gCGContext, newAlpha);

#endif
	globalAlpha = newAlpha;
}

//-----------------------------------------------------------------------------
void CDrawContext::setLineStyle (CLineStyle style)
{
	if (lineStyle == style)
		return;

	lineStyle = style;

#if WINDOWS
	#if GDIPLUS
	if (pPen)
	{
		switch (lineStyle) 
		{
		case kLineOnOffDash: 
			pPen->SetDashStyle (Gdiplus::DashStyleDot);
			break;
		default:
			pPen->SetDashStyle (Gdiplus::DashStyleSolid);
			break;
		}
	}
	#else
	switch (lineStyle) 
	{
	case kLineOnOffDash: 
		iPenStyle = PS_DOT;
		break;
	default:
		iPenStyle = PS_SOLID;
		break;
	}
	
	LOGPEN logPen = {iPenStyle, {frameWidth, frameWidth}, 
					 RGB (frameColor.red, frameColor.green, frameColor.blue)};
	
	HANDLE newPen = CreatePenIndirect (&logPen);
	SelectObject ((HDC)pSystemContext, newPen);
	if (pPen)
		DeleteObject (pPen);
	pPen = newPen;
	#endif

#elif VSTGUI_USES_COREGRAPHICS
	// nothing to do here
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::setLineWidth (CCoord width)
{
	if (frameWidth == width)
		return;

	frameWidth = width;
	
#if WINDOWS
	#if GDIPLUS
	if (pPen)
		pPen->SetWidth ((float)width);
	#else
	LOGPEN logPen = {iPenStyle, {frameWidth, frameWidth},
					 RGB (frameColor.red, frameColor.green, frameColor.blue)};
	
	HANDLE newPen = CreatePenIndirect (&logPen);
	SelectObject ((HDC)pSystemContext, newPen);
	if (pPen)
		DeleteObject (pPen);
	pPen = newPen;
	#endif

#elif VSTGUI_USES_COREGRAPHICS
	if (gCGContext)
		CGContextSetLineWidth (gCGContext, width);
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::setDrawMode (CDrawMode mode)
{
	if (drawMode == mode)
		return;

	drawMode = mode;

#if WINDOWS
	#if GDIPLUS
	if (pGraphics)
	{
		if (drawMode == kAntialias)
			pGraphics->SetSmoothingMode (Gdiplus::SmoothingModeAntiAlias);
		else
			pGraphics->SetSmoothingMode (Gdiplus::SmoothingModeNone);
	}
	#else
	long iMode = R2_COPYPEN;
	SetROP2 ((HDC)pSystemContext, iMode);
	#endif

#elif VSTGUI_USES_COREGRAPHICS
	// quartz only support antialias
	if (gCGContext)
			CGContextSetShouldAntialias (gCGContext, drawMode == kAntialias ? true : false);
#endif
}

//------------------------------------------------------------------------------
void CDrawContext::setClipRect (const CRect &clip)
{
	CRect _clip (clip);
	_clip.offset (offset.h, offset.v);

	if (clipRect == _clip)
		return;

	clipRect = _clip;

#if WINDOWS
	#if GDIPLUS
	if (pGraphics)
		pGraphics->SetClip (Gdiplus::Rect ((INT)clipRect.left, (INT)clipRect.top, (INT)clipRect.getWidth (), (INT)clipRect.getHeight ()), Gdiplus::CombineModeReplace);
	#else
	RECT r = {clipRect.left, clipRect.top, clipRect.right, clipRect.bottom};
	HRGN hRgn  = CreateRectRgn (r.left, r.top, r.right, r.bottom);
	SelectClipRgn ((HDC)pSystemContext, hRgn);
	DeleteObject (hRgn);
	#endif

#endif
}

//------------------------------------------------------------------------------
void CDrawContext::resetClipRect ()
{
	CRect newClip;
	if (pFrame)
		pFrame->getViewSize (newClip);
	else
		newClip (0, 0, 1000, 1000);

#if WINDOWS
	setClipRect (newClip);

#endif

	clipRect = newClip;
}

//-----------------------------------------------------------------------------
void CDrawContext::moveTo (const CPoint &_point)
{
	CPoint point (_point);
	point.offset (offset.h, offset.v);

	penLoc = point;

#if WINDOWS
	#if GDIPLUS
	#else
	MoveToEx ((HDC)pSystemContext, point.h, point.v, NULL);
	#endif  

#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::lineTo (const CPoint& _point)
{
	CPoint point (_point);
	point.offset (offset.h, offset.v);

#if WINDOWS
	#if GDIPLUS
	if (pGraphics && pPen)
		pGraphics->DrawLine (pPen, (INT)penLoc.h, (INT)penLoc.v, (INT)point.h, (INT)point.v);
	penLoc = point;
	#else
	LineTo ((HDC)pSystemContext, point.h, point.v);
	#endif
	
#elif VSTGUI_USES_COREGRAPHICS
	CGContextRef context = beginCGContext (true);
	if (context)
	{
		QuartzSetLineDash (context, lineStyle, frameWidth);

		if ((((int)frameWidth) % 2))
			CGContextTranslateCTM (gCGContext, 0.5f, -0.5f);

		CGContextBeginPath (context);
		CGContextMoveToPoint (context, penLoc.h, penLoc.v);
		CGContextAddLineToPoint (context, point.h, point.v);
		CGContextDrawPath (context, kCGPathStroke);
		releaseCGContext (context);
	}
	penLoc = point;

#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::drawLines (const CPoint* points, const long& numLines)
{
	#if VSTGUI_USES_COREGRAPHICS
	CGContextRef context = beginCGContext (true);
	if (context) 
	{
		QuartzSetLineDash (context, lineStyle, frameWidth);

		if ((((int)frameWidth) % 2))
			CGContextTranslateCTM (gCGContext, 0.5f, -0.5f);

		CGPoint* cgPoints = new CGPoint[numLines*2];
		for (long i = 0; i < numLines * 2; i += 2)
		{
			cgPoints[i].x = points[i].x + offset.x;
			cgPoints[i+1].x = points[i+1].x + offset.x;
			cgPoints[i].y = points[i].y + offset.y;
			cgPoints[i+1].y = points[i+1].y + offset.y;
		}
		CGContextStrokeLineSegments (context, cgPoints, numLines*2);
		delete [] cgPoints;

		releaseCGContext (context);
	}

	#elif 0 //(WINDOWS && GDIPLUS)
	// Graphics::DrawLines does other things than the quartz one, so we can not use it
	if (pGraphics)
	{
		#if VSTGUI_FLOAT_COORDINATES
		Gdiplus::PointF* myPoints = new Gdiplus::PointF[numLines];
		#else
		Gdiplus::Point* myPoints = new Gdiplus::Point[numLines];
		#endif
		for (long i = 0; i < numLines; i++)
		{
			myPoints[i].X = points[i].x + offset.x;
			myPoints[i].Y = points[i].y + offset.y;
		}
		pGraphics->DrawLines(pPen, myPoints, numLines);
	}
	#else
	// default implementation, when no platform optimized code is implemented
	for (long i = 0; i < numLines * 2; i+=2)
	{
		moveTo (points[i]);
		lineTo (points[i+1]);
	}
	#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::drawPolygon (const CPoint* pPoints, long numberOfPoints, const CDrawStyle drawStyle)
{
#if VSTGUI_USES_COREGRAPHICS
	CGContextRef context = beginCGContext (true);
	{
		CGPathDrawingMode m;
		switch (drawStyle)
		{
			case kDrawFilled : m = kCGPathFill; break;
			case kDrawFilledAndStroked : m = kCGPathFillStroke; break;
			default : m = kCGPathStroke; break;
		}

		QuartzSetLineDash (context, lineStyle, frameWidth);

		CGContextBeginPath (context);
		CGContextMoveToPoint (context, pPoints[0].h + offset.h, pPoints[0].v + offset.v);
		for (long i = 1; i < numberOfPoints; i++)
			CGContextAddLineToPoint (context, pPoints[i].h + offset.h, pPoints[i].v + offset.v);
		CGContextDrawPath (context, m);
		releaseCGContext (context);
	}

#elif WINDOWS
	#if GDIPLUS
	Gdiplus::PointF points[30];
	Gdiplus::PointF* polyPoints;
	bool allocated = false;
	if (numberOfPoints > 30)
	{
		polyPoints = new Gdiplus::PointF[numberOfPoints];
		allocated = true;
	}
	else
		polyPoints = points;
	
	for (long i = 0; i < numberOfPoints; i++)
	{
		polyPoints[i].X = (Gdiplus::REAL)(pPoints[i].h + offset.h);
		polyPoints[i].Y = (Gdiplus::REAL)(pPoints[i].v + offset.v);
	}

	if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
		pGraphics->FillPolygon (pBrush, polyPoints, numberOfPoints);
	if (drawStyle == kDrawFilledAndStroked || drawStyle == kDrawStroked)
		pGraphics->DrawPolygon (pPen, polyPoints, numberOfPoints);


	if (allocated)
		delete[] polyPoints;

	#else
	POINT points[30];
	POINT* polyPoints;
	bool allocated = false;

	if (numberOfPoints > 30)
	{
		polyPoints = (POINT*)new char [numberOfPoints * sizeof (POINT)];
		if (!polyPoints)
			return;
		allocated = true;
	}
	else
		polyPoints = points;

	for (long i = 0; i < numberOfPoints; i++)
	{
		polyPoints[i].x = pPoints[i].h + offset.h;
		polyPoints[i].y = pPoints[i].v + offset.v;
	}

	if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
	{
		HANDLE nullPen = GetStockObject (NULL_PEN);
		HANDLE oldPen  = SelectObject ((HDC)pSystemContext, nullPen);
		Polygon ((HDC)pSystemContext, polyPoints, numberOfPoints);
		SelectObject ((HDC)pSystemContext, oldPen);
	}
	if (drawStyle == kDrawFilledAndStroked || drawStyle == kDrawStroked)
		Polyline ((HDC)pSystemContext, polyPoints, numberOfPoints);
	
	if (allocated)
		delete[] polyPoints;
	#endif


#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::drawRect (const CRect &_rect, const CDrawStyle drawStyle)
{
	CRect rect (_rect);
	rect.offset (offset.h, offset.v);

#if WINDOWS
	#if GDIPLUS
	if (pGraphics)
	{
		rect.normalize ();
		if (pBrush && (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked))
		{
			Gdiplus::RectF r ((Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.getWidth (), (Gdiplus::REAL)rect.getHeight ());
			pGraphics->FillRectangle (pBrush, r);
		}
		if (pPen && (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked))
		{
			Gdiplus::RectF r ((Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.getWidth ()-1, (Gdiplus::REAL)rect.getHeight ()-1);
			pGraphics->DrawRectangle (pPen, r);
		}
	}
	#else
	if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
	{
		RECT wr = {rect.left, rect.top, rect.right, rect.bottom};
		HANDLE nullPen = GetStockObject (NULL_PEN);
		HANDLE oldPen  = SelectObject ((HDC)pSystemContext, nullPen);
		FillRect ((HDC)pSystemContext, &wr, (HBRUSH)pBrush);
		SelectObject ((HDC)pSystemContext, oldPen);
	}
	if (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked)
	{
		MoveToEx ((HDC)pSystemContext, rect.left, rect.top, NULL);
		LineTo ((HDC)pSystemContext, rect.right-1, rect.top);
		LineTo ((HDC)pSystemContext, rect.right-1, rect.bottom-1);
		LineTo ((HDC)pSystemContext, rect.left, rect.bottom-1);
		LineTo ((HDC)pSystemContext, rect.left, rect.top);
	}
	#endif

#elif VSTGUI_USES_COREGRAPHICS
	CGContextRef context = beginCGContext (true);
	{
		CGPathDrawingMode m;
		switch (drawStyle)
		{
			case kDrawFilled : m = kCGPathFill; break;
			case kDrawFilledAndStroked : m = kCGPathFillStroke; break;
			default : m = kCGPathStroke; break;
		}

		CGRect r = CGRectMake (rect.left, rect.top+1, rect.width () - 1, rect.height () - 1);

		QuartzSetLineDash (context, lineStyle, frameWidth);

		if ((((int)frameWidth) % 2))
			CGContextTranslateCTM (gCGContext, 0.5f, -0.5f);

		CGContextBeginPath (context);
		CGContextMoveToPoint (context, r.origin.x, r.origin.y);
		CGContextAddLineToPoint (context, r.origin.x + r.size.width, r.origin.y);
		CGContextAddLineToPoint (context, r.origin.x + r.size.width, r.origin.y + r.size.height);
		CGContextAddLineToPoint (context, r.origin.x, r.origin.y + r.size.height);
		CGContextClosePath (context);

		CGContextDrawPath (context, m);

		releaseCGContext (context);
	}

#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::drawEllipse (const CRect &_rect, const CDrawStyle drawStyle)
{
	CRect rect (_rect);
	rect.offset (offset.h, offset.v);

	#if VSTGUI_USES_COREGRAPHICS
	CGContextRef context = beginCGContext (true);
	{
		CGPathDrawingMode m;
		switch (drawStyle)
		{
			case kDrawFilled : m = kCGPathFill; break;
			case kDrawFilledAndStroked : m = kCGPathFillStroke; break;
			default : m = kCGPathStroke; break;
		}
		if (rect.width () != rect.height ())
		{
			CGContextSaveGState (context);

			QuartzSetLineDash (context, lineStyle, frameWidth);

			CGContextBeginPath (context);

			CGRect cgRect = CGRectMake (rect.left, rect.top, rect.width (), rect.height ());
			CGPoint center = CGPointMake (CGRectGetMidX (cgRect), CGRectGetMidY (cgRect));
			CGFloat a = CGRectGetWidth (cgRect) / 2.;
			CGFloat b = CGRectGetHeight (cgRect) / 2.;

		    CGContextTranslateCTM (context, center.x, center.y);
		    CGContextScaleCTM (context, a, b);
		    CGContextMoveToPoint (context, 1, 0);
		    CGContextAddArc (context, 0, 0, 1, radians (0), radians (360), 0);

			CGContextClosePath (context);
			CGContextRestoreGState (context);
			CGContextDrawPath (context, m);
		}
		else
		{
			CGFloat radius = rect.width () * 0.5;
			CGContextBeginPath (context);
			CGContextAddArc (context, rect.left + radius, rect.top + radius, radius, radians (0), radians (360), 0);
			CGContextClosePath (context);
			CGContextDrawPath (context, m);
		}
		releaseCGContext (context);
	}
	#elif GDIPLUS

	if (pGraphics)
	{
		rect.normalize ();
		if (pBrush && (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked))
		{
			pGraphics->FillEllipse (pBrush, (INT)rect.left, (INT)rect.top, (INT)rect.getWidth ()-1, (INT)rect.getHeight ()-1);
		}
		if (pPen && (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked))
		{
			pGraphics->DrawEllipse (pPen, (INT)rect.left, (INT)rect.top, (INT)rect.getWidth ()-1, (INT)rect.getHeight ()-1);
		}
	}

	#else
	CPoint point (_rect.left + (_rect.right - _rect.left) / 2, _rect.top);
	drawArc (_rect, point, point);

	#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::drawPoint (const CPoint &_point, CColor color)
{
	CPoint point (_point);

#if WINDOWS && !GDIPLUS
	point.offset (offset.h, offset.v);
	SetPixel ((HDC)pSystemContext, point.h, point.v, RGB(color.red, color.green, color.blue));

#else
	CCoord oldframeWidth = frameWidth;
	CColor oldframecolor = frameColor;
	setLineWidth (1);
	setFrameColor (color);
	CPoint point2 (point);
	point2.h++;
	moveTo (point);
	lineTo (point2);
	
	setFrameColor (oldframecolor);
	setLineWidth (oldframeWidth);

#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::drawArc (const CRect &_rect, const float _startAngle, const float _endAngle, const CDrawStyle drawStyle) // in degree
{
	CRect rect (_rect);
	rect.offset (offset.h, offset.v);

#if WINDOWS
	#if GDIPLUS
	if (pGraphics)
	{
		float endAngle = _endAngle;
		if (endAngle < _startAngle)
			endAngle += 360.f;
		endAngle = fabs (endAngle - _startAngle);
		Gdiplus::RectF r ((float)rect.left, (float)rect.top, (float)rect.getWidth (), (float)rect.getHeight ());
		Gdiplus::GraphicsPath path;
		path.AddArc (r, _startAngle, endAngle);
		if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
			pGraphics->FillPath (pBrush, &path);
		if (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked)
			pGraphics->DrawPath (pPen, &path);
	}
	#else
	float startRad = (float)(k2PI * _startAngle / 360.f);
	float endRad   = (float)(k2PI * _endAngle / 360.f);
	
	CPoint point1, point2;
	long midX = _rect.width () / 2;
	long midY = _rect.height () / 2;

	point1.x = (long)(midX + midX * cosf (startRad));
	point1.y = (long)(midY - midY * sinf (startRad));
	point2.x = (long)(midX + midX * cosf (endRad));
	point2.y = (long)(midY - midY * sinf (endRad));
	point1.offset (offset.h, offset.v);
	point2.offset (offset.h, offset.v);

	if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
	{
		HANDLE nullPen = GetStockObject (NULL_PEN);
		HANDLE oldPen  = SelectObject ((HDC)pSystemContext, nullPen);
		Pie ((HDC)pSystemContext, rect.left, rect.top, rect.right + 1, rect.bottom + 1, 
				 point1.h, point1.v, point2.h, point2.v);
		SelectObject ((HDC)pSystemContext, oldPen);
	}
	if (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked)
	{
		Arc ((HDC)pSystemContext, rect.left, rect.top, rect.right + 1, rect.bottom + 1, 
				 point1.h, point1.v, point2.h, point2.v);
	}
	#endif

#elif VSTGUI_USES_COREGRAPHICS

	CGContextRef context = beginCGContext (true);
	{
		CGPathDrawingMode m;
		switch (drawStyle)
		{
			case kDrawFilled : m = kCGPathFill; break;
			case kDrawFilledAndStroked : m = kCGPathFillStroke; break;
			default : m = kCGPathStroke; break;
		}
		QuartzSetLineDash (context, lineStyle, frameWidth);

		CGContextBeginPath (context);
		addOvalToPath (context, CPoint (rect.left + rect.width () / 2, rect.top + rect.height () / 2), rect.width () / 2, rect.height () / 2, -_startAngle, -_endAngle);
		if (drawStyle == kDrawFilled || kDrawFilledAndStroked)
			CGContextAddLineToPoint (context, rect.left + rect.width () / 2, rect.top + rect.height () / 2);
		CGContextDrawPath (context, m);
		releaseCGContext (context);
	}

#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::setFontColor (const CColor color)
{
	fontColor = color;
	
#if WINDOWS
	#if GDIPLUS
	if (pFontBrush)
		pFontBrush->SetColor (Gdiplus::Color ((BYTE)((float)color.alpha * globalAlpha), color.red, color.green, color.blue));
	#else
	SetTextColor ((HDC)pSystemContext, RGB (fontColor.red, fontColor.green, fontColor.blue));
	#endif

#elif MAC
	// on quartz the fill color is the font color

#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::setFrameColor (const CColor color)
{
	if (frameColor == color)
		return;
		
	frameColor = color;

#if WINDOWS
	#if GDIPLUS
	if (pPen)
		pPen->SetColor (Gdiplus::Color ((BYTE)((float)color.alpha * globalAlpha), color.red, color.green, color.blue));
	#else
	LOGPEN logPen = {iPenStyle, {frameWidth, frameWidth}, 
					 RGB (frameColor.red, frameColor.green, frameColor.blue)};
	
	HANDLE newPen = CreatePenIndirect (&logPen);
	SelectObject ((HDC)pSystemContext, newPen);
	if (pPen)
		DeleteObject (pPen);
	pPen = newPen;
	#endif

#elif VSTGUI_USES_COREGRAPHICS
	if (gCGContext)
		CGContextSetRGBStrokeColor (gCGContext, color.red/255.f, color.green/255.f, color.blue/255.f, color.alpha/255.f);
        
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::setFillColor (const CColor color)
{
	if (fillColor == color)
		return;

	fillColor = color;

#if WINDOWS
	#if GDIPLUS
	if (pBrush)
		pBrush->SetColor (Gdiplus::Color ((BYTE)((float)color.alpha * globalAlpha), color.red, color.green, color.blue));
	#else
 	SetBkColor ((HDC)pSystemContext, RGB (color.red, color.green, color.blue));
	LOGBRUSH logBrush = {BS_SOLID, RGB (color.red, color.green, color.blue), 0 };
	HANDLE newBrush = CreateBrushIndirect (&logBrush);
	if (newBrush == 0)
	{
		DWORD err = GetLastError ();
		return;
	}
	SelectObject ((HDC)pSystemContext, newBrush);
	if (pBrush)
		DeleteObject (pBrush);
	pBrush = newBrush;
	#endif
	
#elif VSTGUI_USES_COREGRAPHICS
	if (gCGContext)
		CGContextSetRGBFillColor (gCGContext, color.red/255.f, color.green/255.f, color.blue/255.f, color.alpha/255.f);

#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::setFont (const CFontRef newFont, const long& size, const long& style)
{
	if (newFont == 0)
		return;
	if (font)
		font->forget ();
	if ((size > 0 && newFont->getSize () != size) || (style != -1 && newFont->getStyle () != style))
	{
		font = (CFontRef)newFont->newCopy ();
		if (size > 0)
			font->setSize (size);
		if (style != -1)
			font->setStyle (style);
	}
	else
	{
		font = newFont;
		font->remember ();
	}
}

//------------------------------------------------------------------------------
CCoord CDrawContext::getStringWidth (const char* pStr)
{
	return getStringWidthUTF8 (pStr);
}

//-----------------------------------------------------------------------------
void CDrawContext::drawString (const char* string, const CRect &_rect,
							 const short opaque, const CHoriTxtAlign hAlign)
{
	if (!string)
		return;
	
	drawStringUTF8 (string, _rect, hAlign);

}

//-----------------------------------------------------------------------------
CCoord CDrawContext::getStringWidthUTF8 (const char* string)
{
	CCoord result = -1;
	if (font == 0 || string == 0)
		return result;

	IFontPainter* painter = font->getFontPainter ();
	if (painter)
		result = painter->getStringWidth (this, string, true);

	return result;
}

//-----------------------------------------------------------------------------
void CDrawContext::drawStringUTF8 (const char* string, const CPoint& _point, bool antialias)
{
	if (string == 0 || font == 0)
		return;

	CPoint point (_point);
	point.offset (offset.h, offset.v);

	IFontPainter* painter = font->getFontPainter ();
	if (painter)
		painter->drawString (this, string, point, antialias);

}

//-----------------------------------------------------------------------------
void CDrawContext::drawStringUTF8 (const char* string, const CRect& _rect, const CHoriTxtAlign hAlign, bool antialias)
{
	if (!string)
		return;
	
	CRect rect (_rect);

	double capHeight = -1;
	CPlatformFont* platformFont = font->getPlatformFont ();
	if (platformFont)
		capHeight = platformFont->getCapHeight ();
	
	if (capHeight > 0.)
		rect.bottom -= (rect.height ()/2 - capHeight / 2);
	else
		rect.bottom -= (rect.height ()/2 - font->getSize () / 2) + 1;
	if (hAlign != kLeftText)
	{
		CCoord stringWidth = getStringWidthUTF8 (string);
		if (hAlign == kRightText)
			rect.left = rect.right - stringWidth;
		else
			rect.left = (CCoord)(rect.left + (rect.getWidth () / 2.f) - (stringWidth / 2.f));
	}
	CRect oldClip;
	getClipRect (oldClip);
	CRect newClip (_rect);
	newClip.bound (oldClip);
	setClipRect (newClip);
	drawStringUTF8 (string, CPoint (rect.left, rect.bottom), antialias);
	setClipRect (oldClip);
}

//-----------------------------------------------------------------------------
#if VSTGUI_USES_COREGRAPHICS
//-----------------------------------------------------------------------------
CGContextRef CDrawContext::beginCGContext (bool swapYAxis)
{
	if (gCGContext)
	{
		CGContextSaveGState (gCGContext);
		QuartzSetupClip (gCGContext, clipRect);
		if (!swapYAxis)
			CGContextScaleCTM (gCGContext, 1, -1);
		return gCGContext;
	}
	return 0;
}

//-----------------------------------------------------------------------------
void CDrawContext::releaseCGContext (CGContextRef context)
{
	if (context)
	{
		CGContextRestoreGState (context);
	}
}

//-----------------------------------------------------------------------------
CGImageRef CDrawContext::getCGImage () const
{
	// only for subclasses
	return 0;
}

//-----------------------------------------------------------------------------
void QuartzSetupClip (CGContextRef context, const CRect clipRect)
{
	CGRect cgClipRect = CGRectMake (clipRect.left, clipRect.top, clipRect.width (), clipRect.height ());
	CGContextClipToRect (context, cgClipRect);
}

//-----------------------------------------------------------------------------
void QuartzSetLineDash (CGContextRef context, CLineStyle style, CCoord lineWidth)
{
	if (style == kLineOnOffDash)
	{
		CGFloat offset = 0;
		CGFloat dotf[2] = { lineWidth, lineWidth };
		CGContextSetLineDash (context, offset, dotf, 2);
	}
}

//-----------------------------------------------------------------------------
static void addOvalToPath (CGContextRef c, CPoint center, CGFloat a, CGFloat b, CGFloat start_angle, CGFloat end_angle)
{
	CGContextSaveGState (c);
	CGContextTranslateCTM (c, center.x, center.y);
	CGContextScaleCTM (c, a, b);
	CGContextRotateCTM (c, radians (-90.f));

	CGContextMoveToPoint (c, cos (radians (start_angle)), sin (radians (start_angle)));

	CGContextAddArc(c, 0, 0, 1, radians (start_angle), radians (end_angle), 1);

	CGContextRestoreGState(c);
}

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

#endif // VSTGUI_USES_COREGRAPHICS

END_NAMESPACE_VSTGUI
