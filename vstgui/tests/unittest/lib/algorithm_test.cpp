// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/algorithm.h"
#include "../unittests.h"

namespace VSTGUI {

//------------------------------------------------------------------------
TEST_CASE (Algorithm, Clamp)
{
	EXPECT_EQ (clamp (2.8, 1.4, 1.8), 1.8);
	EXPECT_EQ (clamp (1.2, 1.4, 1.8), 1.4);
}

//------------------------------------------------------------------------
TEST_CASE (Algorithm, ClampNorm)
{
	EXPECT_EQ (clampNorm (1.8), 1.);
	EXPECT_EQ (clampNorm (-0.1), 0.);
}

//------------------------------------------------------------------------
TEST_CASE (Algorithm, NormalizedToSteps)
{
	EXPECT_EQ (normalizedToSteps (0., 1), 0);
	EXPECT_EQ (normalizedToSteps (0.49, 1), 0);
	EXPECT_EQ (normalizedToSteps (0.50, 1), 1);
	EXPECT_EQ (normalizedToSteps (0.51, 1), 1);
	EXPECT_EQ (normalizedToSteps (1., 1), 1);

	EXPECT_EQ (normalizedToSteps (0., 3), 0);
	EXPECT_EQ (normalizedToSteps (0.24, 3), 0);
	EXPECT_EQ (normalizedToSteps (0.25, 3), 1);
	EXPECT_EQ (normalizedToSteps (0.26, 3), 1);
	EXPECT_EQ (normalizedToSteps (0.49, 3), 1);
	EXPECT_EQ (normalizedToSteps (0.50, 3), 2);
	EXPECT_EQ (normalizedToSteps (0.51, 3), 2);
	EXPECT_EQ (normalizedToSteps (0.74, 3), 2);
	EXPECT_EQ (normalizedToSteps (0.75, 3), 3);
	EXPECT_EQ (normalizedToSteps (0.76, 3), 3);
	EXPECT_EQ (normalizedToSteps (1., 3), 3);

	EXPECT_EQ (normalizedToSteps (0., 1, 1), 1);
	EXPECT_EQ (normalizedToSteps (0.5, 1, 1), 2);
	EXPECT_EQ (normalizedToSteps (1., 1, 1), 2);
}

//------------------------------------------------------------------------
TEST_CASE (Agorithm, StepsToNormalized)
{
	EXPECT_EQ (stepsToNormalized<double> (1, 1), 1.);
	EXPECT_EQ (stepsToNormalized<double> (0, 1), 0.);

	EXPECT_EQ (stepsToNormalized<double> (0, 4), 0.);
	EXPECT_EQ (stepsToNormalized<double> (1, 4), 0.25);
	EXPECT_EQ (stepsToNormalized<double> (2, 4), 0.50);
	EXPECT_EQ (stepsToNormalized<double> (3, 4), 0.75);
	EXPECT_EQ (stepsToNormalized<double> (4, 4), 1.);

	EXPECT_EQ (stepsToNormalized<double> (2, 1, 1), 1.);
	EXPECT_EQ (stepsToNormalized<double> (1, 1, 1), 0.);
}

} // VSTGUI
