//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
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

#ifndef __timingfunctions__
#define __timingfunctions__

#include "animator.h"
#include <map>

namespace VSTGUI {
namespace Animation {

//-----------------------------------------------------------------------------
class TimingFunctionBase : public ITimingFunction
{
public:
	TimingFunctionBase (unsigned long length) : length (length) {}

	unsigned long getLength () const { return length; }
	bool isDone (unsigned long milliseconds) { return milliseconds >= length; }
protected:
	unsigned long length; // in milliseconds
};

//-----------------------------------------------------------------------------
/// @ingroup AnimationTimingFunctions
//-----------------------------------------------------------------------------
class LinearTimingFunction : public TimingFunctionBase
{
public:
	LinearTimingFunction (unsigned long length);

protected:
	float getPosition (unsigned long milliseconds);
};

//-----------------------------------------------------------------------------
/// @ingroup AnimationTimingFunctions
//-----------------------------------------------------------------------------
class PowerTimingFunction : public TimingFunctionBase
{
public:
	PowerTimingFunction (unsigned long length, float factor);

protected:
	float getPosition (unsigned long milliseconds);

	float factor;
};

//-----------------------------------------------------------------------------
/// @ingroup AnimationTimingFunctions
//-----------------------------------------------------------------------------
class InterpolationTimingFunction : public TimingFunctionBase
{
public:
	InterpolationTimingFunction (unsigned long length, float startPos = 0.f, float endPos = 1.f);

	void addPoint (float time, float pos); ///< both values are normalized ones

protected:
	float getPosition (unsigned long milliseconds);

	std::map<unsigned long, float> points;
};

//-----------------------------------------------------------------------------
/// @ingroup AnimationTimingFunctions
//-----------------------------------------------------------------------------
class RepeatTimingFunction : public ITimingFunction
{
public:
	RepeatTimingFunction (TimingFunctionBase* tf, long repeatCount, bool autoReverse = true);
	~RepeatTimingFunction ();

	float getPosition (unsigned long milliseconds);
	bool isDone (unsigned long milliseconds);
protected:
	TimingFunctionBase* tf;
	long repeatCount;
	bool autoReverse;
	unsigned long runCounter;
	bool isReverse;
};

}} // namespaces

#endif // __timingfunctions__
