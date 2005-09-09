//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 3.5       $Date: 2005-09-09 08:18:01 $
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
#define __vstgui__

// define global defines
#if WIN32
	#define WINDOWS 1
#elif __MWERKS__ || __APPLE_CC__
	#define MAC 1
	#if __MACH__
	#define MACX 1
	#define QUARTZ 1
	#ifndef TARGET_API_MAC_CARBON
	#define TARGET_API_MAC_CARBON 1
	#endif
	#ifndef __CF_USE_FRAMEWORK_INCLUDES__
	#define __CF_USE_FRAMEWORK_INCLUDES__ 1
	#endif
	#endif
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
#define VSTGUI_VERSION_MAJOR  3
#define VSTGUI_VERSION_MINOR  5

//----------------------------------------------------
//----------------------------------------------------
BEGIN_NAMESPACE_VSTGUI

class CFrame;
class CDrawContext;
class COffscreenContext;
class CControl;
class CBitmap;

END_NAMESPACE_VSTGUI

//----------------------------------------------------
class VSTGUIEditorInterface
{
public:
	virtual void doIdleStuff () {}
	virtual long getKnobMode () const { return 0; }
	
	virtual void beginEdit (long index) {}
	virtual void endEdit (long index) {}

protected:
	VSTGUIEditorInterface () : frame (0) {}
	virtual ~VSTGUIEditorInterface () {}

#if USE_NAMESPACE
	VSTGUI::CFrame* frame;
#else
	CFrame* frame;
#endif
};
//----------------------------------------------------

#define VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING	1 //QUARTZ

//----------------------------------------------------
#if WINDOWS
	#include <windows.h>

//----------------------------------------------------
#elif MAC
	#if MACX
	#include <Carbon/Carbon.h>
	//macho VST's set gBundleRef which is a CFBundleRef
	BEGIN_NAMESPACE_VSTGUI
	extern void* gBundleRef;
	END_NAMESPACE_VSTGUI
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

#define CLASS_METHODS(name, parent)             \
	virtual bool isTypeOf (const char* s) const \
		{ return (!strcmp (s, (#name))) ? true : parent::isTypeOf (s); }\

#ifdef VSTGUI_FLOAT_COORDINATES
typedef float CCoord;
#else
typedef long CCoord;
#endif

//-----------------------------------------------------------------------------
// Structure CRect
//-----------------------------------------------------------------------------
struct CRect
{
	CRect (CCoord left = 0, CCoord top = 0, CCoord right = 0, CCoord bottom = 0)
	:	left (left), top (top), right (right), bottom (bottom) {}
	CRect (const CRect& r)
	:	left (r.left), top (r.top), right (r.right), bottom (r.bottom) {}
	CRect& operator () (CCoord left, CCoord top, CCoord right, CCoord bottom)
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
	
	inline CCoord width () const  { return right - left; }
	inline CCoord height () const { return bottom - top; }

	inline CCoord getWidth () const  { return right - left; }
	inline CCoord getHeight () const { return bottom - top; }

	inline void setWidth (CCoord width) { right = left + width; }
	inline void setHeight (CCoord height) { bottom = top + height; }

	CRect &offset (CCoord x, CCoord y)
	{ left += x; right += x; top += y; bottom += y; return *this; }

	CRect &inset (CCoord deltaX, CCoord deltaY)
	{ left += deltaX; right -= deltaX; top += deltaY; bottom -= deltaY;
    return *this; }

	CRect &moveTo (CCoord x, CCoord y)
	{ CCoord vDiff = y - top; CCoord hDiff = x - left; 
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
	{ CCoord left; CCoord x;};

	union
	{ CCoord top; CCoord y;};

	union
	{ CCoord right; CCoord x2;};

	union
	{ CCoord bottom; CCoord y2;};
};

//-----------------------------------------------------------------------------
// Structure CPoint
//-----------------------------------------------------------------------------
struct CPoint
{
	CPoint (CCoord h = 0, CCoord v = 0) : h (h), v (v) {}
	CPoint& operator () (CCoord h, CCoord v)
	{ this->h = h; this->v = v; return *this; }

	bool isInside (CRect& r) const
	{ return h >= r.left && h <= r.right && v >= r.top && v <= r.bottom; } 

	bool operator != (const CPoint &other) const
	{ return (h != other.h || v != other.v); }

	bool operator == (const CPoint &other) const
	{ return (h == other.h && v == other.v); }

	CPoint &offset (CCoord h, CCoord v)
	{ this->h += h; this->v += v; return *this; }

	union
	{ CCoord h; CCoord x;};

	union
	{ CCoord v; CCoord y;};
};

//-----------------------------------------------------------------------------
// Structure CColor
//-----------------------------------------------------------------------------
struct CColor
{
	CColor& operator () (unsigned char red,
						unsigned char green,
						unsigned char blue,
						unsigned char alpha)
	{
		this->red   = red;
		this->green = green;
		this->blue  = blue;
		this->alpha = alpha;
		return *this; 
	}

	CColor& operator = (const CColor& newColor)
	{
		red   = newColor.red;
		green = newColor.green;
		blue  = newColor.blue;
		alpha = newColor.alpha;
		return *this; 
	}
	
	CColor operator ~ ()
	{
		CColor c;
		c.red   = ~red;
		c.green = ~green;
		c.blue  = ~blue;
		c.alpha = ~alpha;
		return c;
	}

	bool operator != (const CColor &other) const 
	{ return (red != other.red || green != other.green || blue  != other.blue || alpha != other.alpha); }

	bool operator == (const CColor &other) const
	{ return (red == other.red && green == other.green && blue  == other.blue && alpha == other.alpha); }
	
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char alpha;
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

#if WINDOWS
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
#define kRegisteredSymbol	"\xC2\xAE"
#define kMicroSymbol       "\xC2\xB5"
#define kPerthousandSymbol "\xE2\x80\xB0"

#elif MAC
#define kDegreeSymbol      "\xA1"
#define kInfiniteSymbol    "oo"
#define kCopyrightSymbol   "\xA9"
#define kTrademarkSymbol   "\xAA"
#define kRegisteredSymbol  "\xA8"
#define kMicroSymbol       "\xB5"
#define kPerthousandSymbol "\xE4"
#endif

class CDragContainer;
class CCView;
class CAttributeListEntry;
class CTextLabel;

//-----------------------------------------------------------------------------
typedef unsigned long CViewAttributeID;
//-----------------------------------------------------------------------------
// Attributes
//		all attributes where the first letter is lowercase are reserved for the vstgui lib

extern const CViewAttributeID kCViewAttributeReferencePointer;	// 'cvrp'
extern const CViewAttributeID kCViewTooltipAttribute;			// 'cvtt'

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
	kXorMode,
	kAntialias
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
	kCursorNWSESize,
	kCursorCopy,
	kCursorNotAllowed,
	kCursorHand
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

//----------------------------
// Draw Style
//----------------------------
enum CDrawStyle
{
	kDrawStroked = 0,
	kDrawFilled,
	kDrawFilledAndStroked
};

//----------------------------
// Mouse Wheel Axis
//----------------------------
enum CMouseWheelAxis
{
	kMouseWheelAxisX = 0,
	kMouseWheelAxisY
};

//----------------------------
// Mouse Event Results
//----------------------------
enum CMouseEventResult
{
	kMouseEventNotImplemented = 0,
	kMouseEventHandled,
	kMouseEventNotHandled,
	
	kMouseDownEventHandledButDontNeedMovedOrUpEvents
};

//-----------------------------------------------------------------------------
// CReferenceCounter Declaration (Reference Counting)
//-----------------------------------------------------------------------------
class CReferenceCounter
{
public:
	CReferenceCounter () : nbReference (1) {}
	virtual ~CReferenceCounter () {}
	
	virtual void forget () { nbReference--; if (nbReference == 0) delete this; }
	virtual void remember () { nbReference++; }
	long getNbReference () const { return nbReference; }

private:
	long nbReference;
};

//-----------------------------------------------------------------------------
// CDrawContext Declaration
//! A drawing context encapsulates the drawing context of the underlying OS. It implements the drawing functions.
//-----------------------------------------------------------------------------
class CDrawContext : public CReferenceCounter
{
public:
	CDrawContext (CFrame *pFrame, void *pSystemContext, void *pWindow = 0);
	virtual ~CDrawContext ();	

	void moveTo (const CPoint &point);	///< move line position to point
	void lineTo (const CPoint &point);	///< draw a line from current position to point
	void drawLines (const CPoint* points, const long& numberOfLines);	///< draw multiple lines at once

	void drawPolygon (const CPoint *pPoints, long numberOfPoints, const CDrawStyle drawStyle = kDrawStroked); ///< draw a polygon
	void polyLine (const CPoint *pPoint, long numberOfPoints);	///< draw a stroked polygon
	void fillPolygon (const CPoint *pPoint, long numberOfPoints);	///< draw a filled polygon

	void drawRect (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked);	///< draw a stroked rect
	void fillRect (const CRect &rect);	///< draw a filled rect

	void drawArc (const CRect &rect, const float startAngle1, const float endAngle2, const CDrawStyle drawStyle = kDrawStroked);	///< draw a stroked arc, where the angles are in degree
	void drawArc (const CRect &rect, const CPoint &point1, const CPoint &point2);		///< draw a stroked arc between point1 and point2
	void fillArc (const CRect &rect, const CPoint &point1, const CPoint &point2);		///< draw a filled arc between point1 and point2

	void drawEllipse (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked);	///< draw an ellipse
	void fillEllipse (const CRect &rect);	///< draw a filled ellipse
	
	void drawPoint (const CPoint &point, CColor color);	///< draw a point
	CColor getPoint (const CPoint& point); ///< \deprecated

	void floodFill (const CPoint& start); ///< \deprecated
	
	void       setLineStyle (CLineStyle style);				///< set the current line style
	CLineStyle getLineStyle () const { return lineStyle; }	///< get the current line style

	void   setLineWidth (CCoord width);						///< set the current line width
	CCoord getLineWidth () const { return frameWidth; }		///< get the current line width

	void      setDrawMode (CDrawMode mode);					///< set the current draw mode, see CDrawMode
	CDrawMode getDrawMode () const { return drawMode; }		///< get the current draw mode, see CDrawMode

	void   setClipRect (const CRect &clip);					///< set the current clip
	CRect &getClipRect (CRect &clip) const { clip = clipRect; clip.offset (-offset.h, -offset.v); return clip; }	///< get the current clip
	void   resetClipRect ();	///< reset the clip to the default state

	void   setFillColor  (const CColor color);			///< set current fill color
	CColor getFillColor () const { return fillColor; }	///< get current fill color

	void   setFrameColor (const CColor color);				///< set current stroke color
	CColor getFrameColor () const { return frameColor; }	///< get current stroke color

	void   setFontColor (const CColor color);			///< set current font color
	CColor getFontColor () const { return fontColor; }	///< get current font color
	void   setFont (CFont fontID, const long size = 0, long style = 0);	///< set current font
	CFont  getFont () const { return fontId; }							///< get current font
	long   getFontSize () const { return fontSize; }	///< get current font size

	CCoord getStringWidth (const char* pStr);	///< get the width of a string

	void drawString (const char *pString, const CRect &rect, const short opaque = false,
					 const CHoriTxtAlign hAlign = kCenterText);	///< draw a string

	long getMouseButtons ();	///< get current mouse buttons
	void getMouseLocation (CPoint &point);	///< get current mouse location. should not be used, see CView::getMouseLocation
	bool waitDoubleClick ();	///< check if another mouse click occurs in the near future
	bool waitDrag ();			///< check if the mouse will be dragged

	void *getWindow () { return pWindow; }
	void setWindow (void *ptr)  { pWindow = ptr; }
	void getLoc (CPoint &where) const { where = penLoc; }
	CFrame* getFrame () const { return pFrame; }

	CPoint offsetScreen;
	CPoint offset;

	void   *getSystemContext () const { return pSystemContext; }

	virtual void forget ();

	//-------------------------------------------
protected:

	friend class CBitmap;
	friend class COffscreenContext;

	void   *pSystemContext;
	void   *pWindow;
	CFrame *pFrame;

	long   fontSize;
	long   fontStyle;
	CFont  fontId;
	CColor fontColor;
	CPoint penLoc;

	CCoord   frameWidth;
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
	HDC  pHDC;

#elif MAC
	#if QUARTZ
	CGContextRef gCGContext;
	bool needToSynchronizeCGContext;
	public:
	CGContextRef getCGContext () const { return gCGContext; }
	CGContextRef beginCGContext (bool swapYAxis = false);
	void releaseCGContext (CGContextRef context);
	void synchronizeCGContext ();
	
	virtual CGImageRef getCGImage () const;
	protected:
	#else

	FontInfo fontInfoStruct;
	Pattern fillPattern;
	bool bInitialized;
	#endif
	virtual BitMapPtr getBitmap ();
	virtual void releaseBitmap ();
	virtual CGrafPtr getPort ();
	
#elif BEOS
	BView*	pView;
	BFont	font;
	void lineFromTo (CPoint& cstart, CPoint& cend);

#endif
};


//-----------------------------------------------------------------------------
// COffscreenContext Declaration
//! A drawing device which uses a pixmap as its drawing surface.
//-----------------------------------------------------------------------------
class COffscreenContext : public CDrawContext
{
public:
	COffscreenContext (CDrawContext *pContext, CBitmap *pBitmap, bool drawInBitmap = false);
	COffscreenContext (CFrame *pFrame, long width, long height, const CColor backgroundColor = kBlackCColor);

	virtual ~COffscreenContext ();
	
	void copyFrom (CDrawContext *pContext, CRect destRect, CPoint srcOffset = CPoint (0, 0));	///< copy from offscreen to pContext
	void copyTo (CDrawContext* pContext, CRect& srcRect, CPoint destOffset = CPoint (0, 0));	///< copy to offscreen from pContext

	inline CCoord getWidth () const { return width; }
	inline CCoord getHeight () const { return height; }

	//-------------------------------------------
protected:
	CBitmap *pBitmap;
	CBitmap *pBitmapBg;
	CCoord    height;
	CCoord    width;
	bool    bDestroyPixmap;

	CColor  backgroundColor;

#if WINDOWS
	void* oldBitmap;

#elif BEOS
	BBitmap *offscreenBitmap;

#elif MAC
	#if QUARTZ
	void* offscreenBitmap;
	virtual CGImageRef getCGImage () const;
	#else
	CGrafPtr getPort ();
	#endif
	BitMapPtr getBitmap ();
	void releaseBitmap ();
#endif
};


//-----------------------------------------------------------------------------
// CBitmap Declaration
//! Encapsulates various platform depended kinds of bitmaps.
//-----------------------------------------------------------------------------
class CBitmap : public CReferenceCounter
{
public:
	CBitmap (long resourceID);	///< Create a pixmap from a resource identifier
	CBitmap (CFrame &frame, CCoord width, CCoord height);	///< Create a pixmap with a given size.
	virtual ~CBitmap ();

	virtual void draw (CDrawContext *pContext, CRect &rect, const CPoint &offset = CPoint (0, 0));	///< Draw the pixmap using a given rect as output position and a given offset of its source pixmap.
	virtual void drawTransparent (CDrawContext *pContext, CRect &rect, const CPoint &offset = CPoint (0, 0));
	virtual void drawAlphaBlend  (CDrawContext *pContext, CRect &rect, const CPoint &offset = CPoint (0, 0), unsigned char alpha = 128);	///< Same as CBitmap::draw except that it uses the alpha value to draw the bitmap alpha blended.

	inline CCoord getWidth () const { return width; }
	inline CCoord getHeight () const { return height; }

	bool isLoaded () const;
	void *getHandle () const;
	
	void setTransparentColor (const CColor color);
	CColor getTransparentColor () const { return transparentCColor; }
	void setTransparencyMask (CDrawContext* pContext, const CPoint& offset = CPoint (0, 0));

	void setNoAlpha (bool state) { noAlpha = state; }
	bool getNoAlpha () const { return noAlpha; }

#if BEOS
	static void closeResource ();
#endif
#if MACX
	#if QUARTZ
	virtual CGImageRef createCGImage (bool transparent = false);
	#endif
#endif

	//-------------------------------------------
protected:
	CBitmap ();

	virtual void dispose ();
	virtual bool loadFromResource (long resourceID);
	virtual bool loadFromPath (const void* platformPath);	// load from a platform path. On Windows it's a C string and on Mac OS X its a CFURLRef.

	long resourceID;
	CCoord width;
	CCoord height;

	CColor transparentCColor;
	bool noAlpha;

#if WINDOWS
	void *pHandle;
	void *pMask;

#elif MAC
	void* pHandle;
	void* pMask;
	#if QUARTZ
	void* cgImage;
	#endif
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
class CView : public CReferenceCounter
{
public:
	CView (const CRect &size);
	virtual ~CView ();

	virtual void draw (CDrawContext *pContext);	///< called if the view should draw itself
	virtual void drawRect (CDrawContext *pContext, const CRect& updateRect) { draw (pContext); }	///< called if the view should draw itself
	virtual bool checkUpdate (CRect& updateRect) const { return updateRect.rectOverlap (size); }
	virtual void mouse (CDrawContext *pContext, CPoint &where, long buttons = -1);	///< called if a mouse click event occurs \deprecated

	virtual CMouseEventResult onMouseDown (CPoint &where, const long& buttons) {return kMouseEventNotImplemented;}		///< called when a mouse down event occurs
	virtual CMouseEventResult onMouseUp (CPoint &where, const long& buttons) {return kMouseEventNotImplemented;}		///< called when a mouse up event occurs
	virtual CMouseEventResult onMouseMoved (CPoint &where, const long& buttons) {return kMouseEventNotImplemented;}		///< called when a mouse move event occurs

	virtual CMouseEventResult onMouseEntered (CPoint &where, const long& buttons) {return kMouseEventNotImplemented;}	///< called when the mouse enters this view
	virtual CMouseEventResult onMouseExited (CPoint &where, const long& buttons) {return kMouseEventNotImplemented;}	///< called when the mouse leaves this view
	
	virtual void setBackground (CBitmap *background);				///< set the background image of this view
	virtual CBitmap *getBackground () const { return pBackground; }	///< get the background image of this view

	virtual long onKeyDown (VstKeyCode& keyCode);	///< called if a key down event occurs and this view has focus
	virtual long onKeyUp (VstKeyCode& keyCode);		///< called if a key up event occurs and this view has focus

	virtual bool onWheel (const CPoint &where, const float &distance, const long &buttons);	///< called if a mouse wheel event is happening over this view
	virtual bool onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const long &buttons);	///< called if a mouse wheel event is happening over this view

	virtual bool onDrop (CDragContainer* drag, const CPoint& where) { return false; }	///< called if a drag is dropped onto this view
	virtual void onDragEnter (CDragContainer* drag, const CPoint& where) {}				///< called if a drag is entering this view
	virtual void onDragLeave (CDragContainer* drag, const CPoint& where) {}				///< called if a drag is leaving this view
	virtual void onDragMove (CDragContainer* drag, const CPoint& where) {}				///< called if a drag is current moved over this view

	virtual void looseFocus ();															///< called if view should loose focus
	virtual void takeFocus ();															///< called if view should take focus

	virtual bool isDirty () const { return bDirty; }											///< check if view is dirty
	virtual void setDirty (const bool val = true) { bDirty = val; }								///< set the view to dirty so that it is redrawn in the next idle. Thread Safe !

	#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
	virtual void invalidRect (const CRect rect);
	virtual void invalid () { invalidRect (size); bDirty = false; }
	#endif
	
	virtual void setMouseEnabled (const bool bEnable = true) { bMouseEnabled = bEnable; }		///< turn on/off mouse usage for this view
	virtual bool getMouseEnabled () const { return bMouseEnabled; }								///< get the state of wheather this view uses the mouse or not

	virtual void setMouseableArea (const CRect &rect)  { mouseableArea = rect; }				///< set the area in which the view reacts to the mouse
	virtual CRect &getMouseableArea (CRect &rect) const { rect = mouseableArea; return rect;}	///< get the area in which the view reacts to the mouse

	virtual bool hitTest (const CPoint& where, const long buttons = -1) { return where.isInside (mouseableArea); }	///< check if where hits this view

	virtual void setTransparency (bool val) { bTransparencyEnabled = val; }			///< set views transparent state
	virtual bool getTransparency () const { return bTransparencyEnabled; }			///< is view transparent ?

	CCoord getHeight () const { return size.height (); }								///< get the height of the view
	CCoord getWidth ()  const { return size.width (); }								///< get the width of the view

	virtual void setViewSize (CRect &rect);											///< set views size
	virtual CRect &getViewSize (CRect &rect) const { rect = size; return rect; }	///< returns the current view size

	virtual bool removed (CView* parent) { return true; }   ///< view is removed from parent view
	virtual bool attached (CView* view) { return true; }    ///< view is attached to a parent view

	virtual void getMouseLocation (CDrawContext* context, CPoint &point);	///< get current mouse location in local view coordinates

	virtual CPoint& frameToLocal (CPoint& point) const;		///< conversion from frame coordinates to local view coordinates
	virtual CPoint& localToFrame (CPoint& point) const;		///< conversion from local view coordinates to frame coordinates

	bool getAttributeSize (const CViewAttributeID id, long& outSize) const;									///< get the size of an attribute
	bool getAttribute (const CViewAttributeID id, const long inSize, void* outData, long& outSize) const;	///< get an attribute
	bool setAttribute (const CViewAttributeID id, const long inSize, const void* inData);					///< set an attribute

	CView  *getParentView () const { return pParentView; }
	CFrame *getFrame () const { return pParentFrame; }
	virtual VSTGUIEditorInterface *getEditor () const;

	virtual long notify (CView* sender, const char* message);
	#if !VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
	void redraw ();
	virtual void redrawRect (CDrawContext* context, const CRect& rect);
	#endif

	virtual bool wantsFocus () const { return bWantsFocus; }			///< check if view supports focus
	virtual void setWantsFocus (bool state) { bWantsFocus = state; }	///< set focus support on/off

	#if DEBUG
	virtual void dumpInfo ();
	#endif

	virtual bool isTypeOf (const char* s) const
		{ return (!strcmp (s, "CView")); }

#if ENABLE_DEPRECATED_METHODS
	// deprecated methods will be placed here, so that people who really need them can turn the macro on

	virtual void setParentView (CView *pParentView) { this->pParentView = pParentView; }  ///< \deprecated
	virtual void setFrame (CFrame *pParent) { this->pParentFrame = pParent; }  ///< \deprecated
	virtual void getFrameTopLeftPos (CPoint& topLeft) const; ///< \deprecated
#endif
	//-------------------------------------------
protected:
	friend class CControl;
	friend class CFrame;
	friend class CViewContainer;

	CRect  size;
	CRect  mouseableArea;

	CFrame *pParentFrame;
	CView  *pParentView;

	bool  bDirty;
	bool  bMouseEnabled;
	bool  bTransparencyEnabled;
	bool  bWantsFocus;
	
	CBitmap* pBackground;
	CAttributeListEntry* pAttributeList;
#if !VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
	virtual void update (CDrawContext *pContext); // don't call this !!!
#endif
};

// Message to check if View is a CViewContainer
extern char* kMsgCheckIfViewContainer;

//-----------------------------------------------------------------------------
// CViewContainer Declaration
//! Container Class of CView objects.
//-----------------------------------------------------------------------------
class CViewContainer : public CView
{
public:
	CViewContainer (const CRect &size, CFrame *pParent, CBitmap *pBackground = 0);
	virtual ~CViewContainer ();

	virtual void addView (CView *pView);	///< add a child view
	virtual void addView (CView *pView, CRect &mouseableArea, bool mouseEnabled = true);	///< add a child view
	virtual void removeView (CView *pView, const bool &withForget = true);	///< remove a child view
	virtual void removeAll (const bool &withForget = true);	///< remove all child views
	virtual bool isChild (CView *pView) const;	///< check if pView is a child view of this container
	virtual long getNbViews () const;			///< get the number of child views
	virtual CView *getView (long index) const;	///< get the child view at index

	virtual void setBackgroundColor (const CColor color);	///< set the background color (will only be drawn if this container is not set to transparent and does not have a background bitmap)
	virtual CColor getBackgroundColor () const { return backgroundColor; }	///< get the background color
	virtual void setBackgroundOffset (const CPoint &p) { backgroundOffset = p; }	///< set the offset of the background bitmap
	virtual const CPoint& getBackgroundOffset () const { return backgroundOffset; }	///< get the offset of the background bitmap

	virtual void drawBackgroundRect (CDrawContext *pContext, CRect& _updateRect);	///< draw the background

	enum {
		kNormalUpdate = 0,		///< this mode redraws the whole container if something is dirty
		kOnlyDirtyUpdate		///< this mode only redraws the views which are dirty
	};

	virtual void setMode (long val) { mode = val; }	///< set the update mode
	virtual long getMode () const { return mode; }	///< get the update mode

	virtual void useOffscreen (bool b);	///< turn on/off using an offscreen

	virtual CView *getCurrentView () const;	///< get the current view under the mouse
	virtual CView *getViewAt (const CPoint& where, bool deep = false) const;	///< get the view at point where

	void modifyDrawContext (CCoord save[4], CDrawContext* pContext);
	void restoreDrawContext (CDrawContext* pContext, CCoord save[4]);

	// CView
	virtual void draw (CDrawContext *pContext);
	virtual void drawRect (CDrawContext *pContext, const CRect& updateRect);
	virtual void mouse (CDrawContext *pContext, CPoint &where, long buttons = -1);
	virtual CMouseEventResult onMouseDown (CPoint &where, const long& buttons);
	virtual CMouseEventResult onMouseUp (CPoint &where, const long& buttons);
	virtual CMouseEventResult onMouseMoved (CPoint &where, const long& buttons);
	virtual bool onWheel (const CPoint &where, const float &distance, const long &buttons);
	virtual bool onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const long &buttons);
	#if !VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
	virtual void update (CDrawContext *pContext);
	virtual void redrawRect (CDrawContext* context, const CRect& rect);
	#endif
	virtual bool hitTest (const CPoint& where, const long buttons = -1);
	virtual long onKeyDown (VstKeyCode& keyCode);
	virtual long onKeyUp (VstKeyCode& keyCode);
	virtual long notify (CView* sender, const char* message);

	virtual bool onDrop (CDragContainer* drag, const CPoint& where);
	virtual void onDragEnter (CDragContainer* drag, const CPoint& where);
	virtual void onDragLeave (CDragContainer* drag, const CPoint& where);
	virtual void onDragMove (CDragContainer* drag, const CPoint& where);

	virtual void looseFocus ();
	virtual void takeFocus ();
	virtual bool advanceNextFocusView (CView* oldFocus, bool reverse = false);

	virtual bool isDirty () const;

	#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
	virtual void invalid ();
	virtual void invalidRect (const CRect rect);
	virtual bool invalidateDirtyViews ();
	#endif
	
	virtual void setViewSize (CRect &rect);

	virtual bool removed (CView* parent);
	virtual bool attached (CView* view);
		
	virtual CPoint& frameToLocal (CPoint& point) const;
	virtual CPoint& localToFrame (CPoint& point) const;

	CLASS_METHODS(CViewContainer, CView)

	#if DEBUG
	virtual void dumpInfo ();
	virtual void dumpHierarchy ();
	#endif

	//-------------------------------------------
protected:
	bool hitTestSubViews (const CPoint& where, const long buttons = -1);

	CCView  *pFirstView;
	CCView  *pLastView;
	long mode;
	COffscreenContext *pOffscreenContext;
	CColor backgroundColor;
	CPoint backgroundOffset;
	bool bDrawInOffscreen;

	CView* currentDragView;
	CView* mouseDownView;
	CView* mouseOverView;
};

//-----------------------------------------------------------------------------
// CFrame Declaration
//! The CFrame is the parent container of all views.
//-----------------------------------------------------------------------------
class CFrame : public CViewContainer
{
public:
	CFrame (const CRect &size, void *pSystemWindow, VSTGUIEditorInterface *pEditor);
	CFrame (const CRect &size, const char *pTitle, VSTGUIEditorInterface *pEditor, const long style = 0);
	
	virtual ~CFrame ();

	virtual bool open (CPoint *pPoint = 0);
	virtual bool close ();
	virtual bool isOpen () const { return bOpenFlag; }

	virtual void idle ();
	virtual void doIdleStuff ();

	virtual unsigned long getTicks () const;	///< get the current time (in ms)
	virtual long getKnobMode () const;			///< get hosts knob mode

	virtual bool setPosition (CCoord x, CCoord y);
	virtual bool getPosition (CCoord &x, CCoord &y) const;

	virtual bool setSize (CCoord width, CCoord height);
	virtual bool getSize (CRect *pSize) const;
	virtual bool getSize (CRect &pSize) const;

	virtual long   setModalView (CView *pView);
	virtual CView *getModalView () const { return pModalView; }

	virtual void setTooltipView (CTextLabel* view) { tooltipView = view; }
	virtual CTextLabel* getTooltipView () const { return tooltipView; }
	
	virtual void  beginEdit (long index);
	virtual void  endEdit (long index);

	virtual bool  getCurrentLocation (CPoint &where);
	virtual void  setCursor (CCursorType type);

	virtual void   setFocusView (CView *pView);
	virtual CView *getFocusView () const { return pFocusView; }
	virtual bool advanceNextFocusView (CView* oldFocus, bool reverse = false);

	virtual bool setDropActive (bool val);
	virtual bool isDropActive () const { return bDropActive; };

	CDrawContext* createDrawContext ();

	virtual void setOpenFlag (bool val) { bOpenFlag = val;};
	virtual bool getOpenFlag () const { return bOpenFlag; };

	#if VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
	virtual void invalid () { invalidRect (size); bDirty = false; }
	virtual void invalidRect (const CRect rect);
	#endif
	virtual void invalidate (const CRect &rect);

	virtual bool updatesDisabled () const { return bUpdatesDisabled; }
	virtual bool updatesDisabled (bool state) { bool before = bUpdatesDisabled; bUpdatesDisabled = state; return before; }

	#if WINDOWS
	HWND getOuterWindow () const;
	void *getSystemWindow () const { return pHwnd; }
	COffscreenContext* getBackBuffer ();
	#elif BEOS
	void *getSystemWindow () const { return pPlugView; }
	#else
	void *getSystemWindow () const { return pSystemWindow; }
	#endif
	void *getParentSystemWindow () const { return pSystemWindow; }
	void setParentSystemWindow (void *val) { pSystemWindow = val; }

	// CView
	virtual void draw (CDrawContext *pContext);
	virtual void drawRect (CDrawContext *pContext, const CRect& updateRect);
	virtual void draw (CView *pView = 0);
	virtual void mouse (CDrawContext *pContext, CPoint &where, long buttons = -1);
	virtual CMouseEventResult onMouseDown (CPoint &where, const long& buttons);
	virtual CMouseEventResult onMouseUp (CPoint &where, const long& buttons);
	virtual CMouseEventResult onMouseMoved (CPoint &where, const long& buttons);
	virtual bool onWheel (const CPoint &where, const float &distance, const long &buttons);
	virtual bool onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const long &buttons);
	virtual long onKeyDown (VstKeyCode& keyCode);
	virtual long onKeyUp (VstKeyCode& keyCode);
#if !VSTGUI_USE_SYSTEM_EVENTS_FOR_DRAWING
	virtual void update (CDrawContext *pContext);
#endif
	virtual void setViewSize (CRect& inRect);
	virtual CView *getCurrentView () const;

	virtual VSTGUIEditorInterface *getEditor () const { return pEditor; }

	#if DEBUG
	virtual void dumpHierarchy ();
	#endif

	CLASS_METHODS(CFrame, CViewContainer)

	//-------------------------------------------
protected:
	bool   initFrame (void *pSystemWin);

	VSTGUIEditorInterface   *pEditor;
	
	void    *pSystemWindow;
	CView   *pModalView;
	CView   *pFocusView;
	CTextLabel* tooltipView;

	bool    bFirstDraw;
	bool    bOpenFlag;
	bool    bDropActive;
	bool	bUpdatesDisabled;

#if WINDOWS
	void      *pHwnd;
	HINSTANCE hInstMsimg32dll;
	void*     dropTarget;
	COffscreenContext* backBuffer;

#elif BEOS
	PlugView *pPlugView;
#endif
#if QUARTZ
	void setDrawContext (CDrawContext* context) { pFrameContext = context; }
	friend class CDrawContext;

	static pascal OSStatus carbonMouseEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
	static pascal OSStatus carbonEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
	bool registerWithToolbox ();
	
	ControlDefSpec controlSpec;
	ControlRef controlRef;
	bool hasFocus;
	EventHandlerRef dragEventHandler;
	EventHandlerRef mouseEventHandler;
	public:
	void* getPlatformControl () const { return controlRef; }
	CPoint hiScrollOffset;
	protected:
#endif
	//-------------------------------------------
private:
	CDrawContext *pFrameContext;
	bool     bAddedWindow;
	void     *pVstWindow;
	void     *defaultCursor;
};

//-----------------------------------------------------------------------------
// CDragContainer Declaration
//-----------------------------------------------------------------------------
class CDragContainer : public CReferenceCounter
{
public:
	CDragContainer (void* platformDrag);
	~CDragContainer ();

	void* first (long& size, long& type);		///< returns pointer on a char array if type is known
	void* next (long& size, long& type);		///< returns pointer on a char array if type is known
	
	long getType (long idx) const;
	long getCount () const { return nbItems; }

	enum {
		kFile = 0,
		kText,

		kUnknown = -1
	};

protected:
	void* platformDrag;
	long nbItems;
	
	long iterator;
	void* lastItem;
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

END_NAMESPACE_VSTGUI

// include the control objects
#ifndef __vstcontrols__
#include "vstcontrols.h"
#endif

USING_NAMESPACE_VSTGUI

//-End VSTGUI.H--------------------------------------
#endif	// __vstgui__
