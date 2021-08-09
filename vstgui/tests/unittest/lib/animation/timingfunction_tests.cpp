// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/animation/timingfunctions.h"
#include "../../../../lib/controls/ccontrol.h"
#include "../../../../lib/cview.h"
#include "../../../../lib/cviewcontainer.h"
#include "../../unittests.h"

namespace VSTGUI {
using namespace Animation;

TEST_CASE (TimingFunctionTest, LinearTimingFunction)
{
	LinearTimingFunction f (100);
	ITimingFunction& tf = dynamic_cast<ITimingFunction&> (f);
	EXPECT (tf.getPosition (0) == 0.f);
	EXPECT (tf.getPosition (50) == 0.5f);
	EXPECT (tf.getPosition (100) == 1.f);
	EXPECT (tf.getPosition (150) == 1.f);
}

TEST_CASE (TimingFunctionTest, PowerTwoTimingFunction)
{
	PowerTimingFunction f (100, 2);
	ITimingFunction& tf = dynamic_cast<ITimingFunction&> (f);
	EXPECT (tf.getPosition (0) == 0.f);
	EXPECT (tf.getPosition (50) == 0.25f);
	EXPECT (tf.getPosition (86) == 0.7396f);
	EXPECT (tf.getPosition (100) == 1.f);
	EXPECT (tf.getPosition (150) == 1.f);
}

TEST_CASE (TimingFunctionTest, PowerFourTimingFunction)
{
	PowerTimingFunction f (100, 4);
	ITimingFunction& tf = dynamic_cast<ITimingFunction&> (f);
	EXPECT (tf.getPosition (0) == 0.f);
	EXPECT (tf.getPosition (50) == 0.0625f);
	EXPECT (tf.getPosition (100) == 1.f);
	EXPECT (tf.getPosition (150) == 1.f);
}

TEST_CASE (TimingFunctionTest, InterpolationTimingFunction)
{
	InterpolationTimingFunction f (100);
	f.addPoint (0.5, 0.4f);
	ITimingFunction& tf = dynamic_cast<ITimingFunction&> (f);
	EXPECT (tf.getPosition (0) == 0.f);
	EXPECT (tf.getPosition (25) == 0.2f);
	EXPECT (tf.getPosition (50) == 0.4f);
	EXPECT (tf.getPosition (75) == 0.7f);
	EXPECT (tf.getPosition (100) == 1.f);
	EXPECT (tf.getPosition (150) == 1.f);
}

TEST_CASE (TimingFunctionTest, RepeatTimingFunction)
{
	auto lf = new LinearTimingFunction (100);
	RepeatTimingFunction f (lf, 2, false);
	ITimingFunction& tf = dynamic_cast<ITimingFunction&> (f);
	EXPECT (tf.getPosition (0) == 0.f);
	EXPECT (tf.getPosition (50) == 0.5f);
	EXPECT (tf.getPosition (100) == 1.f);
	EXPECT (tf.isDone (100) == false);
	EXPECT (tf.getPosition (110) == 0.1f);
	EXPECT (tf.getPosition (150) == 0.5f);
	EXPECT (tf.getPosition (200) == 1.0f);
	EXPECT (tf.isDone (200) == true);
}

TEST_CASE (TimingFunctionTest, RepeatTimingFunctionReverse)
{
	auto lf = new LinearTimingFunction (100);
	RepeatTimingFunction f (lf, 2, true);
	ITimingFunction& tf = dynamic_cast<ITimingFunction&> (f);
	EXPECT (tf.getPosition (0) == 0.f);
	EXPECT (tf.getPosition (50) == 0.5f);
	EXPECT (tf.getPosition (100) == 1.f);
	EXPECT (tf.isDone (100) == false);
	EXPECT (tf.getPosition (110) == 0.9f);
	EXPECT (tf.getPosition (150) == 0.5f);
	EXPECT (tf.getPosition (200) == 0.f);
	EXPECT (tf.isDone (200) == true);
}

TEST_CASE (TimingFunctionTest, CubicBezierTimingFunction)
{
	CubicBezierTimingFunction f (100, CPoint (0.42, 0.), CPoint (0.58, 1.));
	EXPECT (f.getPosition (0) == 0.f);
	EXPECT (f.getPosition (25) < 0.25f);
	EXPECT (f.getPosition (50) == 0.5f);
	EXPECT (f.getPosition (75) > 0.75f);
	EXPECT (f.getPosition (100) == 1.0f);
}

} // VSTGUI
