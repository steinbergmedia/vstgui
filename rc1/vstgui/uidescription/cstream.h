//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins :
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __cstream__
#define __cstream__

#include "../lib/vstguifwd.h"
#include <algorithm>
#include <string>
#include <limits>

namespace VSTGUI {

static const uint32_t kStreamIOError = std::numeric_limits<uint32_t>::max ();
static const int64_t kStreamSeekError = -1;

/**
	ByteOrder aware output stream interface
 */
class OutputStream
{
public:
	OutputStream (ByteOrder byteOrder = kNativeByteOrder) : byteOrder (byteOrder) {}
	virtual ~OutputStream () {}

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

	virtual bool operator<< (const std::string& str) = 0;

	virtual uint32_t writeRaw (const void* buffer, uint32_t size) = 0;
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
	virtual ~InputStream () {}

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

	virtual bool operator>> (std::string& string) = 0;

	virtual uint32_t readRaw (void* buffer, uint32_t size) = 0;
private:
	ByteOrder byteOrder;
};

/**
	Seekable stream interface
 */
class SeekableStream
{
public:
	virtual ~SeekableStream () {}
	enum SeekMode {
		kSeekSet,
		kSeekCurrent,
		kSeekEnd
	};

	virtual int64_t seek (int64_t pos, SeekMode mode) = 0;	///< returns -1 if seek fails otherwise new position
	virtual int64_t tell () const = 0;
	virtual void rewind () = 0;
};

/**
	Memory input and output stream
 */
class CMemoryStream : virtual public OutputStream, virtual public InputStream, public SeekableStream, public CBaseObject
{
public:
	CMemoryStream (uint32_t initialSize = 1024, uint32_t delta = 1024, bool binaryMode = true, ByteOrder byteOrder = kNativeByteOrder);
	CMemoryStream (const int8_t* buffer, uint32_t bufferSize, bool binaryMode = true, ByteOrder byteOrder = kNativeByteOrder);
	~CMemoryStream ();

	uint32_t writeRaw (const void* buffer, uint32_t size) VSTGUI_OVERRIDE_VMETHOD;
	uint32_t readRaw (void* buffer, uint32_t size) VSTGUI_OVERRIDE_VMETHOD;

	int64_t seek (int64_t pos, SeekMode mode) VSTGUI_OVERRIDE_VMETHOD;
	int64_t tell () const VSTGUI_OVERRIDE_VMETHOD { return static_cast<int64_t> (pos); }
	void rewind () VSTGUI_OVERRIDE_VMETHOD { pos = 0; }

	const int8_t* getBuffer () const { return buffer; }

	virtual bool operator<< (const std::string& str) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool operator>> (std::string& string) VSTGUI_OVERRIDE_VMETHOD;

	bool end (); // write a zero byte if binaryMode is false
protected:
	bool resize (uint32_t newSize);

	bool binaryMode;
	bool ownsBuffer;
	int8_t* buffer;
	uint32_t bufferSize;
	uint32_t size;
	uint32_t pos;
	uint32_t delta;
};

/**
	File input and output stream
 */
class CFileStream : public OutputStream, public InputStream, public SeekableStream, public CBaseObject
{
public:
	CFileStream ();
	~CFileStream ();

	enum {
		kReadMode		= 1 << 0,
		kWriteMode		= 1 << 1,
		kTruncateMode	= 1 << 2,
		kBinaryMode		= 1 << 3
	};

	bool open (UTF8StringPtr path, int32_t mode, ByteOrder byteOrder = kNativeByteOrder);

	uint32_t writeRaw (const void* buffer, uint32_t size) VSTGUI_OVERRIDE_VMETHOD;
	uint32_t readRaw (void* buffer, uint32_t size) VSTGUI_OVERRIDE_VMETHOD;

	int64_t seek (int64_t pos, SeekMode mode) VSTGUI_OVERRIDE_VMETHOD;
	int64_t tell () const VSTGUI_OVERRIDE_VMETHOD;
	void rewind () VSTGUI_OVERRIDE_VMETHOD;

	virtual bool operator<< (const std::string& str) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool operator>> (std::string& string) VSTGUI_OVERRIDE_VMETHOD;
protected:
	FILE* stream;
	int32_t openMode;
};

static const int8_t unixPathSeparator = '/';
static const int8_t windowsPathSeparator = '\\';
/**
	Helper function to transform all Windows path separators to unix ones
 */
inline void unixfyPath (std::string& path)
{
	std::replace (path.begin (), path.end (), windowsPathSeparator, unixPathSeparator);
}

//------------------------------------------------------------------------
inline bool removeLastPathComponent (std::string& path)
{
	size_t sepPos = path.find_last_of (unixPathSeparator);
	if (sepPos != std::string::npos)
	{
		path.erase (sepPos);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
inline bool pathIsAbsolute (const std::string& path)
{
#if MAC
	return !path.empty () && path[0] == unixPathSeparator;
#elif WINDOWS
	return path.length () >= 2 && path[1] == ':';
#else
	return false;
#endif
}

/**
	Resource input stream
 */
class CResourceInputStream : public InputStream, public SeekableStream
{
public:
	CResourceInputStream (ByteOrder byteOrder = kNativeByteOrder);
	~CResourceInputStream ();

	bool open (const CResourceDescription& res);

	virtual bool operator>> (std::string& string) VSTGUI_OVERRIDE_VMETHOD { return false; }
	virtual uint32_t readRaw (void* buffer, uint32_t size) VSTGUI_OVERRIDE_VMETHOD;
	virtual int64_t seek (int64_t pos, SeekMode mode) VSTGUI_OVERRIDE_VMETHOD;
	virtual int64_t tell () const VSTGUI_OVERRIDE_VMETHOD;
	virtual void rewind () VSTGUI_OVERRIDE_VMETHOD;
protected:
	void* platformHandle;
};


} // namespace

#endif
