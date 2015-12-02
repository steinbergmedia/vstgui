//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
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

#include "vstguifwd.h"

#include "cpoint.h"
#include "crect.h"
#include "cfont.h"
#include "ccolor.h"
#include "cgraphicstransform.h"
#include "clinestyle.h"
#include "cdrawdefs.h"
#include <cmath>
#include <stack>
#include <vector>

namespace VSTGUI {

class CString;
struct CNinePartTiledDescription;

//-----------------------------------------------------------------------------
// CDrawContext Declaration
//! @brief A drawing context encapsulates the drawing context of the underlying OS
//-----------------------------------------------------------------------------
class CDrawContext : public CBaseObject
{
public:
	//-----------------------------------------------------------------------------
	/** Add a transform to all draw routines. Must be used as stack object. */
	//-----------------------------------------------------------------------------
	struct Transform
	{
		Transform (CDrawContext& context, const CGraphicsTransform& transformation);
		~Transform ();
		
	private:
		CDrawContext& context;
		const CGraphicsTransform transformation;
	};
	
	//-----------------------------------------------------------------------------
	/// @name Draw primitives
	//-----------------------------------------------------------------------------
	//@{
	VSTGUI_DEPRECATED(void moveTo (const CPoint &point);)	///< \deprecated use drawLine
	VSTGUI_DEPRECATED(void lineTo (const CPoint &point);)	///< \deprecated use drawLine
	VSTGUI_DEPRECATED(void getLoc (CPoint &where) const { where = currentState.penLoc; })
	VSTGUI_DEPRECATED(void drawLines (const CPoint* points, const int32_t& numberOfLines);) ///< \deprecated use drawLines (const LineList&)
	VSTGUI_DEPRECATED(void drawPolygon (const CPoint* pPoints, int32_t numberOfPoints, const CDrawStyle drawStyle = kDrawStroked)); ///< \deprecated use drawPolygon (const PointList&)

	typedef std::pair<CPoint, CPoint> LinePair;
	typedef std::vector<LinePair> LineList;
	typedef std::vector<CPoint> PointList;

	inline void drawLine (const CPoint& start, const CPoint& end) { drawLine (std::make_pair (start, end)); }
	virtual void drawLine (const LinePair& line) = 0;	///< draw a line
	virtual void drawLines (const LineList& lines) = 0;	///< draw multiple lines at once
	virtual void drawPolygon (const PointList& polygonPointList, const CDrawStyle drawStyle = kDrawStroked) = 0; ///< draw a polygon
	virtual void drawRect (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked) = 0;	///< draw a rect
	virtual void drawArc (const CRect &rect, const float startAngle1, const float endAngle2, const CDrawStyle drawStyle = kDrawStroked) = 0;	///< draw an arc, angles are in degree
	virtual void drawEllipse (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked) = 0;	///< draw an ellipse
	virtual void drawPoint (const CPoint &point, const CColor& color) = 0;	///< draw a point
	virtual void drawBitmap (CBitmap* bitmap, const CRect& dest, const CPoint& offset = CPoint (0, 0), float alpha = 1.f) = 0; ///< don't call directly, please use CBitmap::draw instead

	virtual void drawBitmapNinePartTiled (CBitmap* bitmap, const CRect& dest, const CNinePartTiledDescription& desc, float alpha = 1.f);
	virtual void fillRectWithBitmap (CBitmap* bitmap, const CRect& srcRect, const CRect& dstRect, float alpha);

	virtual void clearRect (const CRect& rect) = 0;	///< clears the rect (makes r = 0, g = 0, b = 0, a = 0)
	//@}

	//-----------------------------------------------------------------------------
	/// @name Line Mode
	//-----------------------------------------------------------------------------
	//@{
	virtual void setLineStyle (const CLineStyle& style);	///< set the current line style
	const CLineStyle& getLineStyle () const { return currentState.lineStyle; }	///< get the current line style

	virtual void setLineWidth (CCoord width);	///< set the current line width
	CCoord getLineWidth () const { return currentState.frameWidth; }	///< get the current line width
	//@}

	//-----------------------------------------------------------------------------
	/// @name Draw Mode
	//-----------------------------------------------------------------------------
	//@{
	virtual void setDrawMode (CDrawMode mode);	///< set the current draw mode, see CDrawMode
	CDrawMode getDrawMode () const { return currentState.drawMode; }	///< get the current draw mode, see CDrawMode
	//@}

	//-----------------------------------------------------------------------------
	/// @name Clipping
	//-----------------------------------------------------------------------------
	//@{
	virtual void setClipRect (const CRect &clip);	///< set the current clip
	CRect& getClipRect (CRect &clip) const;///< get the current clip
	virtual void resetClipRect ();	///< reset the clip to the default state
	//@}

	//-----------------------------------------------------------------------------
	/// @name Color
	//-----------------------------------------------------------------------------
	//@{
	virtual void setFillColor  (const CColor& color);	///< set current fill color
	CColor getFillColor () const { return currentState.fillColor; }	///< get current fill color
	virtual void setFrameColor (const CColor& color);	///< set current stroke color
	CColor getFrameColor () const { return currentState.frameColor; }///< get current stroke color
	//@}

	//-----------------------------------------------------------------------------
	/// @name Font
	//-----------------------------------------------------------------------------
	//@{
	virtual void setFontColor (const CColor& color);	///< set current font color
	CColor getFontColor () const { return currentState.fontColor; }	///< get current font color
	virtual void setFont (const CFontRef font, const CCoord& size = 0, const int32_t& style = -1); ///< set current font
	const CFontRef getFont () const { return currentState.font; }	///< get current font
	//@}
	
	//-----------------------------------------------------------------------------
	/// @name Text
	//-----------------------------------------------------------------------------
	//@{
	CCoord getStringWidth (UTF8StringPtr pStr);	///< get the width of an UTF-8 encoded string
	void drawString (UTF8StringPtr string, const CRect& _rect, const CHoriTxtAlign hAlign = kCenterText, bool antialias = true);	///< draw an UTF-8 encoded string
	void drawString (UTF8StringPtr string, const CPoint& _point, bool antialias = true);	///< draw an UTF-8 encoded string

	CCoord getStringWidth (IPlatformString* pStr);	///< get the width of a platform string
	void drawString (IPlatformString* string, const CRect& _rect, const CHoriTxtAlign hAlign = kCenterText, bool antialias = true);	///< draw a platform string
	void drawString (IPlatformString* string, const CPoint& _point, bool antialias = true);	///< draw a platform string
	//@}
	
	//-----------------------------------------------------------------------------
	/// @name Global Alpha State
	//-----------------------------------------------------------------------------
	//@{
	virtual void setGlobalAlpha (float newAlpha);	///< sets the global alpha value[0..1]
	float getGlobalAlpha () const { return currentState.globalAlpha; }	///< get current global alpha value
	//@}
	
	//-----------------------------------------------------------------------------
	/// @name Global State Stack
	//-----------------------------------------------------------------------------
	//@{
	virtual void saveGlobalState ();
	virtual void restoreGlobalState ();
	//@}

	//-----------------------------------------------------------------------------
	/// @name Transformation
	//-----------------------------------------------------------------------------
	//@{
	const CGraphicsTransform& getCurrentTransform () const;
	const CRect& getAbsoluteClipRect () const { return currentState.clipRect; }
	//@}

	//-----------------------------------------------------------------------------
	/// @name Graphics Paths
	//-----------------------------------------------------------------------------
	//@{
	virtual CGraphicsPath* createGraphicsPath () = 0;	///< create a graphics path object, you need to forget it after usage
	virtual CGraphicsPath* createTextPath (const CFontRef font, UTF8StringPtr text) = 0; ///< create a graphics path from a text

	CGraphicsPath* createRoundRectGraphicsPath (const CRect& size, CCoord radius);	///< create a rect with round corners as graphics path, you need to forget it after usage

	enum PathDrawMode
	{
		kPathFilled,
		kPathFilledEvenOdd,
		kPathStroked
	};

	virtual void drawGraphicsPath (CGraphicsPath* path, PathDrawMode mode = kPathFilled, CGraphicsTransform* transformation = 0) = 0;
	virtual void fillLinearGradient (CGraphicsPath* path, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd = false, CGraphicsTransform* transformation = 0) = 0;
	virtual void fillRadialGradient (CGraphicsPath* path, const CGradient& gradient, const CPoint& center, CCoord radius, const CPoint& originOffset = CPoint (0,0), bool evenOdd = false, CGraphicsTransform* transformation = 0) = 0;
	//@}

	virtual double getScaleFactor () const { return 1.; }

	virtual void beginDraw () {}
	virtual void endDraw () {}

	CLASS_METHODS_NOCOPY(CDrawContext, CBaseObject)
protected:
	CDrawContext (const CRect& surfaceRect);
	~CDrawContext ();

	virtual void init ();

	void pushTransform (const CGraphicsTransform& transformation);
	void popTransform ();

	const CString& getDrawString (UTF8StringPtr string);
	void clearDrawString ();

	/// @cond ignore
	struct CDrawContextState
	{
		SharedPointer<CFontDesc> font;
		CColor frameColor;
		CColor fillColor;
		CColor fontColor;
		CCoord frameWidth;
		CPoint penLoc;
		CRect clipRect;
		CLineStyle lineStyle;
		CDrawMode drawMode;
		float globalAlpha;

		CDrawContextState ();
		CDrawContextState (const CDrawContextState& state);
		CDrawContextState& operator= (const CDrawContextState& state);
	#if VSTGUI_RVALUE_REF_SUPPORT
		CDrawContextState (CDrawContextState&& state) noexcept;
		CDrawContextState& operator= (CDrawContextState&& state) noexcept;
	#endif
	};
	/// @endcond

	CString* drawStringHelper;
	CRect surfaceRect;

	CDrawContextState currentState;

private:
	std::stack<CDrawContextState> globalStatesStack;
	std::stack<CGraphicsTransform> transformStack;
};

} // namespace

#endif
