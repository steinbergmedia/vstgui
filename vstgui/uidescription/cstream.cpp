// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cstream.h"
#include "../lib/cresourcedescription.h"
#include "../lib/malloc.h"
#include "../lib/platform/iplatformresourceinputstream.h"
#include "../lib/platform/platformfactory.h"
#include <algorithm>
#include <sstream>

#if WINDOWS
	#define fseeko _fseeki64
	#define ftello _ftelli64
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
CMemoryStream::CMemoryStream (uint32_t initialSize, uint32_t inDelta, bool binaryMode, ByteOrder byteOrder)
: OutputStream (byteOrder)
, InputStream (byteOrder)
, buffer (nullptr)
, bufferSize (0)
, size (0)
, pos (0)
, delta (inDelta)
, binaryMode (binaryMode)
, ownsBuffer (true)
{
	resize (initialSize);
}

//-----------------------------------------------------------------------------
CMemoryStream::CMemoryStream (const int8_t* inBuffer, uint32_t bufferSize, bool binaryMode, ByteOrder byteOrder)
: OutputStream (byteOrder)
, InputStream (byteOrder)
, buffer (const_cast<int8_t*> (inBuffer))
, bufferSize (bufferSize)
, size (bufferSize)
, pos (0)
, delta (0)
, binaryMode (binaryMode)
, ownsBuffer (false)
{
}

//-----------------------------------------------------------------------------
CMemoryStream::~CMemoryStream () noexcept
{
	if (ownsBuffer && buffer)
		std::free (buffer);
}

//-----------------------------------------------------------------------------
bool CMemoryStream::resize (uint32_t inSize)
{
	if (bufferSize >= inSize)
		return true;

	if (ownsBuffer == false)
		return false;

	uint32_t newSize = bufferSize + delta;
	while (newSize < inSize)
		newSize += delta;

	int8_t* newBuffer = (int8_t*)std::malloc (newSize);
	if (newBuffer && buffer)
		memcpy (newBuffer, buffer, size);
	if (buffer)
		std::free (buffer);
	buffer = newBuffer;
	bufferSize = newSize;
	
	return buffer != nullptr;
}

//-----------------------------------------------------------------------------
uint32_t CMemoryStream::writeRaw (const void* inBuffer, uint32_t inSize)
{
	if (!resize (pos + inSize))
		return kStreamIOError;
	
	memcpy (buffer + pos, inBuffer, inSize);
	pos += inSize;
	size = pos;
	
	return inSize;
}

//-----------------------------------------------------------------------------
uint32_t CMemoryStream::readRaw (void* outBuffer, uint32_t outSize)
{
	if ((size - pos) <= 0)
		return 0;

	outSize = std::min<uint32_t> (outSize, size - pos);
	memcpy (outBuffer, buffer + pos, outSize);
	pos += outSize;

	return outSize;
}

//-----------------------------------------------------------------------------
int64_t CMemoryStream::seek (int64_t seekpos, SeekMode mode)
{
	int64_t newPos;
	switch (mode)
	{
		case kSeekSet: newPos = seekpos; break;
		case kSeekCurrent: newPos = pos + seekpos; break;
		case kSeekEnd:
		default:
			newPos = size - seekpos;
			break;
	}
	if (newPos <= size && newPos > 0)
	{
		pos = static_cast<uint32_t> (newPos);
		return pos;
	}
	return kStreamSeekError;
}

//-----------------------------------------------------------------------------
bool CMemoryStream::operator>> (std::string& string)
{
	if (binaryMode)
	{
		int32_t identifier;
		if (!(*static_cast<InputStream*> (this) >> identifier)) return false;
		if (identifier == 'str ')
		{
			uint32_t length;
			if (!(*static_cast<InputStream*> (this) >> length)) return false;
			Buffer<int8_t> buf (length);
			uint32_t read = readRaw (buf.data (), length);
			if (read == length)
				string.assign ((const char*)buf.data (), length);
			return read == length;
		}
	}
	else
	{
		int8_t character;
		while (readRaw (&character, sizeof (character)) == sizeof (character))
		{
			if (character == 0)
				break;
			string.push_back (character);
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CMemoryStream::operator<< (const std::string& str)
{
	if (binaryMode)
	{
		if (!((*this) << (uint32_t)'str ')) return false;
		if (!((*this) << (uint32_t)str.length ())) return false;
	}
	return writeRaw (str.c_str (), static_cast<uint32_t> (str.length ())) == str.length ();
}

//-----------------------------------------------------------------------------
bool CMemoryStream::end ()
{
	if (binaryMode == false)
	{
		const int8_t zero = 0;
		return writeRaw (&zero, 1) == 1;
	}
	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CFileStream::CFileStream ()
: stream (nullptr)
{
}

//-----------------------------------------------------------------------------
CFileStream::~CFileStream () noexcept
{
	if (stream)
	{
		fclose (stream);
	}
}

//-----------------------------------------------------------------------------
bool CFileStream::open (UTF8StringPtr path, int32_t mode, ByteOrder byteOrder)
{
	if (stream)
		return false;
	InputStream::setByteOrder (byteOrder);
	OutputStream::setByteOrder (byteOrder);
	std::stringstream fmode;
	if (mode & kTruncateMode)
	{
		if (mode & kReadMode && mode & kWriteMode)
			fmode << "w+";
		else if (mode & kReadMode)
			fmode << "r";
		else if (mode & kWriteMode)
			fmode << "w";
	}
	else
	{
		if (mode & kReadMode && mode & kWriteMode)
			fmode << "a+";
		else if (mode & kWriteMode)
			fmode << "a";
		else if (mode & kReadMode)
			fmode << "r";
		else
			return false;
	}
#if WINDOWS
	// always use binary mode so that newlines are not converted
	fmode << "b";
#else
	if (mode & kBinaryMode)
		fmode << "b";
#endif
	stream = fopen (path, fmode.str ().c_str ());
	openMode = mode;

	return stream != nullptr;
}

//------------------------------------------------------------------------
bool CFileStream::isEndOfFile () const
{
	if (stream)
	{
		return feof (stream) != 0;
	}
	return true;
}

//-----------------------------------------------------------------------------
uint32_t CFileStream::writeRaw (const void* buffer, uint32_t size)
{
	if (stream)
	{
		uint32_t written = static_cast<uint32_t> (fwrite (buffer, size, 1, stream));
		return written * size;
	}
	return kStreamIOError;
}

//-----------------------------------------------------------------------------
uint32_t CFileStream::readRaw (void* buffer, uint32_t size)
{
	if (stream)
	{
		return static_cast<uint32_t> (fread (buffer, 1, size, stream));
	}
	return kStreamIOError;
}

//-----------------------------------------------------------------------------
int64_t CFileStream::seek (int64_t pos, SeekMode mode)
{
	if (stream)
	{
		int fseekmode;
		switch (mode)
		{
			case kSeekSet: fseekmode = SEEK_SET; break;
			case kSeekCurrent: fseekmode = SEEK_CUR; break;
			case kSeekEnd:
			default:
				fseekmode = SEEK_END;
				break;
		}
		if (fseeko (stream, pos, fseekmode) == 0)
			return tell ();
	}
	return kStreamSeekError;
}

//-----------------------------------------------------------------------------
int64_t CFileStream::tell () const
{
	if (stream)
	{
		return ftello (stream);
	}
	return kStreamSeekError;
}

//-----------------------------------------------------------------------------
void CFileStream::rewind ()
{
	if (stream)
	{
		fseek (stream, 0, SEEK_SET);
	}
}

//-----------------------------------------------------------------------------
bool CFileStream::operator>> (std::string& string)
{
	int8_t character;
	string.clear ();
	while (readRaw (&character, sizeof (character)) == sizeof (character))
	{
		if (character == 0)
			break;
		string.push_back (character);
	}
	return true;
}

//-----------------------------------------------------------------------------
bool CFileStream::operator<< (const std::string& str)
{
	if (writeRaw (str.c_str (), static_cast<uint32_t> (str.size ())) == str.size ())
	{
		if (openMode & kBinaryMode)
		{
			if (!((*this) << (int8_t)0))
				return false;
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CResourceInputStream::CResourceInputStream (ByteOrder byteOrder)
: InputStream (byteOrder)
{
}

//-----------------------------------------------------------------------------
CResourceInputStream::~CResourceInputStream () noexcept
{
}

//-----------------------------------------------------------------------------
bool CResourceInputStream::open (const CResourceDescription& res)
{
	if (platformStream)
		return false;
	platformStream = getPlatformFactory ().createResourceInputStream (res);
	return platformStream != nullptr;
}

//-----------------------------------------------------------------------------
uint32_t CResourceInputStream::readRaw (void* buffer, uint32_t size)
{
	if (platformStream)
		return platformStream->readRaw (buffer, size);
	return kStreamIOError;
}

//-----------------------------------------------------------------------------
int64_t CResourceInputStream::seek (int64_t pos, SeekMode mode)
{
	if (platformStream)
	{
		VSTGUI::SeekMode sm = VSTGUI::SeekMode::Set;
		switch (mode)
		{
			case kSeekCurrent: sm = VSTGUI::SeekMode::Current; break;
			case kSeekSet: sm = VSTGUI::SeekMode::Set; break;
			case kSeekEnd: sm = VSTGUI::SeekMode::End; break;
		}
		return platformStream->seek (pos, sm);
	}
	return kStreamSeekError;
}

//-----------------------------------------------------------------------------
int64_t CResourceInputStream::tell () const
{
	if (platformStream)
		return platformStream->tell ();

	return kStreamSeekError;
}

//-----------------------------------------------------------------------------
void CResourceInputStream::rewind ()
{
	if (platformStream)
		platformStream->seek (0, VSTGUI::SeekMode::Set);
}

//-----------------------------------------------------------------------------
template<typename T>
void endianSwap (T& value)
{
	uint32_t size = sizeof (T);
	auto low = reinterpret_cast<uint8_t*> (&value);
	auto high = low + size - 1;
	while (size >= 2)
	{
		auto tmp = *low;
		*low = *high;
		*high = tmp;
		low += 2;
		high -= 2;
		size -= 2;
	}
}

//-----------------------------------------------------------------------------
template<typename T>
bool writeEndianSwap (const T& value, OutputStream& s)
{
	auto tmp = value;
	endianSwap (tmp);
	return s.writeRaw (&tmp, sizeof (T)) == sizeof (T);
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
	return writeEndianSwap (input, *this);
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const uint16_t& input)
{
	if (byteOrder == kNativeByteOrder)
	{
		return writeRaw (&input, sizeof (uint16_t)) == sizeof (uint16_t);
	}
	return writeEndianSwap (input, *this);
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const int32_t& input)
{
	if (byteOrder == kNativeByteOrder)
	{
		return writeRaw (&input, sizeof (int32_t)) == sizeof (int32_t);
	}
	return writeEndianSwap (input, *this);
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const uint32_t& input)
{
	if (byteOrder == kNativeByteOrder)
	{
		return writeRaw (&input, sizeof (uint32_t)) == sizeof (uint32_t);
	}
	return writeEndianSwap (input, *this);
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const int64_t& input)
{
	if (byteOrder == kNativeByteOrder)
	{
		return writeRaw (&input, sizeof (int64_t)) == sizeof (int64_t);
	}
	return writeEndianSwap (input, *this);
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const uint64_t& input)
{
	if (byteOrder == kNativeByteOrder)
	{
		return writeRaw (&input, sizeof (uint64_t)) == sizeof (uint64_t);
	}
	return writeEndianSwap (input, *this);
}

//-----------------------------------------------------------------------------
bool OutputStream::operator<< (const double& input)
{
	if (byteOrder == kNativeByteOrder)
	{
		return writeRaw (&input, sizeof (double)) == sizeof (double);
	}
	return writeEndianSwap (input, *this);
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
			endianSwap (output);
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
			endianSwap (output);
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
			endianSwap (output);
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
			endianSwap (output);
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
			endianSwap (output);
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
			endianSwap (output);
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
			endianSwap (output);
		}
		return true;
	}
	return false;
}

} // VSTGUI
