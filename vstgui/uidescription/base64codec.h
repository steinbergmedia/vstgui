// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../lib/malloc.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class Base64Codec
{
public:
	struct Result
	{
		Buffer<uint8_t> data;
		uint32_t dataSize {0};
	};

	template<typename T>
	static inline Result decode (const T& base64String)
	{
		return decode (base64String.data (), base64String.size ());
	}

	template <typename T>
	static inline Result decode (const T* inBuffer, size_t inBufferSize)
	{
		static_assert (sizeof (T) == 1, "T must be one byte type");
		Result r;
		r.data.allocate ((inBufferSize * 3 / 4) + 3);
		uint8_t input1[4];
		uint8_t input2[4];
		auto input1Ptr = reinterpret_cast<uint32_t*>(&input1[0]);
		auto input2Ptr = reinterpret_cast<uint32_t*>(&input2[0]);
		auto buffer32Ptr = reinterpret_cast<const uint32_t*> (inBuffer);
		while (inBufferSize > 8)
		{
			*input1Ptr = *buffer32Ptr++;
			*input2Ptr = *buffer32Ptr++;
			r.dataSize += decodeblock<false> (input1, r.data.get () + r.dataSize);
			r.dataSize += decodeblock<false> (input2, r.data.get () + r.dataSize);
			inBufferSize -= 8;
		}
		while (inBufferSize > 4)
		{
			*input1Ptr = *buffer32Ptr++;
			r.dataSize += decodeblock<false> (input1, r.data.get () + r.dataSize);
			inBufferSize -= 4;
		}
		if (inBufferSize > 0)
		{
			input1[0] = input1[1] = input1[2] = input1[3] = '=';
			auto ptr = reinterpret_cast<const uint8_t*>(buffer32Ptr);
			for (uint32_t j = 0; j < inBufferSize; j++)
			{
				input1[j] = *ptr++;
			}
			r.dataSize += decodeblock<true> (input1, r.data.get () + r.dataSize);
		}
		return r;
	}

	static inline Result encode (const void* binaryData, size_t binaryDataSize)
	{
		Result r;
		r.data.allocate ((binaryDataSize * 4) / 3 + 4);
		auto ptr = reinterpret_cast<const uint8_t*> (binaryData);
		uint8_t input[3];
		uint32_t i;
		for (i = 0; i < (binaryDataSize - 3); i += 3)
		{
			input[0] = *ptr++;
			input[1] = *ptr++;
			input[2] = *ptr++;
			encodeblock (input, r.data.get () + r.dataSize, 3);
			r.dataSize += 4;
		}
		if (i < binaryDataSize)
		{
			input[0] = input[1] = input[2] = 0;
			uint32_t j;
			for (j = 0; i < binaryDataSize; i++, j++)
			{
				input[j] = *ptr++;
			}
			encodeblock (input, r.data.get () + r.dataSize, j);
			r.dataSize += 4;
		}
		return r;
	}

private:
	template<bool finalBlock = true>
	static inline uint32_t decodeblock (uint8_t input[4], uint8_t output[3])
	{
		static constexpr uint8_t cd64[] = {
			62,   0xFF, 0xFF, 0xFF, 63,   52,   53, 54, 55, 56, 57, 58, 59, 60, 61, 0xFF,
			0xFF, 0xFF, 0,	0xFF, 0xFF, 0xFF, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
			10,   11,   12,   13,   14,   15,   16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
			36,   37,   38,   39,   40,   41,   42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

		uint32_t result = 3;
		if (finalBlock)
		{
			if (input[2] == '=')
				result = 1;
			else if (input[3] == '=')
				result = 2;
		}
		input[0] = cd64[input[0] - 43];
		input[1] = cd64[input[1] - 43];
		input[2] = cd64[input[2] - 43];
		input[3] = cd64[input[3] - 43];
		output[0] = static_cast<uint8_t> ((input[0] << 2) | ((input[1] & 0x30) >> 4));
		output[1] = static_cast<uint8_t> (((input[1] & 0xF) << 4) | ((input[2] & 0x3C) >> 2));
		output[2] = static_cast<uint8_t> (((input[2] & 0x03) << 6) | input[3]);
		return result;
	}

	static inline void encodeblock (uint8_t input[3], uint8_t output[4], uint32_t len)
	{
		static constexpr uint8_t cb64[] =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		output[0] = cb64[input[0] >> 2];
		output[1] = cb64[((input[0] & 0x03) << 4) | ((input[1] & 0xf0) >> 4)];
		output[2] = static_cast<uint8_t>
			(len > 1 ? cb64[((input[1] & 0x0f) << 2) | ((input[2] & 0xc0) >> 6)] : '=');
		output[3] = static_cast<uint8_t>(len > 2 ? cb64[input[2] & 0x3f] : '=');
	}
};

} // VSTGUI
