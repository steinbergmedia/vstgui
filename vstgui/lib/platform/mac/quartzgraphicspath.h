// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../cgraphicspath.h"
#include "../../cgradient.h"
#include "../iplatformgraphicspath.h"

#if MAC

#include "macglobals.h"
#if TARGET_OS_IPHONE
	#include <CoreGraphics/CoreGraphics.h>
	#include <ImageIO/ImageIO.h>
#else
	#include <ApplicationServices/ApplicationServices.h>
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
CGAffineTransform createCGAffineTransform (const CGraphicsTransform& t);

//-----------------------------------------------------------------------------
class CGGraphicsPathFactory : public IPlatformGraphicsPathFactory
{
public:
	static IPlatformGraphicsPathFactoryPtr instance ();

	IPlatformGraphicsPathPtr createPath () override;
	IPlatformGraphicsPathPtr createTextPath (const PlatformFontPtr& font, UTF8StringPtr text) override;
};

//-----------------------------------------------------------------------------
class CGGraphicsPath : public IPlatformGraphicsPath
{
public:
	CGGraphicsPath (CGMutablePathRef path = nullptr); // take ownership of path
	~CGGraphicsPath () noexcept;

	using PixelAlignPointFunc = CGPoint (*) (const CGPoint&, void* context);
	void pixelAlign (const PixelAlignPointFunc& func, void* context);

	CGPathRef getCGPathRef () const { return path; }

	// IPlatformGraphicsPath
	void addArc (const CRect& rect, double startAngle, double endAngle, bool clockwise) override;
	void addEllipse (const CRect& rect) override;
	void addRect (const CRect& rect) override;
	void addLine (const CPoint& to) override;
	void addBezierCurve (const CPoint& control1, const CPoint& control2,
	                     const CPoint& end) override;
	void beginSubpath (const CPoint& start) override;
	void closeSubpath () override;
	void finishBuilding () override;
	bool hitTest (const CPoint& p, bool evenOddFilled = false,
	              CGraphicsTransform* transform = nullptr) const override;
	CRect getBoundingBox () const override;

private:
	CGMutablePathRef path {nullptr};
};



//------------------------------------------------------------------------------------
class QuartzGraphicsPath : public CGraphicsPath
{
public:
	QuartzGraphicsPath (const IPlatformGraphicsPathFactoryPtr& factory,
	                    const IPlatformGraphicsPathPtr& path = nullptr);
	~QuartzGraphicsPath () noexcept override;

	const IPlatformGraphicsPathPtr& getPlatformPath ();
	
	void dirty () override;

	bool hitTest (const CPoint& p, bool evenOddFilled = false, CGraphicsTransform* transform = nullptr) override;
	CPoint getCurrentPosition () override;
	CRect getBoundingBox () override;

	CGradient* createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2) override;

//------------------------------------------------------------------------------------
protected:
	void makeCGGraphicsPath ();
	bool ensurePathValid ();

	IPlatformGraphicsPathFactoryPtr factory;
	IPlatformGraphicsPathPtr path;
};

//-----------------------------------------------------------------------------
class QuartzGradient : public CGradient
{
public:
	explicit QuartzGradient (const ColorStopMap& map);
	QuartzGradient (double _color1Start, double _color2Start, const CColor& _color1, const CColor& _color2);
	~QuartzGradient () noexcept override;

	operator CGGradientRef () const;

	void addColorStop (const std::pair<double, CColor>& colorStop) override;
	void addColorStop (std::pair<double, CColor>&& colorStop) override;

protected:
	void createCGGradient () const;
	void releaseCGGradient ();

	mutable CGGradientRef gradient;
};

} // VSTGUI

#endif
