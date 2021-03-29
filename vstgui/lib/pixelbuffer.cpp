// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "pixelbuffer.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace PixelBuffer {
namespace Private {

//------------------------------------------------------------------------
template<int8_t bs1, int8_t bs2, int8_t bs3, int8_t bs4>
inline uint32_t shuffle (uint32_t input)
{
	static constexpr auto s1 = bs1 * 8;
	static constexpr auto s2 = bs2 * 8;
	static constexpr auto s3 = bs3 * 8;
	static constexpr auto s4 = bs4 * 8;

	auto b1 = (input & 0xFF000000);
	auto b2 = (input & 0x00FF0000);
	auto b3 = (input & 0x0000FF00);
	auto b4 = (input & 0x000000FF);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4293)
#endif

	b1 = (s1 >= 0) ? b1 << s1 : b1 >> -s1;
	b2 = (s2 >= 0) ? b2 << s2 : b2 >> -s2;
	b3 = (s3 >= 0) ? b3 << s3 : b3 >> -s3;
	b4 = (s4 >= 0) ? b4 << s4 : b4 >> -s4;

#ifdef _MSC_VER
#pragma warning(pop)
#endif

	return b1 | b2 | b3 | b4;
}

//------------------------------------------------------------------------
template<Format SourceFormat, Format DestinationFormat>
inline void convert (uint8_t* buffer, uint32_t bytesPerRow, uint32_t width, uint32_t height)
{
	for (auto y = 0u; y < height; ++y, buffer += bytesPerRow)
	{
		auto intPtr = reinterpret_cast<uint32_t*> (buffer);
		for (auto x = 0u; x < width; ++x, ++intPtr)
		{
			auto pixel = *intPtr;
			switch (SourceFormat)
			{
				case Format::ARGB:
				{
					switch (DestinationFormat)
					{
						case Format::ARGB:
						{
							// nothing to do
							break;
						}
						case Format::ABGR:
						{
							*intPtr = shuffle<0, -2, 0, 2> (pixel);
							break;
						}
						case Format::BGRA:
						{
							*intPtr = shuffle<-3, -1, 1, 3> (pixel);
							break;
						}
						case Format::RGBA:
						{
							*intPtr = shuffle<-3, 1, 1, 1> (pixel);
							break;
						}
					}
					break;
				}
				case Format::ABGR:
				{
					switch (DestinationFormat)
					{
						case Format::ARGB:
						{
							*intPtr = shuffle<0, -2, 0, 2> (pixel);
							break;
						}
						case Format::ABGR:
						{
							// nothing to do
							break;
						}
						case Format::BGRA:
						{
							*intPtr = shuffle<-3, 1, 1, 1> (pixel);
							break;
						}
						case Format::RGBA:
						{
							*intPtr = shuffle<-3, -1, 1, 3> (pixel);
							break;
						}
					}
					break;
				}
				case Format::RGBA:
				{
					switch (DestinationFormat)
					{
						case Format::ARGB:
						{
							*intPtr = shuffle<-3, 1, 1, 1> (pixel);
							break;
						}
						case Format::ABGR:
						{
							*intPtr = shuffle<-3, -1, 1, 3> (pixel);
							break;
						}
						case Format::BGRA:
						{
							*intPtr = shuffle<-2, 0, 2, 0> (pixel);
							break;
						}
						case Format::RGBA:
						{
							// nothing to do
							break;
						}
					}
					break;
				}
				case Format::BGRA:
				{
					switch (DestinationFormat)
					{
						case Format::ARGB:
						{
							*intPtr = shuffle<-3, -1, 1, 3> (pixel);
							break;
						}
						case Format::ABGR:
						{
							*intPtr = shuffle<-3, 1, 1, 1> (pixel);
							break;
						}
						case Format::BGRA:
						{
							// nothing to do
							break;
						}
						case Format::RGBA:
						{
							*intPtr = shuffle<-2, 0, 2, 0> (pixel);
							break;
						}
					}
					break;
				}
			}
		}
	}
}

//------------------------------------------------------------------------
} // Private

//------------------------------------------------------------------------
void convert (Format srcFormat, Format dstFormat, uint8_t* buffer, uint32_t bytesPerRow,
			  uint32_t width, uint32_t height)
{
	using namespace Private;
	switch (srcFormat)
	{
		case Format::ARGB:
		{
			switch (dstFormat)
			{
				case Format::ARGB:
					return;
				case Format::ABGR:
				{
					convert<Format::ARGB, Format::ABGR> (buffer, bytesPerRow, width, height);
					break;
				}
				case Format::RGBA:
				{
					convert<Format::ARGB, Format::RGBA> (buffer, bytesPerRow, width, height);
					break;
				}
				case Format::BGRA:
				{
					convert<Format::ARGB, Format::BGRA> (buffer, bytesPerRow, width, height);
					break;
				}
			}
			break;
		}
		case Format::ABGR:
		{
			switch (dstFormat)
			{
				case Format::ARGB:
				{
					convert<Format::ABGR, Format::ARGB> (buffer, bytesPerRow, width, height);
					break;
				}
				case Format::ABGR:
					return;
				case Format::RGBA:
				{
					convert<Format::ABGR, Format::RGBA> (buffer, bytesPerRow, width, height);
					break;
				}
				case Format::BGRA:
				{
					convert<Format::ABGR, Format::BGRA> (buffer, bytesPerRow, width, height);
					break;
				}
			}
			break;
		}
		case Format::RGBA:
		{
			switch (dstFormat)
			{
				case Format::ARGB:
				{
					convert<Format::RGBA, Format::ARGB> (buffer, bytesPerRow, width, height);
					break;
				}
				case Format::ABGR:
				{
					convert<Format::RGBA, Format::ABGR> (buffer, bytesPerRow, width, height);
					break;
				}
				case Format::RGBA:
					return;
				case Format::BGRA:
				{
					convert<Format::RGBA, Format::BGRA> (buffer, bytesPerRow, width, height);
					break;
				}
			}
			break;
		}
		case Format::BGRA:
		{
			switch (dstFormat)
			{
				case Format::ARGB:
				{
					convert<Format::BGRA, Format::ARGB> (buffer, bytesPerRow, width, height);
					break;
				}
				case Format::ABGR:
				{
					convert<Format::BGRA, Format::ABGR> (buffer, bytesPerRow, width, height);
					break;
				}
				case Format::RGBA:
				{
					convert<Format::BGRA, Format::RGBA> (buffer, bytesPerRow, width, height);
					break;
				}
				case Format::BGRA:
					return;
			}
		}
	}
}

//------------------------------------------------------------------------
} // PixelBuffer
} // VSTGUI
