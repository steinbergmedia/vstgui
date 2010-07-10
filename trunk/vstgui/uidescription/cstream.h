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

#ifndef __cstream__
#define __cstream__

#include "../lib/vstguibase.h"
#include <string>

namespace VSTGUI {

enum ByteOrder {
	kBigEndianByteOrder = 0,
	kLittleEndianByteOrder,
	#if WINDOWS || defined (__LITTLE_ENDIAN__)
	kNativeByteOrder = kLittleEndianByteOrder
	#else
	kNativeByteOrder = kBigEndianByteOrder
	#endif
};

/**
	ByteOrder aware output stream interface
 */
class OutputStream
{
public:
	OutputStream (ByteOrder byteOrder = kNativeByteOrder) : byteOrder (byteOrder) {}

	ByteOrder getByteOrder () const { return byteOrder; }
	void setByteOrder (ByteOrder newByteOrder) { byteOrder = newByteOrder; }
	
	bool operator<< (const int8_t& input);
	bool operator<< (const uint8_t& input);
	bool operator<< (const int16_t& input);
	bool operator<< (const uint16_t& input);
	bool operator<< (const int32_t& input);
	bool operator<< (const uint32_t& input);
	bool operator<< (const int64_t& input);
	bool operator<< (const uint64_t& input);
	bool operator<< (const double& input);

	bool operator<< (const std::string& str);

	virtual int32_t writeRaw (const void* buffer, int32_t size) = 0;
private:
	ByteOrder byteOrder;
};

/**
	ByteOrder aware input stream interface
 */
class InputStream
{
public:
	InputStream (ByteOrder byteOrder = kNativeByteOrder) : byteOrder (byteOrder) {}
	
	ByteOrder getByteOrder () const { return byteOrder; }
	void setByteOrder (ByteOrder newByteOrder) { byteOrder = newByteOrder; }
	
	bool operator>> (int8_t& output);
	bool operator>> (uint8_t& output);
	bool operator>> (int16_t& output);
	bool operator>> (uint16_t& output);
	bool operator>> (int32_t& output);
	bool operator>> (uint32_t& output);
	bool operator>> (int64_t& output);
	bool operator>> (uint64_t& output);
	bool operator>> (double& output);

	bool operator>> (std::string& string);

	virtual int32_t readRaw (void* buffer, int32_t size) = 0;
private:
	ByteOrder byteOrder;
};

/**
	Memory input and output stream
 */
class CMemoryStream : public OutputStream, public InputStream, public CBaseObject
{
public:
	CMemoryStream (int32_t initialSize = 1024, int32_t delta = 1024, ByteOrder byteOrder = kNativeByteOrder);
	CMemoryStream (const int8_t* buffer, int32_t bufferSize, ByteOrder byteOrder = kNativeByteOrder);
	~CMemoryStream ();
	
	int32_t writeRaw (const void* buffer, int32_t size);
	int32_t readRaw (void* buffer, int32_t size);

	int32_t tell () const { return pos; }
	void rewind () { pos = 0; }

	const int8_t* getBuffer () const { return buffer; }

protected:
	bool resize (int32_t newSize);

	bool ownsBuffer;
	int8_t* buffer;
	int32_t size;
	int32_t pos;
	int32_t delta;
};

} // namespace

#endif
