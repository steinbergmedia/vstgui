// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../lib/vstguifwd.h"
#include "../lib/optional.h"
#include <algorithm>
#include <string>
#include <limits>
#include <memory>
#include <vector>
#include <iostream>

namespace VSTGUI {

/**
	ByteOrder aware output stream interface
 */
class OutputStream
{
public:
	explicit OutputStream (ByteOrder byteOrder = kNativeByteOrder) : byteOrder (byteOrder) {}
	virtual ~OutputStream () noexcept = default;

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
	explicit InputStream (ByteOrder byteOrder = kNativeByteOrder) : byteOrder (byteOrder) {}
	virtual ~InputStream () noexcept = default;

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
	virtual ~SeekableStream () noexcept = default;
	enum SeekMode {
		kSeekSet,
		kSeekCurrent,
		kSeekEnd
	};

	/** returns -1 if seek fails otherwise new position */
	virtual int64_t seek (int64_t pos, SeekMode mode) = 0;
	virtual int64_t tell () const = 0;
	virtual void rewind () = 0;
};

/**
	Memory input and output stream
 */
class CMemoryStream : virtual public OutputStream, virtual public InputStream, public SeekableStream, public AtomicReferenceCounted
{
public:
	CMemoryStream (uint32_t initialSize = 1024, uint32_t delta = 1024, bool binaryMode = true, ByteOrder byteOrder = kNativeByteOrder);
	CMemoryStream (const int8_t* buffer, uint32_t bufferSize, bool binaryMode = true, ByteOrder byteOrder = kNativeByteOrder);
	~CMemoryStream () noexcept override;

	uint32_t writeRaw (const void* buffer, uint32_t size) override;
	uint32_t readRaw (void* buffer, uint32_t size) override;

	int64_t seek (int64_t pos, SeekMode mode) override;
	int64_t tell () const override { return static_cast<int64_t> (pos); }
	void rewind () override { pos = 0; }

	const int8_t* getBuffer () const { return buffer; }

	bool operator<< (const std::string& str) override;
	bool operator>> (std::string& string) override;

	using OutputStream::operator<<;
	using InputStream::operator>>;

	bool end (); // write a zero byte if binaryMode is false
protected:
	bool resize (uint32_t newSize);

	int8_t* buffer;
	uint32_t bufferSize;
	uint32_t size;
	uint32_t pos;
	uint32_t delta;
	bool binaryMode;
	bool ownsBuffer;
};

/**
	File input and output stream
 */
class CFileStream : public OutputStream, public InputStream, public SeekableStream, public AtomicReferenceCounted
{
public:
	CFileStream ();
	~CFileStream () noexcept override;

	enum {
		kReadMode		= 1 << 0,
		kWriteMode		= 1 << 1,
		kTruncateMode	= 1 << 2,
		kBinaryMode		= 1 << 3
	};

	bool open (UTF8StringPtr path, int32_t mode, ByteOrder byteOrder = kNativeByteOrder);
	bool isEndOfFile () const;

	uint32_t writeRaw (const void* buffer, uint32_t size) override;
	uint32_t readRaw (void* buffer, uint32_t size) override;

	int64_t seek (int64_t pos, SeekMode mode) override;
	int64_t tell () const override;
	void rewind () override;

	bool operator<< (const std::string& str) override;
	bool operator>> (std::string& string) override;

	using OutputStream::operator<<;
	using InputStream::operator>>;
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
inline Optional<std::string> lastPathComponent (const std::string path)
{
	size_t sepPos = path.find_last_of (unixPathSeparator);
	if (sepPos == std::string::npos)
		return Optional<std::string> (path);
	return {path.substr (sepPos + 1)};
}

//------------------------------------------------------------------------
inline bool pathIsAbsolute (const std::string& path)
{
#if MAC || LINUX
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
	explicit CResourceInputStream (ByteOrder byteOrder = kNativeByteOrder);
	~CResourceInputStream () noexcept override;

	bool open (const CResourceDescription& res);

	bool operator>> (std::string& string) override { return false; }
	uint32_t readRaw (void* buffer, uint32_t size) override;
	int64_t seek (int64_t pos, SeekMode mode) override;
	int64_t tell () const override;
	void rewind () override;

	using InputStream::operator>>;
protected:
	PlatformResourceInputStreamPtr platformStream;
};

//------------------------------------------------------------------------
class BufferedOutputStream : public OutputStream
{
public:
	BufferedOutputStream (OutputStream& stream, size_t bufferSize = 8192)
	: stream (stream), bufferSize (bufferSize)
	{
		buffer.reserve (bufferSize);
	}
	~BufferedOutputStream () noexcept override { flush (); }
	bool operator<< (const std::string& str) override
	{
		return writeRaw (str.c_str (), static_cast<uint32_t> (str.size ())) == str.size ();
	}
	uint32_t writeRaw (const void* inBuffer, uint32_t size) override
	{
		auto written = size;
		const uint8_t* ptr = reinterpret_cast<const uint8_t*> (inBuffer);
		while (size)
		{
			auto toWrite = 1;
			buffer.emplace_back (*ptr);
			if (buffer.size () == bufferSize)
			{
				if (!flush ())
					return kStreamIOError;
			}
			size -= toWrite;
			ptr += toWrite;
		}
		return written;
	}
	bool flush ()
	{
		if (buffer.empty ())
			return true;
		auto result = stream.writeRaw (buffer.data (), static_cast<uint32_t> (buffer.size ())) ==
		              buffer.size ();
		buffer.clear ();
		return result;
	}

private:
	OutputStream& stream;
	std::vector<uint8_t> buffer;
	size_t bufferSize;
};

//------------------------------------------------------------------------
class StdOutStream : public OutputStream
{
public:
	StdOutStream () {}
	uint32_t writeRaw (const void* buffer, uint32_t size) override
	{
		auto byteBuffer = reinterpret_cast<const uint8_t*>(buffer);
		for (auto i = 0u; i < size; ++i, ++byteBuffer)
		{
			std::cout << *byteBuffer;
		}
		return size;
	}
	bool operator<< (const std::string& str) override
	{
		std::cout << str;
		return true;
	}
};

//------------------------------------------------------------------------
} // VSTGUI
