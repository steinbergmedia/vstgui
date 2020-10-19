// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "fileresourceinputstream.h"

#if WINDOWS
#define fseeko _fseeki64
#define ftello _ftelli64
#endif

//-----------------------------------------------------------------------------
namespace VSTGUI {

PlatformResourceInputStreamPtr FileResourceInputStream::create (const std::string& path)
{
	auto cstr = path.data ();
	if (auto handle = fopen (cstr, "rb"))
		return PlatformResourceInputStreamPtr (new FileResourceInputStream (handle));
	return nullptr;
}

//-----------------------------------------------------------------------------
FileResourceInputStream::FileResourceInputStream (FILE* handle) : fileHandle (handle) {}

//-----------------------------------------------------------------------------
FileResourceInputStream::~FileResourceInputStream () noexcept
{
	fclose (fileHandle);
}

//-----------------------------------------------------------------------------
uint32_t FileResourceInputStream::readRaw (void* buffer, uint32_t size)
{
	uint32_t readResult = static_cast<uint32_t> (fread (buffer, 1, size, fileHandle));
	if (readResult == 0)
	{
		if (ferror (fileHandle) != 0)
		{
			readResult = kStreamIOError;
			clearerr (fileHandle);
		}
	}
	return readResult;
}

//-----------------------------------------------------------------------------
int64_t FileResourceInputStream::seek (int64_t pos, SeekMode mode)
{
	int whence;
	switch (mode)
	{
		case SeekMode::Set:
			whence = SEEK_SET;
			break;
		case SeekMode::Current:
			whence = SEEK_CUR;
			break;
		case SeekMode::End:
		default:
			whence = SEEK_END;
			break;
	}
	if (fseeko (fileHandle, pos, whence) == 0)
		return tell ();
	return kStreamSeekError;
}

//-----------------------------------------------------------------------------
int64_t FileResourceInputStream::tell ()
{
	return ftello (fileHandle);
}

//-----------------------------------------------------------------------------
} // VSTGUI
