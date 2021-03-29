// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <cstdint>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace PixelBuffer {

//------------------------------------------------------------------------
enum class Format
{
	ARGB,
	RGBA,
	ABGR,
	BGRA
};

//------------------------------------------------------------------------
/** Convert a buffer of 32 bit pixels from one format to another
 *
 *	@param srcFormat Source Pixel Format
 *	@param dstFormat Destination Pixel Format
 *	@param buffer Pixel Buffer
 *	@param bytesPerRow Number of bytes per row in buffer
 *	@param width Number of pixels per row
 *	@param height Number of rows
 */
void convert (Format srcFormat, Format dstFormat, uint8_t* buffer, uint32_t bytesPerRow,
			  uint32_t width, uint32_t height);

//------------------------------------------------------------------------
} // PixelBuffer
} // VSTGUI
