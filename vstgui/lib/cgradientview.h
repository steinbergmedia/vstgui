// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cview.h"
#include "ccolor.h"
#include "cgradient.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
///	@brief View which draws a gradient
///	@ingroup new_in_4_2
//-----------------------------------------------------------------------------
class CGradientView : public CView
{
public:
	explicit CGradientView (const CRect& size);
	~CGradientView () noexcept override = default;

	//-----------------------------------------------------------------------------
	/// @name Gradient Style Methods
	//-----------------------------------------------------------------------------
	//@{
	enum GradientStyle {
		kLinearGradient,
		kRadialGradient
	};
	
	void setGradientStyle (GradientStyle style);
	void setGradient (CGradient* gradient);
	void setFrameColor (const CColor& newColor);
	void setGradientAngle (double angle);
	void setRoundRectRadius (CCoord radius);
	void setFrameWidth (CCoord width);
	void setDrawAntialiased (bool state);
	void setRadialCenter (const CPoint& center);
	void setRadialRadius (CCoord radius);

	GradientStyle getGradientStyle () const { return gradientStyle; }
	CGradient* getGradient () const { return gradient; }
	const CColor& getFrameColor () const { return frameColor; }
	double getGradientAngle () const { return gradientAngle; }
	CCoord getRoundRectRadius () const { return roundRectRadius; }
	CCoord getFrameWidth () const { return frameWidth; }
	bool getDrawAntialised () const { return drawAntialiased; }
	const CPoint& getRadialCenter () const { return radialCenter; }
	CCoord getRadialRadius () const { return radialRadius; }
	//@}

	// override
	void setViewSize (const CRect& rect, bool invalid = true) override;
	void draw (CDrawContext* context) override;
protected:
	virtual void attributeChanged ();

	GradientStyle gradientStyle {kLinearGradient};
	CColor frameColor {kBlackCColor};
	double gradientAngle {0.};
	CCoord roundRectRadius {5.};
	CCoord frameWidth {1.};
	CCoord radialRadius {1.};
	CPoint radialCenter {0.5, 0.5};
	bool drawAntialiased {true};

	SharedPointer<CGraphicsPath> path;
	SharedPointer<CGradient> gradient;
};
	
} // VSTGUI
