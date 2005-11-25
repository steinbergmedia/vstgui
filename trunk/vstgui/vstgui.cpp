//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 3.5       $Date: 2005-11-25 16:40:52 $ 
//
// Added Motif/Windows vers.: Yvan Grabit              01.98
// Added Mac version        : Charlie Steinberg        02.98
// Added BeOS version       : Georges-Edouard Berenger 05.99
// Added new functions      : Matthias Juwan           12.01
// Added MacOSX version     : Arne Scheffler           02.03
// Added Quartz stuff		: Arne Scheffler           08.03
// Added Win Alpha Blending : Arne Scheffler           04.04
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// © 2004, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __vstgui__
#include "vstgui.h"
#endif

#if ENABLE_VST_EXTENSION_IN_VSTGUI
#include "audioeffectx.h"
#include "aeffguieditor.h"
#endif

#include "vstkeycode.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//---Some defines-------------------------------------
#define USE_ALPHA_BLEND			QUARTZ || USE_LIBPNG
#define MAC_OLD_DRAG			1

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#if USE_NAMESPACE
#define VSTGUI_CFrame				VSTGUI::CFrame
#define VSTGUI_CPoint				VSTGUI::CPoint
#define VSTGUI_CTextEdit			VSTGUI::CTextEdit
#define VSTGUI_CColor				VSTGUI::CColor
#define VSTGUI_CDrawContext			VSTGUI::CDrawContext
#define VSTGUI_COffscreenContext	VSTGUI::COffscreenContext
#define VSTGUI_COptionMenu			VSTGUI::COptionMenu
#define VSTGUI_COptionMenuScheme	VSTGUI::COptionMenuScheme
#define VSTGUI_CDragContainer		VSTGUI::CDragContainer
#else
#define VSTGUI_CFrame     CFrame
#define VSTGUI_CPoint     CPoint
#define VSTGUI_CTextEdit  CTextEdit
#define VSTGUI_CColor     CColor
#define VSTGUI_CDrawContext CDrawContext
#define VSTGUI_COffscreenContext COffscreenContext
#define VSTGUI_COptionMenu COptionMenu
#define VSTGUI_COptionMenuScheme COptionMenuScheme
#define VSTGUI_CDragContainer	CDragContainer
#endif

static VSTGUI_CDragContainer* gDragContainer = 0;

//---For Debugging------------------------
#if DEBUG

long gNbCBitmap = 0;
long gNbCView = 0;
long gNbCDrawContext = 0;
long gNbCOffscreenContext = 0;
long gBitmapAllocation = 0;
long gNbDC = 0;

#include <stdarg.h>

void DebugPrint (char *format, ...);
void DebugPrint (char *format, ...)
{
	char string[300];
	va_list marker;
	va_start (marker, format);
	vsprintf (string, format, marker);
	if (!string)
		strcpy (string, "Empty string\n");
	#if WINDOWS
	OutputDebugString (string);
	#elif MAC && !MACX
	Str255 pStr;
	c2pstrcpy (pStr, string);
	DebugStr (pStr);
	#else
	fprintf (stderr, string);
	#endif
}
#endif
//---End For Debugging------------------------

#if WINDOWS
static bool bSwapped_mouse_buttons = false; 
OSVERSIONINFOEX	gSystemVersion;

// Alpha blending for Windows using library : msimg32.dll
#define DYNAMICALPHABLEND   !GDIPLUS

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <zmouse.h>
#include <commdlg.h>

#if DYNAMICALPHABLEND
typedef  BOOL (WINAPI *PFNALPHABLEND)(
  HDC hdcDest,                 // handle to destination DC
  int nXOriginDest,            // x-coord of upper-left corner
  int nYOriginDest,            // y-coord of upper-left corner
  int nWidthDest,              // destination width
  int nHeightDest,             // destination height
  HDC hdcSrc,                  // handle to source DC
  int nXOriginSrc,             // x-coord of upper-left corner
  int nYOriginSrc,             // y-coord of upper-left corner
  int nWidthSrc,               // source width
  int nHeightSrc,              // source height
  BLENDFUNCTION blendFunction  // alpha-blending function
);

PFNALPHABLEND pfnAlphaBlend = NULL;

typedef	BOOL (WINAPI *PFNTRANSPARENTBLT)(
  HDC hdcDest,        // handle to destination DC
  int nXOriginDest,   // x-coord of destination upper-left corner
  int nYOriginDest,   // y-coord of destination upper-left corner
  int nWidthDest,     // width of destination rectangle
  int hHeightDest,    // height of destination rectangle
  HDC hdcSrc,         // handle to source DC
  int nXOriginSrc,    // x-coord of source upper-left corner
  int nYOriginSrc,    // y-coord of source upper-left corner
  int nWidthSrc,      // width of source rectangle
  int nHeightSrc,     // height of source rectangle
  UINT crTransparent  // color to make transparent
);

PFNTRANSPARENTBLT	pfnTransparentBlt = NULL;
#endif

extern void* hInstance;
inline HINSTANCE GetInstance () { return (HINSTANCE)hInstance; }

static long   gUseCount = 0;
static char   gClassName[20];
static bool   InitWindowClass ();
static void   ExitWindowClass ();
LONG_PTR WINAPI WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

static HANDLE CreateMaskBitmap (CDrawContext* pContext, CRect& rect, CColor transparentColor);
static void   DrawTransparent (CDrawContext* pContext, CRect& rect, const CPoint& offset, HDC hdcBitmap, POINT ptSize, HBITMAP pMask, COLORREF color);
static bool   checkResolveLink (const char* nativePath, char* resolved);
static void   *createDropTarget (VSTGUI_CFrame* pFrame);

BEGIN_NAMESPACE_VSTGUI
long        gStandardFontSize[] = { 12, 18, 14, 12, 11, 10, 9, 13 };
const char* gStandardFontName[] = {
	"Arial", "Arial", "Arial", 
	"Arial", "Arial", "Arial", 
	"Arial", "Symbol" };
END_NAMESPACE_VSTGUI

#if USE_LIBPNG
#include "png.h"
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#elif MAC
BEGIN_NAMESPACE_VSTGUI

long pSystemVersion;

#if MACX
//-----------------------------------------------------------------------------
#include <QuickTime/QuickTime.h>
#include <CoreServices/CoreServices.h>

#if QUARTZ
const char* gMacXfontNames[] = {
	"Lucida Grande",
	"Helvetica",
	"Helvetica",
	"Helvetica",
	"Helvetica",
	"Helvetica",
	"Helvetica",
	"Symbol"
};

#ifndef M_PI
#define	M_PI		3.14159265358979323846	/* pi */
#endif

bool isWindowComposited (WindowRef window);
static inline void QuartzSetLineDash (CGContextRef context, CLineStyle style, CCoord lineWidth);
static inline void QuartzSetupClip (CGContextRef context, const CRect clipRect);
static inline ATSUStyle CreateATSUStyle (const CColor &fontColor, CFont fontID, const long size, long style, const char* name = 0);

static inline double radians (double degrees) { return degrees * M_PI / 180; }
CGColorSpaceRef GetGenericRGBColorSpace ();

typedef void (*CGContextStrokeLineSegmentsProc) (CGContextRef c, const CGPoint points[], size_t count);
typedef CGImageRef (*CGImageCreateWithImageInRectProc) (CGImageRef image, CGRect rect);
static CGImageCreateWithImageInRectProc _CGImageCreateWithImageInRect = NULL;
static CGContextStrokeLineSegmentsProc _CGContextStrokeLineSegments = NULL;

// cache graphics importer
static ComponentInstance bmpGI = 0;
static ComponentInstance pngGI = 0;
static ComponentInstance jpgGI = 0;
static ComponentInstance pictGI = 0;


#else
const unsigned char* gMacXfontNames[] = {
	"\pArial",
	"\pArial",
	"\pArial",
	"\pArial",
	"\pArial",
	"\pArial",
	"\pArial",
	"\pSymbol"
};
#endif

//-----------------------------------------------------------------------------
#else
#include <QDOffscreen.h>
#include <StandardFile.h>
#include <Navigation.h>
#include <PictUtils.h>
#endif

long gStandardFontSize[] = { 12, 18, 14, 12, 11,10, 9, 12 };

long convertPoint2Angle (CPoint &pm, CPoint &pt);
void RectNormalize (Rect& rect);
void CRect2Rect (const CRect &cr, Rect &rr);
void Rect2CRect (Rect &rr, CRect &cr);
void CColor2RGBColor (const CColor &cc, RGBColor &rgb);
void RGBColor2CColor (const RGBColor &rgb, CColor &cc);

#if MAC_OLD_DRAG
static void install_drop (CFrame *frame);
static void remove_drop (CFrame *frame);
#endif

//-----------------------------------------------------------------------------
void RectNormalize (Rect& rect)
{
	if (rect.left > rect.right)
	{
		long temp = rect.right;
		rect.right = rect.left;
		rect.left = temp;
	}
	if (rect.top > rect.bottom)
	{
		long temp = rect.bottom;
		rect.bottom = rect.top;
		rect.top = temp;
	}
}

//-----------------------------------------------------------------------------
void CRect2Rect (const CRect &cr, Rect &rr)
{
	rr.left   = (short)cr.left;
	rr.right  = (short)cr.right;
	rr.top    = (short)cr.top;
	rr.bottom = (short)cr.bottom;
	RectNormalize (rr);
}

//-----------------------------------------------------------------------------
void Rect2CRect (Rect &rr, CRect &cr)
{
	cr.left   = rr.left;
	cr.right  = rr.right;
	cr.top    = rr.top;
	cr.bottom = rr.bottom;
}

//-----------------------------------------------------------------------------
void CColor2RGBColor (const CColor &cc, RGBColor &rgb)
{
	rgb.red   = cc.red   * 257;
	rgb.green = cc.green * 257;
	rgb.blue  = cc.blue  * 257;
}

//-----------------------------------------------------------------------------
void RGBColor2CColor (const RGBColor &rgb, CColor &cc)
{
	cc.red   = rgb.red   / 257;
	cc.green = rgb.green / 257;
	cc.blue  = rgb.blue  / 257;
}

END_NAMESPACE_VSTGUI
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#elif BEOS

#include <TranslationUtils.h>
#include <Resources.h>
#include <Bitmap.h>
#include <Region.h>
#include <View.h>
#include <Window.h>
#include <Message.h>
#include <Entry.h>
#include <Path.h>

//--------------------------
class PlugView: public BView
{
public:
	PlugView (BRect frame, CFrame* cframe);
	void Draw (BRect updateRect);
	void MouseDown (BPoint where);
	void MessageReceived (BMessage *msg);
private:
	CFrame*	cframe;
};

long convertPoint2Angle (CPoint &pm, CPoint &pt);

drawing_mode modeToPlatform [] = {
//  kCopyMode  kOrMode    kXorMode
	B_OP_COPY, B_OP_OVER, B_OP_INVERT
};

long gStandardFontSize[] = { 12, 18, 14, 12, 11, 10, 9, 12 };
const char*	standardFont  = "Swis721 BT";
const char* standardFontS = "Roman";
const char* systemFont    = "Swis721 BT";
const char* systemFontS   = "Bold";
const char* gStandardFontName[] = { systemFont,
	standardFont, standardFont, standardFont, standardFont, standardFont,
	standardFont };
const char* gStandardFontStyle[] = { systemFontS,
	standardFontS, standardFontS, standardFontS, standardFontS, standardFontS,
	standardFontS };
#endif

//-----------------------------------------------------------------------------
bool CRect::pointInside (const CPoint& where) const
{
	return where.h >= left && where.h < right && where.v >= top && where.v < bottom;
}

//-----------------------------------------------------------------------------
bool CRect::isEmpty () const 
{
	if (right <= left)
		return true;
	if (bottom <= top)
		return true;
	return false;
}

//-----------------------------------------------------------------------------
void CRect::bound (const CRect& rect)
{
	if (left < rect.left)
		left = rect.left;
	if (top < rect.top)
		top = rect.top;
	if (right > rect.right)
		right = rect.right;
	if (bottom > rect.bottom)
		bottom = rect.bottom;
	if (bottom < top)
		bottom = top;
	if (right < left)
		right = left;
}

BEGIN_NAMESPACE_VSTGUI

CColor kTransparentCColor = {255, 255, 255, 0};
CColor kBlackCColor  = {0,     0,   0, 255};
CColor kWhiteCColor  = {255, 255, 255, 255};
CColor kGreyCColor   = {127, 127, 127, 255};
CColor kRedCColor    = {255,   0,   0, 255};
CColor kGreenCColor  = {0  , 255,   0, 255};
CColor kBlueCColor   = {0  ,   0, 255, 255};
CColor kYellowCColor = {255, 255,   0, 255};
CColor kMagentaCColor= {255,   0, 255, 255};
CColor kCyanCColor   = {0  , 255, 255, 255};

#define kDragDelay 0

//-----------------------------------------------------------------------------
// CDrawContext Implementation
//-----------------------------------------------------------------------------
/**
 * CDrawContext constructor.
 * @param inFrame the parent CFrame
 * @param inSystemContext the platform system context, can be NULL
 * @param inWindow the platform window object
 */
CDrawContext::CDrawContext (CFrame *inFrame, void *inSystemContext, void *inWindow)
: pSystemContext (inSystemContext)
, pWindow (inWindow)
, pFrame (inFrame)
, fontSize (-1)
, fontStyle (0)
, fontId (kNumStandardFonts)
, frameWidth (0)
, lineStyle (kLineOnOffDash)
, drawMode (kAntialias)
#if WINDOWS
	#if GDIPLUS
	#else
	, pBrush (0), pFont (0), pPen (0)
	, pOldBrush (0), pOldFont (0), pOldPen (0)
	#endif
#elif MAC && !QUARTZ
, bInitialized (false)
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
	pGraphics = 0;
	if (inSystemContext || pWindow)
	{
		HDC hdc = inSystemContext ? (HDC)inSystemContext : GetDC ((HWND)pWindow);
		pGraphics = new Gdiplus::Graphics (hdc);
		pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeLowQuality);
	}
	pPen = new Gdiplus::Pen (Gdiplus::Color (0, 0, 0), 1);
	pBrush = new Gdiplus::SolidBrush (Gdiplus::Color (0, 0, 0));
	pFontBrush = new Gdiplus::SolidBrush (Gdiplus::Color (0, 0, 0));
	pFont = 0;
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
		offsetScreen.h = rctTempWnd.left;
		offsetScreen.v = rctTempWnd.top;
	}

#elif MAC
	#if QUARTZ
	atsuStyle = 0;
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
	gCGContext = 0;
	if (pSystemContext)
	{
		gCGContext = (CGContextRef) pSystemContext;
		CGContextSaveGState (gCGContext); // save the original state
		CGContextScaleCTM (gCGContext, 1, -1);
		CGContextSetShouldAntialias (gCGContext, false);
		CGContextSetFillColorSpace (gCGContext, GetGenericRGBColorSpace ());
		CGContextSetStrokeColorSpace (gCGContext, GetGenericRGBColorSpace ()); 
		CGContextSaveGState (gCGContext);
		setClipRect (clipRect);
		if (pFrame)
			pFrame->setDrawContext (this);
	}
	else if (pWindow)
	{
		GrafPtr port = GetWindowPort ((WindowRef)pWindow);
		OSStatus err = QDBeginCGContext (port, &gCGContext);
		if (err == noErr)
		{
			CGContextSaveGState (gCGContext); // save the original state
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
			CGContextScaleCTM (gCGContext, 1, -1);
			CGContextSaveGState (gCGContext);
			setClipRect (clipRect);
			if (pFrame)
				pFrame->setDrawContext (this);
		}
	}
	if (gCGContext)
	{
		CGAffineTransform cgCTM = CGAffineTransformMake (1.0, 0.0, 0.0, -1.0, 0.0, 0.0);
		CGContextSetTextMatrix (gCGContext, cgCTM);
	}
	needToSynchronizeCGContext = false;

	#else
	pSystemContext = pWindow;

	#endif
	
#elif BEOS
	pView = (BView*) pSystemContext;
	if (pView)
		pView->LockLooper ();

#endif

	if (1 || pSystemContext)
	{
		// set the default values
		setFrameColor (kWhiteCColor);
		setLineStyle (kLineSolid);
		setLineWidth (1);
		setFillColor (kBlackCColor);
		setFontColor (kWhiteCColor);
		setFont (kSystemFont);
		setDrawMode (kCopyMode);
	}
}

//-----------------------------------------------------------------------------
CDrawContext::~CDrawContext ()
{
	#if DEBUG
	gNbCDrawContext--;
	#endif

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
	if (pFont)
		DeleteObject (pFont);
  
	if (pHDC)
	{
		ReleaseDC ((HWND)pWindow, pHDC);
		#if DEBUG
		gNbDC--;
		#endif
	}
	#endif
#elif (MAC && QUARTZ)
	if (gCGContext)
	{
		CGContextRestoreGState (gCGContext); // restore the original state
		CGContextRestoreGState (gCGContext); // we need to do it twice !!!
		CGContextSynchronize (gCGContext);
		if (!pSystemContext && pWindow)
			QDEndCGContext (GetWindowPort ((WindowRef)pWindow), &gCGContext);
		if (pFrame)
			pFrame->setDrawContext (0);
	}
	if (atsuStyle)
	{
		ATSUDisposeStyle (atsuStyle);
		atsuStyle = 0;
	}
#elif BEOS
	if (pView)
	{
		pView->Flush ();
		pView->UnlockLooper ();
	}
#endif
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

#elif MAC
	#if QUARTZ

	// nothing to do here

	#else
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	if (pWindow)
	{
		GetGWorld (&OrigPort, &OrigDevice);
		SetGWorld (getPort (), NULL);
		PenState penState;
		GetPenState (&penState);
		switch (lineStyle) 
		{
		case kLineOnOffDash:
			StuffHex (&penState.pnPat, "\pF0F0F0F00F0F0F0F"); // dashed line 4 pixel
			break;
		default:
			StuffHex (&penState.pnPat, "\pFFFFFFFFFFFFFFFF");
			break;
		}
		SetPenState (&penState);
		SetGWorld (OrigPort, OrigDevice);
	}
	#endif

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

#elif MAC
	#if QUARTZ
	if (gCGContext)
		CGContextSetLineWidth (gCGContext, width);
	#else
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	if (pWindow)
	{
		GetGWorld (&OrigPort, &OrigDevice);
		SetGWorld (getPort (), NULL);
		PenState penState;
		GetPenState (&penState);
		penState.pnSize.h = width;
		penState.pnSize.v = width;
		SetPenState (&penState);
		SetGWorld (OrigPort, OrigDevice);
	}
	#endif
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
	long iMode = 0;
	switch (drawMode) 
	{
	case kXorMode :
		iMode = R2_NOTXORPEN; // Pixel is the inverse of the R2_XORPEN color (final pixel = ~ (pen ^ screen pixel)).
		break;
	case kOrMode :
		iMode = R2_MERGEPEN; // Pixel is a combination of the pen color and the screen color (final pixel = pen | screen pixel).
		break;
	default:
		iMode = R2_COPYPEN;
		break;
	}
	SetROP2 ((HDC)pSystemContext, iMode);
	#endif

#elif MAC
	#if QUARTZ
	// quartz only support antialias
	if (gCGContext)
			CGContextSetShouldAntialias (gCGContext, drawMode == kAntialias ? true : false);

	#else
	if (pWindow)
	{
		CGrafPtr OrigPort;
		GDHandle OrigDevice;
		GetGWorld (&OrigPort, &OrigDevice);
		SetGWorld (getPort (), NULL);
		long iMode = 0;

		switch (drawMode) 
		{
		case kXorMode :
			iMode = patXor;
			break;
		case kOrMode :
			iMode = patOr;
			break;
		default:
			iMode = patCopy;
		}
		PenMode (mode);
		
		SetGWorld (OrigPort, OrigDevice);
	}
	#endif

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

#if MAC
	#if QUARTZ
	if (0 && gCGContext)
	{
		CGContextRestoreGState (gCGContext);
		CGContextSaveGState (gCGContext);
		CGContextScaleCTM (gCGContext, 1, -1);
		CGRect cgClipRect = CGRectMake (clipRect.left, clipRect.top, clipRect.width ()-1.f, clipRect.height ()-1.f);
		CGContextClipToRect (gCGContext, cgClipRect);
		CGContextScaleCTM (gCGContext, 1, -1);
		setLineWidth (frameWidth);
		setLineStyle (lineStyle);
		setFrameColor (frameColor);
		setFillColor (fillColor);
		setFont (fontId, fontSize);
		setDrawMode (drawMode);
	}
	
	#else
	Rect r;
	CRect2Rect (_clip, r);

	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	GetGWorld (&OrigPort, &OrigDevice);
	SetGWorld (getPort (), NULL);
	ClipRect (&r);
	SetGWorld (OrigPort, OrigDevice);
	#endif

#elif WINDOWS
	#if GDIPLUS
	if (pGraphics)
		pGraphics->SetClip (Gdiplus::Rect (clipRect.left, clipRect.top, clipRect.getWidth (), clipRect.getHeight ()), Gdiplus::CombineModeReplace);
	#else
	RECT r = {clipRect.left, clipRect.top, clipRect.right, clipRect.bottom};
	HRGN hRgn  = CreateRectRgn (r.left, r.top, r.right, r.bottom);
	SelectClipRgn ((HDC)pSystemContext, hRgn);
	DeleteObject (hRgn);
	#endif

#elif BEOS
	clipping_rect r = {clipRect.left, clipRect.top, clipRect.right - 1, clipRect.bottom - 1};	
	BRegion region;
	region.Set (r);
	pView->ConstrainClippingRegion (&region);
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

#if (MAC && QUARTZ)
	if (0 && gCGContext)
	{
		CGContextRestoreGState (gCGContext);
		CGContextScaleCTM (gCGContext, 1, -1);
		CGRect cgClipRect = CGRectMake (newClip.left, newClip.top, newClip.width (), newClip.height ());
		CGContextClipToRect (gCGContext, cgClipRect);
		CGContextScaleCTM (gCGContext, 1, -1);
		CGContextSaveGState (gCGContext);
		setLineWidth (frameWidth);
		setLineStyle (lineStyle);
		setFrameColor (frameColor);
		setFillColor (fillColor);
		setFont (fontId, fontSize);
		setDrawMode (drawMode);
	}

#elif MAC || WINDOWS
	setClipRect (newClip);

#elif BEOS
	pView->ConstrainClippingRegion (NULL);
#endif

	clipRect = newClip;
}

//-----------------------------------------------------------------------------
void CDrawContext::moveTo (const CPoint &_point)
{
	CPoint point (_point);
	point.offset (offset.h, offset.v);

#if WINDOWS
	#if GDIPLUS
	penLoc = point;
	#else
	MoveToEx ((HDC)pSystemContext, point.h, point.v, NULL);
	#endif  

#elif MAC
	#if QUARTZ
	#else
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	GetGWorld (&OrigPort, &OrigDevice); // get current GrafPort
	SetGWorld (getPort (), NULL);       // activate our GWorld
	MoveTo (point.h, point.v);
	SetGWorld (OrigPort, OrigDevice);
	#endif
  	penLoc = point;
  	
#elif BEOS
	penLoc = point;
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
		pGraphics->DrawLine (pPen, penLoc.h, penLoc.v, point.h, point.v);
	penLoc = point;
	#else
	LineTo ((HDC)pSystemContext, point.h, point.v);
	#endif
	
#elif MAC
	#if QUARTZ
	CGContextRef context = beginCGContext (true);
	{
		QuartzSetLineDash (context, lineStyle, frameWidth);

		CGContextBeginPath (context);
		CGContextMoveToPoint (context, penLoc.h, penLoc.v);
		CGContextAddLineToPoint (context, point.h, point.v);
		CGContextDrawPath (context, kCGPathStroke);
		releaseCGContext (context);
	}
	penLoc = point;
	#else
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	GetGWorld (&OrigPort, &OrigDevice); // get current GrafPort
	SetGWorld (getPort (), NULL);       // activate our GWorld
	RGBColor col;
	CColor2RGBColor (frameColor, col);
	RGBForeColor (&col);
	#if 1
	if (point.v == penLoc.v)
	{
		CPoint old = point;
		if (point.h > penLoc.h)
			point.h--;
		else
			point.h++;
		penLoc = old;
		LineTo (point.h, point.v);
		MoveTo (penLoc.h, penLoc.v);
	}
	else if (point.h == penLoc.h)
	{
		CPoint old = point;
		if (point.v > penLoc.v)
			point.v--;
		else
			point.v++;
		penLoc = old;
		LineTo (point.h, point.v);
		MoveTo (penLoc.h, penLoc.v);
	}
	else
	{
		penLoc = point;	
		LineTo (point.h, point.v);
	}
	#else
	if (point.v > penLoc.v)
		point.v--;
	else if (point.v < penLoc.v)
		point.v++;
	if (point.h > penLoc.h)
		point.h--;
	else if (point.h < penLoc.h)
		point.h++;
	penLoc = point;
	LineTo (point.h, point.v);
	#endif
	SetGWorld (OrigPort, OrigDevice);
	#endif

#elif BEOS
	rgb_color c = { frameColor.red, frameColor.green, frameColor.blue, 255 };
	pView->SetHighColor (c);
	pView->SetDrawingMode (modeToPlatform [drawMode]);
	pView->SetPenSize (frameWidth);
	lineFromTo (penLoc, point);
	penLoc = point;
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::drawLines (const CPoint* points, const long& numLines)
{
	#if QUARTZ
	CGContextRef context = beginCGContext (true);
	if (context) 
	{
		QuartzSetLineDash (context, lineStyle, frameWidth);

		#ifdef MAC_OS_X_VERSION_10_4
		if (_CGContextStrokeLineSegments)
		{
			CGPoint* cgPoints = new CGPoint[numLines*2];
			for (long i = 0; i < numLines * 2; i += 2)
			{
				cgPoints[i].x = points[i].x + offset.x;
				cgPoints[i+1].x = points[i+1].x + offset.x;
				cgPoints[i].y = points[i].y + offset.y;
				cgPoints[i+1].y = points[i+1].y + offset.y;
			}
			_CGContextStrokeLineSegments (context, cgPoints, numLines*2);
			delete [] cgPoints;
		}
		else
		#endif
		{
			CGContextBeginPath (context);
			for (long i = 0; i < numLines * 2; i += 2)
			{
				CGContextMoveToPoint (context, points[i].x + offset.x, points[i].y + offset.y);
				CGContextAddLineToPoint (context, points[i+1].x + offset.x, points[i+1].y + offset.y);
			}
			CGContextDrawPath (context, kCGPathStroke);
		}
		releaseCGContext (context);
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
void CDrawContext::drawPolygon (const CPoint *pPoints, long numberOfPoints, const CDrawStyle drawStyle)
{
#if MAC && QUARTZ
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
#else
	if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
		fillPolygon (pPoints, numberOfPoints);
	if (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked)
		polyLine (pPoints, numberOfPoints);
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::polyLine (const CPoint *pPoints, long numberOfPoints)
{
#if WINDOWS
	#if GDIPLUS
	#else
	POINT points[30];
	POINT *polyPoints;
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

	Polyline ((HDC)pSystemContext, polyPoints, numberOfPoints);

	if (allocated)
		delete[] polyPoints;
	#endif

#elif MAC
	#if QUARTZ
	drawPolygon (pPoints, numberOfPoints);

	#else
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	GetGWorld (&OrigPort, &OrigDevice);
	SetGWorld (getPort (), NULL);
	RGBColor col;
	CColor2RGBColor (frameColor, col);
	RGBForeColor (&col);
	MoveTo (pPoints[0].h, pPoints[0].v);
	for (long i = 1; i < numberOfPoints; i++)
		LineTo (pPoints[i].h + offset.h, pPoints[i].v + offset.v);
	SetGWorld (OrigPort, OrigDevice);
	#endif

#elif BEOS
	rgb_color c = { frameColor.red, frameColor.green, frameColor.blue, 255 };
	pView->SetHighColor (c);
	pView->SetDrawingMode (modeToPlatform [drawMode]);
	pView->SetPenSize (frameWidth);

	CPoint begin (pPoints[0]);
	begin.offset (offset.h, offset.v);
	CPoint end;
	for (long i = 1; i < numberOfPoints; i++)
	{
		end = pPoints[i];
		end.offset (offset.h, offset.v);
		lineFromTo (begin, end);
		begin = end;
	}
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::fillPolygon (const CPoint *pPoints, long numberOfPoints)
{
	// Don't draw boundary
#if WINDOWS
	#if GDIPLUS
	#else
	POINT points[30];
	POINT *polyPoints;
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

	HANDLE nullPen = GetStockObject (NULL_PEN);
	HANDLE oldPen  = SelectObject ((HDC)pSystemContext, nullPen);
	Polygon ((HDC)pSystemContext, polyPoints, numberOfPoints);
	SelectObject ((HDC)pSystemContext, oldPen);

	if (allocated)
		delete[] polyPoints;
	#endif

#elif MAC
	#if QUARTZ
	drawPolygon (pPoints, numberOfPoints, kDrawFilled);

	#else
	CGrafPtr   OrigPort;
	GDHandle   OrigDevice;
	PolyHandle thePoly;
	RGBColor	col;
	
	GetGWorld (&OrigPort, &OrigDevice);
	SetGWorld (getPort (), NULL);
	CColor2RGBColor (fillColor, col);
	RGBForeColor (&col);
	thePoly = OpenPoly ();              // start recording 
	polyLine (pPoints, numberOfPoints); // draw polygon
	LineTo (pPoints[0].h + offset.h, pPoints[0].v + offset.v);  // close the boundary
	ClosePoly ();                       // stop recording 
	
	PixPatHandle pixPatHandle = NewPixPat ();
	CColor2RGBColor (fillColor, col);
	MakeRGBPat (pixPatHandle, &col);    // create pixel pattern with fill color
	
	FillCPoly (thePoly, pixPatHandle);  // fill inside 
	KillPoly (thePoly);                 // deallocate all memory used here 
	DisposePixPat (pixPatHandle);
	SetGWorld (OrigPort, OrigDevice);
	#endif
	
#elif BEOS
	BPoint bpoints[30];
	BPoint* polyPoints;
	bool allocated = false;
	
	if (numberOfPoints > 30)
	{
		polyPoints = new BPoint [numberOfPoints];
		if (!polyPoints)
			return;
		allocated = true;
	}	
	else
		polyPoints = bpoints;
			
	for (long i = 0; i < numberOfPoints; i++)
		polyPoints[i].Set (pPoints[i].h + offset.h, pPoints[i].v + offset.v);

	rgb_color c = { fillColor.red, fillColor.green, fillColor.blue, 255 };
	pView->SetHighColor (c);
	pView->SetDrawingMode (modeToPlatform [drawMode]);
	pView->FillPolygon (polyPoints, numberOfPoints);

	if (allocated)
		delete[] polyPoints;
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::drawRect (const CRect &_rect, const CDrawStyle drawStyle)
{
	CRect rect (_rect);
	rect.offset (offset.h, offset.v);

#if WINDOWS
	#if GDIPLUS
	if (pGraphics && pBrush && pPen)
	{
		rect.normalize ();
		if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
		{
			pGraphics->FillRectangle (pBrush, rect.left, rect.top, rect.getWidth ()-1, rect.getHeight ()-1);
		}
		if (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked)
		{
			pGraphics->DrawRectangle (pPen, rect.left, rect.top, rect.getWidth ()-1, rect.getHeight ()-1);
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

#elif MAC
	#if QUARTZ
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

		CGContextBeginPath (context);
		CGContextMoveToPoint (context, r.origin.x, r.origin.y);
		CGContextAddLineToPoint (context, r.origin.x + r.size.width, r.origin.y);
		CGContextAddLineToPoint (context, r.origin.x + r.size.width, r.origin.y + r.size.height);
		CGContextAddLineToPoint (context, r.origin.x, r.origin.y + r.size.height);
		CGContextClosePath (context);

		CGContextDrawPath (context, m);

		releaseCGContext (context);
	}
	#else
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	GetGWorld (&OrigPort, &OrigDevice);	// get current GrafPort
	SetGWorld (getPort (), NULL);       // activate our GWorld
	
	if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked)
	{
		Rect rr;
		RGBColor col;
		CColor2RGBColor (fillColor, col);
		RGBForeColor (&col);
		CRect2Rect (rect, rr);
		FillRect (&rr, &fillPattern);
	}
	if (drawStyle == kDrawStroked || drawStyle == kDrawFilledAndStroked)
	{
		RGBColor col;
		CColor2RGBColor (frameColor, col);
		RGBForeColor (&col);
		MoveTo (rect.left, rect.top);
		LineTo (rect.right-1, rect.top);
		LineTo (rect.right-1, rect.bottom-1);
		LineTo (rect.left, rect.bottom-1);
		LineTo (rect.left, rect.top);
	}
	SetGWorld (OrigPort, OrigDevice);
	#endif

#elif BEOS
	rgb_color c = { frameColor.red, frameColor.green, frameColor.blue, 255 };
	pView->SetHighColor (c);
	pView->SetDrawingMode (modeToPlatform [drawMode]);
	BRect r (rect.left, rect.top, rect.right, rect.bottom);
	pView->SetPenSize (frameWidth);
	pView->StrokeRect (r);

#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::fillRect (const CRect &_rect)
{
	CRect rect (_rect);
	rect.offset (offset.h, offset.v);

	// Don't draw boundary
#if WINDOWS
	#if GDIPLUS
	if (pGraphics && pBrush)
	{
		rect.normalize ();
		pGraphics->FillRectangle (pBrush, rect.left, rect.top, rect.getWidth (), rect.getHeight ());
	}
	#else
	RECT wr = {rect.left + 1, rect.top + 1, rect.right, rect.bottom};
	HANDLE nullPen = GetStockObject (NULL_PEN);
	HANDLE oldPen  = SelectObject ((HDC)pSystemContext, nullPen);
	FillRect ((HDC)pSystemContext, &wr, (HBRUSH)pBrush);
	SelectObject ((HDC)pSystemContext, oldPen);
	#endif

#elif MAC
	#if QUARTZ
	CGContextRef context = beginCGContext (true);
	{
		CGRect r = CGRectMake (rect.left, rect.top, rect.width (), rect.height ());
		CGContextFillRect (context, r);
		releaseCGContext (context);
	}
	#else
	Rect     rr;
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	GetGWorld (&OrigPort, &OrigDevice);
	SetGWorld (getPort (), NULL);
	RGBColor col;
	CColor2RGBColor (fillColor, col);
	RGBForeColor (&col);
	CRect2Rect (rect, rr);
	rr.left++;
	rr.top++;
	FillRect (&rr, &fillPattern);
	SetGWorld (OrigPort, OrigDevice);
	#endif

#elif BEOS
	rgb_color c = { fillColor.red, fillColor.green, fillColor.blue, 255 };
	pView->SetHighColor (c);
	pView->SetDrawingMode (modeToPlatform [drawMode]);
	BRect r (rect.left + 1, rect.top + 1, rect.right - 1, rect.bottom - 1);
	pView->FillRect (r);
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::drawEllipse (const CRect &_rect, const CDrawStyle drawStyle)
{
	#if QUARTZ
	CRect rect (_rect);
	rect.offset (offset.h, offset.v);

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
			float a = CGRectGetWidth (cgRect) / 2;
			float b = CGRectGetHeight (cgRect) / 2;

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
			float radius = rect.width () * 0.5f;
			CGContextBeginPath (context);
			CGContextAddArc (context, rect.left + radius, rect.top + radius, radius, radians (0), radians (360), 0);
			CGContextClosePath (context);
			CGContextDrawPath (context, m);
		}
		releaseCGContext (context);
	}

	#else
	CPoint point (_rect.left + (_rect.right - _rect.left) / 2, _rect.top);
	drawArc (_rect, point, point);
	#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::fillEllipse (const CRect &_rect)
{
	CRect rect (_rect);
	rect.offset (offset.h, offset.v);

	// Don't draw boundary
#if WINDOWS
	#if GDIPLUS
	#else
	HANDLE nullPen = GetStockObject (NULL_PEN);
	HANDLE oldPen  = SelectObject ((HDC)pSystemContext, nullPen);
	Ellipse ((HDC)pSystemContext, rect.left + 1, rect.top + 1, rect.right + 1, rect.bottom + 1);
	SelectObject ((HDC)pSystemContext, oldPen);
	#endif

#elif QUARTZ
	CGContextRef context = beginCGContext (true);
	{
		CGContextSaveGState (context);
		CGContextBeginPath (context);

		CGRect cgRect = CGRectMake (rect.left, rect.top, rect.width (), rect.height ());
		CGPoint center = CGPointMake (CGRectGetMidX (cgRect), CGRectGetMidY (cgRect));
		float a = CGRectGetWidth (cgRect) / 2;
		float b = CGRectGetHeight (cgRect) / 2;

	    CGContextTranslateCTM (context, center.x, center.y);
	    CGContextScaleCTM (context, a, b);
	    CGContextMoveToPoint (context, 1, 0);
	    CGContextAddArc (context, 0, 0, 1, radians (0), radians (360), 0);

		CGContextClosePath (context);
		CGContextRestoreGState (context);
		CGContextDrawPath (context, kCGPathFill);
		releaseCGContext (context);
	}

#else
	CPoint point (_rect.left + ((_rect.right - _rect.left) / 2), _rect.top);
	fillArc (_rect, point, point);
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::drawPoint (const CPoint &_point, CColor color)
{
	CPoint point (_point);

#if WINDOWS
	#if GDIPLUS
	#else
	point.offset (offset.h, offset.v);
	SetPixel ((HDC)pSystemContext, point.h, point.v, RGB(color.red, color.green, color.blue));
	#endif

#elif MAC
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

#else
	int oldframeWidth = frameWidth;
	CColor oldframecolor = frameColor;
	setLineWidth (1);
	setFrameColor (color);
	moveTo (point);
	lineTo (point);
	
	setFrameColor (oldframecolor);
	setLineWidth (oldframeWidth);
#endif
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
CColor CDrawContext::getPoint (const CPoint& _point)
{
	CPoint point (_point);
	point.offset (offset.h, offset.v);
	CColor color = kBlackCColor;

	#if WINDOWS
	#if GDIPLUS
	#else
	COLORREF c  = GetPixel ((HDC)pSystemContext, point.h, point.v);
	color.red   = GetRValue (c);
	color.green = GetGValue (c);
	color.blue  = GetBValue (c);
	#endif

	#elif MAC
	#if QUARTZ
	// no quartz equivalent
	
	#else
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	GetGWorld (&OrigPort, &OrigDevice);
	SetGWorld (getPort (), NULL);
	RGBColor cPix;
	GetCPixel (point.h, point.v, &cPix);
	RGBColor2CColor (cPix, color);
	SetGWorld (OrigPort, OrigDevice);
	#endif
	#endif

	return color;
}

//-----------------------------------------------------------------------------
void CDrawContext::floodFill (const CPoint& _start)
{
	CPoint start (_start);
	start.offset (offset.h, offset.v);

	#if WINDOWS
	#if GDIPLUS
	#else
	COLORREF c = GetPixel ((HDC)pSystemContext, start.h, start.v);
	ExtFloodFill ((HDC)pSystemContext, start.h, start.v, c, FLOODFILLSURFACE);
	#endif
	
	#elif MAC
	#if QUARTZ
	// no quartz equivalent
	
	#else
	CGrafPtr oldPort;
	GDHandle oldDevice;
	GetGWorld (&oldPort, &oldDevice);
	SetGWorld (getPort (), 0);
	
	Rect r;
	GetPortBounds (getPort (), &r);
	GWorldPtr pMask;
	OSErr err = NewGWorld ((GWorldPtr*)&pMask, 1, &r, 0, 0, 0); // create monochrome GWorld
	if (!err)
	{
		// generate fill mask
		PixMapHandle srcBits = GetGWorldPixMap (getPort ());
		PixMapHandle dstBits = GetGWorldPixMap (pMask);
		if (srcBits && dstBits)
		{
			LockPixels (srcBits);
			LockPixels (dstBits);
		
			SeedCFill ((BitMapPtr)*srcBits, (BitMapPtr)*dstBits, &r, &r, start.h, start.v, 0, 0);

			// fill destination
			RGBColor oldForeColor, oldBackColor;
			GetForeColor (&oldForeColor);
			GetBackColor (&oldBackColor);
		
			::BackColor (whiteColor);
		
			RGBColor col;
			CColor2RGBColor (fillColor, col);
			RGBForeColor (&col);		
		
			CopyMask ((BitMapPtr)*dstBits, (BitMapPtr)*dstBits, (BitMapPtr)*srcBits, &r, &r, &r);
		
			RGBForeColor (&oldForeColor);
			RGBBackColor (&oldBackColor);

			// cleanup
			UnlockPixels (srcBits);
			UnlockPixels (dstBits);
		}
		
		DisposeGWorld (pMask);
	}

	SetGWorld (oldPort, oldDevice);
	#endif
	#endif
}
#endif // VSTGUI_ENABLE_DEPRECATED_METHODS

#if QUARTZ
void addOvalToPath(CGContextRef c, CPoint center, float a, float b, float start_angle, float end_angle)
{
	CGContextSaveGState (c);
	CGContextTranslateCTM (c, center.x, center.y);
	CGContextScaleCTM (c, a, b);

	CGContextMoveToPoint (c, cos (radians (start_angle)), sin (radians (start_angle)));

	CGContextAddArc(c, 0, 0, 1, radians (start_angle), radians (end_angle), 1);

	CGContextRestoreGState(c);
}
#endif

//-----------------------------------------------------------------------------
void CDrawContext::drawArc (const CRect &_rect, const float _startAngle, const float _endAngle, const CDrawStyle drawStyle) // in degree
{
	CRect rect (_rect);
	rect.offset (offset.h, offset.v);

	#if WINDOWS
	#if GDIPLUS
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

	#elif MAC

	#if QUARTZ
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

		CGContextDrawPath (context, m);
		releaseCGContext (context);
	}
	#else
	Rect     rr;
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	GetGWorld (&OrigPort, &OrigDevice);
	SetGWorld (getPort (), NULL);
	RGBColor col;
	CColor2RGBColor (frameColor, col);
	RGBForeColor (&col);
	CRect2Rect (rect, rr);
	FrameArc (&rr, 90 - _startAngle, -_endAngle);
	SetGWorld (OrigPort, OrigDevice);
	#endif

	#elif BEOS
	rgb_color c = { frameColor.red, frameColor.green, frameColor.blue, 255 };
	pView->SetHighColor (c);
	pView->SetDrawingMode (modeToPlatform [drawMode]);
	BRect r (rect.left, rect.top, rect.right, rect.bottom);
	pView->SetPenSize (frameWidth);
	pView->StrokeArc (r, _startAngle, _endAngle);

	#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::drawArc (const CRect &_rect, const CPoint &_point1, const CPoint &_point2)
{
	CRect rect (_rect);
	rect.offset (offset.h, offset.v);
	CPoint point1 (_point1);
	point1.offset (offset.h, offset.v);
	CPoint point2 (_point2);
	point2.offset (offset.h, offset.v);

	// draws from point1 to point2 counterclockwise
#if WINDOWS
	#if GDIPLUS
	#else
	Arc ((HDC)pSystemContext, rect.left, rect.top, rect.right + 1, rect.bottom + 1, 
			 point1.h, point1.v, point2.h, point2.v);
	#endif

#elif MAC || BEOS
	
	int	angle1, angle2;
	if ((point1.v == point2.v) && (point1.h == point2.h))
	{
		angle1 = 0;
		angle2 = 23040; // 360 * 64
	}
	else
	{
		CPoint pm ((rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2);
		angle1 = convertPoint2Angle (pm, point1);
		angle2 = convertPoint2Angle (pm, point2) - angle1;
		if (angle2 < 0)
			angle2 += 23040; // 360 * 64
	}

#if MAC

	#if QUARTZ
	angle1 /= 64;
	angle2 /= 64;
	CGContextRef context = beginCGContext (true);
	{
		QuartzSetLineDash (context, lineStyle, frameWidth);

		CGContextBeginPath (context);
		addOvalToPath (context, CPoint (rect.left + rect.width () / 2, rect.top + rect.height () / 2), rect.width () / 2, rect.height () / 2, 90-angle1, (90-angle1)-angle2);
		CGContextDrawPath (context, kCGPathStroke);
		releaseCGContext (context);
	}
	#else
	Rect     rr;
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	GetGWorld (&OrigPort, &OrigDevice);
	SetGWorld (getPort (), NULL);
	RGBColor col;
	CColor2RGBColor (frameColor, col);
	RGBForeColor (&col);
	CRect2Rect (rect, rr);
	FrameArc (&rr, 90 - (angle1 / 64), -angle2 / 64);
	SetGWorld (OrigPort, OrigDevice);
	#endif
	        
#elif BEOS
	rgb_color c = { frameColor.red, frameColor.green, frameColor.blue, 255 };
	pView->SetHighColor (c);
	pView->SetDrawingMode (modeToPlatform [drawMode]);
	BRect r (rect.left, rect.top, rect.right, rect.bottom);
	pView->SetPenSize (frameWidth);
	pView->StrokeArc (r, angle1 / 64, angle2 / 64);
#endif	

#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::fillArc (const CRect &_rect, const CPoint &_point1, const CPoint &_point2)
{
	CRect rect (_rect);
	rect.offset (offset.h, offset.v);
	CPoint point1 (_point1);
	point1.offset (offset.h, offset.v);
	CPoint point2 (_point2);
	point2.offset (offset.h, offset.v);

	// Don't draw boundary
#if WINDOWS
	#if GDIPLUS
	#else
	HANDLE nullPen = GetStockObject (NULL_PEN);
	HANDLE oldPen  = SelectObject ((HDC)pSystemContext, nullPen);
	Pie ((HDC)pSystemContext, offset.h + rect.left + 1, offset.v + rect.top + 1, offset.h + rect.right, offset.v + rect.bottom, 
			 point1.h, point1.v, point2.h, point2.v);
	SelectObject ((HDC)pSystemContext, oldPen);
	#endif

#elif MAC || BEOS
	
	int	angle1, angle2;
	if ((point1.v == point2.v) && (point1.h == point2.h))
	{
		angle1 = 0;
		angle2 = 23040; // 360 * 64
	}
	else
	{
		CPoint pm ((rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2);
		angle1 = convertPoint2Angle (pm, point1);
		angle2 = convertPoint2Angle (pm, point2);
	}

#if MAC
	#if QUARTZ
	angle1 /= 64;
	angle2 /= 64;
	CGContextRef context = beginCGContext (true);
	{
		CGContextBeginPath (context);
		addOvalToPath (context, CPoint (rect.left + rect.width () / 2, rect.top + rect.height () / 2), rect.width () / 2, rect.height () / 2, -angle1, -angle2);
		CGContextClosePath (context);
		CGContextDrawPath (context, kCGPathFill);
		releaseCGContext (context);
	}
	
	#else
	Rect     rr;
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	GetGWorld (&OrigPort, &OrigDevice);
	SetGWorld (getPort (), NULL);
	RGBColor col;
	CColor2RGBColor (fillColor, col);
	RGBForeColor (&col);
	CRect2Rect (rect, rr);
	
	angle2 = angle2 - angle1;
	if (angle2 < 0)
		angle2 = -angle2;
	FillArc (&rr, 90 - (angle1 / 64), -angle2 / 64, &fillPattern);

	SetGWorld (OrigPort, OrigDevice);
	#endif
        
#elif BEOS
	rgb_color c = { fillColor.red, fillColor.green, fillColor.blue, 255 };
	pView->SetHighColor (c);
	pView->SetDrawingMode (modeToPlatform [drawMode]);
	BRect r (rect.left + 1, rect.top + 1, rect.right - 1, rect.bottom - 1);
	pView->FillArc (r, angle1 / 64, angle2 / 64);

#endif
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::setFontColor (const CColor color)
{
	fontColor = color;
	
#if WINDOWS
	#if GDIPLUS
	if (pFontBrush)
		pFontBrush->SetColor (Gdiplus::Color (color.alpha, color.red, color.green, color.blue));
	#else
	SetTextColor ((HDC)pSystemContext, RGB (fontColor.red, fontColor.green, fontColor.blue));
	#endif

#elif MAC
	#if QUARTZ
	// on quartz the fill color is the font color

	if (atsuStyle)
	{
		ATSUDisposeStyle (atsuStyle);
		atsuStyle = 0;
	}

	#else
	RGBColor col;
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	if (pWindow)
	{
		GetGWorld (&OrigPort, &OrigDevice);
		SetGWorld (getPort (), NULL);
		CColor2RGBColor (fontColor, col);
		RGBForeColor (&col);
		SetGWorld (OrigPort, OrigDevice);
	}
	#endif
        
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
		pPen->SetColor (Gdiplus::Color (color.alpha, color.red, color.green, color.blue));
	#else
	LOGPEN logPen = {iPenStyle, {frameWidth, frameWidth}, 
					 RGB (frameColor.red, frameColor.green, frameColor.blue)};
	
	HANDLE newPen = CreatePenIndirect (&logPen);
	SelectObject ((HDC)pSystemContext, newPen);
	if (pPen)
		DeleteObject (pPen);
	pPen = newPen;
	#endif

#elif MAC
	#if QUARTZ
	if (gCGContext)
		CGContextSetRGBStrokeColor (gCGContext, color.red/255.f, color.green/255.f, color.blue/255.f, color.alpha/255.f);
	#else
	RGBColor col;
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	if (pWindow)
	{
		GetGWorld (&OrigPort, &OrigDevice);
		SetGWorld (getPort (), NULL);
		CColor2RGBColor (frameColor, col);
		RGBForeColor (&col);
		SetGWorld (OrigPort, OrigDevice);
	}
	#endif
        
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
		pBrush->SetColor (Gdiplus::Color (color.alpha, color.red, color.green, color.blue));
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
	
#elif MAC
	#if QUARTZ
	if (gCGContext)
		CGContextSetRGBFillColor (gCGContext, color.red/255.f, color.green/255.f, color.blue/255.f, color.alpha/255.f);
	#else
	RGBColor col;
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	if (pWindow)
	{
		GetGWorld (&OrigPort, &OrigDevice);
		SetGWorld (getPort (), NULL);
		CColor2RGBColor (fillColor, col);
		RGBForeColor (&col);
		SetGWorld (OrigPort, OrigDevice);
	}
	#endif
        
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::setFont (CFont fontID, const long size, long style)
{
	if (fontID < 0 || fontID >= kNumStandardFonts)
		fontID = kSystemFont;

	if (fontId == fontID && fontSize == (size != 0 ? size : gStandardFontSize[fontID]) && fontStyle == style)
		return;

	fontStyle = style;
	fontId = fontID;
	if (size != 0)
		fontSize = size;
	else
		fontSize = gStandardFontSize[fontID];

#if WINDOWS
	#if GDIPLUS

	if (pFont)
		delete pFont;

	int gdiStyle = Gdiplus::FontStyleRegular;
	if (style & kBoldFace)
		gdiStyle = Gdiplus::FontStyleBold;
	if (style & kItalicFace)
		gdiStyle = Gdiplus::FontStyleItalic;
	if (style & kUnderlineFace)
		gdiStyle = Gdiplus::FontStyleUnderline;

	WCHAR tempName[200];
	mbstowcs(tempName,gStandardFontName[fontID],200);
	pFont=new Gdiplus::Font (tempName,(Gdiplus::REAL)fontSize, gdiStyle, Gdiplus::UnitPixel);

	#else
	LOGFONT logfont = {0};

	if (style & kBoldFace)
		logfont.lfWeight = FW_BOLD;
	else
		logfont.lfWeight = FW_NORMAL;
	if (style & kItalicFace)
		logfont.lfItalic = true;
	if (style & kUnderlineFace)
		logfont.lfUnderline = true;
	
	logfont.lfHeight         = -fontSize;
	logfont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	strcpy (logfont.lfFaceName, gStandardFontName[fontID]);

	if (fontID == kSymbolFont)
		logfont.lfPitchAndFamily = DEFAULT_PITCH | FF_DECORATIVE;
	else if (fontID == kSystemFont)
		logfont.lfWeight     = FW_BOLD;
  
	logfont.lfClipPrecision = CLIP_STROKE_PRECIS;
	logfont.lfOutPrecision  = OUT_STRING_PRECIS;
	logfont.lfQuality 	    = DEFAULT_QUALITY;
	logfont.lfCharSet       = ANSI_CHARSET;

	HANDLE newFont = CreateFontIndirect (&logfont);
	SelectObject ((HDC)pSystemContext, newFont);
	if (pFont)
		DeleteObject (pFont);
	pFont = newFont;
	#endif

#elif MAC
	#if QUARTZ
	char myMacXFontName[255];
	strcpy(myMacXFontName, gMacXfontNames[fontId]);
	if (style & kBoldFace)
		strcat(myMacXFontName, " Bold");
	if (style & kItalicFace)
		strcat(myMacXFontName, " Italic");
	if (style & kUnderlineFace)
		strcat(myMacXFontName, " Underline");
	
	if (gCGContext)
		CGContextSelectFont (gCGContext, (const char*)myMacXFontName, fontSize, kCGEncodingMacRoman);

	if (atsuStyle)
	{
		ATSUDisposeStyle (atsuStyle);
		atsuStyle = 0;
	}
		
	#else
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	if (pWindow)
	{
		GetGWorld (&OrigPort, &OrigDevice);
		SetGWorld (getPort (), NULL);
		
		TextFace (style); // normal, bold, italic, underline...
   		TextMode (0);
		TextSize (fontSize);
		
		#if MACX
		short familyID;
		
		GetFNum (gMacXfontNames[fontID], &familyID);

		TextFont (familyID);
		
		#else
		if (fontID == kSymbolFont)
			TextFont (kFontIDSymbol);
		else if (fontID == kSystemFont)
			TextFont (0);	// system
		else if (fontID == kNormalFontSmaller)
			TextFont (kFontIDGeneva);	// Geneva
		else
			TextFont (kFontIDHelvetica);
		#endif
		
		GetFontInfo (&fontInfoStruct);
		SetGWorld (OrigPort, OrigDevice);
	}
	#endif
        
#elif BEOS
	font.SetFamilyAndStyle (gStandardFontName[fontID], gStandardFontStyle[fontID]);
	font.SetSize (fontSize);
	pView->SetFont (&font, B_FONT_FAMILY_AND_STYLE | B_FONT_SIZE);
#endif
}

//------------------------------------------------------------------------------
CCoord CDrawContext::getStringWidth (const char *pStr)
{
	CCoord result = 0;

	#if MAC
	#if QUARTZ
	CGContextRef context = beginCGContext (true);
	if (context)
	{
		CGContextSetTextDrawingMode (context, kCGTextInvisible);
		CGContextSetTextPosition (context, 0.f, 0.f);
		CGContextShowText (context, pStr, strlen (pStr));
		CGPoint p = CGContextGetTextPosition (context);
		result = (CCoord)p.x;
		releaseCGContext (context);
	}
	#else
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	GetGWorld (&OrigPort, &OrigDevice);
	SetGWorld (getPort (), NULL);

	result = (long)TextWidth (pStr, 0, strlen (pStr));

	SetGWorld (OrigPort, OrigDevice);
	#endif
        
	#elif WINDOWS
	#if GDIPLUS
	#else
	SIZE size;
	GetTextExtentPoint32 ((HDC)pSystemContext, pStr, (int)strlen (pStr), &size);
	result = (long)size.cx;
	#endif

	#elif BEOS
	result = (long)(ceil (pView->StringWidth (pStr)));
	#endif

	return result;
}

//-----------------------------------------------------------------------------
void CDrawContext::drawString (const char *string, const CRect &_rect,
							 const short opaque, const CHoriTxtAlign hAlign)
{
	if (!string)
		return;
	
	CRect rect (_rect);
	rect.offset (offset.h, offset.v);

#if WINDOWS
	#if GDIPLUS
	WCHAR buffer[1024];
	Gdiplus::StringFormat z(Gdiplus::StringFormatFlagsNoWrap,LANG_NEUTRAL);;
	Gdiplus::RectF layoutRect((Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.width(), (Gdiplus::REAL)rect.height());

	mbstowcs(buffer,string,1024);
	switch (hAlign)
	{
	case kCenterText:
		// without DT_SINGLELINE no vertical center alignment here
		z.SetAlignment(Gdiplus::StringAlignmentCenter);
		//DrawText ((HDC)pSystemContext, string, (int)strlen (string), &Rect, flag + DT_CENTER);
		break;
		
	case kRightText:
		//DrawText ((HDC)pSystemContext, string, (int)strlen (string), &Rect, flag + DT_RIGHT);
		z.SetAlignment(Gdiplus::StringAlignmentFar);
		break;
		
	default : // left adjust
		layoutRect.X++;
		z.SetAlignment(Gdiplus::StringAlignmentNear);
	}

	Gdiplus::SolidBrush bgBrush(Gdiplus::Color(255, fillColor.red, fillColor.green, fillColor.blue));
	
	if (opaque) {
		pGraphics->FillRectangle(&bgBrush,layoutRect);
	}
	
	pGraphics->DrawString(buffer,-1,pFont,layoutRect,&z,pFontBrush);

	#else
	// set the visibility mask
	SetBkMode ((HDC)pSystemContext, opaque ? OPAQUE : TRANSPARENT);

	RECT Rect = {rect.left, rect.top, rect.right, rect.bottom};
	UINT flag = DT_VCENTER + DT_SINGLELINE + DT_NOPREFIX;
	switch (hAlign)
	{
	case kCenterText:
		// without DT_SINGLELINE no vertical center alignment here
		DrawText ((HDC)pSystemContext, string, (int)strlen (string), &Rect, flag + DT_CENTER);
		break;
		
	case kRightText:
		DrawText ((HDC)pSystemContext, string, (int)strlen (string), &Rect, flag + DT_RIGHT);
		break;
		
	default : // left adjust
		Rect.left++;
		DrawText ((HDC)pSystemContext, string, (int)strlen (string), &Rect, flag + DT_LEFT);
	}

	SetBkMode ((HDC)pSystemContext, TRANSPARENT);
	#endif

#elif MAC
	#if QUARTZ
	CGContextRef context = beginCGContext (true);
	if (context)
	{
		CCoord strWidth = getStringWidth (string);
		rect.bottom -= rect.height ()/2 - fontSize / 2 + 1;
		switch (hAlign)
		{
			case kCenterText:
			{
				rect.left += rect.width () / 2 - strWidth/2;
				break;
			}
			case kRightText:
				rect.left = rect.right - strWidth;
				break;
			default : // left adjust
				rect.left++;
		}

		CGContextSetShouldAntialias (context, true);
		CGContextSetTextDrawingMode (context, kCGTextFill);
		CGContextSetRGBFillColor (context, fontColor.red/255.f, fontColor.green/255.f, fontColor.blue/255.f, fontColor.alpha/255.f);
		CGContextSetTextPosition (context, rect.left, rect.bottom);
		CGContextShowText (context, string, strlen (string));
		releaseCGContext (context);
	}

	#else
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	int width;
	int xPos, yPos;
	int fontHeight;
	int rectHeight;
	int stringLength;

	Rect stringsRect;
	Rect contextsClip;
	Rect compositeClip;
	
	CRect2Rect (rect, stringsRect);
	CRect2Rect (clipRect, contextsClip);
	
	if (SectRect (&stringsRect, &contextsClip, &compositeClip))
	{	
		GetGWorld (&OrigPort, &OrigDevice);
		SetGWorld (getPort (), NULL);
		
		if (opaque)
			TextMode (srcCopy);
		else
			TextMode (srcOr);
		
		RGBColor col;
		CColor2RGBColor (fontColor, col);
		RGBForeColor (&col);

		CColor2RGBColor (fillColor, col);
		RGBBackColor (&col);
		
		rectHeight = rect.height ();
		fontHeight = fontInfoStruct.ascent + fontInfoStruct.descent;
		yPos = rect.bottom - fontInfoStruct.descent;
		if (rectHeight >= fontHeight)
			yPos -= (rectHeight - fontHeight) / 2;
			
		stringLength = strlen (string);
		width = TextWidth ((Ptr)string, 0, stringLength);
	
		switch (hAlign)
		{
		case kCenterText:
			xPos = (rect.right + rect.left - width) / 2;
			break;
			
		case kRightText:
			xPos = rect.right - width;
			break;
			
		default: // left adjust
			xPos = rect.left;
		}
		
		RgnHandle saveRgn = NewRgn ();
		GetClip (saveRgn);
		
		ClipRect (&compositeClip);

		#if TARGET_API_MAC_CARBON
		CFStringRef str;

		// Create a unicode string
		str = CFStringCreateWithCString(NULL, string, kCFStringEncodingMacRoman);
	
		// Initialize proper text box options
		TXNTextBoxOptionsData myOptions;
		myOptions.optionTags = kTXNSetJustificationMask;
		myOptions.justification = kTXNFlushLeft;

		// Determine the vertical alignment of the text box.
		// It is centered vertically.
		// Somehow, the yPos calculation above doesn't work here
		// or I am too stupid to understand it. Therefore I calculate
		// the text position in the surrounding control rect myself.
		long myHeight = (rect.height () - fontHeight) / 2;
		if (myHeight>0)
		{
			stringsRect.top += myHeight;
			stringsRect.bottom += myHeight;
		}
		stringsRect.left = xPos;
		stringsRect.right = xPos + width;//rect.width ();
	
		// Draw the unicode string
		TXNDrawCFStringTextBox (str, &stringsRect, NULL, &myOptions);

		// Release the unicode string
		CFRelease (str);
		#else
		MoveTo (xPos, yPos);
		DrawText ((Ptr)string, 0, stringLength);
		#endif
		
		SetClip (saveRgn);
		DisposeRgn (saveRgn);
		TextMode (srcOr);
		SetGWorld (OrigPort, OrigDevice);
	}
        #endif
        
#elif BEOS
	BRect r (rect.left, rect.top, rect.right - 1, rect.bottom - 1);
	BRegion LocalRegion (r);
	pView->ConstrainClippingRegion (&LocalRegion);
	pView->SetFontSize (fontSize);
	float width = -1;
	if (opaque)
	{
		width = ceil (pView->StringWidth (string));
		CRect cr (rect.left, rect.top, rect.left + width, rect.bottom);
		fillRect (cr);
	}
	rgb_color c = { fontColor.red, fontColor.green, fontColor.blue, 255 };
	pView->SetHighColor (c);
	if (drawMode == kXorMode)
		pView->SetDrawingMode (B_OP_INVERT);
	else
		pView->SetDrawingMode (B_OP_OVER);
	BPoint		p;
	font_height	height;
	pView->GetFontHeight (&height);
	p.y = r.bottom - (rect.height () - height.ascent) / 2;
	if (hAlign == kCenterText || hAlign == kRightText)
	{
		if (width < 0)
			width = ceil (pView->StringWidth (string));
		if (hAlign == kCenterText)
			p.x = rect.left + (rect.right - rect.left - width) / 2;
		else
			p.x = rect.right - width - 1;
	}
	else
		p.x = rect.left + 1;
	pView->DrawString (string, p);
	pView->ConstrainClippingRegion (NULL);
#endif
}

//-----------------------------------------------------------------------------
CCoord CDrawContext::getStringWidthUTF8 (const char* string)
{
	CCoord result = 0;
	#if QUARTZ
	if (atsuStyle == 0)
		atsuStyle = CreateATSUStyle (fontColor, fontId, fontSize, fontStyle);
	CFStringRef utf8Str = CFStringCreateWithCString (NULL, string, kCFStringEncodingUTF8);
	if (utf8Str)
	{
		CGContextRef context = beginCGContext (false);
		if (context)
		{
			OSStatus status;
			CFIndex stringLength = CFStringGetLength (utf8Str);
			UniChar* textBuffer = (UniChar*)malloc (stringLength*sizeof (UniChar));
			CFStringGetCharacters (utf8Str, CFRangeMake (0, stringLength), textBuffer);

			ATSUTextLayout textLayout;
			status = ATSUCreateTextLayout (&textLayout);
			status = ATSUSetTextPointerLocation (textLayout, textBuffer, kATSUFromTextBeginning, kATSUToTextEnd, stringLength);
			status = ATSUSetRunStyle (textLayout, atsuStyle, kATSUFromTextBeginning, kATSUToTextEnd);
			status = ATSUSetTransientFontMatching (textLayout, true);
			
			ATSUAttributeTag		theTags[]	= { kATSUCGContextTag };
			ByteCount				theSizes[]	= { sizeof (CGContextRef) };
			ATSUAttributeValuePtr	theValues[]	= { &context };
			status = ATSUSetLayoutControls (textLayout, 1, theTags, theSizes, theValues);

			ATSUTextMeasurement iBefore, iAfter, ascent, descent; 
			status = ATSUGetUnjustifiedBounds (textLayout, 0, kATSUToTextEnd, &iBefore, &iAfter, &ascent, &descent);
			result = FixRound (iAfter);
			
			ATSUDisposeTextLayout (textLayout);
			free (textBuffer);
			
			releaseCGContext (context);
		}
		CFRelease (utf8Str);
	}
	#endif
	return result;
}

//-----------------------------------------------------------------------------
void CDrawContext::drawStringUTF8 (const char* string, const CPoint& _point, bool antialias)
{
	if (!string)
		return;

	CPoint point (_point);
	point.offset (offset.h, offset.v);

	#if QUARTZ
	if (atsuStyle == 0)
		atsuStyle = CreateATSUStyle (fontColor, fontId, fontSize, fontStyle);

	CFStringRef utf8Str = CFStringCreateWithCString (NULL, string, kCFStringEncodingUTF8);
	if (utf8Str)
	{
		CGContextRef context = beginCGContext (false);
		if (context)
		{
			CGContextSetShouldAntialias (context, antialias);

			OSStatus status;
			CFIndex stringLength = CFStringGetLength (utf8Str);
			UniChar* textBuffer = (UniChar*)malloc (stringLength*sizeof (UniChar));
			CFStringGetCharacters (utf8Str, CFRangeMake (0, stringLength), textBuffer);

			ATSUTextLayout textLayout;
			status = ATSUCreateTextLayout (&textLayout);
			status = ATSUSetTextPointerLocation (textLayout, textBuffer, kATSUFromTextBeginning, kATSUToTextEnd, stringLength);
			status = ATSUSetRunStyle (textLayout, atsuStyle, kATSUFromTextBeginning, kATSUToTextEnd);
			status = ATSUSetTransientFontMatching (textLayout, true);
			
			ATSUAttributeTag		theTags[]	= { kATSUCGContextTag };
			ByteCount				theSizes[]	= { sizeof (CGContextRef) };
			ATSUAttributeValuePtr	theValues[]	= { &context };
			status = ATSUSetLayoutControls (textLayout, 1, theTags, theSizes, theValues);

			status = ATSUDrawText (textLayout, kATSUFromTextBeginning, kATSUToTextEnd, X2Fix(point.h), X2Fix(point.v*-1));
			
			ATSUDisposeTextLayout (textLayout);
			free (textBuffer);
			
			releaseCGContext (context);
		}
		CFRelease (utf8Str);
	}
	#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::drawStringUTF8 (const char* string, const CRect& _rect, const CHoriTxtAlign hAlign, bool antialias)
{
	if (!string)
		return;
	
	CRect rect (_rect);

	rect.bottom -= rect.height ()/2 - fontSize / 2 + 1;
	if (hAlign != kLeftText)
	{
		CCoord stringWidth = getStringWidthUTF8 (string);
		if (hAlign == kRightText)
			rect.left = rect.right - stringWidth;
		else
			rect.left = rect.left + (rect.getWidth () / 2) - (stringWidth / 2);
	}

	drawStringUTF8 (string, CPoint (rect.left, rect.bottom), antialias);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
long CDrawContext::getMouseButtons ()
{
	long buttons = 0;
	
#if WINDOWS
	if (GetAsyncKeyState (VK_LBUTTON) < 0)
		buttons |= (bSwapped_mouse_buttons ? kRButton : kLButton);
	if (GetAsyncKeyState (VK_MBUTTON) < 0)
		buttons |= kMButton;
	if (GetAsyncKeyState (VK_RBUTTON) < 0)
		buttons |= (bSwapped_mouse_buttons ? kLButton : kRButton);
	
	if (GetAsyncKeyState (VK_SHIFT)   < 0)
		buttons |= kShift;
	if (GetAsyncKeyState (VK_CONTROL) < 0)
		buttons |= kControl;
	if (GetAsyncKeyState (VK_MENU)    < 0)
		buttons |= kAlt;

#elif MAC
	#if MACX // this works for MacOSX 10.2 and later
	UInt32 state = GetCurrentButtonState ();
	if (state & kEventMouseButtonPrimary)
		buttons |= kLButton;
	if (state & kEventMouseButtonSecondary)
		buttons |= kRButton;
	if (state & 4)//kEventMouseButtonTertiary) this define is false...Apple ?
		buttons |= kMButton;

	state = GetCurrentKeyModifiers ();
	if (state & cmdKey)
		buttons |= kControl;
	if (state & shiftKey)
		buttons |= kShift;
	if (state & optionKey)
		buttons |= kAlt;
	if (state & controlKey)
		buttons |= kApple;
	// for the one buttons
	if (buttons & kApple && buttons & kLButton)
	{
		buttons &= ~(kApple | kLButton);
		buttons |= kRButton;
	}
	#else
	if (Button ())
		buttons |= kLButton;
	
	KeyMap	Keys;
	unsigned char *BytePtr = (unsigned char*)Keys;
	GetKeys (Keys);
	
	if (BytePtr[7] & 1)		// Shift   		0x38 == 56 = (7 * 8) + 0
		buttons |= kShift;
	if (BytePtr[7] & 8)		// Control (extra Mac) 0x3B == 59 = (7 * 8) + 3
		buttons |= kApple;			
	if (BytePtr[7] & 4)		// Alt   		0x3A == 58 = (7 * 8) + 2
		buttons |= kAlt;			
	if (BytePtr[6] & 128)	// Apple => ctrl (PC)  0x37 == 55 = (6 * 8) + 7
		buttons |= kControl;
	#endif

#elif BEOS
	BPoint	where;
	uint32	b;
	pView->GetMouse (&where, &b);
	if (b & B_PRIMARY_MOUSE_BUTTON)
		buttons |= kLButton;
	if (b & B_SECONDARY_MOUSE_BUTTON)
		buttons |= kRButton;
	if (b & B_TERTIARY_MOUSE_BUTTON)
		buttons |= kMButton;
	int32 m = modifiers ();
	if (m & B_SHIFT_KEY)
		buttons |= kShift;
	if (m & B_COMMAND_KEY)
		buttons |= kControl;
	if (m & B_OPTION_KEY)
		buttons |= kApple;
	if (m & B_CONTROL_KEY)
		buttons |= kAlt;
#endif
	
	return buttons;
}
#endif

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
void CDrawContext::getMouseLocation (CPoint &point)
{
#if WINDOWS
	POINT where;
	GetCursorPos (&where);
	point (where.x, where.y);

#elif MACX
	#if 0 // QUARTZ // does not work sic!
	Point where;
	UInt32 mod;
	MouseTrackingResult result;
	if (TrackMouseLocationWithOptions ((CGrafPtr)-1, 0, kEventDurationNoWait, &where, &mod, &result) == noErr)
	{
		QDGlobalToLocalPoint (getPort (), &where);
		point (where.h, where.v);
	}
	#else
	Point where;
	CGrafPtr savedPort;
	Boolean portChanged = QDSwapPort (getPort (), &savedPort);
	GetMouse (&where);
	if (portChanged)
		QDSwapPort (savedPort, NULL);
	point (where.h, where.v);
	#endif
	#if QUARTZ
	point.offset (pFrame->hiScrollOffset.x,pFrame->hiScrollOffset.y);
	#endif
#elif MAC
	Point where;
	GetMouse (&where);
	point (where.h, where.v);
	
#elif BEOS
	BPoint	where;
	uint32	b;
	pView->GetMouse (&where, &b);
	point (where.x, where.y);
#endif

	point.offset (-offsetScreen.h, -offsetScreen.v);
}
#endif

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
bool CDrawContext::waitDoubleClick ()
{
	bool doubleClick = false;

#if WINDOWS
	CPoint mouseLoc;
	getMouseLocation (mouseLoc);
	CRect observe (mouseLoc.h - 2, mouseLoc.v - 2, mouseLoc.h + 2, mouseLoc.v + 2);

	DWORD currentTime = GetTickCount ();
	DWORD clickTime = GetMessageTime () + (DWORD)GetDoubleClickTime ();

	MSG message;
	while (currentTime < clickTime)
	{
		getMouseLocation (mouseLoc);
		if (!observe.pointInside (mouseLoc))
			break;

		if (PeekMessage (&message, 0, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_REMOVE | PM_NOYIELD)) 
		{
			doubleClick = true;
			break;
		}
		
		currentTime = GetTickCount ();
	}

#elif MAC
	#if MACX
	#if QUARTZ
	EventTimeout timeout = GetDblTime () * kEventDurationSecond / 60;
	const EventTypeSpec eventTypes[] = { { kEventClassMouse, kEventMouseDown }, { kEventClassMouse, kEventMouseDragged } };
	EventRef event;
	if (ReceiveNextEvent (GetEventTypeCount (eventTypes), eventTypes, timeout, true, &event) == noErr)
	{
		if (GetEventKind (event) == kEventMouseDown)
		{
			doubleClick = true;
		}
		ReleaseEvent (event);
	}
	
	#else
	unsigned long clickTime, doubletime;
	EventRecord downEvent;

	doubletime = GetDblTime ();
	clickTime = TickCount () + doubletime;
	while (TickCount () < clickTime)
	{
		if (GetNextEvent (mDownMask, &downEvent))
		{
			doubleClick = true;
			break;
		}
	}
	#endif // !QUARTZ

	#else
	long clickTime, doubleTime;
	EventRecord downEvent;

	#define MOUSE_IS_DOWN   ((* (char*)0x172) >= 0)

	doubleTime = GetDblTime () / 2;
	clickTime  = TickCount () + doubleTime;
	
	while (TickCount () < clickTime)
		if (!MOUSE_IS_DOWN) break;        /* look for mouse up! */
	
	if (GetNextEvent (mUpMask, &downEvent))
	{
		clickTime += doubleTime;
		while (TickCount () < clickTime)
			if (MOUSE_IS_DOWN) break; /* look for mouse down! */
		if (GetNextEvent (mDownMask, &downEvent))
			doubleClick = true;
	}
	#endif
#elif BEOS
	const bigtime_t snoozeTime = 5000;
	bigtime_t	latest = system_time ();
	bigtime_t	doubleclicktime;
	get_click_speed (&doubleclicktime);
	latest += doubleclicktime;
	BPoint	location;
	uint32	buttons;
	pView->GetMouse (&location, &buttons);
	while (buttons)	// user should release the mouse button
	{
		if (system_time () > latest)
			return false;

		snooze (snoozeTime);
		pView->GetMouse (&location, &buttons);
	}
	
	while (!buttons)
	{
		if (system_time () > latest)
			return false;

		snooze (snoozeTime);
		pView->GetMouse (&location, &buttons);
	}
	
	doubleClick = true;

#endif

	return doubleClick;
}
#endif

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
bool CDrawContext::waitDrag ()
{
	#if MACX && QUARTZ
	bool dragged = false;
	if (GetCurrentEventButtonState () & kEventMouseButtonPrimary)
	{
		const EventTypeSpec eventTypes[] = { { kEventClassMouse, kEventMouseUp }, { kEventClassMouse, kEventMouseDown }, { kEventClassMouse, kEventMouseDragged } };
		EventRef event;
		if (ReceiveNextEvent (GetEventTypeCount (eventTypes), eventTypes, kEventDurationForever, true, &event) == noErr)
		{
			if (GetEventKind (event) == kEventMouseDragged)
			{
				dragged = true;
			}
			ReleaseEvent (event);
		}
	}
	return dragged;

	#else
	if (!pFrame)
		return false;
	
	CPoint mouseLoc;
	getMouseLocation (mouseLoc);
	CRect observe (mouseLoc.h - 2, mouseLoc.v - 2, mouseLoc.h + 2, mouseLoc.v + 2);
	
	long currentTime = pFrame->getTicks ();
	bool wasOutside = false;

	while (((getMouseButtons () & ~(kMButton|kRButton)) & kLButton) != 0)
	{
		pFrame->doIdleStuff ();
		if (!wasOutside)
		{
			getMouseLocation (mouseLoc);
			if (!observe.pointInside (mouseLoc))
			{
				if (kDragDelay <= 0)
					return true;
				wasOutside = true;
			}
		}

		if (wasOutside && (pFrame->getTicks () - currentTime > kDragDelay))
			return true;
	}
	return false;
	#endif
}
#endif

//-----------------------------------------------------------------------------
void CDrawContext::forget ()
{
	#if QUARTZ
	synchronizeCGContext ();
	#endif
	CReferenceCounter::forget ();
}

//-----------------------------------------------------------------------------
#if BEOS
//-----------------------------------------------------------------------------
void CDrawContext::lineFromTo (CPoint& cstart, CPoint& cend)
{
	BPoint start (cstart.h, cstart.v);
	BPoint end (cend.h, cend.v);
	if (start.x == end.x)
	{
		if (start.y < end.y)
			end.y--;
		else if (end.y < start.y)
			start.y--;
	}
	else if (start.y == end.y)
	{
		if (start.x < end.x)
			end.x--;
		else if (end.x < start.x)
			start.x--;
	}
	else
	{
		if (start.x > end.x)
		{
			BPoint t = end;
			end = start;
			start = t;
		}
		end.x--;
		if (end.y > start.y)
			end.y--;
		else
			end.y++;
	}
	
	pView->MovePenTo (start);
	if (lineStyle == kLineSolid)
		pView->StrokeLine (end);
	else
	{
		pattern stripes = { {0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3} };
		pView->StrokeLine (end, stripes);
	}
}

//-----------------------------------------------------------------------------
#elif MAC
#if QUARTZ
//-----------------------------------------------------------------------------
CGContextRef CDrawContext::beginCGContext (bool swapYAxis)
{
	if (gCGContext)
	{
		CGContextSaveGState (gCGContext);
		CGContextScaleCTM (gCGContext, 1, -1);
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
		needToSynchronizeCGContext = true;
	}
}

//-----------------------------------------------------------------------------
void CDrawContext::synchronizeCGContext ()
{
	if (needToSynchronizeCGContext && gCGContext)
	{
		CGContextSynchronize (gCGContext);
		needToSynchronizeCGContext = false;
	}
}

//-----------------------------------------------------------------------------
CGImageRef CDrawContext::getCGImage () const
{
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
		float offset = 0;
		float dotf[2] = { lineWidth, lineWidth };
		CGContextSetLineDash (context, offset, dotf, 2);
	}
}

//-----------------------------------------------------------------------------
// This can be optimized with a cache. See Technical Q&A QA1027 on Apple's website
ATSUStyle CreateATSUStyle (const CColor &fontColor, CFont fontID, const long size, long style, const char* name)
{
	char myMacXFontName[255];
	if (name == 0)
	{
		strcpy (myMacXFontName, gMacXfontNames[fontID]);
		if (style & kBoldFace)
			strcat(myMacXFontName, " Bold");
	}
	else
		strcpy (myMacXFontName, name);

	ATSUStyle atsuStyle;
	OSStatus status = ATSUCreateStyle (&atsuStyle);

	ATSUFontID atsuFontID;
	status = ATSUFindFontFromName (myMacXFontName, strlen(myMacXFontName), kFontFullName, kFontNoPlatformCode, kFontNoScriptCode, kFontNoLanguageCode, &atsuFontID);
	if (status != noErr)
		status = ATSUFindFontFromName (gMacXfontNames[fontID], strlen(gMacXfontNames[fontID]), kFontFullName, kFontNoPlatformCode, kFontNoScriptCode, kFontNoLanguageCode, &atsuFontID);
	Fixed atsuSize = FloatToFixed ((float)size);
	ATSURGBAlphaColor color = {fontColor.red/255.f, fontColor.green/255.f, fontColor.blue/255.f, fontColor.alpha/255.f};
	Boolean italic = style & kItalicFace;
	Boolean underline = style & kUnderlineFace;
	ATSUAttributeTag  theTags[] =  { kATSUFontTag, kATSUSizeTag, kATSURGBAlphaColorTag, kATSUQDItalicTag, kATSUQDUnderlineTag};
	ByteCount        theSizes[] = { sizeof(ATSUFontID), sizeof(Fixed), sizeof(ATSURGBAlphaColor), sizeof (Boolean), sizeof (Boolean) };
	ATSUAttributeValuePtr theValues[] = {&atsuFontID, &atsuSize, &color, &italic, &underline};
	status = ATSUSetAttributes (atsuStyle, 5, theTags, theSizes, theValues);

	return atsuStyle;
}

#endif

//-----------------------------------------------------------------------------
BitMapPtr CDrawContext::getBitmap ()
{
	#if QUARTZ
	return (BitMapPtr)GetPortBitMapForCopyBits (GetWindowPort ((WindowRef)pWindow));
	#else
	PixMapHandle pixMap = GetPortPixMap (GetWindowPort ((WindowRef)pWindow));
	if (pixMap)
	{
		LockPixels (pixMap);
		return (BitMapPtr)*pixMap;
	}
	#endif
	return 0;
}

//-----------------------------------------------------------------------------
void CDrawContext::releaseBitmap ()
{
	#if !QUARTZ
	PixMapHandle pixMap = GetPortPixMap (GetWindowPort ((WindowRef)pWindow));
	UnlockPixels (pixMap);
	#endif
}

//-----------------------------------------------------------------------------
CGrafPtr CDrawContext::getPort ()
{
	#if QUARTZ
	if (pWindow)
		return (CGrafPtr)GetWindowPort ((WindowRef)pWindow);
	return 0;
	#else
	if (!bInitialized)
	{
		CGrafPtr OrigPort;
		GDHandle OrigDevice;
		GetGWorld (&OrigPort, &OrigDevice);
		SetGWorld ((CGrafPtr)GetWindowPort ((WindowRef)pWindow), NULL);
	
		TextMode (srcOr);
		PenMode (patCopy);
		StuffHex (&fillPattern, "\pFFFFFFFFFFFFFFFF");
	
		SetGWorld (OrigPort, OrigDevice);
		
		bInitialized = true;
	}
	return (CGrafPtr)GetWindowPort ((WindowRef)pWindow);
	#endif
}

#endif


//-----------------------------------------------------------------------------
// COffscreenContext Implementation
//-----------------------------------------------------------------------------
COffscreenContext::COffscreenContext (CDrawContext *pContext, CBitmap *pBitmapBg, bool drawInBitmap)
: CDrawContext (pContext->pFrame, NULL, NULL)
, pBitmap (0)
, pBitmapBg (pBitmapBg)
, height (20)
, width (20)
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
	#endif
		
	bDestroyPixmap = false;
	
#if WINDOWS
	#if GDIPLUS
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
	#endif

#elif MAC
	#if QUARTZ
	offscreenBitmap = 0;
	if (drawInBitmap)
	{
		if (pBitmapBg->getHandle ())
		{
			PixMapHandle pixMap = GetGWorldPixMap ((GWorldPtr)pBitmapBg->getHandle ());
			LockPixels (pixMap);
			size_t pixDepth = GetPixDepth (pixMap) / 4;
			size_t rowBytes = GetPixRowBytes (pixMap);
			gCGContext = CGBitmapContextCreate (GetPixBaseAddr (pixMap), (size_t)width, (size_t)height, pixDepth, rowBytes, GetGenericRGBColorSpace (), kCGImageAlphaPremultipliedFirst);
			if (gCGContext)
			{
				CGContextTranslateCTM (gCGContext, 0, (float)height);
				CGContextSetFillColorSpace (gCGContext, GetGenericRGBColorSpace ());
				CGContextSetStrokeColorSpace (gCGContext, GetGenericRGBColorSpace ());
				CGAffineTransform cgCTM = CGAffineTransformMake (1.0, 0.0, 0.0, -1.0, 0.0, 0.0);
				CGContextSetTextMatrix (gCGContext, cgCTM);
				CGContextSaveGState (gCGContext);
			}
		}
	}
	else
	{ // todo !!!
	}
	
	#else
	
	if (drawInBitmap)
		pWindow = pBitmapBg->getHandle ();
	else
	{
		Rect	GWRect;
		GWRect.top  = 0;
		GWRect.left = 0;
		GWRect.right  = width;
		GWRect.bottom = height;
		NewGWorld ((GWorldPtr*)&pWindow, 0, &GWRect, NULL, NULL, 0);
		bDestroyPixmap = true;
	}

	StuffHex (&fillPattern, "\pFFFFFFFFFFFFFFFF");
	#endif
        
#elif BEOS
	bDestroyPixmap = true;
	offscreenBitmap = new BBitmap (BRect (0, 0, width - 1, height - 1), B_RGB16, true, false);
	pView = new BView (BRect (0, 0, width - 1, height - 1), NULL, 0, 0);
	offscreenBitmap->Lock ();
	offscreenBitmap->AddChild (pView);

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
			fillRect (r);
		}
	}
}

//-----------------------------------------------------------------------------
COffscreenContext::COffscreenContext (CFrame *pFrame, long width, long height, const CColor backgroundColor)
: CDrawContext (pFrame, NULL, NULL)
, pBitmap (0)
, pBitmapBg (0)
, height (height)
, width (width)
, backgroundColor (backgroundColor)
{
	clipRect (0, 0, width, height);

	#if DEBUG
	gNbCOffscreenContext++;
	gBitmapAllocation += height * width;
	#endif

	bDestroyPixmap = true;

#if WINDOWS
	#if GDIPLUS
	pBitmap = new CBitmap (*pFrame, width, height);
	pGraphics = new Gdiplus::Graphics (pBitmap->getBitmap ());
	pGraphics->SetInterpolationMode (Gdiplus::InterpolationModeLowQuality);
	#else
	void *SystemWindow = pFrame->getSystemWindow ();
	void *SystemContext = GetDC ((HWND)SystemWindow);
	
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
	fillRect (r);
	drawRect (r);
	#endif

#elif MAC
	#if QUARTZ
	CGContextRef    context = NULL; 
	int             bitmapByteCount; 
	int             bitmapBytesPerRow; 

	// each pixel is represented by four bytes 
	// (8 bits each of alpha, R, G, B) 
	bitmapBytesPerRow   = width * 4; 
	bitmapByteCount     = bitmapBytesPerRow * height; 

	// create the bitmap 
	offscreenBitmap = malloc (bitmapByteCount);
	if (offscreenBitmap != NULL)
	{
		memset (offscreenBitmap, 0, bitmapByteCount);
		// create the context 
		context = CGBitmapContextCreate (offscreenBitmap,
		width, 
		height, 
		8,              // bits per component 
		bitmapBytesPerRow, 
		GetGenericRGBColorSpace (), 
		kCGImageAlphaPremultipliedFirst);

		if (context == NULL)
		{
			// the context couldn't be created for some reason, 
			// and we have no use for the bitmap without the context 
			free (offscreenBitmap);
			offscreenBitmap = 0;
		}
		else
		{
			CGContextTranslateCTM (context, 0, (float)height);
			CGContextSetFillColorSpace (context, GetGenericRGBColorSpace ());
			CGContextSetStrokeColorSpace (context, GetGenericRGBColorSpace ()); 
			CGAffineTransform cgCTM = CGAffineTransformMake (1.0, 0.0, 0.0, -1.0, 0.0, 0.0);
			CGContextSetTextMatrix (context, cgCTM);
			CGContextSaveGState (context);
			CGRect r = CGRectMake (0, 0, width, height);
			CGContextClearRect (context, r);
		}
	}
	gCGContext = context;

	CRect r (0, 0, width, height);
	setFillColor (backgroundColor);
	setFrameColor (backgroundColor);
	fillRect (r);
	drawRect (r);


	#else
	QDErr	err;
	Rect	GWRect;
	
	GWRect.top = GWRect.left = 0;
	GWRect.right = width;
	GWRect.bottom = height;
	err = NewGWorld ((GWorldPtr*) &pWindow, 0, &GWRect, NULL, NULL, 0);
	if (err)
		pWindow = NULL;

	StuffHex (&fillPattern, "\pFFFFFFFFFFFFFFFF");

	CRect r (0, 0, width, height);
	setFillColor (backgroundColor);
	setFrameColor (backgroundColor);
	fillRect (r);
	drawRect (r);
	#endif
        
#elif BEOS
	BRect frame (0, 0, width - 1, height - 1);
	offscreenBitmap = new BBitmap (frame, B_RGB16, true, false);
	pView = new BView (BRect (0, 0, width - 1, height - 1), NULL, 0, 0);
	offscreenBitmap->Lock ();
	offscreenBitmap->AddChild (pView);
	if (backgroundColor.red != 255 || backgroundColor.green != 255 || backgroundColor.blue != 255)
	{
		rgb_color c = { backgroundColor.red, backgroundColor.green, backgroundColor.blue, 255 };
		pView->SetHighColor (c);
		pView->FillRect (frame);
	}
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

#elif MAC
	#if QUARTZ
	if (gCGContext)
	{
		CGContextRestoreGState (gCGContext);
		CGContextRelease (gCGContext);
	}
	gCGContext = 0;
	if (offscreenBitmap)
		free (offscreenBitmap);
	else if (pBitmapBg && pBitmapBg->getHandle ())
	{
		PixMapHandle pixMap = GetGWorldPixMap ((GWorldPtr)pBitmapBg->getHandle ());
		UnlockPixels (pixMap);
	}
	#else
	if (bDestroyPixmap && pWindow)
		DisposeGWorld ((GWorldPtr)pWindow);
	#endif
        
#elif BEOS
	delete offscreenBitmap;
	pView = 0;	// deleted because attached to the offscreen
#endif
}

//-----------------------------------------------------------------------------
void COffscreenContext::copyTo (CDrawContext* pContext, CRect& srcRect, CPoint destOffset)
{
#if WINDOWS
	#if GDIPLUS
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
	#if QUARTZ
	if (!pBitmapBg)
		return;
	#else
	if (!pWindow)
		return;
	#endif
	
	Rect source, dest;
	RGBColor savedForeColor, savedBackColor;
	
	source.left   = (short)(srcRect.left + pContext->offset.h + pContext->offsetScreen.h);
	source.top    = (short)(srcRect.top + pContext->offset.v + pContext->offsetScreen.v);
	source.right  = (short)(source.left + srcRect.right - srcRect.left);
	source.bottom = (short)(source.top + srcRect.bottom - srcRect.top);
	
	dest.left   = (short)destOffset.h;
	dest.top    = (short)destOffset.v;
	dest.right  = (short)(dest.left + srcRect.right - srcRect.left);
	dest.bottom = (short)(dest.top + srcRect.bottom - srcRect.top);

	GetForeColor (&savedForeColor);
	GetBackColor (&savedBackColor);
	::BackColor (whiteColor);
	::ForeColor (blackColor);

	CopyBits (pContext->getBitmap (), getBitmap (), &source, &dest, srcCopy, 0L);
	releaseBitmap ();
	pContext->releaseBitmap ();

	RGBForeColor (&savedForeColor);
	RGBBackColor (&savedBackColor);
#endif
}

//-----------------------------------------------------------------------------
void COffscreenContext::copyFrom (CDrawContext *pContext, CRect destRect, CPoint srcOffset)
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

#elif MAC
	#if QUARTZ
	if (!gCGContext)
		return;
	CGContextRef context = pContext->beginCGContext ();
	if (context)
	{
		size_t pixRowBytes = CGBitmapContextGetBytesPerRow (gCGContext);
		short pixDepth = CGBitmapContextGetBitsPerPixel (gCGContext);
		size_t size = pixRowBytes * CGBitmapContextGetHeight (gCGContext);

		CGImageRef image = 0;
		CGDataProviderRef provider = CGDataProviderCreateWithData (NULL, CGBitmapContextGetData (gCGContext), size, NULL);
		CGImageAlphaInfo alphaInfo = CGBitmapContextGetAlphaInfo (gCGContext);
		image = CGImageCreate (CGBitmapContextGetWidth (gCGContext), CGBitmapContextGetHeight (gCGContext), 8 , pixDepth, pixRowBytes, GetGenericRGBColorSpace (), alphaInfo, provider, NULL, 0, kCGRenderingIntentDefault);
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
		CGDataProviderRelease (provider);
		
		pContext->releaseCGContext (context);
	}
	#else
	if (!pWindow)
		return;

	Rect source, dest;
	RGBColor savedForeColor, savedBackColor;
	
	source.left   = srcOffset.h;
	source.top    = srcOffset.v;
	source.right  = source.left + destRect.right - destRect.left;
	source.bottom = source.top + destRect.bottom - destRect.top;
	
	dest.top    = destRect.top + pContext->offset.v;
	dest.left   = destRect.left + pContext->offset.h;
	dest.bottom = destRect.bottom + pContext->offset.v;
	dest.right  = destRect.right + pContext->offset.h;

	GetForeColor (&savedForeColor);
	GetBackColor (&savedBackColor);
	::BackColor (whiteColor);
	::ForeColor (blackColor);

	CopyBits (getBitmap (), pContext->getBitmap (), &source, &dest, srcCopy, 0L);
	#if MACX
	QDAddRectToDirtyRegion (pContext->getPort (), &dest);
	#endif
	releaseBitmap ();
	pContext->releaseBitmap ();

	RGBForeColor (&savedForeColor);
	RGBBackColor (&savedBackColor);
	#endif

#elif BEOS
	pContext->pView->SetDrawingMode (B_OP_COPY);
	BRect destination (destRect.left, destRect.top, destRect.right - 1, destRect.bottom - 1);
	BRect source = destination;
	source.OffsetTo (srcOffset.h, srcOffset.v);
	pView->Sync ();
	pContext->pView->DrawBitmap (offscreenBitmap, source, destination);
#endif
}

//-----------------------------------------------------------------------------
#if MAC
#if QUARTZ
//-----------------------------------------------------------------------------
CGImageRef COffscreenContext::getCGImage () const
{
	#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
	if (CGBitmapContextCreateImage && gCGContext)
	{
		return CGBitmapContextCreateImage (gCGContext);
	}
	#endif
	return 0;
}

#endif

//-----------------------------------------------------------------------------
BitMapPtr COffscreenContext::getBitmap ()
{
	#if QUARTZ
	return pBitmapBg ? (BitMapPtr)GetPortBitMapForCopyBits ((GWorldPtr)pBitmapBg->getHandle ()) : 0;
	#else
	PixMapHandle pixMap = GetGWorldPixMap ((GWorldPtr)pWindow);
	if (pixMap)
	{
		LockPixels (pixMap);
		return (BitMapPtr)*pixMap;
	}
	return 0;
	#endif
}

//-----------------------------------------------------------------------------
void COffscreenContext::releaseBitmap ()
{
	#if QUARTZ
	#else
	PixMapHandle pixMap = GetGWorldPixMap ((GWorldPtr)pWindow);
	UnlockPixels (pixMap);
	#endif
}

#if !QUARTZ
//-----------------------------------------------------------------------------
CGrafPtr COffscreenContext::getPort ()
{
	if (!bInitialized)
		bInitialized = true;

	return (CGrafPtr)pWindow;
}
#endif // QUARTZ
#endif // MAC

//-----------------------------------------------------------------------------
class CAttributeListEntry
{
public:
	CAttributeListEntry (long size, CViewAttributeID id)
	: nextEntry (0)
	, pointer (0)
	, sizeOfPointer (size)
	, id (id)
	{
		pointer = malloc (size);
	}

	~CAttributeListEntry ()
	{
		if (pointer)
			free (pointer);
	}

	const CViewAttributeID getID () const { return id; }
	const long getSize () const { return sizeOfPointer; }
	void* getPointer () const { return pointer; }
	CAttributeListEntry* getNext () const { return nextEntry; }
	
	void setNext (CAttributeListEntry* entry) { nextEntry = entry; }

protected:
	CAttributeListEntry () : nextEntry (0), pointer (0), sizeOfPointer (0), id (0) {}

	CAttributeListEntry* nextEntry;
	void* pointer;
	long sizeOfPointer;
	CViewAttributeID id;
};

//-----------------------------------------------------------------------------
char* kMsgCheckIfViewContainer	= "kMsgCheckIfViewContainer";

//-----------------------------------------------------------------------------
// CView
//-----------------------------------------------------------------------------
/*! @class CView
base class of all view objects
*/
//-----------------------------------------------------------------------------
CView::CView (const CRect& size)
: size (size)
, mouseableArea (size)
, pParentFrame (0)
, pParentView (0)
, bDirty (false)
, bMouseEnabled (true)
, bTransparencyEnabled (false)
, bWantsFocus (false)
, bIsAttached (false)
, pBackground (0)
, pAttributeList (0)
{
	#if DEBUG
	gNbCView++;
	#endif
}

//-----------------------------------------------------------------------------
CView::~CView ()
{
	if (pBackground)
		pBackground->forget ();

	if (pAttributeList)
	{
		CAttributeListEntry* entry = pAttributeList;
		while (entry)
		{
			CAttributeListEntry* nextEntry = entry->getNext ();
			delete entry;
			entry = nextEntry;
		}
	}
	#if DEBUG
	gNbCView--;
	#endif
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
void CView::getMouseLocation (CDrawContext* context, CPoint &point)
{
	if (context)
	{
		if (pParentView && pParentView->notify (this, kMsgCheckIfViewContainer) == kMessageNotified)
		{
			CCoord save[4];
			((CViewContainer*)pParentView)->modifyDrawContext (save, context);
			pParentView->getMouseLocation (context, point);
			((CViewContainer*)pParentView)->restoreDrawContext (context, save);
		}
		else
			context->getMouseLocation (point);
	}
}
#endif

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
void CView::getFrameTopLeftPos (CPoint& topLeft) const
{
	topLeft.h += size.left;
	topLeft.v += size.top;
	if (pParentView && pParentView->notify (0, kMsgCheckIfViewContainer) == kMessageNotified)
		pParentView->getFrameTopLeftPos (topLeft);
}
#endif

//-----------------------------------------------------------------------------
CPoint& CView::frameToLocal (CPoint& point) const
{
	if (pParentView && pParentView->isTypeOf ("CViewContainer"))
		return pParentView->frameToLocal (point);
	return point;
}

//-----------------------------------------------------------------------------
CPoint& CView::localToFrame (CPoint& point) const
{
	if (pParentView && pParentView->isTypeOf ("CViewContainer"))
		return pParentView->localToFrame (point);
	return point;
}

#if !VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
//-----------------------------------------------------------------------------
void CView::redraw ()
{
	if (pParentFrame)
		pParentFrame->draw (this);
}

//-----------------------------------------------------------------------------
void CView::redrawRect (CDrawContext* context, const CRect& rect)
{
	// we always pass it on to the parent view as it knows what else must be drawn (needed for nested view containers)
	if (pParentView)
		pParentView->redrawRect (context, rect);
	else if (pParentFrame)
		pParentFrame->drawRect (context, rect);
}
#endif

#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
//-----------------------------------------------------------------------------
void CView::invalidRect (CRect rect)
{
	if (bIsAttached)
	{
		if (pParentView)
			pParentView->invalidRect (rect);
		else if (pParentFrame)
			pParentFrame->invalidRect (rect);
	}
}
#endif

//-----------------------------------------------------------------------------
void CView::draw (CDrawContext *pContext)
{
	if (pBackground)
	{
		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, size);
		else
			pBackground->draw (pContext, size);
	}
	setDirty (false);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
void CView::mouse (CDrawContext *pContext, CPoint &where, long buttons)
{}
#endif

//-----------------------------------------------------------------------------
bool CView::onWheel (const CPoint &where, const float &distance, const long &buttons)
{
	return false;
}

//------------------------------------------------------------------------
bool CView::onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const long &buttons)
{
	if (axis == kMouseWheelAxisX)
		return onWheel (where, distance*-1, buttons);
	return onWheel (where, distance, buttons);
}

#if !VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
//------------------------------------------------------------------------
void CView::update (CDrawContext *pContext)
{
	if (isDirty ())
	{
		if (pContext)
			redrawRect (pContext, size);
		else
			redraw ();
		setDirty (false);
	}
}
#endif

//------------------------------------------------------------------------------
long CView::onKeyDown (VstKeyCode& keyCode)
{
	return -1;
}

//------------------------------------------------------------------------------
long CView::onKeyUp (VstKeyCode& keyCode)
{
	return -1;
}

//------------------------------------------------------------------------------
long CView::notify (CView* sender, const char* message)
{
	return kMessageUnknown;
}

//------------------------------------------------------------------------------
void CView::looseFocus ()
{}

//------------------------------------------------------------------------------
void CView::takeFocus ()
{}

//------------------------------------------------------------------------------
void CView::setViewSize (CRect &rect)
{
	size = rect;
	setDirty ();
}

//------------------------------------------------------------------------------
CRect CView::getVisibleSize () const
{
	if (pParentView && pParentView->isTypeOf("CViewContainer"))
		return ((CViewContainer*)pParentView)->getVisibleSize (size);
	else if (pParentFrame)
		return pParentFrame->getVisibleSize (size);
	return CRect (0, 0, 0, 0);
}

//-----------------------------------------------------------------------------
VSTGUIEditorInterface *CView::getEditor () const
{ 
	return pParentFrame ? pParentFrame->getEditor () : 0; 
}

//-----------------------------------------------------------------------------
void CView::setBackground (CBitmap *background)
{
	if (pBackground)
		pBackground->forget ();
	pBackground = background;
	if (pBackground)
		pBackground->remember ();
	setDirty (true);
}

//-----------------------------------------------------------------------------
const CViewAttributeID kCViewAttributeReferencePointer = 'cvrp';
const CViewAttributeID kCViewTooltipAttribute = 'cvtt';

//-----------------------------------------------------------------------------
/**
 * @param id the ID of the Attribute
 * @param outSize on return the size of the attribute
 */
bool CView::getAttributeSize (const CViewAttributeID id, long& outSize) const
{
	if (pAttributeList)
	{
		CAttributeListEntry* entry = pAttributeList;
		while (entry)
		{
			if (entry->getID () == id)
				break;
			entry = entry->getNext ();
		}
		if (entry)
		{
			outSize = entry->getSize ();
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
/**
 * @param id the ID of the Attribute
 * @param inSize the size of the outData pointer
 * @param outData a pointer where to copy the attribute data
 * @param outSize the size in bytes which was copied into outData
 */
bool CView::getAttribute (const CViewAttributeID id, const long inSize, void* outData, long& outSize) const
{
	if (pAttributeList)
	{
		CAttributeListEntry* entry = pAttributeList;
		while (entry)
		{
			if (entry->getID () == id)
				break;
			entry = entry->getNext ();
		}
		if (entry && inSize >= entry->getSize ())
		{
			outSize = entry->getSize ();
			memcpy (outData, entry->getPointer (), outSize);
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
/**
 * copies data into the attribute. If it does not exist, creates a new attribute.
 * @param id the ID of the Attribute
 * @param inSize the size of the outData pointer
 * @param inData a pointer to the data
 */
bool CView::setAttribute (const CViewAttributeID id, const long inSize, const void* inData)
{
	CAttributeListEntry* lastEntry = 0;
	if (pAttributeList)
	{
		CAttributeListEntry* entry = pAttributeList;
		while (entry)
		{
			if (entry->getID () == id)
				break;
			if (entry->getNext () == 0)
				lastEntry = entry;
			entry = entry->getNext ();
		}
		if (entry)
		{
			if (entry->getSize () >= inSize)
			{
				memcpy (entry->getPointer (), inData, inSize);
				return true;
			}
			else
				return false;
		}
	}
	
	// create a new attribute
	CAttributeListEntry* newEntry = new CAttributeListEntry (inSize, id);
	memcpy (newEntry->getPointer (), inData, inSize);
	if (lastEntry)
		lastEntry->setNext (newEntry);
	else if (!pAttributeList)
		pAttributeList = newEntry;
	else
	{
		delete newEntry;
		return false;
	}
	return true;
}

#if DEBUG
//-----------------------------------------------------------------------------
void CView::dumpInfo ()
{
	CRect viewRect = getViewSize (viewRect);
	DebugPrint ("left:%4d, top:%4d, width:%4d, height:%4d ", viewRect.left, viewRect.top, viewRect.getWidth (), viewRect.getHeight ());
	if (getMouseEnabled ())
		DebugPrint ("(Mouse Enabled) ");
	if (getTransparency ())
		DebugPrint ("(Transparent) ");
	CRect mouseRect = getMouseableArea (mouseRect);
	if (mouseRect != viewRect)
		DebugPrint (" (Mouseable Area: left:%4d, top:%4d, width:%4d, height:%4d ", mouseRect.left, mouseRect.top, mouseRect.getWidth (), mouseRect.getHeight ());
}
#endif

#define FOREACHSUBVIEW for (CCView *pSv = pFirstView; pSv; pSv = pSv->pNext) {CView *pV = pSv->pView;
#define FOREACHSUBVIEW_REVERSE(reverse) for (CCView *pSv = reverse ? pLastView : pFirstView; pSv; pSv = reverse ? pSv->pPrevious : pSv->pNext) {CView *pV = pSv->pView;
#define ENDFOR }

//-----------------------------------------------------------------------------
// CFrame Implementation
//-----------------------------------------------------------------------------
/*! @class CFrame
It creates a platform dependend view object. 
On classic Mac OS it just draws into the provided window.
On Mac OS X it is a ControlRef. 
On Windows it's a WS_CHILD Window.
*/
CFrame::CFrame (const CRect &inSize, void *inSystemWindow, VSTGUIEditorInterface *inEditor)
: CViewContainer (inSize, 0, 0)
, pEditor (inEditor)
, pSystemWindow (inSystemWindow)
, pModalView (0)
, pFocusView (0)
, tooltipView (0)
, bFirstDraw (true)
, bDropActive (false)
, bUpdatesDisabled (false)
, pFrameContext (0)
, bAddedWindow (false)
, pVstWindow (0)
, defaultCursor (0)
{
	setOpenFlag (true);
	bIsAttached = true;
	
	pParentFrame = this;

#if WINDOWS
	pHwnd = 0;
	dropTarget = 0;
	backBuffer = 0;
	OleInitialize (0);

	#if DYNAMICALPHABLEND
	pfnAlphaBlend = 0;
	pfnTransparentBlt = 0;

	hInstMsimg32dll = LoadLibrary ("msimg32.dll");
	if (hInstMsimg32dll)
	{
		pfnAlphaBlend = (PFNALPHABLEND)GetProcAddress (hInstMsimg32dll, "AlphaBlend");

		// get OS version
		memset (&gSystemVersion, 0, sizeof (gSystemVersion));
		gSystemVersion.dwOSVersionInfoSize = sizeof (gSystemVersion);

		if (GetVersionEx ((OSVERSIONINFO *)&gSystemVersion))
		{
			// Is this win NT or better?
			if (gSystemVersion.dwPlatformId >= VER_PLATFORM_WIN32_NT)
			{
				// Yes, then TransparentBlt doesn't have the memory-leak and can be safely used
				pfnTransparentBlt = (PFNTRANSPARENTBLT)GetProcAddress (hInstMsimg32dll, "TransparentBlt");
			}
		}
	}
	#endif	// DYNAMICALPHABLEND
    
#elif BEOS
	pPlugView = NULL;
#endif

	initFrame (pSystemWindow);

#if WINDOWS

#elif MAC
	Gestalt (gestaltSystemVersion, &pSystemVersion);
	#if QUARTZ
	pFrameContext = 0;
	#else
	pFrameContext = new CDrawContext (this, getSystemWindow (), getSystemWindow ());
	pFrameContext->offset.h = size.left;
	pFrameContext->offset.v = size.top;
	#endif
	
#endif
}

//-----------------------------------------------------------------------------
CFrame::CFrame (const CRect& inSize, const char* inTitle, VSTGUIEditorInterface* inEditor, const long inStyle)
: CViewContainer (inSize, 0, 0)
, pEditor (inEditor)
, pSystemWindow (0)
, pModalView (0)
, pFocusView (0)
, tooltipView (0)
, bFirstDraw (true)
, bDropActive (false)
, bUpdatesDisabled (false)
, pFrameContext (0)
, pVstWindow (0) 
, defaultCursor (0)
{
	bAddedWindow  = true;
	setOpenFlag (false);
	pParentFrame = this;
	bIsAttached = true;

#if WINDOWS
	pHwnd = 0;
	dropTarget = 0;
	backBuffer = 0;
	OleInitialize (0);

	#if DYNAMICALPHABLEND
	pfnAlphaBlend = 0;
	pfnTransparentBlt = 0;

	hInstMsimg32dll = LoadLibrary ("msimg32.dll");
	if (hInstMsimg32dll)
	{
		pfnAlphaBlend = (PFNALPHABLEND)GetProcAddress (hInstMsimg32dll, "AlphaBlend");

		// get OS version
		OSVERSIONINFOEX	osvi;

		memset (&osvi, 0, sizeof (osvi));
		osvi.dwOSVersionInfoSize = sizeof (osvi);

		if (GetVersionEx ((OSVERSIONINFO *)&osvi))
		{
			// Is this win NT or better?
			if (osvi.dwPlatformId >= VER_PLATFORM_WIN32_NT)
			{
				// Yes, then TransparentBlt doesn't have the memory-leak and can be safely used
				pfnTransparentBlt = (PFNTRANSPARENTBLT)GetProcAddress (hInstMsimg32dll, "TransparentBlt");
			}
		}
	}
	#endif
    
#elif BEOS
	pPlugView = NULL;

#endif

	#if ENABLE_VST_EXTENSION_IN_VSTGUI
	pVstWindow = (VstWindow*)malloc (sizeof (VstWindow));
	strcpy (((VstWindow*)pVstWindow)->title, inTitle);
	((VstWindow*)pVstWindow)->xPos   = (short)size.left;
	((VstWindow*)pVstWindow)->yPos   = (short)size.top;
	((VstWindow*)pVstWindow)->width  = (short)size.width ();
	((VstWindow*)pVstWindow)->height = (short)size.height ();
	((VstWindow*)pVstWindow)->style  = inStyle;
	((VstWindow*)pVstWindow)->parent     = 0;
	((VstWindow*)pVstWindow)->userHandle = 0;
	((VstWindow*)pVstWindow)->winHandle  = 0;
	#endif
}

//-----------------------------------------------------------------------------
CFrame::~CFrame ()
{
	if (pModalView)
		removeView (pModalView, false);

	setCursor (kCursorDefault);

	setDropActive (false);

	if (pFrameContext)
		pFrameContext->forget ();

#if WINDOWS
	OleUninitialize ();

	if (backBuffer)
		backBuffer->forget ();

	#if DYNAMICALPHABLEND
	if (hInstMsimg32dll)
		FreeLibrary (hInstMsimg32dll);
	#endif

	#if GDIPLUS
	Gdiplus::GdiplusShutdown(gdiplusToken);
	#endif

	if (pHwnd)
	{
		SetWindowLong ((HWND)pHwnd, GWL_USERDATA, (long)NULL);
		DestroyWindow ((HWND)pHwnd);

		ExitWindowClass ();
	}

#endif
	
	if (bAddedWindow)
		close ();
	if (pVstWindow)
		free (pVstWindow);

#if BEOS
	CBitmap::closeResource ();	// must be done only once at the end of the story.
#endif

#if MAC && QUARTZ
	if (mouseEventHandler)
		RemoveEventHandler (mouseEventHandler);
	if (controlRef)
		DisposeControl (controlRef);
	if (controlSpec.u.classRef)
	{
		OSStatus status = UnregisterToolboxObjectClass ((ToolboxObjectClassRef)controlSpec.u.classRef);
		if (status != noErr)
			fprintf (stderr, "UnregisterToolboxObjectClass failed : %d\n", (int)status);
	}
#endif
	pParentFrame = 0;
}

//-----------------------------------------------------------------------------
bool CFrame::open (CPoint *point)
{
#if ENABLE_VST_EXTENSION_IN_VSTGUI
	if (!bAddedWindow)
		return false;
	if (getOpenFlag ())
	{
		#if WINDOWS
		BringWindowToTop (GetParent (GetParent ((HWND)getSystemWindow ())));

		#elif BEOS
		pPlugView->Window ()->Activate (true);
		#endif
		return false;
	}

	if (pVstWindow)
	{
		if (point)
		{
			((VstWindow*)pVstWindow)->xPos = (short)point->h;
			((VstWindow*)pVstWindow)->yPos = (short)point->v;
		}
		AudioEffectX *pAudioEffectX = (AudioEffectX*)(((AEffGUIEditor*)pEditor)->getEffect ());
		pSystemWindow = pAudioEffectX->openWindow ((VstWindow*)pVstWindow);
	}

	if (pSystemWindow)
	{
		if (initFrame (pSystemWindow))
			setOpenFlag (true);
	}

	return getOpenFlag ();
#else
	return false;
#endif
}

//-----------------------------------------------------------------------------
bool CFrame::close ()
{
#if ENABLE_VST_EXTENSION_IN_VSTGUI
	if (!bAddedWindow || !getOpenFlag () || !pSystemWindow)
		return false;

	AudioEffectX *pAudioEffectX = (AudioEffectX*)(((AEffGUIEditor*)pEditor)->getEffect ());
	pAudioEffectX->closeWindow ((VstWindow*)pVstWindow);

	pSystemWindow = 0;

	return true;
#else
	return false;
#endif
}

//-----------------------------------------------------------------------------
bool CFrame::initFrame (void *systemWin)
{
	if (!systemWin)
		return false;
	
#if WINDOWS

	InitWindowClass ();
	pHwnd = CreateWindowEx (0, gClassName, "Window",
			 WS_CHILD | WS_VISIBLE, 
			 0, 0, size.width (), size.height (), 
			 (HWND)pSystemWindow, NULL, GetInstance (), NULL);

	SetWindowLongPtr ((HWND)pHwnd, GWLP_USERDATA, (LONG_PTR)this);

#if GDIPLUS

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

#endif

#elif MAC

	#if QUARTZ
	dragEventHandler = 0;
	if (!registerWithToolbox ())
		return false;

	hasFocus = false;
	Rect r = {(short)size.top, (short)size.left, (short)size.bottom, (short)size.right};
	OSStatus status = CreateCustomControl (NULL, &r, &controlSpec, NULL, &controlRef);
	if (status != noErr)
	{
		fprintf (stderr, "Could not create Control : %d\n", (int)status);
		return false;
	}
	EventTypeSpec keyWorkaroundEvents[] = {
		{ kEventClassTextInput, kEventTextInputUnicodeForKeyEvent }
	};
	InstallWindowEventHandler ((WindowRef)systemWin, carbonEventHandler, GetEventTypeCount (keyWorkaroundEvents), keyWorkaroundEvents, this, NULL);
	EventTypeSpec mouseEvents[] = {
		{ kEventClassMouse, kEventMouseDown },
		{ kEventClassMouse, kEventMouseUp },
		{ kEventClassMouse, kEventMouseMoved },
		{ kEventClassMouse, kEventMouseDragged },
	};
	InstallWindowEventHandler ((WindowRef)systemWin, carbonMouseEventHandler, GetEventTypeCount (mouseEvents), mouseEvents, this, &mouseEventHandler);
	
	SetControlDragTrackingEnabled (controlRef, true);
	SetAutomaticControlDragTrackingEnabledForWindow ((WindowRef)systemWin, true);
	#if !AU // for AudioUnits define AU and embed the controlRef at your AUCarbonViewBase
	if (isWindowComposited ((WindowRef)systemWin)) 
	{
		HIViewRef contentView;
		HIViewRef rootView = HIViewGetRoot ((WindowRef)systemWin);
		if (HIViewFindByID (rootView, kHIViewWindowContentID, &contentView) != noErr)
			contentView = rootView;
		HIViewAddSubview (contentView, controlRef);
	}
	else
	{
		ControlRef rootControl;
		GetRootControl ((WindowRef)systemWin, &rootControl);
		if (rootControl == NULL)
			CreateRootControl ((WindowRef)systemWin, &rootControl);
		EmbedControl(controlRef, rootControl);	
	}
	#endif
	size.offset (-size.left, -size.top);
	mouseableArea.offset (-size.left, -size.top);
	#endif
	
#elif BEOS
	BView* parentView = (BView*) pSystemWindow;
	BRect frame = parentView->Frame ();
	frame.OffsetTo (B_ORIGIN);
	pPlugView = new PlugView (frame, this);
	parentView->AddChild (pPlugView);
#endif

	setDropActive (true);

	return true;
}

//-----------------------------------------------------------------------------
bool CFrame::setDropActive (bool val)
{	
	if (!bDropActive && !val)
		return true;

#if WINDOWS
	if (!pHwnd)
		return false;
	if (dropTarget)
	{
		RevokeDragDrop ((HWND)pHwnd);
		dropTarget = 0;
	}
	if (val)
	{
		dropTarget = createDropTarget (this);
		RegisterDragDrop ((HWND)pHwnd, (IDropTarget*)dropTarget);
	}

#elif MAC
#if MAC_OLD_DRAG
	if (!isWindowComposited ((WindowRef)pSystemWindow))
	{
		if (val)
			install_drop (this);
		else
			remove_drop (this);
	}
#endif
#endif

	bDropActive = val;
	return true;
}

//-----------------------------------------------------------------------------
CDrawContext* CFrame::createDrawContext ()
{
	if (pFrameContext)
	{
		pFrameContext->remember ();
		return pFrameContext;
	}

	CDrawContext* pContext = 0;
	#if WINDOWS || MAC
	pContext = new CDrawContext (this, NULL, getSystemWindow ());

	#elif BEOS
	pContext = new CDrawContext (this, pPlugView, 0);
	#endif
	
	return pContext;
}

//-----------------------------------------------------------------------------
void CFrame::draw (CDrawContext *pContext)
{
	if (bFirstDraw)
		bFirstDraw = false;
	
	if (!pContext)
		pContext = pFrameContext;

	// draw the background and the children
	CViewContainer::draw (pContext);
}

//-----------------------------------------------------------------------------
void CFrame::drawRect (CDrawContext *pContext, const CRect& updateRect)
{
	if (bFirstDraw)
		bFirstDraw = false;

	if (!pContext)
		pContext = pFrameContext;

	if (pContext)
		pContext->remember ();
	else
		pContext = createDrawContext ();

	CRect oldClip;
	pContext->getClipRect (oldClip);
	CRect newClip (updateRect);
	newClip.bound (oldClip);
	pContext->setClipRect (newClip);
	
	// draw the background and the children
	if (updateRect.getWidth () > 0 && updateRect.getHeight () > 0)
		CViewContainer::drawRect (pContext, updateRect);

	pContext->setClipRect (oldClip);

	pContext->forget ();
}

//-----------------------------------------------------------------------------
void CFrame::draw (CView *pView)
{
	CView *pViewToDraw = 0;

		// Search it in the view list
	if (pView && isChild(pView))
		pViewToDraw = pView;

	CDrawContext *pContext = createDrawContext ();
	if (pContext)
	{
		if (pViewToDraw)
			pViewToDraw->draw (pContext);
		else
			draw (pContext);

		pContext->forget ();
	}
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
void CFrame::mouse (CDrawContext *pContext, CPoint &where, long buttons)
{
	if (!pContext)
		pContext = pFrameContext;

#if 1
	// emulate new mouse handling for testing
	if (buttons == -1)
		buttons = pContext->getMouseButtons ();
	long origButtons = buttons;
	if (onMouseDown (where, buttons) == kMouseEventHandled)
	{
		CPoint where2 (where);
		while (true)
		{
			doIdleStuff ();
			buttons = pContext->getMouseButtons ();
			pContext->getMouseLocation (where);
			if (origButtons & kLButton && !(buttons & kLButton) || origButtons & kRButton && !(buttons & kRButton) || origButtons & kMButton && !(buttons & kMButton))
			{
				onMouseUp (where, buttons);
				break;
			}
			if (where != where2)
			{
				onMouseMoved (where, buttons);
			}
		}
	}
	
#else
	if (pFocusView)
		setFocusView (NULL);

	if (buttons == -1 && pContext)
		buttons = pContext->getMouseButtons ();

	if (pModalView)
	{
		if (pModalView->hitTest (where, buttons))
			pModalView->mouse (pContext, where, buttons);
	}
	else 
	{
		CViewContainer::mouse (pContext, where, buttons);
	}
#endif
}
#endif

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::onMouseDown (CPoint &where, const long& buttons)
{
	mouseDownView = 0;
	if (pFocusView)
		setFocusView (NULL);
	if (pModalView)
	{
		if (pModalView->hitTest (where, buttons))
		{
			CMouseEventResult result = pModalView->onMouseDown (where, buttons);
			if (result == kMouseEventHandled)
			{
				mouseDownView = pModalView;
				return kMouseEventHandled;
			}
			#if VSTGUI_ENABLE_DEPRECATED_METHODS
			else if (result == kMouseEventNotImplemented)
			{
				CDrawContext* context = createDrawContext ();
				pModalView->mouse (context, where, buttons);
				context->forget ();
				return kMouseEventHandled;
			}
			#endif
		}
	}
	else
		return CViewContainer::onMouseDown (where, buttons);
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::onMouseUp (CPoint &where, const long& buttons)
{
	return CViewContainer::onMouseUp (where, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::onMouseMoved (CPoint &where, const long& buttons)
{
	return CViewContainer::onMouseMoved (where, buttons);
}

//-----------------------------------------------------------------------------
long CFrame::onKeyDown (VstKeyCode& keyCode)
{
	long result = -1;

	if (pFocusView)
		result = pFocusView->onKeyDown (keyCode);

	if (result == -1 && pModalView)
		result = pModalView->onKeyDown (keyCode);

	if (result == -1 && keyCode.virt == VKEY_TAB)
		result = advanceNextFocusView (pFocusView, (keyCode.modifier & MODIFIER_SHIFT) ? true : false) ? 1 : -1;

	return result;
}

//-----------------------------------------------------------------------------
long CFrame::onKeyUp (VstKeyCode& keyCode)
{
	long result = -1;

	if (pFocusView)
		result = pFocusView->onKeyUp (keyCode);

	if (result == -1 && pModalView)
		result = pModalView->onKeyUp (keyCode);

	return result;
}

//------------------------------------------------------------------------
bool CFrame::onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const long &buttons)
{
	bool result = false;

	CView *view = pModalView ? pModalView : getViewAt (where);
	if (view)
	{
		result = view->onWheel (where, axis, distance, buttons);
	#if BEOS
		pPlugView->UnlockLooper ();
	#endif
	}
	return result;
}

//-----------------------------------------------------------------------------
bool CFrame::onWheel (const CPoint &where, const float &distance, const long &buttons)
{
	return onWheel (where, kMouseWheelAxisY, distance, buttons);
}

#if !VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
//-----------------------------------------------------------------------------
void CFrame::update (CDrawContext *pContext)
{
	if (!getOpenFlag () || updatesDisabled ())
		return;

	#if WINDOWS && USE_ALPHA_BLEND
	CDrawContext* oldFrameContext = pFrameContext;
	CDrawContext* dc = pFrameContext = getBackBuffer ();
	#else
	CDrawContext* dc = pContext;
	#endif

	if (bDirty)
	{
		draw (dc);
		setDirty (false);
	}
	else
	{
		CRect oldClipRect;
		dc->getClipRect (oldClipRect);
		if (pModalView && pModalView->isDirty ())
			pModalView->update (dc);
		FOREACHSUBVIEW
			CRect viewSize (pV->size);
			viewSize.bound (oldClipRect);
			dc->setClipRect (viewSize);
			pV->update (dc);
		ENDFOR
		dc->setClipRect (oldClipRect);
	}

	#if MACX && !QUARTZ
	if (QDIsPortBufferDirty (GetWindowPort ((WindowRef)pSystemWindow)))
	{
		QDFlushPortBuffer (GetWindowPort ((WindowRef)pSystemWindow), NULL);
	}
	#endif
	#if WINDOWS && USE_ALPHA_BLEND
	backBuffer->copyFrom (pContext, size);
	pFrameContext = oldFrameContext;
	#endif
}
#endif

//-----------------------------------------------------------------------------
void CFrame::idle ()
{
	if (!getOpenFlag ())
		return;

	#if MAC
	// if the window is collapsed, we don't need to draw anything
	if (pSystemWindow && IsWindowCollapsed ((WindowRef)pSystemWindow))
		return;
	#endif
	
	// don't do an idle before a draw
	if (bFirstDraw)
		return;

	if (!isDirty ())
		return;

	#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
	invalidateDirtyViews ();

	#else

	#if BEOS
	if (pPlugView->LockLooperWithTimeout (0) != B_OK)
		return;
	#endif

	CDrawContext *pContext = createDrawContext ();
	
	update (pContext);

	pContext->forget ();

	#if BEOS
	pPlugView->UnlockLooper ();
	#endif
	#endif
}

//-----------------------------------------------------------------------------
void CFrame::doIdleStuff ()
{
#if (MAC && QUARTZ)
	if (pFrameContext)
		pFrameContext->synchronizeCGContext ();
#endif
	if (pEditor)
		pEditor->doIdleStuff ();
}

//-----------------------------------------------------------------------------
unsigned long CFrame::getTicks () const
{
	#if MAC
	return (TickCount () * 1000) / 60;
	
	#elif WINDOWS
	return (unsigned long)GetTickCount ();
	
	#elif BEOS
	return (system_time () / 1000);
	#endif

	return 0;
}

//-----------------------------------------------------------------------------
long CFrame::getKnobMode () const
{
	return pEditor->getKnobMode ();
}

//-----------------------------------------------------------------------------
#if WINDOWS
COffscreenContext* CFrame::getBackBuffer ()
{
	#if WINDOWS && USE_ALPHA_BLEND
	if (!backBuffer)
		backBuffer = new COffscreenContext (this, size.width (), size.height ());
	#endif

	return backBuffer;
}

HWND CFrame::getOuterWindow () const
{
	int diffWidth, diffHeight;
	RECT  rctTempWnd, rctPluginWnd;
	HWND  hTempWnd = (HWND)pHwnd;
	GetWindowRect (hTempWnd, &rctPluginWnd);
    
	while (hTempWnd != NULL)
	{
		// Looking for caption bar
		if (GetWindowLong (hTempWnd, GWL_STYLE) & WS_CAPTION)
			return hTempWnd;

		// Looking for last parent
		if (!GetParent (hTempWnd))
			return hTempWnd;
    
		// get difference between plugin-window and current parent
		GetWindowRect (GetParent (hTempWnd), &rctTempWnd);
	    
		diffWidth  = (rctTempWnd.right - rctTempWnd.left) - (rctPluginWnd.right - rctPluginWnd.left);
		diffHeight = (rctTempWnd.bottom - rctTempWnd.top) - (rctPluginWnd.bottom - rctPluginWnd.top);
		
		// Looking for size mismatch
		if ((abs (diffWidth) > 60) || (abs (diffHeight) > 60)) // parent belongs to host
			return (hTempWnd);

		if (diffWidth < 0)
			diffWidth = 0;
        if (diffHeight < 0)
			diffHeight = 0; 
		
		// get the next parent window
		hTempWnd = GetParent (hTempWnd);
	}

	return NULL;
}
#endif

//-----------------------------------------------------------------------------
bool CFrame::setPosition (CCoord x, CCoord y)
{
	if (!getOpenFlag ())
		return false;
#if MAC
	#if QUARTZ
	if (controlRef)
	{
		HIRect r;
		if (HIViewGetFrame (controlRef, &r) != noErr)
			return false;
		if (HIViewMoveBy (controlRef, x - r.origin.x, y - r.origin.y) != noErr)
			return false;
		return true;
	}
	#else
	return false;
	#endif
#elif WINDOWS
	// not implemented yet

#else
	// not implemented yet

#endif
	return false;
}

//-----------------------------------------------------------------------------
bool CFrame::getPosition (CCoord &x, CCoord &y) const
{
	if (!getOpenFlag ())
		return false;
	
	// get the position of the Window including this frame in the main pWindow
#if WINDOWS
	HWND wnd = (HWND)getOuterWindow ();
	HWND wndParent = GetParent (wnd);

	RECT  rctTempWnd;
	GetWindowRect (wnd, &rctTempWnd);

	POINT point;
	point.x = rctTempWnd.left;
	point.y = rctTempWnd.top;

	MapWindowPoints (HWND_DESKTOP, wndParent, &point, 1);
	
	x = point.x;
	y = point.y;

#elif MAC
	Rect bounds;
	GetWindowBounds ((WindowRef)pSystemWindow, kWindowContentRgn, &bounds);
	
	x   = bounds.left;
	y   = bounds.top;

	#if QUARTZ
	if (isWindowComposited ((WindowRef)pSystemWindow))
	{
		HIPoint hip = { 0.f, 0.f };
		HIViewRef contentView;
		HIViewFindByID (HIViewGetRoot ((WindowRef)pSystemWindow), kHIViewWindowContentID, &contentView);
		if (HIViewGetSuperview ((HIViewRef)controlRef) != contentView)
			HIViewConvertPoint (&hip, controlRef, contentView);
		x += (CCoord)hip.x;
		y += (CCoord)hip.y;
	}
	else
	{
		HIRect hirect;
		HIViewGetFrame ((HIViewRef)controlRef, &hirect);
		x += (CCoord)hirect.origin.x;
		y += (CCoord)hirect.origin.y;
	}
	x -= hiScrollOffset.x;
	y -= hiScrollOffset.y;
	#endif

#elif BEOS
	BRect frame = pPlugView->Window ()->Frame ();
	x = (long) frame.left;
	y = (long) frame.top;
#endif
	return true;
}

//-----------------------------------------------------------------------------
void CFrame::setViewSize (CRect& inRect)
{
	setSize (inRect.width (), inRect.height ());
}

//-----------------------------------------------------------------------------
bool CFrame::setSize (CCoord width, CCoord height)
{
	if (!getOpenFlag ())
		return false;
	
	if ((width == size.width ()) && (height == size.height ()))
	 return false;

#if WINDOWS
	if (backBuffer)
		backBuffer->forget ();
	backBuffer = 0;
#endif
#if ENABLE_VST_EXTENSION_IN_VSTGUI
	if (pEditor)
	{
		AudioEffectX* effect = (AudioEffectX*)((AEffGUIEditor*)pEditor)->getEffect ();
		if (effect && effect->canHostDo ("sizeWindow"))
		{
			if (effect->sizeWindow ((long)width, (long)height))
			{
				size.right = size.left + width;
				size.bottom = size.top + height;

				#if WINDOWS
				SetWindowPos ((HWND)pHwnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE);
				
				#elif (MAC && QUARTZ)
				Rect bounds;
				CRect2Rect (size, bounds);
				SetControlBounds (controlRef, &bounds);
				#endif
				
				return true;
			}
		}
	}
#endif

	// keep old values
	CCoord oldWidth  = size.width ();
	CCoord oldHeight = size.height ();

	// set the new size
	size.right  = size.left + width;
	size.bottom = size.top  + height;

#if WINDOWS
	RECT  rctTempWnd, rctParentWnd;
	HWND  hTempWnd;
	long   iFrame = (2 * GetSystemMetrics (SM_CYFIXEDFRAME));
	
	long diffWidth  = 0;
	long diffHeight = 0;
	
	hTempWnd = (HWND)pHwnd;
	
	while ((diffWidth != iFrame) && (hTempWnd != NULL)) // look for FrameWindow
	{
		HWND hTempParentWnd = GetParent (hTempWnd);
		char buffer[1024];
		GetClassName (hTempParentWnd, buffer, 1024);
		if (!hTempParentWnd || !strcmp (buffer, "MDIClient"))
			break;
		GetWindowRect (hTempWnd, &rctTempWnd);
		GetWindowRect (hTempParentWnd, &rctParentWnd);
		
		SetWindowPos (hTempWnd, HWND_TOP, 0, 0, width + diffWidth, height + diffHeight, SWP_NOMOVE);
		
		diffWidth  += (rctParentWnd.right - rctParentWnd.left) - (rctTempWnd.right - rctTempWnd.left);
		diffHeight += (rctParentWnd.bottom - rctParentWnd.top) - (rctTempWnd.bottom - rctTempWnd.top);
		
		if ((diffWidth > 80) || (diffHeight > 80)) // parent belongs to host
			return true;

		if (diffWidth < 0)
			diffWidth = 0;
        if (diffHeight < 0)
			diffHeight = 0;
		
		hTempWnd = hTempParentWnd;
	}
	
	if (hTempWnd)
		SetWindowPos (hTempWnd, HWND_TOP, 0, 0, width + diffWidth, height + diffHeight, SWP_NOMOVE);

#elif MAC
	#if QUARTZ
	if (getSystemWindow ())
	{
		if (!isWindowComposited ((WindowRef)getSystemWindow ()))
		{
			Rect bounds;
			GetPortBounds (GetWindowPort ((WindowRef)getSystemWindow ()), &bounds);
			SizeWindow ((WindowRef)getSystemWindow (), (short)((bounds.right - bounds.left) - oldWidth + width),
									(short)((bounds.bottom - bounds.top) - oldHeight + height), true);
		}
	}
	if (controlRef)
	{
		HIRect frameRect;
		HIViewGetFrame (controlRef, &frameRect);
		frameRect.size.width = width;
		frameRect.size.height = height;
		HIViewSetFrame (controlRef, &frameRect);
	}

	#else
	if (getSystemWindow ())
	{
		Rect bounds;
		GetPortBounds (GetWindowPort ((WindowRef)getSystemWindow ()), &bounds);
		SizeWindow ((WindowRef)getSystemWindow (), (bounds.right - bounds.left) - oldWidth + width,
								(bounds.bottom - bounds.top) - oldHeight + height, true);
		#if MACX && !QUARTZ
		SetPort (GetWindowPort ((WindowRef)getSystemWindow ()));
		#endif
		#if QUARTZ
		CRect2Rect (size, bounds);
		SetControlBounds (controlRef, &bounds);
		#endif
	}
	#endif
	
#elif BEOS
	BView* parent = pPlugView->Parent ();
	parent->SetResizingMode (B_FOLLOW_ALL_SIDES);
	BRect frame = pPlugView->Frame ();
	pPlugView->Window ()->ResizeBy (width - frame.Width () - 1, height - frame.Height () - 1);
	parent->SetResizingMode (B_FOLLOW_NONE);
#endif

	CRect myViewSize (0, 0, size.width (), size.height ());
	CViewContainer::setViewSize (myViewSize);

	return true;
}

//-----------------------------------------------------------------------------
bool CFrame::getSize (CRect *pRect) const
{
	if (!getOpenFlag ())
		return false;

#if WINDOWS
	// return the size relative to the client rect of this window
	// get the main window
	HWND wnd = GetParent ((HWND)getSystemWindow ());
	HWND wndParent = GetParent (wnd);
	HWND wndParentParent = GetParent (wndParent);

	RECT  rctTempWnd;
	GetWindowRect (wnd, &rctTempWnd);
	
	POINT point;
	point.x = rctTempWnd.left;
	point.y = rctTempWnd.top;

	MapWindowPoints (HWND_DESKTOP, wndParentParent, &point, 1);
	
	pRect->left   = point.x;
	pRect->top    = point.y;
	pRect->right  = pRect->left + rctTempWnd.right - rctTempWnd.left;
	pRect->bottom = pRect->top  + rctTempWnd.bottom - rctTempWnd.top;

#elif MAC
	#if QUARTZ
	HIRect hiRect;
	if (HIViewGetFrame (controlRef, &hiRect) == noErr)
	{
		pRect->left = (CCoord)hiRect.origin.x;
		pRect->top = (CCoord)hiRect.origin.y;
		pRect->setWidth ((CCoord)hiRect.size.width);
		pRect->setHeight ((CCoord)hiRect.size.height);
		return true;
	}
	#endif

	Rect bounds;
	GetPortBounds (GetWindowPort ((WindowRef)getSystemWindow ()), &bounds);

	pRect->left   = bounds.left;
	pRect->top    = bounds.top;
	pRect->right  = bounds.right;
	pRect->bottom = bounds.bottom;
	
#elif BEOS
	BRect v = pPlugView->Frame ();
	(*pRect) (v.left, v.top, v.right + 1, v.bottom + 1);
#endif
	return true;
}

//-----------------------------------------------------------------------------
bool CFrame::getSize (CRect& outSize) const
{
	return getSize (&outSize);
}

//-----------------------------------------------------------------------------
long CFrame::setModalView (CView *pView)
{
	// There's already a modal view so we get out
	if (pView && pModalView)
			return 0;

	if (pModalView)
		removeView (pModalView, false);
	
	pModalView = pView;
	if (pModalView)
		addView (pModalView);

	return 1;
}

//-----------------------------------------------------------------------------
void CFrame::beginEdit (long index)
{
	if (pEditor)
		pEditor->beginEdit (index);
}

//-----------------------------------------------------------------------------
void CFrame::endEdit (long index)
{
	if (pEditor)
		pEditor->endEdit (index);
}

//-----------------------------------------------------------------------------
bool CFrame::getCurrentMouseLocation (CPoint &where)
{
	#if WINDOWS
	HWND hwnd = (HWND)this->getSystemWindow ();
	POINT _where;
	GetCursorPos (&_where);
	where (_where.x, _where.y);
	if (hwnd)
	{
		RECT rctTempWnd;
		GetWindowRect (hwnd, &rctTempWnd);
		where.offset (-rctTempWnd.left, -rctTempWnd.top);
	}
	return true;
	#elif MAC
	// no up-to-date API call available for this, so use QuickDraw
	Point p;
	CGrafPtr savedPort;
	Boolean portChanged = QDSwapPort (GetWindowPort ((WindowRef)getSystemWindow ()), &savedPort);
	GetMouse (&p);
	if (portChanged)
		QDSwapPort (savedPort, NULL);
	where (p.h, p.v);
	#if QUARTZ
	HIPoint location;
	HIViewRef fromView = NULL;
	HIViewFindByID (HIViewGetRoot ((WindowRef)getSystemWindow ()), kHIViewWindowContentID, &fromView);
	location = CGPointMake (where.x, where.y);
	#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
	if (HIPointConvert)
		HIPointConvert (&location, kHICoordSpaceView, fromView, kHICoordSpaceView, controlRef);
	else
	#endif
		HIViewConvertPoint (&location, fromView, controlRef);
	where.x = (CCoord)location.x;
	where.y = (CCoord)location.y;
	#endif // QUARTZ
	return true;
	#endif // MAC

	#if VSTGUI_ENABLE_DEPRECATED_METHODS
	// create a local context
	CDrawContext *pContext = createDrawContext ();
	if (pContext)
	{
	// get the current position
		pContext->getMouseLocation (where);
		pContext->forget ();
	}
	return true;
	#endif

	return false;
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
CView *CFrame::getCurrentView () const
{
	if (pModalView)
		return pModalView;
	
	return CViewContainer::getCurrentView ();
}
#endif

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
bool CFrame::getCurrentLocation (CPoint &where)
{
	return getCurrentMouseLocation (where);
}
#endif

#if MACX
#define kThemeResizeUpDownCursor	21
#define kThemeNotAllowedCursor		18
#endif

//-----------------------------------------------------------------------------
void CFrame::setCursor (CCursorType type)
{
	#if WINDOWS
	if (!defaultCursor)
		defaultCursor = GetCursor ();
	switch (type)
	{
		case kCursorWait:
			SetCursor (LoadCursor (0, IDC_WAIT));
			break;
		case kCursorHSize:
			SetCursor (LoadCursor (0, IDC_SIZEWE));
			break;
		case kCursorVSize:
			SetCursor (LoadCursor (0, IDC_SIZENS));
			break;
		case kCursorNESWSize:
			SetCursor (LoadCursor (0, IDC_SIZENESW));
			break;
		case kCursorNWSESize:
			SetCursor (LoadCursor (0, IDC_SIZENWSE));
			break;
		case kCursorSizeAll:
			SetCursor (LoadCursor (0, IDC_SIZEALL));
			break;
		case kCursorNotAllowed:
			SetCursor (LoadCursor (0, IDC_NO));
			break;
		case kCursorHand:
			SetCursor (LoadCursor (0, IDC_HAND));
			break;
		default:
			SetCursor ((HCURSOR)defaultCursor);
			break;
	}
	#elif MAC
	#if MACX
	switch (type)
	{
		case kCursorWait:
			SetThemeCursor (kThemeWatchCursor);
			break;
		case kCursorHSize:
			SetThemeCursor (pSystemVersion < 0x1030 ? kThemeCrossCursor : kThemeResizeLeftRightCursor);
			break;
		case kCursorVSize:
			SetThemeCursor (pSystemVersion < 0x1030 ? kThemeCrossCursor : kThemeResizeUpDownCursor);
			break;
		case kCursorNESWSize:
			SetThemeCursor (kThemeCrossCursor);
			break;
		case kCursorNWSESize:
			SetThemeCursor (kThemeCrossCursor);
			break;
		case kCursorSizeAll:
			SetThemeCursor (kThemeCrossCursor);
			break;
		case kCursorCopy:
			SetThemeCursor (kThemeCopyArrowCursor);
			break;
		case kCursorNotAllowed:
			SetThemeCursor (pSystemVersion < 0x1020 ? kThemeArrowCursor : kThemeNotAllowedCursor);
			break;
		case kCursorHand:
			SetThemeCursor (kThemeOpenHandCursor);
			break;
		default:
			SetThemeCursor (kThemeArrowCursor);
			break;
	}
	#else
	//if (!defaultCursor)
	//	defaultCursor = GetCursor (0);
	switch (type)
	{
		case kCursorWait:
			SetCursor (*GetCursor (watchCursor));
			break;
		case kCursorHSize:
			SetCursor (*GetCursor (crossCursor));
			break;
		case kCursorVSize:
			SetCursor (*GetCursor (crossCursor));
			break;
		case kCursorNESWSize:
			SetCursor (*GetCursor (crossCursor));
			break;
		case kCursorNWSESize:
			SetCursor (*GetCursor (crossCursor));
			break;
		case kCursorSizeAll:
			SetCursor (*GetCursor (plusCursor));
			break;
		default:
			InitCursor ();
			break;
	}
	#endif
	#endif
}

//-----------------------------------------------------------------------------
void CFrame::setFocusView (CView *pView)
{
	CView *pOldFocusView = pFocusView;
	pFocusView = pView;
	if (pFocusView && pFocusView->wantsFocus ())
		pFocusView->setDirty ();

	if (pOldFocusView)
	{
		pOldFocusView->looseFocus ();
		if (pOldFocusView->wantsFocus ())
			pOldFocusView->setDirty ();
	}
}

//-----------------------------------------------------------------------------
bool CFrame::advanceNextFocusView (CView* oldFocus, bool reverse)
{
	if (pModalView)
	{
		if (pModalView->isTypeOf("CViewContainer"))
		{
			return ((CViewContainer*)pModalView)->advanceNextFocusView (oldFocus, reverse);
		}
		else if (oldFocus != pModalView)
		{
			setFocusView (pModalView);
			return true;
		}
		return false; // currently not supported, but should be done sometime
	}
	if (oldFocus == 0)
	{
		if (pFocusView == 0)
			return CViewContainer::advanceNextFocusView (0, reverse);
		oldFocus = pFocusView;
	}
	if (isChild (oldFocus))
	{
		if (CViewContainer::advanceNextFocusView (oldFocus, reverse))
			return true;
		else
		{
			setFocusView (NULL);
			return false;
		}
	}
	CView* parentView = oldFocus->getParentView ();
	if (parentView && parentView->isTypeOf ("CViewContainer"))
	{
		CView* tempOldFocus = oldFocus;
		CViewContainer* vc = (CViewContainer*)parentView;
		while (vc)
		{
			if (vc->advanceNextFocusView (tempOldFocus, reverse))
				return true;
			else
			{
				tempOldFocus = vc;
				if (vc->getParentView () && vc->getParentView ()->isTypeOf ("CViewContainer"))
					vc = (CViewContainer*)vc->getParentView ();
				else
					vc = 0;
			}
		}
	}
	return CViewContainer::advanceNextFocusView (oldFocus, reverse);
}

//-----------------------------------------------------------------------------
void CFrame::removeView (CView *pView, const bool &withForget)
{
	if (pModalView == pView)
		pModalView = 0;
	CViewContainer::removeView (pView, withForget);
}

//-----------------------------------------------------------------------------
void CFrame::removeAll (const bool &withForget)
{
	pModalView = 0;
	pFocusView = 0;
	tooltipView = 0;
	CViewContainer::removeAll (withForget);
}

//-----------------------------------------------------------------------------
void CFrame::invalidate (const CRect &rect)
{
	CRect rectView;
	FOREACHSUBVIEW
	if (pV)
	{
		pV->getViewSize (rectView);
		if (rect.rectOverlap (rectView))
			pV->setDirty (true);
	}
	ENDFOR
}

#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
//-----------------------------------------------------------------------------
void CFrame::invalidRect (CRect rect)
{
	#if QUARTZ
	if (isWindowComposited ((WindowRef)pSystemWindow))
	{
		#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
		if (HIViewSetNeedsDisplayInRect)
		{
			HIRect r = { {rect.left, rect.top}, {rect.getWidth (), rect.getHeight ()} };
			HIViewSetNeedsDisplayInRect (controlRef, &r, true);
		}
		else
		#endif
		{
			RgnHandle region = NewRgn ();
			SetRectRgn (region, (short)rect.left, (short)rect.top, (short)rect.right, (short)rect.bottom);
			HIViewSetNeedsDisplayInRegion (controlRef, region, true);
			DisposeRgn(region);
		}
	}
	else
	{
		HIRect hiRect;
		HIViewGetFrame (controlRef, &hiRect);
		CRect _rect (rect);
		_rect.offset (size.left, size.top);
		_rect.offset ((CCoord)hiRect.origin.x, (CCoord)hiRect.origin.y);
		Rect r = {(short)_rect.top, (short)_rect.left, (short)_rect.bottom, (short)_rect.right};
		InvalWindowRect ((WindowRef)pSystemWindow, &r);
	}
	
	#elif WINDOWS
	RECT r = {rect.left, rect.top, rect.right, rect.bottom};
	InvalidateRect ((HWND)pHwnd, &r, true);
	#else
	// not supported yet
	#endif
}
#endif

#if DEBUG
//-----------------------------------------------------------------------------
void CFrame::dumpHierarchy ()
{
	dumpInfo ();
	DebugPrint ("\n");
	CViewContainer::dumpHierarchy ();
}
#endif

//-----------------------------------------------------------------------------
// CCView Implementation
//-----------------------------------------------------------------------------
CCView::CCView (CView* pView)
: pView (pView)
, pNext (0)
, pPrevious (0)
{
	if (pView)
		pView->remember ();
}

//-----------------------------------------------------------------------------
CCView::~CCView ()
{ 
	if (pView)
		pView->forget (); 
}

//-----------------------------------------------------------------------------
// CViewContainer Implementation
//-----------------------------------------------------------------------------
/**
 * CViewContainer constructor.
 * @param rect the size of the container
 * @param pParent the parent CFrame
 * @param pBackground the background bitmap, can be NULL
 */
CViewContainer::CViewContainer (const CRect &rect, CFrame *pParent, CBitmap *pBackground)
: CView (rect)
, pFirstView (0)
, pLastView (0)
, mode (kOnlyDirtyUpdate)
, pOffscreenContext (0)
, bDrawInOffscreen (true)
, currentDragView (0)
, mouseDownView (0)
, mouseOverView (0)
{
	#if MACX || USE_ALPHA_BLEND || VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
	bDrawInOffscreen = false;
	#endif
	backgroundOffset (0, 0);
	this->pParentFrame = pParent;
	setBackground (pBackground);
	backgroundColor = kBlackCColor;	
	mode = kOnlyDirtyUpdate;
}

//-----------------------------------------------------------------------------
CViewContainer::~CViewContainer ()
{
	// remove all views
	removeAll (true);

	#if !BEOS
	 if (pOffscreenContext)
		pOffscreenContext->forget ();
	pOffscreenContext = 0;
	#endif
}

//-----------------------------------------------------------------------------
void CViewContainer::parentSizeChanged ()
{
	FOREACHSUBVIEW
		pV->parentSizeChanged ();	// notify children that the size of the parent or this container has changed
	ENDFOR
}

//-----------------------------------------------------------------------------
/**
 * @param rect the new size of the container
 */
void CViewContainer::setViewSize (CRect &rect)
{
	CView::setViewSize (rect);

	parentSizeChanged ();

	#if !BEOS
	if (pOffscreenContext && bDrawInOffscreen)
	{
		pOffscreenContext->forget ();
		pOffscreenContext = new COffscreenContext (pParentFrame, (long)size.width (), (long)size.height (), kBlackCColor);
	}
	#endif
}

//-----------------------------------------------------------------------------
/**
 * @param rect the size you want to check 
 */
CRect CViewContainer::getVisibleSize (const CRect rect) const
{
	CRect result (rect);
	result.offset (size.left, size.top);
	result.bound (size);
	if (pParentFrame == this)
	{}
	else if (pParentView && pParentView->isTypeOf("CViewContainer"))
		result = ((CViewContainer*)pParentView)->getVisibleSize (result);
	else if (pParentFrame)
		result = pParentFrame->getVisibleSize (result);
	result.offset (-size.left, -size.top);
	return result;
}

//-----------------------------------------------------------------------------
/**
 * @param color the new background color of the container
 */
void CViewContainer::setBackgroundColor (CColor color)
{
	backgroundColor = color;
	setDirty (true);
}

//------------------------------------------------------------------------------
long CViewContainer::notify (CView* sender, const char* message)
{
	if (message == kMsgCheckIfViewContainer)
		return kMessageNotified;
	return kMessageUnknown;
}

//-----------------------------------------------------------------------------
/**
 * @param pView the view object to add to this container
 */
void CViewContainer::addView (CView *pView)
{
	if (!pView)
		return;

	CCView *pSv = new CCView (pView);
	
	pView->pParentFrame = pParentFrame;
	pView->pParentView = this;

	CCView *pV = pFirstView;
	if (!pV)
	{
		pLastView = pFirstView = pSv;
	}
	else
	{
		while (pV->pNext)
			pV = pV->pNext;
		pV->pNext = pSv;
		pSv->pPrevious = pV;
		pLastView = pSv;
	}
	if (isAttached ())
	{
		pView->attached (this);
		pView->setDirty ();
	}
}

//-----------------------------------------------------------------------------
/**
 * @param pView the view object to add to this container
 * @param mouseableArea the view area in where the view will get mouse events
 * @param mouseEnabled bool to set if view will get mouse events
 */
void CViewContainer::addView (CView *pView, CRect &mouseableArea, bool mouseEnabled)
{
	if (!pView)
		return;

	pView->setMouseEnabled (mouseEnabled);
	pView->setMouseableArea (mouseableArea);

	addView (pView);
}

//-----------------------------------------------------------------------------
/**
 * @param withForget bool to indicate if the view's reference counter should be decreased after removed from the container
 */
void CViewContainer::removeAll (const bool &withForget)
{
	mouseOverView = 0;
	CCView *pV = pFirstView;
	while (pV)
	{
		CCView *pNext = pV->pNext;
		if (pV->pView)
		{
			if (isAttached ())
				pV->pView->removed (this);
			if (withForget)
				pV->pView->forget ();
		}

		delete pV;

		pV = pNext;
	}
	pFirstView = 0;
	pLastView = 0;
}

//-----------------------------------------------------------------------------
/**
 * @param pView the view which should be removed from the container
 * @param withForget bool to indicate if the view's reference counter should be decreased after removed from the container
 */
void CViewContainer::removeView (CView *pView, const bool &withForget)
{
	if (mouseOverView = pView)
		mouseOverView = 0;
	if (pParentFrame && pParentFrame->getFocusView () == pView)
		pParentFrame->setFocusView (0);
	CCView *pV = pFirstView;
	while (pV)
	{
		if (pView == pV->pView)
		{
			CCView *pNext = pV->pNext;
			CCView *pPrevious = pV->pPrevious;
			if (pV->pView)
			{
				if (isAttached ())
					pV->pView->removed (this);
				if (withForget)
					pV->pView->forget ();
			}
			delete pV;
			if (pPrevious)
			{
				pPrevious->pNext = pNext;
				if (pNext)
					pNext->pPrevious = pPrevious;
				else
					pLastView = pPrevious;
			}
			else
			{
				pFirstView = pNext;
				if (pNext)
					pNext->pPrevious = 0;
				else
					pLastView = 0;	
			}
			break;
		}
		else
			pV = pV->pNext;
	}
}

//-----------------------------------------------------------------------------
/**
 * @param pView the view which should be checked if it is a child of this container
 */
bool CViewContainer::isChild (CView *pView) const
{
	bool found = false;

	CCView *pV = pFirstView;
	while (pV)
	{
		if (pView == pV->pView)
		{
			found = true;
			break;
		}
		pV = pV->pNext;
	}
	return found;
}

//-----------------------------------------------------------------------------
long CViewContainer::getNbViews () const
{
	long nb = 0;
	CCView *pV = pFirstView;
	while (pV)
	{
		pV = pV->pNext;
		nb++;
	}
	return nb;
}

//-----------------------------------------------------------------------------
/**
 * @param index the index of the view to return
 */
CView *CViewContainer::getView (long index) const
{
	long nb = 0;
	CCView *pV = pFirstView;
	while (pV)
	{
		if (nb == index)
			return pV->pView;
		pV = pV->pNext;
		nb++;
	}
	return 0;
}

#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
//-----------------------------------------------------------------------------
bool CViewContainer::invalidateDirtyViews ()
{
	if (bDirty)
	{
		if (pParentView)
			pParentView->invalidRect (size);
		else if (pParentFrame)
			pParentFrame->invalidRect (size);
		return true;
	}
	FOREACHSUBVIEW
		if (pV->isDirty ())
		{
			if (pV->isTypeOf ("CViewContainer"))
				((CViewContainer*)pV)->invalidateDirtyViews ();
			else
				pV->invalid ();
		}
	ENDFOR
	return true;
}

//-----------------------------------------------------------------------------
void CViewContainer::invalid ()
{
	CRect _rect (size);
	if (pParentView)
		pParentView->invalidRect (_rect);
	else if (pParentFrame)
		pParentFrame->invalidRect (_rect);
}

//-----------------------------------------------------------------------------
void CViewContainer::invalidRect (CRect rect)
{
	CRect _rect (rect);
	_rect.offset (size.left, size.top);
	_rect.bound (size);
	if (_rect.isEmpty ())
		return;
	if (pParentView)
		pParentView->invalidRect (_rect);
	else if (pParentFrame)
		pParentFrame->invalidRect (_rect);
}
#endif

//-----------------------------------------------------------------------------
/**
 * @param pContext the context which to use to draw this container and its subviews
 */
void CViewContainer::draw (CDrawContext *pContext)
{
	CDrawContext *pC;
	CCoord save[4];

	#if BEOS
	// create offscreen
	if (pBackground)
		pC = new COffscreenContext (pContext, pBackground);
	else
		pC = new COffscreenContext (pParentFrame, size.width (), size.height (), backgroundColor);
	
	#else
	if (!pOffscreenContext && bDrawInOffscreen)
		pOffscreenContext = new COffscreenContext (pParentFrame, (long)size.width (), (long)size.height (), kBlackCColor);
	#if USE_ALPHA_BLEND
	if (pOffscreenContext && bTransparencyEnabled)
		pOffscreenContext->copyTo (pContext, size);
	#endif

	if (bDrawInOffscreen)
		pC = pOffscreenContext;
	else
	{
		pC = pContext;
		modifyDrawContext (save, pContext);
	}

	CRect r (0, 0, size.width (), size.height ());

	CRect oldClip;
	pContext->getClipRect (oldClip);
	CRect oldClip2 (oldClip);
	if (bDrawInOffscreen && getFrame () != this)
		oldClip.offset (-oldClip.left, -oldClip.top);
		
	CRect newClip (r);
	newClip.bound (oldClip);
	pC->setClipRect (newClip);

	// draw the background
	if (pBackground)
	{
		if (bTransparencyEnabled)
			pBackground->drawTransparent (pC, r, backgroundOffset);
		else
			pBackground->draw (pC, r, backgroundOffset);
	}
	else if (!bTransparencyEnabled)
	{
		pC->setFillColor (backgroundColor);
		pC->fillRect (r);
	}
	#endif
	
	// draw each view
	FOREACHSUBVIEW
		CRect vSize (pV->size);
		vSize.bound (oldClip);
		pC->setClipRect (vSize);
		pV->draw (pC);
	ENDFOR

	pC->setClipRect (oldClip2);
	
	// transfert offscreen
	if (bDrawInOffscreen)
		((COffscreenContext*)pC)->copyFrom (pContext, size);
	else
		restoreDrawContext (pContext, save);

	#if BEOS
	delete pC;
	#endif

	setDirty (false);
}

//-----------------------------------------------------------------------------
/**
 * @param pContext the context which to use to draw the background
 * @param _updateRect the area which to draw
 */
void CViewContainer::drawBackgroundRect (CDrawContext *pContext, CRect& _updateRect)
{
	if (pBackground)
	{
		CRect oldClip;
		pContext->getClipRect (oldClip);
		CRect newClip (_updateRect);
		newClip.bound (oldClip);
		pContext->setClipRect (newClip);
		CRect tr (0, 0, pBackground->getWidth (), pBackground->getHeight ());
		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, tr, backgroundOffset);
		else
			pBackground->draw (pContext, tr, backgroundOffset);
		pContext->setClipRect (oldClip);
	}
	else if (!bTransparencyEnabled)
	{
		pContext->setFillColor (backgroundColor);
		pContext->fillRect (_updateRect);
	}
}

//-----------------------------------------------------------------------------
/**
 * @param pContext the context which to use to draw
 * @param _updateRect the area which to draw
 */
void CViewContainer::drawRect (CDrawContext *pContext, const CRect& _updateRect)
{
	CDrawContext *pC;
	CCoord save[4];

	#if BEOS
	// create offscreen
	if (pBackground)
		pC = new COffscreenContext (pContext, pBackground);
	else
		pC = new COffscreenContext (pParentFrame, size.width (), size.height (), backgroundColor);
	
	#else
	if (!pOffscreenContext && bDrawInOffscreen)
		pOffscreenContext = new COffscreenContext (pParentFrame, (long)size.width (), (long)size.height (), kBlackCColor);
	#if USE_ALPHA_BLEND
	if (pOffscreenContext && bTransparencyEnabled)
		pOffscreenContext->copyTo (pContext, size);
	#endif

	if (bDrawInOffscreen)
		pC = pOffscreenContext;
	else
	{
		pC = pContext;
		modifyDrawContext (save, pContext);
	}

	CRect updateRect (_updateRect);
	updateRect.bound (size);

	CRect clientRect (updateRect);
	clientRect.offset (-size.left, -size.top);

	CRect oldClip;
	pContext->getClipRect (oldClip);
	CRect oldClip2 (oldClip);
	if (bDrawInOffscreen && getFrame () != this)
		oldClip.offset (-oldClip.left, -oldClip.top);
	
	CRect newClip (clientRect);
	newClip.bound (oldClip);
	pC->setClipRect (newClip);
	
	// draw the background
	drawBackgroundRect (pC, clientRect);
	#endif
	
	// draw each view
	FOREACHSUBVIEW
		if (pV->checkUpdate (clientRect))
		{
			CRect viewSize (pV->size);
			viewSize.bound (newClip);
			if (viewSize.getWidth () == 0 || viewSize.getHeight () == 0)
				continue;
			pC->setClipRect (viewSize);

			bool wasDirty = pV->isDirty ();
			pV->drawRect (pC, clientRect);
			
			#if DEBUG_FOCUS_DRAWING
			if (getFrame ()->getFocusView() == pV && pV->wantsFocus ())
			{
				pC->setDrawMode (kCopyMode);
				pC->setFrameColor (kRedCColor);
				pC->drawRect (pV->size);
			}
			#endif
			if (wasDirty && pV->size != viewSize && !isTypeOf ("CScrollContainer"))
			{
//				pV->setDirty (true);
			}
		}
	ENDFOR

	pC->setClipRect (oldClip2);

	// transfer offscreen
	if (bDrawInOffscreen)
		((COffscreenContext*)pC)->copyFrom (pContext, updateRect, CPoint (clientRect.left, clientRect.top));
	else
		restoreDrawContext (pContext, save);

	#if BEOS
	delete pC;
	#endif

	#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
	setDirty (false);
	#else
	newClip.offset (size.left, size.top);
	if (bDirty && newClip == size)
		setDirty (false);
	else if (bDirty)
		fprintf (stderr, "Not expected!\n");
	#endif
}

#if !VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
//-----------------------------------------------------------------------------
/**
 * @param context the context which to use to redraw this container
 * @param rect the area which to redraw
 */
void CViewContainer::redrawRect (CDrawContext* context, const CRect& rect)
{
	CRect _rect (rect);
	_rect.offset (size.left, size.top);
	if (bTransparencyEnabled)
	{
		// as this is transparent, we call the parentview to redraw this area.
		if (pParentView)
			pParentView->redrawRect (context, _rect);
		else if (pParentFrame)
			pParentFrame->drawRect (context, _rect);
	}
	else
	{
		CCoord save[4];
		if (pParentView)
		{
			CPoint off;
			pParentView->localToFrame (off);
			// store
			save[0] = context->offsetScreen.h;
			save[1] = context->offsetScreen.v;
			save[2] = context->offset.h;
			save[3] = context->offset.v;

			context->offsetScreen.h += off.x;
			context->offsetScreen.v += off.y;
			context->offset.h += off.x;
			context->offset.v += off.y;
		}

		drawRect (context, _rect);

		if (pParentView)
		{
			// restore
			context->offsetScreen.h = save[0];
			context->offsetScreen.v = save[1];
			context->offset.h = save[2];
			context->offset.v = save[3];
		}
	}
}
#endif

//-----------------------------------------------------------------------------
bool CViewContainer::hitTestSubViews (const CPoint& where, const long buttons)
{
	CPoint where2 (where);
	where2.offset (-size.left, -size.top);

	CCView *pSv = pLastView;
	while (pSv)
	{
		CView *pV = pSv->pView;
		if (pV && pV->getMouseEnabled () && pV->hitTest (where2, buttons))
			return true;
		pSv = pSv->pPrevious;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CViewContainer::hitTest (const CPoint& where, const long buttons)
{
	//return hitTestSubViews (where); would change default behavior
	return CView::hitTest (where, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CViewContainer::onMouseDown (CPoint &where, const long& buttons)
{
	// convert to relativ pos
	CPoint where2 (where);
	where2.offset (-size.left, -size.top);

	CCView *pSv = pLastView;
	while (pSv)
	{
		CView *pV = pSv->pView;
		if (pV && pV->getMouseEnabled () && pV->hitTest (where2, buttons))
		{
			if (pV->isTypeOf("CControl") && ((CControl*)pV)->getListener () && buttons & (kAlt | kShift | kControl | kApple))
			{
				if (((CControl*)pV)->getListener ()->controlModifierClicked ((CControl*)pV, buttons) != 0)
					return kMouseEventHandled;
			}
			CMouseEventResult result = pV->onMouseDown (where2, buttons);
			#if VSTGUI_ENABLE_DEPRECATED_METHODS
			if (result == kMouseEventNotImplemented)
			{
				// compatibilty for old CViews without mouse down/up/moved handling
				CDrawContext* context = getFrame ()->createDrawContext ();
				pV->mouse (context, where2, buttons);
				context->forget ();
			}
			else 
			#endif
			if (result == kMouseEventHandled)
				mouseDownView = pV;
			return result;
		}
		pSv = pSv->pPrevious;
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CViewContainer::onMouseUp (CPoint &where, const long& buttons)
{
	if (mouseDownView)
	{
		// convert to relativ pos
		CPoint where2 (where);
		where2.offset (-size.left, -size.top);
		mouseDownView->onMouseUp (where2, buttons);
		mouseDownView = 0;
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CViewContainer::onMouseMoved (CPoint &where, const long& buttons)
{
	if (mouseDownView)
	{
		// convert to relativ pos
		CPoint where2 (where);
		where2.offset (-size.left, -size.top);
		if (mouseDownView->onMouseMoved (where2, buttons) != kMouseEventHandled)
		{
			mouseDownView = 0;
			return kMouseEventNotHandled;
		}
		return kMouseEventHandled;
	}
	else
	{
		CView* v = getViewAt (where, true);
		if (v != mouseOverView)
		{
			#if 0
			if (v)
			{
				CPoint vr (v->size.left, v->size.top);
				v->localToFrame (vr);
				fprintf (stdout, "New Mouse Over View : x=%d, y=%d, width=%d, height=%d\n", (long)vr.x, (long)vr.y, (long)v->size.getWidth (), (long)v->size.getHeight ());
			}
			#endif
			if (mouseOverView)
				mouseOverView->onMouseExited (where, buttons);
			mouseOverView = 0;
			if (v)
			{
				v->onMouseEntered (where, buttons);
				mouseOverView = v;
				// check for tooltip
				if (getFrame ()->getTooltipView ())
				{
					long tooltipSize = 0;
					if (v->getAttributeSize (kCViewTooltipAttribute, tooltipSize))
					{
						char* text = (char*)malloc (tooltipSize + 1);
						if (v->getAttribute (kCViewTooltipAttribute, tooltipSize, text, tooltipSize))
							getFrame ()->getTooltipView ()->setText (text);
						free (text);
					}
					else
						getFrame ()->getTooltipView ()->setText (0);
				}
			}
			else if (getFrame ()->getTooltipView ())
			{
				// remove tooltip
				getFrame ()->getTooltipView ()->setText (0);
			}
			return kMouseEventHandled;
		}
	}
	return kMouseEventNotHandled;
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
void CViewContainer::mouse (CDrawContext *pContext, CPoint &where, long buttons)
{
	// convert to relativ pos
	CPoint where2 (where);
	where2.offset (-size.left, -size.top);

	if (buttons == -1 && pContext)
		buttons = pContext->getMouseButtons ();

	CCView *pSv = pLastView;
	while (pSv)
	{
		CView *pV = pSv->pView;
		if (pV && pV->getMouseEnabled () && pV->hitTest (where2, buttons))
		{
			pV->mouse (pContext, where2, buttons);
			break;
		}
		pSv = pSv->pPrevious;
	}
}
#endif

//-----------------------------------------------------------------------------
long CViewContainer::onKeyDown (VstKeyCode& keyCode)
{
	long result = -1;

	CCView* pSv = pLastView;
	while (pSv)
	{
		long result = pSv->pView->onKeyDown (keyCode);
		if (result != -1)
			break;

		pSv = pSv->pPrevious;
	}

	return result;
}

//-----------------------------------------------------------------------------
long CViewContainer::onKeyUp (VstKeyCode& keyCode)
{
	long result = -1;

	CCView* pSv = pLastView;
	while (pSv)
	{
		long result = pSv->pView->onKeyUp (keyCode);
		if (result != -1)
			break;

		pSv = pSv->pPrevious;
	}

	return result;
}

//-----------------------------------------------------------------------------
bool CViewContainer::onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const long &buttons)
{
	bool result = false;
	CView *view = getViewAt (where);
	if (view)
	{
		// convert to relativ pos
		CPoint where2 (where);
		where2.offset (-size.left, -size.top);

		result = view->onWheel (where2, axis, distance, buttons);
	}
	return result;
}

//-----------------------------------------------------------------------------
bool CViewContainer::onWheel (const CPoint &where, const float &distance, const long &buttons)
{
	return onWheel (where, kMouseWheelAxisY, distance, buttons);
}

//-----------------------------------------------------------------------------
bool CViewContainer::onDrop (CDragContainer* drag, const CPoint& where)
{
	if (!pParentFrame)
		return false;

	bool result = false;

	// convert to relativ pos
	CPoint where2 (where);
	where2.offset (-size.left, -size.top);

	CView* view = getViewAt (where);
	if (view != currentDragView)
	{
		if (currentDragView)
			currentDragView->onDragLeave (drag, where2);
		currentDragView = view;
	}
	if (currentDragView)
	{
		result = currentDragView->onDrop (drag, where2);
		currentDragView->onDragLeave (drag, where2);
	}
	currentDragView = 0;
	
	return result;
}

//-----------------------------------------------------------------------------
void CViewContainer::onDragEnter (CDragContainer* drag, const CPoint& where)
{
	if (!pParentFrame)
		return;
	
	// convert to relativ pos
	CPoint where2 (where);
	where2.offset (-size.left, -size.top);

	if (currentDragView)
		currentDragView->onDragLeave (drag, where2);
	CView* view = getViewAt (where);
	currentDragView = view;
	if (view)
		view->onDragEnter (drag, where2);
}

//-----------------------------------------------------------------------------
void CViewContainer::onDragLeave (CDragContainer* drag, const CPoint& where)
{
	if (!pParentFrame)
		return;
	
	// convert to relativ pos
	CPoint where2 (where);
	where2.offset (-size.left, -size.top);

	if (currentDragView)
		currentDragView->onDragLeave (drag, where2);
	currentDragView = 0;
}

//-----------------------------------------------------------------------------
void CViewContainer::onDragMove (CDragContainer* drag, const CPoint& where)
{
	if (!pParentFrame)
		return;
	
	// convert to relativ pos
	CPoint where2 (where);
	where2.offset (-size.left, -size.top);

	CView* view = getViewAt (where);
	if (view != currentDragView)
	{
		if (currentDragView)
			currentDragView->onDragLeave (drag, where2);
		if (view)
			view->onDragEnter (drag, where2);
		currentDragView = view;
	}
	else if (currentDragView)
		currentDragView->onDragMove (drag, where2);
}

#if !VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
//-----------------------------------------------------------------------------
void CViewContainer::update (CDrawContext *pContext)
{
	switch (mode)
	{
		//---Normal : redraw all...
		case kNormalUpdate:
			if (isDirty ())
			{
				CRect ur (0, 0, size.width (), size.height ());
				redrawRect (pContext, ur);
				setDirty (false);
			}
		break;
	
		//---Redraw only dirty controls-----
		case kOnlyDirtyUpdate:
		{
			if (bDirty)
			{
				CRect ur (0, 0, size.width (), size.height ());
				redrawRect (pContext, ur);
			}
			else
			{
				CRect updateRect (size);
				updateRect.offset (-size.left, -size.top);
				FOREACHSUBVIEW
					if (pV->isDirty () && pV->checkUpdate (updateRect))
					{
						if (pV->notify (this, kMsgCheckIfViewContainer))
							pV->update (pContext);
						else
						{
							CRect drawSize (pV->size);
							drawSize.bound (updateRect);
							pV->redrawRect (pContext, drawSize);
						}
					}
				ENDFOR
			}
			setDirty (false);
		break;
		}
	}
}
#endif

//-----------------------------------------------------------------------------
void CViewContainer::looseFocus ()
{
	FOREACHSUBVIEW
		pV->looseFocus ();
	ENDFOR
}

//-----------------------------------------------------------------------------
void CViewContainer::takeFocus ()
{
	FOREACHSUBVIEW
		pV->takeFocus ();
	ENDFOR
}

//-----------------------------------------------------------------------------
bool CViewContainer::advanceNextFocusView (CView* oldFocus, bool reverse)
{
	bool foundOld = false;
	FOREACHSUBVIEW_REVERSE(reverse)
		if (oldFocus && !foundOld)
		{
			if (oldFocus == pV)
			{
				foundOld = true;
				continue;
			}
		}
		else
		{
			if (pV->wantsFocus ())
			{
				getFrame ()->setFocusView (pV);
				return true;
			}
			else if (pV->isTypeOf ("CViewContainer"))
			{
				if (((CViewContainer*)pV)->advanceNextFocusView (0, reverse))
					return true;
			}
		}
	ENDFOR
	return false;
}

//-----------------------------------------------------------------------------
bool CViewContainer::isDirty () const
{
	if (bDirty)
		return true;
		
	CRect viewSize (size);
	viewSize.offset (-size.left, -size.top);

	FOREACHSUBVIEW
		if (pV->isDirty ())
		{
			CRect r (pV->size);
			r.bound (viewSize);
			if (r.getWidth () > 0 && r.getHeight () > 0)
				return true;
		}
	ENDFOR
	return false;
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
CView *CViewContainer::getCurrentView () const
{
	if (!pParentFrame)
		return 0;

	// get the current position
	CPoint where;
	pParentFrame->getCurrentLocation (where);

	frameToLocal (where);
	
	CCView *pSv = pLastView;
	while (pSv)
	{
		CView *pV = pSv->pView;
		if (pV && where.isInside (pV->mouseableArea))
			return pV;
		pSv = pSv->pPrevious;
	}

	return 0;
}
#endif

//-----------------------------------------------------------------------------
CView *CViewContainer::getViewAt (const CPoint& p, bool deep) const
{
	if (!pParentFrame)
		return 0;

	CPoint where (p);

	// convert to relativ pos
	where.offset (-size.left, -size.top);

	CCView *pSv = pLastView;
	while (pSv)
	{
		CView *pV = pSv->pView;
		if (pV && where.isInside (pV->mouseableArea))
		{
			if (deep)
			{
				if (pV->isTypeOf ("CViewContainer"))
					return ((CViewContainer*)pV)->getViewAt (where, deep);
			}
			return pV;
		}
		pSv = pSv->pPrevious;
	}

	return 0;
}

//-----------------------------------------------------------------------------
CPoint& CViewContainer::frameToLocal (CPoint& point) const
{
	point.offset (-size.left, -size.top);
	if (pParentView && pParentView->isTypeOf ("CViewContainer"))
		return pParentView->frameToLocal (point);
	return point;
}

//-----------------------------------------------------------------------------
CPoint& CViewContainer::localToFrame (CPoint& point) const
{
	point.offset (size.left, size.top);
	if (pParentView && pParentView->isTypeOf ("CViewContainer"))
		return pParentView->localToFrame (point);
	return point;
}

//-----------------------------------------------------------------------------
bool CViewContainer::removed (CView* parent)
{
	#if !BEOS
	 if (pOffscreenContext)
		pOffscreenContext->forget ();
	pOffscreenContext = 0;
	#endif

	pParentFrame = 0;

	FOREACHSUBVIEW
		pV->pParentFrame = 0;
		pV->removed (this);
	ENDFOR
	
	return CView::removed (parent);
}

//-----------------------------------------------------------------------------
bool CViewContainer::attached (CView* view)
{
	#if !BEOS
	// create offscreen bitmap
	if (!pOffscreenContext && bDrawInOffscreen)
		pOffscreenContext = new COffscreenContext (pParentFrame, (long)size.width (), (long)size.height (), kBlackCColor);
	#endif

	pParentFrame = view->getFrame ();

	FOREACHSUBVIEW
		pV->attached (this);
		pV->pParentFrame = pParentFrame;
	ENDFOR

	return CView::attached (view);
}

//-----------------------------------------------------------------------------
void CViewContainer::useOffscreen (bool b)
{
	bDrawInOffscreen = b;
	
	#if !BEOS
	if (!bDrawInOffscreen && pOffscreenContext)
	{
		pOffscreenContext->forget ();
		pOffscreenContext = 0;
	}
	#endif
}

//-----------------------------------------------------------------------------
void CViewContainer::modifyDrawContext (CCoord save[4], CDrawContext* pContext)
{
	// store
	save[0] = pContext->offsetScreen.h;
	save[1] = pContext->offsetScreen.v;
	save[2] = pContext->offset.h;
	save[3] = pContext->offset.v;

	pContext->offsetScreen.h += size.left;
	pContext->offsetScreen.v += size.top;
	pContext->offset.h += size.left;
	pContext->offset.v += size.top;
}

//-----------------------------------------------------------------------------
void CViewContainer::restoreDrawContext (CDrawContext* pContext, CCoord save[4])
{
	// restore
	pContext->offsetScreen.h = save[0];
	pContext->offsetScreen.v = save[1];
	pContext->offset.h = save[2];
	pContext->offset.v = save[3];
}

#if DEBUG
static long _debugDumpLevel = 0;
//-----------------------------------------------------------------------------
void CViewContainer::dumpInfo ()
{
	static const char* modeString[] = { "Normal Update Mode", "Only Dirty Update Mode"};
	DebugPrint ("CViewContainer: Mode: %s, Offscreen:%s ", modeString[mode], bDrawInOffscreen ? "Yes" : "No");
	CView::dumpInfo ();
}

//-----------------------------------------------------------------------------
void CViewContainer::dumpHierarchy ()
{
	_debugDumpLevel++;
	FOREACHSUBVIEW
		for (long i = 0; i < _debugDumpLevel; i++)
			DebugPrint ("\t");
		pV->dumpInfo ();
		DebugPrint ("\n");
		if (pV->isTypeOf ("CViewContainer"))
			((CViewContainer*)pV)->dumpHierarchy ();
	ENDFOR
	_debugDumpLevel--;
}

#endif

#if WINDOWS && USE_LIBPNG
class PNGResourceStream
{
public:
		PNGResourceStream ()
		: streamPos (0)
		, resData (0)
		, resSize (0)
		{
		}

		~PNGResourceStream ()
		{
		}

		bool open (long resourceID)
		{
			HRSRC rsrc = FindResource (GetInstance (), MAKEINTRESOURCE (resourceID), "PNG");
			if (rsrc)
			{
				resSize = SizeofResource (GetInstance (), rsrc);
				HGLOBAL resDataLoad = LoadResource (GetInstance (), rsrc);
				if (resDataLoad)
				{
					resData = LockResource (resDataLoad);
					return true;
				}
			}
			return false;
		}

		void read (unsigned char* ptr, size_t size)
		{
			if (streamPos + size <= resSize)
			{
				memcpy (ptr, ((unsigned char*)resData+streamPos), size);
				streamPos += size;
			}
		}

		static void readCallback (png_struct* pngPtr, unsigned char* ptr, size_t size)
		{
			void* obj = png_get_io_ptr (pngPtr);
			if (obj)
				((PNGResourceStream*)obj)->read (ptr, size);
		}
protected:
	HGLOBAL resData;
	unsigned long streamPos;
	unsigned long resSize;
};
#endif

//-----------------------------------------------------------------------------
// CBitmap Implementation
//-----------------------------------------------------------------------------
/*! @class CBitmap
@section cbitmap_alphablend Alpha Blend and Transparency
With Version 3.0 of VSTGUI it is possible to use alpha blended bitmaps. This comes free on Mac OS X and with Windows you need to include libpng.
Per default PNG images will be rendered alpha blended. If you want to use a transparency color with PNG Bitmaps, you need to call setNoAlpha(true) on the bitmap and set the transparency color.
@section cbitmap_macos Classic Apple Mac OS
The Bitmaps are PICTs and stored inside the resource fork.
@section cbitmap_macosx Apple Mac OS X
The Bitmaps can be of type PNG, JPEG, PICT, BMP and are stored in the Resources folder of the plugin bundle.
They must be named bmp00100.png (or bmp00100.jpg, etc). The number is the resource id.
@section cbitmap_windows Microsoft Windows
The Bitmaps are .bmp files and must be included in the plug (usually using a .rc file).
It's also possible to use png as of version 3.0 if you define the macro USE_LIBPNG and include the libpng and zlib libraries/sources to your project.
*/
CBitmap::CBitmap (long resourceID)
: resourceID (resourceID)
, width (0)
, height (0)
, noAlpha (true)
{
	#if DEBUG
	gNbCBitmap++;
	#endif

#if GDIPLUS
	pBitmap = 0;
	bits = 0;
#endif

#if WINDOWS || MAC
	pMask = 0;
	pHandle = 0;
	#if QUARTZ
	cgImage = 0;
	#endif

	loadFromResource (resourceID);

#elif BEOS
	bbitmap = 0;
	transparencySet = false;
	if (resourceFile == 0)
	{
		// this is a hack to find the plug-in on the disk to access resources.
		const char* locate_me = "";
		int32	cookie = 0;
		image_info	iinfo;
		uint32	here = uint32 (locate_me);
		while (get_next_image_info (0, &cookie, &iinfo) == B_OK)
		{
			uint32	begin = uint32 (iinfo.text);
			if (begin <= here && here <= begin + iinfo.text_size)
				break;
		}
		BFile resource (iinfo.name, B_READ_ONLY);
		resourceFile = new BResources (&resource);
		resourceFile->PreloadResourceType ();
	}
	size_t	outSize;
	const char* res = (const char*) resourceFile->LoadResource ('RAWT', resourceID, &outSize);
	if (res)
	{
		BMemoryIO	memoryIO (res, outSize);
		bbitmap = BTranslationUtils::GetBitmap (&memoryIO);
		if (bbitmap)
		{
			BRect rect = bbitmap->Bounds ();
			width = (long) rect.Width () + 1;
			height = (long) rect.Height () + 1;
		}
	}
	if (!bbitmap)
		fprintf (stderr, "********* Resource %d could NOT be loaded!\n", (int)resourceID);
#endif

	setTransparentColor (kTransparentCColor);
	
	#if DEBUG
	gBitmapAllocation += (long)height * (long)width;
	#endif
}

//-----------------------------------------------------------------------------
CBitmap::CBitmap (CFrame& frame, CCoord width, CCoord height)
: width (width)
, height (height)
, noAlpha (true)
{
	#if DEBUG
	gNbCBitmap++;
	#endif

#if WINDOWS
	HDC hScreen = GetDC (0);
#if GDIPLUS
	pBitmap = 0;
	bits = 0;
	Gdiplus::Graphics *hScreenGraphics = new Gdiplus::Graphics(hScreen);
	pBitmap = new Gdiplus::Bitmap(width,height,hScreenGraphics);	// format?
	delete hScreenGraphics;
#else
	pHandle = CreateCompatibleBitmap (hScreen, width, height);
#endif
	ReleaseDC (0, hScreen);	
	pMask = 0;

#elif MAC
	pHandle = 0;
	pMask = 0;
	
	Rect r;
	r.left = r.top = 0;
	r.right = (short)width;
	r.bottom = (short)height;

    #if QUARTZ
	NewGWorld ((GWorldPtr*)&pHandle, 32, &r, 0, 0, 0);
	cgImage = 0;
    #else
	NewGWorld ((GWorldPtr*)&pHandle, 0, &r, 0, 0, 0);

	#endif

#elif BEOS
	bbitmap = 0;
	transparencySet = false;
#endif
	
	setTransparentColor (kTransparentCColor);
}

//-----------------------------------------------------------------------------
CBitmap::CBitmap ()
: resourceID (0)
, width (0)
, height (0)
, noAlpha (true)
{
	#if WINDOWS
	#if GDIPLUS
	pBitmap = 0;
	bits = 0;
	#endif

	pHandle = 0;
	pMask = 0;
	
	#elif MAC
	pHandle = 0;
	pMask = 0;
	#if QUARTZ
	cgImage = 0;
	#endif
	
	#elif BEOS
	bbitmap = 0;

	#endif
}

//-----------------------------------------------------------------------------
CBitmap::~CBitmap ()
{
	dispose ();
}

//-----------------------------------------------------------------------------
void CBitmap::dispose ()
{
	#if DEBUG
	gNbCBitmap--;
	gBitmapAllocation -= (long)height * (long)width;
	#endif

	#if WINDOWS
	#if GDIPLUS
	if (pBitmap)
		delete pBitmap;
	pBitmap = 0;
	bits = 0;
	#endif
	if (pHandle)
		DeleteObject (pHandle);
	if (pMask)
		DeleteObject (pMask);

	pHandle = 0;
	pMask = 0;
	noAlpha = false;
		
	#elif MAC
	#if QUARTZ
	if (cgImage)
		CGImageRelease ((CGImageRef)cgImage);
	cgImage = 0;
	#endif
	if (pHandle)
		DisposeGWorld ((GWorldPtr)pHandle);
	if (pMask)
		DisposeGWorld ((GWorldPtr)pMask);

	pHandle = 0;
	pMask = 0;
	
	#elif BEOS
	if (bbitmap)
		delete bbitmap;
	
	bbitmap = 0;

	#endif

	width = 0;
	height = 0;

}

//-----------------------------------------------------------------------------
void *CBitmap::getHandle () const
 {
	#if WINDOWS
	return pHandle; 

	#elif MAC
	return pHandle;

	#elif BEOS
	return bbitmap;
	#endif
}

//-----------------------------------------------------------------------------
bool CBitmap::loadFromResource (long resourceID)
{
	bool result = false;

	dispose ();
	
	//---------------------------------------------------------------------------------------------
	#if WINDOWS
	//---------------------------------------------------------------------------------------------
	#if USE_LIBPNG
	PNGResourceStream resStream;
	if (resStream.open (resourceID))
	{
		// setup libpng
		png_structp png_ptr;
		png_infop info_ptr;
		png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (png_ptr)
		{
			info_ptr = png_create_info_struct (png_ptr);
			if (info_ptr)
			{
				if (setjmp (png_jmpbuf (png_ptr)) == 0)
				{
					int bit_depth, color_type;
					png_set_read_fn (png_ptr, (void *)&resStream, PNGResourceStream::readCallback);
					png_read_info (png_ptr, info_ptr);
					png_get_IHDR (png_ptr, info_ptr, (png_uint_32*)&width, (png_uint_32*)&height, &bit_depth, &color_type, 0, 0, 0);
					long bytesPerRow = width * (32 / 8);
					while (bytesPerRow & 0x03)
						bytesPerRow++;
					// create BITMAP
					BITMAPINFO* bmInfo = new BITMAPINFO;
					BITMAPINFOHEADER* header = (BITMAPINFOHEADER*)bmInfo;
					memset (header, 0, sizeof(BITMAPINFOHEADER));
					header->biSize = sizeof(BITMAPINFOHEADER);
					header->biWidth = width;
					header->biHeight = height;
					header->biPlanes = 1;
					header->biBitCount = 32;
					header->biCompression = BI_RGB;
					header->biClrUsed = 0;
					#if !GDIPLUS
					void* bits;
					#endif
					HDC dstDC = 0; //CreateCompatibleDC (0);
					pHandle = CreateDIBSection (dstDC, bmInfo, DIB_RGB_COLORS, &bits, NULL, 0);
					delete bmInfo;
					if (pHandle)
					{
						if (color_type == PNG_COLOR_TYPE_PALETTE)
							png_set_palette_to_rgb (png_ptr);
						if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
							png_set_gray_to_rgb (png_ptr);
						if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
							png_set_gray_1_2_4_to_8 (png_ptr);
						if (png_get_valid (png_ptr, info_ptr, PNG_INFO_tRNS))
							png_set_tRNS_to_alpha (png_ptr);
						else
							png_set_filler (png_ptr, 0xFF, PNG_FILLER_AFTER);
						if (bit_depth == 16)
						{
							png_set_swap (png_ptr);
							png_set_strip_16 (png_ptr);
						}
						if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
							png_set_bgr (png_ptr);
						png_read_update_info (png_ptr, info_ptr);

						unsigned char** rows = new unsigned char*[1];
						#if GDIPLUS
						rows[0] = (unsigned char*)bits;
						for (long i = 0; i < height; i++)
						{
							png_read_rows (png_ptr, rows, NULL, 1);
							rows[0] += bytesPerRow;
						}
						#else
						rows[0] = (unsigned char*)bits + (height-1) * bytesPerRow;
						for (long i = 0; i < height; i++)
						{
							png_read_rows (png_ptr, rows, NULL, 1);
							rows[0] -= bytesPerRow;
						}
						#endif
						delete [] rows;
						png_read_end (png_ptr, 0);
						#if 1 //!GDIPLUS
						// premultiply alpha
						unsigned long* pixelPtr = (unsigned long*)bits;
						for (int y = 0; y < height; y++)
						{
							for (int x = 0; x < width; x++)
							{
								unsigned char* pixel = (unsigned char*)pixelPtr;
								if (pixel[3] != 0)
								{
									pixel[0] = ((pixel[0] * pixel[3]) >> 8);
									pixel[1] = ((pixel[1] * pixel[3]) >> 8);
									pixel[2] = ((pixel[2] * pixel[3]) >> 8);
								}
								else
									*pixelPtr = 0UL;
								pixelPtr++;
							}
						}
						#endif
						if (dstDC)
							DeleteDC (dstDC);
#if 0
						HDC srcDC = CreateCompatibleDC (0);
						SelectObject (srcDC, pHandle);

						HDC dstDC = CreateCompatibleDC (0);
						this->pHandle = CreateCompatibleBitmap (dstDC, width, height);
						SelectObject (dstDC, this->pHandle);

						BLENDFUNCTION blendFunction;
						blendFunction.BlendOp = AC_SRC_OVER;
						blendFunction.BlendFlags = 0;
						blendFunction.SourceConstantAlpha = 255;
						#if USE_ALPHA_BLEND
						if (noAlpha)
							blendFunction.AlphaFormat = 0;//AC_SRC_NO_ALPHA;
						else
							blendFunction.AlphaFormat = AC_SRC_ALPHA;
						#else
						blendFunction.AlphaFormat = 0;//AC_SRC_NO_ALPHA;
						#endif
						#if DYNAMICALPHABLEND
						(*pfnAlphaBlend) (dstDC, 
									0, 0,
									width, height, 
									srcDC,
									0, 0,
									width, height,
									blendFunction);
						#else
						#endif

						DeleteDC (srcDC);
						DeleteDC (dstDC);
						DeleteObject (pHandle);
#endif
					}
				}
			}
			png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
		}
		noAlpha = false;
		result = true;
	}
	#endif
	if (!result)
	{
#if GDIPLUS
		pBitmap = Gdiplus::Bitmap::FromResource(GetInstance (),(WCHAR*)MAKEINTRESOURCE(resourceID));
		result = true;
		width = pBitmap->GetWidth ();
		height = pBitmap->GetHeight ();
#else
		pHandle = LoadBitmap (GetInstance (), MAKEINTRESOURCE (resourceID));
		BITMAP bm;
		if (pHandle && GetObject (pHandle, sizeof (bm), &bm))
		{
			width  = bm.bmWidth; 
			height = bm.bmHeight;
			noAlpha = true;
			result = true;
		}
#endif
	}
	
	//---------------------------------------------------------------------------------------------
	#elif MAC
	//---------------------------------------------------------------------------------------------
	pHandle = 0;
	pMask = 0;
	#if QUARTZ
	cgImage = 0;
	#endif
	#if MACX
	if (gBundleRef)
	{
		// find the bitmap in our Bundle. It must be in the form of bmp00123.png, where the resource id would be 123.
		char filename [PATH_MAX];
		sprintf (filename, "bmp%05d", (int)resourceID);
		CFStringRef cfStr = CFStringCreateWithCString (NULL, filename, kCFStringEncodingASCII);
		if (cfStr)
		{
			CFURLRef url = NULL;
			int i = 0;
			while (url == NULL)
			{
				static CFStringRef resTypes [] = { CFSTR("png"), CFSTR("bmp"), CFSTR("jpg"), CFSTR("pict"), NULL };
				url = CFBundleCopyResourceURL ((CFBundleRef)gBundleRef, cfStr, resTypes[i], NULL);
				if (resTypes[++i] == NULL)
					break;
			}
			CFRelease (cfStr);
			if (url)
			{
				result = loadFromPath (url);
				CFRelease (url);
			}
			else
			{
				#if DEVELOPMENT
				fprintf (stderr, "Bitmap Nr.:%d not found.\n", resourceID);
				#endif
			}
		}
	}
	#endif
	
	if (!result && pHandle == 0)
	{
		Handle picHandle = GetResource ('PICT', resourceID);
		if (picHandle)
		{
			HLock (picHandle);
			
			PictInfo info;
			GetPictInfo ((PicHandle)picHandle, &info, recordComments, 0, systemMethod, 0);
			width  = info.sourceRect.right;
			height = info.sourceRect.bottom;
			
			OSErr err = NewGWorld ((GWorldPtr*)&pHandle, 32, &info.sourceRect, 0, 0, 0);
			if (!err)
			{
				GWorldPtr oldPort;
				GDHandle oldDevice;
				GetGWorld (&oldPort, &oldDevice);
				SetGWorld ((GWorldPtr)pHandle, 0);
				
				DrawPicture ((PicHandle)picHandle, &info.sourceRect);
				
				SetGWorld (oldPort, oldDevice);
				result = true;
			}

			HUnlock (picHandle);
			ReleaseResource (picHandle);
		}
	}

	#else
	// other platforms go here
	#endif
	return result;
}

//-----------------------------------------------------------------------------
bool CBitmap::loadFromPath (const void* platformPath)
{
	bool result = false;

	dispose ();

	#if QUARTZ
	CFURLRef url = (CFURLRef)platformPath;

	FSRef fsRef;
	if (CFURLGetFSRef (url, &fsRef))
	{
		FSSpec fsSpec;
		FSCatalogInfoBitmap infoBitmap = kFSCatInfoNone;
		if (FSGetCatalogInfo (&fsRef, infoBitmap, NULL, NULL, &fsSpec, NULL) == noErr)
		{
			ComponentInstance* gi = 0;
			CFStringRef ext = CFURLCopyPathExtension (url);
			if (ext == 0)
				return false;
			if (CFStringCompare (ext, CFSTR("bmp"), 0) == kCFCompareEqualTo)
				gi = &bmpGI;
			else if (CFStringCompare (ext, CFSTR("png"), 0) == kCFCompareEqualTo)
			{
				gi = &pngGI;
				noAlpha = false;
			}
			else if (CFStringCompare (ext, CFSTR("jpg"), 0) == kCFCompareEqualTo)
				gi = &jpgGI;
			else if (CFStringCompare (ext, CFSTR("pict"), 0) == kCFCompareEqualTo)
				gi = &pictGI;
			CFRelease (ext);

			if (*gi == 0)
				GetGraphicsImporterForFile (&fsSpec, gi);
			else
				if (GraphicsImportSetDataFile (*gi, &fsSpec) != noErr)
					return false;
			if (*gi)
			{
				#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3
				if (!noAlpha && GraphicsImportCreateCGImage)
				{
					if (GraphicsImportCreateCGImage (*gi, (CGImageRef*)&cgImage, 0) == noErr)
					{
						width = CGImageGetWidth ((CGImageRef)cgImage);
						height = CGImageGetHeight ((CGImageRef)cgImage);
						result = true;
					}
				}
				else
				#endif
				{
					Rect r;
					GraphicsImportGetSourceRect (*gi, &r);
					OSErr err = NewGWorld ((GWorldPtr*)&pHandle, 32, &r, 0, 0, 0);
					if (!err)
					{
						width = r.right;
						height = r.bottom;
						GraphicsImportSetGWorld (*gi, (GWorldPtr)pHandle, 0);
						GraphicsImportDraw (*gi);
						result = true;
					}
				}
			}
		}
	}
	#elif WINDOWS
	// todo
	
	#endif
	
	return result;
}

//-----------------------------------------------------------------------------
bool CBitmap::isLoaded () const
{
	#if QUARTZ
	if (cgImage || getHandle ())
		return true;
	#else
	if (getHandle ())
		return true;
	#endif

	return false;
}

#if QUARTZ
class CDataProvider 
{
public:
	CDataProvider (CBitmap* bitmap) : bmp (bitmap) 
	{ 
		pos = 0; 
		PixMapHandle pixMap = GetGWorldPixMap ((GWorldPtr)bmp->getHandle ());
		ptr = (unsigned char*)GetPixBaseAddr (pixMap);
		color = bmp->getTransparentColor ();
	}

	static size_t getBytes (void *info, void *buffer, size_t count)
	{	// this could be optimized ;-)
		CDataProvider* p = (CDataProvider*)info;
		unsigned char* dst = (unsigned char*)buffer;
		unsigned char* src = p->ptr + p->pos;
		for (unsigned long i = 0; i < count / 4; i++)
		{
			if (src[1] == p->color.red && src[2] == p->color.green && src[3] == p->color.blue)
			{
				*dst++ = 0;
				src++;
			}
			else
				*dst++ = *src++;
			*dst++ = *src++;
			*dst++ = *src++;
			*dst++ = *src++;
		}
		p->pos += count;
		return count;
	}

	static void skipBytes (void *info, size_t count)
	{
		CDataProvider* p = (CDataProvider*)info;
		p->pos += count;
	}

	static void rewind (void *info)
	{
		CDataProvider* p = (CDataProvider*)info;
		p->pos = 0;
	}

	static void releaseProvider (void *info)
	{
		CDataProvider* p = (CDataProvider*)info;
		delete p;
	}

	unsigned long pos;
	CBitmap* bmp;
	unsigned char* ptr;
	CColor color;
};

//-----------------------------------------------------------------------------
CGImageRef CBitmap::createCGImage (bool transparent)
{
	if (cgImage)
	{
		CGImageRetain ((CGImageRef)cgImage);
		return (CGImageRef)cgImage;
	}
	if (!pHandle)
		return NULL;

	PixMapHandle pixMap = GetGWorldPixMap ((GWorldPtr)pHandle);
	
	Rect bounds;
	GetPixBounds (pixMap, &bounds);

	size_t pixRowBytes = GetPixRowBytes (pixMap);
	short pixDepth = GetPixDepth (pixMap);
	size_t size = pixRowBytes * (bounds.bottom - bounds.top);

	CGImageRef image = 0;
	CGDataProviderRef provider = 0;
	static CGDataProviderCallbacks callbacks = { CDataProvider::getBytes, CDataProvider::skipBytes, CDataProvider::rewind, CDataProvider::releaseProvider };
	if (transparent)
		provider = CGDataProviderCreate (new CDataProvider (this), &callbacks);
	else
		provider = CGDataProviderCreateWithData (NULL, GetPixBaseAddr (pixMap), size, NULL);
	CGImageAlphaInfo alphaInfo = kCGImageAlphaFirst;
	if (GetPixDepth (pixMap) != 32)
		alphaInfo = kCGImageAlphaNone;
	image = CGImageCreate (bounds.right - bounds.left, bounds.bottom - bounds.top, 8 , pixDepth, pixRowBytes, GetGenericRGBColorSpace (), alphaInfo, provider, NULL, false, kCGRenderingIntentDefault);
	CGDataProviderRelease (provider);

	cgImage = image;
	CGImageRetain (image);
	return image;
}
#endif
#if GDIPLUS
//-----------------------------------------------------------------------------
Gdiplus::Bitmap* CBitmap::getBitmap ()
{
	if (pBitmap == 0 && pHandle)
	{
		if (bits) // it´s a png image
		{
			pBitmap = new Gdiplus::Bitmap (width, height, 4*width, PixelFormat32bppPARGB, (unsigned char*)bits);
		}
		else
			pBitmap = new Gdiplus::Bitmap ((HBITMAP)pHandle, 0);
	}

	return pBitmap;
}

#endif

//-----------------------------------------------------------------------------
void CBitmap::draw (CDrawContext *pContext, CRect &rect, const CPoint &offset)
{
#if WINDOWS
	#if GDIPLUS
	drawAlphaBlend (pContext, rect, offset, 255);
	#else
	#if USE_ALPHA_BLEND
	if (!noAlpha)
	{
		drawAlphaBlend (pContext, rect, offset, 255);
		return;
	}
	#endif	

	if (pHandle)
	{
		HGDIOBJ hOldObj;
		HDC hdcMemory = CreateCompatibleDC ((HDC)pContext->pSystemContext);
		hOldObj = SelectObject (hdcMemory, pHandle);
		BitBlt ((HDC)pContext->pSystemContext, 
						rect.left + pContext->offset.h, rect.top + pContext->offset.v, rect.width (), rect.height (), 
						(HDC)hdcMemory, offset.h, offset.v, SRCCOPY);
		SelectObject (hdcMemory, hOldObj);
		DeleteDC (hdcMemory);
	}
	#endif

#elif MAC

	#if QUARTZ
	drawAlphaBlend (pContext, rect, offset, 255);

	#else
	Rect source, dest;
	dest.top    = rect.top  + pContext->offset.v;
	dest.left   = rect.left + pContext->offset.h;
	dest.bottom = dest.top  + rect.height ();
	dest.right  = dest.left + rect.width ();
		
	source.top    = offset.v;
	source.left   = offset.h;
	source.bottom = source.top  + rect.height ();
	source.right  = source.left + rect.width ();

	pContext->getPort ();
	BitMapPtr bitmapPtr = pContext->getBitmap ();
	
	if (pHandle && bitmapPtr)
	{
		PixMapHandle pmHandle = GetGWorldPixMap ((GWorldPtr)pHandle);
		if (pmHandle && LockPixels (pmHandle))
		{
			RGBColor oldForeColor, oldBackColor;
			GetForeColor (&oldForeColor);
			GetBackColor (&oldBackColor);
			::BackColor (whiteColor);
			::ForeColor (blackColor);
			
			CopyBits ((BitMapPtr)*pmHandle, bitmapPtr, &source, &dest, srcCopy, 0L);
			#if MACX
			QDAddRectToDirtyRegion (pContext->getPort (), &dest);
			#endif
			
			RGBForeColor (&oldForeColor);
			RGBBackColor (&oldBackColor);
			
			UnlockPixels (pmHandle);
		}
	}
	
	pContext->releaseBitmap ();
	#endif

#elif BEOS
	BRect	brect (rect.left, rect.top, rect.right - 1, rect.bottom - 1);
	BRect	drect = brect;
	brect.OffsetTo (offset.h, offset.v);
	drect.OffsetBy (pContext->offset.h, pContext->offset.v);
	pContext->pView->SetDrawingMode (B_OP_COPY);
	pContext->pView->DrawBitmap (bbitmap, brect, drect);
#endif
}

//-----------------------------------------------------------------------------
void CBitmap::drawTransparent (CDrawContext *pContext, CRect &rect, const CPoint &offset)
{
#if WINDOWS
	#if GDIPLUS
	drawAlphaBlend (pContext, rect, offset, 255);
	#else
	#if USE_ALPHA_BLEND
	if (!noAlpha)
	{
		drawAlphaBlend (pContext, rect, offset, 255);
		return;
	}
	#endif	

	BITMAP bm;
	HDC hdcBitmap;
	POINT ptSize;

	hdcBitmap = CreateCompatibleDC ((HDC)pContext->pSystemContext);
	SelectObject (hdcBitmap, pHandle);	 // Select the bitmap

	GetObject (pHandle, sizeof (BITMAP), (LPSTR)&bm);
	ptSize.x = bm.bmWidth;            // Get width of bitmap
	ptSize.y = bm.bmHeight;           // Get height of bitmap
	DPtoLP (hdcBitmap, &ptSize, 1);   // Convert from device to logical points

	DrawTransparent (pContext, rect, offset, hdcBitmap, ptSize, (HBITMAP)pMask, RGB(transparentCColor.red, transparentCColor.green, transparentCColor.blue));

	DeleteDC (hdcBitmap);
	#endif

#elif MAC

	#if QUARTZ
	if (noAlpha)
	{
		CGImageRef image = createCGImage (true);
		if (image)
		{
			drawAlphaBlend (pContext, rect, offset, 255);
			CGImageRelease (image);
		}
	}
	else
		drawAlphaBlend (pContext, rect, offset, 255);

	#else
	Rect source, dest;
	dest.top    = rect.top  + pContext->offset.v;
	dest.left   = rect.left + pContext->offset.h;
	dest.bottom = dest.top  + rect.height ();
	dest.right  = dest.left + rect.width ();
	
	source.top    = offset.v;
	source.left   = offset.h;
	source.bottom = source.top  + rect.height ();
	source.right  = source.left + rect.width ();

	pContext->getPort ();
	BitMapPtr bitmapPtr = pContext->getBitmap ();

	if (pHandle && bitmapPtr)
	{
		PixMapHandle pmHandle = GetGWorldPixMap ((GWorldPtr)pHandle);
		if (pmHandle && LockPixels (pmHandle))
		{
			RGBColor oldForeColor, oldBackColor;
			GetForeColor (&oldForeColor);
			GetBackColor (&oldBackColor);
			
			RGBColor col;
			CColor2RGBColor (transparentCColor, col);
			RGBBackColor (&col);
			::ForeColor (blackColor);
			
			if (pMask)
			{
				PixMapHandle pmHandleMask = GetGWorldPixMap ((GWorldPtr)pMask);
				if (pmHandleMask && LockPixels (pmHandleMask))
				{
					CopyMask ((BitMapPtr)*pmHandle, (BitMapPtr)*pmHandleMask, bitmapPtr, 
								&source, &source, &dest);
										
					UnlockPixels (pmHandleMask);
				}
			}
			else
				CopyBits ((BitMapPtr)*pmHandle, bitmapPtr, &source, &dest, transparent, 0L);
			
			RGBForeColor (&oldForeColor);
			RGBBackColor (&oldBackColor);

			#if MACX
			QDAddRectToDirtyRegion (pContext->getPort (), &dest);
			#endif
			
			UnlockPixels (pmHandle);
		}
	}

	pContext->releaseBitmap ();
	#endif
        
#elif BEOS
	if (!transparencySet)
	{
		uint32 c32 = transparentCColor.red | (transparentCColor.green  << 8) | (transparentCColor.blue << 16);
		uint32 *pix = (uint32*) bbitmap->Bits ();
		uint32 ctr = B_TRANSPARENT_32_BIT.red | (B_TRANSPARENT_32_BIT.green << 8) | (B_TRANSPARENT_32_BIT.blue << 16) | (B_TRANSPARENT_32_BIT.alpha << 24);
		
		for (int32 z = 0, count = bbitmap->BitsLength () / 4; z < count; z++)
		{
			if ((pix[z] & 0xffffff) == c32) 
				pix[z] = ctr;
		}
		transparencySet = true;
	}
	BRect	brect (rect.left, rect.top, rect.right - 1, rect.bottom - 1);
	BRect	drect = brect;
	brect.OffsetTo (offset.h, offset.v);
	drect.OffsetBy (pContext->offset.h, pContext->offset.v);
	pContext->pView->SetDrawingMode (B_OP_OVER);
	pContext->pView->DrawBitmap (bbitmap, brect, drect);

#endif
}

//-----------------------------------------------------------------------------
void CBitmap::drawAlphaBlend (CDrawContext *pContext, CRect &rect, const CPoint &offset, unsigned char alpha)
{
#if WINDOWS
	#if GDIPLUS
	Gdiplus::Bitmap* bitmap = getBitmap ();
	if (bitmap)
	{
		Gdiplus::Graphics* graphics = pContext->getGraphics ();
		if (graphics)
		{
			graphics->DrawImage (bitmap, rect.left + pContext->offset.h, rect.top + pContext->offset.v, offset.x, offset.y, rect.getWidth (), rect.getHeight (), Gdiplus::UnitPixel);
		}
	}
	#else
	if (pHandle)
	{
		HGDIOBJ hOldObj;
		HDC hdcMemory = CreateCompatibleDC ((HDC)pContext->pSystemContext);
		hOldObj = SelectObject (hdcMemory, pHandle);

		BLENDFUNCTION blendFunction;
		blendFunction.BlendOp = AC_SRC_OVER;
		blendFunction.BlendFlags = 0;
		blendFunction.SourceConstantAlpha = alpha;
		#if USE_ALPHA_BLEND
		if (noAlpha)
			blendFunction.AlphaFormat = 0;//AC_SRC_NO_ALPHA;
		else
			blendFunction.AlphaFormat = AC_SRC_ALPHA;
		#else
		blendFunction.AlphaFormat = 0;//AC_SRC_NO_ALPHA;
		#endif
		#if DYNAMICALPHABLEND
		// check for Win98 as it has a bug in AlphaBlend
		if (gSystemVersion.dwMajorVersion == 4 && gSystemVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && gSystemVersion.dwMinorVersion == 10)
		{
			HGDIOBJ hOldObj1;
			HDC hdcMemory1 = CreateCompatibleDC ((HDC)pContext->pSystemContext);
			HBITMAP hbmp = CreateCompatibleBitmap(hdcMemory, rect.width(), rect.height());
			//this does NOT work:
			//HBITMAP hbmp = CreateCompatibleBitmap(hdcMemory1, rect.width(), rect.height());
			hOldObj1 = SelectObject (hdcMemory1, hbmp);

			//copy contents of original picture in hdcMemory 
			//from the offset to hdcMemory1 (0,0)
			long res = BitBlt((HDC)hdcMemory1, 
					0, 0, 
					rect.width(), rect.height(), 
					(HDC)hdcMemory, offset.h, offset.v, SRCCOPY);

			//Copy the resulting image with alpha blending:
			(*pfnAlphaBlend) ((HDC)pContext->pSystemContext, 
						rect.left + pContext->offset.h, rect.top + pContext->offset.v,
						rect.width (), rect.height (), 
						hdcMemory1,
						0, 0,//the offset is done in BitBlt
						rect.width (), rect.height (),
						blendFunction);
			SelectObject (hdcMemory1, hOldObj1);
			DeleteDC(hdcMemory1);
			DeleteObject(hbmp);
		}
		else
		{
			(*pfnAlphaBlend) ((HDC)pContext->pSystemContext, 
						rect.left + pContext->offset.h, rect.top + pContext->offset.v,
						rect.width (), rect.height (), 
						(HDC)hdcMemory,
						offset.h, offset.v,
						rect.width (), rect.height (),
						blendFunction);
		}
		#else
		AlphaBlend ((HDC)pContext->pSystemContext, 
					rect.left + pContext->offset.h, rect.top + pContext->offset.v,
					rect.width (), rect.height (), 
					(HDC)hdcMemory,
					offset.h, offset.v,
					rect.width (), rect.height (),
					blendFunction);
		#endif
		SelectObject (hdcMemory, hOldObj);
		DeleteDC (hdcMemory);
	}
	#endif

#elif MAC

    #if QUARTZ
	if (pHandle || cgImage)
	{
		CGContextRef context = pContext->beginCGContext ();
		if (context)
		{
			if (alpha != 255)
				CGContextSetAlpha (context, (float)alpha / 255.f);
			
			CGImageRef image = createCGImage ();

			if (image)
			{
				CGRect dest;
				dest.origin.x = rect.left - offset.h + pContext->offset.h;
				dest.origin.y = (rect.top + pContext->offset.v) * -1 - (getHeight () - offset.v);
				dest.size.width = getWidth ();
				dest.size.height = getHeight ();
				
				CRect ccr;
				pContext->getClipRect (ccr);
				CGRect cgClipRect = CGRectMake (ccr.left + pContext->offset.h, (ccr.top + pContext->offset.v) * -1 - ccr.height (), ccr.width (), ccr.height ());
				CGContextClipToRect (context, cgClipRect);

				CGRect clipRect;
				clipRect.origin.x = rect.left + pContext->offset.h;
			    clipRect.origin.y = (rect.top + pContext->offset.v) * -1  - rect.height ();
			    clipRect.size.width = rect.width (); 
			    clipRect.size.height = rect.height ();
				
				CGContextClipToRect (context, clipRect);

				CGContextDrawImage (context, dest, image);
				CGImageRelease (image);
			}
			pContext->releaseCGContext (context);
		}
	}
	
    #else
	Rect source, dest;
	dest.top    = rect.top  + pContext->offset.v;
	dest.left   = rect.left + pContext->offset.h;
	dest.bottom = dest.top  + rect.height ();
	dest.right  = dest.left + rect.width ();
		
	source.top    = offset.v;
	source.left   = offset.h;
	source.bottom = source.top  + rect.height ();
	source.right  = source.left + rect.width ();
		
	pContext->getPort ();
	BitMapPtr bitmapPtr = pContext->getBitmap ();
	if (bitmapPtr)
	{
		RGBColor col;
		CColor color = {alpha, alpha, alpha, 0};
		CColor2RGBColor (color, col);
		OpColor (&col);
		
		if (pHandle)
		{
			PixMapHandle pmHandle = GetGWorldPixMap ((GWorldPtr)pHandle);
			if (pmHandle && LockPixels (pmHandle))
			{
				RGBColor oldForeColor, oldBackColor;
				GetForeColor (&oldForeColor);
				GetBackColor (&oldBackColor);
				::BackColor (whiteColor);
				::ForeColor (blackColor);
			
				CopyBits ((BitMapPtr)*pmHandle, bitmapPtr, &source, &dest, blend, 0L);
				#if MACX
				QDAddRectToDirtyRegion (pContext->getPort (), &dest);
				#endif
			
				RGBForeColor (&oldForeColor);
				RGBBackColor (&oldBackColor);
			
				UnlockPixels (pmHandle);
			}
		}
	}

	pContext->releaseBitmap ();
        #endif
#endif
}
//-----------------------------------------------------------------------------
void CBitmap::setTransparentColor (const CColor color)
{
	transparentCColor = color;
#if QUARTZ
	if (noAlpha)
	{
		if (cgImage)
			CGImageRelease ((CGImageRef)cgImage);
		cgImage = 0;
	}
#endif
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
void CBitmap::setTransparencyMask (CDrawContext* pContext, const CPoint& offset)
{
#if WINDOWS
	#if GDIPLUS
	#else
	if (pMask)
		DeleteObject (pMask);

	CRect r (0, 0, width, height);
	r.offset (offset.h, offset.v);
	pMask = CreateMaskBitmap (pContext, r, transparentCColor);
	#endif

#elif MAC
	#if QUARTZ
	#else
	if (pMask)
		DisposeGWorld ((GWorldPtr)pMask);
	pMask = 0;
	
	Rect r;
	r.left = r.top = 0;
	r.right = width;
	r.bottom = height;
	OSErr err = NewGWorld ((GWorldPtr*)&pMask, 1, &r, 0, 0, 0); // create monochrome GWorld
	if (!err)
	{
		GWorldPtr oldPort;
		GDHandle oldDevice;
		GetGWorld (&oldPort, &oldDevice);
		SetGWorld ((GWorldPtr)pMask, 0);

		PixMapHandle pmHandle = GetGWorldPixMap ((GWorldPtr)pMask);
		BitMapPtr sourcePtr = pContext->getBitmap ();
		
		if (sourcePtr && pmHandle && LockPixels (pmHandle))
		{
			RGBColor oldForeColor, oldBackColor;
			GetForeColor (&oldForeColor);
			GetBackColor (&oldBackColor);
			
			RGBColor col;
			CColor2RGBColor (transparentCColor, col);
			RGBBackColor (&col);

			::ForeColor (blackColor);

			Rect src = r;
			src.left   += offset.h;
			src.right  += offset.h;
			src.top    += offset.v;
			src.bottom += offset.v;

			CopyBits (sourcePtr, (BitMapPtr)*pmHandle, &src, &r, srcCopy, 0L);

			RGBForeColor (&oldForeColor);
			RGBBackColor (&oldBackColor);
			
			UnlockPixels (pmHandle);
		}
		
		pContext->releaseBitmap ();
				
		SetGWorld (oldPort, oldDevice);
	}
	#endif
        
#else
	// todo: implement me!
#endif
}
#endif // VSTGUI_ENABLE_DEPRECATED_METHODS

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#if BEOS
//----------------------------------------------------------------------------

BResources* CBitmap::resourceFile = 0;

//----------------------------------------------------------------------------

void CBitmap::closeResource ()
{
	if (resourceFile)
	{
		delete resourceFile;
		resourceFile = 0;
	}
}

//----------------------------------------------------------------------------
#endif


//-----------------------------------------------------------------------------
// CDragContainer Implementation
//-----------------------------------------------------------------------------
CDragContainer::CDragContainer (void* platformDrag)
: platformDrag (platformDrag)
, nbItems (0)
, iterator (0)
, lastItem (0)
{
	#if MAC
	DragRef dragRef = (DragRef)platformDrag;
	UInt16 numItems;
	CountDragItems (dragRef, &numItems);
	nbItems = numItems;
	
	#elif WINDOWS
	
	IDataObject* dataObject = (IDataObject*)platformDrag;
	STGMEDIUM medium;
	FORMATETC formatTEXTDrop = {CF_TEXT,  0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	FORMATETC formatHDrop    = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	long type = 0; // 0 = file, 1 = text

	HRESULT hr = dataObject->GetData (&formatTEXTDrop, &medium);
	if (hr != S_OK)
		hr = dataObject->GetData (&formatHDrop, &medium);
	else
		type = 1;
	
	if (type == 0)
		nbItems = (long)DragQueryFile ((HDROP)medium.hGlobal, 0xFFFFFFFFL, 0, 0);
	else
		nbItems = 1;
	
	#else
	#endif
}

//-----------------------------------------------------------------------------
CDragContainer::~CDragContainer ()
{
	if (lastItem)
	{
		free (lastItem);
		lastItem = 0;
	}
}

//-----------------------------------------------------------------------------
long CDragContainer::getType (long idx) const
{
	#if MACX
	DragItemRef itemRef;
	if (GetDragItemReferenceNumber ((DragRef)platformDrag, idx+1, &itemRef) == noErr)
	{
		FlavorType type;
		if (GetFlavorType ((DragRef)platformDrag, itemRef, 1, &type) == noErr)
		{
			if (type == flavorTypeHFS || type == typeFileURL)
				return kFile;
			else if (type == 'TEXT' || type == 'XML ')
				return kText;
		}
	}
	#elif WINDOWS
	IDataObject* dataObject = (IDataObject*)platformDrag;
	STGMEDIUM medium;
	FORMATETC formatTEXTDrop = {CF_TEXT,  0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	FORMATETC formatHDrop    = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	long type = 0; // 0 = file, 1 = text

	HRESULT hr = dataObject->GetData (&formatTEXTDrop, &medium);
	if (hr != S_OK)
		hr = dataObject->GetData (&formatHDrop, &medium);
	else
		type = 1;
	if (type == 0)
		return kFile;
	else
		return kText;

	#else
	// not implemented
	#endif
	return kUnknown;
}

//-----------------------------------------------------------------------------
void* CDragContainer::first (long& size, long& type)
{
	iterator = 0;
	return next (size, type);
}

//-----------------------------------------------------------------------------
void* CDragContainer::next (long& size, long& type)
{
	if (lastItem)
	{
		free (lastItem);
		lastItem = 0;
	}
	size = 0;
	type = kUnknown;
	#if MACX
	long flavorSize;
	DragItemRef itemRef;
	if (GetDragItemReferenceNumber ((DragRef)platformDrag, ++iterator, &itemRef) == noErr)
	{
		FlavorType flavorType;
		if (GetFlavorType ((DragRef)platformDrag, itemRef, 1, &flavorType) == noErr)
		{
			if (flavorType == flavorTypeHFS)
			{
				HFSFlavor     hfs;
				if (GetFlavorDataSize ((DragRef)platformDrag, itemRef, flavorTypeHFS, &flavorSize) == noErr)
				{ 
					GetFlavorData ((DragRef)platformDrag, itemRef, flavorTypeHFS, &hfs, &flavorSize, 0L);
					
					FSRef fsRef;
					if (FSpMakeFSRef (&hfs.fileSpec, &fsRef) == noErr)
					{
						lastItem = malloc (PATH_MAX);
						if (FSRefMakePath (&fsRef, (unsigned char*)lastItem, PATH_MAX) == noErr)
						{
							size = strlen ((const char*)lastItem);
							type = kFile;
							return lastItem;
						}
						}
					}
			}
			else if (flavorType == typeFileURL)
			{
				if (GetFlavorDataSize ((DragRef)platformDrag, itemRef, typeFileURL, &flavorSize) == noErr)
				{
					void* bytes = malloc (flavorSize);
					if (GetFlavorData ((DragRef)platformDrag, itemRef, typeFileURL, bytes, &flavorSize, 0L) == noErr)
					{
						CFURLRef url = CFURLCreateWithBytes (NULL, (const unsigned char*)bytes, flavorSize, kCFStringEncodingUTF8, NULL);
						lastItem = malloc (PATH_MAX);
						CFURLGetFileSystemRepresentation (url, false, (unsigned char*)lastItem, PATH_MAX);
						CFRelease (url);
						type = kFile;
					}
					free (bytes);
					return lastItem;
				}
			}
			else
			{
				if (GetFlavorDataSize ((DragRef)platformDrag, itemRef, flavorType, &flavorSize) == noErr)
				{
					lastItem = malloc (flavorSize + 1);
					((char*)lastItem)[0] = 0;
					if (GetFlavorData ((DragRef)platformDrag, itemRef, flavorType, lastItem, &flavorSize, 0) == noErr)
					{
						((char*)lastItem)[flavorSize] = 0;
						size = flavorSize;
						if (flavorType == 'TEXT' || flavorType == 'XML ')
							type = kText;
						return lastItem;
					}
				}
				else
				{
					if (GetFlavorDataSize ((DragRef)platformDrag, itemRef, 'TEXT', &flavorSize) == noErr)
					{
						lastItem = malloc (flavorSize + 1);
						((char*)lastItem)[0] = 0;
						if (GetFlavorData ((DragRef)platformDrag, itemRef, 'TEXT', lastItem, &flavorSize, 0) == noErr)
						{
							((char*)lastItem)[flavorSize] = 0;
							size = flavorSize;
							type = kText;
							return lastItem;
						}
					}
				}
				
			}
		}
	}
	#elif WINDOWS
	IDataObject* dataObject = (IDataObject*)platformDrag;
	void* hDrop = 0;
	STGMEDIUM medium;
	FORMATETC formatTEXTDrop = {CF_TEXT,  0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	FORMATETC formatHDrop    = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	long wintype = 0; // 0 = file, 1 = text

	HRESULT hr = dataObject->GetData (&formatTEXTDrop, &medium);
	if (hr != S_OK)
		hr = dataObject->GetData (&formatHDrop, &medium);
	else
		wintype = 1;
	if (hr == S_OK)
		hDrop = medium.hGlobal;

	if (hDrop)
	{
		if (wintype == 0)
		{
			char fileDropped[1024];

			long nbRealItems = 0;
			if (DragQueryFile ((HDROP)hDrop, iterator++, fileDropped, sizeof (fileDropped))) 
			{
				// resolve link
				checkResolveLink (fileDropped, fileDropped);
				lastItem = malloc (strlen (fileDropped)+1);
				strcpy ((char*)lastItem, fileDropped);
				size = (long)strlen ((const char*)lastItem);
				type = kFile;
				return lastItem;
			}
		}
		else if (iterator++ == 0)
		//---TEXT----------------------------
		{
			void* data = GlobalLock (medium.hGlobal);
			long dataSize = (long)GlobalSize (medium.hGlobal);
			if (data && dataSize)
			{
				lastItem = malloc (dataSize+1);
				memcpy (lastItem, data, dataSize);
				size = dataSize;
				type = kText;
			}

			GlobalUnlock (medium.hGlobal);
			if (medium.pUnkForRelease)
				medium.pUnkForRelease->Release ();
			else
				GlobalFree (medium.hGlobal);
			return lastItem;
		}
	}
	#else
	// not implemented
	#endif
	return NULL;
}

END_NAMESPACE_VSTGUI


#if WINDOWS

#if USE_MOUSE_HOOK
HHOOK MouseHook = 0L;

LRESULT CALLBACK MouseProc (int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)
		return CallNextHookEx (MouseHook, nCode, wParam, lParam);

	if (wParam == 522)
	{
		MOUSEHOOKSTRUCT* struct2 = (MOUSEHOOKSTRUCT*) lParam;
		if (struct2->hwnd == ???)
		{
			return -1;
		}
	}
	return CallNextHookEx (MouseHook, nCode, wParam, lParam);
}
#endif

//-----------------------------------------------------------------------------
bool InitWindowClass ()
{
	gUseCount++;
	if (gUseCount == 1)
	{
		sprintf (gClassName, "Plugin%x", GetInstance ());
		
		WNDCLASS windowClass;
		windowClass.style = CS_GLOBALCLASS;//|CS_OWNDC; // add Private-DC constant 

		windowClass.lpfnWndProc = WindowProc; 
		windowClass.cbClsExtra  = 0; 
		windowClass.cbWndExtra  = 0; 
		windowClass.hInstance   = GetInstance (); 
		windowClass.hIcon = 0; 

		windowClass.hCursor = LoadCursor (NULL, IDC_ARROW);
		windowClass.hbrBackground = 0; //GetSysColorBrush (COLOR_BTNFACE); // must be NULL, really, only for debugging is this something else 
		windowClass.lpszMenuName  = 0; 
		windowClass.lpszClassName = gClassName; 
		RegisterClass (&windowClass);

		#if USE_MOUSE_HOOK
		MouseHook = SetWindowsHookEx (WH_MOUSE, MouseProc, GetInstance (), 0);
		#endif

		bSwapped_mouse_buttons = GetSystemMetrics (SM_SWAPBUTTON) > 0;
	}
	return true;
}

//-----------------------------------------------------------------------------
void ExitWindowClass ()
{
	gUseCount--;
	if (gUseCount == 0)
	{
		UnregisterClass (gClassName, GetInstance ());

		#if USE_MOUSE_HOOK
		if (MouseHook)
		{
			UnhookWindowsHookEx (MouseHook);
			MouseHook = 0L;
		}
		#endif
	}
}

//-----------------------------------------------------------------------------
LONG_PTR WINAPI WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	USING_NAMESPACE_VSTGUI
	CFrame* pFrame = (CFrame*)GetWindowLongPtr (hwnd, GWLP_USERDATA);

	switch (message)
	{
	case WM_MOUSEWHEEL:
	{
		if (pFrame)
		{
			long buttons = 0;
			if (GetKeyState (VK_SHIFT)   < 0)
				buttons |= kShift;
			if (GetKeyState (VK_CONTROL) < 0)
				buttons |= kControl;
			if (GetKeyState (VK_MENU)    < 0)
				buttons |= kAlt;
			VSTGUI_CPoint where (LOWORD (lParam), HIWORD (lParam));
			short zDelta = (short) HIWORD(wParam);
			RECT rctWnd;
			GetWindowRect (hwnd, &rctWnd);
			where.offset (-rctWnd.left, -rctWnd.top);
			pFrame->onWheel (where, (float)(zDelta / WHEEL_DELTA), buttons); // todo, check modifier
		}
		break;
	}
	case WM_CTLCOLOREDIT:
	{
		if (pFrame)
		{
			VSTGUI_CTextEdit *textEdit = (VSTGUI_CTextEdit*)pFrame->getFocusView ();
			if (textEdit)
			{
				VSTGUI_CColor fontColor = textEdit->getFontColor ();
				SetTextColor ((HDC) wParam, RGB (fontColor.red, fontColor.green, fontColor.blue));

				VSTGUI_CColor backColor = textEdit->getBackColor ();
				SetBkColor ((HDC) wParam, RGB (backColor.red, backColor.green, backColor.blue));

				if (textEdit->platformFontColor)
					DeleteObject (textEdit->platformFontColor);
				textEdit->platformFontColor = CreateSolidBrush (RGB (backColor.red, backColor.green, backColor.blue));
				return (LRESULT)(textEdit->platformFontColor);
			}
		}
	}
	break;

	case WM_PAINT:
	{
		if (pFrame)// && GetUpdateRect (hwnd, &r, false))
		{
			HRGN rgn = CreateRectRgn (0, 0, 0, 0);
			if (GetUpdateRgn (hwnd, rgn, false) == NULLREGION)
			{
				DeleteObject (rgn);
				return 0;
			}

			PAINTSTRUCT ps;
			HDC hdc = BeginPaint (hwnd, &ps);

			VSTGUI_CDrawContext* context = pFrame->getBackBuffer ();
			if (!context)
				context = new VSTGUI_CDrawContext (pFrame, hdc, hwnd);
			
			CRect updateRect (ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);

			#if 0
			int len = GetRegionData (rgn, 0, NULL);
			if (len)
			{
				RGNDATA *rlist = (RGNDATA *)new char[len];
				GetRegionData (rgn, len, rlist);
				if (rlist->rdh.nCount > 0)
				{
					RECT* rp = (RECT*)rlist->Buffer;
					for (unsigned int i = 0; i < rlist->rdh.nCount; i++)
					{
						CRect ur (rp->left, rp->top, rp->right, rp->bottom);
						pFrame->drawRect (context, ur);
						rp++;
					}
				}
				else
					pFrame->drawRect (context, updateRect);
				delete [] (char*)rlist;
			}
			else
			#endif
				pFrame->drawRect (context, updateRect);

			if (pFrame->getBackBuffer ())
			{
				VSTGUI_CDrawContext localContext (pFrame, hdc, hwnd);
				pFrame->getBackBuffer ()->copyFrom (&localContext, updateRect, CPoint (ps.rcPaint.left, ps.rcPaint.top));
			}
			else
				context->forget ();


			EndPaint (hwnd, &ps);
			DeleteObject (rgn);
			return 0;
		}
	}
	break;

	case WM_MEASUREITEM :
	{
		MEASUREITEMSTRUCT* ms = (MEASUREITEMSTRUCT*)lParam;
		if (pFrame && ms && ms->CtlType == ODT_MENU && ms->itemData)
		{
			VSTGUI_COptionMenu* optMenu = (VSTGUI_COptionMenu*)pFrame->getFocusView ();
			if (optMenu && optMenu->getScheme ())
			{
				VSTGUI_CPoint size;

				VSTGUI_CDrawContext context (pFrame, 0, hwnd);
				optMenu->getScheme ()->getItemSize ((const char*)ms->itemData, &context, size);

				ms->itemWidth  = size.h;
				ms->itemHeight = size.v;
				return TRUE;
			}
		}
	}
	break;

	case WM_DRAWITEM :
	{
		DRAWITEMSTRUCT* ds = (DRAWITEMSTRUCT*)lParam;
		if (pFrame && ds && ds->CtlType == ODT_MENU && ds->itemData)
		{
			VSTGUI_COptionMenu* optMenu = (VSTGUI_COptionMenu*)pFrame->getFocusView ();
			if (optMenu && optMenu->getScheme ())
			{
				long state = 0;
				if (ds->itemState & ODS_CHECKED)
					state |= VSTGUI_COptionMenuScheme::kChecked;
				if (ds->itemState & ODS_DISABLED) // ODS_GRAYED?
					state |= VSTGUI_COptionMenuScheme::kDisabled;
				if (ds->itemState & ODS_SELECTED)
					state |= VSTGUI_COptionMenuScheme::kSelected;
					
				CRect r (ds->rcItem.left, ds->rcItem.top, ds->rcItem.right, ds->rcItem.bottom);
				r.bottom++;
				
				VSTGUI_CDrawContext* pContext = new VSTGUI_CDrawContext (pFrame, ds->hDC, 0);
				optMenu->getScheme ()->drawItem ((const char*)ds->itemData, ds->itemID, state, pContext, r);
				delete pContext;
				return TRUE;
			}
		}
	}
	break;
	
#if 1
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONDOWN:
		if (pFrame)
		{
			long buttons = 0;
			if (wParam & MK_LBUTTON)
				buttons |= kLButton;
			if (wParam & MK_RBUTTON)
				buttons |= kRButton;
			if (wParam & MK_MBUTTON)
				buttons |= kMButton;
			if (wParam & MK_CONTROL)
				buttons |= kControl;
			if (wParam & MK_SHIFT)
				buttons |= kShift;
			VSTGUI_CPoint where (LOWORD (lParam), HIWORD (lParam));
			if (pFrame->onMouseDown (where, buttons) == kMouseEventHandled)
				SetCapture ((HWND)pFrame->getSystemWindow ());
			return 0;
		}
		break;
	case WM_MOUSEMOVE:
		if (pFrame)
		{
			long buttons = 0;
			if (wParam & MK_LBUTTON)
				buttons |= kLButton;
			if (wParam & MK_RBUTTON)
				buttons |= kRButton;
			if (wParam & MK_MBUTTON)
				buttons |= kMButton;
			if (wParam & MK_CONTROL)
				buttons |= kControl;
			if (wParam & MK_SHIFT)
				buttons |= kShift;
			VSTGUI_CPoint where (LOWORD (lParam), HIWORD (lParam));
			pFrame->onMouseMoved (where, buttons);
			return 0;
		}
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		if (pFrame)
		{
			long buttons = 0;
			if (wParam & MK_LBUTTON)
				buttons |= kLButton;
			if (wParam & MK_RBUTTON)
				buttons |= kRButton;
			if (wParam & MK_MBUTTON)
				buttons |= kMButton;
			if (wParam & MK_CONTROL)
				buttons |= kControl;
			if (wParam & MK_SHIFT)
				buttons |= kShift;
			VSTGUI_CPoint where (LOWORD (lParam), HIWORD (lParam));
			pFrame->onMouseUp (where, buttons);
			ReleaseCapture ();
			return 0;
		}
		break;
#else
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONDOWN:
		if (pFrame)
		{
		#if 1
			VSTGUI_CDrawContext context (pFrame, 0, hwnd);
			VSTGUI_CPoint where (LOWORD (lParam), HIWORD (lParam));
			pFrame->mouse (&context, where);
		#else
			VSTGUI_CPoint where (LOWORD (lParam), HIWORD (lParam));
			pFrame->mouse ((VSTGUI_CDrawContext*)0, where);
		#endif

			return 0;
		}
		break;
#endif
		
	case WM_DESTROY:
		if (pFrame)
		{
			pFrame->setOpenFlag (false);
			pFrame->setParentSystemWindow (0);
		}
		break;
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}

#if !GDIPLUS
//-----------------------------------------------------------------------------
HANDLE CreateMaskBitmap (CDrawContext* pContext, CRect& rect, CColor transparentColor)
{
	HBITMAP pMask = CreateBitmap (rect.width (), rect.height (), 1, 1, 0);

	HDC hSrcDC = (HDC)pContext->getSystemContext ();
	HDC hDstDC = CreateCompatibleDC (hSrcDC);
	SelectObject (hDstDC, pMask);

	COLORREF oldBkColor = SetBkColor (hSrcDC, RGB (transparentColor.red, transparentColor.green, transparentColor.blue));
	
	BitBlt (hDstDC, 0, 0, rect.width (), rect.height (), hSrcDC, rect.left, rect.top, SRCCOPY);
	
	SetBkColor (hSrcDC, oldBkColor);
	DeleteDC (hDstDC);
	
	return pMask;
}

//-----------------------------------------------------------------------------
void DrawTransparent (CDrawContext* pContext, CRect& rect, const CPoint& offset,
					  HDC hdcBitmap, POINT ptSize, HBITMAP pMask, COLORREF color)
{
	if (pMask == NULL)
	{
		if (pfnTransparentBlt)
		{
			HDC		hdcSystemContext = (HDC)pContext->getSystemContext ();
			long	x, y;
			long	width  = rect.width ();
			long	height = rect.height ();

			x = rect.x + pContext->offset.x;
			y = rect.y + pContext->offset.y;

			pfnTransparentBlt (hdcSystemContext, x, y, width, height, hdcBitmap, offset.x, offset.y, width, height, color);
		}
		else
		{
			// OPTIMIZATION: we only do four instead of EIGHT blits
			HDC		hdcSystemContext = (HDC)pContext->getSystemContext ();
			HDC		hdcMask = CreateCompatibleDC (hdcSystemContext);

			COLORREF	crOldBack = SetBkColor (hdcSystemContext, 0xFFFFFF);
			COLORREF	crOldText = SetTextColor (hdcSystemContext, 0x000000);
			HBITMAP		bmMaskOld, maskMap;

			long	x, y;
			long	width  = rect.width ();
			long	height = rect.height ();

			x = rect.x + pContext->offset.x;
			y = rect.y + pContext->offset.y;

			// Create mask-bitmap in memory
			maskMap = CreateBitmap (width, height, 1, 1, NULL);
			bmMaskOld = (HBITMAP)SelectObject (hdcMask, maskMap);

			// Copy bitmap into mask-bitmap and converting it into a black'n'white mask
			SetBkColor (hdcBitmap, color);
			BitBlt (hdcMask, 0, 0, width, height, hdcBitmap, offset.x, offset.y, SRCCOPY);

			// Copy image masked to screen
			BitBlt (hdcSystemContext, x, y, width, height, hdcBitmap, offset.x, offset.y, SRCINVERT);
			BitBlt (hdcSystemContext, x, y, width, height, hdcMask, 0, 0, SRCAND);
			BitBlt (hdcSystemContext, x, y, width, height, hdcBitmap, offset.x, offset.y, SRCINVERT);

			DeleteObject (SelectObject (hdcMask, bmMaskOld));
			DeleteDC (hdcMask);

			SetBkColor (hdcSystemContext, crOldBack);
			SetTextColor (hdcSystemContext, crOldText);
		}
	}
	else
	{
		// OPTIMIZATION: we only do five instead of EIGHT blits
		HDC		hdcSystemContext = (HDC)pContext->getSystemContext ();
		HDC		hdcMask = CreateCompatibleDC (hdcSystemContext);
		HDC		hdcMem = CreateCompatibleDC (hdcSystemContext);
		HBITMAP	bmAndMem;
		HBITMAP	bmMemOld, bmMaskOld;

		long	x, y;
		long	width = rect.width ();
		long	height = rect.height ();

		x = rect.x + pContext->offset.x;
		y = rect.y + pContext->offset.y;

		bmAndMem = CreateCompatibleBitmap(hdcSystemContext, width, height);

		bmMaskOld   = (HBITMAP)SelectObject (hdcMask, pMask);
		bmMemOld    = (HBITMAP)SelectObject (hdcMem, bmAndMem);

		BitBlt (hdcMem, 0, 0, width, height, hdcSystemContext, x, y, SRCCOPY);
		BitBlt (hdcMem, 0, 0, width, height, hdcBitmap, offset.x, offset.y, SRCINVERT);
		BitBlt (hdcMem, 0, 0, width, height, hdcMask, offset.x, offset.y, SRCAND);
		BitBlt (hdcMem, 0, 0, width, height, hdcBitmap, offset.x, offset.y, SRCINVERT);
		BitBlt (hdcSystemContext, x, y, width, height, hdcMem, 0, 0, SRCCOPY);

		DeleteObject (SelectObject (hdcMem, bmMemOld));
		SelectObject (hdcMask, bmMaskOld);

		DeleteDC (hdcMem);
		DeleteDC(hdcMask);
	}
}
#endif
#endif

//-----------------------------------------------------------------------------
#if MAC || BEOS
BEGIN_NAMESPACE_VSTGUI
// return a degre value between [0, 360 * 64[
long convertPoint2Angle (CPoint &pm, CPoint &pt)
{
	long angle;
	if (pt.h == pm.h)
	{
		if (pt.v < pm.v)
			angle = 5760;	// 90 * 64
		else
			angle = 17280; // 270 * 64
	}
	else if (pt.v == pm.v)
	{
		if (pt.h < pm.h)
			angle = 11520;	// 180 * 64
		else
			angle = 0;	
	}
	else
	{
		// 3666.9299 = 180 * 64 / pi
		angle = (long)(3666.9298 * atan ((double)(pm.v - pt.v) / (double)(pt.h - pm.h)));
    
		if (pt.v < pm.v)
		{
			if (pt.h < pm.h)
				angle += 11520; // 180 * 64
		}
		else
		{
			if (pt.h < pm.h)
				angle += 11520; // 180 * 64
			else
				angle += 23040; // 360 * 64
		}
	}
	return angle;
}
END_NAMESPACE_VSTGUI
#endif


#if BEOS
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PlugView::PlugView (BRect frame, CFrame* cframe)
	: BView (frame, NULL, B_FOLLOW_ALL, B_WILL_DRAW), cframe (cframe)
{
	SetViewColor (B_TRANSPARENT_COLOR);
}

//-----------------------------------------------------------------------------
void PlugView::Draw (BRect updateRect)
{
	cframe->draw ();
}

//-----------------------------------------------------------------------------
void PlugView::MouseDown (BPoint where)
{
	BMessage*	m = Window ()->CurrentMessage ();
	int32 buttons;
	m->FindInt32 ("buttons", &buttons);
	
	if (buttons & B_SECONDARY_MOUSE_BUTTON && !Window ()->IsFront () && !Window ()->IsFloating ())
	{
		Window ()->Activate (true);
		return;
	}

	CDrawContext context (cframe, this, NULL);
	CPoint here (where.x, where.y);
	cframe->mouse (&context, here);
}

//-----------------------------------------------------------------------------
void PlugView::MessageReceived (BMessage *msg)
{
	if (msg->what == B_SIMPLE_DATA)
	{
		int32		countMax = 0;	// max number of references. Possibly not all valid...
		type_code	typeFound;
		msg->GetInfo ("refs", &typeFound, &countMax);
		if (countMax > 0)
		{ 
			entry_ref	item;
			int			nbRealItems = 0;
			char **		ptrItems = new char* [countMax];
			for (int k = 0; k < countMax; k++)
				if (msg->FindRef ("refs", k, &item) == B_OK)
				{
					BPath path (&item);
					if (path.InitCheck () == B_OK)
						ptrItems[nbRealItems++] = strdup (path.Path ());
				}
			BPoint bwhere = msg->DropPoint ();
			ConvertFromScreen (&bwhere);
			CPoint where (bwhere.x, bwhere.y);
			cframe->onDrop ((void**)ptrItems, nbRealItems, kDropFiles, where);
			for (long i = 0; i < nbRealItems; i++)
				free (ptrItems[i]);
			delete []ptrItems;
		}
	}
	else BView::MessageReceived (msg);
}

#endif


//-----------------------------------------------------------------------------
#if WINDOWS
//-----------------------------------------------------------------------------
// Drop Implementation
//-----------------------------------------------------------------------------
class CDropTarget : public IDropTarget
{	
public:
	CDropTarget (VSTGUI_CFrame* pFrame);
	virtual ~CDropTarget ();

	// IUnknown
	STDMETHOD (QueryInterface) (REFIID riid, void** object);
	STDMETHOD_ (ULONG, AddRef) (void);
	STDMETHOD_ (ULONG, Release) (void);
   
	// IDropTarget
	STDMETHOD (DragEnter) (IDataObject *dataObject, DWORD keyState, POINTL pt, DWORD *effect);
	STDMETHOD (DragOver) (DWORD keyState, POINTL pt, DWORD *effect);
	STDMETHOD (DragLeave) (void);
	STDMETHOD (Drop) (IDataObject *dataObject, DWORD keyState, POINTL pt, DWORD *effect);
private:
	long refCount;
	bool accept;
	VSTGUI_CFrame* pFrame;
};

//-----------------------------------------------------------------------------
// CDropTarget
//-----------------------------------------------------------------------------
void* createDropTarget (VSTGUI_CFrame* pFrame)
{
	return new CDropTarget (pFrame);
}

//-----------------------------------------------------------------------------
CDropTarget::CDropTarget (VSTGUI_CFrame* pFrame)
: refCount (0), pFrame (pFrame)
{
}

//-----------------------------------------------------------------------------
CDropTarget::~CDropTarget ()
{
}

//-----------------------------------------------------------------------------
STDMETHODIMP CDropTarget::QueryInterface (REFIID riid, void** object)
{
	if (riid == IID_IDropTarget || riid == IID_IUnknown)
	{
		*object = this;
		AddRef ();
      return NOERROR;
	}
	*object = 0;
	return E_NOINTERFACE;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CDropTarget::AddRef (void)
{
	return ++refCount;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CDropTarget::Release (void)
{
	refCount--;
	if (refCount <= 0)
	{
		delete this;
		return 0;
	}
	return refCount;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CDropTarget::DragEnter (IDataObject *dataObject, DWORD keyState, POINTL pt, DWORD *effect)
{
	if (dataObject && pFrame)
	{
		gDragContainer = new CDragContainer (dataObject);
		CDrawContext* context = pFrame->createDrawContext ();
		VSTGUI_CPoint where;
		pFrame->getMouseLocation (context, where);
		pFrame->onDragEnter (gDragContainer, where);
		context->forget ();
		*effect = DROPEFFECT_MOVE;
	}
	else
	*effect = DROPEFFECT_NONE;
	return S_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CDropTarget::DragOver (DWORD keyState, POINTL pt, DWORD *effect)
{
	if (gDragContainer && pFrame)
	{
		CDrawContext* context = pFrame->createDrawContext ();
		VSTGUI_CPoint where;
		pFrame->getMouseLocation (context, where);
		pFrame->onDragMove (gDragContainer, where);
		context->forget ();
		*effect = DROPEFFECT_MOVE;
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CDropTarget::DragLeave (void)
{
	if (gDragContainer && pFrame)
	{
		CDrawContext* context = pFrame->createDrawContext ();
		VSTGUI_CPoint where;
		pFrame->getMouseLocation (context, where);
		pFrame->onDragLeave (gDragContainer, where);
		context->forget ();
		gDragContainer->forget ();
		gDragContainer = 0;
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CDropTarget::Drop (IDataObject *dataObject, DWORD keyState, POINTL pt, DWORD *effect)
{
	if (gDragContainer && pFrame)
	{
		CDrawContext* context = pFrame->createDrawContext ();
		VSTGUI_CPoint where;
		pFrame->getMouseLocation (context, where);
		pFrame->onDrop (gDragContainer, where);
		context->forget ();
		gDragContainer->forget ();
		gDragContainer = 0;
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
bool checkResolveLink (const char* nativePath, char* resolved)
{
	char* ext = strrchr (nativePath, '.');
	if (ext && stricmp (ext, ".lnk") == NULL)
	{
		IShellLink* psl;
		IPersistFile* ppf;
		WIN32_FIND_DATA wfd;
		HRESULT hres;
		WORD wsz[2048];
		
		// Get a pointer to the IShellLink interface.
		hres = CoCreateInstance (CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
			IID_IShellLink, (void**)&psl);
		if (SUCCEEDED (hres))
		{
			// Get a pointer to the IPersistFile interface.
			hres = psl->QueryInterface (IID_IPersistFile, (void**)&ppf);
			if (SUCCEEDED (hres))
			{
				// Ensure string is Unicode.
				MultiByteToWideChar (CP_ACP, 0, nativePath, -1, (LPWSTR)wsz, 2048);
				// Load the shell link.
				hres = ppf->Load ((LPWSTR)wsz, STGM_READ);
				if (SUCCEEDED (hres))
				{					
					hres = psl->Resolve (0, MAKELONG (SLR_ANY_MATCH | SLR_NO_UI, 500));
					if (SUCCEEDED (hres))
					{
						// Get the path to the link target.
						hres = psl->GetPath (resolved, 2048, &wfd, SLGP_SHORTPATH);
					}
				}
				// Release pointer to IPersistFile interface.
				ppf->Release ();
			}
			// Release pointer to IShellLink interface.
			psl->Release ();
		}
		return SUCCEEDED(hres);
	}
	return false;	
}

#elif MAC
#if !MACX
#include "Drag.h"
#endif

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
static CPoint GetMacDragMouse (CFrame* frame)
{
	WindowRef window = (WindowRef)frame->getSystemWindow ();
	HIViewRef view = (HIViewRef)frame->getPlatformControl ();
	CPoint where;
	Point r;
	if (GetDragMouse ((DragRef)gDragContainer->getPlatformDrag (), NULL, &r) == noErr)
	{
		HIPoint location;
		#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
		if (HIPointConvert)
		{
			location = CGPointMake (r.h, r.v);
			HIPointConvert (&location, kHICoordSpaceScreenPixel, NULL, kHICoordSpaceView, view);
		}
		else
		#endif
		{
			QDGlobalToLocalPoint (GetWindowPort (window), &r);
			location = CGPointMake (r.h, r.v);
			HIViewRef fromView = NULL;
			HIViewFindByID (HIViewGetRoot (window), kHIViewWindowContentID, &fromView);
			HIViewConvertPoint (&location, fromView, view);
		}
		where.x = (CCoord)location.x;
		where.y = (CCoord)location.y;
	}
	return where;
}

#if MAC_OLD_DRAG
//-----------------------------------------------------------------------------
// Drop Implementation
//-----------------------------------------------------------------------------
pascal static short drag_receiver (WindowPtr w, void* ref, DragReference drag);
pascal static OSErr drag_tracker (DragTrackingMessage message, WindowRef theWindow, void *handlerRefCon, DragRef theDrag);

static DragReceiveHandlerUPP drh;
static DragTrackingHandlerUPP dth;

static bool gEventDragWorks = false;

//-------------------------------------------------------------------------------------------
void install_drop (CFrame *frame)
{
	drh = NewDragReceiveHandlerUPP (drag_receiver);
	dth = NewDragTrackingHandlerUPP (drag_tracker);
#if TARGET_API_MAC_CARBON
	InstallReceiveHandler (drh, (WindowRef)(frame->getSystemWindow ()), (void*)frame);
	InstallTrackingHandler (dth, (WindowRef)(frame->getSystemWindow ()), (void*)frame);
#else
	InstallReceiveHandler (drh, (GrafPort*)(frame->getSystemWindow ()), (void*)frame);
	InstallTrackingHandler (dth, (GrafPort*)(frame->getSystemWindow ()), (void*)frame);
#endif
}

//-------------------------------------------------------------------------------------------
void remove_drop (CFrame *frame)
{
#if TARGET_API_MAC_CARBON
	RemoveReceiveHandler (drh, (WindowRef)(frame->getSystemWindow ()));
	RemoveTrackingHandler (dth, (WindowRef)(frame->getSystemWindow ()));
#else
	RemoveReceiveHandler (drh, (GrafPort*)(frame->getSystemWindow ()));
	RemoveTrackingHandler (dth, (GrafPort*)(frame->getSystemWindow ()));
#endif
	DisposeDragReceiveHandlerUPP (drh);
	DisposeDragTrackingHandlerUPP (dth);
}

// drag tracking for visual feedback
pascal OSErr drag_tracker (DragTrackingMessage message, WindowRef theWindow, void *handlerRefCon, DragRef dragRef)
{
	#if QUARTZ
	if (gEventDragWorks)
		return noErr;
	#endif

	CFrame* frame = (CFrame*)handlerRefCon;
	switch (message)
	{
		case kDragTrackingEnterWindow:
		{
			if (gDragContainer)
				gDragContainer->forget ();
			gDragContainer = new CDragContainer (dragRef);

			VSTGUI_CPoint where = GetMacDragMouse (frame);
			frame->setCursor (kCursorNotAllowed);
			frame->onDragEnter (gDragContainer, where);
			break;
		}
		case kDragTrackingLeaveWindow:
		{
			VSTGUI_CPoint where = GetMacDragMouse (frame);
			frame->onDragLeave (gDragContainer, where);
			frame->setCursor (kCursorDefault);
			gDragContainer->forget ();
			gDragContainer = NULL;
			break;
		}
		case kDragTrackingInWindow:
		{
			VSTGUI_CPoint where = GetMacDragMouse (frame);
			frame->onDragMove (gDragContainer, where);

			break;
		}
	}
	return noErr;
}

//-------------------------------------------------------------------------------------------
// Drop has happened in one of our's windows.
// The data is either of our own type (flavour type stCA), or comes from
// another app. The only data from outside that is currently accepted are
// HFS-files
//-------------------------------------------------------------------------------------------
pascal short drag_receiver (WindowPtr w, void* ref, DragReference drag)
{
	#if QUARTZ
	if (gEventDragWorks)
		return noErr;
	#endif

	if (!gDragContainer)
		return noErr;
	
	CFrame* frame = (CFrame*) ref;
	
	VSTGUI_CPoint where = GetMacDragMouse (frame);
	frame->onDrop (gDragContainer, where);
	frame->setCursor (kCursorDefault);

	gDragContainer->forget ();
	gDragContainer = NULL;
	return noErr;
}
#endif // MAC_OLD_DRAG

#if QUARTZ
#define defControlStringMask	CFSTR ("net.sourceforge.vstgui.%d")

bool CFrame::registerWithToolbox ()
{
	CFStringRef defControlString = CFStringCreateWithFormat (NULL, NULL, defControlStringMask, this);

	controlSpec.defType = kControlDefObjectClass;
	controlSpec.u.classRef = NULL;

	EventTypeSpec eventTypes[] = {	{kEventClassControl, kEventControlDraw},
									{kEventClassControl, kEventControlHitTest},
									{kEventClassControl, kEventControlClick},
									//{kEventClassControl, kEventControlTrack},
									//{kEventClassControl, kEventControlContextualMenuClick},
									{kEventClassKeyboard, kEventRawKeyDown},
									{kEventClassKeyboard, kEventRawKeyRepeat},
									{kEventClassMouse, kEventMouseWheelMoved},
									{kEventClassControl, kEventControlDragEnter},
									{kEventClassControl, kEventControlDragWithin},
									{kEventClassControl, kEventControlDragLeave},
									{kEventClassControl, kEventControlDragReceive},
									{kEventClassControl, kEventControlInitialize},
									{kEventClassControl, kEventControlGetClickActivation},
									{kEventClassControl, kEventControlGetOptimalBounds},
									{kEventClassScrollable, kEventScrollableGetInfo},
									{kEventClassScrollable, kEventScrollableScrollTo},
									{kEventClassControl, kEventControlSetFocusPart},
									{kEventClassControl, kEventControlGetFocusPart},
								};

	ToolboxObjectClassRef controlClass = NULL;

	OSStatus status = RegisterToolboxObjectClass (	defControlString,
													NULL,
													GetEventTypeCount (eventTypes),
													eventTypes,
													CFrame::carbonEventHandler,
													this,
													&controlClass);
	if (status == noErr)
		controlSpec.u.classRef = controlClass;

	CFRelease (defControlString);

	return (controlSpec.u.classRef != NULL);
}
//------------------------------------------------------------------------------
static short keyTable[] = {
	VKEY_BACK,		0x33, 
	VKEY_TAB,		0x30, 
	VKEY_RETURN,	0x24, 
	VKEY_PAUSE,		0x71, 
	VKEY_ESCAPE,	0x35, 
	VKEY_SPACE,		0x31, 

	VKEY_END,		0x77, 
	VKEY_HOME,		0x73, 

	VKEY_LEFT,		0x7B, 
	VKEY_UP,		0x7E, 
	VKEY_RIGHT,		0x7C, 
	VKEY_DOWN,		0x7D, 
	VKEY_PAGEUP,	0x74, 
	VKEY_PAGEDOWN,	0x79, 

	VKEY_PRINT,		0x69, 			
	VKEY_ENTER,		0x4C, 
	VKEY_HELP,		0x72, 
	VKEY_DELETE,	0x75, 
	VKEY_NUMPAD0,	0x52, 
	VKEY_NUMPAD1,	0x53, 
	VKEY_NUMPAD2,	0x54, 
	VKEY_NUMPAD3,	0x55, 
	VKEY_NUMPAD4,	0x56, 
	VKEY_NUMPAD5,	0x57, 
	VKEY_NUMPAD6,	0x58, 
	VKEY_NUMPAD7,	0x59, 
	VKEY_NUMPAD8,	0x5B, 
	VKEY_NUMPAD9,	0x5C, 
	VKEY_MULTIPLY,	0x43, 
	VKEY_ADD,		0x45, 
	VKEY_SUBTRACT,	0x4E, 
	VKEY_DECIMAL,	0x41, 
	VKEY_DIVIDE,	0x4B, 
	VKEY_F1,		0x7A, 
	VKEY_F2,		0x78, 
	VKEY_F3,		0x63, 
	VKEY_F4,		0x76, 
	VKEY_F5,		0x60, 
	VKEY_F6,		0x61, 
	VKEY_F7,		0x62, 
	VKEY_F8,		0x64, 
	VKEY_F9,		0x65, 
	VKEY_F10,		0x6D, 
	VKEY_F11,		0x67, 
	VKEY_F12,		0x6F, 
	VKEY_NUMLOCK,	0x47, 
	VKEY_EQUALS,	0x51
};

#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
class VSTGUIDrawRectsHelper
{
public:
	VSTGUIDrawRectsHelper (CFrame* inFrame, CDrawContext* inContext, bool inIsComposited) : frame (inFrame), context (inContext), isComposited (inIsComposited) {}
	
	CFrame* frame;
	CDrawContext* context;
	bool isComposited;
};

static OSStatus VSTGUIDrawRectsProc (UInt16 message, RgnHandle rgn, const Rect *rect, void *refCon)
{
	if (message == kQDRegionToRectsMsgParse)
	{
		VSTGUIDrawRectsHelper* h = (VSTGUIDrawRectsHelper*)refCon;
		CRect r;
		Rect2CRect ((Rect&)*rect, r);
		if (!h->isComposited)
			r.offset (-h->context->offsetScreen.x, -h->context->offsetScreen.y);
		h->frame->drawRect (h->context, r);
	}
	return noErr;
}
#endif

#ifndef kHIViewFeatureGetsFocusOnClick
#define   kHIViewFeatureGetsFocusOnClick (1 << 8)
#endif

bool hiToolboxAllowFocusChange = true;

//---------------------------------------------------------------------------------------
pascal OSStatus CFrame::carbonEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus result = eventNotHandledErr;
	CFrame* frame = (CFrame*)inUserData;
	UInt32 eventClass = GetEventClass (inEvent);
	UInt32 eventKind = GetEventKind (inEvent);
	WindowRef window = (WindowRef)frame->getSystemWindow ();

	// WARNING :
	// I've not implemented the old style resource file handling.
	// Use the CFBundleCopyResourceURL... functions to get your resources.

	switch (eventClass)
	{
		case kEventClassScrollable:
		{
			switch (eventKind)
			{
				case kEventScrollableGetInfo:
				{
					HISize cs = {frame->getWidth (), frame->getHeight ()};
					SetEventParameter (inEvent, kEventParamImageSize, typeHISize, sizeof (HISize), &cs);
					HIPoint origin = {frame->hiScrollOffset.x, frame->hiScrollOffset.y};
					SetEventParameter (inEvent, kEventParamOrigin, typeHIPoint, sizeof (HIPoint), &origin);
					HISize lineSize = {50.0, 20.0};
					SetEventParameter(inEvent, kEventParamLineSize, typeHISize, sizeof(lineSize), &lineSize);
					HIRect bounds;
					HIViewGetBounds ((HIViewRef)frame->controlRef, &bounds);
					SetEventParameter(inEvent, kEventParamViewSize, typeHISize, sizeof(bounds.size), &bounds.size);
					result = noErr;
					break;
				}
				case kEventScrollableScrollTo:
				{
					HIPoint where;
					GetEventParameter(inEvent, kEventParamOrigin, typeHIPoint, NULL, sizeof(where), NULL, &where);
					frame->hiScrollOffset.x = (CCoord)where.x;
					frame->hiScrollOffset.y = (CCoord)where.y;
					HIViewSetBoundsOrigin((HIViewRef)frame->controlRef, where.x, where.y);
					HIViewSetNeedsDisplay((HIViewRef)frame->controlRef, true);
					result = noErr;
					break;
				}
			}
			break;
		}
		case kEventClassControl:
		{
			switch (eventKind)
			{
				case kEventControlInitialize:
				{
					UInt32 controlFeatures = kControlSupportsDragAndDrop | kControlSupportsFocus | kControlHandlesTracking | kControlSupportsEmbedding | kHIViewFeatureGetsFocusOnClick | kHIViewIsOpaque;
					SetEventParameter (inEvent, kEventParamControlFeatures, typeUInt32, sizeof (UInt32), &controlFeatures);
					result = noErr;
					break;
				}
				case kEventControlDraw:
				{
					CDrawContext* context = 0;
					if (frame->pFrameContext)
					{
						context = frame->pFrameContext;
						context->remember ();
					}
					else
					{
						CGContextRef cgcontext = 0;
						OSStatus res = GetEventParameter (inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof (cgcontext), NULL, &cgcontext);
						context = new CDrawContext (frame, (res == noErr) ? cgcontext : NULL, window);
					}
					RgnHandle dirtyRegion;
					if (GetEventParameter (inEvent, kEventParamRgnHandle, typeQDRgnHandle, NULL, sizeof (RgnHandle), NULL, &dirtyRegion) == noErr)
					{
						#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
						VSTGUIDrawRectsHelper helper (frame, context, isWindowComposited (window));
						RegionToRectsUPP upp = NewRegionToRectsUPP (VSTGUIDrawRectsProc);
						QDRegionToRects (dirtyRegion, kQDParseRegionFromTopLeft, upp, &helper);
						DisposeRegionToRectsUPP (upp);
						#else
						bool frameWasDirty = frame->bDirty;
						Rect bounds;
						GetRegionBounds (dirtyRegion, &bounds);
						CRect updateRect;
						Rect2CRect (bounds, updateRect);
						if (!isWindowComposited (window))
							updateRect.offset (-context->offsetScreen.x, -context->offsetScreen.y);
						frame->drawRect (context, updateRect);
						if (frameWasDirty && updateRect != frame->size)
							frame->setDirty (true);
						#endif
					}
					else
						frame->draw (context);
					context->forget ();
					result = noErr;
					break;
				}
				case kEventControlGetClickActivation:
				{
					ClickActivationResult activation = kActivateAndHandleClick;
					SetEventParameter (inEvent, kEventParamClickActivation, typeClickActivationResult, sizeof (ClickActivationResult), &activation);
					result = noErr;
					break;
				}
				case kEventControlHitTest:
				{
					ControlPartCode code = kControlContentMetaPart;
					SetEventParameter (inEvent, kEventParamControlPart, typeControlPartCode, sizeof (ControlPartCode), &code);
					result = noErr;
					break;
				}
				case kEventControlClick:
				{
					return noErr;
					EventMouseButton buttonState;
					GetEventParameter (inEvent, kEventParamMouseButton, typeMouseButton, NULL, sizeof (EventMouseButton), NULL, &buttonState);
					if (buttonState == kEventMouseButtonPrimary)
					{
						result = CallNextEventHandler (inHandlerCallRef, inEvent);
						break;
					}
				}
				case kEventControlTrack:
				case kEventControlContextualMenuClick:
				{
					#if VSTGUI_ENABLE_DEPRECATED_METHODS
					long buttons = 0;
					EventMouseButton buttonState;
					HIPoint hipoint;
					UInt32 modifiers;
					GetEventParameter (inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof (HIPoint), NULL, &hipoint);
					if (eventKind == kEventControlContextualMenuClick)
						buttons = kRButton;
					else if (eventKind == kEventControlTrack)
					{
						buttons = kLButton;
						GetEventParameter (inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof (UInt32), NULL, &modifiers);
						if (modifiers & cmdKey)
							buttons |= kControl;
						if (modifiers & shiftKey)
							buttons |= kShift;
						if (modifiers & optionKey)
							buttons |= kAlt;
						if (modifiers & controlKey)
							buttons |= kApple;
					}
					else
					{
						GetEventParameter (inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof (UInt32), NULL, &modifiers);
						GetEventParameter (inEvent, kEventParamMouseButton, typeMouseButton, NULL, sizeof (EventMouseButton), NULL, &buttonState);
						if (buttonState == kEventMouseButtonPrimary)
							buttons |= kLButton;
						if (buttonState == kEventMouseButtonSecondary)
							buttons |= kRButton;
						if (buttonState == kEventMouseButtonTertiary)
							buttons |= kMButton;
						if (modifiers & cmdKey)
							buttons |= kControl;
						if (modifiers & shiftKey)
							buttons |= kShift;
						if (modifiers & optionKey)
							buttons |= kAlt;
						if (modifiers & controlKey)
							buttons |= kApple;
					}
					//SetUserFocusWindow (window);
					//AdvanceKeyboardFocus (window);
					//SetKeyboardFocus (window, frame->controlRef, kControlFocusNextPart);
					bool windowIsComposited = isWindowComposited (window);
					Point point = {(short)hipoint.y, (short)hipoint.x};
					if (eventKind == kEventControlClick && !windowIsComposited)
						QDGlobalToLocalPoint (GetWindowPort (window), &point);
					CDrawContext* context = frame->createDrawContext ();
					CPoint p (point.h, point.v);
					if (!windowIsComposited)
						p.offset (-context->offsetScreen.x, -context->offsetScreen.y);
					frame->mouse (context, p, buttons);
					context->forget ();
					result = noErr;
					#endif
					break;
				}
				case kEventControlGetOptimalBounds:
				{
					HIRect optimalBounds = { {0, 0}, { frame->getWidth (), frame->getHeight ()}};
					SetEventParameter (inEvent, kEventParamControlOptimalBounds, typeHIRect, sizeof (HIRect), &optimalBounds);
					result = noErr;
					break;
				}
				case kEventControlGetFocusPart:
				{
					if (hiToolboxAllowFocusChange)
					{
						ControlPartCode code = frame->hasFocus ? 127 : kControlFocusNoPart;
						SetEventParameter (inEvent, kEventParamControlPart, typeControlPartCode, sizeof (ControlPartCode), &code);
						result = noErr;
					}
					break;
				}
				case kEventControlSetFocusPart:
				{
					if (hiToolboxAllowFocusChange)
					{
						ControlPartCode code;
						GetEventParameter (inEvent, kEventParamControlPart, typeControlPartCode, NULL, sizeof (ControlPartCode), NULL, &code);
						if (code == kControlFocusNoPart)
						{
							frame->hasFocus = false;
							frame->setFocusView (NULL);
						}
						else
						{
							bool anfResult = false;
							if (code == kControlFocusNextPart)
								anfResult = frame->advanceNextFocusView (frame->pFocusView);
							else if (code == kControlFocusPrevPart)
								anfResult = frame->advanceNextFocusView (frame->pFocusView, true);
							if (anfResult)
							{
								frame->hasFocus = true;
								code = 127;
							}
							else
							{
								frame->hasFocus = false;
								code = kControlFocusNoPart;
							}
						}
						SetEventParameter (inEvent, kEventParamControlPart, typeControlPartCode, sizeof (code), &code);
						result = noErr;
					}
					break;
				}
				case kEventControlDragEnter:
				{
					#if MAC_OLD_DRAG
					gEventDragWorks = true;
					#endif

					DragRef dragRef;
					if (GetEventParameter (inEvent, kEventParamDragRef, typeDragRef, NULL, sizeof (DragRef), NULL, &dragRef) == noErr)
					{
						gDragContainer = new CDragContainer (dragRef);
						
						VSTGUI_CPoint where = GetMacDragMouse (frame);
						frame->setCursor (kCursorNotAllowed);
						frame->onDragEnter (gDragContainer, where);

						Boolean acceptDrop = true;
						SetEventParameter (inEvent, kEventParamControlWouldAcceptDrop, typeBoolean, sizeof (Boolean), &acceptDrop);
					}
					result = noErr;
					break;
				}
				case kEventControlDragWithin:
				{
					if (gDragContainer)
					{
						VSTGUI_CPoint where = GetMacDragMouse (frame);
						frame->onDragMove (gDragContainer, where);
					}
					result = noErr;
					break;
				}
				case kEventControlDragLeave:
				{
					if (gDragContainer)
					{
						VSTGUI_CPoint where = GetMacDragMouse (frame);
						frame->onDragLeave (gDragContainer, where);
						frame->setCursor (kCursorDefault);
					}
					result = noErr;
					break;
				}
				case kEventControlDragReceive:
				{
					if (gDragContainer)
					{
						VSTGUI_CPoint where = GetMacDragMouse (frame);
						frame->onDrop (gDragContainer, where);
						frame->setCursor (kCursorDefault);
						gDragContainer->forget ();
						gDragContainer = 0;
					}
					result = noErr;
					break;
				}
			}
			break;
		}
		case kEventClassMouse:
		{
			switch (eventKind)
			{
				case kEventMouseWheelMoved:
				{
					UInt32 modifiers;
					HIPoint windowHIPoint;
					SInt32 wheelDelta;
					EventMouseWheelAxis wheelAxis;
					WindowRef windowRef;
					GetEventParameter (inEvent, kEventParamWindowRef, typeWindowRef, NULL, sizeof (WindowRef), NULL, &windowRef);
					GetEventParameter (inEvent, kEventParamMouseWheelAxis, typeMouseWheelAxis, NULL, sizeof (EventMouseWheelAxis), NULL, &wheelAxis);
					GetEventParameter (inEvent, kEventParamMouseWheelDelta, typeLongInteger, NULL, sizeof (SInt32), NULL, &wheelDelta);
					GetEventParameter (inEvent, kEventParamWindowMouseLocation, typeHIPoint, NULL, sizeof (HIPoint), NULL, &windowHIPoint);
					GetEventParameter (inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof (UInt32), NULL, &modifiers);
					long buttons = 0;
					if (modifiers & cmdKey)
						buttons |= kControl;
					if (modifiers & shiftKey)
						buttons |= kShift;
					if (modifiers & optionKey)
						buttons |= kAlt;
					if (modifiers & controlKey)
						buttons |= kApple;
					
					#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
					if (HIPointConvert)
						HIPointConvert (&windowHIPoint, kHICoordSpaceWindow, windowRef, kHICoordSpaceView, frame->controlRef);
					else
					#endif
					HIViewConvertPoint (&windowHIPoint, HIViewGetRoot (windowRef), frame->controlRef);
					
					// non-compositing window controls need to handle offset themselves
					if (!isWindowComposited (windowRef))
					{
						HIRect viewRect;
						HIViewGetFrame(frame->controlRef, &viewRect);
						windowHIPoint.x -= viewRect.origin.x;
						windowHIPoint.y -= viewRect.origin.y;
					}
					
					CPoint p ((CCoord)windowHIPoint.x, (CCoord)windowHIPoint.y);
					float distance = wheelDelta;
					CMouseWheelAxis axis = kMouseWheelAxisY;
					if (wheelAxis == kEventMouseWheelAxisX)
						axis = kMouseWheelAxisX;
					frame->onWheel (p, axis, distance, buttons); // todo check modifier
					result = noErr;
					break;
				}
			}
			break;
		}
		case kEventClassTextInput:
		{
			switch (eventKind)
			{
				case kEventTextInputUnicodeForKeyEvent:
				{
					// The "Standard Event Handler" of a window would return noErr even though no one has handled the key event. 
					// This prevents the "Standard Handler" to be called for this event, with the exception of the tab key as it is used for control focus changes.
					EventRef rawKeyEvent;
					GetEventParameter (inEvent, kEventParamTextInputSendKeyboardEvent, typeEventRef, NULL, sizeof (EventRef), NULL, &rawKeyEvent);
					if (rawKeyEvent)
					{
						UInt32 keyCode = 0;
						GetEventParameter (rawKeyEvent, kEventParamKeyCode, typeUInt32, NULL, sizeof (UInt32), NULL, &keyCode);
						if (keyCode == (UInt32)keyTable[VKEY_TAB+1])
							return result;
					}
					result = eventPassToNextTargetErr;
					break;
				}
			}
			break;
		}
		case kEventClassKeyboard:
		{
			if (frame->hasFocus)
			{
				switch (eventKind)
				{
					case kEventRawKeyDown:
					case kEventRawKeyRepeat:
					{
						// todo: make this work

						char character = 0;
						UInt32 keyCode = 0;
						UInt32 modifiers = 0;
						GetEventParameter (inEvent, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof (char), NULL, &character);
						GetEventParameter (inEvent, kEventParamKeyCode, typeUInt32, NULL, sizeof (UInt32), NULL, &keyCode);
						GetEventParameter (inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof (UInt32), NULL, &modifiers);
						char scanCode = keyCode;
						VstKeyCode vstKeyCode;
						memset (&vstKeyCode, 0, sizeof (VstKeyCode));
						KeyboardLayoutRef layout;
						if (KLGetCurrentKeyboardLayout (&layout) == noErr)
						{
							const void* pKCHR = 0;
							KLGetKeyboardLayoutProperty (layout, kKLKCHRData, &pKCHR);
							if (pKCHR)
							{
								static UInt32 keyTranslateState = 0;
								vstKeyCode.character = KeyTranslate (pKCHR, keyCode, &keyTranslateState);
								if (modifiers & shiftKey)
								{
									vstKeyCode.character = toupper (vstKeyCode.character);
								}
							}
						}
						short entries = sizeof (keyTable) / (sizeof (short));
						for (int i = 0; i < entries; i += 2)
						{
							if (keyTable[i + 1] == scanCode)
							{
								vstKeyCode.virt = keyTable[i];
								vstKeyCode.character = 0;
								break;
							}
						}
						if (modifiers & cmdKey)
							vstKeyCode.modifier |= MODIFIER_CONTROL;
						if (modifiers & shiftKey)
							vstKeyCode.modifier |= MODIFIER_SHIFT;
						if (modifiers & optionKey)
							vstKeyCode.modifier |= MODIFIER_ALTERNATE;
						if (modifiers & controlKey)
							vstKeyCode.modifier |= MODIFIER_COMMAND;
						if (frame->onKeyDown (vstKeyCode) != -1)
							result = noErr;
						
						break;
					}
				}
			}
			break;
		}
	}
	return result;
}

#if ENABLE_LOGGING
#define LOG_HIPOINT(text,point) fprintf (stdout, "%s%d, %d\n", text, (long)point.x, (long)point.y);
#define LOG(text) fprintf (stdout, "%s\n", text);
#else
#define LOG_HIPOINT(x,y)
#define LOG(x)
#endif

//---------------------------------------------------------------------------------------
pascal OSStatus CFrame::carbonMouseEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus result = eventNotHandledErr;
	CFrame* frame = (CFrame*)inUserData;
	UInt32 eventClass = GetEventClass (inEvent);
	UInt32 eventKind = GetEventKind (inEvent);
	WindowRef window = (WindowRef)frame->getSystemWindow ();
	HIViewRef hiView = frame->controlRef;

	HIViewRef view;
	if (HIViewGetViewForMouseEvent (HIViewGetRoot (window), inEvent, &view) == noErr)
	{
		if (view != hiView && !((eventKind == kEventMouseDragged || eventKind == kEventMouseUp) && frame->mouseDownView != 0))
			return result;
	}
	switch (eventClass)
	{
		case kEventClassMouse:
		{
			UInt32 modifiers = 0;
			EventMouseButton buttonState = 0;
			long buttons = 0;
			HIPoint location = { 0.f, 0.f };
			if (GetEventParameter (inEvent, kEventParamWindowMouseLocation, typeHIPoint, NULL, sizeof (HIPoint), NULL, &location) == noErr)
			{
				LOG_HIPOINT("window :",location)
				#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
				if (HIPointConvert)
					HIPointConvert (&location, kHICoordSpaceWindow, window, kHICoordSpaceView, hiView);
				else
				#endif
					HIViewConvertPoint (&location, HIViewGetRoot (window), hiView);
				LOG_HIPOINT("view   :",location)
			}
			if (!isWindowComposited ((WindowRef)window))
			{
				HIRect viewRect;
				HIViewGetFrame(hiView, &viewRect);
				location.x -= viewRect.origin.x;
				location.y -= viewRect.origin.y;
			}
			GetEventParameter (inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof (UInt32), NULL, &modifiers);
			GetEventParameter (inEvent, kEventParamMouseButton, typeMouseButton, NULL, sizeof (EventMouseButton), NULL, &buttonState);
			if (buttonState == kEventMouseButtonPrimary)
				buttons |= kLButton;
			if (buttonState == kEventMouseButtonSecondary)
				buttons |= kRButton;
			if (buttonState == kEventMouseButtonTertiary)
				buttons |= kMButton;
			if (buttonState == 4)
				buttons |= kButton4;
			if (buttonState == 5)
				buttons |= kButton5;
			if (modifiers & cmdKey)
				buttons |= kControl;
			if (modifiers & shiftKey)
				buttons |= kShift;
			if (modifiers & optionKey)
				buttons |= kAlt;
			if (modifiers & controlKey)
				buttons |= kApple;
			CPoint point ((CCoord)location.x, (CCoord)location.y);
			switch (eventKind)
			{
				case kEventMouseDown:
				{
					LOG("Mouse Down")
					result = CallNextEventHandler (inHandlerCallRef, inEvent); // calls default handler, which activates the window if not already active, or sets the process to front
					if (frame->onMouseDown (point, buttons))
						result = noErr;
					break;
				}
				case kEventMouseUp:
				{
					LOG("Mouse Up")
					if (frame->onMouseUp (point, buttons))
						result = noErr;
					break;
				}
				case kEventMouseDragged:
				{
					LOG("Mouse Dragged")
					if (frame->onMouseMoved (point, buttons))
						result = noErr;
					break;
				}
				case kEventMouseMoved:
				{
					LOG("Mouse Moved")
					if (IsWindowActive (window))
					{
						if (frame->onMouseMoved (point, buttons))
							result = noErr;
					}
					break;
				}
			}
			break;
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
bool isWindowComposited (WindowRef window)
{
	WindowAttributes attr;
	GetWindowAttributes (window, &attr);
	if (attr & kWindowCompositingAttribute)
		return true;
	return false;
}

// code from CarbonSketch Example Code
#define	kGenericRGBProfilePathStr       "/System/Library/ColorSync/Profiles/Generic RGB Profile.icc"

class QuartzStatics
{
public:
	//-----------------------------------------------------------------------------
	QuartzStatics ()
	: genericRGBColorSpace (0)
	{
		CreateGenericRGBColorSpace ();
        CFBundleRef coregraphicsBundle = CFBundleGetBundleWithIdentifier (CFSTR("com.apple.CoreGraphics"));
        if (coregraphicsBundle)
        {
            _CGImageCreateWithImageInRect = (CGImageCreateWithImageInRectProc)CFBundleGetFunctionPointerForName (coregraphicsBundle, CFSTR("CGImageCreateWithImageInRect"));
            _CGContextStrokeLineSegments = (CGContextStrokeLineSegmentsProc)CFBundleGetFunctionPointerForName (coregraphicsBundle, CFSTR("CGContextStrokeLineSegments"));
        }
	}

	//-----------------------------------------------------------------------------
	~QuartzStatics ()
	{
		// we don't want to leak ;-)
		CGColorSpaceRelease (genericRGBColorSpace);

		if (bmpGI)
			CloseComponent (bmpGI);
		if (pngGI)
			CloseComponent (pngGI);
		if (jpgGI)
			CloseComponent (jpgGI);
		if (pictGI)
			CloseComponent (pictGI);
		bmpGI = 0;
		pngGI = 0;
		jpgGI = 0;
		pictGI = 0;
	}
	
	inline CGColorSpaceRef getGenericRGBColorSpace () { return genericRGBColorSpace; }

protected:
	//-----------------------------------------------------------------------------
	CMProfileRef OpenGenericProfile(void)
	{
		CMProfileLocation 	loc;
		CMProfileRef cmProfile;
			
		loc.locType = cmPathBasedProfile;
		strcpy(loc.u.pathLoc.path, kGenericRGBProfilePathStr);
	
		if (CMOpenProfile(&cmProfile, &loc) != noErr)
			cmProfile = NULL;
		
	    return cmProfile;
	}

	//-----------------------------------------------------------------------------
	void CreateGenericRGBColorSpace(void)
	{
		CMProfileRef genericRGBProfile = OpenGenericProfile();
	
		if (genericRGBProfile)
		{
			genericRGBColorSpace = CGColorSpaceCreateWithPlatformColorSpace(genericRGBProfile);
			
			// we opened the profile so it is up to us to close it
			CMCloseProfile(genericRGBProfile); 
		}
		if (genericRGBColorSpace == NULL)
			genericRGBColorSpace = CGColorSpaceCreateDeviceRGB ();
	}

	CGColorSpaceRef genericRGBColorSpace;
};

static QuartzStatics _gQuartzStatics;

inline CGColorSpaceRef GetGenericRGBColorSpace ()
{
	return _gQuartzStatics.getGenericRGBColorSpace ();
}

END_NAMESPACE_VSTGUI
#endif

#endif

