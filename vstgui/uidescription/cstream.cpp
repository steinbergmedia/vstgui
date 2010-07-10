//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "cstream.h"
#include <algorithm>

namespace VSTGUI {

//-----------------------------------------------------------------------------
CMemoryStream::CMemoryStream (int32_t initialSize, int32_t inDelta, ByteOrder byteOrder)
: OutputStream (byteOrder)
, InputStream (byteOrder)
, buffer (0)
, size (0)
, pos (0)
, delta (inDelta)
, ownsBuffer (true)
{
	resize (initialSize);
}

//-----------------------------------------------------------------------------
CMemoryStream::CMemoryStream (const int8_t* inBuffer, int32_t bufferSize, ByteOrder byteOrder)
: OutputStream (byteOrder)
, InputStream (byteOrder)
, buffer (const_cast<int8_t*> (inBuffer))
, size (bufferSize)
, pos (0)
, delta (0)
, ownsBuffer (false)
{
}

//-----------------------------------------------------------------------------
CMemoryStream::~CMemoryStream ()
{
	if (ownsBuffer && buffer)
		free (buffer);
}

//-----------------------------------------------------------------------------
bool CMemoryStream::resize (int32_t inSize)
{
	if (size >= inSize)
		return true;

	if (ownsBuffer == false)
		return false;

	int32_t newSize = size + delta;
	while (newSize < inSize)
		newSize += delta;

	int8_t* newBuffer = (int8_t*)malloc (newSize);
	if (newBuffer && buffer)
		memcpy (newBuffer, buffer, size);
	if (buffer)
		free (buffer);
	buffer = newBuffer;
	size = newSize;
	
	return buffer != 0;
}

//-----------------------------------------------------------------------------
int32_t CMemoryStream::writeRaw (const void* inBuffer, int32_t size)
{
	if (!resize (pos + size))
		return -1;
	
	memcpy (buffer + pos, inBuffer, size);
	pos += size;
	
	return size;
}

//-----------------------------------------------------------------------------
int32_t CMemoryStream::readRaw (void* outBuffer, int32_t outSize)
{
	if ((size - pos) <= 0)
		return 0;

	outSize = std::min<int32_t> (outSize, size - pos);
	memcpy (outBuffer, buffer + pos, outSize);
	pos += outSize;

	return outSize;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const int8_t& input)
{
	return writeRaw (&input, sizeof (int8_t)) == sizeof (int8_t);
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const uint8_t& input)
{
	return writeRaw (&input, sizeof (uint8_t)) == sizeof (uint8_t);
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const int16_t& input)
{
	if (byteOrder == kNativeByteOrder)
	{
		return writeRaw (&input, sizeof (int16_t)) == sizeof (int16_t);
	}
	else
	{
		const uint8_t* p = (const uint8_t*)&input;
		if (writeRaw (&p[1], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[0], sizeof (int8_t)) != sizeof (int8_t)) return false;
		return true;
	}
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const uint16_t& input)
{
	if (byteOrder == kNativeByteOrder)
	{
		return writeRaw (&input, sizeof (uint16_t)) == sizeof (uint16_t);
	}
	else
	{
		const uint8_t* p = (const uint8_t*)&input;
		if (writeRaw (&p[1], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[0], sizeof (int8_t)) != sizeof (int8_t)) return false;
		return true;
	}
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const int32_t& input)
{
	if (byteOrder == kNativeByteOrder)
	{
		return writeRaw (&input, sizeof (int32_t)) == sizeof (int32_t);
	}
	else
	{
		const uint8_t* p = (const uint8_t*)&input;
		if (writeRaw (&p[3], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[2], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[1], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[0], sizeof (int8_t)) != sizeof (int8_t)) return false;
		return true;
	}
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const uint32_t& input)
{
	if (byteOrder == kNativeByteOrder)
	{
		return writeRaw (&input, sizeof (uint32_t)) == sizeof (uint32_t);
	}
	else
	{
		const uint8_t* p = (const uint8_t*)&input;
		if (writeRaw (&p[3], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[2], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[1], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[0], sizeof (int8_t)) != sizeof (int8_t)) return false;
		return true;
	}
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const int64_t& input)
{
	if (byteOrder == kNativeByteOrder)
	{
		return writeRaw (&input, sizeof (int64_t)) == sizeof (int64_t);
	}
	else
	{
		const uint8_t* p = (const uint8_t*)&input;
		if (writeRaw (&p[7], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[6], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[5], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[4], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[3], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[2], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[1], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[0], sizeof (int8_t)) != sizeof (int8_t)) return false;
		return true;
	}
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const uint64_t& input)
{
	if (byteOrder == kNativeByteOrder)
	{
		return writeRaw (&input, sizeof (uint64_t)) == sizeof (uint64_t);
	}
	else
	{
		const uint8_t* p = (const uint8_t*)&input;
		if (writeRaw (&p[7], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[6], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[5], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[4], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[3], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[2], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[1], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[0], sizeof (int8_t)) != sizeof (int8_t)) return false;
		return true;
	}
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const double& input)
{
	if (byteOrder == kNativeByteOrder)
	{
		return writeRaw (&input, sizeof (double)) == sizeof (double);
	}
	else
	{
		const uint8_t* p = (const uint8_t*)&input;
		if (writeRaw (&p[7], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[6], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[5], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[4], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[3], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[2], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[1], sizeof (int8_t)) != sizeof (int8_t)) return false;
		if (writeRaw (&p[0], sizeof (int8_t)) != sizeof (int8_t)) return false;
		return true;
	}
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const std::string& str)
{
	if (!(*this << (int32_t)'str ')) return false;
	if (!(*this << (int32_t)str.length ())) return false;
	return writeRaw (str.c_str (), str.length ()) == str.length ();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool InputStream::operator>> (int8_t& output)
{
	return readRaw (&output, sizeof (int8_t)) == sizeof (int8_t);
}

//-----------------------------------------------------------------------------
bool InputStream::operator>> (uint8_t& output)
{
	return readRaw (&output, sizeof (uint8_t)) == sizeof (uint8_t);
}

//-----------------------------------------------------------------------------
bool InputStream::operator>> (int16_t& output)
{
	if (readRaw (&output, sizeof (int16_t)) == sizeof (int16_t))
	{
		if (byteOrder != kNativeByteOrder)
		{
			uint8_t* p = (uint8_t*)&output;
			int8_t temp = p[0];
			p[0] = p[1];
			p[1] = temp;
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool InputStream::operator>> (uint16_t& output)
{
	if (readRaw (&output, sizeof (uint16_t)) == sizeof (uint16_t))
	{
		if (byteOrder != kNativeByteOrder)
		{
			uint8_t* p = (uint8_t*)&output;
			int8_t temp = p[0];
			p[0] = p[1];
			p[1] = temp;
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool InputStream::operator>> (int32_t& output)
{
	if (readRaw (&output, sizeof (int32_t)) == sizeof (int32_t))
	{
		if (byteOrder != kNativeByteOrder)
		{
			uint8_t* p = (uint8_t*)&output;
			int8_t temp = p[0];
			p[0] = p[3];
			p[3] = temp;
			temp = p[1];
			p[1] = p[2];
			p[2] = temp;
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool InputStream::operator>> (uint32_t& output)
{
	if (readRaw (&output, sizeof (uint32_t)) == sizeof (uint32_t))
	{
		if (byteOrder != kNativeByteOrder)
		{
			uint8_t* p = (uint8_t*)&output;
			int8_t temp = p[0];
			p[0] = p[3];
			p[3] = temp;
			temp = p[1];
			p[1] = p[2];
			p[2] = temp;
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool InputStream::operator>> (int64_t& output)
{
	if (readRaw (&output, sizeof (int64_t)) == sizeof (int64_t))
	{
		if (byteOrder != kNativeByteOrder)
		{
			uint8_t* p = (uint8_t*)&output;
			int8_t temp = p[0];
			p[0] = p[7];
			p[7] = temp;
			temp = p[6];
			p[1] = p[6];
			p[6] = temp;
			temp = p[5];
			p[2] = p[5];
			p[2] = temp;
			temp = p[3];
			p[3] = p[4];
			p[4] = temp;
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool InputStream::operator>> (uint64_t& output)
{
	if (readRaw (&output, sizeof (uint64_t)) == sizeof (uint64_t))
	{
		if (byteOrder != kNativeByteOrder)
		{
			uint8_t* p = (uint8_t*)&output;
			int8_t temp = p[0];
			p[0] = p[7];
			p[7] = temp;
			temp = p[6];
			p[1] = p[6];
			p[6] = temp;
			temp = p[5];
			p[2] = p[5];
			p[2] = temp;
			temp = p[3];
			p[3] = p[4];
			p[4] = temp;
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool InputStream::operator>> (double& output)
{
	if (readRaw (&output, sizeof (double)) == sizeof (double))
	{
		if (byteOrder != kNativeByteOrder)
		{
			uint8_t* p = (uint8_t*)&output;
			int8_t temp = p[0];
			p[0] = p[7];
			p[7] = temp;
			temp = p[6];
			p[1] = p[6];
			p[6] = temp;
			temp = p[5];
			p[2] = p[5];
			p[2] = temp;
			temp = p[3];
			p[3] = p[4];
			p[4] = temp;
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool InputStream::operator>> (std::string& string)
{
	int32_t identifier;
	if (!(*this >> identifier)) return false;
	if (identifier == 'str ')
	{
		int32_t length;
		if (!(*this >> length)) return false;
		int8_t* buffer = (int8_t*)malloc (length);
		int32_t read = readRaw (buffer, length);
		if (read == length)
			string.assign ((const char*)buffer, length);
		free (buffer);
		return read == length;
	}
	return false;
}

} // namespace
