/*
 * TinyJS
 *
 * A single-file Javascript-alike engine
 *
 * -  Math and Trigonometry functions
 *
 * Authored By O.Z.L.B. <ozlbinfo@gmail.com>
 *
 * Copyright (C) 2011 O.Z.L.B.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cmath>
#include <cstdlib>
#include <sstream>
#include "TinyJS_MathFunctions.h"

//------------------------------------------------------------------------
namespace TJS {

using namespace std;

#define k_E exp (1.0)
#define k_PI 3.1415926535897932384626433832795

#define F_ABS(a) ((a) >= 0 ? (a) : (-(a)))
#define F_MIN(a, b) ((a) > (b) ? (b) : (a))
#define F_MAX(a, b) ((a) > (b) ? (a) : (b))
#define F_SGN(a) ((a) > 0 ? 1 : ((a) < 0 ? -1 : 0))
#define F_RNG(a, min, max) ((a) < (min) ? min : ((a) > (max) ? max : a))
#define F_ROUND(a) ((a) > 0 ? (int)((a) + 0.5) : (int)((a)-0.5))

// CScriptVar shortcut macro
#define scIsInt(a) (c->getParameter (a)->isInt ())
#define scIsDouble(a) (c->getParameter (a)->isDouble ())
#define scGetInt(a) (c->getParameter (a)->getInt ())
#define scGetDouble(a) (c->getParameter (a)->getDouble ())
#define scReturnInt(a) (c->getReturnVar ()->setInt (a))
#define scReturnDouble(a) (c->getReturnVar ()->setDouble (a))

#ifdef _MSC_VER
namespace {
double asinh (const double& value)
{
	double returned;

	if (value > 0)
		returned = log (value + sqrt (value * value + 1));
	else
		returned = -log (-value + sqrt (value * value + 1));

	return (returned);
}

double acosh (const double& value)
{
	double returned;

	if (value > 0)
		returned = log (value + sqrt (value * value - 1));
	else
		returned = -log (-value + sqrt (value * value - 1));

	return (returned);
}
}
#endif

// Math.abs(x) - returns absolute of given value
void scMathAbs (CScriptVar* c)
{
	if (scIsInt ("a"))
	{
		scReturnInt (F_ABS (scGetInt ("a")));
	}
	else if (scIsDouble ("a"))
	{
		scReturnDouble (F_ABS (scGetDouble ("a")));
	}
}

// Math.round(a) - returns nearest round of given value
void scMathRound (CScriptVar* c)
{
	if (scIsInt ("a"))
	{
		scReturnInt (F_ROUND (scGetInt ("a")));
	}
	else if (scIsDouble ("a"))
	{
		scReturnDouble (F_ROUND (scGetDouble ("a")));
	}
}

// Math.min(a,b) - returns minimum of two given values
void scMathMin (CScriptVar* c)
{
	if ((scIsInt ("a")) && (scIsInt ("b")))
	{
		scReturnInt (F_MIN (scGetInt ("a"), scGetInt ("b")));
	}
	else
	{
		scReturnDouble (F_MIN (scGetDouble ("a"), scGetDouble ("b")));
	}
}

// Math.max(a,b) - returns maximum of two given values
void scMathMax (CScriptVar* c)
{
	if ((scIsInt ("a")) && (scIsInt ("b")))
	{
		scReturnInt (F_MAX (scGetInt ("a"), scGetInt ("b")));
	}
	else
	{
		scReturnDouble (F_MAX (scGetDouble ("a"), scGetDouble ("b")));
	}
}

// Math.range(x,a,b) - returns value limited between two given values
void scMathRange (CScriptVar* c)
{
	if ((scIsInt ("x")))
	{
		scReturnInt (F_RNG (scGetInt ("x"), scGetInt ("a"), scGetInt ("b")));
	}
	else
	{
		scReturnDouble (F_RNG (scGetDouble ("x"), scGetDouble ("a"), scGetDouble ("b")));
	}
}

// Math.sign(a) - returns sign of given value (-1==negative,0=zero,1=positive)
void scMathSign (CScriptVar* c)
{
	if (scIsInt ("a"))
	{
		scReturnInt (F_SGN (scGetInt ("a")));
	}
	else if (scIsDouble ("a"))
	{
		scReturnDouble (F_SGN (scGetDouble ("a")));
	}
}

// Math.PI() - returns PI value
void scMathPI (CScriptVar* c) { scReturnDouble (k_PI); }

// Math.toDegrees(a) - returns degree value of a given angle in radians
void scMathToDegrees (CScriptVar* c) { scReturnDouble ((180.0 / k_PI) * (scGetDouble ("a"))); }

// Math.toRadians(a) - returns radians value of a given angle in degrees
void scMathToRadians (CScriptVar* c) { scReturnDouble ((k_PI / 180.0) * (scGetDouble ("a"))); }

// Math.sin(a) - returns trig. sine of given angle in radians
void scMathSin (CScriptVar* c) { scReturnDouble (sin (scGetDouble ("a"))); }

// Math.asin(a) - returns trig. arcsine of given angle in radians
void scMathASin (CScriptVar* c) { scReturnDouble (asin (scGetDouble ("a"))); }

// Math.cos(a) - returns trig. cosine of given angle in radians
void scMathCos (CScriptVar* c) { scReturnDouble (cos (scGetDouble ("a"))); }

// Math.acos(a) - returns trig. arccosine of given angle in radians
void scMathACos (CScriptVar* c) { scReturnDouble (acos (scGetDouble ("a"))); }

// Math.tan(a) - returns trig. tangent of given angle in radians
void scMathTan (CScriptVar* c) { scReturnDouble (tan (scGetDouble ("a"))); }

// Math.atan(a) - returns trig. arctangent of given angle in radians
void scMathATan (CScriptVar* c) { scReturnDouble (atan (scGetDouble ("a"))); }

// Math.sinh(a) - returns trig. hyperbolic sine of given angle in radians
void scMathSinh (CScriptVar* c) { scReturnDouble (sinh (scGetDouble ("a"))); }

// Math.asinh(a) - returns trig. hyperbolic arcsine of given angle in radians
void scMathASinh (CScriptVar* c) { scReturnDouble (asinh ((long double)scGetDouble ("a"))); }

// Math.cosh(a) - returns trig. hyperbolic cosine of given angle in radians
void scMathCosh (CScriptVar* c) { scReturnDouble (cosh (scGetDouble ("a"))); }

// Math.acosh(a) - returns trig. hyperbolic arccosine of given angle in radians
void scMathACosh (CScriptVar* c) { scReturnDouble (acosh ((long double)scGetDouble ("a"))); }

// Math.tanh(a) - returns trig. hyperbolic tangent of given angle in radians
void scMathTanh (CScriptVar* c) { scReturnDouble (tanh (scGetDouble ("a"))); }

// Math.atan(a) - returns trig. hyperbolic arctangent of given angle in radians
void scMathATanh (CScriptVar* c) { scReturnDouble (atan (scGetDouble ("a"))); }

// Math.E() - returns E Neplero value
void scMathE (CScriptVar* c) { scReturnDouble (k_E); }

// Math.log(a) - returns natural logaritm (base E) of given value
void scMathLog (CScriptVar* c) { scReturnDouble (log (scGetDouble ("a"))); }

// Math.log10(a) - returns logaritm(base 10) of given value
void scMathLog10 (CScriptVar* c) { scReturnDouble (log10 (scGetDouble ("a"))); }

// Math.exp(a) - returns e raised to the power of a given number
void scMathExp (CScriptVar* c) { scReturnDouble (exp (scGetDouble ("a"))); }

// Math.pow(a,b) - returns the result of a number raised to a power (a)^(b)
void scMathPow (CScriptVar* c) { scReturnDouble (pow (scGetDouble ("a"), scGetDouble ("b"))); }

// Math.sqr(a) - returns square of given value
void scMathSqr (CScriptVar* c) { scReturnDouble ((scGetDouble ("a") * scGetDouble ("a"))); }

// Math.sqrt(a) - returns square root of given value
void scMathSqrt (CScriptVar* c) { scReturnDouble (sqrt (scGetDouble ("a"))); }

// ----------------------------------------------- Register Functions
void registerMathFunctions (CTinyJS* tinyJS)
{
	using namespace std::literals;

	// --- Math and Trigonometry functions ---
	tinyJS->addNative ("function Math.abs(a)"sv, scMathAbs);
	tinyJS->addNative ("function Math.round(a)"sv, scMathRound);
	tinyJS->addNative ("function Math.min(a,b)"sv, scMathMin);
	tinyJS->addNative ("function Math.max(a,b)"sv, scMathMax);
	tinyJS->addNative ("function Math.range(x,a,b)"sv, scMathRange);
	tinyJS->addNative ("function Math.sign(a)"sv, scMathSign);

	tinyJS->addNative ("function Math.PI()"sv, scMathPI);
	tinyJS->addNative ("function Math.toDegrees(a)"sv, scMathToDegrees);
	tinyJS->addNative ("function Math.toRadians(a)"sv, scMathToRadians);
	tinyJS->addNative ("function Math.sin(a)"sv, scMathSin);
	tinyJS->addNative ("function Math.asin(a)"sv, scMathASin);
	tinyJS->addNative ("function Math.cos(a)"sv, scMathCos);
	tinyJS->addNative ("function Math.acos(a)"sv, scMathACos);
	tinyJS->addNative ("function Math.tan(a)"sv, scMathTan);
	tinyJS->addNative ("function Math.atan(a)"sv, scMathATan);
	tinyJS->addNative ("function Math.sinh(a)"sv, scMathSinh);
	tinyJS->addNative ("function Math.asinh(a)"sv, scMathASinh);
	tinyJS->addNative ("function Math.cosh(a)"sv, scMathCosh);
	tinyJS->addNative ("function Math.acosh(a)"sv, scMathACosh);
	tinyJS->addNative ("function Math.tanh(a)"sv, scMathTanh);
	tinyJS->addNative ("function Math.atanh(a)"sv, scMathATanh);

	tinyJS->addNative ("function Math.E()"sv, scMathE);
	tinyJS->addNative ("function Math.log(a)"sv, scMathLog);
	tinyJS->addNative ("function Math.log10(a)"sv, scMathLog10);
	tinyJS->addNative ("function Math.exp(a)"sv, scMathExp);
	tinyJS->addNative ("function Math.pow(a,b)"sv, scMathPow);

	tinyJS->addNative ("function Math.sqr(a)"sv, scMathSqr);
	tinyJS->addNative ("function Math.sqrt(a)"sv, scMathSqrt);
}

//------------------------------------------------------------------------
} // TJS
