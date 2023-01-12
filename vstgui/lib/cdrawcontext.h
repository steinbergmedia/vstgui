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
	using LinePair = VSTGUI::LinePair;
	using LineList = VSTGUI::LineList;
	using PointList = VSTGUI::PointList;

	inline void drawLine (const CPoint& start, const CPoint& end)
	{
		drawLine (std::make_pair (start, end));
	}
	/** draw a line */
	void drawLine (const LinePair& line);
	/** draw multiple lines at once */
	void drawLines (const LineList& lines);
	/** draw a polygon */
	void drawPolygon (const PointList& polygonPointList, const CDrawStyle drawStyle = kDrawStroked);
	/** draw a rect */
	void drawRect (const CRect& rect, const CDrawStyle drawStyle = kDrawStroked);
	/** draw an arc, angles are in degree */
	void drawArc (const CRect& rect, const float startAngle1, const float endAngle2,
				  const CDrawStyle drawStyle = kDrawStroked);
	/** draw an ellipse */
	void drawEllipse (const CRect& rect, const CDrawStyle drawStyle = kDrawStroked);
	/** draw a point */
	void drawPoint (const CPoint& point, const CColor& color);
	/** don't call directly, please use CBitmap::draw instead */
	void drawBitmap (CBitmap* bitmap, const CRect& dest, const CPoint& offset = CPoint (0, 0),
					 float alpha = 1.f);
	void drawBitmapNinePartTiled (CBitmap* bitmap, const CRect& dest,
								  const CNinePartTiledDescription& desc, float alpha = 1.f);
	void fillRectWithBitmap (CBitmap* bitmap, const CRect& srcRect, const CRect& dstRect,
							 float alpha);

	/** clears the rect (makes r = 0, g = 0, b = 0, a = 0) */
	void clearRect (const CRect& rect);
	//@}

	//-----------------------------------------------------------------------------
	// @name Bitmap Interpolation Quality
	//-----------------------------------------------------------------------------
	//@{
	/** set the current bitmap interpolation quality */
	void setBitmapInterpolationQuality (BitmapInterpolationQuality quality);
	/** get the current bitmap interpolation quality */
	BitmapInterpolationQuality getBitmapInterpolationQuality () const;
	//@}

	//-----------------------------------------------------------------------------
	/// @name Line Mode
	//-----------------------------------------------------------------------------
	//@{
	/** set the current line style */
	void setLineStyle (const CLineStyle& style);
	/** get the current line style */
	const CLineStyle& getLineStyle () const;

	/** set the current line width */
	void setLineWidth (CCoord width);
	/** get the current line width */
	CCoord getLineWidth () const;
	//@}

	//-----------------------------------------------------------------------------
	/// @name Draw Mode
	//-----------------------------------------------------------------------------
	//@{
	/** set the current draw mode, see CDrawMode */
	void setDrawMode (CDrawMode mode);
	/** get the current draw mode, see CDrawMode */
	CDrawMode getDrawMode () const;
	//@}

	//-----------------------------------------------------------------------------
	/// @name Clipping
	//-----------------------------------------------------------------------------
	//@{
	/** set the current clip */
	void setClipRect (const CRect& clip);
	/** get the current clip */
	CRect& getClipRect (CRect &clip) const;
	/** reset the clip to the default state */
	void resetClipRect ();
	//@}

	//-----------------------------------------------------------------------------
	/// @name Color
	//-----------------------------------------------------------------------------
	//@{
	/** set current fill color */
	void setFillColor (const CColor& color);
	/** get current fill color */
	CColor getFillColor () const;
	/** set current stroke color */
	void setFrameColor (const CColor& color);
	/** get current stroke color */
	CColor getFrameColor () const;
	//@}

	//-----------------------------------------------------------------------------
	/// @name Font
	//-----------------------------------------------------------------------------
	//@{
	/** set current font color */
	void setFontColor (const CColor& color);
	/** get current font color */
	CColor getFontColor () const;
	/** set current font */
	void setFont (const CFontRef font, const CCoord& size = 0, const int32_t& style = -1);
	/** get current font */
	const CFontRef getFont () const;
	//@}
	
	//-----------------------------------------------------------------------------
	/// @name Text
	//-----------------------------------------------------------------------------
	//@{
	/** get the width of an UTF-8 encoded string */
	CCoord getStringWidth (UTF8StringPtr pStr);
	/** draw an UTF-8 encoded string */
	void drawString (UTF8StringPtr string, const CRect& _rect,
					 const CHoriTxtAlign hAlign = kCenterText, bool antialias = true);
	/** draw an UTF-8 encoded string */
	void drawString (UTF8StringPtr string, const CPoint& _point, bool antialias = true);

	/** get the width of a platform string */
	CCoord getStringWidth (IPlatformString* pStr);
	/** draw a platform string */
	void drawString (IPlatformString* string, const CRect& _rect,
					 const CHoriTxtAlign hAlign = kCenterText, bool antialias = true);
	/** draw a platform string */
	void drawString (IPlatformString* string, const CPoint& _point, bool antialias = true);
	//@}
	
	//-----------------------------------------------------------------------------
	/// @name Global Alpha State
	//-----------------------------------------------------------------------------
	//@{
	/** sets the global alpha value[0..1] */
	void setGlobalAlpha (float newAlpha);
	/** get current global alpha value */
	float getGlobalAlpha () const;
	//@}
	
	//-----------------------------------------------------------------------------
	/// @name Global State Stack
	//-----------------------------------------------------------------------------
	//@{
	void saveGlobalState ();
	void restoreGlobalState ();
	//@}

	//-----------------------------------------------------------------------------
	/// @name Transformation
	//-----------------------------------------------------------------------------
	//@{
	const CGraphicsTransform& getCurrentTransform () const;
	const CRect& getAbsoluteClipRect () const;

	/** returns the backend scale factor. */
	double getScaleFactor () const;

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
	CGraphicsPath* createGraphicsPath ();
	/** create a graphics path from a text */
	CGraphicsPath* createTextPath (const CFontRef font, UTF8StringPtr text);

	/** create a rect with round corners as graphics path, you need to forget it after usage */
	CGraphicsPath* createRoundRectGraphicsPath (const CRect& size, CCoord radius);

	enum PathDrawMode
	{
		kPathFilled,
		kPathFilledEvenOdd,
		kPathStroked
	};

	void drawGraphicsPath (CGraphicsPath* path, PathDrawMode mode = kPathFilled,
						   CGraphicsTransform* transformation = nullptr);
	void fillLinearGradient (CGraphicsPath* path, const CGradient& gradient,
							 const CPoint& startPoint, const CPoint& endPoint, bool evenOdd = false,
							 CGraphicsTransform* transformation = nullptr);
	void fillRadialGradient (CGraphicsPath* path, const CGradient& gradient, const CPoint& center,
							 CCoord radius, const CPoint& originOffset = CPoint (0, 0),
							 bool evenOdd = false, CGraphicsTransform* transformation = nullptr);
	//@}

	void beginDraw ();
	void endDraw ();

	const CRect& getSurfaceRect () const;

	CDrawContext (const PlatformGraphicsDeviceContextPtr device, const CRect& surfaceRect,
				  double scaleFactor);
	~CDrawContext () noexcept override;

	const PlatformGraphicsDeviceContextPtr& getPlatformDeviceContext () const;

protected:
	CDrawContext () = delete;
	explicit CDrawContext (const CRect& surfaceRect);

	void init ();

	void pushTransform (const CGraphicsTransform& transformation);
	void popTransform ();

	const UTF8String& getDrawString (UTF8StringPtr string);
	void clearDrawString ();

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
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
