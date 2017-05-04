// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __timingfunctions__
#define __timingfunctions__

#include "animator.h"
#include "itimingfunction.h"
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

protected:
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

protected:
	float getPosition (uint32_t milliseconds) override;

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

	void addPoint (float time, float pos); ///< both values are normalized ones

protected:
	float getPosition (uint32_t milliseconds) override;

	using PointMap = std::map<uint32_t, float>;
	PointMap points;
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

}} // namespaces

#endif // __timingfunctions__
