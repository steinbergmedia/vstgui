//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __cgraphicspath__
#define __cgraphicspath__

#include "cdrawcontext.h"

namespace VSTGUI {
class CGradient;
class CFrame;

//-----------------------------------------------------------------------------
/// @brief Graphics Path Transformation [new in 4.0]
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
	@brief Graphics Path Object [new in 4.0]
*/
//-----------------------------------------------------------------------------
class CGraphicsPath : public CBaseObject
{
public:
	static CGraphicsPath* create (CFrame* frame);

	//-----------------------------------------------------------------------------
	/// @name Creating gradients
	//-----------------------------------------------------------------------------
	//@{
	/**
	 * @brief creates a new gradient object, you must release it with forget() when you're done with it
	 * @param color1Start value between zero and one which defines the normalized start offset for color1
	 * @param color2Start value between zero and one which defines the normalized start offset for color2
	 * @param color1 the first color of the gradient
	 * @param color2 the second color of the gradient
	 * @return a new gradient object
	*/
	virtual CGradient* createGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2) = 0;
	//@}

	//-----------------------------------------------------------------------------
	/// @name Adding Elements
	//-----------------------------------------------------------------------------
	//@{
	virtual void addArc (const CRect& rect, double startAngle, double endAngle) = 0;
	virtual void addCurve (const CPoint& start, const CPoint& control1, const CPoint& control2, const CPoint& end) = 0;
	virtual void addEllipse (const CRect& rect) = 0;
	virtual void addLine (const CPoint& start, const CPoint& end) = 0;
	virtual void addRect (const CRect& rect) = 0;
	virtual void addPath (const CGraphicsPath& path, CGraphicsTransformation* transformation = 0) = 0;
	virtual void addString (const char* utf8String, CFontRef font, const CPoint& position) = 0;
	virtual void closeSubpath () = 0;
	//@}

	//-----------------------------------------------------------------------------
	/// @name Drawing
	//-----------------------------------------------------------------------------
	//@{
	enum PathDrawMode
	{
		kFilled,
		kFilledEvenOdd,
		kStroked,
	};

	virtual void draw (CDrawContext* context, PathDrawMode mode = kFilled, CGraphicsTransformation* transformation = 0) = 0;
	virtual void fillLinearGradient (CDrawContext* context, const CGradient& gradient, const CPoint& startPoint, const CPoint& endPoint, bool evenOdd = false, CGraphicsTransformation* transformation = 0) = 0;
	//@}
	
	//-----------------------------------------------------------------------------
	/// @name States
	//-----------------------------------------------------------------------------
	//@{
	virtual CPoint getCurrentPosition () const = 0;
	virtual CRect getBoundingBox () const = 0;
	//@}
	
//-----------------------------------------------------------------------------
	CLASS_METHODS_NOCOPY(CGraphicsPath, CBaseObject)
};

//-----------------------------------------------------------------------------
/*! @class CGradient
	@brief Gradient Object [new in 4.0]
*/
class CGradient : public CBaseObject
{
public:
	//-----------------------------------------------------------------------------
	/// @name Member Access
	//-----------------------------------------------------------------------------
	//@{
	double getColor1Start () const { return color1Start; }
	double getColor2Start () const { return color2Start; }
	const CColor& getColor1 () const { return color1; }
	const CColor& getColor2 () const { return color2; }
	//@}
//-----------------------------------------------------------------------------
	CLASS_METHODS_NOCOPY(CGradient, CBaseObject)
protected:
	CGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
	: color1Start (color1Start), color2Start (color2Start), color1 (color1), color2 (color2) {}
	
	double color1Start;
	double color2Start;
	CColor color1;
	CColor color2;
};

} // namespace

#endif
