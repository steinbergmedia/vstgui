//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
// Standard Control Objects
//
// Version 2.2         Date : 25/03/03
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
	kHorizontal = 1 << 0,
	kVertical   = 1 << 1,
	kShadowText = 1 << 2,
	kLeft       = 1 << 3,
	kRight      = 1 << 4,
	kTop        = 1 << 5,
	kBottom     = 1 << 6,
	k3DIn       = 1 << 7,
	k3DOut      = 1 << 8,
	kPopupStyle = 1 << 9,
	kCheckStyle = 1 << 10,
	kMultipleCheckStyle,
	kNoTextStyle = 1 << 11,
	kNoDrawStyle = 1 << 12,
	kDoubleClickStyle = 1 << 13
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
	#else
	virtual void valueChanged (CDrawContext *pContext, CControl *pControl) = 0;
	#endif
};

class AudioEffectX;

//-----------------------------------------------------------------------------
BEGIN_NAMESPACE_VSTGUI
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CControl : public CView
{
public:
	CControl (const CRect &size, CControlListener *listener = 0, long tag = 0, 
						CBitmap *pBackground = 0);
	virtual ~CControl ();

	virtual void  draw (CDrawContext *pContext) = 0;
	virtual void  doIdleStuff () { if (pParent) pParent->doIdleStuff (); }

	virtual void  setValue (float val) { value = val; }
	virtual float getValue () { return value; };

	virtual void  setMin (float val) { vmin = val; }
	virtual float getMin () { return vmin; }
	virtual void  setMax (float val) { vmax = val; }
	virtual float getMax () { return vmax; }

	virtual void  setOldValue (float val) { oldValue = val; }
	virtual	float getOldValue (void) { return oldValue; }
	virtual void  setDefaultValue (float val) { defaultValue = val; }
	virtual	float getDefaultValue (void) { return defaultValue; }

	virtual void  setTag (long val) { tag = val; }
	inline  long  getTag () { return tag; }

	virtual bool  isDirty ();
	virtual void  setDirty (const bool val = true);

	virtual void     setBackground (CBitmap* pBackground);
	virtual CBitmap *getBackground () { return pBackground; }

	virtual void setBackOffset (CPoint &offset);
	virtual void copyBackOffset ();

	virtual void  setWheelInc (float val) { wheelInc = val; }
	virtual float getWheelInc () { return wheelInc; }

	virtual void bounceValue ();

	CControlListener* getListener () { return listener; }
	bool isDoubleClick ();

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

	CBitmap *pBackground;
	CPoint	backOffset;
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class COnOffButton : public CControl
{
public:
	COnOffButton (const CRect &size, CControlListener *listener, long tag,
                  CBitmap *background);
	virtual ~COnOffButton ();

	virtual void draw (CDrawContext*);
	virtual void mouse (CDrawContext *pContext, CPoint &where);
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CParamDisplay : public CControl
{
public:
	CParamDisplay (const CRect &size, CBitmap *background = 0, const long style = 0);
	virtual ~CParamDisplay ();
	
	virtual void setFont (CFont fontID);
	CFont getFont () { return fontID; }

	virtual void setFontColor (CColor color);
	CColor getFontColor () { return fontColor; }

	virtual void setBackColor (CColor color);
	CColor getBackColor () { return backColor; }

	virtual void setFrameColor (CColor color);
	CColor getFrameColor () { return frameColor; }

	virtual void setShadowColor (CColor color);
	CColor getShadowColor () { return shadowColor; }

	virtual void setHoriAlign (CHoriTxtAlign hAlign);

	virtual void setStringConvert (void (*convert) (float value, char *string));
	virtual void setStringConvert (void (*convert) (float value, char *string, void *userDta),
									void *userData);
	virtual void setString2FloatConvert (void (*convert) (char *string, float &output));

	virtual void setStyle (long val);
	long getStyle () { return style; }

	virtual void setTxtFace (CTxtFace val);
	CTxtFace getTxtFace () { return txtFace; }

	virtual void draw (CDrawContext *pContext);

	virtual void setTextTransparency (bool val) { bTextTransparencyEnabled = val; }
	bool getTextTransparency () { return bTextTransparencyEnabled; }

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
//-----------------------------------------------------------------------------
class CTextEdit : public CParamDisplay
{
public:
	CTextEdit (const CRect &size, CControlListener *listener, long tag, const char *txt = 0,
               CBitmap *background = 0, const long style = 0);
	virtual ~CTextEdit ();

	virtual void setText (char *txt);
	virtual void getText (char *txt);

	virtual	void draw (CDrawContext *pContext);
	virtual	void mouse (CDrawContext *pContext, CPoint &where);

	virtual void setTextEditConvert (void (*editConvert) (char *input, char *string));
	virtual void setTextEditConvert (void (*editConvert2) (char *input, char *string,
										void *userDta), void *userData);

	virtual	void takeFocus (CDrawContext *pContext = 0);
	virtual	void looseFocus (CDrawContext *pContext = 0);

	void *platformFontColor;
	void *platformControl;
	bool bWasReturnPressed;

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
//-----------------------------------------------------------------------------
class COptionMenuScheme
{
public:
	COptionMenuScheme ();
	virtual ~COptionMenuScheme () {}

	enum { kChecked = 0x01, kDisabled = 0x02, kSelected = 0x04 };

	virtual void getItemSize (const char* text, CDrawContext* pContext, CPoint& size);
	virtual void drawItem (const char* text, long itemId, long state, CDrawContext* pContext, const CRect& rect);	

	void setColors (CColor back, CColor select, CColor text, CColor htext, CColor dtext)
	{ backgroundColor = back; selectionColor = select; textColor = text;
	hiliteTextColor = htext; disableTextColor = dtext;}
	
	void setFont (CFont f) { font = f; }

	virtual void forget ();
	virtual void remember ();
	virtual	long getNbReference () { return nbReference; }
protected:
	long nbReference;

	CColor backgroundColor;
	CColor selectionColor;
	CColor textColor;
	CColor hiliteTextColor;
	CColor disableTextColor;
	CFont font;

	virtual void drawItemBack (CDrawContext* pContext, const CRect& rect, bool hilite);
};


//-----------------------------------------------------------------------------
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
	virtual	long getCurrent (char *txt = 0, bool countSeparator = true);
	virtual	bool setCurrent (long index, bool countSeparator = true);
	virtual	bool getEntry (long index, char *txt);
	virtual	bool setEntry (long index, char *txt);
	virtual	bool removeEntry (long index);
	virtual	bool removeAllEntry ();
	virtual long getNbEntries () { return nbEntries; }
	virtual long getIndex (char *txt);

	virtual bool checkEntry (long index, bool state);
	virtual bool checkEntryAlone (long index);
	virtual bool isCheckEntry (long index);

	virtual	void draw (CDrawContext *pContext);
	virtual	void mouse (CDrawContext *pContext, CPoint &where);

	virtual	void takeFocus (CDrawContext *pContext = 0);
	virtual	void looseFocus (CDrawContext *pContext = 0);

	virtual void setNbItemsPerColumn (long val) { nbItemsPerColumn = val; }
	virtual long getNbItemsPerColumn () { return nbItemsPerColumn; }

#if MOTIF
	void    setCurrentSelected (void *itemSelected);
#elif MAC
	short   getMenuID () { return menuID; }
#endif

	long getLastResult () { return lastResult; }
	COptionMenu *getLastItemMenu (long &idxInMenu);

	void setScheme (COptionMenuScheme* s) { scheme = s; }
	virtual COptionMenuScheme* getScheme () { return scheme; }

	virtual void setPrefixNumbers (long preCount);

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
//-----------------------------------------------------------------------------
class CKnob : public CControl
{
public:
	CKnob (const CRect &size, CControlListener *listener, long tag, 
           CBitmap *background,
           CBitmap *handle, const CPoint &offset);
	virtual ~CKnob ();

	virtual void draw (CDrawContext *pContext);
	virtual	void mouse (CDrawContext *pContext, CPoint &where);
	virtual bool onWheel (CDrawContext *pContext, const CPoint &where, float distance);
	virtual long onKeyDown (VstKeyCode& keyCode);

	virtual void drawHandle (CDrawContext *pContext);

	virtual void  setStartAngle (float val);
	virtual float getStartAngle () { return startAngle; }

	virtual void  setRangeAngle (float val);
	virtual float getRangeAngle () { return rangeAngle; }

	virtual void  valueToPoint (CPoint &point);
	virtual float valueFromPoint (CPoint &point);

	virtual void setInsetValue (long val) { inset = val; }

	virtual void setColorShadowHandle (CColor color);
	virtual void setColorHandle (CColor color);

	virtual void setHandleBitmap (CBitmap *bitmap);

	virtual void  setZoomFactor (float val) { zoomFactor = val; }
	virtual float getZoomFactor () { return zoomFactor; }

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
//-----------------------------------------------------------------------------
class CAnimKnob : public CKnob
{
public:
	CAnimKnob (const CRect &size, CControlListener *listener, long tag, 
               long subPixmaps,        // number of subPixmaps
               long heightOfOneImage,  // pixel
               CBitmap *background, CPoint &offset);
	virtual ~CAnimKnob ();

	virtual void draw (CDrawContext* pContext);

	void setInverseBitmap (bool val) { bInverseBitmap = val; }

protected:
	long subPixmaps;		// number of subPixmaps
	long heightOfOneImage;
	bool bInverseBitmap;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CVerticalSwitch : public CControl
{
public:
	CVerticalSwitch (const CRect &size, CControlListener *listener, long tag, 
                     long subPixmaps,         // number of subPixmaps
                     long heightOfOneImage,   // pixel
                     long iMaxPositions,
                     CBitmap *background, CPoint &offset);
	virtual ~CVerticalSwitch ();

	virtual void draw (CDrawContext*);
	virtual void mouse (CDrawContext *pContext, CPoint &where);

protected:
	CPoint   offset;
	long     subPixmaps;            // number of subPixmaps
	long     heightOfOneImage;
	long     iMaxPositions;
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CHorizontalSwitch : public CControl
{
public:
	CHorizontalSwitch (const CRect &size, CControlListener *listener, long tag, 
                       long subPixmaps,        // number of subPixmaps
                       long heightOfOneImage,  // pixel
                       long iMaxPositions,
                       CBitmap *background,
                       CPoint &offset);
	virtual	~CHorizontalSwitch ();

	virtual void draw (CDrawContext*);
	virtual void mouse (CDrawContext *pContext, CPoint &where);

protected:
	CPoint   offset;
	long     subPixmaps;        // number of subPixmaps
	long     heightOfOneImage;
	long     iMaxPositions;
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CRockerSwitch : public CControl
{
public:
	CRockerSwitch (const CRect &size, CControlListener *listener, long tag, 
                   long heightOfOneImage,  // pixel
                   CBitmap *background, CPoint &offset, const long style = kHorizontal);
	virtual ~CRockerSwitch ();

	virtual void draw (CDrawContext*);
	virtual void mouse (CDrawContext *pContext, CPoint &where);
	virtual bool onWheel (CDrawContext *pContext, const CPoint &where, float distance);

protected:
	CPoint   offset;
	long     heightOfOneImage;
	long     style;
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CMovieBitmap : public CControl
{
public:
	CMovieBitmap (const CRect &size, CControlListener *listener, long tag, 
                  long subPixmaps,        // number of subPixmaps
                  long heightOfOneImage,  // pixel
                  CBitmap *background, CPoint &offset);
	virtual	~CMovieBitmap ();

	virtual void draw (CDrawContext*);

protected:
	CPoint   offset;
	long     subPixmaps;         // number of subPixmaps
	long     heightOfOneImage;
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CMovieButton : public CControl
{
public:
	CMovieButton (const CRect &size, CControlListener *listener, long tag, 
                  long heightOfOneImage,  // pixel
                  CBitmap *background, CPoint &offset);
	virtual ~CMovieButton ();	

	virtual void draw (CDrawContext*);
	virtual void mouse (CDrawContext *pContext, CPoint &where);

protected:
	CPoint   offset;
	long     heightOfOneImage;
	float    buttonState;
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CAutoAnimation : public CControl
{
public:
	CAutoAnimation (const CRect &size, CControlListener *listener, long tag, 
                    long subPixmaps,        // number of subPixmaps...
                    long heightOfOneImage,  // pixel
                    CBitmap *background, CPoint &offset);
	virtual ~CAutoAnimation ();

	virtual void draw (CDrawContext*);
	virtual void mouse (CDrawContext *pContext, CPoint &where);

	virtual void openWindow (void);
	virtual void closeWindow (void);

	virtual void nextPixmap (void);
	virtual void previousPixmap (void);

	bool    isWindowOpened () { return bWindowOpened; }

protected:
	CPoint   offset;

	long     subPixmaps;
	long     heightOfOneImage;
	long     totalHeightOfBitmap;

	bool     bWindowOpened;
};


//-----------------------------------------------------------------------------
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
	virtual void mouse (CDrawContext *pContext, CPoint &where);
	virtual bool onWheel (CDrawContext *pContext, const CPoint &where, float distance);
	virtual long onKeyDown (VstKeyCode& keyCode);

	virtual void setDrawTransparentHandle (bool val) { bDrawTransparentEnabled = val; }
	virtual void setFreeClick (bool val) { bFreeClick = val; }
	virtual bool getFreeClick () { return bFreeClick; }
	virtual void setOffsetHandle (CPoint &val);

	virtual void     setHandle (CBitmap* pHandle);
	virtual CBitmap *getHandle () { return pHandle; }

	virtual void  setZoomFactor (float val) { zoomFactor = val; }
	virtual float getZoomFactor () { return zoomFactor; }

protected:
	CPoint   offset; 
	CPoint   offsetHandle;

	CBitmap *pHandle;
	COffscreenContext *pOScreen;

	long     widthOfSlider; // size of the handle-slider
	long     heightOfSlider;

	long     rangeHandle;
	long     style;

	long     minTmp;
	long     maxTmp;
	long     minPos;
	long     widthControl;
	long     heightControl;
	float    zoomFactor;

	bool     bDrawTransparentEnabled;
	bool     bFreeClick;
};

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
// special display with custom numbers (0...9)
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

	virtual float getNormValue (void);

protected:
	long     iNumbers;   // amount of numbers
	long     xpos[7];    // array of all XPOS, max 7 possible
	long     ypos[7];    // array of all YPOS, max 7 possible
	long     width;      // width  of ONE number
	long     height;     // height of ONE number
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CKickButton : public CControl
{
public:
	CKickButton (const CRect &size, CControlListener *listener, long tag, 
                 long heightOfOneImage,  // pixel
                 CBitmap *background, CPoint &offset);
	virtual ~CKickButton ();	

	virtual void draw (CDrawContext*);
	virtual void mouse (CDrawContext *pContext, CPoint &where);

protected:
	CPoint   offset;
	long     heightOfOneImage;
};


//-----------------------------------------------------------------------------
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
	virtual void mouse (CDrawContext *pContext, CPoint &where);
	virtual void unSplash ();

protected:
	CRect    toDisplay;
	CRect    keepSize;
	CPoint   offset;
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CVuMeter : public CControl
{
public:
	CVuMeter (const CRect& size, CBitmap *onBitmap, CBitmap *offBitmap,
              long nbLed, const long style = kVertical);
	virtual ~CVuMeter ();	
  
	virtual void setDecreaseStepValue (float value) { decreaseValue = value; }

	virtual void draw (CDrawContext *pContext);
	virtual void  setDirty (const bool val = true);

protected:
	CBitmap *onBitmap;
	CBitmap *offBitmap;
	long     nbLed;
	long     style;
	float    decreaseValue;

	CRect    rectOn;
	CRect    rectOff;
};


#if !PLUGGUI
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CFileSelector
{
public:
	CFileSelector (AudioEffectX* effect);
	virtual ~CFileSelector ();

	long run (VstFileSelect *vstFileSelect);

protected:
	AudioEffectX* effect;
	VstFileSelect *vstFileSelect;
};
#endif

END_NAMESPACE_VSTGUI

#endif
