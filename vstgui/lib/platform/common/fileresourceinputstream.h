// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformresourceinputstream.h"
#include <string>
#include <cstdio>

//-----------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
class FileResourceInputStream : public IPlatformResourceInputStream
{
public:
	static PlatformResourceInputStreamPtr create (const std::string& path);

private:
	FileResourceInputStream (FILE* handle);
	~FileResourceInputStream () noexcept override;

	uint32_t readRaw (void* buffer, uint32_t size) override;
	int64_t seek (int64_t pos, SeekMode mode) override;
	int64_t tell () override;

	FILE* fileHandle;
};

//-----------------------------------------------------------------------------
} // VSTGUI
