// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/stringconvert.h"
#include "../unittests.h"
#include <array>

//------------------------------------------------------------------------
namespace VSTGUI {

static constexpr uint8_t grinningFaceUTF8[] = {0xF0, 0x9F, 0x98, 0x80};
static constexpr char16_t grinningFaceUTF16[2] = {0xD83D, 0xDE00};
static constexpr char32_t grinningFaceUTF32 = 0x1F600;

//------------------------------------------------------------------------
TEST_CASE (StringConvertTest, convertSingleCodePoint)
{
	auto utf8 = toUTF8 (std::u16string_view {grinningFaceUTF16, 2});
	EXPECT_EQ (utf8.size (), 4);
	EXPECT_EQ (static_cast<uint8_t> (utf8[0]), grinningFaceUTF8[0]);
	EXPECT_EQ (static_cast<uint8_t> (utf8[1]), grinningFaceUTF8[1]);
	EXPECT_EQ (static_cast<uint8_t> (utf8[2]), grinningFaceUTF8[2]);
	EXPECT_EQ (static_cast<uint8_t> (utf8[3]), grinningFaceUTF8[3]);
	utf8 = toUTF8 (std::u32string_view {&grinningFaceUTF32, 1});
	EXPECT_EQ (static_cast<uint8_t> (utf8[0]), grinningFaceUTF8[0]);
	EXPECT_EQ (static_cast<uint8_t> (utf8[1]), grinningFaceUTF8[1]);
	EXPECT_EQ (static_cast<uint8_t> (utf8[2]), grinningFaceUTF8[2]);
	EXPECT_EQ (static_cast<uint8_t> (utf8[3]), grinningFaceUTF8[3]);

	auto utf16 = toUTF16 (std::string_view {reinterpret_cast<const char*> (grinningFaceUTF8), 4});
	EXPECT_EQ (utf16.size (), 2);
	EXPECT_EQ (utf16[0], grinningFaceUTF16[0]);
	EXPECT_EQ (utf16[1], grinningFaceUTF16[1]);
	utf16 = toUTF16 (std::u32string_view {&grinningFaceUTF32, 1});
	EXPECT_EQ (utf16.size (), 2);
	EXPECT_EQ (utf16[0], grinningFaceUTF16[0]);
	EXPECT_EQ (utf16[1], grinningFaceUTF16[1]);

	auto utf32 = toUTF32 (std::string_view {reinterpret_cast<const char*> (grinningFaceUTF8), 4});
	EXPECT_EQ (utf32.size (), 1);
	EXPECT_EQ (utf32[0], grinningFaceUTF32);
	utf32 = toUTF32 (std::u16string_view {grinningFaceUTF16, 2});
	EXPECT_EQ (utf32.size (), 1);
	EXPECT_EQ (utf32[0], grinningFaceUTF32);
}

//------------------------------------------------------------------------
TEST_CASE (StringConvertTest, toUTF16)
{
	// UTF-8 -> UTF-16 for BMP only string
	static constexpr const char asciiStr[] = "Hello";
	auto utf16 = toUTF16 (std::string_view {asciiStr, sizeof (asciiStr) - 1});
	EXPECT_EQ (utf16.size (), 5);
	EXPECT_EQ (utf16[0], u'H');
	EXPECT_EQ (utf16[1], u'e');
	EXPECT_EQ (utf16[2], u'l');
	EXPECT_EQ (utf16[3], u'l');
	EXPECT_EQ (utf16[4], u'o');

	// UTF-8 -> UTF-16 for mixed BMP + surrogate pair
	static constexpr uint8_t mixedUTF8[] = {0x41, 0xF0, 0x9F, 0x98, 0x80, 0x42};
	utf16 = toUTF16 (std::string_view {reinterpret_cast<const char*> (mixedUTF8), 6});
	EXPECT_EQ (utf16.size (), 4); // A, high, low, B
	EXPECT_EQ (utf16[0], u'A');
	EXPECT_EQ (utf16[1], 0xD83D);
	EXPECT_EQ (utf16[2], 0xDE00);
	EXPECT_EQ (utf16[3], u'B');

	// UTF-32 -> UTF-16 for surrogate pair
	auto utf16From32 = toUTF16 (std::u32string_view {&grinningFaceUTF32, 1});
	EXPECT_EQ (utf16From32.size (), 2);
	EXPECT_EQ (utf16From32[0], grinningFaceUTF16[0]);
	EXPECT_EQ (utf16From32[1], grinningFaceUTF16[1]);
}

//------------------------------------------------------------------------
TEST_CASE (StringConvertTest, toUTF8)
{
	// UTF-16 (BMP) -> UTF-8
	static constexpr const char16_t hello16[] = {u'H', u'e', u'l', u'l', u'o'};
	auto utf8 = toUTF8 (std::u16string_view {hello16, 5});
	EXPECT_EQ (utf8.size (), 5);
	EXPECT_EQ (static_cast<uint8_t> (utf8[0]), 0x48);
	EXPECT_EQ (static_cast<uint8_t> (utf8[1]), 0x65);
	EXPECT_EQ (static_cast<uint8_t> (utf8[2]), 0x6C);
	EXPECT_EQ (static_cast<uint8_t> (utf8[3]), 0x6C);
	EXPECT_EQ (static_cast<uint8_t> (utf8[4]), 0x6F);

	// UTF-16 (surrogate pair) -> UTF-8
	utf8 = toUTF8 (std::u16string_view {grinningFaceUTF16, 2});
	EXPECT_EQ (utf8.size (), 4);
	EXPECT_EQ (static_cast<uint8_t> (utf8[0]), grinningFaceUTF8[0]);
	EXPECT_EQ (static_cast<uint8_t> (utf8[1]), grinningFaceUTF8[1]);
	EXPECT_EQ (static_cast<uint8_t> (utf8[2]), grinningFaceUTF8[2]);
	EXPECT_EQ (static_cast<uint8_t> (utf8[3]), grinningFaceUTF8[3]);

	// UTF-32 -> UTF-8
	utf8 = toUTF8 (std::u32string_view {&grinningFaceUTF32, 1});
	EXPECT_EQ (utf8.size (), 4);
	EXPECT_EQ (static_cast<uint8_t> (utf8[0]), grinningFaceUTF8[0]);
	EXPECT_EQ (static_cast<uint8_t> (utf8[1]), grinningFaceUTF8[1]);
	EXPECT_EQ (static_cast<uint8_t> (utf8[2]), grinningFaceUTF8[2]);
	EXPECT_EQ (static_cast<uint8_t> (utf8[3]), grinningFaceUTF8[3]);

	// UTF-16 (BMP non-ASCII) -> UTF-8 two-byte sequence: "é" (U+00E9) -> C3 A9
	static constexpr const char16_t eAcute16[] = {u'\u00E9'};
	utf8 = toUTF8 (std::u16string_view {eAcute16, 1});
	EXPECT_EQ (utf8.size (), 2);
	EXPECT_EQ (static_cast<uint8_t> (utf8[0]), 0xC3);
	EXPECT_EQ (static_cast<uint8_t> (utf8[1]), 0xA9);

	// UTF-32 -> UTF-8 with mixed multibyte characters: "é€😀" -> [C3 A9 E2 82 AC F0 9F 98 80]
	static constexpr const char32_t mixed32[] = {U'\u00E9', U'\u20AC', 0x1F600};
	utf8 = toUTF8 (std::u32string_view {mixed32, 3});
	EXPECT_EQ (utf8.size (), 9);
	EXPECT_EQ (static_cast<uint8_t> (utf8[0]), 0xC3);
	EXPECT_EQ (static_cast<uint8_t> (utf8[1]), 0xA9);
	EXPECT_EQ (static_cast<uint8_t> (utf8[2]), 0xE2);
	EXPECT_EQ (static_cast<uint8_t> (utf8[3]), 0x82);
	EXPECT_EQ (static_cast<uint8_t> (utf8[4]), 0xAC);
	EXPECT_EQ (static_cast<uint8_t> (utf8[5]), 0xF0);
	EXPECT_EQ (static_cast<uint8_t> (utf8[6]), 0x9F);
	EXPECT_EQ (static_cast<uint8_t> (utf8[7]), 0x98);
	EXPECT_EQ (static_cast<uint8_t> (utf8[8]), 0x80);
}

//------------------------------------------------------------------------
TEST_CASE (StringConvertTest, toUTF32)
{
	// UTF-8 -> UTF-32 for mixed BMP + surrogate pair
	static constexpr const uint8_t mixedUTF8[] = {0x41, 0xF0, 0x9F, 0x98, 0x80, 0x42};
	auto utf32 = toUTF32 (std::string_view {reinterpret_cast<const char*> (mixedUTF8), 6});
	EXPECT_EQ (utf32.size (), 3);
	EXPECT_EQ (utf32[0], U'A');
	EXPECT_EQ (utf32[1], grinningFaceUTF32);
	EXPECT_EQ (utf32[2], U'B');

	// UTF-16 -> UTF-32 (surrogate pair)
	utf32 = toUTF32 (std::u16string_view {grinningFaceUTF16, 2});
	EXPECT_EQ (utf32.size (), 1);
	EXPECT_EQ (utf32[0], grinningFaceUTF32);

	// UTF-8 (2-byte) -> UTF-32: "é" (U+00E9) encoded as C3 A9
	static constexpr const uint8_t twoByteUTF8[] = {0xC3, 0xA9};
	utf32 = toUTF32 (std::string_view {reinterpret_cast<const char*> (twoByteUTF8), 2});
	EXPECT_EQ (utf32.size (), 1);
	EXPECT_EQ (utf32[0], U'\u00E9');

	// UTF-8 (3-byte) -> UTF-32: "€" (U+20AC) encoded as E2 82 AC
	static constexpr const uint8_t threeByteUTF8[] = {0xE2, 0x82, 0xAC};
	utf32 = toUTF32 (std::string_view {reinterpret_cast<const char*> (threeByteUTF8), 3});
	EXPECT_EQ (utf32.size (), 1);
	EXPECT_EQ (utf32[0], U'\u20AC');

	// UTF-8 (4-byte) -> UTF-32: (U+1F600)
	utf32 = toUTF32 (std::string_view {reinterpret_cast<const char*> (grinningFaceUTF8), 4});
	EXPECT_EQ (utf32.size (), 1);
	EXPECT_EQ (utf32[0], grinningFaceUTF32);

	// UTF-8 mixed multi-byte sequence: [43 61 66 C3 A9 20 E2 82 AC 20 F0 9F 98 80]
	static constexpr const uint8_t mixedMultiUTF8[] = {0x43, 0x61, 0x66, 0xC3, 0xA9, 0x20, 0xE2,
													   0x82, 0xAC, 0x20, 0xF0, 0x9F, 0x98, 0x80};
	utf32 = toUTF32 (std::string_view {reinterpret_cast<const char*> (mixedMultiUTF8), 14});
	EXPECT_EQ (utf32.size (), 8);
	EXPECT_EQ (utf32[0], U'C');
	EXPECT_EQ (utf32[1], U'a');
	EXPECT_EQ (utf32[2], U'f');
	EXPECT_EQ (utf32[3], U'\u00E9');
	EXPECT_EQ (utf32[4], U' ');
	EXPECT_EQ (utf32[5], U'\u20AC');
	EXPECT_EQ (utf32[6], U' ');
	EXPECT_EQ (utf32[7], grinningFaceUTF32);
}

//------------------------------------------------------------------------
TEST_CASE (StringConvertTest, roundTrip)
{
	// Roundtrip UTF-8 -> UTF-16 -> UTF-8 for mixed content
	static constexpr const uint8_t mixedUTF8[] = {0x41, 0xE2, 0x82, 0xAC, 0xF0,
												  0x9F, 0x98, 0x80, 0x42};
	auto utf16 = toUTF16 (std::string_view {reinterpret_cast<const char*> (mixedUTF8), 9});
	auto utf8 = toUTF8 (std::u16string_view {utf16.data (), utf16.size ()});
	EXPECT_EQ (utf8.size (), 9);
	EXPECT_EQ (static_cast<uint8_t> (utf8[0]), 0x41);
	EXPECT_EQ (static_cast<uint8_t> (utf8[1]), 0xE2);
	EXPECT_EQ (static_cast<uint8_t> (utf8[2]), 0x82);
	EXPECT_EQ (static_cast<uint8_t> (utf8[3]), 0xAC);
	EXPECT_EQ (static_cast<uint8_t> (utf8[4]), 0xF0);
	EXPECT_EQ (static_cast<uint8_t> (utf8[5]), 0x9F);
	EXPECT_EQ (static_cast<uint8_t> (utf8[6]), 0x98);
	EXPECT_EQ (static_cast<uint8_t> (utf8[7]), 0x80);
	EXPECT_EQ (static_cast<uint8_t> (utf8[8]), 0x42);

	// Roundtrip UTF-16 -> UTF-32 -> UTF-16 for surrogate pair
	auto utf32 = toUTF32 (std::u16string_view {grinningFaceUTF16, 2});
	auto utf16Again = toUTF16 (std::u32string_view {utf32.data (), utf32.size ()});
	EXPECT_EQ (utf16Again.size (), 2);
	EXPECT_EQ (utf16Again[0], grinningFaceUTF16[0]);
	EXPECT_EQ (utf16Again[1], grinningFaceUTF16[1]);
}

//------------------------------------------------------------------------
TEST_CASE (StringConvertTest, invalidInput)
{
	// Invalid UTF-8: lone continuation byte (80)
	static constexpr const uint8_t invalidCont[] = {0x80};
	EXPECT_EXCEPTION (toUTF16 (std::string_view {reinterpret_cast<const char*> (invalidCont), 1}),
					  "invalid UTF-8 string");

	// Invalid UTF-8: overlong encoding of 'A' (should be 0x41, but encoded as C1 81)
	static constexpr const uint8_t overlongA[] = {0xC1, 0x81};
	EXPECT_EXCEPTION (toUTF16 (std::string_view {reinterpret_cast<const char*> (overlongA), 2}),
					  "invalid UTF-8 string");

	// Invalid UTF-8: incomplete 2-byte sequence (C3 without continuation)
	static constexpr const uint8_t incomplete2[] = {0xC3};
	EXPECT_EXCEPTION (toUTF16 (std::string_view {reinterpret_cast<const char*> (incomplete2), 1}),
					  "invalid UTF-8 string");

	// Invalid UTF-8: bad continuation byte in 3-byte sequence (E2 28 AC)
	static constexpr const uint8_t badCont3[] = {0xE2, 0x28, 0xAC};
	EXPECT_EXCEPTION (toUTF16 (std::string_view {reinterpret_cast<const char*> (badCont3), 3}),
					  "invalid UTF-8 string");

	// Invalid UTF-8: codepoint in surrogate range (ED A0 80)
	static constexpr const uint8_t surrogateRange[] = {0xED, 0xA0, 0x80};
	EXPECT_EXCEPTION (
		toUTF16 (std::string_view {reinterpret_cast<const char*> (surrogateRange), 3}),
		"invalid UTF-8 string");

	// Invalid UTF-16: lone high surrogate
	static constexpr const char16_t loneHigh[] = {0xD83D};
	EXPECT_EXCEPTION (toUTF8 (std::u16string_view {loneHigh, 1}), "invalid UTF-16 string");

	// Invalid UTF-16: high surrogate not followed by low surrogate
	static constexpr const char16_t badPair[] = {0xD83D, u'A'};
	EXPECT_EXCEPTION (toUTF8 (std::u16string_view {badPair, 2}), "invalid UTF-16 string");

	// Invalid UTF-16: lone low surrogate
	static constexpr const char16_t loneLow[] = {0xDC00};
	EXPECT_EXCEPTION (toUTF32 (std::u16string_view {loneLow, 1}), "invalid UTF-16 string");

	// Invalid UTF-32: surrogate code point
	static constexpr const char32_t surrogate32[] = {0xD800};
	EXPECT_EXCEPTION (toUTF16 (std::u32string_view {surrogate32, 1}), "invalid UTF-32 string");

	// Invalid UTF-32: out of range (> 0x10FFFF)
	static constexpr const char32_t outOfRange32[] = {0x110000};
	EXPECT_EXCEPTION (toUTF16 (std::u32string_view {outOfRange32, 1}), "invalid UTF-32 string");

	// Invalid UTF-8 for toUTF32: invalid start byte (FE)
	static constexpr const uint8_t invalidStartFE[] = {0xFE};
	EXPECT_EXCEPTION (
		toUTF32 (std::string_view {reinterpret_cast<const char*> (invalidStartFE), 1}),
		"invalid UTF-8 string");

	// Invalid UTF-8 for toUTF32: invalid start byte (FF)
	static constexpr const uint8_t invalidStartFF[] = {0xFF};
	EXPECT_EXCEPTION (
		toUTF32 (std::string_view {reinterpret_cast<const char*> (invalidStartFF), 1}),
		"invalid UTF-8 string");

	// Invalid UTF-8 for toUTF32: overlong encoding of NUL (C0 80)
	static constexpr const uint8_t overlongNUL[] = {0xC0, 0x80};
	EXPECT_EXCEPTION (toUTF32 (std::string_view {reinterpret_cast<const char*> (overlongNUL), 2}),
					  "invalid UTF-8 string");

	// Invalid UTF-8 for toUTF32: truncated 3-byte sequence (E2 82)
	static constexpr const uint8_t truncated3[] = {0xE2, 0x82};
	EXPECT_EXCEPTION (toUTF32 (std::string_view {reinterpret_cast<const char*> (truncated3), 2}),
					  "invalid UTF-8 string");

	// Invalid UTF-8 for toUTF32: truncated 4-byte sequence (F0 9F 98)
	static constexpr const uint8_t truncated4[] = {0xF0, 0x9F, 0x98};
	EXPECT_EXCEPTION (toUTF32 (std::string_view {reinterpret_cast<const char*> (truncated4), 3}),
					  "invalid UTF-8 string");

	// Invalid UTF-8 for toUTF32: continuation byte as start (80 80)
	static constexpr const uint8_t contAsStart[] = {0x80, 0x80};
	EXPECT_EXCEPTION (toUTF32 (std::string_view {reinterpret_cast<const char*> (contAsStart), 2}),
					  "invalid UTF-8 string");

	// Invalid UTF-8 for toUTF32: bad continuation in 4-byte sequence (F0 28 98 80)
	static constexpr const uint8_t badCont4[] = {0xF0, 0x28, 0x98, 0x80};
	EXPECT_EXCEPTION (toUTF32 (std::string_view {reinterpret_cast<const char*> (badCont4), 4}),
					  "invalid UTF-8 string");

	// Invalid UTF-16 for toUTF8: low surrogate followed by high surrogate (reversed pair)
	static constexpr const char16_t reversedPair[] = {0xDC00, 0xD83D};
	EXPECT_EXCEPTION (toUTF8 (std::u16string_view {reversedPair, 2}), "invalid UTF-16 string");

	// Invalid UTF-16 for toUTF8: high surrogate at end of string
	static constexpr const char16_t highAtEnd[] = {0xD83D, 0};
	EXPECT_EXCEPTION (toUTF8 (std::u16string_view {highAtEnd, 1}), "invalid UTF-16 string");

	// Invalid UTF-16 for toUTF32: low surrogate alone
	static constexpr const char16_t lowAlone[] = {0xDFFF};
	EXPECT_EXCEPTION (toUTF32 (std::u16string_view {lowAlone, 1}), "invalid UTF-16 string");

	// Invalid UTF-16 for toUTF32: high surrogate not followed by low surrogate, followed by BMP
	static constexpr const char16_t highThenBMP[] = {0xD83D, u'A'};
	EXPECT_EXCEPTION (toUTF32 (std::u16string_view {highThenBMP, 2}), "invalid UTF-16 string");

	// Invalid UTF-16 for toUTF32: high surrogate at end
	static constexpr const char16_t highEndOnly[] = {0xD83D};
	EXPECT_EXCEPTION (toUTF32 (std::u16string_view {highEndOnly, 1}), "invalid UTF-16 string");
}

//------------------------------------------------------------------------
} // VSTGUI
