//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 2.2       Date : 14/05/03
//
// First version            : Wolfgang Kundrus         06.97
// Added Motif/Windows vers.: Yvan Grabit              01.98
// Added Mac version        : Charlie Steinberg        02.98
// Added BeOS version       : Georges-Edouard Berenger 05.99
// Added new functions      : Matthias Juwan           12.01
// Added MacOSX version     : Arne Scheffler           02.03
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// © 2003, Steinberg Media Technologies, All Rights Reserved
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

#if !PLUGGUI
#ifndef __audioeffectx__
#include "audioeffectx.h"
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#if USE_NAMESPACE
#define VSTGUI_CFrame     VSTGUI::CFrame
#define VSTGUI_CPoint     VSTGUI::CPoint
#define VSTGUI_kDropFiles VSTGUI::kDropFiles
#define VSTGUI_kDropText  VSTGUI::kDropText
#define VSTGUI_CTextEdit  VSTGUI::CTextEdit
#define VSTGUI_CColor     VSTGUI::CColor
#define VSTGUI_CDrawContext VSTGUI::CDrawContext
#define VSTGUI_COptionMenu VSTGUI::COptionMenu
#define VSTGUI_COptionMenuScheme VSTGUI::COptionMenuScheme
#else
#define VSTGUI_CFrame     CFrame
#define VSTGUI_CPoint     CPoint
#define VSTGUI_kDropFiles kDropFiles
#define VSTGUI_kDropText  kDropText
#define VSTGUI_CTextEdit  CTextEdit
#define VSTGUI_CColor     CColor
#define VSTGUI_CDrawContext CDrawContext
#define VSTGUI_COptionMenu COptionMenu
#define VSTGUI_COptionMenuScheme COptionMenuScheme
#endif

#if !WINDOWS
#define USE_GLOBAL_CONTEXT 1
#endif

#if DEBUG //---For Debugging------------------------

long gNbCBitmap = 0;
long gNbCView = 0;
long gNbCDrawContext = 0;
long gNbCOffscreenContext = 0;
long gBitmapAllocation = 0;
long gNbDC = 0;

#include <stdarg.h>

void FDebugPrint (char *format, ...);
void FDebugPrint (char *format, ...)
{
	char string[300];
	va_list marker;
	va_start (marker, format);
	vsprintf (string, format, marker);
	if (!string)
		strcpy (string, "Empty string\n");
	#if MAC
//	printf (string);
	#elif WINDOWS
	OutputDebugString (string);
	#else
	fprintf (stderr, string);
	#endif
}
#endif //---For Debugging------------------------

#if WINDOWS

static bool swapped_mouse_buttons = false; 

#define DYNAMICALPHABLEND   1

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>

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

#if PLUGGUI
	extern HINSTANCE ghInst;
	inline HINSTANCE GetInstance () { return ghInst; }
#else
	extern void* hInstance;
	inline HINSTANCE GetInstance () { return (HINSTANCE)hInstance; }
#endif

long   useCount = 0;
char   className[20];
bool   InitWindowClass ();
void   ExitWindowClass ();
LONG WINAPI WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

static HANDLE CreateMaskBitmap (CDrawContext* pContext, CRect& rect, CColor transparentColor);
static void   DrawTransparent (CDrawContext* pContext, CRect& rect, const CPoint& offset, HDC hdcBitmap, POINT ptSize, HBITMAP pMask, COLORREF color);
static bool   checkResolveLink (const char* nativePath, char* resolved);
static void   *createDropTarget (VSTGUI_CFrame* pFrame);

BEGIN_NAMESPACE_VSTGUI
long        standardFontSize[] = { 12, 18, 14, 12, 11, 10, 9, 13 };
const char* standardFontName[] = {
	"Arial", "Arial", "Arial", 
	"Arial", "Arial", "Arial", 
	"Arial", "Symbol" };
END_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#elif MOTIF

 #define USE_XPM 0
 #define TEST_REGION 0

 #if USE_XPM
  #include <X11/xpm.h>
 #endif

 #include <X11/Xlib.h>
 #include <Xm/DrawingA.h>
 #include <assert.h>
 #include <Xm/MwmUtil.h>
 #include <Xm/DialogS.h>
 #include <time.h>

 #if SGI
   #include <sys/syssgi.h>
 #elif SUN
 #elif LINUX
 #endif

 #define XDRAWPARAM pDisplay, (Window)pWindow, (GC)pSystemContext
 #define XWINPARAM  pDisplay, (Window)pWindow
 #define XGCPARAM   pDisplay, (GC)pSystemContext

// init the static variable about font
bool fontInit = false;
XFontStruct *fontStructs[] = {0, 0, 0, 0, 0, 0, 0};

struct SFontTable {char* name; char* string;};

static SFontTable fontTable[] = {
  {"SystemFont",        "-adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*"},   // kSystemFont
  {"NormalFontVeryBig", "-adobe-helvetica-medium-r-*-*-18-*-*-*-*-*-*-*"}, // kNormalFontVeryBig
  {"NormalFontBig",     "-adobe-helvetica-medium-r-normal-*-14-*-*-*-*-*-*-*"}, // kNormalFontBig
  {"NormalFont",        "-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*"}, // kNormalFont
  {"NormalFontSmall",   "-adobe-helvetica-medium-r-*-*-10-*-*-*-*-*-*-*"}, // kNormalFontSmall
  {"NormalFontSmaller",   "-adobe-helvetica-medium-r-*-*-9-*-*-*-*-*-*-*"}, // kNormalFontSmaller
  {"NormalFontVerySmall", "-adobe-helvetica-medium-r-*-*-8-*-*-*-*-*-*-*"},  // kNormalFontVerySmall
  {"SymbolFont",        "-adobe-symbol-medium-r-*-*-12-*-*-*-*-*-*-*"}     // kSymbolFont
};

long standardFontSize[] = { 12, 16, 14, 12, 10, 9, 8, 10 };

//-----------------------------------------------------------------------------
// declaration of different local functions
long convertPoint2Angle (CPoint &pm, CPoint &pt);

// callback for the frame
void _drawingAreaCallback (Widget widget, XtPointer clientData, XtPointer callData);
void _eventHandler (Widget w, XtPointer clientData, XEvent *event, char *p);
void _destroyCallback (Widget widget, XtPointer clientData, XtPointer callData);

// stuff for color
long getIndexColor8Bit (CColor color, Display *pDisplay, Colormap colormap);
long CDrawContext::nbNewColor = 0;
static CColor paletteNewColor[256];

//------ our user-defined XPM functions
bool xpmGetValues (char **ppDataXpm, long *pWidth, long *pHeight, long *pNcolor, long *pCpp);

 #if !USE_XPM
  #include "xpmloader.cpp"
 #endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#elif MAC

#if MACX
//-----------------------------------------------------------------------------
#include <QuickTime/ImageCompression.h>

const unsigned char* macXfontNames[] = {
	"\pArial",
	"\pArial",
	"\pArial",
	"\pArial",
	"\pArial",
	"\pArial",
	"\pArial",
	"\pSymbol"
};

//-----------------------------------------------------------------------------
#else
#include <QDOffscreen.h>
#include <StandardFile.h>
#include <Navigation.h>
#include <PictUtils.h>
#endif

long standardFontSize[] = { 12, 18, 14, 12, 10, 9, 9, 12 };

long convertPoint2Angle (CPoint &pm, CPoint &pt);
void RectNormalize (Rect& rect);
void CRect2Rect (const CRect &cr, Rect &rr);
void Rect2CRect (Rect &rr, CRect &cr);
void CColor2RGBColor (const CColor &cc, RGBColor &rgb);
void RGBColor2CColor (const RGBColor &rgb, CColor &cc);

static void install_drop (CFrame *frame);
static void remove_drop (CFrame *frame);

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
	rr.left   = cr.left;
	rr.right  = cr.right;
	rr.top    = cr.top;
	rr.bottom = cr.bottom;
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
	cc.red   = rgb.red  / 257;
	cc.green = rgb.green / 257;
	cc.blue  = rgb.blue / 257;
}

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

long standardFontSize[] = { 12, 18, 14, 12, 11, 10, 9, 12 };
const char*	standardFont  = "Swis721 BT";
const char* standardFontS = "Roman";
const char* systemFont    = "Swis721 BT";
const char* systemFontS   = "Bold";
const char* standardFontName[] = { systemFont,
	standardFont, standardFont, standardFont, standardFont, standardFont,
	standardFont };
const char* standardFontStyle[] = { systemFontS,
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
CColor kBlackCColor  = {0,     0,   0, 0};
CColor kWhiteCColor  = {255, 255, 255, 0};
CColor kGreyCColor   = {127, 127, 127, 0};
CColor kRedCColor    = {255,   0,   0, 0};
CColor kGreenCColor  = {0  , 255,   0, 0};
CColor kBlueCColor   = {0  ,   0, 255, 0};
CColor kYellowCColor = {255, 255,   0, 0};
CColor kMagentaCColor= {255,   0, 255, 0};
CColor kCyanCColor   = {0  , 255, 255, 0};

#define kDragDelay 0

//-----------------------------------------------------------------------------
// CDrawContext Implementation
//-----------------------------------------------------------------------------

CDrawContext::CDrawContext (CFrame *pFrame, void *pSystemContext, void *pWindow)
:	pSystemContext (pSystemContext), pWindow (pWindow), pFrame (pFrame), 
	frameWidth (1), lineStyle (kLineSolid), drawMode (kCopyMode)
	#if WINDOWS
	,pBrush (0), pFont (0), pPen (0), pOldBrush (0), pOldPen (0), pOldFont (0)
	#elif MAC
	,bInitialized (false)
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
	frameColor = kWhiteCColor;
	fillColor  = kBlackCColor;
	fontColor  = kWhiteCColor;

	// offsets use by offscreen
	offset (0, 0);
	offsetScreen (0, 0);

#if WINDOWS
	if (pSystemContext)
	{
		pOldBrush  = GetCurrentObject ((HDC)pSystemContext, OBJ_BRUSH);
		pOldPen    = GetCurrentObject ((HDC)pSystemContext, OBJ_PEN);
		pOldFont   = GetCurrentObject ((HDC)pSystemContext, OBJ_FONT);
		SetBkMode ((HDC)pSystemContext, TRANSPARENT);
	}
	iPenStyle = PS_SOLID;

	// get position 
	if (pWindow)
	{
		RECT  rctTempWnd;
		GetWindowRect ((HWND)pWindow, &rctTempWnd);
		offsetScreen.h = rctTempWnd.left;
		offsetScreen.v = rctTempWnd.top;
	}

#elif MOTIF 
	if (pFrame)
		pDisplay = pFrame->getDisplay ();
	
	// set the current font
	if (pSystemContext)
		setFont (kNormalFont);
	else
		fprintf (stderr, "Error in CDrawContext::CDrawContext : pSystemContext must not be Null!!!\n");

#elif BEOS
	pView = (BView*) pSystemContext;
	if (pView)
		pView->LockLooper ();

#endif

	if (pSystemContext)
	{
		// set the default values
		setFrameColor (frameColor);
		setLineStyle (lineStyle);
		
#if !MOTIF
		setFillColor (fillColor);
		setFontColor (fontColor);
#endif
	}
}

//-----------------------------------------------------------------------------
CDrawContext::~CDrawContext ()
{
	#if DEBUG
	gNbCDrawContext--;
	#endif

#if WINDOWS
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
  
#elif MAC
#elif MOTIF
#elif BEOS
	if (pView)
	{
		pView->Flush ();
		pView->UnlockLooper ();
	}
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::moveTo (const CPoint &_point)
{
	CPoint point (_point);
	point.offset (offset.h, offset.v);

#if WINDOWS
	MoveToEx ((HDC)pSystemContext, point.h, point.v, NULL);
  
#elif MAC
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	GetGWorld (&OrigPort, &OrigDevice); // get current GrafPort
	SetGWorld (getPort (), NULL);       // activate our GWorld
	MoveTo (point.h, point.v);
	SetGWorld (OrigPort, OrigDevice);
  	penLoc = point;
  	
#elif MOTIF || BEOS
	penLoc = point;
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::lineTo (const CPoint& _point)
{
	CPoint point (_point);
	point.offset (offset.h, offset.v);

#if WINDOWS
	LineTo ((HDC)pSystemContext, point.h, point.v);
	
#elif MAC
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

#elif MOTIF
	CPoint start (penLoc);
	CPoint end (point);
	if (start.h == end.h)
	{
		if (start.v < -5)
			start.v = -5;
		else if (start.v > 10000)
			start.v = 10000;
		
		if (end.v < -5)
			end.v = -5;
		else if (end.v > 10000)
			end.v = 10000;
	}
	if (start.v == end.v)
	{
		if (start.h < -5)
			start.h = -5;
		else if (start.h > 10000)
			start.h = 10000;
		
		if (end.h < -5)
			end.h = -5;
		else if (end.h > 10000)
			end.h = 10000;
	}
	XDrawLine (XDRAWPARAM, start.h, start.v, end.h, end.v);
	
	// keep trace of the new position
	penLoc = point;

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
void CDrawContext::polyLine (const CPoint *pPoints, long numberOfPoints)
{
#if WINDOWS
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

#elif MAC
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

#elif MOTIF
	XPoint* pt = (XPoint*)malloc (numberOfPoints * sizeof (XPoint));
	if (!pt)
		return;
	for (long i = 0; i < numberOfPoints; i++)
	{
		pt[i].x = (short)pPoints[i].h + offset.h;
		pt[i].y = (short)pPoints[i].v + offset.v;
	}
	
	XDrawLines (XDRAWPARAM, pt, numberOfPoints, CoordModeOrigin);

	free (pt);

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
void CDrawContext::setLineStyle (CLineStyle style)
{
	lineStyle = style;

#if WINDOWS
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

#elif MAC
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

#elif MOTIF
	long line_width;
	long line_style;
	if (frameWidth == 1)
		line_width = 0;
	else 
		line_width = frameWidth;

	switch (lineStyle)
	{
	case kLineOnOffDash:
		line_style = LineOnOffDash;
		break;
	default:
		line_style = LineSolid;
		break;
	}
	
	XSetLineAttributes (XGCPARAM, line_width, line_style, CapNotLast, JoinRound);
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::setLineWidth (long width)
{
	frameWidth = width;
	
#if WINDOWS
	LOGPEN logPen = {iPenStyle, {frameWidth, frameWidth},
					 RGB (frameColor.red, frameColor.green, frameColor.blue)};
	
	HANDLE newPen = CreatePenIndirect (&logPen);
	SelectObject ((HDC)pSystemContext, newPen);
	if (pPen)
		DeleteObject (pPen);
	pPen = newPen;
	
#elif MAC
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
#elif MOTIF
	setLineStyle (lineStyle);
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::setDrawMode (CDrawMode mode)
{
	if (drawMode == mode)
		return;

	drawMode = mode;

#if WINDOWS
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

#elif MAC
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

#elif MOTIF
	long iMode = 0;
	switch (drawMode) 
	{
	case kXorMode :
		iMode = GXinvert;
		break;
	case kOrMode :
		iMode = GXor;
		break;
	default:
		iMode = GXcopy;
	}
	((XGCValues*)pSystemContext)->function = iMode;
	XChangeGC (XGCPARAM, GCFunction, (XGCValues*)pSystemContext);
#endif
}

//------------------------------------------------------------------------------
void CDrawContext::setClipRect (const CRect &clip)
{
	clipRect = clip;

#if MAC
	Rect r;
	CRect2Rect (clip, r);

	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	GetGWorld (&OrigPort, &OrigDevice);
	SetGWorld (getPort (), NULL);
	ClipRect (&r);
	SetGWorld (OrigPort, OrigDevice);

#elif WINDOWS
	RECT r = {clip.left, clip.top, clip.right, clip.bottom};
	HRGN hRgn  = CreateRectRgn (r.left, r.top, r.right, r.bottom);
	SelectClipRgn ((HDC)pSystemContext, hRgn);
	DeleteObject (hRgn);

#elif MOTIF
	XRectangle r;
	r.x = 0;
	r.y = 0;
	r.width  = clip.right - clip.left;
	r.height = clip.bottom - clip.top;
	XSetClipRectangles (XGCPARAM, clip.left, clip.top, &r, 1, Unsorted); 

#elif BEOS
	clipping_rect r = {clip.left, clip.top, clip.right - 1, clip.bottom - 1};	
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

#if MAC || WINDOWS || MOTIF
	setClipRect (newClip);

#elif BEOS
	pView->ConstrainClippingRegion (NULL);
#endif

	clipRect = newClip;
}

//-----------------------------------------------------------------------------
void CDrawContext::fillPolygon (const CPoint *pPoints, long numberOfPoints)
{
	// Don't draw boundary
#if WINDOWS
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

#elif MAC
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
	
#elif MOTIF
	// convert the points
	XPoint* pt = (XPoint*)malloc (numberOfPoints * sizeof (XPoint));
	for (long i = 0; i < numberOfPoints; i++)
	{
		pt[i].x = (short)pPoints[i].h + offset.h;
		pt[i].y = (short)pPoints[i].v + offset.v;
	}
	
	XFillPolygon (XDRAWPARAM, pt, numberOfPoints, Convex, CoordModeOrigin);

	free (pt);

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
void CDrawContext::drawRect (const CRect &_rect)
{
	CRect rect (_rect);
	rect.offset (offset.h, offset.v);

#if WINDOWS
	MoveToEx ((HDC)pSystemContext, rect.left, rect.top, NULL);
	LineTo ((HDC)pSystemContext, rect.right, rect.top);
	LineTo ((HDC)pSystemContext, rect.right, rect.bottom);
	LineTo ((HDC)pSystemContext, rect.left, rect.bottom);
	LineTo ((HDC)pSystemContext, rect.left, rect.top);
	
#elif MAC
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	GetGWorld (&OrigPort, &OrigDevice);	// get current GrafPort
	SetGWorld (getPort (), NULL);       // activate our GWorld
	RGBColor col;
	CColor2RGBColor (frameColor, col);
	RGBForeColor (&col);
	MoveTo (rect.left, rect.top);
	LineTo (rect.right, rect.top);
	LineTo (rect.right, rect.bottom);
	LineTo (rect.left, rect.bottom);
	LineTo (rect.left, rect.top);
	SetGWorld (OrigPort, OrigDevice);

#elif MOTIF
	XDrawRectangle (XDRAWPARAM, rect.left, rect.top, rect.width (), rect.height ());

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
	RECT wr = {rect.left + 1, rect.top + 1, rect.right, rect.bottom};
	HANDLE nullPen = GetStockObject (NULL_PEN);
	HANDLE oldPen  = SelectObject ((HDC)pSystemContext, nullPen);
	FillRect ((HDC)pSystemContext, &wr, (HBRUSH)pBrush);
	SelectObject ((HDC)pSystemContext, oldPen);

#elif MAC
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

#elif MOTIF
	XFillRectangle (XDRAWPARAM, rect.left + 1, rect.top + 1, rect.width () - 1, rect.height () - 1);

#elif BEOS
	rgb_color c = { fillColor.red, fillColor.green, fillColor.blue, 255 };
	pView->SetHighColor (c);
	pView->SetDrawingMode (modeToPlatform [drawMode]);
	BRect r (rect.left + 1, rect.top + 1, rect.right - 1, rect.bottom - 1);
	pView->FillRect (r);
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::drawEllipse (const CRect &_rect)
{
	CPoint point (_rect.left + (_rect.right - _rect.left) / 2, _rect.top);
	drawArc (_rect, point, point);
}

//-----------------------------------------------------------------------------
void CDrawContext::fillEllipse (const CRect &_rect)
{
	CRect rect (_rect);
	rect.offset (offset.h, offset.v);

	// Don't draw boundary
#if WINDOWS
	HANDLE nullPen = GetStockObject (NULL_PEN);
	HANDLE oldPen  = SelectObject ((HDC)pSystemContext, nullPen);
	Ellipse ((HDC)pSystemContext, rect.left + 1, rect.top + 1, rect.right + 1, rect.bottom + 1);
	SelectObject ((HDC)pSystemContext, oldPen);

#else
	CPoint point (_rect.left + ((_rect.right - _rect.left) / 2), _rect.top);
	fillArc (_rect, point, point);
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::drawPoint (const CPoint &_point, CColor color)
{
	CPoint point (_point);
	point.offset (offset.h, offset.v);

#if WINDOWS
	SetPixel ((HDC)pSystemContext, point.h, point.v, RGB(color.red, color.green, color.blue));

#elif MOTIF
	CColor oldframecolor = frameColor;
	setFrameColor (color);
	XDrawPoint (XDRAWPARAM, point.h, point.v);
	setFrameColor (oldframecolor);

#elif MAC
	int oldframeWidth = frameWidth;
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

//-----------------------------------------------------------------------------
CColor CDrawContext::getPoint (const CPoint& _point)
{
	CPoint point (_point);
	point.offset (offset.h, offset.v);
	CColor color = kBlackCColor;

	#if WINDOWS
	COLORREF c  = GetPixel ((HDC)pSystemContext, point.h, point.v);
	color.red   = GetRValue (c);
	color.green = GetGValue (c);
	color.blue  = GetBValue (c);

	#elif MAC
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	GetGWorld (&OrigPort, &OrigDevice);
	SetGWorld (getPort (), NULL);
	RGBColor cPix;
	GetCPixel (point.h, point.v, &cPix);
	RGBColor2CColor (cPix, color);
	SetGWorld (OrigPort, OrigDevice);
	#endif

	return color;
}

//-----------------------------------------------------------------------------
void CDrawContext::floodFill (const CPoint& _start)
{
	CPoint start (_start);
	start.offset (offset.h, offset.v);

	#if WINDOWS
	COLORREF c = GetPixel ((HDC)pSystemContext, start.h, start.v);
	ExtFloodFill ((HDC)pSystemContext, start.h, start.v, c, FLOODFILLSURFACE);
	
	#elif MAC
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

	Arc ((HDC)pSystemContext, rect.left, rect.top, rect.right + 1, rect.bottom + 1, 
			 point1.h, point1.v, point2.h, point2.v);

#elif MAC || MOTIF || BEOS
	
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
	
#elif MOTIF
	XDrawArc (XDRAWPARAM, rect.left, rect.top, rect.width (), rect.height (),
						angle1, angle2);

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
	HANDLE nullPen = GetStockObject(NULL_PEN);
	HANDLE oldPen  = SelectObject ((HDC)pSystemContext, nullPen);
	Pie ((HDC)pSystemContext, offset.h + rect.left + 1, offset.v + rect.top + 1, offset.h + rect.right, offset.v + rect.bottom, 
			 point1.h, point1.v, point2.h, point2.v);
	SelectObject ((HDC)pSystemContext, oldPen);

#elif MAC || MOTIF || BEOS
	
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

#elif MOTIF
	XFillArc (XDRAWPARAM, rect.left, rect.top, rect.width (), rect.height (),
				angle1, angle2);

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
	SetTextColor ((HDC)pSystemContext, RGB (fontColor.red, fontColor.green, fontColor.blue));
	
#elif MAC
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

#elif MOTIF
	setFrameColor (fontColor);

#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::setFrameColor (const CColor color)
{
	frameColor = color;

#if WINDOWS
	LOGPEN logPen = {iPenStyle, {frameWidth, frameWidth}, 
					 RGB (frameColor.red, frameColor.green, frameColor.blue)};
	
	HANDLE newPen = CreatePenIndirect (&logPen);
	SelectObject ((HDC)pSystemContext, newPen);
	if (pPen)
		DeleteObject (pPen);
	pPen = newPen;

#elif MAC
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

#elif MOTIF
	XSetForeground (XGCPARAM, getIndexColor (frameColor));
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::setFillColor (const CColor color)
{
	fillColor = color;

#if WINDOWS
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
	
#elif MAC
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

#elif MOTIF
	// set the background for the text
	XSetBackground (XGCPARAM, getIndexColor (fillColor));
	
	// set the foreground for the fill
	setFrameColor (fillColor);
#endif
}

//-----------------------------------------------------------------------------
void CDrawContext::setFont (CFont fontID, const long size, long style)
{
	if (fontID < 0 || fontID >= kNumStandardFonts)
		fontID = kSystemFont;

	fontId = fontID;
	if (size != 0)
		fontSize = size;
	else
		fontSize = standardFontSize [fontID];

#if WINDOWS
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
	strcpy (logfont.lfFaceName, standardFontName[fontID]);

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
  
#elif MAC
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
		
		GetFNum (macXfontNames[fontID], &familyID);

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

#elif MOTIF
	XSetFont (XGCPARAM, fontStructs[fontID]->fid);
	
	// keep trace of the current font
	pFontInfoStruct = fontStructs[fontID];

#elif BEOS
	font.SetFamilyAndStyle (standardFontName[fontID], standardFontStyle[fontID]);
	font.SetSize (fontSize);
	pView->SetFont (&font, B_FONT_FAMILY_AND_STYLE | B_FONT_SIZE);
#endif
}

//------------------------------------------------------------------------------
long CDrawContext::getStringWidth (const char *pStr)
{
	long result = 0;

	#if MAC
	CGrafPtr OrigPort;
	GDHandle OrigDevice;
	GetGWorld (&OrigPort, &OrigDevice);
	SetGWorld (getPort (), NULL);

	result = (long)TextWidth (pStr, 0, strlen (pStr));

	SetGWorld (OrigPort, OrigDevice);

	#elif WINDOWS
	SIZE size;
	GetTextExtentPoint32 ((HDC)pSystemContext, pStr, strlen (pStr), &size);
	result = (long)size.cx;

	#elif MOTIF
	result = (long)XTextWidth (pFontInfoStruct, pStr, strlen (pStr));
	
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
	// set the visibility mask
	SetBkMode ((HDC)pSystemContext, opaque ? OPAQUE : TRANSPARENT);

	RECT Rect = {rect.left, rect.top, rect.right, rect.bottom};
	UINT flag = DT_VCENTER + DT_SINGLELINE + DT_NOPREFIX;
	switch (hAlign)
	{
	case kCenterText:
		// without DT_SINGLELINE no vertical center alignment here
		DrawText ((HDC)pSystemContext, string, strlen (string), &Rect, flag + DT_CENTER);
		break;
		
	case kRightText:
		DrawText ((HDC)pSystemContext, string, strlen (string), &Rect, flag + DT_RIGHT);
		break;
		
	default : // left adjust
		Rect.left++;
		DrawText ((HDC)pSystemContext, string, strlen (string), &Rect, flag + DT_LEFT);
	}

	SetBkMode ((HDC)pSystemContext, TRANSPARENT);

#elif MAC
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
		MoveTo (xPos, yPos);
		DrawText ((Ptr)string, 0, stringLength);

		SetClip (saveRgn);
		DisposeRgn (saveRgn);
		TextMode (srcOr);
		SetGWorld (OrigPort, OrigDevice);
	}

#elif MOTIF
	int width;
	int fontHeight = pFontInfoStruct->ascent + pFontInfoStruct->descent;
	int xPos;
	int yPos;
	int rectHeight = rect.height ();

	if (rectHeight >= fontHeight)
		yPos = rect.bottom - (rectHeight - fontHeight) / 2;
	else 
		yPos = rect.bottom;
	yPos -=	pFontInfoStruct->descent;

	switch (hAlign)
	{
	case kCenterText:
		width = XTextWidth (pFontInfoStruct, string, strlen (string));
		xPos = (rect.right + rect.left - width) / 2;
		break;
		
	case kRightText:
		width = XTextWidth (pFontInfoStruct, string, strlen (string));
		xPos = rect.right - width;
		break;
		
	default: // left adjust
		xPos = rect.left + 1;
	}
	
	if (opaque)
		XDrawImageString (XDRAWPARAM, xPos, yPos, string, strlen (string));
	else
		XDrawString (XDRAWPARAM, xPos, yPos, string, strlen (string));

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
long CDrawContext::getMouseButtons ()
{
	long buttons = 0;
	
#if WINDOWS
	if (GetAsyncKeyState (VK_LBUTTON) < 0)
		buttons |= (swapped_mouse_buttons ? kRButton : kLButton);
	if (GetAsyncKeyState (VK_MBUTTON) < 0)
		buttons |= kMButton;
	if (GetAsyncKeyState (VK_RBUTTON) < 0)
		buttons |= (swapped_mouse_buttons ? kLButton : kRButton);
	
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

#elif MOTIF
	Window root, child;
	long rootX, rootY, childX, childY;
	unsigned int mask;
	int result = XQueryPointer (XWINPARAM, &root, &child, &rootX, &rootY,
								&childX, &childY, &mask);
	if (mask & Button1Mask)
		buttons |= kLButton;
	if (mask & Button2Mask)
		buttons |= kMButton;
	if (mask & Button3Mask)
		buttons |= kRButton;

	if (mask & ShiftMask)
		buttons |= kShift;
	if (mask & ControlMask)
		buttons |= kControl;
	if (mask & Mod1Mask)
		buttons |= kAlt;

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

//-----------------------------------------------------------------------------
void CDrawContext::getMouseLocation (CPoint &point)
{
#if WINDOWS
	POINT where;
	GetCursorPos (&where);
	point (where.x, where.y);

#elif MAC
	Point where;
	GetMouse (&where);
	point (where.h, where.v);

#elif MOTIF
	Window root, child;
	int rootX, rootY, childX, childY;
	unsigned int mask;
	int result = XQueryPointer (XWINPARAM, &root, &child, &rootX, &rootY, 
								&childX, &childY, &mask);
	point (childX, childY);

#elif BEOS
	BPoint	where;
	uint32	b;
	pView->GetMouse (&where, &b);
	point (where.x, where.y);
#endif
	point.offset (-offsetScreen.h, -offsetScreen.v);
}

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
#elif MOTIF	
	long currentTime = _getTicks ();
	long clickTime = currentTime + XtGetMultiClickTime (pDisplay);
	
	XEvent e;
	while (currentTime < clickTime)
	{
		if (XCheckTypedEvent (pDisplay, ButtonPress, &e))
		{
			doubleClick = true;
			break;
		}
	
		currentTime = _getTicks ();
	}

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

//-----------------------------------------------------------------------------
bool CDrawContext::waitDrag ()
{
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
}

//-----------------------------------------------------------------------------
#if MOTIF
//-----------------------------------------------------------------------------
long CDrawContext::getIndexColor (CColor color)
{
	// 24bit visual ?
	if (pFrame->getDepth () == 24) 
		return (unsigned int)color.blue << 16 | (unsigned int)color.green << 8 | (unsigned int)color.red;

	// 8bit stuff
	return getIndexColor8Bit (color, pDisplay, pFrame->getColormap ());
}

//-----------------------------------------------------------------------------
Colormap CDrawContext::getColormap ()
{
	if (pFrame)
		return pFrame->getColormap ();
	else
		return NULL;
}

//-----------------------------------------------------------------------------
Visual* CDrawContext::getVisual ()
{
	if (pFrame)
		return pFrame->getVisual ();
	else
		return NULL;
}

//-----------------------------------------------------------------------------
unsigned int CDrawContext::getDepth ()
{
	if (pFrame)
		return pFrame->getDepth ();
	else
		return NULL;
}

//-----------------------------------------------------------------------------
#elif BEOS
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
BitMapPtr CDrawContext::getBitmap ()
{
	PixMapHandle pixMap = GetPortPixMap (GetWindowPort ((WindowRef)pWindow));
	if (pixMap)
	{
		LockPixels (pixMap);
		return (BitMapPtr)*pixMap;
	}
	return 0;
}

//-----------------------------------------------------------------------------
void CDrawContext::releaseBitmap ()
{
	PixMapHandle pixMap = GetPortPixMap (GetWindowPort ((WindowRef)pWindow));
	UnlockPixels (pixMap);
}

//-----------------------------------------------------------------------------
CGrafPtr CDrawContext::getPort ()
{
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
}

#endif


//-----------------------------------------------------------------------------
// COffscreenContext Implementation
//-----------------------------------------------------------------------------
COffscreenContext::COffscreenContext (CDrawContext *pContext, CBitmap *pBitmapBg, bool drawInBitmap)
	: CDrawContext (pContext->pFrame, NULL, NULL),
		pBitmap (0), pBitmapBg (pBitmapBg), height (20), width (20)
{
	if (pBitmapBg)
	{
		height = pBitmapBg->getHeight ();
		width  = pBitmapBg->getWidth ();
		
		clipRect (0, 0, width, height);
	}

	#if DEBUG
	gNbCOffscreenContext++;
	gBitmapAllocation += height * width;
	#endif
		
	bDestroyPixmap = false;
	
#if WINDOWS
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

#elif MAC
	
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

#elif MOTIF
 	// if no bitmap handle => create one
	if (!pWindow)
	{
		Drawable dWindow = pContext->pFrame->getWindow ();
		pWindow = (void*)XCreatePixmap (pDisplay, dWindow, width, height, pFrame->getDepth ());
		bDestroyPixmap = true;
	}

	// set the current font
	if (pSystemContext)
		setFont (kNormalFont);

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
	: CDrawContext (pFrame, NULL, NULL), 
	pBitmap (0), pBitmapBg (0), height (height), width (width), backgroundColor (backgroundColor)
{
	clipRect (0, 0, width, height);

	#if DEBUG
	gNbCOffscreenContext++;
	gBitmapAllocation += height * width;
	#endif

	bDestroyPixmap = true;

#if WINDOWS
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

#elif MAC
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
	
#elif MOTIF
	Drawable dWindow = pFrame->getWindow ();

	pWindow = (void*)XCreatePixmap (pDisplay, dWindow, width, height, pFrame->getDepth ());

	// clear the pixmap
	XGCValues values;
	values.foreground = getIndexColor (backgroundColor);
	GC gc = XCreateGC (pDisplay, (Drawable)pWindow, GCForeground, &values); 
	XFillRectangle (pDisplay, (Drawable)pWindow, gc, 0, 0, width, height);
	XFreeGC (pDisplay, gc);
	
	// set the current font
	if (pSystemContext)
		setFont (kNormalFont);

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
	gBitmapAllocation -= height * width;
	#endif

	if (pBitmap)
		pBitmap->forget ();

#if WINDOWS
	if (pSystemContext)
	{
		DeleteDC ((HDC)pSystemContext);
		#if DEBUG
		gNbDC--;
		#endif
	}
	if (bDestroyPixmap && pWindow)
		DeleteObject (pWindow);

#elif MAC
	if (bDestroyPixmap && pWindow)
		DisposeGWorld ((GWorldPtr)pWindow);

#elif MOTIF
	if (bDestroyPixmap && pWindow)
		XFreePixmap (pDisplay, (Pixmap)pWindow);

#elif BEOS
	delete offscreenBitmap;
	pView = 0;	// deleted because attached to the offscreen
#endif
}

//-----------------------------------------------------------------------------
void COffscreenContext::copyTo (CDrawContext* pContext, CRect& srcRect, CPoint destOffset)
{
#if WINDOWS
	BitBlt ((HDC)pSystemContext,
			destOffset.h,
			destOffset.v,
			srcRect.width (),
			srcRect.height (),
			(HDC)pContext->getSystemContext (),
			srcRect.left + pContext->offset.h,
			srcRect.top + pContext->offset.v,
			SRCCOPY);
			
#elif MAC
	if (!pWindow)
		return;

	Rect source, dest;
	RGBColor savedForeColor, savedBackColor;
	
	source.left   = srcRect.left + pContext->offset.h;
	source.top    = srcRect.top + pContext->offset.v;
	source.right  = source.left + srcRect.right - srcRect.left;
	source.bottom = source.top + srcRect.bottom - srcRect.top;
	
	dest.left   = destOffset.h;
	dest.top    = destOffset.v;
	dest.right  = dest.left + srcRect.right - srcRect.left;
	dest.bottom = dest.top + srcRect.bottom - srcRect.top;

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
	BitBlt ((HDC)pContext->getSystemContext (),  // hdcDest
					destRect.left + pContext->offset.h, // xDest
					destRect.top + pContext->offset.v,  // yDest
					destRect.right - destRect.left,     // xWidth,
					destRect.bottom - destRect.top,     // yHeight
          
					(HDC)pSystemContext,                // hdcSrc
					srcOffset.h,                        // xSrc
					srcOffset.v,                        // ySrc
					SRCCOPY);                           // dwROP

#elif MAC
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

#elif MOTIF
	XCopyArea (pDisplay, (Drawable)pWindow, (Drawable)pContext->getWindow (),
						 (GC)pSystemContext, srcOffset.h, srcOffset.v,
						 destRect.width (), destRect.height (),
						 destRect.left, destRect.top);

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
BitMapPtr COffscreenContext::getBitmap ()
{
	PixMapHandle pixMap = GetGWorldPixMap ((GWorldPtr)pWindow);
	if (pixMap)
	{
		LockPixels (pixMap);
		return (BitMapPtr)*pixMap;
	}
	return 0;
}

//-----------------------------------------------------------------------------
void COffscreenContext::releaseBitmap ()
{
	PixMapHandle pixMap = GetGWorldPixMap ((GWorldPtr)pWindow);
	UnlockPixels (pixMap);
}

//-----------------------------------------------------------------------------
CGrafPtr COffscreenContext::getPort ()
{
	if (!bInitialized)
		bInitialized = true;

	return (CGrafPtr)pWindow;
}
#endif

//-----------------------------------------------------------------------------
// CView
//-----------------------------------------------------------------------------
CView::CView (const CRect& size)
:	nbReference (1), size (size), mouseableArea (size), pParent (0), pParentView (0),
	bDirty (false), bMouseEnabled (true), bTransparencyEnabled (false)
{
	#if DEBUG
	gNbCView++;
	#endif
}

//-----------------------------------------------------------------------------
CView::~CView ()
{
	#if DEBUG
	gNbCView--;

	if (nbReference > 1)
		FDebugPrint ("nbReference is %d when trying to delete CView\n", nbReference);		
	#endif
}

//-----------------------------------------------------------------------------
void CView::redraw ()
{
	if (pParent)
		pParent->draw (this);
}

//-----------------------------------------------------------------------------
void CView::draw (CDrawContext *pContext)
{
	setDirty (false);
}

//-----------------------------------------------------------------------------
void CView::mouse (CDrawContext *pContext, CPoint &where)
{}

//-----------------------------------------------------------------------------
bool CView::onDrop (void **ptrItems, long nbItems, long type, CPoint &where)
{
	return false;
}

//-----------------------------------------------------------------------------
bool CView::onWheel (CDrawContext *pContext, const CPoint &where, float distance)
{
	return false;
}

//------------------------------------------------------------------------
void CView::update (CDrawContext *pContext)
{
	if (isDirty ())
	{
		draw (pContext);
		setDirty (false);
	}
}

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
void CView::looseFocus (CDrawContext *pContext)
{}

//------------------------------------------------------------------------------
void CView::takeFocus (CDrawContext *pContext)
{}

//------------------------------------------------------------------------------
void CView::setViewSize (CRect &rect)
{
	size = rect;
	setDirty ();
}

//-----------------------------------------------------------------------------
void CView::remember ()
{
	nbReference++;
}

//-----------------------------------------------------------------------------
void CView::forget ()
{
	if (nbReference > 0)
	{
		nbReference--;
		if (nbReference == 0)
			delete this;
	}
}

//-----------------------------------------------------------------------------
void *CView::getEditor ()
{ 
	return pParent ? pParent->getEditor () : 0; 
}

//-----------------------------------------------------------------------------
// CFrame Implementation
//-----------------------------------------------------------------------------
CFrame::CFrame (const CRect &size, void *pSystemWindow, void *pEditor)
:	CView (size), pEditor (pEditor), pSystemWindow (pSystemWindow), pBackground (0),
	viewCount (0), maxViews (0), ppViews (0), pModalView (0), pEditView (0),
	bFirstDraw (true), bDropActive (false), pFrameContext (0), bAddedWindow (false), 
	pVstWindow (0), defaultCursor (0)
{
	setOpenFlag (true);

#if WINDOWS
	pHwnd = 0;
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
    
#elif MOTIF
	gc = 0;
	depth    = 0;
	pDisplay = 0;
	pVisual  = 0;
	window   = 0;

#elif BEOS
	pPlugView = NULL;
#endif

	initFrame (pSystemWindow);

#if WINDOWS

#if USE_GLOBAL_CONTEXT
	hdc = GetDC ((HWND)getSystemWindow ());
	#if DEBUG
	gNbDC++;
	#endif

	pFrameContext = new CDrawContext (this, hdc, getSystemWindow ());
#endif

#elif MAC
	pFrameContext = new CDrawContext (this, getSystemWindow (), getSystemWindow ());
	pFrameContext->offset.h = size.left;
	pFrameContext->offset.v = size.top;
	
#elif MOTIF
	pFrameContext = new CDrawContext (this, gc, (void*)window);
#endif
}

//-----------------------------------------------------------------------------
CFrame::CFrame (const CRect &rect, char *pTitle, void *pEditor, const long style)
:	CView (rect), pEditor (pEditor), pSystemWindow (0), pBackground (0), viewCount (0),
	maxViews (0), ppViews (0), pModalView (0), pEditView (0), bFirstDraw (true),
	pFrameContext (0), defaultCursor (0)
{
	bAddedWindow  = true;
	setOpenFlag (false);

#if WINDOWS
	pHwnd = 0;
	OleInitialize (0);

#elif MOTIF
	gc = 0;
	depth    = 0;
	pDisplay = 0;
	pVisual  = 0;
	window   = 0;

#elif BEOS
	pPlugView = NULL;

#endif

	#if !PLUGGUI
	pVstWindow = (VstWindow*)malloc (sizeof (VstWindow));
	strcpy (((VstWindow*)pVstWindow)->title, pTitle);
	((VstWindow*)pVstWindow)->xPos   = (short)size.left;
	((VstWindow*)pVstWindow)->yPos   = (short)size.top;
	((VstWindow*)pVstWindow)->width  = (short)size.width ();
	((VstWindow*)pVstWindow)->height = (short)size.height ();
	((VstWindow*)pVstWindow)->style  = style;
	((VstWindow*)pVstWindow)->parent     = 0;
	((VstWindow*)pVstWindow)->userHandle = 0;
	((VstWindow*)pVstWindow)->winHandle  = 0;
	#endif
}

//-----------------------------------------------------------------------------
CFrame::~CFrame ()
{
	setCursor (kCursorDefault);

	setDropActive (false);

	removeAll (true);

	if (pBackground)
		pBackground->forget ();

	if (pFrameContext)
		delete pFrameContext;

#if WINDOWS
	OleUninitialize ();
	
	#if DYNAMICALPHABLEND
	if (hInstMsimg32dll)
		FreeLibrary (hInstMsimg32dll);
	#endif
		
	if (pHwnd)
	{
	#if USE_GLOBAL_CONTEXT
		ReleaseDC ((HWND)getSystemWindow (), hdc);
		#if DEBUG
		gNbDC--;
		#endif
	#endif

		SetWindowLong ((HWND)pHwnd, GWL_USERDATA, (long)NULL);
		DestroyWindow ((HWND)pHwnd);

		ExitWindowClass ();
	}

#elif MOTIF
 #if TEST_REGION
	XDestroyRegion (region);
 #endif

	// remove callbacks to avoid undesirable update
	if (pSystemWindow)
	{
		XtRemoveCallback ((Widget)pSystemWindow, XmNexposeCallback,  _drawingAreaCallback, this);
		XtRemoveCallback ((Widget)pSystemWindow, XmNinputCallback,   _drawingAreaCallback, this);
		XtRemoveCallback ((Widget)pSystemWindow, XmNdestroyCallback, _destroyCallback, this);
		
		freeGc ();
	}
#endif
	
	if (bAddedWindow)
		close ();
	if (pVstWindow)
		free (pVstWindow);

#if BEOS
	CBitmap::closeResource ();	// must be done only once at the end of the story.
#endif
}

//-----------------------------------------------------------------------------
bool CFrame::open (CPoint *point)
{
#if PLUGGUI
	return false;
#else
	if (!bAddedWindow)
		return false;
	if (getOpenFlag ())
	{
#if WINDOWS
		BringWindowToTop (GetParent (GetParent ((HWND)getSystemWindow ())));

#elif MOTIF
		Widget widget = (Widget)getSystemWindow ();
		while (widget && !XtIsTopLevelShell (widget))
			widget = XtParent (widget);
		if (widget)
			XRaiseWindow (getDisplay (), XtWindow (widget));

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
#endif
}

//-----------------------------------------------------------------------------
bool CFrame::close ()
{
#if PLUGGUI
	return false;
#else
	if (!bAddedWindow || !getOpenFlag () || !pSystemWindow)
		return false;

	AudioEffectX *pAudioEffectX = (AudioEffectX*)(((AEffGUIEditor*)pEditor)->getEffect ());
	pAudioEffectX->closeWindow ((VstWindow*)pVstWindow);

	pSystemWindow = 0;

	return true;
#endif
}

//-----------------------------------------------------------------------------
bool CFrame::initFrame (void *systemWin)
{
	if (!systemWin)
		return false;
	
#if WINDOWS
	InitWindowClass ();
	pHwnd = CreateWindowEx (0, className, "Window",
			 WS_CHILD | WS_VISIBLE, 
			 0, 0, size.width (), size.height (), 
			 (HWND)pSystemWindow, NULL, GetInstance (), NULL);

	SetWindowLong ((HWND)pHwnd, GWL_USERDATA, (long)this);

#elif MAC

#elif MOTIF
	// attach the callbacks
	XtAddCallback ((Widget)systemWin, XmNexposeCallback, _drawingAreaCallback, this);
	XtAddCallback ((Widget)systemWin, XmNinputCallback,  _drawingAreaCallback, this);
	XtAddCallback ((Widget)systemWin, XmNdestroyCallback, _destroyCallback, this);
	XtAddEventHandler ((Widget)systemWin, LeaveWindowMask, true, _eventHandler, this);

	// init a default gc
	window  = XtWindow ((Widget)systemWin);
	pDisplay = XtDisplay ((Widget)systemWin);
	XGCValues values;
	values.foreground = 1;
	gc = XCreateGC (pDisplay, (Drawable)window, GCForeground, &values); 
	
#if TEST_REGION
	region = XCreateRegion ();
#endif

	// get the std colormap
	XWindowAttributes attr;
	XGetWindowAttributes (pDisplay, window, &attr);
	colormap = attr.colormap;
	pVisual  = attr.visual;
	depth    = attr.depth;

	// init and load the fonts
	if (!fontInit)
	{
		for (long i = 0; i < kNumStandardFonts; i++) 
		{
			fontStructs[i] = XLoadQueryFont (pDisplay, fontTable[i].string);
			assert (fontStructs[i] != 0);
		}
		fontInit = true;
	}

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
	if (val)
		RegisterDragDrop ((HWND)pHwnd, (IDropTarget*)createDropTarget (this));
	else
		RevokeDragDrop ((HWND)pHwnd);

#elif MAC
	if (val)
		install_drop (this);
	else
		remove_drop (this);
#endif

	bDropActive = val;
	return true;
}

#if MOTIF
//-----------------------------------------------------------------------------
void CFrame::freeGc ()
{
	if (gc)
		XFreeGC (pDisplay, gc);
	gc = 0;
}
#endif

//-----------------------------------------------------------------------------
void CFrame::draw (CDrawContext *pContext)
{
	if (bFirstDraw)
		bFirstDraw = false;
	
	if (!pContext)
		pContext = pFrameContext;

	// draw first the background
	if (pBackground)
	{
		CRect r (0, 0, pBackground->getWidth (), pBackground->getHeight ());
		pBackground->draw (pContext, r);
	}

	// and the different children
	for (long i = 0; i < viewCount; i++)
		ppViews[i]->draw (pContext);

	// and the modal view
	if (pModalView)
		pModalView->draw (pContext);
}

//-----------------------------------------------------------------------------
void CFrame::drawRect (CDrawContext *pContext, CRect& updateRect)
{
	if (bFirstDraw)
		bFirstDraw = false;
	
	if (!pContext)
		pContext = pFrameContext;

	// draw first the background
	if (pBackground)
		pBackground->draw (pContext, updateRect, CPoint (updateRect.left, updateRect.top));

	// and the different children
	for (long i = 0; i < viewCount; i++)
	{
		if (ppViews[i]->checkUpdate (updateRect))
			ppViews[i]->drawRect (pContext, updateRect);
	}

	// and the modal view
	if (pModalView && pModalView->checkUpdate (updateRect))
		pModalView->draw (pContext);
}

//-----------------------------------------------------------------------------
void CFrame::draw (CView *pView)
{
	CView *pViewToDraw = 0;
	if (pView)
	{
		// Search it in the view list
		for (long i = 0; i < viewCount; i++)
			if (ppViews[i] == pView)
			{
				pViewToDraw = ppViews[i];
				break;
			}
	}

	#if WINDOWS
	HDC hdc2;
	#endif
	
	CDrawContext *pContext = pFrameContext;
	if (!pContext)
	{
	#if WINDOWS
		hdc2 = GetDC ((HWND)getSystemWindow ());
		#if DEBUG
		gNbDC++;
		#endif
		pContext = new CDrawContext (this, hdc2, getSystemWindow ());

	#elif MAC
		pContext = new CDrawContext (this, getSystemWindow (), getSystemWindow ());

	#elif MOTIF
		pContext = new CDrawContext (this, gc, (void*)window);

	#elif BEOS
		pContext = new CDrawContext (this, pPlugView, 0);
	#endif
	}

	if (pContext)
	{
		if (pViewToDraw)
			pViewToDraw->draw (pContext);
		else
			draw (pContext);

		if (!pFrameContext)
			delete pContext;
	}

	#if WINDOWS
	if (!pFrameContext)
	{
		ReleaseDC ((HWND)getSystemWindow (), hdc2);
		#if DEBUG
		gNbDC--;
		#endif
	}
	#endif
}

//-----------------------------------------------------------------------------
void CFrame::mouse (CDrawContext *pContext, CPoint &where)
{
	if (!pContext)
		pContext = pFrameContext;
	
	if (pEditView)
	{
		pEditView->looseFocus ();
		pEditView = 0;
	}

	long buttons = -1;
	if (pContext)
		buttons = pContext->getMouseButtons ();

	if (pModalView)
	{
		if (pModalView->hitTest (where, buttons))
			pModalView->mouse (pContext, where);
	}
	else 
	{
		for (long i = viewCount - 1; i >= 0; i--)
		{
			if (ppViews[i]->getMouseEnabled () && ppViews[i]->hitTest (where, buttons))
			{
				ppViews[i]->mouse (pContext, where);
				return;
			}
		}
	}
}

//-----------------------------------------------------------------------------
long CFrame::onKeyDown (VstKeyCode& keyCode)
{
	long result = -1;

	if (pEditView)
		result = pEditView->onKeyDown (keyCode);

	if (result == -1 && pModalView)
		result = pModalView->onKeyDown (keyCode);

	if (result == -1)
	{
		for (long i = viewCount - 1; i >= 0; i--)
		{
			if ((result = ppViews[i]->onKeyDown (keyCode)) != -1)
				break;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
long CFrame::onKeyUp (VstKeyCode& keyCode)
{
	long result = -1;

	if (pEditView)
		result = pEditView->onKeyUp (keyCode);

	if (result == -1 && pModalView)
		result = pModalView->onKeyUp (keyCode);

	if (result == -1)
	{
		for (long i = viewCount - 1; i >= 0; i--)
		{
			if ((result = ppViews[i]->onKeyUp (keyCode)) != -1)
				break;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
bool CFrame::onDrop (void **ptrItems, long nbItems, long type, CPoint &where)
{
	if (pModalView || pEditView)
		return false;

	bool result = false;

	// call the correct child
	for (long i = viewCount - 1; i >= 0; i--)
	{
		if (ppViews[i]->getMouseEnabled () && where.isInside (ppViews[i]->size))
		{
			if (ppViews[i]->onDrop (ptrItems, nbItems, type, where))
			{
				result = true;
				break;
			}
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
bool CFrame::onWheel (CDrawContext *pContext, const CPoint &where, float distance)
{
	bool result = false;

	CView *view = getCurrentView ();
	if (view)
	{
		CDrawContext *pContext2;
		if (pContext)
			pContext2 = pContext;
		else
			pContext2 = pFrameContext;

		#if WINDOWS
		HDC hdc2;
		#endif

		if (!pContext2)
		{
		#if WINDOWS
			hdc2 = GetDC ((HWND)getSystemWindow ());
			#if DEBUG
			gNbDC++;
			#endif
			pContext2 = new CDrawContext (this, hdc2, getSystemWindow ());

		#elif MAC
			pContext2 = new CDrawContext (this, getSystemWindow (), getSystemWindow ());

		#elif MOTIF
			pContext2 = new CDrawContext (this, gc, (void*)window);

		#elif BEOS
			if (pPlugView->LockLooperWithTimeout (0) != B_OK)
				return false;
			pContext2 = new CDrawContext (this, pPlugView, 0);
		#endif
		}

		CPoint where;
		getCurrentLocation (where);
		result = view->onWheel (pContext2, where, distance);

		if (!pFrameContext && !pContext)
			delete pContext2;
	
	#if WINDOWS
		if (!pFrameContext && !pContext)
		{
			ReleaseDC ((HWND)getSystemWindow (), hdc2);
			#if DEBUG
			gNbDC--;
			#endif
		}
	#elif BEOS
		pPlugView->UnlockLooper ();
	#endif
	}
	return result;
}
		
//-----------------------------------------------------------------------------
void CFrame::update (CDrawContext *pContext)
{
	if (!getOpenFlag ())
		return;
	
	if (pModalView)
		pModalView->update (pContext);
	else
	{
		if (isDirty ())
		{
			draw (pContext);
			setDirty (false);
		}
		else
		{
			for (long i = 0; i < viewCount; i++)
				ppViews[i]->update (pContext);
		}
	}
	#if MACX
	if (QDIsPortBufferDirty (GetWindowPort ((WindowRef)pSystemWindow)))
	{
		QDFlushPortBuffer (GetWindowPort ((WindowRef)pSystemWindow), NULL);
	}
	#endif
}

//-----------------------------------------------------------------------------
bool CFrame::isSomethingDirty ()
{
	if (pModalView || isDirty ())
		return true;
	else
	{
		for (long i = 0; i < viewCount; i++)
			if (ppViews[i]->isDirty ())
				return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CFrame::idle ()
{
	if (!getOpenFlag ())
		return;

	// don't do an idle before a draw
	if (bFirstDraw)
		return;

	if (!isSomethingDirty ())
		return;
		
	#if WINDOWS
	HDC hdc2;
	#endif

	CDrawContext *pContext = pFrameContext;
	if (!pContext)
	{
	#if WINDOWS
		hdc2 = GetDC ((HWND)getSystemWindow ());
		#if DEBUG
		gNbDC++;
		#endif
		pContext = new CDrawContext (this, hdc2, getSystemWindow ());

	#elif MAC
		pContext = new CDrawContext (this, getSystemWindow (), getSystemWindow ());

	#elif MOTIF
		pContext = new CDrawContext (this, gc, (void*)window);

	#elif BEOS
		if (pPlugView->LockLooperWithTimeout (0) != B_OK)
			return;
		pContext = new CDrawContext (this, pPlugView, 0);
	#endif
	}

	update (pContext);

	if (!pFrameContext)
		delete pContext;

	#if WINDOWS
	if (!pFrameContext)
	{
		ReleaseDC ((HWND)getSystemWindow (), hdc2);
		#if DEBUG
		gNbDC--;
		#endif
	}
	#elif BEOS
	pPlugView->UnlockLooper ();
	#endif
}

//-----------------------------------------------------------------------------
void CFrame::doIdleStuff ()
{
#if PLUGGUI
	if (pEditor)
		((PluginGUIEditor*)pEditor)->doIdleStuff ();
#else
	if (pEditor)
		((AEffGUIEditor*)pEditor)->doIdleStuff ();
#endif
}

//-----------------------------------------------------------------------------
unsigned long CFrame::getTicks ()
{
#if PLUGGUI
	if (pEditor)
		((PluginGUIEditor*)pEditor)->getTicks ();
#else
	if (pEditor)
		return ((AEffGUIEditor*)pEditor)-> getTicks ();
#endif
	return 0;
}

//-----------------------------------------------------------------------------
long CFrame::getKnobMode ()
{
#if PLUGGUI
	return PluginGUIEditor::getKnobMode ();
#else
	return AEffGUIEditor::getKnobMode ();
#endif
}

//-----------------------------------------------------------------------------
#if WINDOWS
HWND CFrame::getOuterWindow ()
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
		 
		// get the next parent window
		hTempWnd = GetParent (hTempWnd);
	}

	return NULL;
}
#endif

//-----------------------------------------------------------------------------
bool CFrame::getPosition (long &x, long &y)
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

#elif MOTIF
	Position xWin, yWin;

	// get the topLevelShell of the pSystemWindow
	Widget parent = (Widget)getSystemWindow ();
	Widget parentOld = parent;
	while (parent != 0 && !XtIsTopLevelShell (parent))
	{
		parentOld = parent;
		parent = XtParent (parent);
	}

	if (parent == 0)
		parent = parentOld;

	if (parent)
	{
		XtVaGetValues (parent, XtNx, &xWin, XtNy, &yWin, NULL);
		x = xWin - 8;
		y = yWin - 30;
	}

#elif BEOS
	BRect frame = pPlugView->Window ()->Frame ();
	x = (long) frame.left;
	y = (long) frame.top;
#endif
	return true;
}

//-----------------------------------------------------------------------------
bool CFrame::setSize (long width, long height)
{
	if (!getOpenFlag ())
		return false;
	
	if ((width == size.width ()) &&
		 (height == size.height ()))
	 return false;

#if !PLUGGUI
	if (pEditor)
	{
		AudioEffectX* effect = (AudioEffectX*)((AEffGUIEditor*)pEditor)->getEffect ();
		if (effect && effect->canHostDo ("sizeWindow"))
		{
			if (effect->sizeWindow (width, height))
			{
				#if WINDOWS
				SetWindowPos ((HWND)pHwnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE);
				#endif

				return true;
			}
		}
	}
#endif

	// keep old values
	long oldWidth  = size.width ();
	long oldHeight = size.height ();

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
		
		diffWidth  = (rctParentWnd.right - rctParentWnd.left) - (rctTempWnd.right - rctTempWnd.left);
		diffHeight = (rctParentWnd.bottom - rctParentWnd.top) - (rctTempWnd.bottom - rctTempWnd.top);
		
		if ((diffWidth > 80) || (diffHeight > 80)) // parent belongs to host
			return true;
		
		hTempWnd = hTempParentWnd;
	}
	
	if (hTempWnd)
		SetWindowPos (hTempWnd, HWND_TOP, 0, 0, width + diffWidth, height + diffHeight, SWP_NOMOVE);

#elif MAC
	if (getSystemWindow ())
	{
		Rect bounds;
		GetPortBounds (GetWindowPort ((WindowRef)getSystemWindow ()), &bounds);
		SizeWindow ((WindowRef)getSystemWindow (), (bounds.right - bounds.left) - oldWidth + width,
								(bounds.bottom - bounds.top) - oldHeight + height, true);
		#if MACX
		SetPort (GetWindowPort ((WindowRef)getSystemWindow ()));
		#endif
	}

#elif MOTIF
	Dimension heightWin, widthWin;

	// get the topLevelShell of the pSystemWindow
	Widget parent = (Widget)getSystemWindow ();
	Widget parentOld = parent;
	while (parent != 0 && !XtIsTopLevelShell (parent))
	{
		parentOld = parent;
		parent = XtParent (parent);
	}

	if (parent == 0)
		parent = parentOld;
	if (parent)
	{
		XtVaGetValues (parent, XtNwidth, &widthWin, XtNheight, &heightWin, NULL);	
		long diffWidth  = widthWin - oldWidth;
		long diffHeight = heightWin - oldHeight;
		XtVaSetValues (parent, XmNwidth, width + diffWidth, 
									 XmNheight, height + diffHeight, NULL);
	}

#elif BEOS
	BView* parent = pPlugView->Parent ();
	parent->SetResizingMode (B_FOLLOW_ALL_SIDES);
	BRect frame = pPlugView->Frame ();
	pPlugView->Window ()->ResizeBy (width - frame.Width () - 1, height - frame.Height () - 1);
	parent->SetResizingMode (B_FOLLOW_NONE);
#endif

	return true;
}

//-----------------------------------------------------------------------------
bool CFrame::getSize (CRect *pRect)
{
	if (!getOpenFlag ())
		return false;

#if WINDOWS
	// return the size relatif to the client rect of this window
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
	Rect bounds;
	GetPortBounds (GetWindowPort ((WindowRef)getSystemWindow ()), &bounds);

	pRect->left   = bounds.left;
	pRect->top    = bounds.top;
	pRect->right  = bounds.right;
	pRect->bottom = bounds.bottom;
	
#elif MOTIF
	Dimension height, width;
	XtVaGetValues ((Widget)getSystemWindow (),
								 XtNwidth, &width, XtNheight, &height, NULL);

	Position x, y;
	Position xTotal = 0, yTotal = 0;
	Widget parent = (Widget)getSystemWindow ();
	while (parent != 0 && !XtIsTopLevelShell (parent) && !XmIsDialogShell (parent))
	{
		XtVaGetValues (parent, XtNx, &x, XtNy, &y, NULL);	
		xTotal += x;
		yTotal += y;
		parent = XtParent (parent);
	}

	pRect->left   = xTotal;
	pRect->top    = yTotal;
	pRect->right  = width + pRect->left;
	pRect->bottom = height + pRect->top;

#elif BEOS
	BRect v = pPlugView->Frame ();
	(*pRect) (v.left, v.top, v.right + 1, v.bottom + 1);
#endif
	return true;
}

//-----------------------------------------------------------------------------
void CFrame::setBackground (CBitmap *background)
{
	if (pBackground)
		pBackground->forget ();
	pBackground = background;
	if (pBackground)
		pBackground->remember ();
}

//-----------------------------------------------------------------------------
bool CFrame::addView (CView *pView)
{
 	if (viewCount == maxViews)
	{
		maxViews += 20;
		if (ppViews)
			ppViews = (CView**)realloc (ppViews, maxViews * sizeof (CView*));
		else
			ppViews = (CView**)malloc (maxViews * sizeof (CView*));
		if (ppViews == 0)
		{
			maxViews = 0;
			return false;
		}
	}
	ppViews[viewCount] = pView;
	viewCount++;
	
	pView->pParent = this;
	pView->attached (this);

	return true;
}

//-----------------------------------------------------------------------------
bool CFrame::removeView (CView *pView, const bool &withForget)
{
	bool found = false;
	for (long i = 0; i < viewCount; i++)
	{
		if (found)
			ppViews[i - 1] = ppViews[i];
		if (ppViews[i] == pView)
		{
			pView->removed (this);
			if (withForget)
				pView->forget ();
			found = true;
		}
	}
	if (found)
		viewCount--;

	return true;
}

//-----------------------------------------------------------------------------
bool CFrame::removeAll (const bool &withForget)
{
	if (pEditView)
	{
		pEditView->looseFocus ();
		pEditView = 0;
	}

	if (ppViews)
	{
		for (long i = 0; i < viewCount; i++)
		{
			ppViews[i]->removed (this);
			if (withForget)
				ppViews[i]->forget ();
			ppViews[i] = 0;
		}

		free (ppViews);
		ppViews = 0;
		viewCount = 0;
		maxViews = 0;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool CFrame::isChild (CView *pView)
{
	bool found = false;
	for (long i = 0; i < viewCount; i++)
	{
		if (ppViews[i] == pView)
		{
			found = true;
			break;
		}
	}
	return found;
}

//-----------------------------------------------------------------------------
CView *CFrame::getView (long index)
{
	if (index >= 0 && index < viewCount)
		return ppViews[index];
	return 0;
}

//-----------------------------------------------------------------------------
long CFrame::setModalView (CView *pView)
{
	if (pView != NULL)
		if (pModalView)
			return 0;

	if (pModalView)
		pModalView->removed (this);
	
	pModalView = pView;
	if (pModalView)
		pModalView->attached (this);

	return 1;
}

//-----------------------------------------------------------------------------
void CFrame::beginEdit (long index)
{
#if PLUGGUI
#else
	if (pEditor)
		((AudioEffectX*)(((AEffGUIEditor*)pEditor)->getEffect ()))->beginEdit (index);
#endif
}

//-----------------------------------------------------------------------------
void CFrame::endEdit (long index)
{
#if PLUGGUI
#else
	if (pEditor)
		((AudioEffectX*)(((AEffGUIEditor*)pEditor)->getEffect ()))->endEdit (index);
#endif
}

//-----------------------------------------------------------------------------
CView *CFrame::getCurrentView ()
{
	if (pModalView)
		return pModalView;
	
	CPoint where;
	getCurrentLocation (where);

	for (long i = viewCount - 1; i >= 0; i--)
	{
		if (ppViews[i] && where.isInside (ppViews[i]->size))
			return ppViews[i];
	}
	return 0;
}

//-----------------------------------------------------------------------------
bool CFrame::getCurrentLocation (CPoint &where)
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
#endif

	// create a local context
	CDrawContext *pContextTemp = 0;

#if MAC
	pContextTemp = new CDrawContext (this, this->getSystemWindow (), this->getSystemWindow ());

#elif MOTIF
	pContextTemp = new CDrawContext (this, this->getGC (), (void *)this->getWindow ());

#elif BEOS
	pContextTemp = new CDrawContext (this, this->getSystemWindow (), NULL);
#endif

	// get the current position
	if (pContextTemp)
	{
		pContextTemp->getMouseLocation (where);
		delete pContextTemp;
	}
	return true;
}

//-----------------------------------------------------------------------------
void CFrame::setCursor (CCursorType type)
{
	#if WINDOWS
	if (!defaultCursor)
		defaultCursor = GetCursor ();
	switch (type)
	{
		case kCursorDefault:
			SetCursor ((HCURSOR)defaultCursor);
			break;
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
	}
	#elif MAC
	//if (!defaultCursor)
	//	defaultCursor = GetCursor (0);
	switch (type)
	{
		case kCursorDefault:
			InitCursor ();
			break;
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
	}
	#endif
}

//-----------------------------------------------------------------------------
void CFrame::setEditView (CView *pView)
{
	CView *pOldEditView = pEditView;
	pEditView = pView;

	if (pOldEditView)
		pOldEditView->looseFocus ();
}

//-----------------------------------------------------------------------------
void CFrame::invalidate (const CRect &rect)
{
	CRect rectView;
	long i;
	for (i = 0; i < viewCount; i++)
	{
		if (ppViews[i])
		{
			ppViews[i]->getViewSize (rectView);
			if (rect.rectOverlap (rectView))
				ppViews[i]->setDirty (true);
		}
	}   
}

//-----------------------------------------------------------------------------
// CCView Implementation
//-----------------------------------------------------------------------------
CCView::CCView (CView *pView)
 :  pView (pView), pNext (0), pPrevious (0)
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

#define FOREACHSUBVIEW for (CCView *pSv = pFirstView; pSv; pSv = pSv->pNext) {CView *pV = pSv->pView;
#define ENDFOR }

//-----------------------------------------------------------------------------
void modifyDrawContext (long save[4], CDrawContext* pContext, CRect& size);
void restoreDrawContext (CDrawContext* pContext, long save[4]);

char* kMsgCheckIfViewContainer	= "kMsgCheckIfViewContainer";

//-----------------------------------------------------------------------------
// CViewContainer Implementation
//-----------------------------------------------------------------------------
CViewContainer::CViewContainer (const CRect &rect, CFrame *pParent, CBitmap *pBackground)
: CView (rect), pFirstView (0), pLastView (0), pBackground (pBackground),
 mode (kNormalUpdate), pOffscreenContext (0), bDrawInOffscreen (true)
{
	#if MACX
	bDrawInOffscreen = false;
	#endif
	backgroundOffset (0, 0);
	this->pParent = pParent;
	if (pBackground)
		pBackground->remember ();
	backgroundColor = kBlackCColor;	
}

//-----------------------------------------------------------------------------
CViewContainer::~CViewContainer ()
{
	 if (pBackground)
		 pBackground->forget ();

	// remove all views
	removeAll (true);

	#if !BEOS
	 if (pOffscreenContext)
		delete pOffscreenContext;
	pOffscreenContext = 0;
	#endif
}

//-----------------------------------------------------------------------------
void CViewContainer::setViewSize (CRect &rect)
{
	CView::setViewSize (rect);

	#if !BEOS
	if (pOffscreenContext)
	{
		delete pOffscreenContext;
		pOffscreenContext = new COffscreenContext (pParent, size.width (), size.height (), kBlackCColor);
	}
	#endif
}

//-----------------------------------------------------------------------------
void CViewContainer::setBackground (CBitmap *background)
{
	if (pBackground)
		pBackground->forget ();
	pBackground = background;
	if (pBackground)
		pBackground->remember ();
}

//-----------------------------------------------------------------------------
void CViewContainer::setBackgroundColor (CColor color)
{
	backgroundColor = color;
}

//------------------------------------------------------------------------------
long CViewContainer::notify (CView* sender, const char* message)
{
	if (message == kMsgCheckIfViewContainer)
		return kMessageNotified;
	return kMessageUnknown;
}

//-----------------------------------------------------------------------------
void CViewContainer::addView (CView *pView)
{
	if (!pView)
		return;

	CCView *pSv = new CCView (pView);
	
	pView->pParent = pParent;
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
	pView->attached (this);
	pView->setDirty ();
}

//-----------------------------------------------------------------------------
void CViewContainer::addView (CView *pView, CRect &mouseableArea, bool mouseEnabled)
{
	if (!pView)
		return;

	pView->setMouseEnabled (mouseEnabled);
	pView->setMouseableArea (mouseableArea);

	addView (pView);
}

//-----------------------------------------------------------------------------
void CViewContainer::removeAll (const bool &withForget)
{
	CCView *pV = pFirstView;
	while (pV)
	{
		CCView *pNext = pV->pNext;
		if (pV->pView)
		{
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
void CViewContainer::removeView (CView *pView, const bool &withForget)
{
	CCView *pV = pFirstView;
	while (pV)
	{
		if (pView == pV->pView)
		{
			CCView *pNext = pV->pNext;
			CCView *pPrevious = pV->pPrevious;
			if (pV->pView)
			{
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
			pV = pNext;
		}
		else
			pV = pV->pNext;
	}
}

//-----------------------------------------------------------------------------
bool CViewContainer::isChild (CView *pView)
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
long CViewContainer::getNbViews ()
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
CView *CViewContainer::getView (long index)
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

//-----------------------------------------------------------------------------
void CViewContainer::draw (CDrawContext *pContext)
{
	CDrawContext *pC;
	long save[4];

	#if BEOS
	// create offscreen
	if (pBackground)
		pC = new COffscreenContext (pContext, pBackground);
	else
		pC = new COffscreenContext (pParent, size.width (), size.height (), backgroundColor);
	
	#else
	if (!pOffscreenContext && bDrawInOffscreen)
		pOffscreenContext = new COffscreenContext (pParent, size.width (), size.height (), kBlackCColor);

	if (bDrawInOffscreen)
		pC = pOffscreenContext;
	else
	{
		pC = pContext;
		modifyDrawContext (save, pContext, size);
	}

	// draw the background
	CRect r (0, 0, size.width (), size.height ());
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
		pV->draw (pC);
	ENDFOR

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
void CViewContainer::drawBackgroundRect (CDrawContext *pContext, CRect& _updateRect)
{
	if (pBackground)
	{
		CPoint p (_updateRect.left + backgroundOffset.h, _updateRect.top + backgroundOffset.v);
		if (bTransparencyEnabled)
			pBackground->drawTransparent (pContext, _updateRect, p);
		else
			pBackground->draw (pContext, _updateRect, p);
	}
	else if (!bTransparencyEnabled)
	{
		pContext->setFillColor (backgroundColor);
		pContext->fillRect (_updateRect);
	}
}

//-----------------------------------------------------------------------------
void CViewContainer::drawRect (CDrawContext *pContext, CRect& _updateRect)
{
	CDrawContext *pC;
	long save[4];

	#if BEOS
	// create offscreen
	if (pBackground)
		pC = new COffscreenContext (pContext, pBackground);
	else
		pC = new COffscreenContext (pParent, size.width (), size.height (), backgroundColor);
	
	#else
	if (!pOffscreenContext && bDrawInOffscreen)
		pOffscreenContext = new COffscreenContext (pParent, size.width (), size.height (), kBlackCColor);

	if (bDrawInOffscreen)
		pC = pOffscreenContext;
	else
	{
		pC = pContext;
		modifyDrawContext (save, pContext, size);
	}

	CRect updateRect (_updateRect);
	updateRect.bound (size);

	CRect clientRect (updateRect);
	clientRect.offset (-size.left, -size.top);

	// draw the background
	if (pBackground)
	{
		CPoint bgoffset (clientRect.left + backgroundOffset.h, clientRect.top+ backgroundOffset.v);
		if (bTransparencyEnabled)
			pBackground->drawTransparent (pC, clientRect, bgoffset);
		else
			pBackground->draw (pC, clientRect, bgoffset);
	}
	else if (!bTransparencyEnabled)
	{
		pC->setFillColor (backgroundColor);
		pC->fillRect (clientRect);
	}
	#endif
	
	// draw each view
	FOREACHSUBVIEW
		if (pV->checkUpdate (clientRect))
			pV->draw (pC);
	ENDFOR

	// transfert offscreen
	if (bDrawInOffscreen)
		((COffscreenContext*)pC)->copyFrom (pContext, updateRect, CPoint (clientRect.left, clientRect.top));
	else
		restoreDrawContext (pContext, save);

	#if BEOS
	delete pC;
	#endif

	setDirty (false);
}

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
void CViewContainer::mouse (CDrawContext *pContext, CPoint &where)
{
	// convert to relativ pos
	CPoint where2 (where);
	where2.offset (-size.left, -size.top);

	long save[4];
	modifyDrawContext (save, pContext, size);
	
	long buttons = -1;
	if (pContext)
		buttons = pContext->getMouseButtons ();

	CCView *pSv = pLastView;
	while (pSv)
	{
		CView *pV = pSv->pView;
		if (pV && pV->getMouseEnabled () && pV->hitTest (where2, buttons))
		{
			pV->mouse (pContext, where2);
			break;
		}
		pSv = pSv->pPrevious;
	}
	
	restoreDrawContext (pContext, save);
}

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
bool CViewContainer::onDrop (void **ptrItems, long nbItems, long type, CPoint &where)
{
	if (!pParent)
		return false;

	// convert to relativ pos
	CPoint where2 (where);
	where2.offset (-size.left, -size.top);

	bool result = false;
	CCView *pSv = pLastView;
	while (pSv)
	{
		CView *pV = pSv->pView;
		if (pV && pV->getMouseEnabled () && where2.isInside (pV->mouseableArea))
		{
			if (pV->onDrop (ptrItems, nbItems, type, where2))
			{
				result = true;
				break;
			}
		}
		pSv = pSv->pPrevious;
	}
	return result;
}

//-----------------------------------------------------------------------------
bool CViewContainer::onWheel (CDrawContext *pContext, const CPoint &where, float distance)
{
	bool result = false;
	CView *view = getCurrentView ();
	if (view)
	{
		// convert to relativ pos
		CPoint where2 (where);
		where2.offset (-size.left, -size.top);

		long save[4];
		modifyDrawContext (save, pContext, size);
	
		result = view->onWheel (pContext, where2, distance);

		restoreDrawContext (pContext, save);
	}
	return result;
}

//-----------------------------------------------------------------------------
void CViewContainer::update (CDrawContext *pContext)
{
	switch (mode)
	{
		//---Normal : redraw all...
		case kNormalUpdate:
			if (isDirty ())
				draw (pContext);
		break;
	
		//---Redraw only dirty controls-----
		case kOnlyDirtyUpdate:
			if (bDirty)
				draw (pContext);
			else if (bDrawInOffscreen && pOffscreenContext) 
			{
				bool doCopy = false;
				if (isDirty ())
					doCopy = true;

				FOREACHSUBVIEW
					pV->update (pOffscreenContext);
				ENDFOR

				// transfert offscreen
				if (doCopy)
					pOffscreenContext->copyFrom (pContext, size);
			}
			else
			{
				long save[4];
				modifyDrawContext (save, pContext, size);

				FOREACHSUBVIEW
					if (pV->isDirty ())
					{
						long oldMode = 0;
						CViewContainer* child = 0;
						if (pV->notify (this, kMsgCheckIfViewContainer))
						{
							child = (CViewContainer*)pV;
							oldMode = child->getMode ();
							child->setMode (kNormalUpdate);
						}
						CRect viewSize;
						pV->getViewSize (viewSize);
						drawBackgroundRect (pContext, viewSize);
						pV->update (pContext);
						if (child)
							child->setMode (oldMode);
					}
				ENDFOR

				restoreDrawContext (pContext, save);
			}
			setDirty (false);
		break;
	}
}

//-----------------------------------------------------------------------------
void CViewContainer::looseFocus (CDrawContext *pContext)
{
	FOREACHSUBVIEW
		pV->looseFocus (pContext);
	ENDFOR
}

//-----------------------------------------------------------------------------
void CViewContainer::takeFocus (CDrawContext *pContext)
{
	FOREACHSUBVIEW
		pV->takeFocus (pContext);
	ENDFOR
}

//-----------------------------------------------------------------------------
bool CViewContainer::isDirty ()
{
	if (bDirty)
		return true;
		
	FOREACHSUBVIEW
		if (pV->isDirty ())
			return true;
	ENDFOR
	return false;
}

//-----------------------------------------------------------------------------
CView *CViewContainer::getCurrentView ()
{
	if (!pParent)
		return 0;

	// get the current position
	CPoint where;
	pParent->getCurrentLocation (where);

	// convert to relativ pos
	where.offset (-size.left, -size.top);

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

//-----------------------------------------------------------------------------
bool CViewContainer::removed (CView* parent)
{
	#if !BEOS
	 if (pOffscreenContext)
		delete pOffscreenContext;
	pOffscreenContext = 0;
	#endif

	return true;
}

//-----------------------------------------------------------------------------
bool CViewContainer::attached (CView* view)
{
	#if !BEOS
	// create offscreen bitmap
	if (!pOffscreenContext && bDrawInOffscreen)
		pOffscreenContext = new COffscreenContext (pParent, size.width (), size.height (), kBlackCColor);
	#endif

	return true;
}

//-----------------------------------------------------------------------------
void CViewContainer::useOffscreen (bool b)
{
	bDrawInOffscreen = b;
	
	#if !BEOS
	if (!bDrawInOffscreen && pOffscreenContext)
	{
		delete pOffscreenContext;
		pOffscreenContext = 0;
	}
	#endif
}

//-----------------------------------------------------------------------------
void modifyDrawContext (long save[4], CDrawContext* pContext, CRect& size)
{
	// get the position of the context in the screen
	long offParentX = 0;
	long offParentY = 0;

	#if WINDOWS
	RECT rctTempWnd;
	GetWindowRect ((HWND)(pContext->getWindow ()), &rctTempWnd);
	offParentX = rctTempWnd.left;
	offParentY = rctTempWnd.top;	
	#endif

	// store
	save[0] = pContext->offsetScreen.h;
	save[1] = pContext->offsetScreen.v;
	save[2] = pContext->offset.h;
	save[3] = pContext->offset.v;

	pContext->offsetScreen.h = size.left + offParentX;
	pContext->offsetScreen.v = size.top + offParentY;
	pContext->offset.h = size.left;
	pContext->offset.v = size.top;
}

//-----------------------------------------------------------------------------
void restoreDrawContext (CDrawContext* pContext, long save[4])
{
	// restore
	pContext->offsetScreen.h = save[0];
	pContext->offsetScreen.v = save[1];
	pContext->offset.h = save[2];
	pContext->offset.v = save[3];
}

//-----------------------------------------------------------------------------
// CBitmap Implementation
//-----------------------------------------------------------------------------
CBitmap::CBitmap (long resourceID)
	: resourceID (resourceID), nbReference (1), width (0), height (0)
{
	#if DEBUG
	gNbCBitmap++;
	#endif

#if WINDOWS
	pMask = 0;
	pHandle = LoadBitmap (GetInstance (), MAKEINTRESOURCE (resourceID));
	BITMAP bm;
	if (pHandle && GetObject (pHandle, sizeof (bm), &bm))
	{
		width  = bm.bmWidth; 
		height = bm.bmHeight; 
	}

#elif MAC
	
	pHandle = 0;
	pMask = 0;
	
	#if (MACX && !PLUGGUI)
	if (gBundleRef)
	{
		char filename [PATH_MAX];
		sprintf (filename, "bmp%05d", (int)resourceID);
		CFStringRef cfStr = CFStringCreateWithCString (NULL, filename, kCFStringEncodingASCII);
		if (cfStr)
		{
			CFURLRef url = NULL;
			int i = 0;
			while (url == NULL)
			{
				static CFStringRef resTypes [] = { CFSTR("bmp"), CFSTR("png"), CFSTR("jpg"), CFSTR("pict"), NULL };
				url = CFBundleCopyResourceURL ((CFBundleRef)gBundleRef, cfStr, resTypes[i], NULL);
				if (resTypes[++i] == NULL)
					break;
			}
			CFRelease (cfStr);
			if (url)
			{
				FSRef fsRef;
				if (CFURLGetFSRef (url, &fsRef))
				{
					FSSpec fsSpec;
					FSCatalogInfoBitmap infoBitmap = kFSCatInfoNone;
					if (FSGetCatalogInfo (&fsRef, infoBitmap, NULL, NULL, &fsSpec, NULL) == noErr)
					{
						ComponentInstance gi;
						GetGraphicsImporterForFile (&fsSpec, &gi);
						if (gi)
						{
							Rect r;
							GraphicsImportGetSourceRect (gi, &r);
							OSErr err = NewGWorld ((GWorldPtr*)&pHandle, 0, &r, 0, 0, 0);
							if (!err)
							{
								width = r.right;
								height = r.bottom;
								GraphicsImportSetGWorld (gi, (GWorldPtr)pHandle, 0);
								GraphicsImportDraw (gi);
							}
							CloseComponent (gi);
						}
					}
				}
			}
			else
			{
				fprintf (stderr, "Bitmap Nr.:%d not found.\n", (int)resourceID);
			}
		}
	}
	#endif
	
	if (pHandle == 0)
	{
		Handle picHandle = GetResource ('PICT', resourceID);
		if (picHandle)
		{
			HLock (picHandle);
			
			PictInfo info;
			GetPictInfo ((PicHandle)picHandle, &info, recordComments, 0, systemMethod, 0);
			width  = info.sourceRect.right;
			height = info.sourceRect.bottom;
			
			OSErr err = NewGWorld ((GWorldPtr*)&pHandle, 0, &info.sourceRect, 0, 0, 0);
			if (!err)
			{
				GWorldPtr oldPort;
				GDHandle oldDevice;
				GetGWorld (&oldPort, &oldDevice);
				SetGWorld ((GWorldPtr)pHandle, 0);
				
				DrawPicture ((PicHandle)picHandle, &info.sourceRect);
				
				SetGWorld (oldPort, oldDevice);
			}

			HUnlock (picHandle);
			ReleaseResource (picHandle);
		}
	}

#elif MOTIF
	bool found = false;
	long  i = 0;
	long  ncolors, cpp;

	pHandle = 0;
	pMask  = 0;

	// find the good pixmap resource
	while (xpmResources[i].id != 0)
	{
		if (xpmResources[i].id == resourceID) 
		{
			if (xpmResources[i].xpm != NULL) 
			{
				found = true;
				ppDataXpm = xpmResources[i].xpm;
				
				xpmGetValues (ppDataXpm, &width, &height, &ncolors, &cpp);
				break;
			}
		}
		i++;
	}

	if (!found)
		ppDataXpm = 0;

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
	gBitmapAllocation += height * width;
	#endif
}

//-----------------------------------------------------------------------------
CBitmap::CBitmap (CFrame &frame, long width, long height)
	: nbReference (1), width (width), height (height)
{
	#if DEBUG
	gNbCBitmap++;
	#endif

#if WINDOWS
	HDC hScreen = GetDC (0);
	pHandle = CreateCompatibleBitmap (hScreen, width, height);
	ReleaseDC (0, hScreen);	
	pMask = 0;

#elif MAC
	pHandle = 0;
	pMask = 0;
	
	Rect r;
	r.left = r.top = 0;
	r.right = width;
	r.bottom = height;
	NewGWorld ((GWorldPtr*)&pHandle, 0, &r, 0, 0, 0);

	// todo: init pixels

#elif MOTIF
	pXdisplay = frame.getDisplay ();
	Drawable pWindow = frame.getWindow ();

	pMask = 0;
	pHandle = (void*)XCreatePixmap (pXdisplay, (Drawable)pWindow, width, height, frame.getDepth ());

#elif BEOS
	bbitmap = 0;
	transparencySet = false;
#endif
	
	setTransparentColor (kTransparentCColor);
}

//-----------------------------------------------------------------------------
CBitmap::~CBitmap ()
{
	#if DEBUG
	gNbCBitmap--;
	gBitmapAllocation -= height * width;
	#endif

	#if WINDOWS
	if (pHandle)
		DeleteObject (pHandle);
	if (pMask)
		DeleteObject (pMask);
	
	#elif MAC
	if (pHandle)
		DisposeGWorld ((GWorldPtr)pHandle);
	if (pMask)
		DisposeGWorld ((GWorldPtr)pMask);

	#elif MOTIF
	if (pHandle)
		XFreePixmap (pXdisplay, (Pixmap)pHandle);
	if (pMask) 
		XFreePixmap (pXdisplay, (Pixmap)pMask);

	#elif BEOS
	if (bbitmap)
		delete bbitmap;
	#endif
}

//-----------------------------------------------------------------------------
void *CBitmap::getHandle ()
 {
	#if WINDOWS||MOTIF
	return pHandle; 

	#elif MAC
	return pHandle;

	#elif BEOS
	return bbitmap;
	#endif
}

//-----------------------------------------------------------------------------
bool CBitmap::isLoaded ()
{
	#if MOTIF
	if (ppDataXpm)
		return true;
	
	#else
	if (getHandle ())
		return true;
	#endif

	return false;
}

//-----------------------------------------------------------------------------
void CBitmap::remember ()
{
	nbReference++;
}

//-----------------------------------------------------------------------------
void CBitmap::forget ()
{
	if (nbReference > 0)
	{
		nbReference--;
		if (nbReference == 0)
			delete this;
	}
}

//-----------------------------------------------------------------------------
void CBitmap::draw (CDrawContext *pContext, CRect &rect, const CPoint &offset)
{
#if WINDOWS
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

#elif MAC

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

#elif MOTIF
	if (!pHandle)
	{
		// the first time try to decode the pixmap
		pHandle = createPixmapFromXpm (pContext);
		if (!pHandle)
			return;
		
		// keep a trace of the display for deletion
		pXdisplay = pContext->pDisplay;
	}
	
#if DEVELOPMENT
	if (!(offset.h >= 0 && offset.v >= 0 &&
				rect.width () <= (getWidth () - offset.h) &&
				rect.height () <= (getHeight () - offset.v)))
	{
		fprintf (stderr, "%s(%d) -> Assert failed: try to display outside from the bitmap\n", __FILE__, __LINE__);
		return;
	}
#endif

	XCopyArea (pContext->pDisplay, (Drawable)pHandle, 
						 (Drawable)pContext->pWindow, 
						 (GC)pContext->pSystemContext, offset.h, offset.v,
						 rect.width (), rect.height (), rect.left, rect.top);

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
	
#elif MAC

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
	
#elif MOTIF
	if (!pHandle) 
	{
		// the first time try to decode the pixmap
		pHandle = createPixmapFromXpm (pContext);
		if (!pHandle)
			return;
		
		// keep a trace of the display for deletion
		pXdisplay = pContext->pDisplay;
	}
	
	if (pMask == 0)
	{
		// get image from the pixmap
		XImage* image = XGetImage (pContext->pDisplay, (Drawable)pHandle, 
                0, 0, width, height, AllPlanes, ZPixmap);
		assert (image);
		
		// create the bitmap mask
		pMask = (void*)XCreatePixmap (pContext->pDisplay, (Drawable)pContext->pWindow, 
                width, height, 1);
		assert (pMask);
		
		// create a associated GC
		XGCValues values;
		values.foreground = 1;
		GC gc = XCreateGC (pContext->pDisplay, (Drawable)pMask, GCForeground, &values); 
		
		// clear the mask
		XFillRectangle (pContext->pDisplay, (Drawable)pMask, gc, 0, 0, width, height);
   
		// get the transparent color index
		int color = pContext->getIndexColor (transparentCColor);
		
		// inverse the color
		values.foreground = 0;
		XChangeGC (pContext->pDisplay, gc, GCForeground, &values);
     
		// compute the mask
		XPoint *points = new XPoint [height * width];
		int x, y, nbPoints = 0;
		switch (image->depth) 
		{
		case 8:
			for (y = 0; y < height; y++) 
			{
				char* src = image->data + (y * image->bytes_per_line);
				
				for (x = 0; x < width; x++) 
				{
					if (src[x] == color) 
					{
						points[nbPoints].x = x;
						points[nbPoints].y = y;
						nbPoints++;
					}
				}
			}
			break;

		case 24: {
			int bytesPerPixel = image->bits_per_pixel >> 3;
			char *lp = image->data;
			for (y = 0; y < height; y++)
			{
				char* cp = lp;
				for (x = 0; x < width; x++)
				{
					if (*(int*)cp == color)
					{
						points[nbPoints].x = x;
						points[nbPoints].y = y;
						nbPoints++;
					}
					cp += bytesPerPixel;
				}
				lp += image->bytes_per_line;
			}
		} break;

		default :
			break;
		}

		XDrawPoints (pContext->pDisplay, (Drawable)pMask, gc,
								 points, nbPoints, CoordModeOrigin);
     
		// free 
		XFreeGC (pContext->pDisplay, gc);
		delete []points;

		// delete 
		XDestroyImage (image);
	}
	
	// set the new clipmask
	XGCValues value;
	value.clip_mask = (Pixmap)pMask;
	value.clip_x_origin = rect.left - offset.h;
	value.clip_y_origin = rect.top - offset.v;
	XChangeGC (pContext->pDisplay, (GC)pContext->pSystemContext,
						 GCClipMask|GCClipXOrigin|GCClipYOrigin, &value);

	XCopyArea (pContext->pDisplay, (Drawable)pHandle, (Drawable)pContext->pWindow, 
						 (GC)pContext->pSystemContext, offset.h, offset.v,
						 rect.width (), rect.height (), rect.left, rect.top);
	
	// unset the clipmask
	XSetClipMask (pContext->pDisplay, (GC)pContext->pSystemContext, None);
	

#elif BEOS
	if (!transparencySet)
	{
		uint32 c32 = transparentCColor.red | (transparentCColor.green  << 8) | (transparentCColor.blue << 16);
		uint32 *pix = (uint32*) bbitmap->Bits();
		uint32 ctr = B_TRANSPARENT_32_BIT.red | (B_TRANSPARENT_32_BIT.green << 8) | (B_TRANSPARENT_32_BIT.blue << 16) | (B_TRANSPARENT_32_BIT.alpha << 24);
		
		for (int32 z = 0, count = bbitmap->BitsLength() / 4; z < count; z++)
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
	if (pHandle)
	{
		HGDIOBJ hOldObj;
		HDC hdcMemory = CreateCompatibleDC ((HDC)pContext->pSystemContext);
		hOldObj = SelectObject (hdcMemory, pHandle);

		BLENDFUNCTION blendFunction;
		blendFunction.BlendOp = AC_SRC_OVER;
		blendFunction.BlendFlags = 0;
		blendFunction.SourceConstantAlpha = alpha;
		blendFunction.AlphaFormat = 0;//AC_SRC_NO_ALPHA;

		#if DYNAMICALPHABLEND
		(*pfnAlphaBlend) ((HDC)pContext->pSystemContext, 
					rect.left + pContext->offset.h, rect.top + pContext->offset.v,
					rect.width (), rect.height (), 
					(HDC)hdcMemory,
					offset.h, offset.v,
					rect.width (), rect.height (),
					blendFunction);
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

#elif MAC

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
}
//-----------------------------------------------------------------------------
void CBitmap::setTransparentColor (const CColor color)
{
	transparentCColor = color;
}

//-----------------------------------------------------------------------------
void CBitmap::setTransparencyMask (CDrawContext* pContext, const CPoint& offset)
{
#if WINDOWS
	if (pMask)
		DeleteObject (pMask);

	CRect r (0, 0, width, height);
	r.offset (offset.h, offset.v);
	pMask = CreateMaskBitmap (pContext, r, transparentCColor);

#elif MAC
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
	
#else
	// todo: implement me!
#endif
}

//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#if MOTIF
//-----------------------------------------------------------------------------
void* CBitmap::createPixmapFromXpm (CDrawContext *pContext)
{
	if (!ppDataXpm)
		return NULL;
  
	Pixmap pixmap = 0;
	XpmAttributes attributes;
	
	attributes.valuemask = XpmCloseness|XpmColormap|XpmVisual|XpmDepth; 
	attributes.closeness = 100000;
	attributes.visual = pContext->getVisual ();
	attributes.depth  = pContext->getDepth ();

	// use the pContext colormap instead of the DefaultColormapOfScreen
	attributes.colormap = pContext->getColormap ();

	int status;
	if (attributes.depth == 8 || attributes.depth == 24)
	{
#if USE_XPM
		status = XpmCreatePixmapFromData (pContext->pDisplay,
							(Drawable)pContext->pWindow, ppDataXpm, &pixmap, NULL, &attributes);
		if (status != XpmSuccess)
		{
			fprintf (stderr, "createPixmapFromXpm-> XpmError: %s\n", XpmGetErrorString(status));
			return NULL;
		}
#else
		status = createPixmapFromData (pContext->pDisplay,
         (Drawable)pContext->pWindow, ppDataXpm, &pixmap, &attributes);
		if (!status)
		{
			fprintf (stderr, "createPixmapFromXpm-> Error\n");
			return NULL;
		}
#endif
	}
	else
	{
		fprintf (stderr, "createPixmapFromXpm-> Depth %d not supported\n", attributes.depth);
		return NULL;
	}

#if DEVELOPMENT
	fprintf (stderr, "createPixmapFromXpm-> There are %d requested colors\n", attributes.ncolors);
#endif

	return (void*)pixmap;
}
#endif

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
END_NAMESPACE_VSTGUI

#if !PLUGGUI
//-----------------------------------------------------------------------------
// CFileSelector Implementation
//-----------------------------------------------------------------------------
#define stringAnyType  "Any Type (*.*)"
#define stringAllTypes "All Types: ("
#define stringSelect   "Select"
#define stringCancel   "Cancel"
#define stringLookIn   "Look in"
#define kPathMax        1024

#if WINDOWS
UINT APIENTRY SelectDirectoryHook (HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SelectDirectoryButtonProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
FARPROC fpOldSelectDirectoryButtonProc;
UINT APIENTRY WinSaveHook (HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
static bool folderSelected;
static bool didCancel;
static char selDirPath[kPathMax];
#endif


BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
CFileSelector::CFileSelector (AudioEffectX* effect)
: effect (effect), vstFileSelect (0)
{}

//-----------------------------------------------------------------------------
CFileSelector::~CFileSelector ()
{
	if (vstFileSelect)
	{
		if (effect && effect->canHostDo ("closeFileSelector"))
			effect->closeFileSelector (vstFileSelect);
		else 
		{
			if (vstFileSelect->reserved == 1 && vstFileSelect->returnPath)
			{
				delete []vstFileSelect->returnPath;
				vstFileSelect->returnPath = 0;
				vstFileSelect->sizeReturnPath = 0;
			}
			if (vstFileSelect->returnMultiplePaths)
			{
				for (long i = 0; i < vstFileSelect->nbReturnPath; i++)
				{
					delete []vstFileSelect->returnMultiplePaths[i];
					vstFileSelect->returnMultiplePaths[i] = 0;
				}
				delete[] vstFileSelect->returnMultiplePaths;
				vstFileSelect->returnMultiplePaths = 0;
			}
		}
	}
}

#if CARBON
pascal void navEventProc (const NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD);
pascal void navEventProc (const NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD) 
{
	AudioEffectX* effect = (AudioEffectX*)callBackUD;
	switch (callBackSelector)
	{
		case kNavCBEvent:
		{
			if (callBackParms->eventData.eventDataParms.event->what == nullEvent)
				effect->masterIdle ();
			break;
		}
	}
}
#endif

//-----------------------------------------------------------------------------
long CFileSelector::run (VstFileSelect *vstFileSelect)
{
	this->vstFileSelect = vstFileSelect;
	vstFileSelect->nbReturnPath = 0;
	vstFileSelect->returnPath[0] = 0;

	if (effect
	#if MACX 
		&& vstFileSelect->command != kVstFileSave 
	#endif
		&& effect->canHostDo ("openFileSelector") && effect->canHostDo ("closeFileSelector"))
	{
		if (effect->openFileSelector (vstFileSelect))
			return vstFileSelect->nbReturnPath;
	}
	else
	{
#if WINDOWS
		char filter[512];
		char filePathBuffer[kPathMax];
		strcpy (filePathBuffer, "");
		char* filePath = filePathBuffer;
		char fileName[kPathMax];
		strcpy (fileName, "");
		filter[0] = 0;
		filePath[0] = 0;
		fileName[0] = 0;

		//-----------------------------------------
		if (vstFileSelect->command == kVstFileLoad ||
			vstFileSelect->command == kVstMultipleFilesLoad ||
			vstFileSelect->command == kVstDirectorySelect)
		{
			char* multiBuffer = 0;
			if (vstFileSelect->command == kVstMultipleFilesLoad)
			{
				multiBuffer = new char [kPathMax * 100];
				strcpy (multiBuffer, "");
				filePath = multiBuffer;
			}

			if (vstFileSelect->command != kVstDirectorySelect) 
			{
				char allBuffer [kPathMax] = {0};
				char* p = filter;
				char* p2 = allBuffer;

				const char* ext;
				const char* extensions [100];
				long i, j, extCount = 0;
				char string[24];

				for (long ty = 0; ty < vstFileSelect->nbFileTypes; ty++)
				{
					for (i = 0; i < 2 ; i++)
					{				
						if (i == 0)
						{
							ext = vstFileSelect->fileTypes[ty].dosType;
						
							strcpy (p, vstFileSelect->fileTypes[ty].name);
							strcat (p, " (.");
							strcat (p, ext);
							strcat (p, ")");
							p += strlen (p) + 1;

							strcpy (string, "*.");
							strcat (string, ext);
							strcpy (p, string);
							p += strlen (p);	
						}
						else
						{
							if (!strcmp (vstFileSelect->fileTypes[ty].dosType, vstFileSelect->fileTypes[ty].unixType) || !strcmp (vstFileSelect->fileTypes[ty].unixType, ""))
								break; // for
							ext = vstFileSelect->fileTypes[ty].unixType;
							strcpy (string, ";*.");
							strcat (string, ext);
							strcpy (p, string);
							p += strlen (p);	
						}
						bool found = false;
						for (j = 0; j < extCount;j ++)
						{
							if (strcmp (ext, extensions [j]) == 0)
							{
								found = true;
								break;
							}
						}
						if (!found && extCount < 100)
							extensions [extCount++] = ext;
					}
					p ++;
				} // end for filetype
			
				if (extCount > 1)
				{
					for (i = 0; i < extCount ;i ++)
					{					
						ext = extensions [i];
						strcpy (string, "*.");
						strcat (string, ext);

						if (p2 != allBuffer)
						{
							strcpy (p2, ";");
							p2++;
						}
						strcpy (p2, string);
						p2 += strlen (p2);
					}

					// add the : All types
					strcpy (p, stringAllTypes);			
					strcat (p, allBuffer);
					strcat (p, ")");
					p += strlen (p) + 1;
					strcpy (p, allBuffer);
					p += strlen (p) + 1;			
				}

				strcpy (p, stringAnyType);
				p += strlen (p) + 1;
				strcpy (p, "*.*");
				p += strlen (p) + 1;

				*p++ = 0;
				*p++ = 0;
			}

			OPENFILENAME ofn = {0};
			ofn.lStructSize  = sizeof (OPENFILENAME);
			HWND owner = 0;
			if (effect->getEditor () && ((AEffGUIEditor*)effect->getEditor ())->getFrame ())
				owner = (HWND)((AEffGUIEditor*)effect->getEditor ())->getFrame ()->getSystemWindow ();
			ofn.hwndOwner    = owner;
	
			if (vstFileSelect->command == kVstDirectorySelect) 
				ofn.lpstrFilter = "HideFileFilter\0*.___\0\0"; // to hide files
			else
				ofn.lpstrFilter  = filter[0] ? filter : 0;
			ofn.nFilterIndex = 1;
			ofn.lpstrCustomFilter = NULL;
			ofn.lpstrFile    = filePath;
			if (vstFileSelect->command == kVstMultipleFilesLoad)
				ofn.nMaxFile    = 100 * kPathMax - 1;
			else
				ofn.nMaxFile    = sizeof (filePathBuffer) - 1;

			ofn.lpstrFileTitle  = fileName;
			ofn.nMaxFileTitle   = 64;
			ofn.lpstrInitialDir = vstFileSelect->initialPath;
			ofn.lpstrTitle      = vstFileSelect->title;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLEHOOK;
			if (vstFileSelect->command == kVstDirectorySelect)
			{
				ofn.Flags &= ~OFN_FILEMUSTEXIST;
				ofn.lpfnHook = SelectDirectoryHook;
			}

			if (vstFileSelect->command == kVstMultipleFilesLoad)
				ofn.Flags |= OFN_ALLOWMULTISELECT;
		
			vstFileSelect->nbReturnPath = 0;
			didCancel = true;

			if (GetOpenFileName (&ofn) || 
				((vstFileSelect->command == kVstDirectorySelect) && !didCancel && strlen (selDirPath) != 0))  
			{
				switch (vstFileSelect->command)
				{
				case kVstFileLoad:
					vstFileSelect->nbReturnPath = 1;
					if (!vstFileSelect->returnPath)
					{
						vstFileSelect->reserved = 1;
						vstFileSelect->returnPath = new char[strlen (ofn.lpstrFile) + 1];
						vstFileSelect->sizeReturnPath = strlen (ofn.lpstrFile) + 1;			
					}
					strcpy (vstFileSelect->returnPath, ofn.lpstrFile);
					break;
				
				case kVstMultipleFilesLoad:
					{
					char string[kPathMax], directory[kPathMax];
					char *previous = ofn.lpstrFile;
					long len;
					bool dirFound = false;
					bool first = true;
					directory[0] = 0; // !!
					vstFileSelect->returnMultiplePaths = new char*[kPathMax];
					long i = 0;
					while (1)
					{
						if (*previous != 0)
						{   // something found
							if (!dirFound) 
							{
								dirFound = true;
								strcpy (directory, previous);
								len = strlen (previous) + 1;  // including 0
								previous += len;

								if (*previous == 0)
								{  // 1 selected file only		
									vstFileSelect->returnMultiplePaths[i] = new char [strlen (directory) + 1];
									strcpy (vstFileSelect->returnMultiplePaths[i++], directory);
								}
								else
								{
									if (directory[strlen (directory) - 1] != '\\')
										strcat (directory, "\\");
								}
							}
							else 
							{
								sprintf (string, "%s%s", directory, previous);
								len = strlen (previous) + 1;  // including 0
								previous += len;

								vstFileSelect->returnMultiplePaths[i] = new char [strlen (string) + 1];
								strcpy (vstFileSelect->returnMultiplePaths[i++], string);
							}
						}
						else
							break;
					}
					vstFileSelect->nbReturnPath = i;					
					} break;

				case kVstDirectorySelect: 
					vstFileSelect->nbReturnPath = 1;
					if (!vstFileSelect->returnPath)
					{
						vstFileSelect->reserved = 1;
						vstFileSelect->returnPath = new char[strlen (selDirPath) + 1];
						vstFileSelect->sizeReturnPath = strlen (selDirPath) + 1;			
					}
					strcpy (vstFileSelect->returnPath, selDirPath);
				}
				if (multiBuffer)
					delete []multiBuffer;
				return vstFileSelect->nbReturnPath;
			}
			if (multiBuffer)
				delete []multiBuffer;
		}

		//-----------------------------------------
		else if (vstFileSelect->command == kVstFileSave)
		{
			char* p = filter;
			for (long ty = 0; ty < vstFileSelect->nbFileTypes; ty++)
			{
				const char* ext = vstFileSelect->fileTypes[ty].dosType;
				if (ext)
				{
					strcpy (p, vstFileSelect->fileTypes[ty].name);
					strcat (p, " (.");
					strcat (p, ext);
					strcat (p, ")");
					p += strlen (p) + 1;
	
					char string[24];
					strcpy (string, "*.");
					strcat (string, ext);
					strcpy (p, string);
					p += strlen (p) + 1;
				}
			}
			*p++ = 0;
			*p++ = 0;
		
			OPENFILENAME ofn = {0};
			ofn.lStructSize  = sizeof (OPENFILENAME);
			HWND owner = 0;
			if (effect->getEditor () && ((AEffGUIEditor*)effect->getEditor ())->getFrame ())
				owner = (HWND)((AEffGUIEditor*)effect->getEditor ())->getFrame ()->getSystemWindow ();
			ofn.hwndOwner    = owner;
			ofn.hInstance    = GetInstance ();
			ofn.lpstrFilter = filter[0] ? filter : 0;
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = filePath;
			ofn.lpstrCustomFilter = NULL;
			ofn.nMaxFile = sizeof (filePathBuffer) - 1;
			ofn.lpstrFileTitle = fileName;
			ofn.nMaxFileTitle = 64;
			ofn.lpstrInitialDir = vstFileSelect->initialPath;
			ofn.lpstrTitle = vstFileSelect->title;
			ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_ENABLEHOOK;
			
			if (vstFileSelect->nbFileTypes >= 1)
				ofn.lpstrDefExt = vstFileSelect->fileTypes[0].dosType;
			
			// add a template view
			ofn.lCustData = (DWORD)0;
			ofn.lpfnHook = WinSaveHook;
			
			if (GetSaveFileName (&ofn))	
			{
				vstFileSelect->nbReturnPath = 1;
				if (!vstFileSelect->returnPath)
				{
					vstFileSelect->reserved = 1;
					vstFileSelect->returnPath = new char[strlen (ofn.lpstrFile) + 1];
					vstFileSelect->sizeReturnPath = strlen (ofn.lpstrFile) + 1;			
				}
				strcpy (vstFileSelect->returnPath, ofn.lpstrFile);
			
				return vstFileSelect->nbReturnPath;
			}
			#if _DEBUG
			else
			{
				DWORD err = CommDlgExtendedError (); // for breakpoint
			}
			#endif
		}

#elif MAC
#if CARBON
		NavEventUPP	eventUPP = NewNavEventUPP (navEventProc);
		if (vstFileSelect->command == kVstFileSave)
		{
			NavDialogCreationOptions dialogOptions;
			NavGetDefaultDialogCreationOptions (&dialogOptions);
			dialogOptions.windowTitle = CFSTR ("Select a Destination");
	        CFStringRef defSaveName = 0;
			if (vstFileSelect->initialPath && ((FSSpec*)vstFileSelect->initialPath)->name)
			{
				FSSpec* defaultSpec = (FSSpec*)vstFileSelect->initialPath;
				defSaveName = CFStringCreateWithPascalString (NULL, defaultSpec->name, kCFStringEncodingASCII);
				if (defSaveName)
					dialogOptions.saveFileName = defSaveName;
				*defaultSpec->name = 0;
			}
			NavDialogRef dialogRef;
			if (NavCreatePutFileDialog (&dialogOptions, NULL, kNavGenericSignature, eventUPP, effect, &dialogRef) == noErr) 
			{
			    AEDesc defaultLocation;   
			    AEDesc* defLocPtr = 0;   
				if (vstFileSelect->initialPath)
				{
					FSSpec* defaultSpec = (FSSpec*)vstFileSelect->initialPath;
			        if (defaultSpec->parID && defaultSpec->vRefNum)
			        {
			            if (AECreateDesc (typeFSS, defaultSpec, sizeof(FSSpec), &defaultLocation) == noErr)
			                defLocPtr = &defaultLocation;
					}
				}
		        if (defLocPtr)
		            NavCustomControl (dialogRef, kNavCtlSetLocation, (void*)defLocPtr);
				NavDialogRun (dialogRef);

				if (defLocPtr)
		            AEDisposeDesc (defLocPtr);

				NavReplyRecord navReply;
				if (NavDialogGetReply (dialogRef, &navReply) == noErr)
				{
					FSRef parentFSRef;
					AEKeyword theAEKeyword;
					DescType typeCode;
					Size actualSize;
			        // get the FSRef referring to the parent directory
				    if (AEGetNthPtr(&navReply.selection, 1, typeFSRef,
		        		&theAEKeyword, &typeCode, &parentFSRef, sizeof(FSRef), &actualSize) == noErr)
					{
						FSSpec spec;
						FSCatalogInfoBitmap infoBitmap = kFSCatInfoNone;
						FSGetCatalogInfo (&parentFSRef, infoBitmap, NULL, NULL, &spec, NULL);
						CInfoPBRec pbRec = {0};	
						pbRec.dirInfo.ioDrDirID = spec.parID;
						pbRec.dirInfo.ioVRefNum = spec.vRefNum;
						pbRec.dirInfo.ioNamePtr = spec.name;
						if (PBGetCatInfoSync (&pbRec) == noErr)
						{
							spec.parID = pbRec.dirInfo.ioDrDirID;
							// the cfstring -> pascalstring can fail if the filename length > 63 (FSSpec sucks)
							if (CFStringGetPascalString (navReply.saveFileName, (unsigned char*)&spec.name, sizeof (spec.name), kCFStringEncodingASCII))
							{
								vstFileSelect->nbReturnPath = 1;
								if (!vstFileSelect->returnPath)
								{
									vstFileSelect->reserved = 1;
									vstFileSelect->returnPath = new char [sizeof (FSSpec)];
								}
								memcpy (vstFileSelect->returnPath, &spec, sizeof (FSSpec));
							}
						}
					}
					NavDisposeReply (&navReply);
				}
				if (defSaveName)
					CFRelease (defSaveName);
				NavDialogDispose (dialogRef);
				DisposeNavEventUPP (eventUPP);
				return vstFileSelect->nbReturnPath;
			}
			if (defSaveName)
				CFRelease (defSaveName);
		}
		else if (vstFileSelect->command == kVstDirectorySelect)
		{
			NavDialogCreationOptions dialogOptions;
			NavGetDefaultDialogCreationOptions (&dialogOptions);
			dialogOptions.windowTitle = CFSTR ("Select Directory");
			NavDialogRef dialogRef;
			if (NavCreateChooseFolderDialog (&dialogOptions, eventUPP, NULL, effect, &dialogRef) == noErr)
			{
			    AEDesc defaultLocation;   
			    AEDesc* defLocPtr = 0;   
				if (vstFileSelect->initialPath)
				{
					FSSpec* defaultSpec = (FSSpec*)vstFileSelect->initialPath;
			        if (defaultSpec->parID && defaultSpec->vRefNum)       
			            if (AECreateDesc (typeFSS, defaultSpec, sizeof(FSSpec), &defaultLocation) == noErr)
			                defLocPtr = &defaultLocation;
				}
		        if (defLocPtr)
		            NavCustomControl (dialogRef, kNavCtlSetLocation, (void*)defLocPtr);
				NavDialogRun (dialogRef);
				if (defLocPtr)
		            AEDisposeDesc (defLocPtr);
				NavReplyRecord navReply;
				if (NavDialogGetReply (dialogRef, &navReply) == noErr)
				{
					FSRef parentFSRef;
					AEKeyword theAEKeyword;
					DescType typeCode;
					Size actualSize;
			        // get the FSRef referring to the parent directory
				    if (AEGetNthPtr(&navReply.selection, 1, typeFSRef,
		        		&theAEKeyword, &typeCode, &parentFSRef, sizeof(FSRef), &actualSize) == noErr)
					{
						FSSpec spec;
						FSCatalogInfoBitmap infoBitmap = kFSCatInfoNone;
						FSGetCatalogInfo (&parentFSRef, infoBitmap, NULL, NULL, &spec, NULL);
						vstFileSelect->nbReturnPath = 1;
						if (!vstFileSelect->returnPath)
						{
							vstFileSelect->reserved = 1;
							vstFileSelect->returnPath = new char [sizeof (FSSpec)];
						}
						memcpy (vstFileSelect->returnPath, &spec, sizeof (FSSpec));
					}
					
					NavDisposeReply (&navReply);
				}
				NavDialogDispose (dialogRef);
				DisposeNavEventUPP (eventUPP);
				return vstFileSelect->nbReturnPath;
			}
		}
		else // FileLoad
		{
			NavDialogCreationOptions dialogOptions;
			NavGetDefaultDialogCreationOptions (&dialogOptions);
			if (vstFileSelect->command == kVstFileLoad)
			{
				dialogOptions.windowTitle = CFSTR ("Select a File to Open");
				dialogOptions.optionFlags &= ~kNavAllowMultipleFiles;
			}
			else
			{
				dialogOptions.windowTitle = CFSTR ("Select Files to Open");
				dialogOptions.optionFlags |= kNavAllowMultipleFiles;
			}
			NavDialogRef dialogRef;
			if (NavCreateGetFileDialog (&dialogOptions, NULL, eventUPP, NULL, NULL, effect, &dialogRef) == noErr)
			{
			    AEDesc defaultLocation;   
			    AEDesc* defLocPtr = 0;   
				if (vstFileSelect->initialPath)
				{
					FSSpec* defaultSpec = (FSSpec*)vstFileSelect->initialPath;
			        if (defaultSpec->parID && defaultSpec->vRefNum)       
			            if (AECreateDesc (typeFSS, defaultSpec, sizeof(FSSpec), &defaultLocation) == noErr)
			                defLocPtr = &defaultLocation;
				}
		        if (defLocPtr)
		            NavCustomControl (dialogRef, kNavCtlSetLocation, (void*)defLocPtr);

				NavDialogRun (dialogRef);

				if (defLocPtr)
		            AEDisposeDesc (defLocPtr);

				NavReplyRecord navReply;
				if (NavDialogGetReply (dialogRef, &navReply) == noErr)
				{
					FSRef parentFSRef;
					AEKeyword theAEKeyword;
					DescType typeCode;
					Size actualSize;
					if (vstFileSelect->command == kVstFileLoad)
					{
					    if (AEGetNthPtr(&navReply.selection, 1, typeFSRef,
			        		&theAEKeyword, &typeCode, &parentFSRef, sizeof(FSRef), &actualSize) == noErr)
						{
							FSSpec spec;
							FSCatalogInfoBitmap infoBitmap = kFSCatInfoNone;
							FSGetCatalogInfo (&parentFSRef, infoBitmap, NULL, NULL, &spec, NULL);
							vstFileSelect->nbReturnPath = 1;
							if (!vstFileSelect->returnPath)
							{
								vstFileSelect->reserved = 1;
								vstFileSelect->returnPath = new char [sizeof (FSSpec)];
							}
							memcpy (vstFileSelect->returnPath, &spec, sizeof (FSSpec));
						}
					}
					else
					{
						AECountItems (&navReply.selection, &vstFileSelect->nbReturnPath);
						vstFileSelect->returnMultiplePaths = new char* [vstFileSelect->nbReturnPath];
						int index = 1;
					    while (AEGetNthPtr(&navReply.selection, index++, typeFSRef,
			        		&theAEKeyword, &typeCode, &parentFSRef, sizeof(FSRef), &actualSize) == noErr)
						{
							FSSpec spec;
							FSCatalogInfoBitmap infoBitmap = kFSCatInfoNone;
							FSGetCatalogInfo (&parentFSRef, infoBitmap, NULL, NULL, &spec, NULL);
							vstFileSelect->returnMultiplePaths[index-2] = new char[sizeof (FSSpec)];
							memcpy (vstFileSelect->returnMultiplePaths[index-2], &spec, sizeof (FSSpec));
						}
					}
				}
				DisposeNavEventUPP (eventUPP);
				NavDialogDispose (dialogRef);
				return vstFileSelect->nbReturnPath;
			}
		}
		DisposeNavEventUPP (eventUPP);
#else
		StandardFileReply reply;
		if (vstFileSelect->command == kVstFileSave)
		{
			unsigned char defName[64];
			defName[0] = 0;
			StandardPutFile ("\pSelect a Destination", defName, &reply);
			if (reply.sfGood && reply.sfFile.name[0] != 0)
			{
				if (!vstFileSelect->returnPath)
				{
					vstFileSelect->reserved = 1;
					vstFileSelect->returnPath = new char [301];
				}
				memcpy (vstFileSelect->returnPath, &reply.sfFile, 300);
				vstFileSelect->nbReturnPath = 1;
				return 1;
			}
		}

		else if (vstFileSelect->command == kVstDirectorySelect) 
		{
		#if USENAVSERVICES
			if (NavServicesAvailable ())
			{
				NavReplyRecord navReply;
				NavDialogOptions dialogOptions;
				short ret = false;
				AEDesc defLoc;
				defLoc.descriptorType = typeFSS;
				defLoc.dataHandle = NewHandle (sizeof (FSSpec));
				FSSpec	finalFSSpec;
				finalFSSpec.parID   = 0;	// *dirID;
				finalFSSpec.vRefNum = 0;	// *volume;
				finalFSSpec.name[0] = 0;

				NavGetDefaultDialogOptions (&dialogOptions);
				dialogOptions.dialogOptionFlags &= ~kNavAllowMultipleFiles;
				dialogOptions.dialogOptionFlags |= kNavSelectDefaultLocation;
				strcpy ((char* )dialogOptions.message, "Select Directory");
				c2pstr ((char* )dialogOptions.message);
				NavChooseFolder (&defLoc, &navReply, &dialogOptions, 0 /* eventUPP */, 0, 0);
				DisposeHandle (defLoc.dataHandle);
				
				AEDesc 	resultDesc;	
				AEKeyword keyword;
				resultDesc.dataHandle = 0L;

				if (navReply.validRecord && AEGetNthDesc (&navReply.selection, 1, typeFSS, &keyword, &resultDesc) == noErr)
				{
					ret = true;
					vstFileSelect->nbReturnPath = 1;
					if (!vstFileSelect->returnPath)
					{
						vstFileSelect->reserved = 1;
						vstFileSelect->returnPath = new char [sizeof (FSSpec)];
					}
					memcpy (vstFileSelect->returnPath, *resultDesc.dataHandle, sizeof (FSSpec));
				}
				NavDisposeReply (&navReply);
				return vstFileSelect->nbReturnPath;
			}
			else
		#endif
			{
				// Can't select a Folder; the Application does not support it, and Navigational Services are not available...
				return 0;
			}
		}

		else
		{
			SFTypeList typelist;
			long numFileTypes = vstFileSelect->nbFileTypes;
			//seem not to work... if (numFileTypes <= 0)
			{
				numFileTypes = -1;	// all files
				typelist[0] = 'AIFF';
			}
			/*else
			{
				if (numFileTypes > 4)
					numFileTypes = 4;
				for (long i = 0; i < numFileTypes; i++)
					memcpy (&typelist[i], vstFileSelect->fileTypes[i].macType, 4);
			}*/
			StandardGetFile (0L, numFileTypes, typelist, &reply);
			if (reply.sfGood)
			{
				if (!vstFileSelect->returnPath)
				{
					vstFileSelect->reserved = 1;
					vstFileSelect->returnPath = new char [301];
				}
				memcpy (vstFileSelect->returnPath, &reply.sfFile, 300);
				vstFileSelect->nbReturnPath = 1;
				return 1;
			}
		}
#endif // CARBON
#else
		//CAlert::alert ("The current Host application doesn't support FileSelector !", "Warning");
#endif
	}
	return 0;
}

END_NAMESPACE_VSTGUI

#if WINDOWS
#include <Commdlg.h>
//-----------------------------------------------------------------------------
UINT APIENTRY SelectDirectoryHook (HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NOTIFY: 
	{
		OFNOTIFY *lpon = (OFNOTIFY *)lParam;
	
		switch (lpon->hdr.code)
		{
		case CDN_FILEOK:
			CommDlg_OpenSave_GetFolderPath (GetParent (hdlg), selDirPath, kPathMax);
			didCancel = false;
			break;
		
		case CDN_INITDONE: {
			#define HIDE_ITEMS 4
			int  i;
			UINT hide_items[HIDE_ITEMS] = {edt1, stc3, cmb1, stc2};	

			for (i = 0; i < HIDE_ITEMS; i++)
				CommDlg_OpenSave_HideControl (GetParent (hdlg), hide_items[i]);
			
			CommDlg_OpenSave_SetControlText (GetParent (hdlg), stc4, (char*)(const char*)stringLookIn);
			CommDlg_OpenSave_SetControlText (GetParent (hdlg), IDOK, (char*)(const char*)stringSelect);
			CommDlg_OpenSave_SetControlText (GetParent (hdlg), IDCANCEL, (char*)(const char*)stringCancel);
		} break;
		}
	} break;

	case WM_INITDIALOG:
		fpOldSelectDirectoryButtonProc = (FARPROC)SetWindowLong (
					GetDlgItem (GetParent (hdlg), IDOK), 
					GWL_WNDPROC, 
					(long) SelectDirectoryButtonProc);

		break;
		
	case WM_DESTROY:
		SetWindowLong (GetDlgItem (GetParent (hdlg), IDOK), 
				GWL_WNDPROC, (long) fpOldSelectDirectoryButtonProc);
	}
	return false;
}

//-----------------------------------------------------------------------------
LRESULT CALLBACK SelectDirectoryButtonProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SETTEXT: 
		if (! (strcmp ((char *)lParam, stringSelect) == 0))
			return false;
		break;
	
	case WM_LBUTTONUP:
	case WM_RBUTTONUP: {
		char mode[256];
		GetWindowText (hwnd, mode, 256);
		if (!strcmp (mode, stringSelect))
		{
			folderSelected = true;
			char oldDirPath[kPathMax];
			CommDlg_OpenSave_GetFolderPath (GetParent (hwnd), oldDirPath, kPathMax);
			// you need a lot of tricks to get name of currently selected folder:
			// the following call of the original windows procedure causes the
			// selected folder to open and after that you can retrieve its name
			// by calling ..._GetFolderPath (...)
			CallWindowProc ((WNDPROC)fpOldSelectDirectoryButtonProc, hwnd, message, wParam, lParam);
			CommDlg_OpenSave_GetFolderPath (GetParent (hwnd), selDirPath, kPathMax);

			if (1) // consumers like it like this
			{
				if (strcmp (oldDirPath, selDirPath) == 0 || selDirPath [0] == 0)
				{
					// the same folder as the old one, means nothing selected: close
					folderSelected = true;
					didCancel = false;
					PostMessage (GetParent (hwnd), WM_CLOSE, 0, 0);
					return false;
				}
				else
				{
					// another folder is selected: browse into it
					folderSelected = false;
					return true;
				}
			}
			else // original code
			{
				if (strcmp (oldDirPath, selDirPath) == 0 || selDirPath [0] == 0)
				{
					// the same folder as the old one, means nothing selected: stay open
					folderSelected = false;
					return true;
				}
			}
		}

		didCancel = false;
		PostMessage (GetParent (hwnd), WM_CLOSE, 0, 0); 
		return false;
	} break;
	} // end switch

	return CallWindowProc ((WNDPROC)fpOldSelectDirectoryButtonProc, hwnd, message, wParam, lParam);
}

//-----------------------------------------------------------------------------
static void showPathInWindowTitle (HWND hParent, LPOFNOTIFY lpon)
{
	#define WINDOWTEXTSIZE 260 + 64
	OPENFILENAME *ofn = lpon->lpOFN;
	char text[WINDOWTEXTSIZE];
	char *p;
	int  len;

	// Put the path into the Window Title
	if (lpon->lpOFN->lpstrTitle)
		strcpy (text, lpon->lpOFN->lpstrTitle);
	else
	{
		char *pp;

		GetWindowText (hParent, text, WINDOWTEXTSIZE);
		pp = strchr (text, '-');
		if (pp)
			*--pp = 0;
	}

	p = strcat (text, " - [");
	p = text;
	len = strlen (text); 
	p += len;
	len = WINDOWTEXTSIZE - len - 2;
	CommDlg_OpenSave_GetFolderPath (hParent, p, len);
	strcat (text, "]");
	SetWindowText (hParent, text);
}

//------------------------------------------------------------------------
UINT APIENTRY WinSaveHook (HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_NOTIFY: {
		LPOFNOTIFY lpon = (LPOFNOTIFY)lParam; 
		if (!lpon)
			break;

		switch (lpon->hdr.code)
		{
		case CDN_FOLDERCHANGE: 
			showPathInWindowTitle (GetParent (hdlg), lpon);
			break;
		}
	} break;
	} // end switch

	return 0;
}
#endif

//-----------------------------------------------------------------------------
#endif // !PLUGGUI
//-----------------------------------------------------------------------------

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
	useCount++;
	if (useCount == 1)
	{
		sprintf (className, "Plugin%08x", GetInstance ());
		
		WNDCLASS windowClass;
		windowClass.style = CS_GLOBALCLASS;//|CS_OWNDC; // add Private-DC constant 

		windowClass.lpfnWndProc = WindowProc; 
		windowClass.cbClsExtra = 0; 
		windowClass.cbWndExtra = 0; 
		windowClass.hInstance = GetInstance (); 
		windowClass.hIcon = 0; 

		windowClass.hCursor = LoadCursor (NULL, IDC_ARROW);
		windowClass.hbrBackground = GetSysColorBrush (COLOR_BTNFACE); 
		windowClass.lpszMenuName = 0; 
		windowClass.lpszClassName = className; 
		RegisterClass (&windowClass);

		#if USE_MOUSE_HOOK
		MouseHook = SetWindowsHookEx (WH_MOUSE, MouseProc,(GetInstance (), 0);
		#endif

		swapped_mouse_buttons = GetSystemMetrics (SM_SWAPBUTTON) > 0;
	}
	return true;
}

//-----------------------------------------------------------------------------
void ExitWindowClass ()
{
	useCount--;
	if (useCount == 0)
	{
		UnregisterClass (className, GetInstance ());

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
LONG WINAPI WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	USING_NAMESPACE_VSTGUI
	CFrame* pFrame = (CFrame*)GetWindowLong (hwnd, GWL_USERDATA);

	switch (message)
	{
	case WM_CTLCOLOREDIT:
	{
		if (pFrame)
		{
			VSTGUI_CTextEdit *textEdit = (VSTGUI_CTextEdit*)pFrame->getEditView ();
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
		RECT r;
		if (pFrame && GetUpdateRect (hwnd, &r, false))
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint (hwnd, &ps);

			VSTGUI_CDrawContext *pContext = new VSTGUI_CDrawContext (pFrame, hdc, hwnd);
			
			#if 1
			CRect updateRect (ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);
			pFrame->drawRect (pContext, updateRect);
			#else
			pFrame->draw (pContext);
			#endif
			delete pContext;

			EndPaint (hwnd, &ps);
			return 0;
		}
	}
	break;

	case WM_MEASUREITEM :
	{
		MEASUREITEMSTRUCT* ms = (MEASUREITEMSTRUCT*)lParam;
		if (pFrame && ms && ms->CtlType == ODT_MENU && ms->itemData)
		{
			VSTGUI_COptionMenu* optMenu = (VSTGUI_COptionMenu*)pFrame->getEditView ();
			if (optMenu && optMenu->getScheme ())
			{
				VSTGUI_CPoint size;

				HDC hdc = GetDC (hwnd);
				VSTGUI_CDrawContext* pContext = new VSTGUI_CDrawContext (pFrame, hdc, hwnd);
				optMenu->getScheme ()->getItemSize ((const char*)ms->itemData, pContext, size);
				delete pContext;
				ReleaseDC (hwnd, hdc);

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
			VSTGUI_COptionMenu* optMenu = (VSTGUI_COptionMenu*)pFrame->getEditView ();
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
	
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONDOWN:
		if (pFrame)
		{
		#if 1
			HDC hdc = GetDC (hwnd);
			VSTGUI_CDrawContext *pContext = new VSTGUI_CDrawContext (pFrame, hdc, hwnd);
			VSTGUI_CPoint where (LOWORD (lParam), HIWORD (lParam));
			pFrame->mouse (pContext, where);
			delete pContext;
			ReleaseDC (hwnd, hdc);
		#else
			VSTGUI_CPoint where (LOWORD (lParam), HIWORD (lParam));
			pFrame->mouse ((VSTGUI_CDrawContext*)0, where);
		#endif

			return 0;
		}
		break;
		
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
		HDC		hdcSystemContext = (HDC)pContext->getSystemContext();
		HDC		hdcMask = CreateCompatibleDC (hdcSystemContext);
		HDC		hdcMem = CreateCompatibleDC (hdcSystemContext);
		HBITMAP	bmAndMem;
		HBITMAP	bmMemOld, bmMaskOld;

		long	x, y;
		long	width = rect.width();
		long	height = rect.height();

		x = rect.x + pContext->offset.x;
		y = rect.y + pContext->offset.y;

		bmAndMem = CreateCompatibleBitmap(hdcSystemContext, width, height);

		bmMaskOld   = (HBITMAP)SelectObject(hdcMask, pMask);
		bmMemOld    = (HBITMAP)SelectObject (hdcMem, bmAndMem);

		BitBlt(hdcMem, 0, 0, width, height, hdcSystemContext, x, y, SRCCOPY);
		BitBlt(hdcMem, 0, 0, width, height, hdcBitmap, offset.x, offset.y, SRCINVERT);
		BitBlt(hdcMem, 0, 0, width, height, hdcMask, offset.x, offset.y, SRCAND);
		BitBlt(hdcMem, 0, 0, width, height, hdcBitmap, offset.x, offset.y, SRCINVERT);
		BitBlt(hdcSystemContext, x, y, width, height, hdcMem, 0, 0, SRCCOPY);

		DeleteObject(SelectObject(hdcMem, bmMemOld));
		SelectObject(hdcMask, bmMaskOld);

		DeleteDC(hdcMem);
		DeleteDC(hdcMask);
	}
}
#endif

//-----------------------------------------------------------------------------
#if MAC || MOTIF || BEOS
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
#endif


//-----------------------------------------------------------------------------
#if MOTIF
XRectangle rect;
static bool first = true;

//-----------------------------------------------------------------------------
void _destroyCallback (Widget widget, XtPointer clientData, XtPointer callData)
{
	CFrame* pFrame = (CFrame*)clientData;
	if (pFrame)
	{
		pFrame->freeGc ();
		pFrame->setOpenFlag (false);
		pFrame->pSystemWindow = 0;
	}
}

//-----------------------------------------------------------------------------
void _drawingAreaCallback (Widget widget, XtPointer clientData, XtPointer callData)
{
	CFrame* pFrame = (CFrame*)clientData;
	XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct *)callData;
	XEvent *event = cbs->event;

	//-------------------------------------
	if (cbs->reason == XmCR_INPUT)
	{
		if (event->xbutton.type == ButtonRelease)
			return;

		if (event->xbutton.type != ButtonPress &&
				event->xbutton.type != KeyPress)
			return;

		Window pWindow = pFrame->getWindow ();
		CDrawContext context (pFrame, pFrame->getGC (), (void*)pWindow);

		CPoint where (event->xbutton.x, event->xbutton.y);
		pFrame->mouse (&context, where);
	}
	//------------------------------------
	else if (cbs->reason == XmCR_EXPOSE)
	{
		XExposeEvent *expose = (XExposeEvent*)event;
#if TEST_REGION
		rect.x      = expose->x;
		rect.y      = expose->y;
		rect.width  = expose->width;
		rect.height = expose->height;
		if (first)
		{
			pFrame->region = XCreateRegion ();
			first = false;
		}

		XUnionRectWithRegion (&rect, pFrame->region, pFrame->region);
#endif
		if (expose->count == 0)
		{
#if TEST_REGION
			XSetRegion (expose->pDisplay, pFrame->getGC (), pFrame->region);

			// add processus of static first to set the region to max after a total draw and destroy it the first time...
#endif
			pFrame->draw ();

#if TEST_REGION
			rect.x      = 0;
			rect.y      = 0;
			rect.width  = pFrame->getWidth ();
			rect.height = pFrame->getHeight ();
			XUnionRectWithRegion (&rect, pFrame->region, pFrame->region);
			XSetRegion (expose->pDisplay, pFrame->getGC (), pFrame->region);
			XDestroyRegion (pFrame->region);
			first = true;
#endif
		}
	}
}

//-----------------------------------------------------------------------------
void _eventHandler (Widget w, XtPointer clientData, XEvent *event, char *p)
{
	switch (event->type)
	{
	case EnterNotify:
		break;

	case LeaveNotify:
		XCrossingEvent *xevent = (XCrossingEvent*)event;
		
		CFrame* pFrame = (CFrame*)clientData;
		if (pFrame && pFrame->getEditView ())
		{
			if (xevent->x < 0 || xevent->x >= pFrame->getWidth () ||
					xevent->y < 0 || xevent->y >= pFrame->getHeight ())
			{
				// if button pressed => don't defocus
				if (xevent->state & (Button1Mask|Button2Mask|Button3Mask))
					break;
				pFrame->getEditView ()->looseFocus ();
				pFrame->setEditView (0);
			}
		}
		break;
	}
}

//-----------------------------------------------------------------------------
long getIndexColor8Bit (CColor color, Display *pDisplay, Colormap colormap)
{
	long i;

	// search in pre-loaded color
	for (i = 0; i < CDrawContext::nbNewColor; i++)
	{
		if ((paletteNewColor[i].red   == color.red)   &&
				(paletteNewColor[i].green == color.green) &&
				(paletteNewColor[i].blue  == color.blue))
			return paletteNewColor[i].unused;
	}
	
	// Allocate new color cell
	XColor xcolor;
	int red   = color.red   << 8;
	int green = color.green << 8;
	int blue  = color.blue  << 8;
	xcolor.red   = red;
	xcolor.green = green;
	xcolor.blue  = blue;	
	if (XAllocColor (pDisplay, colormap, &xcolor))
	{
		// store this new color
		if (CDrawContext::nbNewColor < 255) 
		{
			paletteNewColor[CDrawContext::nbNewColor].red    = color.red;
			paletteNewColor[CDrawContext::nbNewColor].green  = color.green;
			paletteNewColor[CDrawContext::nbNewColor].blue   = color.blue;
			paletteNewColor[CDrawContext::nbNewColor].unused = xcolor.pixel;
			CDrawContext::nbNewColor++;
		}
		return xcolor.pixel;
	}
		
	// take the nearest color
	int diff;
	int min = 3 * 65536;
	int index = 0;

	XColor xcolors[256];
	for (i = 0; i < 256; i++)
		xcolors[i].pixel = i;

	XQueryColors (pDisplay, colormap, xcolors, 256);

	for (i = 0; i < 256; i++)
	{
		diff = fabs (xcolors[i].red - red) + fabs (xcolors[i].green - green) + fabs (xcolors[i].blue - blue);
		if (diff < min)
		{
			min = diff;
			index = i;
		}
	}

	// store this new color
	if (CDrawContext::nbNewColor < 255)
	{
		paletteNewColor[CDrawContext::nbNewColor].red    = color.red;
		paletteNewColor[CDrawContext::nbNewColor].green  = color.green;
		paletteNewColor[CDrawContext::nbNewColor].blue   = color.blue;
		paletteNewColor[CDrawContext::nbNewColor].unused = index;
		CDrawContext::nbNewColor++;
	}
	return (index);
}

//-----------------------------------------------------------------------------
bool xpmGetValues (char **ppDataXpm, long *pWidth, long *pHeight, long *pNcolor, long *pCpp)
{
	// get the size of the pixmap
	sscanf (ppDataXpm[0], "%d %d %d %d", pWidth, pHeight, pNcolor, pCpp);
	
	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#elif BEOS
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
class UDropTarget : public IDropTarget
{	
public:
	UDropTarget (VSTGUI_CFrame* pFrame);
	virtual ~UDropTarget ();

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
// UDropTarget
//-----------------------------------------------------------------------------
void* createDropTarget (VSTGUI_CFrame* pFrame)
{
	return new UDropTarget (pFrame);
}

//-----------------------------------------------------------------------------
UDropTarget::UDropTarget (VSTGUI_CFrame* pFrame)
: refCount (0), pFrame (pFrame)
{
}

//-----------------------------------------------------------------------------
UDropTarget::~UDropTarget ()
{
}

//-----------------------------------------------------------------------------
STDMETHODIMP UDropTarget::QueryInterface (REFIID riid, void** object)
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
STDMETHODIMP_(ULONG) UDropTarget::AddRef (void)
{
	return ++refCount;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) UDropTarget::Release (void)
{
	refCount--;
	if (refCount <= 0)
		delete this;
	return refCount;
}

//-----------------------------------------------------------------------------
STDMETHODIMP UDropTarget::DragEnter (IDataObject *dataObject, DWORD keyState, POINTL pt, DWORD *effect)
{
	accept = false;
	if (dataObject)
	{
		FORMATETC formatTEXTDrop = {CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		if (S_OK == dataObject->QueryGetData (&formatTEXTDrop))
		{
			accept = true;
			return DragOver (keyState, pt, effect);
		}

		FORMATETC formatHDrop = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		if (S_OK == dataObject->QueryGetData (&formatHDrop))
		{
			accept = true;
			return DragOver (keyState, pt, effect);
		}
	}

	*effect = DROPEFFECT_NONE;
	return S_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP UDropTarget::DragOver (DWORD keyState, POINTL pt, DWORD *effect)
{
	if (accept)
	{
		if (keyState & MK_CONTROL)
			*effect = DROPEFFECT_COPY;
		else
			*effect = DROPEFFECT_MOVE;
	}
	else
		*effect = DROPEFFECT_NONE;
	return S_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP UDropTarget::DragLeave (void)
{
	return S_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP UDropTarget::Drop (IDataObject *dataObject, DWORD keyState, POINTL pt, DWORD *effect)
{
	if (pFrame)
	{
		void* hDrop = 0;
		STGMEDIUM medium;
		FORMATETC formatTEXTDrop = {CF_TEXT,  0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		FORMATETC formatHDrop    = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

		long type = 0; // 0 = file, 1 = text

		HRESULT hr = dataObject->GetData (&formatTEXTDrop, &medium);
		if (hr != S_OK)
			hr = dataObject->GetData (&formatHDrop, &medium);
		else
			type = 1;
		if (hr == S_OK)
			hDrop = medium.hGlobal;

		if (hDrop)
		{
			switch (type)
			{
			//---File----------------------
			case 0:
				{
					long nbOfItems = (long)DragQueryFile ((HDROP)hDrop, 0xFFFFFFFFL, 0, 0);
					char fileDropped[1024];

					if (nbOfItems > 0) 
					{
						char **ptrItems = new char* [nbOfItems];
						long itemIndex = 0;
						long nbRealItems = 0;
						while (itemIndex < nbOfItems)
						{
							if (DragQueryFile ((HDROP)hDrop, itemIndex, fileDropped, sizeof (fileDropped))) 
							{
								// resolve link
								checkResolveLink (fileDropped, fileDropped);

								ptrItems[nbRealItems] = new char [sizeof (fileDropped)];
								strcpy ((char*)ptrItems[nbRealItems], fileDropped);
								nbRealItems++;
							}

							itemIndex++;
						}
						VSTGUI_CPoint where;
						pFrame->getCurrentLocation (where);
						pFrame->onDrop ((void**)ptrItems, nbOfItems, VSTGUI_kDropFiles, where);
						for (long i = 0; i < nbRealItems; i++)
							delete []ptrItems[i];
						delete []ptrItems;
					}
				} break;

			//---TEXT----------------------------
			case 1:
				{
					void* data = GlobalLock (medium.hGlobal);
					long dataSize = GlobalSize (medium.hGlobal);
					if (data && dataSize)
					{
						VSTGUI_CPoint where;
						pFrame->getCurrentLocation (where);
						pFrame->onDrop ((void**)&data, dataSize, VSTGUI_kDropText, where);
					}

					GlobalUnlock (medium.hGlobal);
					if (medium.pUnkForRelease)
						medium.pUnkForRelease->Release ();
					else
						GlobalFree (medium.hGlobal);
				} break;
			}
		}
	}
	
	DragLeave ();
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
				MultiByteToWideChar (CP_ACP, 0, nativePath, -1, wsz, 2048);
				// Load the shell link.
				hres = ppf->Load (wsz, STGM_READ);
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
//-----------------------------------------------------------------------------
// Drop Implementation
//-----------------------------------------------------------------------------
#if !MACX
#include "Drag.h"
#endif

pascal short drag_receiver (WindowPtr w, void* ref, DragReference drag);

static DragReceiveHandlerUPP drh;

//-------------------------------------------------------------------------------------------
void install_drop (CFrame *frame)
{
	drh = NewDragReceiveHandlerUPP (drag_receiver);
#if CARBON
	InstallReceiveHandler (drh, (WindowRef)(frame->getSystemWindow ()), (void*)frame);
#else
	InstallReceiveHandler (drh, (GrafPort*)(frame->getSystemWindow ()), (void*)frame);
#endif
}

//-------------------------------------------------------------------------------------------
void remove_drop (CFrame *frame)
{
#if CARBON
	RemoveReceiveHandler (drh, (WindowRef)(frame->getSystemWindow ()));
#else
	RemoveReceiveHandler (drh, (GrafPort*)(frame->getSystemWindow ()));
#endif
}

//-------------------------------------------------------------------------------------------
// Drop has happened in one of our's windows.
// The data is either of our own type (flavour type stCA), or comes from
// another app. The only data from outside that is currently accepted are
// HFS-files
//-------------------------------------------------------------------------------------------
pascal short drag_receiver (WindowPtr w, void* ref, DragReference drag)
{
	unsigned short i, items;
	ItemReference item;
	long          size;
	HFSFlavor     hfs;
	void*         pack;

	// get num of items
	CountDragItems (drag, &items);
	if (items <= 0)
		return cantGetFlavorErr;
			
	char **ptrItems = new char* [items];
	long nbFileItems = 0;
	CFrame *pFrame = (CFrame*)ref;
	char* string = 0;
		
	// for each items
	for (i = 1; i <= items; i++)
	{
		pack = NULL;
	
		GetDragItemReferenceNumber (drag, i, &item);

		//---try file--------------------------
		if (GetFlavorDataSize (drag, item, flavorTypeHFS, &size) == noErr)
		{ 
			GetFlavorData (drag, item, flavorTypeHFS, &hfs, &size, 0L);
			
			ptrItems[nbFileItems] = new char [sizeof (FSSpec)];
			memcpy (ptrItems[nbFileItems], &hfs.fileSpec, sizeof (FSSpec));
			nbFileItems++;
		}
		
		//---try Text-------------------------
		else if (GetFlavorDataSize (drag, item, 'TEXT', &size) == noErr)
		{
			string = new char [size + 2];
			if (string)
			{
				GetFlavorData (drag, item, 'TEXT', string, &size, 0);
				string[size] = 0;
			}
			break;
		}
		
		//---try XML text----------------------
		else if (GetFlavorDataSize (drag, item, 'XML ', &size) == noErr)
		{
			string = new char [size + 2];
			if (string)
			{
				GetFlavorData (drag, item, 'XML ', string, &size, 0);
				string[size] = 0;
			}
			break;
		}
	} // end for eac items
	
	// call the frame
	if (nbFileItems)
	{
		VSTGUI_CPoint where;
		pFrame->getCurrentLocation (where);
		pFrame->onDrop ((void**)ptrItems, nbFileItems, VSTGUI_kDropFiles, where);
		for (long i = 0; i < nbFileItems; i++)
				delete []ptrItems[i];
		delete []ptrItems;
		return noErr;
	}
	if (string)
	{
		VSTGUI_CPoint where;
		pFrame->getCurrentLocation (where);
		pFrame->onDrop ((void**)&string, size, VSTGUI_kDropText, where);
				
		delete []string;
	}
	
	
	delete []ptrItems;
	return cantGetFlavorErr;
}

#endif

