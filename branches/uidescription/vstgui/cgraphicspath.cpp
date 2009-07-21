/*
 *  cgraphicspath.cpp
 *
 *  Created by Arne Scheffler on 6/27/09.
 *  Copyright 2009 Arne Scheffler. All rights reserved.
 *
 */

#include "cgraphicspath.h"
#include <cmath>

#if VSTGUI_CGRAPHICSPATH_AVAILABLE

BEGIN_NAMESPACE_VSTGUI

#if VSTGUI_USES_COREGRAPHICS
//-----------------------------------------------------------------------------
static inline double radians (double degrees) { return degrees * M_PI / 180.; }

//-----------------------------------------------------------------------------
class CGGradient : public CGradient
{
public:
	CGGradient (double _color1Start, double _color2Start, const CColor& _color1, const CColor& _color2)
	: CGradient (_color1Start, _color2Start, _color1, _color2)
	, gradient (0)
	{
		CGColorRef cgColor1 = CGColorCreateGenericRGB (color1.red/255.f, color1.green/255.f, color1.blue/255.f, color1.alpha/255.f);
		CGColorRef cgColor2 = CGColorCreateGenericRGB (color2.red/255.f, color2.green/255.f, color2.blue/255.f, color2.alpha/255.f);
		const void* colors[] = { cgColor1, cgColor2 };
		CFArrayRef colorArray = CFArrayCreate (0, colors, 2, &kCFTypeArrayCallBacks);

		if (color1Start < 0) color1Start = 0;
		else if (color1Start > 1) color1Start = 1;
		if (color2Start < 0) color2Start = 0;
		else if (color2Start > 1) color2Start = 1;
		CGFloat locations[] = { color1Start, color2Start };
		
		gradient = CGGradientCreateWithColors (0, colorArray, locations);

		CFRelease (cgColor1);
		CFRelease (cgColor2);
		CFRelease (colorArray);
	}
	
	~CGGradient ()
	{
		if (gradient)
			CFRelease (gradient);
	}

	operator CGGradientRef () const { return gradient; }

protected:
	CGGradientRef gradient;
};

//-----------------------------------------------------------------------------
static CGAffineTransform createCGAfflineTransform (const CGraphicsTransformation& transformation)
{
	CGAffineTransform transform = CGAffineTransformMakeTranslation (transformation.offset.x, transformation.offset.y);
	transform = CGAffineTransformScale (transform, transformation.scaleX, transformation.scaleY);
	transform = CGAffineTransformRotate (transform, radians (transformation.rotation));
	return transform;
}


#elif GDIPLUS
static Gdiplus::Color createGdiPlusColor (const CColor& color)
{
	return Gdiplus::Color (color.alpha, color.red, color.green, color.blue);
}
#endif

//-----------------------------------------------------------------------------
class PlatformGraphicsPath
{
public:
	PlatformGraphicsPath ()
	: path (0)
	{
		create ();
	}
	~PlatformGraphicsPath ()
	{
		release ();
	}
	
	#if VSTGUI_USES_COREGRAPHICS
	void create ()
	{
		path = CGPathCreateMutable ();
	}
	void release ()
	{
		if (path)
			CFRelease (path);
	}
	
	operator CGMutablePathRef () const { return path; }

	CGMutablePathRef path;
	#endif
	
	#if GDIPLUS
	void create ()
	{
		GDIPlusGlobals::enter ();
		path = new Gdiplus::GraphicsPath ();
	}
	void release ()
	{
		if (path)
			delete path;
		GDIPlusGlobals::exit ();
	}

	operator Gdiplus::GraphicsPath* () const { return path; }
	Gdiplus::GraphicsPath* operator ->() { return path; }

	Gdiplus::GraphicsPath* path;
	#endif
};

//-----------------------------------------------------------------------------
CGraphicsPath::CGraphicsPath ()
: platformPath (new PlatformGraphicsPath ())
{
}

//-----------------------------------------------------------------------------
CGraphicsPath::~CGraphicsPath ()
{
	delete platformPath;
}

//-----------------------------------------------------------------------------
void CGraphicsPath::draw (CDrawContext* context, PathDrawMode mode, CGraphicsTransformation* transformation)
{
	#if VSTGUI_USES_COREGRAPHICS
	CGContextRef cgContext = context->beginCGContext (true);
	CGContextTranslateCTM (cgContext, context->offset.x, context->offset.y);

	if (transformation)
	{
		CGAffineTransform transform = createCGAfflineTransform (*transformation);
		CGMutablePathRef path = CGPathCreateMutable ();
		CGPathAddPath (path, &transform, *platformPath);
		CGContextAddPath (cgContext, path);
		CFRelease (path);
	}
	else
		CGContextAddPath (cgContext, *platformPath);

	CGPathDrawingMode cgMode = kCGPathFill;
	switch (mode)
	{
		case kFilledEvenOdd: cgMode = kCGPathEOFill; break;
		case kStroked: cgMode = kCGPathStroke; break;
	}

	CGContextDrawPath (cgContext, cgMode);
	
	context->releaseCGContext (cgContext);

	#elif GDIPLUS
	Gdiplus::Graphics* graphics = context->getGraphics ();
	Gdiplus::GraphicsState state = graphics->Save ();
	graphics->TranslateTransform ((Gdiplus::REAL)context->offset.x, (Gdiplus::REAL)context->offset.y);

	Gdiplus::GraphicsPath* path = *platformPath;

	if (transformation)
	{
		Gdiplus::Matrix matrix;
		matrix.Translate ((Gdiplus::REAL)transformation->offset.x, (Gdiplus::REAL)transformation->offset.y);
		matrix.Scale ((Gdiplus::REAL)transformation->scaleX, (Gdiplus::REAL)transformation->scaleY);
		matrix.Rotate ((Gdiplus::REAL)transformation->rotation);
		path = (*platformPath)->Clone ();
		path->Transform (&matrix);
	}

	if (mode == kStroked)
	{
		graphics->DrawPath (context->getPen (), path);
	}
	else
	{
		path->SetFillMode (mode == kFilledEvenOdd ? Gdiplus::FillModeAlternate : Gdiplus::FillModeWinding);
		graphics->FillPath (context->getBrush (), path);
	}
	graphics->Restore (state);
	if (path != *platformPath)
		delete path;
	#endif
}

//-----------------------------------------------------------------------------
void CGraphicsPath::fillLinearGradient (CDrawContext* context, const CGradient& gradient, const CPoint& color1StartPoint, const CPoint& color2StartPoint, bool evenOdd, CGraphicsTransformation* transformation)
{
	#if VSTGUI_USES_COREGRAPHICS
	const CGGradient* cgGradient = dynamic_cast<const CGGradient*> (&gradient);
	if (cgGradient == 0)
		return;

	CGContextRef cgContext = context->beginCGContext (true);
	CGContextTranslateCTM (cgContext, context->offset.x, context->offset.y);

	if (transformation)
	{
		CGAffineTransform transform = createCGAfflineTransform (*transformation);
		CGMutablePathRef path = CGPathCreateMutable ();
		CGPathAddPath (path, &transform, *platformPath);
		CGContextAddPath (cgContext, path);
		CFRelease (path);
	}
	else
		CGContextAddPath (cgContext, *platformPath);

	if (evenOdd)
		CGContextEOClip (cgContext);
	else
		CGContextClip (cgContext);

	CGContextDrawLinearGradient (cgContext, *cgGradient, CGPointMake (color1StartPoint.x, color1StartPoint.y), CGPointMake (color2StartPoint.x, color2StartPoint.y), 0);
	
	context->releaseCGContext (cgContext);

	#elif GDIPLUS
	Gdiplus::Graphics* graphics = context->getGraphics ();
	Gdiplus::GraphicsState state = graphics->Save ();
	graphics->TranslateTransform ((Gdiplus::REAL)context->offset.x, (Gdiplus::REAL)context->offset.y);

	Gdiplus::GraphicsPath* path = *platformPath;

	if (transformation)
	{
		Gdiplus::Matrix matrix;
		matrix.Translate ((Gdiplus::REAL)transformation->offset.x, (Gdiplus::REAL)transformation->offset.y);
		matrix.Scale ((Gdiplus::REAL)transformation->scaleX, (Gdiplus::REAL)transformation->scaleY);
		matrix.Rotate ((Gdiplus::REAL)transformation->rotation);
		path = (*platformPath)->Clone ();
		path->Transform (&matrix);
	}

	Gdiplus::PointF c1p ((Gdiplus::REAL)(color1StartPoint.x-context->offset.x), (Gdiplus::REAL)(color1StartPoint.y-context->offset.y));
	Gdiplus::PointF c2p ((Gdiplus::REAL)(color2StartPoint.x-context->offset.x), (Gdiplus::REAL)(color2StartPoint.y-context->offset.y));
	Gdiplus::LinearGradientBrush brush (c1p, c2p, createGdiPlusColor (gradient.getColor1 ()), createGdiPlusColor (gradient.getColor2 ()));
	Gdiplus::REAL blendFactors[] = { 0.f, 0.f, 1.f, 1.f };
	Gdiplus::REAL blendPositions [] = { 0.f, (Gdiplus::REAL)gradient.getColor1Start (), (Gdiplus::REAL)gradient.getColor2Start (), 1.f };
	brush.SetBlend (blendFactors, blendPositions, 4);
	path->SetFillMode (evenOdd ? Gdiplus::FillModeAlternate : Gdiplus::FillModeWinding);

	graphics->FillPath (&brush, path);
	graphics->Restore (state);
	if (path != *platformPath)
		delete path;
	#endif
}

//-----------------------------------------------------------------------------
void CGraphicsPath::fillRadialGradient (CDrawContext* context, const CGradient& gradient, const CPoint& startCenter, double startRadius, const CPoint& endCenter, double endRadius, bool evenOdd, CGraphicsTransformation* transformation)
{
	#if VSTGUI_USES_COREGRAPHICS
	const CGGradient* cgGradient = dynamic_cast<const CGGradient*> (&gradient);
	if (cgGradient == 0)
		return;

	CGContextRef cgContext = context->beginCGContext (true);
	CGContextTranslateCTM (cgContext, context->offset.x, context->offset.y);

	if (transformation)
	{
		CGAffineTransform transform = createCGAfflineTransform (*transformation);
		CGMutablePathRef path = CGPathCreateMutable ();
		CGPathAddPath (path, &transform, *platformPath);
		CGContextAddPath (cgContext, path);
		CFRelease (path);
	}
	else
		CGContextAddPath (cgContext, *platformPath);

	if (evenOdd)
		CGContextEOClip (cgContext);
	else
		CGContextClip (cgContext);

	CGContextDrawRadialGradient (cgContext, *cgGradient, CGPointMake (startCenter.x, startCenter.y), startRadius, CGPointMake (endCenter.x, endCenter.y), endRadius, 0);
	
	context->releaseCGContext (cgContext);

	#elif GDIPLUS
	// TODO: implementation

	#endif
}

//-----------------------------------------------------------------------------
void CGraphicsPath::closeSubpath ()
{
	#if VSTGUI_USES_COREGRAPHICS
	CGPathCloseSubpath (*platformPath);

	#elif GDIPLUS
	(*platformPath)->CloseFigure ();
	#endif
}

//-----------------------------------------------------------------------------
void CGraphicsPath::addArc (const CRect& rect, double startAngle, double endAngle)
{
	#if VSTGUI_USES_COREGRAPHICS
	CGFloat centerX = rect.left + rect.getWidth () / 2.;
	CGFloat centerY = rect.top + rect.getHeight () / 2.;

	CGAffineTransform transform = CGAffineTransformMakeTranslation (centerX, centerY);
	transform = CGAffineTransformScale (transform, rect.getWidth () / 2, rect.getHeight () / 2);
//	transform = CGAffineTransformRotate (transform, radians (-90));
	
	CGPathMoveToPoint (*platformPath, &transform, cos (radians (startAngle)), sin (radians (startAngle)));

	CGPathAddArc (*platformPath, &transform, 0, 0, 1, radians (startAngle), radians (endAngle), false);

	#elif GDIPLUS
	if (endAngle < startAngle)
		endAngle += 360.f;
	endAngle = fabs (endAngle - (Gdiplus::REAL)startAngle);
	(*platformPath)->AddArc ((Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.getWidth (), (Gdiplus::REAL)rect.getHeight (), (Gdiplus::REAL)startAngle, (Gdiplus::REAL)endAngle);

	#endif
}

//-----------------------------------------------------------------------------
void CGraphicsPath::addCurve (const CPoint& start, const CPoint& control1, const CPoint& control2, const CPoint& end)
{
	#if VSTGUI_USES_COREGRAPHICS
	CGPathMoveToPoint (*platformPath, 0, start.x, start.y);
	CGPathAddCurveToPoint (*platformPath, 0, control1.x, control1.y, control2.x, control2.y, end.x, end.y);
	
	#elif GDIPLUS
	(*platformPath)->AddBezier ((Gdiplus::REAL)start.x, (Gdiplus::REAL)start.y, (Gdiplus::REAL)control1.x, (Gdiplus::REAL)control1.y, (Gdiplus::REAL)control2.x, (Gdiplus::REAL)control2.y, (Gdiplus::REAL)end.x, (Gdiplus::REAL)end.y);

	#endif
}

//-----------------------------------------------------------------------------
void CGraphicsPath::addEllipse (const CRect& rect)
{
	#if VSTGUI_USES_COREGRAPHICS
	CGPathAddEllipseInRect (*platformPath, 0, CGRectMake (rect.left, rect.top, rect.getWidth (), rect.getHeight ()));
	
	#elif GDIPLUS
	(*platformPath)->AddEllipse ((Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.getWidth (), (Gdiplus::REAL)rect.getHeight ());

#endif
}

//-----------------------------------------------------------------------------
void CGraphicsPath::addLine (const CPoint& start, const CPoint& end)
{
	#if VSTGUI_USES_COREGRAPHICS
	CGPathMoveToPoint (*platformPath, 0, start.x, start.y);
	CGPathAddLineToPoint (*platformPath, 0, end.x, end.y);
	
	#elif GDIPLUS
	if (start != getCurrentPosition ())
		(*platformPath)->StartFigure ();
	(*platformPath)->AddLine ((Gdiplus::REAL)start.x, (Gdiplus::REAL)start.y, (Gdiplus::REAL)end.x, (Gdiplus::REAL)end.y);

	#endif
}

//-----------------------------------------------------------------------------
void CGraphicsPath::addRect (const CRect& rect)
{
	#if VSTGUI_USES_COREGRAPHICS
	CGRect r = CGRectMake (rect.left, rect.top, rect.getWidth (), rect.getHeight ());
	CGPathAddRect (*platformPath, 0, r);
	
	#elif GDIPLUS
	(*platformPath)->AddRectangle (Gdiplus::RectF ((Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.getWidth (), (Gdiplus::REAL)rect.getHeight ()));

	#endif
}

//-----------------------------------------------------------------------------
void CGraphicsPath::addPath (const CGraphicsPath& path, CGraphicsTransformation* transformation)
{
	#if VSTGUI_USES_COREGRAPHICS
	if (transformation)
	{
		CGAffineTransform transform = createCGAfflineTransform (*transformation);
		if (&path == this)
		{
			CGPathRef pathCopy = CGPathCreateCopy (*platformPath);
			CGPathAddPath (*platformPath, &transform, pathCopy);
			CFRelease (pathCopy);
		}
		else
			CGPathAddPath (*platformPath, &transform, *path.platformPath);
	}
	else
	{
		if (&path == this)
		{
			CGPathRef pathCopy = CGPathCreateCopy (*platformPath);
			CGPathAddPath (*platformPath, 0, pathCopy);
			CFRelease (pathCopy);
		}
		else
			CGPathAddPath (*platformPath, 0, *path.platformPath);
	}
	
	#elif GDIPLUS
	Gdiplus::GraphicsPath* gdiPath = (*path.platformPath)->Clone ();
	if (transformation)
	{
		Gdiplus::Matrix matrix;
		matrix.Translate ((Gdiplus::REAL)transformation->offset.x, (Gdiplus::REAL)transformation->offset.y);
		matrix.Scale ((Gdiplus::REAL)transformation->scaleX, (Gdiplus::REAL)transformation->scaleY);
		matrix.Rotate ((Gdiplus::REAL)transformation->rotation);
		gdiPath->Transform (&matrix);
	}
	(*platformPath)->AddPath (gdiPath, true); // TODO: maybe the second parameter must be false
	delete gdiPath;

	#endif
}

//-----------------------------------------------------------------------------
void CGraphicsPath::addString (const char* utf8String, CFontRef font, const CPoint& position)
{
	#if VSTGUI_USES_COREGRAPHICS && VSTGUI_USES_CORE_TEXT
	CTFontRef fontRef = (CTFontRef)font->getPlatformFont ();
	if (fontRef == 0)
		return;
	CFStringRef utf8Str = CFStringCreateWithCString (NULL, utf8String, kCFStringEncodingUTF8);
	if (utf8Str)
	{
		CFStringRef keys[] = { kCTFontAttributeName };
		CFTypeRef values[] = { fontRef };
		CFDictionaryRef attributes = CFDictionaryCreate (kCFAllocatorDefault, (const void**)&keys,(const void**)&values, sizeof(keys) / sizeof(keys[0]), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFAttributedStringRef attrStr = CFAttributedStringCreate (0, utf8Str, attributes);
		CFRelease (attributes);
		if (attrStr)
		{
			CTLineRef lineRef = CTLineCreateWithAttributedString (attrStr);
			if (lineRef)
			{
				CFArrayRef array = CTLineGetGlyphRuns (lineRef);
				if (array)
				{
					CTRunRef runRef = (CTRunRef)CFArrayGetValueAtIndex (array, 0);
					if (runRef)
					{
						const CGGlyph* glyphs = CTRunGetGlyphsPtr (runRef);
						const CGPoint* points = CTRunGetPositionsPtr (runRef);
						if (glyphs && points)
						{
							CGAffineTransform textTransform = CGAffineTransformMake (1.0, 0.0, 0.0, -1.0, 0.0, 0.0);
							textTransform = CGAffineTransformTranslate (textTransform, position.x, -position.y);
							CFIndex numGlyphs = CTRunGetGlyphCount (runRef);
							for (CFIndex i = 0; i < numGlyphs; i++)
							{
								CGPathRef glyphPath = CTFontCreatePathForGlyph (fontRef, glyphs[i], 0);
								if (glyphPath)
								{
									CGAffineTransform transform = CGAffineTransformTranslate (textTransform, points[i].x, points[i].y);
									CGPathAddPath (*platformPath, &transform, glyphPath);
									CFRelease (glyphPath);
								}
							}
						}
					}
				}
				CFRelease (lineRef);
			}
			CFRelease (attrStr);
		}
		CFRelease (utf8Str);
	}
	#elif GDIPLUS
	// TODO: implementation

	#endif
}

//-----------------------------------------------------------------------------
CPoint CGraphicsPath::getCurrentPosition () const
{
	CPoint p;

	#if VSTGUI_USES_COREGRAPHICS
	CGPoint cgPoint = CGPathGetCurrentPoint (*platformPath);
	p.x = cgPoint.x;
	p.y = cgPoint.y;

	#elif GDIPLUS
	Gdiplus::PointF gdiPoint;
	(*platformPath)->GetLastPoint (&gdiPoint);
	p.x = gdiPoint.X;
	p.y = gdiPoint.Y;

	#endif

	return p;
}

//-----------------------------------------------------------------------------
CRect CGraphicsPath::getBoundingBox () const
{
	CRect r;

	#if VSTGUI_USES_COREGRAPHICS
	CGRect cgRect = CGPathGetBoundingBox (*platformPath);
	r.left = cgRect.origin.x;
	r.top = cgRect.origin.y;
	r.setWidth (cgRect.size.width);
	r.setHeight (cgRect.size.height);
	
	#elif GDIPLUS
	Gdiplus::RectF gdiRect;
	(*platformPath)->GetBounds (&gdiRect);
	r.left = gdiRect.X;
	r.top = gdiRect.Y;
	r.setWidth (gdiRect.Width);
	r.setHeight (gdiRect.Height);

	#endif
	
	return r;
}

//-----------------------------------------------------------------------------
CGradient* CGradient::create (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
{
	#if VSTGUI_USES_COREGRAPHICS
	return new CGGradient (color1Start, color2Start, color1, color2);
	#elif GDIPLUS
	return new CGradient (color1Start, color2Start, color1, color2);
	#endif
}

#if DEBUG
//-----------------------------------------------------------------------------
class CGraphicsPathTestView : public CView
{
public:
	CGraphicsPathTestView (const CRect& size)
	: CView (size)
	, gradient (0)
	{
		gradient = CGradient::create (0.2, 0.8, kBlueCColor, kYellowCColor);
	}
	
	~CGraphicsPathTestView ()
	{
		if (gradient)
			gradient->forget ();
	}

	void testRectGradient (CDrawContext* context, const CRect& r)
	{
		CGraphicsPath path;
		path.addRect (r);
		path.fillLinearGradient (context, *gradient, r.getTopLeft (), r.getBottomRight ());
	}

	void testArc (CDrawContext* context, const CRect& r)
	{
		context->setLineWidth (2);
		CGraphicsPath path;
		path.addArc (r, 0, 270);
		path.draw (context, CGraphicsPath::kFilled);
		path.draw (context, CGraphicsPath::kStroked);
	}
	void testCurve (CDrawContext* context, const CRect& r)
	{
		CGraphicsTransformation transform;
		transform.offset.x = r.left;
		transform.offset.y = r.top;
		transform.scaleX = r.getWidth ();
		transform.scaleY = r.getHeight ();

		CGraphicsPath path;
		path.addCurve (CPoint (0, 0), CPoint (0.5, 0), CPoint (1, 1), CPoint (1, 1));
		path.draw (context, CGraphicsPath::kStroked, &transform);
		// now with a rotation around the center point
		transform.offset.x += r.getWidth ();
		transform.rotation = 90;
		path.draw (context, CGraphicsPath::kStroked, &transform);
		
		CGraphicsPath path2;
		path2.addCurve (CPoint (0, 0), CPoint (0.5, 0), CPoint (1, 1), CPoint (1, 1));
		CGraphicsTransformation transform2;
		transform2.offset.x = 1;
		transform2.rotation = 90;
		path2.addPath (path2, &transform2);
		
		transform.offset.x -= r.getWidth ();
		transform.rotation = 0;
		path2.fillLinearGradient (context, *gradient, r.getTopRight (), r.getBottomLeft (), false, &transform);
	}

	void testString (CDrawContext* context, const CRect& bounds)
	{
		context->setLineWidth (1);
		CFontDesc* font = new CFontDesc (*kSystemFont);
		font->setSize (40);
		CGraphicsPath path;
		path.addString ("This is my test String", font, bounds.getBottomLeft ());
		CRect pathBounds = path.getBoundingBox ();
		path.fillLinearGradient (context, *gradient, pathBounds.getTopRight (), pathBounds.getBottomLeft ());
		path.draw (context, CGraphicsPath::kStroked);
		font->forget ();
	}
	
	void draw (CDrawContext *context)
	{
		const CCoord cellSize = size.getWidth () / 10;
		context->setDrawMode (kAntialias);
		context->setFillColor (kRedCColor);
		context->setFrameColor (kGreenCColor);
		CRect r (size);
		r.setWidth (cellSize);
		r.setHeight (cellSize);
		testRectGradient (context, r);
		r.offset (cellSize, 0);
		testArc (context, r);
		r.offset (cellSize, 0);
		testCurve (context, r);
		r.offset (cellSize, 0);
		testString (context, r);
		setDirty (false);
	}
	
protected:
	CGradient* gradient;
};

//-----------------------------------------------------------------------------
CView* createCGraphicsPathTestView ()
{
	return new CGraphicsPathTestView (CRect (0, 0, 500, 500));
}

#endif

END_NAMESPACE_VSTGUI

#endif // (VSTGUI_USES_COREGRAPHICS || GDIPLUS) && defined (VSTGUI_FLOAT_COORDINATES)
