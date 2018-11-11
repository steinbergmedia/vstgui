// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../cgraphicspath.h"
#include "../../cgradient.h"

#if MAC

#include "macglobals.h"
#if TARGET_OS_IPHONE
	#include <CoreGraphics/CoreGraphics.h>
	#include <ImageIO/ImageIO.h>
#else
	#include <ApplicationServices/ApplicationServices.h>
#endif

namespace VSTGUI {
class CoreTextFont;
class CDrawContext;

//------------------------------------------------------------------------------------
class QuartzGraphicsPath : public CGraphicsPath
{
public:
	QuartzGraphicsPath ();
	QuartzGraphicsPath (const CoreTextFont* font, UTF8StringPtr text);
	~QuartzGraphicsPath () noexcept override;

	void pixelAlign (CDrawContext* context);
	CGPathRef getCGPathRef ();
	void dirty () override;

	bool hitTest (const CPoint& p, bool evenOddFilled = false, CGraphicsTransform* transform = nullptr) override;
	CPoint getCurrentPosition () override;
	CRect getBoundingBox () override;

	CGradient* createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2) override;

	static CGAffineTransform createCGAffineTransform (const CGraphicsTransform& t);

//------------------------------------------------------------------------------------
protected:
	CGMutablePathRef path;
	CGMutablePathRef originalTextPath;
	bool isPixelAlligned;
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
