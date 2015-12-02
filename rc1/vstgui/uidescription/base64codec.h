//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2012, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __base64codec__
#define __base64codec__

#include "../lib/vstguibase.h"
#include <string>
#include <cstdlib>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class Base64Codec
{
public:
	Base64Codec () : data (0), dataSize (0) {}

	~Base64Codec ()
	{
		if (data)
			std::free (data);
	}

	bool init (const std::string& base64String)
	{
		if (data)
			return false;
		uint32_t strLen = (uint32_t)base64String.length ();
		uint8_t* strPtr = (uint8_t*)base64String.c_str ();
		data = (int8_t*)std::malloc ((strLen * 3 / 4) + 3);
		uint8_t input[4];
		uint32_t i;
		for (i = 0; i <= strLen - 4; i += 4)
		{
			input[0] = *strPtr++;
			input[1] = *strPtr++;
			input[2] = *strPtr++;
			input[3] = *strPtr++;
			dataSize += decodeblock (input, (uint8_t*)(data + dataSize));
		}
		if (i < strLen)
		{
			uint32_t j;
			input[0] = input[1] = input[2] = input[3] = '=';
			for (j = 0; i < strLen; i++, j++)
			{
				input[j] = *strPtr++;
			}
			dataSize += decodeblock (input, (uint8_t*)(data + dataSize));
		}
		return true;
	}

	bool init (const void* binaryData, uint32_t binaryDataSize)
	{
		if (data)
			return false;
		data = (int8_t*)std::malloc ((binaryDataSize * 4) / 3 + 4);
		uint8_t* ptr = (uint8_t*)binaryData;
		uint8_t input[3];
		uint32_t i;
		for (i = 0; i < (binaryDataSize - 3); i += 3)
		{
			input[0] = *ptr++;
			input[1] = *ptr++;
			input[2] = *ptr++;
			encodeblock (input, (uint8_t*)(data + dataSize), 3);
			dataSize += 4;
		}
		if (i < binaryDataSize)
		{
			input[0] = input[1] = input[2] = 0;
			uint32_t j;
			for (j = 0; i < binaryDataSize; i++, j++)
			{
				input[j] = *ptr++;
			}
			encodeblock (input, (uint8_t*)(data + dataSize), j);
			dataSize += 4;
		}
		return true;
	}

	const int8_t* getData () const { return data; }
	uint32_t getDataSize () const { return dataSize; }

protected:
	inline uint32_t decodeblock (uint8_t input[4], uint8_t output[3])
	{
		static const uint8_t cd64[] = {
			62,   0xFF, 0xFF, 0xFF, 63,   52,   53, 54, 55, 56, 57, 58, 59, 60, 61, 0xFF,
			0xFF, 0xFF, 0,	0xFF, 0xFF, 0xFF, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
			10,   11,   12,   13,   14,   15,   16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
			36,   37,   38,   39,   40,   41,   42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

		uint32_t result = 3;
		if (input[2] == '=')
			result = 1;
		else if (input[3] == '=')
			result = 2;
		input[0] = cd64[input[0] - 43];
		input[1] = cd64[input[1] - 43];
		input[2] = cd64[input[2] - 43];
		input[3] = cd64[input[3] - 43];
		output[0] = static_cast<uint8_t> ((input[0] << 2) | ((input[1] & 0x30) >> 4));
		output[1] = static_cast<uint8_t> (((input[1] & 0xF) << 4) | ((input[2] & 0x3C) >> 2));
		output[2] = static_cast<uint8_t> (((input[2] & 0x03) << 6) | input[3]);
		return result;
	}

	inline void encodeblock (uint8_t input[3], uint8_t output[4], uint32_t len)
	{
		static const uint8_t cb64[] =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		output[0] = cb64[input[0] >> 2];
		output[1] = cb64[((input[0] & 0x03) << 4) | ((input[1] & 0xf0) >> 4)];
		output[2] =
			(uint8_t)(len > 1 ? cb64[((input[1] & 0x0f) << 2) | ((input[2] & 0xc0) >> 6)] : '=');
		output[3] = (uint8_t)(len > 2 ? cb64[input[2] & 0x3f] : '=');
	}

	int8_t* data;
	uint32_t dataSize;
};

} // namespace VSTGUI

#endif // __base64codec__
