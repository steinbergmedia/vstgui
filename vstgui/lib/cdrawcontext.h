// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

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
	/** draw a line */
	virtual void drawLine (const LinePair& line) = 0;
	/** draw multiple lines at once */
	virtual void drawLines (const LineList& lines) = 0;
	/** draw a polygon */
	virtual void drawPolygon (const PointList& polygonPointList, const CDrawStyle drawStyle = kDrawStroked) = 0;
	/** draw a rect */
	virtual void drawRect (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked) = 0;
	/** draw an arc, angles are in degree */
	virtual void drawArc (const CRect &rect, const float startAngle1, const float endAngle2, const CDrawStyle drawStyle = kDrawStroked) = 0;
	/** draw an ellipse */
	virtual void drawEllipse (const CRect &rect, const CDrawStyle drawStyle = kDrawStroked) = 0;
	/** draw a point */
	virtual void drawPoint (const CPoint &point, const CColor& color) = 0;
	/** don't call directly, please use CBitmap::draw instead */
	virtual void drawBitmap (CBitmap* bitmap, const CRect& dest, const CPoint& offset = CPoint (0, 0), float alpha = 1.f) = 0;
	virtual void drawBitmapNinePartTiled (CBitmap* bitmap, const CRect& dest, const CNinePartTiledDescription& desc, float alpha = 1.f);
	virtual void fillRectWithBitmap (CBitmap* bitmap, const CRect& srcRect, const CRect& dstRect, float alpha);

	/** clears the rect (makes r = 0, g = 0, b = 0, a = 0) */
	virtual void clearRect (const CRect& rect) = 0;
	//@}

	//-----------------------------------------------------------------------------
	// @name Bitmap Interpolation Quality
	//-----------------------------------------------------------------------------
	//@{
	virtual void setBitmapInterpolationQuality (BitmapInterpolationQuality quality);	///< set the current bitmap interpolation quality
	const BitmapInterpolationQuality& getBitmapInterpolationQuality () const { return currentState.bitmapQuality; }	///< get the current bitmap interpolation quality

	//@}

	//-----------------------------------------------------------------------------
	/// @name Line Mode
	//-----------------------------------------------------------------------------
	//@{
	/** set the current line style */
	virtual void setLineStyle (const CLineStyle& style);
	/** get the current line style */
	const CLineStyle& getLineStyle () const { return currentState.lineStyle; }

	/** set the current line width */
	virtual void setLineWidth (CCoord width);
	/** get the current line width */
	CCoord getLineWidth () const { return currentState.frameWidth; }
	//@}

	//-----------------------------------------------------------------------------
	/// @name Draw Mode
	//-----------------------------------------------------------------------------
	//@{
	/** set the current draw mode, see CDrawMode */
	virtual void setDrawMode (CDrawMode mode);
	/** get the current draw mode, see CDrawMode */
	CDrawMode getDrawMode () const { return currentState.drawMode; }
	//@}

	//-----------------------------------------------------------------------------
	/// @name Clipping
	//-----------------------------------------------------------------------------
	//@{
	/** set the current clip */
	virtual void setClipRect (const CRect &clip);
	/** get the current clip */
	CRect& getClipRect (CRect &clip) const;
	/** reset the clip to the default state */
	virtual void resetClipRect ();
	//@}

	//-----------------------------------------------------------------------------
	/// @name Color
	//-----------------------------------------------------------------------------
	//@{
	/** set current fill color */
	virtual void setFillColor  (const CColor& color);
	/** get current fill color */
	CColor getFillColor () const { return currentState.fillColor; }
	/** set current stroke color */
	virtual void setFrameColor (const CColor& color);
	/** get current stroke color */
	CColor getFrameColor () const { return currentState.frameColor; }
	//@}

	//-----------------------------------------------------------------------------
	/// @name Font
	//-----------------------------------------------------------------------------
	//@{
	/** set current font color */
	virtual void setFontColor (const CColor& color);
	/** get current font color */
	CColor getFontColor () const { return currentState.fontColor; }
	/** set current font */
	virtual void setFont (const CFontRef font, const CCoord& size = 0, const int32_t& style = -1);
	/** get current font */
	const CFontRef getFont () const { return currentState.font; }
	//@}
	
	//-----------------------------------------------------------------------------
	/// @name Text
	//-----------------------------------------------------------------------------
	//@{
	/** get the width of an UTF-8 encoded string */
	CCoord getStringWidth (UTF8StringPtr pStr);
	/** draw an UTF-8 encoded string */
	void drawString (UTF8StringPtr string, const CRect& _rect, const CHoriTxtAlign hAlign = kCenterText, bool antialias = true);
	/** draw an UTF-8 encoded string */
	void drawString (UTF8StringPtr string, const CPoint& _point, bool antialias = true);

	/** get the width of a platform string */
	CCoord getStringWidth (IPlatformString* pStr);
	/** draw a platform string */
	void drawString (IPlatformString* string, const CRect& _rect, const CHoriTxtAlign hAlign = kCenterText, bool antialias = true);
	/** draw a platform string */
	void drawString (IPlatformString* string, const CPoint& _point, bool antialias = true);
	//@}
	
	//-----------------------------------------------------------------------------
	/// @name Global Alpha State
	//-----------------------------------------------------------------------------
	//@{
	/** sets the global alpha value[0..1] */
	virtual void setGlobalAlpha (float newAlpha);
	/** get current global alpha value */
	float getGlobalAlpha () const { return currentState.globalAlpha; }
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
	/** create a graphics path object, you need to forget it after usage */
	virtual CGraphicsPath* createGraphicsPath () = 0;
	/** create a graphics path from a text */
	virtual CGraphicsPath* createTextPath (const CFontRef font, UTF8StringPtr text) = 0;

	/** create a rect with round corners as graphics path, you need to forget it after usage */
	CGraphicsPath* createRoundRectGraphicsPath (const CRect& size, CCoord radius);

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

	const CRect& getSurfaceRect () const { return surfaceRect; }

protected:
	CDrawContext () = delete;
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
		BitmapInterpolationQuality bitmapQuality {BitmapInterpolationQuality::kDefault};

		CDrawContextState () = default;
		CDrawContextState (const CDrawContextState& state);
		CDrawContextState& operator= (const CDrawContextState& state) = default;
		CDrawContextState (CDrawContextState&& state) noexcept;
		CDrawContextState& operator= (CDrawContextState&& state) noexcept;
	};
	/// @endcond

	const CDrawContextState& getCurrentState () const { return currentState; }
	CDrawContextState& getCurrentState () { return currentState; }

private:
	UTF8String* drawStringHelper {nullptr};
	CRect surfaceRect;

	CDrawContextState currentState;

	std::stack<CDrawContextState> globalStatesStack;
	std::stack<CGraphicsTransform> transformStack;
};

//-----------------------------------------------------------------------------
struct ConcatClip
{
	ConcatClip (CDrawContext& context, const CRect& rect)
	: context (context), newClip (rect)
	{
		context.getClipRect (origClip);
		newClip.normalize ();
		newClip.bound (origClip);
		context.setClipRect (newClip);
	}
	~ConcatClip () noexcept
	{
		context.setClipRect (origClip);
	}
	
	bool isEmpty () const { return newClip.isEmpty (); }
	const CRect& get () const { return newClip; }
private:
	CDrawContext& context;
	CRect origClip;
	CRect newClip;
};

//-----------------------------------------------------------------------------
template<typename Proc>
void drawClipped (CDrawContext* context, const CRect& clip, Proc proc)
{
	ConcatClip cc (*context, clip);
	if (!cc.isEmpty ())
		proc ();
}

} // VSTGUI
