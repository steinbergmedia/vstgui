// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "cstream.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
class ZLibInputStream : public InputStream
{
public:
	ZLibInputStream (ByteOrder byteOrder = kNativeByteOrder);
	~ZLibInputStream () noexcept;

	bool open (InputStream& stream);

	bool operator>> (std::string& string) override { return false; }
	uint32_t readRaw (void* buffer, uint32_t size) override;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//-----------------------------------------------------------------------------
class ZLibOutputStream : public OutputStream
{
public:
	ZLibOutputStream (ByteOrder byteOrder = kNativeByteOrder);
	~ZLibOutputStream () noexcept;

	bool open (OutputStream& stream, int32_t compressionLevel = 6);
	bool close ();

	bool operator<< (const std::string& str) override
	{
		return writeRaw (str.data (), static_cast<uint32_t> (str.size ())) == str.size ();
	}
	uint32_t writeRaw (const void* buffer, uint32_t size) override;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // VSTGUI
