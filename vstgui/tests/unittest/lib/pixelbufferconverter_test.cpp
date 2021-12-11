// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/pixelbuffer.h"
#include "../unittests.h"

namespace VSTGUI {
using namespace PixelBuffer;

TEST_CASE (PixelBufferTest, ARGB_2_RGBA)
{
	uint32_t pixel = 0x11223344;
	convert (Format::ARGB, Format::RGBA, reinterpret_cast<uint8_t*> (&pixel), 4, 1, 1);
	EXPECT (pixel == 0x22334411);
}

TEST_CASE (PixelBufferTest, ARGB_2_BGRA)
{
	uint32_t pixel = 0x11223344;
	convert (Format::ARGB, Format::BGRA, reinterpret_cast<uint8_t*> (&pixel), 4, 1, 1);
	EXPECT (pixel == 0x44332211);
}

TEST_CASE (PixelBufferTest, ARGB_2_ABGR)
{
	uint32_t pixel = 0x11223344;
	convert (Format::ARGB, Format::ABGR, reinterpret_cast<uint8_t*> (&pixel), 4, 1, 1);
	EXPECT (pixel == 0x11443322);
}

TEST_CASE (PixelBufferTest, ABGR_2_ARGB)
{
	uint32_t pixel = 0x11223344;
	convert (Format::ABGR, Format::ARGB, reinterpret_cast<uint8_t*> (&pixel), 4, 1, 1);
	EXPECT (pixel == 0x11443322);
}

TEST_CASE (PixelBufferTest, ABGR_2_RGBA)
{
	uint32_t pixel = 0x11223344;
	convert (Format::ABGR, Format::RGBA, reinterpret_cast<uint8_t*> (&pixel), 4, 1, 1);
	EXPECT (pixel == 0x44332211);
}

TEST_CASE (PixelBufferTest, ABGR_2_BGRA)
{
	uint32_t pixel = 0x11223344;
	convert (Format::ABGR, Format::BGRA, reinterpret_cast<uint8_t*> (&pixel), 4, 1, 1);
	EXPECT (pixel == 0x22334411);
}

TEST_CASE (PixelBufferTest, RGBA_2_ARGB)
{
	uint32_t pixel = 0x11223344;
	convert (Format::RGBA, Format::ARGB, reinterpret_cast<uint8_t*> (&pixel), 4, 1, 1);
	EXPECT (pixel == 0x22334411);
}

TEST_CASE (PixelBufferTest, RGBA_2_ABGR)
{
	uint32_t pixel = 0x11223344;
	convert (Format::RGBA, Format::ABGR, reinterpret_cast<uint8_t*> (&pixel), 4, 1, 1);
	EXPECT (pixel == 0x44332211);
}

TEST_CASE (PixelBufferTest, RGBA_2_BGRA)
{
	uint32_t pixel = 0x11223344;
	convert (Format::RGBA, Format::BGRA, reinterpret_cast<uint8_t*> (&pixel), 4, 1, 1);
	EXPECT (pixel == 0x11443322);
}

TEST_CASE (PixelBufferTest, BGRA_2_RGBA)
{
	uint32_t pixel = 0x11223344;
	convert (Format::BGRA, Format::RGBA, reinterpret_cast<uint8_t*> (&pixel), 4, 1, 1);
	EXPECT (pixel == 0x33221144);
}

TEST_CASE (PixelBufferTest, BGRA_2_ABGR)
{
	uint32_t pixel = 0x11223344;
	convert (Format::BGRA, Format::ABGR, reinterpret_cast<uint8_t*> (&pixel), 4, 1, 1);
	EXPECT (pixel == 0x22334411);
}

TEST_CASE (PixelBufferTest, BGRA_2_ARGB)
{
	uint32_t pixel = 0x11223344;
	convert (Format::BGRA, Format::ARGB, reinterpret_cast<uint8_t*> (&pixel), 4, 1, 1);
	EXPECT (pixel == 0x44332211);
}

} // namespace VSTGUI
