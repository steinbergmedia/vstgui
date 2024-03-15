// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/enumbitset.h"
#include "../unittests.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
enum class Flag
{
	One,
	Two,
	Three,
};
using Flags = EnumBitset<Flag>;

//------------------------------------------------------------------------
enum class FlagMask : uint8_t
{
	One = 1 << 0,
	Two = 1 << 1,
	Three = 1 << 2,
};
using FlagMasks = EnumBitset<FlagMask, true>;

//------------------------------------------------------------------------
TEST_CASE (EnumBitsetTest, InitTest)
{
	{
		Flags f;
		EXPECT_EQ (f.value (), 0);
	}
	{
		Flags f (Flag::Two);
		EXPECT_EQ (f.value (), 2);
	}
	{
		Flags f ({Flag::One, Flag::Two});
		EXPECT_EQ (f.value (), 3);
	}
	{
		FlagMasks f;
		EXPECT_EQ (f.value (), 0);
	}
	{
		FlagMasks f (FlagMask::Two);
		EXPECT_EQ (f.value (), 2);
	}
	{
		FlagMasks f ({FlagMask::One, FlagMask::Two});
		EXPECT_EQ (f.value (), 3);
	}
}

//------------------------------------------------------------------------
TEST_CASE (EnumBitsetTest, AssignmentTest)
{
	{
		Flags f;
		f = Flag::One;
		EXPECT_EQ (f.value (), 1);

		Flags f2 = Flag::Two;
		EXPECT_EQ (f2.value (), 2);

		f = f2;
		EXPECT_EQ (f.value (), f2.value ());

		f2.exlusive (Flag::Three);
		EXPECT_EQ (f2.value (), 4);
	}
	{
		FlagMasks f;
		f = FlagMask::One;
		EXPECT_EQ (f.value (), 1);

		FlagMasks f2 = FlagMask::Two;
		EXPECT_EQ (f2.value (), 2);

		f = f2;
		EXPECT_EQ (f.value (), f2.value ());

		f2.exlusive (FlagMask::Three);
		EXPECT_EQ (f2.value (), 4);
	}
}

//------------------------------------------------------------------------
TEST_CASE (EnumBitsetTest, AddRemoveTest)
{
	{
		Flags f;
		f.add (Flag::One);
		EXPECT_EQ (f.value (), 1);
		f.add (Flag::Two);
		EXPECT_EQ (f.value (), 3);
		f.remove (Flag::One);
		EXPECT_EQ (f.value (), 2);

		f.clear ();

		f |= Flag::One;
		EXPECT_EQ (f.value (), 1);
		f |= Flag::Two;
		EXPECT_EQ (f.value (), 3);

		f ^= Flag::One;
		EXPECT_EQ (f.value (), 2);

		f.clear ();

		f << Flag::One;
		EXPECT_EQ (f.value (), 1);
		f << Flag::Two;
		EXPECT_EQ (f.value (), 3);
		f >> Flag::One;
		EXPECT_EQ (f.value (), 2);
	}
	{
		FlagMasks f;
		f.add (FlagMask::One);
		EXPECT_EQ (f.value (), 1);
		f.add (FlagMask::Two);
		EXPECT_EQ (f.value (), 3);
		f.remove (FlagMask::One);
		EXPECT_EQ (f.value (), 2);

		f.clear ();

		f |= FlagMask::One;
		EXPECT_EQ (f.value (), 1);
		f |= FlagMask::Two;
		EXPECT_EQ (f.value (), 3);

		f ^= FlagMask::One;
		EXPECT_EQ (f.value (), 2);

		f.clear ();

		f << FlagMask::One;
		EXPECT_EQ (f.value (), 1);
		f << FlagMask::Two;
		EXPECT_EQ (f.value (), 3);
		f >> FlagMask::One;
		EXPECT_EQ (f.value (), 2);
	}
}

//------------------------------------------------------------------------
TEST_CASE (EnumBitsetTest, OperatorTest)
{
	{
		Flags f1 = Flag::One;
		Flags f2 ({Flag::Two, Flag::Three});

		auto f3 = f1 | Flag::Two;
		EXPECT_EQ (f3.value (), 3);

		EXPECT_TRUE (f3 & Flag::One);
		EXPECT_TRUE (f3 & Flag::Two);
	}
	{
		FlagMasks f1 = FlagMask::One;
		FlagMasks f2 ({FlagMask::Two, FlagMask::Three});

		auto f3 = f1 | FlagMask::Two;
		EXPECT_EQ (f3.value (), 3);

		EXPECT_TRUE (f3 & FlagMask::One);
		EXPECT_TRUE (f3 & FlagMask::Two);
	}
}

//------------------------------------------------------------------------
TEST_CASE (EnumBitsetTest, EqualityTest)
{
	{
		Flags f1 ({Flag::One, Flag::Three});
		Flags f2 ({Flag::Two, Flag::Three});
		Flags f3 ({Flag::Three, Flag::One});

		EXPECT_TRUE (f1 != f2);
		EXPECT_TRUE (f1 == f3);

		EXPECT_TRUE (f1.test (Flag::One));
		EXPECT_FALSE (f1.test (Flag::Two));
	}
	{
		FlagMasks f1 ({FlagMask::One, FlagMask::Three});
		FlagMasks f2 ({FlagMask::Two, FlagMask::Three});
		FlagMasks f3 ({FlagMask::Three, FlagMask::One});

		EXPECT_TRUE (f1 != f2);
		EXPECT_TRUE (f1 == f3);

		EXPECT_TRUE (f1.test (FlagMask::One));
		EXPECT_FALSE (f1.test (FlagMask::Two));
	}
}

//------------------------------------------------------------------------
} // VSTGUI
