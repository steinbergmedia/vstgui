// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "timingfunctions.h"
#include "../vstguibase.h"
#include <cmath>

namespace VSTGUI {
namespace Animation {

//------------------------------------------------------------------------
/*! @defgroup AnimationTimingFunctions Animation Timing Functions
 *	@ingroup animation
 */
//------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
LinearTimingFunction::LinearTimingFunction (uint32_t length)
: TimingFunctionBase (length)
{
}

//-----------------------------------------------------------------------------
float LinearTimingFunction::getPosition (uint32_t milliseconds) 
{
	float pos = ((float)milliseconds) / ((float)length);
	if (pos > 1.f)
		pos = 1.f;
	else if (pos < 0.f)
		pos = 0.f;
	return pos;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PowerTimingFunction::PowerTimingFunction (uint32_t length, float factor)
: TimingFunctionBase (length)
, factor (factor)
{
}

//-----------------------------------------------------------------------------
float PowerTimingFunction::getPosition (uint32_t milliseconds)
{
	float pos = ((float)milliseconds) / ((float)length);
	pos = std::pow (pos, factor);
	if (pos > 1.f)
		pos = 1.f;
	else if (pos < 0.f)
		pos = 0.f;
	return pos;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
InterpolationTimingFunction::InterpolationTimingFunction (uint32_t length, float startPos, float endPos)
: TimingFunctionBase (length)
{
	addPoint (0.f, startPos);
	addPoint (1.f, endPos);
}

//-----------------------------------------------------------------------------
void InterpolationTimingFunction::addPoint (float time, float pos)
{
	points.emplace ((uint32_t)((float)getLength () * time), pos);
}

//-----------------------------------------------------------------------------
float InterpolationTimingFunction::getPosition (uint32_t milliseconds)
{
	uint32_t prevTime = getLength ();
	float prevPos = points[prevTime];
	PointMap::reverse_iterator it = points.rbegin ();
	while (it != points.rend ())
	{
		uint32_t time = it->first;
		float pos = it->second;
		if (time == milliseconds)
			return pos;
		else if (time <= milliseconds && prevTime > milliseconds)
		{
			double timePos = (double)(milliseconds - time) / (double)(prevTime - time);
			return static_cast<float> (static_cast<double> (pos) + ((static_cast<double> (prevPos) - static_cast<double> (pos)) * timePos));
		}
		prevTime = time;
		prevPos = pos;
		++it;
	}
	return 1.f;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CubicBezierTimingFunction::CubicBezierTimingFunction (uint32_t milliseconds, CPoint p1, CPoint p2)
: TimingFunctionBase (milliseconds), p1 (p1), p2 (p2)
{
}

//-----------------------------------------------------------------------------
CPoint CubicBezierTimingFunction::lerp (CPoint p1, CPoint p2, float pos)
{
	return p1 * (1.f - pos) + p2 * pos;
}

//-----------------------------------------------------------------------------
float CubicBezierTimingFunction::getPosition (uint32_t milliseconds)
{
	constexpr CPoint p0 (0, 0);
	constexpr CPoint p3 (1, 1);

	auto t = static_cast<float> (milliseconds) / static_cast<float> (length);

	auto a = lerp (p0, p1, t);
	auto b = lerp (p1, p2, t);
	auto c = lerp (p2, p3, t);
	auto d = lerp (a, b, t);
	auto e = lerp (b, c, t);
	auto result = lerp (d, e, t).y;
	return static_cast<float> (result);
}

//-----------------------------------------------------------------------------
CubicBezierTimingFunction CubicBezierTimingFunction::easy (uint32_t time)
{
	return CubicBezierTimingFunction (time, CPoint (0.25, 0.1), CPoint (0.25, 1.));
}

//-----------------------------------------------------------------------------
CubicBezierTimingFunction CubicBezierTimingFunction::easyIn (uint32_t time)
{
	return CubicBezierTimingFunction (time, CPoint (0.42, 0.), CPoint (1., 1.));
}

//-----------------------------------------------------------------------------
CubicBezierTimingFunction CubicBezierTimingFunction::easyOut (uint32_t time)
{
	return CubicBezierTimingFunction (time, CPoint (0., 0.), CPoint (0.58, 1.));
}

//-----------------------------------------------------------------------------
CubicBezierTimingFunction CubicBezierTimingFunction::easyInOut (uint32_t time)
{
	return CubicBezierTimingFunction (time, CPoint (0.42, 0.), CPoint (0.58, 1.));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
RepeatTimingFunction::RepeatTimingFunction (TimingFunctionBase* tf, int32_t repeatCount, bool autoReverse)
: tf (tf)
, repeatCount (repeatCount)
, runCounter (0)
, autoReverse (autoReverse)
, isReverse (false)
{
}

//-----------------------------------------------------------------------------
RepeatTimingFunction::~RepeatTimingFunction () noexcept
{
	auto obj = dynamic_cast<IReference*> (tf);
	if (obj)
		obj->forget ();
	else
		delete tf;
}

//-----------------------------------------------------------------------------
float RepeatTimingFunction::getPosition (uint32_t milliseconds)
{
	if (runCounter > 0)
		milliseconds -= tf->getLength () * runCounter;
	float pos = tf->getPosition (milliseconds);
	return isReverse ? 1.f - pos : pos;
}

//-----------------------------------------------------------------------------
bool RepeatTimingFunction::isDone (uint32_t milliseconds)
{
	if (runCounter > 0)
		milliseconds -= tf->getLength () * runCounter;
	if (tf->isDone (milliseconds))
	{
		runCounter++;
		if (autoReverse)
			isReverse = !isReverse;
		if ((uint64_t)runCounter >= (uint64_t)repeatCount)
			return true;
	}
	return false;
}

}} // namespaces

