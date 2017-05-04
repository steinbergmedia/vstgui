// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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

	virtual int64_t seek (int64_t pos, SeekMode mode) = 0;	///< returns -1 if seek fails otherwise new position
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

	uint32_t writeRaw (const void* buffer, uint32_t size) override;
	uint32_t readRaw (void* buffer, uint32_t size) override;

	int64_t seek (int64_t pos, SeekMode mode) override;
	int64_t tell () const override;
	void rewind () override;

	bool operator<< (const std::string& str) override;
	bool operator>> (std::string& string) override;
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
protected:
	void* platformHandle;
};


} // namespace

#endif
