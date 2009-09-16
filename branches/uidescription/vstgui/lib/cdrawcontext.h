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

#ifndef __cdrawcontext__
#define __cdrawcontext__

#include "vstguibase.h"
#include "cpoint.h"
#include "crect.h"
#include "cfont.h"
#include "ccolor.h"

#if VSTGUI_USES_COREGRAPHICS
#include <ApplicationServices/ApplicationServices.h>
#endif

#if WINDOWS
	#include <windows.h>

	#if GDIPLUS
	#include <objidl.h>
	#include <gdiplus.h>
	#endif
#endif // WINDOWS

BEGIN_NAMESPACE_VSTGUI
class CFrame;

//-----------
// \brief Line Style
//-----------
enum CLineStyle
{
	kLineSolid = 0,
	kLineOnOffDash
};

//-----------
// \brief Draw Mode
//-----------
enum CDrawMode
{
	kCopyMode = 0,					///< non antialiased drawing
	kOrMode,						///< not implementated on Mac OS X and GDI+ \deprecated
	kXorMode,						///< not implementated on Mac OS X and GDI+ \deprecated
	kAntialias						///< antialised drawing
};

//----------------------------
// \brief Text Alignment (Horizontal)
//----------------------------
enum CHoriTxtAlign
{
	kLeftText = 0,
	kCenterText,
	kRightText
};

//----------------------------
// \brief Draw Style
//----------------------------
enum CDrawStyle
{
	kDrawStroked = 0,
	kDrawFilled,
	kDrawFilledAndStroked
};

//-----------------------------------------------------------------------------
// CDrawContext Declaration
//! \brief A drawing context encapsulates the drawing context of the underlying OS
/// \nosubgrouping
//-----------------------------------------------------------------------------
class CDrawContext : public CBaseObject
{
public:
	//-----------------------------------------------------------------------------
	/// \name Constructor
	//-----------------------------------------------------------------------------
	//@{
	CDrawContext (CFrame *pFrame, void *pSystemContext, void *pWindow = 0);
	//@}
	~CDrawContext ();	

	//-----------------------------------------------------------------------------
	/// \name Draw primitives
	//-----------------------------------------------------------------------------
	//@{
	void moveTo (const CPoint &point);	///< move line position to point
	void lineTo (const CPoint &point);	///< draw a line from current position to point
	void drawLines (const CPoint* points, const long& numberOfLines);	///< draw multiple lines at once
	void drawPolygon (const CPoint *pPoints, long numberOfPoints, const CDrawStyle drawStyle = kDrawStroked); ///< draw a polygon
	void drawRect (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked);	///< draw a rect
	void drawArc (const CRect &rect, const float startAngle1, const float endAngle2, const CDrawStyle drawStyle = kDrawStroked);	///< draw an arc, angles are in degree
	void drawEllipse (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked);	///< draw an ellipse
	void drawPoint (const CPoint &point, CColor color);	///< draw a point
	//@}

	//-----------------------------------------------------------------------------
	/// \name Line Mode
	//-----------------------------------------------------------------------------
	//@{
	void       setLineStyle (CLineStyle style);				///< set the current line style
	CLineStyle getLineStyle () const { return lineStyle; }	///< get the current line style

	void   setLineWidth (CCoord width);						///< set the current line width
	CCoord getLineWidth () const { return frameWidth; }		///< get the current line width
	//@}

	//-----------------------------------------------------------------------------
	/// \name Draw Mode
	//-----------------------------------------------------------------------------
	//@{
	void      setDrawMode (CDrawMode mode);					///< set the current draw mode, see CDrawMode
	CDrawMode getDrawMode () const { return drawMode; }		///< get the current draw mode, see CDrawMode
	//@}

	//-----------------------------------------------------------------------------
	/// \name Clipping
	//-----------------------------------------------------------------------------
	//@{
	void   setClipRect (const CRect &clip);																			///< set the current clip
	CRect &getClipRect (CRect &clip) const { clip = clipRect; clip.offset (-offset.h, -offset.v); return clip; }	///< get the current clip
	void   resetClipRect ();																						///< reset the clip to the default state
	//@}

	//-----------------------------------------------------------------------------
	/// \name Color
	//-----------------------------------------------------------------------------
	//@{
	void   setFillColor  (const CColor color);				///< set current fill color
	CColor getFillColor () const { return fillColor; }		///< get current fill color
	void   setFrameColor (const CColor color);				///< set current stroke color
	CColor getFrameColor () const { return frameColor; }	///< get current stroke color
	//@}

	//-----------------------------------------------------------------------------
	/// \name Font
	//-----------------------------------------------------------------------------
	//@{
	void   setFontColor (const CColor color);											///< set current font color
	CColor getFontColor () const { return fontColor; }									///< get current font color
	void   setFont (const CFontRef font, const long& size = 0, const long& style = -1);	///< set current font
	const CFontRef&  getFont () const { return font; }									///< get current font
	//@}
	
	//-----------------------------------------------------------------------------
	/// \name Text
	//-----------------------------------------------------------------------------
	//@{
	CCoord getStringWidth (const char* pStr);																						///< get the width of an ASCII encoded string
	void drawString (const char *pString, const CRect &rect, const short opaque = false, const CHoriTxtAlign hAlign = kCenterText);	///< draw an ASCII encoded string
	CCoord getStringWidthUTF8 (const char* pStr);																					///< get the width of an UTF-8 encoded string
	void drawStringUTF8 (const char* pString, const CRect& rect, const CHoriTxtAlign hAlign = kCenterText, bool antialias = true);	///< draw an UTF-8 encoded string
	void drawStringUTF8 (const char* string, const CPoint& _point, bool antialias = true);											///< draw an UTF-8 encoded string
	//@}
	
	void setGlobalAlpha (float newAlpha);
	float getGlobalAlpha () const { return globalAlpha; }
	
	void *getWindow () { return pWindow; }
	void setWindow (void *ptr)  { pWindow = ptr; }
	void getLoc (CPoint &where) const { where = penLoc; }
	CFrame* getFrame () const { return pFrame; }

	CPoint offsetScreen;
	CPoint offset;

	void   *getSystemContext () const { return pSystemContext; }

	void forget ();

	//-------------------------------------------
protected:

	friend class CBitmap;
	friend class COffscreenContext;

	void   *pSystemContext;
	void   *pWindow;
	CFrame *pFrame;

	CFontRef  font;
	CColor fontColor;
	CPoint penLoc;

	CCoord   frameWidth;
	CColor frameColor;
	CColor fillColor;
	CLineStyle lineStyle;
	CDrawMode  drawMode;
	CRect  clipRect;

	float globalAlpha;

#if WINDOWS
	#if GDIPLUS
	Gdiplus::Graphics	*pGraphics;
	Gdiplus::Pen		*pPen;
	Gdiplus::SolidBrush	*pBrush;
	Gdiplus::SolidBrush	*pFontBrush;
	public:
		Gdiplus::Graphics* getGraphics () const { return pGraphics; }
		Gdiplus::Pen* getPen () const { return pPen; }
		Gdiplus::SolidBrush* getBrush () const { return pBrush; }
		Gdiplus::SolidBrush* getFontBrush () const { return pFontBrush; }
	protected:
	#else
	void *pBrush;
	void *pPen;
	void *pOldBrush;
	void *pOldPen;
	void *pOldFont;
	long iPenStyle;
	HDC  pHDC;
	#endif // GDIPLUS
#endif // WINDOWS

#if VSTGUI_USES_COREGRAPHICS
	CGContextRef gCGContext;
	bool needToSynchronizeCGContext;
	public:
	CGContextRef getCGContext () const { return gCGContext; }
	CGContextRef beginCGContext (bool swapYAxis = false);
	void releaseCGContext (CGContextRef context);
	void synchronizeCGContext ();
	
	virtual CGImageRef getCGImage () const;
	protected:
#endif

#if MAC_CARBON
	virtual BitMapPtr getBitmap ();
	virtual void releaseBitmap ();
	virtual CGrafPtr getPort ();
#endif // MAC_CARBON

};

#if VSTGUI_USES_COREGRAPHICS
	extern CGColorSpaceRef GetGenericRGBColorSpace ();

	extern CFBundleRef getBundleRef ();
	#ifndef VSTGUI_NEW_BUNDLE_REF_DEFINITION	// You can define this in your preprocessor definitions and supply the above function somewhere in your code if you don't want to use the following gBundleRef variable.
		extern void* gBundleRef;	///< must be set to the current CFBundleRef somewhere early in the code
	#endif
#endif // VSTGUI_USES_COREGRAPHICS

END_NAMESPACE_VSTGUI

#endif
