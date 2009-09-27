//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2009, Steinberg Media Technologies, All Rights Reserved
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

#define VSTGUI_CGRAPHICSPATH_AVAILABLE	((defined (VSTGUI_FLOAT_COORDINATES) && (GDIPLUS || (VSTGUI_USES_COREGRAPHICS && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5))) || VSTGUI_BUILD_DOXYGEN)

#if VSTGUI_CGRAPHICSPATH_AVAILABLE

BEGIN_NAMESPACE_VSTGUI
class PlatformGraphicsPath;
class CGradient;

//-----------------------------------------------------------------------------
/// @brief Graphics Path Transformation [new since 4.0]
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
	@brief Graphics Path Object [new since 4.0]

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

	//-----------------------------------------------------------------------------
	/// @name Adding Elements
	//-----------------------------------------------------------------------------
	//@{
	void addArc (const CRect& rect, double startAngle, double endAngle);
	void addCurve (const CPoint& start, const CPoint& control1, const CPoint& control2, const CPoint& end);
	void addEllipse (const CRect& rect);
	void addLine (const CPoint& start, const CPoint& end);
	void addRect (const CRect& rect);
	void addPath (const CGraphicsPath& path, CGraphicsTransformation* transformation = 0);
	void addString (const char* utf8String, CFontRef font, const CPoint& position);
	void closeSubpath ();
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

	void draw (CDrawContext* context, PathDrawMode mode = kFilled, CGraphicsTransformation* transformation = 0);
	void fillLinearGradient (CDrawContext* context, const CGradient& gradient, const CPoint& color1StartPoint, const CPoint& color2StartPoint, bool evenOdd = false, CGraphicsTransformation* transformation = 0);
	//@}
	
	//-----------------------------------------------------------------------------
	/// @name States
	//-----------------------------------------------------------------------------
	//@{
	CPoint getCurrentPosition () const;
	CRect getBoundingBox () const;
	//@}
	
//-----------------------------------------------------------------------------
	CLASS_METHODS_NOCOPY(CGraphicsPath, CBaseObject)
protected:
	PlatformGraphicsPath* platformPath;
};

//-----------------------------------------------------------------------------
/*! @class CGradient
	@brief Gradient Object [new since 4.0]

	Only available when VSTGUI_FLOAT_COORDINATES is defined.
	On Windows GDIPLUS must be defined.
	On Mac OS X only available when building for Mac OS X 10.5 or newer.
*/
class CGradient : public CBaseObject
{
public:
	//-----------------------------------------------------------------------------
	/// @name Creating a Gradient Object
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
	static CGradient* create (double color1Start, double color2Start, const CColor& color1, const CColor& color2);
	//@}

	//-----------------------------------------------------------------------------
	/// @name Property Access
	//-----------------------------------------------------------------------------
	//@{
	double getColor1Start () const { return color1Start; }
	double getColor2Start () const { return color2Start; }
	const CColor& getColor1 () const { return color1; }
	const CColor& getColor2 () const { return color2; }
	//@}
protected:
	CGradient (double color1Start, double color2Start, const CColor& color1, const CColor& color2)
	: color1Start (color1Start), color2Start (color2Start), color1 (color1), color2 (color2) {}
	
	double color1Start;
	double color2Start;
	CColor color1;
	CColor color2;
};

#if DEBUG
class CView;
extern CView* createCGraphicsPathTestView ();
#endif

END_NAMESPACE_VSTGUI

#endif // VSTGUI_CGRAPHICSPATH_AVAILABLE

#endif
