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
	
	bool operator<< (const int& input);
	bool operator<< (const unsigned int& input);
	bool operator<< (const char& input);
	bool operator<< (const unsigned char& input);
	bool operator<< (const short& input);
	bool operator<< (const unsigned short& input);
	bool operator<< (const double& input);

	bool operator<< (const std::string& str);

	virtual int writeRaw (const void* buffer, int size) = 0;
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
	
	bool operator>> (int& output);
	bool operator>> (unsigned int& output);
	bool operator>> (char& output);
	bool operator>> (unsigned char& output);
	bool operator>> (short& output);
	bool operator>> (unsigned short& output);
	bool operator>> (double& output);

	bool operator>> (std::string& string);

	virtual int readRaw (void* buffer, int size) = 0;
private:
	ByteOrder byteOrder;
};

/**
	Memory input and output stream
 */
class CMemoryStream : public OutputStream, public InputStream, public CBaseObject
{
public:
	CMemoryStream (int initialSize = 1024, int delta = 1024, ByteOrder byteOrder = kNativeByteOrder);
	CMemoryStream (const char* buffer, int bufferSize, ByteOrder byteOrder = kNativeByteOrder);
	~CMemoryStream ();
	
	int writeRaw (const void* buffer, int size);
	int readRaw (void* buffer, int size);

	int tell () const { return pos; }
	void rewind () { pos = 0; }

	const char* getBuffer () const { return buffer; }

protected:
	bool resize (int newSize);

	bool ownsBuffer;
	char* buffer;
	int size;
	int pos;
	int delta;
};

} // namespace

#endif
