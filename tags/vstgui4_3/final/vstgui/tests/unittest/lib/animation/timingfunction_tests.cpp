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

#include "../../../../lib/animation/timingfunctions.h"
#include "../../../../lib/cview.h"
#include "../../../../lib/cviewcontainer.h"
#include "../../../../lib/controls/ccontrol.h"
#include "../../unittests.h"

namespace VSTGUI {
using namespace Animation;

TESTCASE(TimingFunctionTests,
	TEST(linearTimingFunction,
		LinearTimingFunction f (100);
		ITimingFunction& tf = dynamic_cast<ITimingFunction&>(f);
		EXPECT(tf.getPosition (0) == 0.f);
		EXPECT(tf.getPosition (50) == 0.5f);
		EXPECT(tf.getPosition (100) == 1.f);
		EXPECT(tf.getPosition (150) == 1.f);
	);
	TEST(powerTwoTimingFunction,
		PowerTimingFunction f (100, 2);
		ITimingFunction& tf = dynamic_cast<ITimingFunction&>(f);
		EXPECT(tf.getPosition (0) == 0.f);
		EXPECT(tf.getPosition (50) == 0.25f);
		EXPECT(tf.getPosition (86) == 0.7396f);
		EXPECT(tf.getPosition (100) == 1.f);
		EXPECT(tf.getPosition (150) == 1.f);
	);
	TEST(powerFourTimingFunction,
		PowerTimingFunction f (100, 4);
		ITimingFunction& tf = dynamic_cast<ITimingFunction&>(f);
		EXPECT(tf.getPosition (0) == 0.f);
		EXPECT(tf.getPosition (50) == 0.0625f);
		EXPECT(tf.getPosition (100) == 1.f);
		EXPECT(tf.getPosition (150) == 1.f);
	);
	TEST(interpolationTimingFunction,
		InterpolationTimingFunction f (100);
		f.addPoint (0.5, 0.4f);
		ITimingFunction& tf = dynamic_cast<ITimingFunction&>(f);
		EXPECT(tf.getPosition (0) == 0.f);
		EXPECT(tf.getPosition (25) == 0.2f);
		EXPECT(tf.getPosition (50) == 0.4f);
		EXPECT(tf.getPosition (75) == 0.7f);
		EXPECT(tf.getPosition (100) == 1.f);
		EXPECT(tf.getPosition (150) == 1.f);
	);
	TEST(repeatTimingFunction,
		auto lf = new LinearTimingFunction (100);
		RepeatTimingFunction f (lf, 2, false);
		ITimingFunction& tf = dynamic_cast<ITimingFunction&>(f);
		EXPECT(tf.getPosition (0) == 0.f);
		EXPECT(tf.getPosition (50) == 0.5f);
		EXPECT(tf.getPosition (100) == 1.f);
		EXPECT(tf.isDone (100) == false);
		EXPECT(tf.getPosition (110) == 0.1f);
		EXPECT(tf.getPosition (150) == 0.5f);
		EXPECT(tf.getPosition (200) == 1.0f);
		EXPECT(tf.isDone (200) == true);
	);
	TEST(repeatTimingFunctionReverse,
		auto lf = new LinearTimingFunction (100);
		RepeatTimingFunction f (lf, 2, true);
		ITimingFunction& tf = dynamic_cast<ITimingFunction&>(f);
		EXPECT(tf.getPosition (0) == 0.f);
		EXPECT(tf.getPosition (50) == 0.5f);
		EXPECT(tf.getPosition (100) == 1.f);
		EXPECT(tf.isDone (100) == false);
		EXPECT(tf.getPosition (110) == 0.9f);
		EXPECT(tf.getPosition (150) == 0.5f);
		EXPECT(tf.getPosition (200) == 0.f);
		EXPECT(tf.isDone (200) == true);
	);
);

} // VSTGUI
