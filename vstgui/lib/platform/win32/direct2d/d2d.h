// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../../cgraphicstransform.h"
#include "../../../ccolor.h"

#include <d2d1_1.h>

//------------------------------------------------------------------------
namespace VSTGUI {

namespace {

using TransformMatrix = CGraphicsTransform;

//-----------------------------------------------------------------------------
template<typename T>
void pixelAlign (const TransformMatrix& tm, T& obj)
{
	CGraphicsTransform tInv = tm.inverse ();
	tm.transform (obj);
	obj.makeIntegral ();
	tInv.transform (obj);
}

//------------------------------------------------------------------------
inline D2D1_POINT_2F convert (const CPoint& p)
{
	D2D1_POINT_2F dp = {(FLOAT)p.x, (FLOAT)p.y};
	return dp;
}

//------------------------------------------------------------------------
inline D2D1::ColorF convert (CColor c, double alpha)
{
	return D2D1::ColorF (c.normRed<float> (), c.normGreen<float> (), c.normBlue<float> (),
						 static_cast<FLOAT> (c.normAlpha<double> () * alpha));
}

//------------------------------------------------------------------------
inline D2D1_RECT_F convert (const CRect& r)
{
	D2D1_RECT_F dr = {(FLOAT)r.left, (FLOAT)r.top, (FLOAT)r.right, (FLOAT)r.bottom};
	return dr;
}

//------------------------------------------------------------------------
inline D2D1_MATRIX_3X2_F convert (const TransformMatrix& t)
{
	D2D1_MATRIX_3X2_F matrix;
	matrix._11 = static_cast<FLOAT> (t.m11);
	matrix._12 = static_cast<FLOAT> (t.m21);
	matrix._21 = static_cast<FLOAT> (t.m12);
	matrix._22 = static_cast<FLOAT> (t.m22);
	matrix._31 = static_cast<FLOAT> (t.dx);
	matrix._32 = static_cast<FLOAT> (t.dy);
	return matrix;
}

//------------------------------------------------------------------------
inline TransformMatrix convert (const D2D1_MATRIX_3X2_F& t)
{
	TransformMatrix matrix;
	matrix.m11 = static_cast<FLOAT> (t._11);
	matrix.m21 = static_cast<FLOAT> (t._12);
	matrix.m12 = static_cast<FLOAT> (t._21);
	matrix.m22 = static_cast<FLOAT> (t._22);
	matrix.dx = static_cast<FLOAT> (t._31);
	matrix.dy = static_cast<FLOAT> (t._32);
	return matrix;
}

//------------------------------------------------------------------------
} // anonymous namespace

//------------------------------------------------------------------------
} // VSTGUI
