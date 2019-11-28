// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "animator.h"
#include "itimingfunction.h"
#include "../cpoint.h"
#include <map>

namespace VSTGUI {
namespace Animation {

//-----------------------------------------------------------------------------
/// @ingroup AnimationTimingFunctions
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class TimingFunctionBase : public ITimingFunction
{
public:
	explicit TimingFunctionBase (uint32_t length) : length (length) {}
	TimingFunctionBase (const TimingFunctionBase&) = default;
	TimingFunctionBase& operator= (const TimingFunctionBase&) = default;

	uint32_t getLength () const { return length; }
	bool isDone (uint32_t milliseconds) override { return milliseconds >= length; }

protected:
	uint32_t length; // in milliseconds
};

//-----------------------------------------------------------------------------
/// @ingroup AnimationTimingFunctions
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class LinearTimingFunction : public TimingFunctionBase
{
public:
	explicit LinearTimingFunction (uint32_t length);
	LinearTimingFunction (const LinearTimingFunction&) = default;
	LinearTimingFunction& operator= (const LinearTimingFunction&) = default;

	float getPosition (uint32_t milliseconds) override;
};

//-----------------------------------------------------------------------------
/// @ingroup AnimationTimingFunctions
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class PowerTimingFunction : public TimingFunctionBase
{
public:
	PowerTimingFunction (uint32_t length, float factor);
	PowerTimingFunction (const PowerTimingFunction&) = default;
	PowerTimingFunction& operator= (const PowerTimingFunction&) = default;

	float getPosition (uint32_t milliseconds) override;

protected:
	float factor;
};

//-----------------------------------------------------------------------------
/// @ingroup AnimationTimingFunctions
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class InterpolationTimingFunction : public TimingFunctionBase
{
public:
	InterpolationTimingFunction (uint32_t length, float startPos = 0.f, float endPos = 1.f);
	InterpolationTimingFunction (const InterpolationTimingFunction&) = default;
	InterpolationTimingFunction& operator= (const InterpolationTimingFunction&) = default;

	/** both values are normalized ones */
	void addPoint (float time, float pos);

	float getPosition (uint32_t milliseconds) override;

protected:

	using PointMap = std::map<uint32_t, float>;
	PointMap points;
};

//-----------------------------------------------------------------------------
/// @ingroup AnimationTimingFunctions
///	@ingroup new_in_4_7
//-----------------------------------------------------------------------------
class CubicBezierTimingFunction : public TimingFunctionBase
{
public:
	CubicBezierTimingFunction (uint32_t milliseconds, CPoint p1, CPoint p2);
	CubicBezierTimingFunction (const CubicBezierTimingFunction&) = default;
	CubicBezierTimingFunction& operator= (const CubicBezierTimingFunction&) = default;

	float getPosition (uint32_t milliseconds) override;

	// some common timings
	static CubicBezierTimingFunction easy (uint32_t time);
	static CubicBezierTimingFunction easyIn (uint32_t time);
	static CubicBezierTimingFunction easyOut (uint32_t time);
	static CubicBezierTimingFunction easyInOut (uint32_t time);

private:
	static CPoint lerp (CPoint p1, CPoint p2, float pos);

	CPoint p1;
	CPoint p2;
};

//-----------------------------------------------------------------------------
/// @ingroup AnimationTimingFunctions
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class RepeatTimingFunction : public ITimingFunction
{
public:
	RepeatTimingFunction (TimingFunctionBase* tf, int32_t repeatCount, bool autoReverse = true);
	~RepeatTimingFunction () noexcept override;

	float getPosition (uint32_t milliseconds) override;
	bool isDone (uint32_t milliseconds) override;
protected:
	TimingFunctionBase* tf;
	int32_t repeatCount;
	uint32_t runCounter;
	bool autoReverse;
	bool isReverse;
};

} // Animation
} // VSTGUI
