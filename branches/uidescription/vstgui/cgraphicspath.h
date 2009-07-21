/*
 *  cgraphicspath.h
 *
 *  Created by Arne Scheffler on 6/27/09.
 *  Copyright 2009 Arne Scheffler. All rights reserved.
 *
 */

#ifndef __cgraphicspath__
#define __cgraphicspath__

#include "vstgui.h"

#define VSTGUI_CGRAPHICSPATH_AVAILABLE	((defined (VSTGUI_FLOAT_COORDINATES) && (GDIPLUS || (VSTGUI_USES_COREGRAPHICS && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5))) || VSTGUI_BUILD_DOXYGEN)

#if VSTGUI_CGRAPHICSPATH_AVAILABLE

BEGIN_NAMESPACE_VSTGUI
class PlatformGraphicsPath;
class CGradient;

//-----------------------------------------------------------------------------
/// @brief Graphics Path Transformation
//-----------------------------------------------------------------------------
struct CGraphicsTransformation
{
	CPoint offset;
	double scaleX;
	double scaleY;
	double rotation;
	
	CGraphicsTransformation ()
	{
		offset (0, 0);
		scaleX = scaleY = 1.;
		rotation = 0;
	}
};

//-----------------------------------------------------------------------------
/*! @class CGraphicsPath
	@brief Graphics Path Object

	Only available when VSTGUI_FLOAT_COORDINATES is defined.
	On Windows GDIPLUS must be defined.
	On Mac OS X only available when building for Mac OS X 10.5 or newer.
*/
//-----------------------------------------------------------------------------
class CGraphicsPath : public CBaseObject
{
public:
	CGraphicsPath ();
	~CGraphicsPath ();

	enum PathDrawMode
	{
		kFilled,
		kFilledEvenOdd,
		kStroked,
	};

	void addArc (const CRect& rect, double startAngle, double endAngle);
	void addCurve (const CPoint& start, const CPoint& control1, const CPoint& control2, const CPoint& end);
	void addEllipse (const CRect& rect);
	void addLine (const CPoint& start, const CPoint& end);
	void addRect (const CRect& rect);
	void addPath (const CGraphicsPath& path, CGraphicsTransformation* transformation = 0);
	void addString (const char* utf8String, CFontRef font, const CPoint& position);
	void closeSubpath ();

	void draw (CDrawContext* context, PathDrawMode mode = kFilled, CGraphicsTransformation* transformation = 0);
	void fillLinearGradient (CDrawContext* context, const CGradient& gradient, const CPoint& color1StartPoint, const CPoint& color2StartPoint, bool evenOdd = false, CGraphicsTransformation* transformation = 0);
	void fillRadialGradient (CDrawContext* context, const CGradient& gradient, const CPoint& startCenter, double startRadius, const CPoint& endCenter, double endRadius, bool evenOdd = false, CGraphicsTransformation* transformation = 0);
	
	CPoint getCurrentPosition () const;
	CRect getBoundingBox () const;
protected:
	PlatformGraphicsPath* platformPath;
};

//-----------------------------------------------------------------------------
class CGradient : public CBaseObject
{
public:
	/**
	 * @brief creates a new gradient object, you must release it with forget() when you're done with it
	 * @param color1Start value between zero and one which defines the normalized start offset for color1
	 * @param color2Start value between zero and one which defines the normalized start offset for color2
	 * @param color1 the first color of the gradient
	 * @param color2 the second color of the gradient
	 * @return a new gradient object
	*/
	static CGradient* create (double color1Start, double color2Start, const CColor& color1, const CColor& color2);

	double getColor1Start () const { return color1Start; }
	double getColor2Start () const { return color2Start; }
	const CColor& getColor1 () const { return color1; }
	const CColor& getColor2 () const { return color2; }
protected:
	CGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
	: color1Start (color1Start), color2Start (color2Start), color1 (color1), color2 (color2) {}
	
	double color1Start;
	double color2Start;
	CColor color1;
	CColor color2;
};

#if DEBUG
extern CView* createCGraphicsPathTestView ();
#endif

END_NAMESPACE_VSTGUI

#endif // (VSTGUI_USES_COREGRAPHICS || GDIPLUS) && defined (VSTGUI_FLOAT_COORDINATES)

#endif
