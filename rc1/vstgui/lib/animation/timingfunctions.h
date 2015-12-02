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
	TimingFunctionBase (uint32_t length) : length (length) {}

	uint32_t getLength () const { return length; }
	bool isDone (uint32_t milliseconds) VSTGUI_OVERRIDE_VMETHOD { return milliseconds >= length; }
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
	LinearTimingFunction (uint32_t length);

protected:
	float getPosition (uint32_t milliseconds) VSTGUI_OVERRIDE_VMETHOD;
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
	float getPosition (uint32_t milliseconds) VSTGUI_OVERRIDE_VMETHOD;

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
	float getPosition (uint32_t milliseconds) VSTGUI_OVERRIDE_VMETHOD;

	typedef std::map<uint32_t, float> PointMap;
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
	~RepeatTimingFunction ();

	float getPosition (uint32_t milliseconds) VSTGUI_OVERRIDE_VMETHOD;
	bool isDone (uint32_t milliseconds) VSTGUI_OVERRIDE_VMETHOD;
protected:
	TimingFunctionBase* tf;
	int32_t repeatCount;
	uint32_t runCounter;
	bool autoReverse;
	bool isReverse;
};

}} // namespaces

#endif // __timingfunctions__
