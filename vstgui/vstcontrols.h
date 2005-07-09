//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
// Standard Control Objects
//
// Version 3.0       $Date: 2005-07-09 13:27:12 $
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

#ifndef __vstcontrols__
#define __vstcontrols__

#ifndef __vstgui__
#include "vstgui.h"
#endif

//------------------
// defines
//------------------
#ifndef kPI
#define kPI    3.14159265358979323846
#endif

#ifndef k2PI
#define k2PI   6.28318530717958647692
#endif

#ifndef kPI_2
#define kPI_2  1.57079632679489661923f
#endif

#ifndef kPI_4
#define kPI_4  0.78539816339744830962
#endif

#ifndef kE
#define kE     2.7182818284590452354
#endif

#ifndef kLN2
#define kLN2   0.69314718055994530942
#endif

#ifndef kSQRT2
#define kSQRT2 1.41421356237309504880
#endif

//------------------
// CControlEnum type
//------------------
enum CControlEnum
{
	kHorizontal			= 1 << 0,
	kVertical			= 1 << 1,
	kShadowText			= 1 << 2,
	kLeft				= 1 << 3,
	kRight				= 1 << 4,
	kTop				= 1 << 5,
	kBottom				= 1 << 6,
	k3DIn				= 1 << 7,
	k3DOut				= 1 << 8,
	kPopupStyle			= 1 << 9,
	kCheckStyle			= 1 << 10,
	kMultipleCheckStyle,
	kNoTextStyle		= 1 << 11,
	kNoDrawStyle		= 1 << 12,
	kDoubleClickStyle	= 1 << 13,
	kNoFrame			= 1 << 14
};

//---------------------------
// Some defines for Menu item
//---------------------------
#define kMenuTitle     "-T"
#define kMenuSeparator "-"
#define kMenuDisable   "-G"
#define kMenuSubMenu   "-M"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CControlListener
{
public:
	#if USE_NAMESPACE
	virtual void valueChanged (VSTGUI::CDrawContext *pContext, VSTGUI::CControl *pControl) = 0;
	virtual long controlModifierClicked (VSTGUI::CDrawContext *pContext, VSTGUI::CControl *pControl, long button) { return 0; }	// return 1 if you want the control to not handle it, otherwise 0
	#else
	virtual void valueChanged (CDrawContext *pContext, CControl *pControl) = 0;
	virtual long controlModifierClicked (CDrawContext *pContext, CControl *pControl, long button) { return 0; }	// return 1 if you want the control to not handle it, otherwise 0
	#endif
};

class AudioEffectX;

//-----------------------------------------------------------------------------
BEGIN_NAMESPACE_VSTGUI
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CControl Declaration
//! base class of all VSTGUI controls
//-----------------------------------------------------------------------------
class CControl : public CView
{
public:
	CControl (const CRect &size, CControlListener *listener = 0, long tag = 0, 
						CBitmap *pBackground = 0);
	virtual ~CControl ();

	virtual void  draw (CDrawContext *pContext) = 0;
	virtual void  doIdleStuff () { if (pParentFrame) pParentFrame->doIdleStuff (); }

	virtual void  setValue (float val) { value = val; }
	virtual float getValue () const { return value; };

	virtual void  setMin (float val) { vmin = val; }
	virtual float getMin () const { return vmin; }
	virtual void  setMax (float val) { vmax = val; }
	virtual float getMax () const { return vmax; }

	virtual void  setOldValue (float val) { oldValue = val; }
	virtual	float getOldValue (void) const { return oldValue; }
	virtual void  setDefaultValue (float val) { defaultValue = val; }
	virtual	float getDefaultValue (void) const { return defaultValue; }

	virtual void  setTag (long val) { tag = val; }
	inline  long  getTag () const { return tag; }

	virtual bool  isDirty () const;
	virtual void  setDirty (const bool val = true);

	virtual void beginEdit ();
	virtual void endEdit ();

	virtual void setBackOffset (CPoint &offset);
	virtual void copyBackOffset ();

	virtual void  setWheelInc (float val) { wheelInc = val; }
	virtual float getWheelInc () const { return wheelInc; }

	virtual void bounceValue ();
	virtual bool checkDefaultValue (CDrawContext *pContext, long button);

	CControlListener* getListener () const { return listener; }
	void setListener (CControlListener* l) { listener = l; }
	bool isDoubleClick ();

	CLASS_METHODS(CControl, CView)

protected:
	CControlListener *listener;
	long  tag;
	float oldValue;
	float defaultValue;
	float value;
	float vmin;
	float vmax;
	float wheelInc;

	long lastTicks;
	long delta;

	CPoint	backOffset;
};


//-----------------------------------------------------------------------------
// COnOffButton Declaration
//! a button control with 2 states
//-----------------------------------------------------------------------------
class COnOffButton : public CControl
{
public:
	COnOffButton (const CRect &size, CControlListener *listener, long tag,
                  CBitmap *background, long style = kPreListenerUpdate);
	virtual ~COnOffButton ();

	virtual void draw (CDrawContext*);
	virtual void mouse (CDrawContext *pContext, CPoint &where, long button = -1);

	virtual long getStyle () const { return style; }
	virtual void setStyle (long newStyle) { style = newStyle; }

	enum {
		kPreListenerUpdate,			///< listener will be called after doIdleStuff was called
		kPostListenerUpdate,		///< listener will be called before doIdleStuff is called
	};

	CLASS_METHODS(COnOffButton, CControl)
protected:
	long style;
};


//-----------------------------------------------------------------------------
// CParamDisplay Declaration
//! a parameter display control
//-----------------------------------------------------------------------------
class CParamDisplay : public CControl
{
public:
	CParamDisplay (const CRect &size, CBitmap *background = 0, const long style = 0);
	virtual ~CParamDisplay ();
	
	virtual void setFont (CFont fontID);
	CFont getFont () const { return fontID; }

	virtual void setFontColor (CColor color);
	CColor getFontColor () const { return fontColor; }

	virtual void setBackColor (CColor color);
	CColor getBackColor () const { return backColor; }

	virtual void setFrameColor (CColor color);
	CColor getFrameColor () const { return frameColor; }

	virtual void setShadowColor (CColor color);
	CColor getShadowColor () const { return shadowColor; }

	virtual void setHoriAlign (CHoriTxtAlign hAlign);

	virtual void setStringConvert (void (*convert) (float value, char *string));
	virtual void setStringConvert (void (*convert) (float value, char *string, void *userDta),
									void *userData);
	virtual void setString2FloatConvert (void (*convert) (char *string, float &output));

	virtual void setStyle (long val);
	long getStyle () const { return style; }

	virtual void setTxtFace (CTxtFace val);
	CTxtFace getTxtFace () const { return txtFace; }

	virtual void draw (CDrawContext *pContext);

	virtual void setTextTransparency (bool val) { bTextTransparencyEnabled = val; }
	bool getTextTransparency () const { return bTextTransparencyEnabled; }

	CLASS_METHODS(CParamDisplay, CControl)

protected:
	void drawText (CDrawContext *pContext, char *string, CBitmap *newBack = 0);

	void (*stringConvert) (float value, char *string);
	void (*stringConvert2) (float value, char *string, void *userData);
	void (*string2FloatConvert) (char *string, float &output);
	void  *userData;

	CHoriTxtAlign horiTxtAlign;
	long    style;

	CFont   fontID;
	CTxtFace txtFace;
	CColor  fontColor;
	CColor  backColor;
	CColor  frameColor;
	CColor  shadowColor;
	bool    bTextTransparencyEnabled;
};


//-----------------------------------------------------------------------------
// CLabel Declaration
//! a text label
//-----------------------------------------------------------------------------
class CTextLabel : public CParamDisplay
{
public:
	CTextLabel (const CRect& size, const char* txt = 0, CBitmap* background = 0, const long style = 0);
	~CTextLabel ();
	
	virtual void setText (const char* txt);
	virtual const char* getText () const;
	
	virtual	void draw (CDrawContext *pContext);

	CLASS_METHODS(CTextLabel, CParamDisplay)

protected:
	void freeText ();
	char* text;
};

//-----------------------------------------------------------------------------
// CTextEdit Declaration
//! a text edit control
//-----------------------------------------------------------------------------
class CTextEdit : public CParamDisplay
{
public:
	CTextEdit (const CRect &size, CControlListener *listener, long tag, const char *txt = 0,
               CBitmap *background = 0, const long style = 0);
	virtual ~CTextEdit ();

	virtual void setText (char *txt);
	virtual void getText (char *txt) const;

	virtual	void draw (CDrawContext *pContext);
	virtual	void mouse (CDrawContext *pContext, CPoint &where, long button = -1);

	virtual void setTextEditConvert (void (*editConvert) (char *input, char *string));
	virtual void setTextEditConvert (void (*editConvert2) (char *input, char *string,
										void *userDta), void *userData);

	virtual	void takeFocus (CDrawContext *pContext = 0);
	virtual	void looseFocus (CDrawContext *pContext = 0);

	void *platformFontColor;
	void *platformControl;
	bool bWasReturnPressed;
	#if MAC
	short pluginResID;
	#if QUARTZ
	HIViewRef textControl;
	#endif
	#endif

	CLASS_METHODS(CTextEdit, CParamDisplay)

protected:
	void *platformFont;
	char text[256];

#if (MAC && !MACX)
	void *text_edit;
	bool bLoosefocusWanted;
#endif

	void (*editConvert) (char *input, char *string);
	void (*editConvert2) (char *input, char *string, void *userData);
};


//-----------------------------------------------------------------------------
// COptionMenuScheme Declaration
//-----------------------------------------------------------------------------
class COptionMenuScheme : public CReferenceCounter
{
public:
	COptionMenuScheme ();
	virtual ~COptionMenuScheme ();

	enum { kChecked = 0x01, kDisabled = 0x02, kSelected = 0x04, kSubMenu = 0x08, kTitle = 0x10 };

	virtual void getItemSize (const char* text, CDrawContext* pContext, CPoint& size);
	virtual void drawItem (const char* text, long itemId, long state, CDrawContext* pContext, const CRect& rect);	

	void setColors (CColor back, CColor select, CColor text, CColor htext, CColor dtext)
	{ backgroundColor = back; selectionColor = select; textColor = text;
	hiliteTextColor = htext; disableTextColor = dtext;}
	
	void setFont (CFont f) { font = f; }
protected:

	CColor backgroundColor;
	CColor selectionColor;
	CColor textColor;
	CColor hiliteTextColor;
	CColor disableTextColor;
	CFont font;

	virtual void drawItemBack (CDrawContext* pContext, const CRect& rect, bool hilite);

	#if MAC_ENABLE_MENU_SCHEME
	static pascal OSStatus eventHandler (EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData);
	void registerWithToolbox ();
	void unregisterWithToolbox ();
	#endif
};

//-----------------------------------------------------------------------------
extern COptionMenuScheme* gOptionMenuScheme;

//-----------------------------------------------------------------------------
// COptionMenu Declaration
//! a popup menu control
//-----------------------------------------------------------------------------
class COptionMenu : public CParamDisplay
{
public:
	COptionMenu (const CRect &size, CControlListener *listener, long tag,
                 CBitmap *background = 0, CBitmap *bgWhenClick = 0,
                 const long style = 0);
	virtual ~COptionMenu ();

	enum { MAX_ENTRY = 1024 };

	virtual void setValue (float val);
	virtual bool addEntry (COptionMenu *subMenu, char *txt);
	virtual	bool addEntry (char *txt, long index = -1);
	virtual	long getCurrent (char *txt = 0, bool countSeparator = true) const;
	virtual	bool setCurrent (long index, bool countSeparator = true);
	virtual	bool getEntry (long index, char *txt) const;
	virtual	bool setEntry (long index, char *txt);
	virtual	bool removeEntry (long index);
	virtual	bool removeAllEntry ();
	virtual long getNbEntries () const { return nbEntries; }
	virtual long getIndex (char *txt) const;

	virtual bool checkEntry (long index, bool state);
	virtual bool checkEntryAlone (long index);
	virtual bool isCheckEntry (long index) const;

	virtual	void draw (CDrawContext *pContext);
	virtual	void mouse (CDrawContext *pContext, CPoint &where, long button = -1);

	virtual	void takeFocus (CDrawContext *pContext = 0);
	virtual	void looseFocus (CDrawContext *pContext = 0);

	virtual void setNbItemsPerColumn (long val) { nbItemsPerColumn = val; }
	virtual long getNbItemsPerColumn () const { return nbItemsPerColumn; }

#if MOTIF
	void    setCurrentSelected (void *itemSelected);
#elif MAC
	short   getMenuID () const { return menuID; }
#endif

	long getLastResult () const { return lastResult; }
	COptionMenu *getLastItemMenu (long &idxInMenu) const;

	void setScheme (COptionMenuScheme* s) { scheme = s; }
	virtual COptionMenuScheme* getScheme () const { return scheme; }

	virtual void setPrefixNumbers (long preCount);

	COptionMenu* getSubMenu (long idx) const;

	CLASS_METHODS(COptionMenu, CParamDisplay)

protected:
	COptionMenu *getItemMenu (long idx, long &idxInMenu, long &offsetIdx);
	void    removeItems ();
	void    *appendItems (long &offsetIdx);

	void    *platformControl;

	bool   allocateMenu (long nb);
	bool   allocateSubMenu (long nb);

	char    **entry;
	COptionMenu **submenuEntry;
	bool    *check;

#if MOTIF
	void    *itemWidget[MAX_ENTRY];
#elif MAC
	short   menuID;
#endif

	long     nbEntries;
	long     nbSubMenus;
	long     currentIndex;
	CBitmap *bgWhenClick;
	long     lastButton;
	long     nbItemsPerColumn;
	long     nbAllocated;
	long     nbSubMenuAllocated;
	long	 lastResult;
	long	 prefixNumbers;
	COptionMenu *lastMenu;
	COptionMenuScheme* scheme;
};


//-----------------------------------------------------------------------------
// CKnob Declaration
//! a knob control
//-----------------------------------------------------------------------------
class CKnob : public CControl
{
public:
	CKnob (const CRect &size, CControlListener *listener, long tag, 
           CBitmap *background, CBitmap *handle, const CPoint &offset);
	virtual ~CKnob ();

	virtual void draw (CDrawContext *pContext);
	virtual	void mouse (CDrawContext *pContext, CPoint &where, long button = -1);
	virtual bool onWheel (CDrawContext *pContext, const CPoint &where, float distance);
	virtual long onKeyDown (VstKeyCode& keyCode);

	virtual void drawHandle (CDrawContext *pContext);

	virtual void  setStartAngle (float val);
	virtual float getStartAngle () const { return startAngle; }

	virtual void  setRangeAngle (float val);
	virtual float getRangeAngle () const { return rangeAngle; }

	virtual void  valueToPoint (CPoint &point) const;
	virtual float valueFromPoint (CPoint &point) const;

	virtual void setInsetValue (long val) { inset = val; }

	virtual void setColorShadowHandle (CColor color);
	virtual void setColorHandle (CColor color);

	virtual void setHandleBitmap (CBitmap *bitmap);

	virtual void  setZoomFactor (float val) { zoomFactor = val; }
	virtual float getZoomFactor () const { return zoomFactor; }

	CLASS_METHODS(CKnob, CControl)

protected:
	void compute ();

	CPoint   offset;
	CColor   colorHandle, colorShadowHandle;

	CBitmap *pHandle;
	long     inset;
	float    startAngle, rangeAngle, halfAngle;
	float    aCoef, bCoef;
	float    radius;
	float    zoomFactor;
};

//-----------------------------------------------------------------------------
// CAnimKnob Declaration
//! a bitmap knob control
//-----------------------------------------------------------------------------
class CAnimKnob : public CKnob
{
public:
	CAnimKnob (const CRect &size, CControlListener *listener, long tag, 
               CBitmap *background, CPoint &offset);
	CAnimKnob (const CRect &size, CControlListener *listener, long tag, 
               long subPixmaps,        // number of subPixmaps
               CCoord heightOfOneImage,  // pixel
               CBitmap *background, CPoint &offset);
	virtual ~CAnimKnob ();

	virtual bool isDirty () const;

	virtual void draw (CDrawContext* pContext);

	void setInverseBitmap (bool val) { bInverseBitmap = val; }

	CLASS_METHODS(CAnimKnob, CKnob)

protected:
	long	subPixmaps;		// number of subPixmaps
	CCoord	heightOfOneImage;
	bool	bInverseBitmap;
	CPoint	lastDrawnPoint;
};

//-----------------------------------------------------------------------------
// CVerticalSwitch Declaration
//! a vertical switch control
//-----------------------------------------------------------------------------
class CVerticalSwitch : public CControl
{
public:
	CVerticalSwitch (const CRect &size, CControlListener *listener, long tag, 
                     CBitmap *background, CPoint &offset);
	CVerticalSwitch (const CRect &size, CControlListener *listener, long tag, 
                     long subPixmaps,         // number of subPixmaps
                     CCoord heightOfOneImage,   // pixel
                     long iMaxPositions,
                     CBitmap *background, CPoint &offset);
	virtual ~CVerticalSwitch ();

	virtual void draw (CDrawContext*);
	virtual void mouse (CDrawContext *pContext, CPoint &where, long button = -1);

	CLASS_METHODS(CVerticalSwitch, CControl)

protected:
	CPoint	offset;
	long	subPixmaps;            // number of subPixmaps
	CCoord	heightOfOneImage;
	long	iMaxPositions;
};


//-----------------------------------------------------------------------------
// CHorizontalSwitch Declaration
//! a horizontal switch control
//-----------------------------------------------------------------------------
class CHorizontalSwitch : public CControl
{
public:
	CHorizontalSwitch (const CRect &size, CControlListener *listener, long tag, 
                       CBitmap *background, CPoint &offset);
	CHorizontalSwitch (const CRect &size, CControlListener *listener, long tag, 
                       long subPixmaps,        // number of subPixmaps
                       CCoord heightOfOneImage,  // pixel
                       long iMaxPositions,
                       CBitmap *background, CPoint &offset);
	virtual	~CHorizontalSwitch ();

	virtual void draw (CDrawContext*);
	virtual void mouse (CDrawContext *pContext, CPoint &where, long button = -1);

	CLASS_METHODS(CHorizontalSwitch, CControl)

protected:
	CPoint	offset;
	long	subPixmaps;        // number of subPixmaps
	long	iMaxPositions;
	CCoord	heightOfOneImage;
};


//-----------------------------------------------------------------------------
// CRockerSwitch Declaration
//! a switch control with 3 sub bitmaps
//-----------------------------------------------------------------------------
class CRockerSwitch : public CControl
{
public:
	CRockerSwitch (const CRect &size, CControlListener *listener, long tag, 
                   CBitmap *background, CPoint &offset, const long style = kHorizontal);
	CRockerSwitch (const CRect &size, CControlListener *listener, long tag, 
                   CCoord heightOfOneImage,  // pixel
                   CBitmap *background, CPoint &offset, const long style = kHorizontal);
	virtual ~CRockerSwitch ();

	virtual void draw (CDrawContext*);
	virtual void mouse (CDrawContext *pContext, CPoint &where, long button = -1);
	virtual bool onWheel (CDrawContext *pContext, const CPoint &where, float distance);

	CLASS_METHODS(CRockerSwitch, CControl)

protected:
	CPoint	offset;
	CCoord	heightOfOneImage;
	long	style;
};


//-----------------------------------------------------------------------------
// CMovieBitmap Declaration
//! a bitmap control that displays different bitmaps according to its current value
//-----------------------------------------------------------------------------
class CMovieBitmap : public CControl
{
public:
	CMovieBitmap (const CRect &size, CControlListener *listener, long tag, 
                  CBitmap *background, CPoint &offset);
	CMovieBitmap (const CRect &size, CControlListener *listener, long tag, 
                  long subPixmaps,        // number of subPixmaps
                  CCoord heightOfOneImage,  // pixel
                  CBitmap *background, CPoint &offset);
	virtual	~CMovieBitmap ();

	virtual void draw (CDrawContext*);

	CLASS_METHODS(CMovieBitmap, CControl)

protected:
	CPoint	offset;
	long	subPixmaps;         // number of subPixmaps
	CCoord	heightOfOneImage;
};


//-----------------------------------------------------------------------------
// CMovieButton Declaration
//! a bi-states button with 2 subbitmaps
//-----------------------------------------------------------------------------
class CMovieButton : public CControl
{
public:
	CMovieButton (const CRect &size, CControlListener *listener, long tag, 
                  CBitmap *background, CPoint &offset);
	CMovieButton (const CRect &size, CControlListener *listener, long tag, 
                  CCoord heightOfOneImage,  // pixel
                  CBitmap *background, CPoint &offset);
	virtual ~CMovieButton ();	

	virtual void draw (CDrawContext*);
	virtual void mouse (CDrawContext *pContext, CPoint &where, long button = -1);

	CLASS_METHODS(CMovieButton, CControl)

protected:
	CPoint   offset;
	CCoord   heightOfOneImage;
	float    buttonState;
};


//-----------------------------------------------------------------------------
// CAutoAnimation Declaration
//!
//-----------------------------------------------------------------------------
class CAutoAnimation : public CControl
{
public:
	CAutoAnimation (const CRect &size, CControlListener *listener, long tag, 
                    CBitmap *background, CPoint &offset);
	CAutoAnimation (const CRect &size, CControlListener *listener, long tag, 
                    long subPixmaps,        // number of subPixmaps...
                    CCoord heightOfOneImage,  // pixel
                    CBitmap *background, CPoint &offset);
	virtual ~CAutoAnimation ();

	virtual void draw (CDrawContext*);
	virtual void mouse (CDrawContext *pContext, CPoint &where, long button = -1);

	virtual void openWindow (void);
	virtual void closeWindow (void);

	virtual void nextPixmap (void);
	virtual void previousPixmap (void);

	bool    isWindowOpened () const { return bWindowOpened; }

	CLASS_METHODS(CAutoAnimation, CControl)

protected:
	CPoint	offset;

	long	subPixmaps;
	CCoord	heightOfOneImage;
	CCoord	totalHeightOfBitmap;

	bool	bWindowOpened;
};


//-----------------------------------------------------------------------------
// CSlider Declaration
//! a slider control
//-----------------------------------------------------------------------------
class CSlider : public CControl
{
public:
	CSlider (const CRect &size, CControlListener *listener, long tag, 
             long    iMinPos,     // min position in pixel
             long    iMaxPos,     // max position in pixel
             CBitmap *handle,     // handle bitmap
             CBitmap *background, // background bitmap
             CPoint  &offset,     // offset in the background
             const long style = kLeft|kHorizontal); // style (kBottom,kRight,kTop,kLeft,kHorizontal,kVertical)

	CSlider (const CRect &rect, CControlListener *listener, long tag,
             CPoint   &offsetHandle, // handle offset
             long     rangeHandle,   // size of handle range
             CBitmap  *handle,       // handle bitmap
             CBitmap  *background,   // background bitmap
             CPoint   &offset,       // offset in the background
             const long style = kLeft|kHorizontal);  // style (kBottom,kRight,kTop,kLeft,kHorizontal,kVertical)

	virtual ~CSlider ();
  
	virtual bool attached (CView *parent);
	virtual bool removed (CView *parent);
	virtual void draw (CDrawContext*);
	virtual void mouse (CDrawContext *pContext, CPoint &where, long button = -1);
	virtual bool onWheel (CDrawContext *pContext, const CPoint &where, float distance);
	virtual long onKeyDown (VstKeyCode& keyCode);

	virtual void setDrawTransparentHandle (bool val) { bDrawTransparentEnabled = val; }
	virtual void setFreeClick (bool val) { bFreeClick = val; }
	virtual bool getFreeClick () const { return bFreeClick; }
	virtual void setOffsetHandle (CPoint &val);

	virtual void     setHandle (CBitmap* pHandle);
	virtual CBitmap *getHandle () const { return pHandle; }

	virtual void  setZoomFactor (float val) { zoomFactor = val; }
	virtual float getZoomFactor () const { return zoomFactor; }

	CLASS_METHODS(CSlider, CControl)

protected:
	CPoint   offset; 
	CPoint   offsetHandle;

	CBitmap *pHandle;
	COffscreenContext *pOScreen;

	long	style;

	CCoord	widthOfSlider; // size of the handle-slider
	CCoord	heightOfSlider;
	CCoord	rangeHandle;
	CCoord	minTmp;
	CCoord	maxTmp;
	CCoord	minPos;
	CCoord	widthControl;
	CCoord	heightControl;
	float	zoomFactor;

	bool     bDrawTransparentEnabled;
	bool     bFreeClick;
};

//-----------------------------------------------------------------------------
// CVerticalSlider Declaration
//! a vertical slider control
//-----------------------------------------------------------------------------
class CVerticalSlider : public CSlider
{
public:
	CVerticalSlider (const CRect &size, CControlListener *listener, long tag, 
                     long    iMinPos,    // min Y position in pixel
                     long    iMaxPos,    // max Y position in pixel
                     CBitmap *handle,     // bitmap slider
                     CBitmap *background, // bitmap background
                     CPoint  &offset,     // offset in the background
                     const long style = kBottom); // style (kBottom, kTop))

	CVerticalSlider (const CRect &rect, CControlListener *listener, long tag,
                     CPoint   &offsetHandle, // handle offset
                     long     rangeHandle,   // size of handle range
                     CBitmap  *handle,     // bitmap of slider
                     CBitmap  *background, // bitmap of background
                     CPoint   &offset,     // offset in the background
                     const long style = kBottom);  // style (kBottom, kTop)
};

//-----------------------------------------------------------------------------
// CHorizontalSlider Declaration
//! a horizontal slider control
//-----------------------------------------------------------------------------
class CHorizontalSlider : public CSlider
{
public:
	CHorizontalSlider (const CRect &size, CControlListener *listener, long tag, 
                       long    iMinPos,    // min X position in pixel
                       long    iMaxPos,    // max X position in pixel
                       CBitmap *handle,     // bitmap slider
                       CBitmap *background, // bitmap background	
                       CPoint  &offset,     // offset in the background
                       const long style = kRight); // style (kRight, kLeft)
  
	CHorizontalSlider (const CRect &rect, CControlListener *listener, long tag,
                       CPoint   &offsetHandle, // handle offset
                       long     rangeHandle,   // size of handle range
                       CBitmap  *handle,     // bitmap of slider
                       CBitmap  *background, // bitmap of background
                       CPoint   &offset,     // offset in the background
                       const long style = kRight);  // style (kRight, kLeft)
};


//-----------------------------------------------------------------------------
// CSpecialDigit Declaration
//! special display with custom numbers (0...9)
//-----------------------------------------------------------------------------
class CSpecialDigit : public CControl
{
public:
	CSpecialDigit (const CRect &size, CControlListener *listener, long tag, // tag identifier
                   long     dwPos,     // actual value
                   long     iNumbers,  // amount of numbers (max 7)
                   long     *xpos,     // array of all XPOS
                   long     *ypos,     // array of all YPOS
                   long     width,     // width of ONE number
                   long     height,    // height of ONE number
                   CBitmap  *background);  // bitmap numbers
	virtual ~CSpecialDigit ();
	
	virtual void  draw (CDrawContext*);

	virtual float getNormValue (void) const;

	CLASS_METHODS(CSpecialDigit, CControl)

protected:
	long     iNumbers;   // amount of numbers
	long     xpos[7];    // array of all XPOS, max 7 possible
	long     ypos[7];    // array of all YPOS, max 7 possible
	long     width;      // width  of ONE number
	long     height;     // height of ONE number
};


//-----------------------------------------------------------------------------
// CKickButton Declaration
//!
//-----------------------------------------------------------------------------
class CKickButton : public CControl
{
public:
	CKickButton (const CRect &size, CControlListener *listener, long tag, 
                 CBitmap *background, CPoint &offset);
	CKickButton (const CRect &size, CControlListener *listener, long tag, 
                 CCoord heightOfOneImage,  // pixel
                 CBitmap *background, CPoint &offset);
	virtual ~CKickButton ();	

	virtual void draw (CDrawContext*);
	virtual void mouse (CDrawContext *pContext, CPoint &where, long button = -1);

	CLASS_METHODS(CKickButton, CControl)

protected:
	CPoint	offset;
	CCoord	heightOfOneImage;
};


//-----------------------------------------------------------------------------
// CSplashScreen Declaration
//!
//-----------------------------------------------------------------------------
class CSplashScreen : public CControl
{
public:
	CSplashScreen (const CRect &size, CControlListener *listener, long tag, 
                   CBitmap *background,
                   CRect &toDisplay, 
                   CPoint &offset);
	virtual ~CSplashScreen ();	
  
	virtual void draw (CDrawContext*);
	virtual bool hitTest (const CPoint& where, const long buttons = -1);
	virtual void mouse (CDrawContext *pContext, CPoint &where, long button = -1);
	virtual void unSplash ();

	void setBitmapTransparency (unsigned char transparency);

	CLASS_METHODS(CSplashScreen, CControl)

protected:
	CRect	toDisplay;
	CRect	keepSize;
	CPoint	offset;
	unsigned char bitmapTransparency;
};


//-----------------------------------------------------------------------------
// CVuMeter Declaration
//!
//-----------------------------------------------------------------------------
class CVuMeter : public CControl
{
public:
	CVuMeter (const CRect& size, CBitmap *onBitmap, CBitmap *offBitmap,
              long nbLed, const long style = kVertical);
	virtual ~CVuMeter ();	
  
	virtual void setDecreaseStepValue (float value) { decreaseValue = value; }

	virtual bool attached (CView *parent);
	virtual bool removed (CView *parent);
	virtual void draw (CDrawContext *pContext);
	virtual void setDirty (const bool val = true);
	
	void setUseOffscreen (bool val = true);
	bool getUseOffscreen () const { return bUseOffscreen; }

	CLASS_METHODS(CVuMeter, CControl)

protected:
	CBitmap *onBitmap;
	CBitmap *offBitmap;
	COffscreenContext *pOScreen;
	
	long     nbLed;
	long     style;
	float    decreaseValue;
	bool	 bUseOffscreen;

	CRect    rectOn;
	CRect    rectOff;
};


#if PLUGGUI
struct VstFileSelect;
#endif

//-----------------------------------------------------------------------------
// CFileSelector Declaration
//!
//-----------------------------------------------------------------------------
class CFileSelector
{
public:
	#if PLUGGUI
	CFileSelector (void*);
	#else
	CFileSelector (AudioEffectX* effect);
	#endif
	virtual ~CFileSelector ();

	long run (VstFileSelect *vstFileSelect);

protected:
	#if !PLUGGUI
	AudioEffectX* effect;
	#endif
	VstFileSelect *vstFileSelect;

	#if MAC && TARGET_API_MAC_CARBON
	static pascal void navEventProc (const NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD);
	static pascal Boolean navObjectFilterProc (AEDesc *theItem, void *info, void *callBackUD, NavFilterModes filterMode);
	#endif
};

#if PLUGGUI
struct VstFileType
{
	VstFileType (char* _name, char *_macType, char *_dosType, char *_unixType = 0, char *_mimeType1 = 0, char *_mimeType2 = 0)
	{
		if (_name)
			strcpy (name, _name);
		if (_macType)
			strcpy (macType, _macType);
		if (_dosType)
			strcpy (dosType, _dosType);
		if (_unixType)
			strcpy (unixType, _unixType);
		if (_mimeType1)
			strcpy (mimeType1, _mimeType1);
		if (_mimeType2)
			strcpy (mimeType2, _mimeType2);
	}
	char name[128];
	char macType[8];
	char dosType[8];
	char unixType[8];
	char mimeType1[128];
	char mimeType2[128];
};

struct VstFileSelect
{
	long command;           // see enum kVstFileLoad....
	long type;              // see enum kVstFileType...

	long macCreator;        // optional: 0 = no creator

	long nbFileTypes;       // nb of fileTypes to used
	VstFileType *fileTypes; // list of fileTypes

	char title[1024];       // text display in the file selector's title

	char *initialPath;      // initial path

	char *returnPath;       // use with kVstFileLoad and kVstDirectorySelect
							// if null is passed, the host will allocated memory
							// the plugin should then called closeOpenFileSelector for freeing memory
	long sizeReturnPath; 

	char **returnMultiplePaths; // use with kVstMultipleFilesLoad
								// the host allocates this array. The plugin should then called closeOpenFileSelector for freeing memory
	long nbReturnPath;			// number of selected paths

	long reserved;				// reserved for host application
	char future[116];			// future use
};

enum {
	kVstFileLoad = 0,
	kVstFileSave,
	kVstMultipleFilesLoad,
	kVstDirectorySelect,

	kVstFileType = 0
};
#endif

END_NAMESPACE_VSTGUI

#endif	// __vstcontrol__
