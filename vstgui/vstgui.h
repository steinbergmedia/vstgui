//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 2.2       Date : 25/03/03
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
#define __vstgui__

// define global defines
#if WIN32
	#define WINDOWS 1
#elif SGI | SUN
	#define MOTIF 1
#elif __MWERKS__
	#define MAC 1
#elif __BEOS__
	#define BEOS 1
#endif

#if WINDOWS
 #define USE_NAMESPACE 1
#endif

#if USE_NAMESPACE
 #define BEGIN_NAMESPACE_VSTGUI  namespace VSTGUI {
 #define END_NAMESPACE_VSTGUI    }
 #define USING_NAMESPACE_VSTGUI using namespace VSTGUI;
#else
 #define BEGIN_NAMESPACE_VSTGUI
 #define END_NAMESPACE_VSTGUI
 #define USING_NAMESPACE_VSTGUI
#endif

// VSTGUI Version
#define VSTGUI_VERSION_MAJOR  2
#define VSTGUI_VERSION_MINOR  2

//----------------------------------------------------
//----------------------------------------------------
BEGIN_NAMESPACE_VSTGUI

class CFrame;
class CDrawContext;
class COffscreenContext;
class CControl;
class CBitmap;

END_NAMESPACE_VSTGUI

#if PLUGGUI
	#ifndef __plugguieditor__
	#include "plugguieditor.h"
	#endif
#else
	#ifndef __aeffguieditor__
	#include "aeffguieditor.h"
	#endif
#endif

//----------------------------------------------------
#if WINDOWS
	#include <windows.h>

//----------------------------------------------------
#elif MOTIF
	#include <X11/Xlib.h>
	#include <X11/Intrinsic.h>
	#ifdef NOBOOL
		#ifndef bool
			typedef short bool;
		#endif
		#ifndef false
			static const bool false = 0; 
		#endif
		#ifndef true
			static const bool true = 1;
		#endif
	#endif

	// definition of struct for XPixmap resources
	struct CResTableEntry {
		int id;
  		char **xpm;
	};

	typedef CResTableEntry CResTable[];
	extern CResTable xpmResources;

//----------------------------------------------------
#elif MAC
	#if MACX
	#include <Carbon/Carbon.h>
	//macho VST's set gBundleRef which is a CFBundleRef
	extern void* gBundleRef;
	#else
	#include <Quickdraw.h>
	#include <Menus.h>
	#include <Windows.h>
	#include <TextUtils.h>
	#include <TextEdit.h>
	#include <ToolUtils.h>
	#include <Resources.h>
	#include <Dialogs.h>
	#endif
//----------------------------------------------------
#elif BEOS
	#include <Font.h>
	class BView;
	class PlugView;
	class BBitmap;
	class BResources;
#endif

struct VstKeyCode;

BEGIN_NAMESPACE_VSTGUI

struct CPoint;

//-----------------------------------------------------------------------------
// Structure CRect
//-----------------------------------------------------------------------------
struct CRect
{
	CRect (long left = 0, long top = 0, long right = 0, long bottom = 0)
	:	left (left), top (top), right (right), bottom (bottom) {}
	CRect (const CRect& r)
	:	left (r.left), top (r.top), right (r.right), bottom (r.bottom) {}
	CRect& operator () (long left, long top, long right, long bottom)
	{
		if (left < right)
			this->left = left, this->right = right;
		else
			this->left = right, this->right = left;
		if (top < bottom)
			this->top = top, this->bottom = bottom;
		else
			this->top = bottom, this->bottom = top;
		return *this;
	}

	bool operator != (const CRect& other) const
	{ return (left != other.left || right != other.right ||
				top != other.top || bottom != other.bottom); }

	bool operator == (const CRect& other) const
	{ return (left == other.left && right == other.right &&
				top == other.top && bottom == other.bottom); }
	
	inline long width () const  { return right - left; }
	inline long height () const { return bottom - top; }

	CRect &offset (long x, long y)
	{ left += x; right += x; top += y; bottom += y; return *this; }

	CRect &inset (long deltaX, long deltaY)
	{ left += deltaX; right -= deltaX; top += deltaY; bottom -= deltaY;
    return *this; }

	CRect &moveTo (long x, long y)
	{ long vDiff = y - top; long hDiff = x - left; 
	top += vDiff; bottom += vDiff; left += hDiff; right += hDiff;
	return *this; }

	bool pointInside (const CPoint& where) const;	// Checks if point is inside this rect
	bool isEmpty () const;

	bool rectOverlap (const CRect& rect) const
	{
		if (right < rect.left) return false;
		if (left > rect.right) return false;
		if (bottom < rect.top) return false;
		if (top > rect.bottom) return false;
		return true;
	}

	void bound (const CRect& rect);

	union
	{ long left; long x;};

	union
	{ long top; long y;};

	union
	{ long right; long x2;};

	union
	{ long bottom; long y2;};
};

//-----------------------------------------------------------------------------
// Structure CPoint
//-----------------------------------------------------------------------------
struct CPoint
{
	CPoint (long h = 0, long v = 0) : h (h), v (v) {}
	CPoint& operator () (long h, long v) 
	{ this->h = h; this->v = v; return *this; }

	bool isInside (CRect& r) const
	{ return h >= r.left && h <= r.right && v >= r.top && v <= r.bottom; } 

	bool operator != (const CPoint &other) const
	{ return (h != other.h || v != other.v); }

	bool operator == (const CPoint &other) const
	{ return (h == other.h && v == other.v); }

	CPoint &offset (long h, long v)
	{ this->h += h; this->v += v; return *this; }

	union
	{ long h; long x;};

	union
	{ long v; long y;};
};

//-----------------------------------------------------------------------------
// Structure CColor
//-----------------------------------------------------------------------------
struct CColor
{
	CColor& operator () (unsigned char red,
						unsigned char green,
						unsigned char blue,
						unsigned char unused)
	{
		this->red   = red;
		this->green = green;
		this->blue  = blue;
		this->unused = unused;
		return *this; 
	}

	CColor& operator = (CColor newColor)
	{
		red   = newColor.red;
		green = newColor.green;
		blue  = newColor.blue;
		unused = newColor.unused;
		return *this; 
	}
	
	CColor operator ~ ()
	{
		CColor c;
		c.red   = ~red;
		c.green = ~green;
		c.blue  = ~blue;
		c.unused = ~unused;
		return c;
	}

	bool operator != (const CColor &other) const 
	{ return (red != other.red || green != other.green || blue  != other.blue); }

	bool operator == (const CColor &other) const
	{ return (red == other.red && green == other.green && blue  == other.blue); }
	
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char unused;
};

// define some basic colors
extern CColor kTransparentCColor;
extern CColor kBlackCColor;
extern CColor kWhiteCColor;
extern CColor kGreyCColor;
extern CColor kRedCColor;
extern CColor kGreenCColor;
extern CColor kBlueCColor;
extern CColor kYellowCColor;
extern CColor kCyanCColor;
extern CColor kMagentaCColor;


//-----------------------------------------------------------------------------
// Definitions of special characters in a platform independent way

#if WINDOWS || MOTIF
#define kDegreeSymbol      "\xB0"
#define kInfiniteSymbol    "oo"
#define kCopyrightSymbol   "\xA9"
#define kTrademarkSymbol   "\x99"
#define kRegisteredSymbol  "\xAE"
#define kMicroSymbol       "\x85"
#define kPerthousandSymbol "\x89"

#elif BEOS
#define kDegreeSymbol      "\xC2\xB0"
#define kInfiniteSymbol    "\xE2\x88\x9E"
#define kCopyrightSymbol   "\xC2\xA9"
#define kTrademarkSymbol   "\xE2\x84\xA2"
#define kRegisteredSymbol	 "\xC2\xAE"
#define kMicroSymbol       "\xC2\xB5"
#define kPerthousandSymbol "\xE2\x80\xB0"

#elif MAC
#define kDegreeSymbol      "\xA1"
#define kInfiniteSymbol    "\xB0"
#define kCopyrightSymbol   "\xA9"
#define kTrademarkSymbol   "\xAA"
#define kRegisteredSymbol  "\xA8"
#define kMicroSymbol       "\xB5"
#define kPerthousandSymbol "\xE4"
#endif

//-----------------------------------------------------------------------------
//-----------
// Font Type
//-----------
enum CFont
{
	kSystemFont = 0,
	kNormalFontVeryBig,
	kNormalFontBig,
	kNormalFont,
	kNormalFontSmall,
	kNormalFontSmaller,
	kNormalFontVerySmall,
	kSymbolFont,

	kNumStandardFonts
};

//-----------
// Text Face
//-----------
enum CTxtFace
{
	kNormalFace    = 0,
	kBoldFace      = 1,
	kItalicFace    = 2,
	kUnderlineFace = 4
};

//-----------
// Line Style
//-----------
enum CLineStyle
{
	kLineSolid = 0,
	kLineOnOffDash
};

//-----------
// Draw Mode
//-----------
enum CDrawMode
{
	kCopyMode = 0,
	kOrMode,
	kXorMode
};

//----------------------------
// Text Alignment (Horizontal)
//----------------------------
enum CHoriTxtAlign
{
	kLeftText = 0,
	kCenterText,
	kRightText
};

//----------------------------
// Buttons Type (+modifiers)
//----------------------------
enum CButton
{
	kLButton =  1,
	kMButton =  2,
	kRButton =  4,
	kShift   =  8,
	kControl = 16,
	kAlt     = 32,
	kApple   = 64
};

//----------------------------
// Drop Type
//----------------------------
enum CDropType
{
	kDropFiles = 0,
	kDropText,

	kDropUser = 1000
};

//----------------------------
// Cursor Type
//----------------------------
enum CCursorType
{
	kCursorDefault = 0,
	kCursorWait,
	kCursorHSize,
	kCursorVSize,
	kCursorSizeAll,
	kCursorNESWSize,
	kCursorNWSESize
};

//----------------------------
// Knob Mode
//----------------------------
enum CKnobMode
{
	kCircularMode = 0,
	kRelativCircularMode,
	kLinearMode
};

//-----------------------------------------------------------------------------
// CDrawContext Declaration
//-----------------------------------------------------------------------------
class CDrawContext
{
public:
	CDrawContext (CFrame *pFrame, void *pSystemContext, void *pWindow = 0);
	virtual ~CDrawContext ();	

	void moveTo (const CPoint &point);
	void lineTo (const CPoint &point);

	void polyLine (const CPoint *pPoint, long numberOfPoints);
	void fillPolygon (const CPoint *pPoint, long numberOfPoints);

	void drawRect (const CRect &rect);
	void fillRect (const CRect &rect);

	void drawArc (const CRect &rect, const CPoint &point1, const CPoint &point2);
	void fillArc (const CRect &rect, const CPoint &point1, const CPoint &point2);

	void drawEllipse (const CRect &rect);
	void fillEllipse (const CRect &rect);
	
	void drawPoint (const CPoint &point, CColor color);
	CColor getPoint (const CPoint& point);

	void floodFill (const CPoint& start);
	
	void       setLineStyle (CLineStyle style);
	CLineStyle getLineStyle () { return lineStyle; }

	void   setLineWidth (long width);
	long   getLineWidth () { return frameWidth; }

	void      setDrawMode (CDrawMode mode);
	CDrawMode getDrawMode () { return drawMode; }

	void   setClipRect (const CRect &clip);
	CRect &getClipRect (CRect &clip) { clip = clipRect; return clip; }
	void   resetClipRect ();

	void   setFillColor  (const CColor color);
	CColor getFillColor () { return fillColor; }

	void   setFrameColor (const CColor color);
	CColor getFrameColor () { return frameColor; }

	void   setFontColor (const CColor color);
	CColor getFontColor () { return fontColor; }
	void   setFont (CFont fontID, const long size = 0, long style = 0);
	CFont  getFont () { return fontId; }

	long getStringWidth (const char* pStr);

	void drawString (const char *pString, const CRect &rect, const short opaque = false,
					 const CHoriTxtAlign hAlign = kCenterText);

	long getMouseButtons ();
	void getMouseLocation (CPoint &point);
	bool waitDoubleClick ();
	bool waitDrag ();

#if MOTIF
	long getIndexColor (CColor color);
	Colormap getColormap ();
	Visual   *getVisual ();
	unsigned int getDepth ();

	static long nbNewColor;
#endif

	void *getWindow () { return pWindow; }
	void setWindow (void *ptr)  { pWindow = ptr; }
	void getLoc (CPoint &where) { where = penLoc; }
	CFrame* getFrame () { return pFrame; }

	CPoint offsetScreen;
	CPoint offset;

	void   *getSystemContext () { return pSystemContext; }

	//-------------------------------------------
protected:

	friend class CBitmap;
	friend class COffscreenContext;

	void   *pSystemContext;
	void   *pWindow;
	CFrame *pFrame;

	long   fontSize;
	CFont  fontId;
	CColor fontColor;
	CPoint penLoc;

	long   frameWidth;
	CColor frameColor;
	CColor fillColor;
	CLineStyle lineStyle;
	CDrawMode  drawMode;
	CRect  clipRect;

#if WINDOWS
	void *pBrush;
	void *pPen;
	void *pFont;
	void *pOldBrush;
	void *pOldPen;
	void *pOldFont;
	long iPenStyle;

#elif MAC
	FontInfo fontInfoStruct;
	Pattern fillPattern;
	bool bInitialized;
	virtual BitMapPtr getBitmap ();
	virtual void releaseBitmap ();
	virtual CGrafPtr getPort ();
	
#elif MOTIF
	Display *pDisplay;

	XFontStruct *pFontInfoStruct;

#elif BEOS
	BView*	pView;
	BFont	font;
	void lineFromTo (CPoint& cstart, CPoint& cend);

#endif
};


//-----------------------------------------------------------------------------
// COffscreenContext Declaration
//-----------------------------------------------------------------------------
class COffscreenContext : public CDrawContext
{
public:
	COffscreenContext (CDrawContext *pContext, CBitmap *pBitmap, bool drawInBitmap = false);
	COffscreenContext (CFrame *pFrame, long width, long height, const CColor backgroundColor = kBlackCColor);

	virtual ~COffscreenContext ();
	
	void copyFrom (CDrawContext *pContext, CRect destRect, CPoint srcOffset = CPoint (0, 0));
	void copyTo (CDrawContext* pContext, CRect& srcRect, CPoint destOffset = CPoint (0, 0));

	inline long getWidth ()  { return width; }
	inline long getHeight () { return height; }

	//-------------------------------------------
protected:
	CBitmap *pBitmap;
	CBitmap *pBitmapBg;
	long    height;
	long    width;
	bool    bDestroyPixmap;

	CColor  backgroundColor;

#if WINDOWS
	void* oldBitmap;

#elif MOTIF
	Display *pXdisplay;

#elif BEOS
	BBitmap *offscreenBitmap;

#elif MAC
	BitMapPtr getBitmap ();
	void releaseBitmap ();
	CGrafPtr getPort ();
#endif
};


//-----------------------------------------------------------------------------
// CBitmap Declaration
//-----------------------------------------------------------------------------
class CBitmap
{
public:
	CBitmap (long resourceID);
	CBitmap (CFrame &frame, long width, long height);
	~CBitmap ();

	void draw (CDrawContext *pContext, CRect &rect, const CPoint &offset = CPoint (0, 0));
	void drawTransparent (CDrawContext *pContext, CRect &rect, const CPoint &offset = CPoint (0, 0));
	void drawAlphaBlend  (CDrawContext *pContext, CRect &rect, const CPoint &offset = CPoint (0, 0), unsigned char alpha = 128);

	inline long getWidth ()  { return width; }
	inline long getHeight () { return height; }

	void forget ();
	void remember ();
	long getNbReference () { return nbReference; }

	bool isLoaded ();
	void *getHandle ();
	
	void setTransparentColor (const CColor color);
	CColor getTransparentColor () { return transparentCColor; }
	void setTransparencyMask (CDrawContext* pContext, const CPoint& offset = CPoint (0, 0));

#if BEOS
	static void closeResource ();
#endif

	//-------------------------------------------
protected:
	long resourceID;
	long nbReference;
	long width;
	long height;

	CColor transparentCColor;

#if WINDOWS
	void *pHandle;
	void *pMask;

#elif MOTIF
	void *createPixmapFromXpm (CDrawContext *pContext);

	char    **ppDataXpm;
	Display *pXdisplay;
	void *pHandle;
	void *pMask;

#elif MAC
	void* pHandle;
	void* pMask;

#elif BEOS
	static BResources *resourceFile;
	BBitmap    *bbitmap;
	bool		transparencySet;
#endif
};

enum {
	kMessageUnknown = 0,
	kMessageNotified = 1
};

//-----------------------------------------------------------------------------
// CView Declaration
//-----------------------------------------------------------------------------
class CView
{
public:
	CView (const CRect &size);
	virtual ~CView ();

	void redraw ();
	virtual void draw (CDrawContext *pContext);
	virtual void drawRect (CDrawContext *pContext, CRect& updateRect) { draw (pContext); }
	virtual bool checkUpdate (CRect& updateRect) { return updateRect.rectOverlap (size); }
	virtual void mouse (CDrawContext *pContext, CPoint &where);
	virtual void update (CDrawContext *pContext);
	virtual long notify (CView* sender, const char* message);
	
	virtual long onKeyDown (VstKeyCode& keyCode);
	virtual long onKeyUp (VstKeyCode& keyCode);

	virtual bool onDrop (void **ptrItems, long nbItems, long type, CPoint &where);
	virtual bool onWheel (CDrawContext *pContext, const CPoint &where, float distance);

	virtual void looseFocus (CDrawContext *pContext = 0);
	virtual void takeFocus (CDrawContext *pContext = 0);

	virtual bool isDirty () { return bDirty; }
	virtual void setDirty (const bool val = true) { bDirty = val; }

	virtual void setMouseEnabled (const bool bEnable = true) { bMouseEnabled = bEnable; }
	virtual bool getMouseEnabled () { return bMouseEnabled; }

	virtual void setMouseableArea (const CRect &rect)  { mouseableArea = rect; }
	virtual CRect &getMouseableArea (CRect &rect) { rect = mouseableArea; return rect;}

	virtual bool hitTest (const CPoint& where, const long buttons = -1) { return where.isInside (size); }

	virtual void setTransparency (bool val) { bTransparencyEnabled = val; }
	virtual bool getTransparency () { return bTransparencyEnabled; }

	long getHeight () { return size.height (); }
	long getWidth ()  { return size.width (); }

	virtual void setViewSize (CRect &rect);
	virtual CRect &getViewSize (CRect &rect) { rect = size; return rect; }

	virtual void setParentView (CView *pParentView) { this->pParentView = pParentView; }
	CView  *getParentView () { return pParentView; }
	
	virtual void setParent (CFrame *pParent) { this->pParent = pParent; }
	CFrame *getParent () { return pParent; }
	
	virtual void *getEditor ();

	virtual bool removed (CView* parent) { return true; }   // it has have been removed from parent view
	virtual bool attached (CView* view) { return true; }    // it has been attached to a view

	virtual void forget ();
	virtual void remember ();
	virtual	long getNbReference () { return nbReference; }

	//-------------------------------------------
protected:
	friend class CControl;
	friend class CFrame;
	friend class CViewContainer;

	long nbReference;

	CRect  size;
	CRect  mouseableArea;

	CFrame *pParent;
	CView  *pParentView;

	bool  bDirty;
	bool  bMouseEnabled;
	bool  bTransparencyEnabled;
};

//-----------------------------------------------------------------------------
// CFrame Declaration
//-----------------------------------------------------------------------------
class CFrame : public CView
{
public:
	CFrame (const CRect &size, void *pSystemWindow, void *pEditor);
	CFrame (const CRect &size, char *pTitle, void *pEditor, const long style = 0);
	
	~CFrame ();

	bool open (CPoint *pPoint = 0);
	bool close ();
	bool isOpen () { return bOpenFlag; }

	void draw (CDrawContext *pContext);
	void drawRect (CDrawContext *pContext, CRect& updateRect);
	void draw (CView *pView = 0);
	void mouse (CDrawContext *pContext, CPoint &where);
	bool onDrop (void **ptrItems, long nbItems, long type, CPoint &where);
	bool onWheel (CDrawContext *pContext, const CPoint &where, float distance);
	long onKeyDown (VstKeyCode& keyCode);
	long onKeyUp (VstKeyCode& keyCode);

	void update (CDrawContext *pContext);
	void idle ();
	void doIdleStuff ();

	// get the current time (in ms)
	unsigned long getTicks ();
	long getKnobMode ();

	bool getPosition (long &x, long &y);
	bool setSize (long width, long height);
	bool getSize (CRect *pSize);

	void     setBackground (CBitmap *pBackground);
	CBitmap *getBackground () { return pBackground; }

	virtual bool addView (CView *pView);
	virtual bool removeView (CView *pView, const bool &withForget = false);
	virtual bool removeAll (const bool &withForget = true);
	virtual bool isChild (CView *pView);

	virtual long getNbViews () { return viewCount; }
	virtual CView *getView (long index);

	long   setModalView (CView *pView);
	CView *getModalView () { return pModalView; }

	void  beginEdit (long index);
	void  endEdit (long index);

	bool  getCurrentLocation (CPoint &where);
	void  setCursor (CCursorType type);

	CView *getCurrentView ();

#if WINDOWS
	HWND getOuterWindow ();
	void *getSystemWindow () { return pHwnd; }
#elif BEOS
	void *getSystemWindow () { return pPlugView; }
#else
	void *getSystemWindow () { return pSystemWindow; }
#endif
	void *getParentSystemWindow () { return pSystemWindow; }
	void setParentSystemWindow (void *val) { pSystemWindow = val; }

	virtual void *getEditor () { return pEditor; }

	void   setEditView (CView *pView);
	CView *getEditView () { return pEditView; }

	bool setDropActive (bool val);
	bool isDropActive () { return bDropActive; };

	void invalidate (const CRect &rect);

#if MOTIF
	Colormap getColormap ()   { return colormap; }
	Visual  *getVisual ()     { return pVisual; }
	unsigned int getDepth ()  { return depth; }
	Display *getDisplay ()    { return pDisplay; }
	Window   getWindow ()     { return window; }
	void     freeGc ();

	Region   region;

	GC       gc;
	GC       getGC ()         { return gc; }
#endif

	void setOpenFlag (bool val) { bOpenFlag = val;};
	bool getOpenFlag () { return bOpenFlag; };

	//-------------------------------------------
protected:
	bool   initFrame (void *pSystemWin);
	bool   isSomethingDirty ();

	void   *pEditor;
	
	void    *pSystemWindow;
	CBitmap *pBackground;
	long    viewCount;
	long    maxViews;
	CView   **ppViews;
	CView   *pModalView;
	CView   *pEditView;

	bool    bFirstDraw;
	bool    bOpenFlag;
	bool    bDropActive;

#if WINDOWS
	void    *pHwnd;
	HDC      hdc;
	HINSTANCE hInstMsimg32dll;

#elif MOTIF
	Colormap  colormap;
	Display  *pDisplay;
	Visual   *pVisual;
	Window    window;
	unsigned int depth;

	friend void _destroyCallback (Widget, XtPointer, XtPointer);

#elif BEOS
	PlugView *pPlugView;
#endif

	//-------------------------------------------
private:
	CDrawContext *pFrameContext;
	bool     bAddedWindow;
	void     *pVstWindow;
	void     *defaultCursor;
};

//-----------------------------------------------------------------------------
// CCView Declaration
//-----------------------------------------------------------------------------
class CCView
{
public:
	CCView (CView *pView);
	~CCView ();

	CView    *pView;
	CCView   *pNext;
	CCView   *pPrevious;
};

//-----------------------------------------------------------------------------
// CViewContainer Declaration
//-----------------------------------------------------------------------------
class CViewContainer : public CView
{
public:
	CViewContainer (const CRect &size, CFrame *pParent, CBitmap *pBackground = 0);
	~CViewContainer ();

	virtual void addView (CView *pView);
	virtual void addView (CView *pView, CRect &mouseableArea, bool mouseEnabled = true);
	virtual void removeView (CView *pView, const bool &withForget = true);
	virtual void removeAll (const bool &withForget = true);
	virtual bool isChild (CView *pView);
	virtual long getNbViews ();
	virtual CView *getView (long index);
	virtual long notify (CView* sender, const char* message);

	virtual void draw (CDrawContext *pContext);
	virtual void drawRect (CDrawContext *pContext, CRect& updateRect);
	virtual void mouse (CDrawContext *pContext, CPoint &where);
	virtual bool onDrop (void **ptrItems, long nbItems, long type, CPoint &where);
	virtual bool onWheel (CDrawContext *pContext, const CPoint &where, float distance);
	virtual void update (CDrawContext *pContext);
	virtual bool hitTest (const CPoint& where, const long buttons = -1);
	virtual long onKeyDown (VstKeyCode& keyCode);
	virtual long onKeyUp (VstKeyCode& keyCode);

	virtual void looseFocus (CDrawContext *pContext = 0);
	virtual void takeFocus (CDrawContext *pContext = 0);

	virtual bool isDirty ();

	virtual void setBackgroundColor (const CColor color);
	virtual CColor getBackgroundColor () { return backgroundColor; }
	virtual void setViewSize (CRect &rect);

	virtual void setBackground (CBitmap *background);
	virtual CBitmap *getBackground () { return pBackground; }
	virtual void setBackgroundOffset (const CPoint &p) { backgroundOffset = p; }
	virtual const CPoint& getBackgroundOffset () { return backgroundOffset; }

	void drawBackgroundRect (CDrawContext *pContext, CRect& _updateRect);

	enum {
		kNormalUpdate = 0,
		kOnlyDirtyUpdate
	};

	virtual void setMode (long val) { mode = val; }
	virtual long getMode () { return mode; }

	virtual void useOffscreen (bool b);

	virtual bool removed (CView* parent);
	virtual bool attached (CView* view);
		
	CView *getCurrentView ();

	//-------------------------------------------
protected:
	bool hitTestSubViews (const CPoint& where, const long buttons = -1);

	CCView  *pFirstView;
	CCView  *pLastView;
	CBitmap *pBackground;
	long mode;
	COffscreenContext *pOffscreenContext;
	CColor backgroundColor;
	CPoint backgroundOffset;
	bool bDrawInOffscreen;
};

END_NAMESPACE_VSTGUI

// include the control objects
#ifndef __vstcontrols__
#include "vstcontrols.h"
#endif

USING_NAMESPACE_VSTGUI

//-End VSTGUI.H--------------------------------------
#endif
