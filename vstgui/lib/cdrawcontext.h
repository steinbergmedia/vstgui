// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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

struct CNinePartTiledDescription;

//-----------------------------------------------------------------------------
// CDrawContext Declaration
//! @brief A drawing context encapsulates the drawing context of the underlying OS
//-----------------------------------------------------------------------------
class CDrawContext : public AtomicReferenceCounted
{
public:
	//-----------------------------------------------------------------------------
	/** Add a transform to all draw routines. Must be used as stack object. */
	//-----------------------------------------------------------------------------
	struct Transform
	{
		Transform (CDrawContext& context, const CGraphicsTransform& transformation);
		~Transform () noexcept;
		
	private:
		CDrawContext& context;
		const CGraphicsTransform transformation;
	};
	
	//-----------------------------------------------------------------------------
	/// @name Draw primitives
	//-----------------------------------------------------------------------------
	//@{
	using LinePair = std::pair<CPoint, CPoint>;
	using LineList = std::vector<LinePair>;
	using PointList = std::vector<CPoint>;

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

	/** returns the backend scale factor. */
	virtual double getScaleFactor () const { return 1.; }
	
	/** returns the current line size which corresponds to one pixel on screen.
	 *
	 *	do not cache this value, instead ask for it every time you need it.
	 */
	CCoord getHairlineSize () const;
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

	virtual void drawGraphicsPath (CGraphicsPath* path, PathDrawMode mode = kPathFilled, CGraphicsTransform* transformation = nullptr) = 0;
	virtual void fillLinearGradient (CGraphicsPath* path, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd = false, CGraphicsTransform* transformation = nullptr) = 0;
	virtual void fillRadialGradient (CGraphicsPath* path, const CGradient& gradient, const CPoint& center, CCoord radius, const CPoint& originOffset = CPoint (0,0), bool evenOdd = false, CGraphicsTransform* transformation = nullptr) = 0;
	//@}

	virtual void beginDraw () {}
	virtual void endDraw () {}

protected:
	explicit CDrawContext (const CRect& surfaceRect);
	~CDrawContext () noexcept override;

	virtual void init ();

	void pushTransform (const CGraphicsTransform& transformation);
	void popTransform ();

	const UTF8String& getDrawString (UTF8StringPtr string);
	void clearDrawString ();

	/// @cond ignore
	struct CDrawContextState
	{
		SharedPointer<CFontDesc> font;
		CColor frameColor {kTransparentCColor};
		CColor fillColor {kTransparentCColor};
		CColor fontColor {kTransparentCColor};
		CCoord frameWidth {0.};
		CPoint penLoc {};
		CRect clipRect {};
		CLineStyle lineStyle {kLineOnOffDash};
		CDrawMode drawMode {kAntiAliasing};
		float globalAlpha {1.f};

		CDrawContextState () = default;
		CDrawContextState (const CDrawContextState& state);
		CDrawContextState& operator= (const CDrawContextState& state) = default;
		CDrawContextState (CDrawContextState&& state) noexcept;
		CDrawContextState& operator= (CDrawContextState&& state) noexcept;
	};
	/// @endcond

	UTF8String* drawStringHelper;
	CRect surfaceRect;

	CDrawContextState currentState;

private:
	std::stack<CDrawContextState> globalStatesStack;
	std::stack<CGraphicsTransform> transformStack;
};

//-----------------------------------------------------------------------------
struct ConcatClip
{
	ConcatClip (CDrawContext& context, CRect rect)
	: context (context)
	{
		context.getClipRect (origClip);
		rect.normalize ();
		rect.bound (origClip);
		context.setClipRect (rect);
	}
	~ConcatClip () noexcept
	{
		context.setClipRect (origClip);
	}
private:
	CDrawContext& context;
	CRect origClip;
};

} // namespace

#endif
