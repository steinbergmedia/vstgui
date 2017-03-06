//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
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

#include "cstream.h"
#include "../lib/cresourcedescription.h"
#include <algorithm>
#include <sstream>

#if MAC
	#include "../lib/platform/mac/macglobals.h"
#elif WINDOWS
	#include "../lib/platform/win32/win32support.h"
	#define fseeko _fseeki64
	#define ftello _ftelli64
#elif LINUX
	#include "../lib/platform/linux/x11platform.h"
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
CMemoryStream::CMemoryStream (uint32_t initialSize, uint32_t inDelta, bool binaryMode, ByteOrder byteOrder)
: OutputStream (byteOrder)
, InputStream (byteOrder)
, buffer (0)
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
CMemoryStream::~CMemoryStream ()
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
	
	return buffer != 0;
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
		case kSeekEnd: newPos = size - seekpos; break;
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
		if (!(*(InputStream*)this >> identifier)) return false;
		if (identifier == 'str ')
		{
			uint32_t length;
			if (!(*(InputStream*)this >> length)) return false;
			int8_t* buffer = (int8_t*)std::malloc (length);
			uint32_t read = readRaw (buffer, length);
			if (read == length)
				string.assign ((const char*)buffer, length);
			std::free (buffer);
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
		if (!(static_cast<OutputStream&>(*this) << (uint32_t)'str ')) return false;
		if (!(static_cast<OutputStream&>(*this) << (uint32_t)str.length ())) return false;
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
: stream (0)
{
}

//-----------------------------------------------------------------------------
CFileStream::~CFileStream ()
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
	if (mode & kBinaryMode)
		fmode << "b";
	stream = fopen (path, fmode.str ().c_str ());
	openMode = mode;

	return stream != 0;
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
			case kSeekEnd: fseekmode = SEEK_END; break;
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
			if (!(*(OutputStream*)this << (int8_t)0))
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
, platformHandle (0)
{
}

//-----------------------------------------------------------------------------
CResourceInputStream::~CResourceInputStream ()
{
	if (platformHandle)
	{
	#if MAC || LINUX
		fclose ((FILE*)platformHandle);
	#elif WINDOWS
		((ResourceStream*)platformHandle)->Release ();
	#endif
	}
}

//-----------------------------------------------------------------------------
bool CResourceInputStream::open (const CResourceDescription& res)
{
	if (platformHandle != 0)
		return false;
#if MAC
	if (res.type == CResourceDescription::kIntegerType)
		return false;
	if (res.type == CResourceDescription::kStringType && res.u.name[0] == '/')
	{
		// it's an absolute path, we can use it as is
//		platformHandle = fopen (res.u.name, "rb");
	}
	if (platformHandle == 0 && getBundleRef ())
	{
		CFStringRef cfStr = CFStringCreateWithCString (NULL, res.u.name, kCFStringEncodingUTF8);
		if (cfStr)
		{
			CFURLRef url = CFBundleCopyResourceURL (getBundleRef (), cfStr, 0, NULL);
			if (url)
			{
				char filePath[PATH_MAX];
				if (CFURLGetFileSystemRepresentation (url, true, (UInt8*)filePath, PATH_MAX))
				{
					platformHandle = fopen (filePath, "rb");
				}
				CFRelease (url);
			}
			CFRelease (cfStr);
		}
	}
#elif LINUX
	if (res.type == CResourceDescription::kIntegerType)
		return false;
	auto path = X11::Platform::getInstance ().getPath ();
	if (!path.empty ())
	{
		path += "/Contents/Resources/";
		path += res.u.name;
		platformHandle = fopen (path.data (), "rb");
	}
#elif WINDOWS
	platformHandle = new ResourceStream ();
	if (!((ResourceStream*)platformHandle)->open (res, "DATA"))
	{
		((ResourceStream*)platformHandle)->Release ();
		platformHandle = 0;
	}
#endif
	return platformHandle != 0;
}

//-----------------------------------------------------------------------------
uint32_t CResourceInputStream::readRaw (void* buffer, uint32_t size)
{
	uint32_t readResult = kStreamIOError;
	if (platformHandle)
	{
	#if MAC || LINUX
		readResult = static_cast<uint32_t> (fread (buffer, 1, size, (FILE*)platformHandle));
		if (readResult == 0)
		{
			if (ferror ((FILE*)platformHandle) != 0)
			{
				readResult = kStreamIOError;
				clearerr ((FILE*)platformHandle);
			}
		}
	#elif WINDOWS
		ULONG read = 0;
		if (((ResourceStream*)platformHandle)->Read (buffer, size, &read) == S_OK)
			readResult = read;
	#endif
	}
	return readResult;
}

//-----------------------------------------------------------------------------
int64_t CResourceInputStream::seek (int64_t pos, SeekMode mode)
{
	if (platformHandle)
	{
	#if MAC || LINUX
		int whence;
		switch (mode)
		{
			case kSeekSet: whence = SEEK_SET; break;
			case kSeekCurrent: whence = SEEK_CUR; break;
			case kSeekEnd: whence = SEEK_END; break;
		}
		if (fseeko ((FILE*)platformHandle, pos, whence) == 0)
			return tell ();
	#elif WINDOWS
		DWORD dwOrigin;
		switch (mode)
		{
			case kSeekSet: dwOrigin = STREAM_SEEK_SET; break;
			case kSeekCurrent: dwOrigin = STREAM_SEEK_CUR; break;
			case kSeekEnd: dwOrigin = STREAM_SEEK_END; break;
		}
		LARGE_INTEGER li;
		li.QuadPart = pos;
		if (((ResourceStream*)platformHandle)->Seek (li, dwOrigin, 0) == S_OK)
			return tell ();
	#endif
	}
	return kStreamSeekError;
}

//-----------------------------------------------------------------------------
int64_t CResourceInputStream::tell () const
{
	if (platformHandle)
	{
	#if MAC || LINUX
		return ftello ((FILE*)platformHandle);
	#elif WINDOWS
		ULARGE_INTEGER pos;
		LARGE_INTEGER dummy = {0};
		if (((ResourceStream*)platformHandle)->Seek (dummy, STREAM_SEEK_CUR, &pos) == S_OK)
			return (int64_t)pos.QuadPart;
	#endif
	}
	return kStreamSeekError;
}

//-----------------------------------------------------------------------------
void CResourceInputStream::rewind ()
{
	if (platformHandle)
	{
	#if MAC || LINUX
		fseek ((FILE*)platformHandle, 0L, SEEK_SET);
	#elif WINDOWS
		((ResourceStream*)platformHandle)->Revert ();
	#endif
	}
}

//-----------------------------------------------------------------------------
template<typename T>
bool writeEndianSwap (const T& value, OutputStream& s)
{
	uint32_t size = sizeof (T);
	const int8_t* ptr = reinterpret_cast<const int8_t*> (&value);
	while (size >= 2)
	{
		if (s.writeRaw (ptr + 1, 1) != 1)
			return false;
		if (s.writeRaw (ptr, 1) != 1)
			return false;
		size -= 2;
		ptr += 2;
	}
	if (size)
	{
		if (s.writeRaw (ptr, 1) != 1)
			return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
template<typename T>
void endianSwap (T& value)
{
	uint32_t size = sizeof (T);
	int8_t* ptr = reinterpret_cast<int8_t*> (&value);
	int8_t tmp;
	while (size >= 2)
	{
		tmp = ptr[0];
		ptr[0] = ptr[1];
		ptr[1] = tmp;
		ptr += 2;
		size -= 2;
	}
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

} // namespace
