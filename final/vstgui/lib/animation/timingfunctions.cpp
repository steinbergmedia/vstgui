//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

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
	points.insert (std::make_pair ((uint32_t)((float)getLength () * time), pos));
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
		it++;
	}
	return 1.f;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
RepeatTimingFunction::RepeatTimingFunction (TimingFunctionBase* tf, int32_t repeatCount, bool autoReverse)
: tf (tf)
, repeatCount (repeatCount)
, autoReverse (autoReverse)
, isReverse (false)
, runCounter (0)
{
}

//-----------------------------------------------------------------------------
RepeatTimingFunction::~RepeatTimingFunction ()
{
	CBaseObject* obj = dynamic_cast<CBaseObject*> (tf);
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
