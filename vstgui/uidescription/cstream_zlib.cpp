// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cstream_zlib.h"
#include <array>

//------------------------------------------------------------------------
namespace VSTGUI {

namespace {
#if _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4334)
#endif
#define MINIZ_NO_STDIO
#define MINIZ_NO_ARCHIVE_APIS
#define MINIZ_NO_ARCHIVE_WRITING_APIS
#include "../thirdparty/miniz/miniz.c"
#if _MSC_VER
#pragma warning(pop)
#endif

using z_stream = mz_stream;
};

//------------------------------------------------------------------------
struct ZLibInputStream::Impl
{
	z_stream zstream {};
	InputStream* stream {nullptr};
	std::array<Bytef, 4096> internalBuffer;
};

//------------------------------------------------------------------------
struct ZLibOutputStream::Impl
{
	z_stream zstream {};
	OutputStream* stream {nullptr};
	std::array<Bytef, 4096> internalBuffer;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ZLibInputStream::ZLibInputStream (ByteOrder byteOrder) : InputStream (byteOrder)
{
	impl = std::make_unique<Impl> ();
}

//-----------------------------------------------------------------------------
ZLibInputStream::~ZLibInputStream () noexcept
{
	if (impl->zstream.next_in != nullptr)
		inflateEnd (&impl->zstream);
}

//-----------------------------------------------------------------------------
bool ZLibInputStream::open (InputStream& _stream)
{
	if (impl->zstream.next_in != nullptr || impl->stream != nullptr)
		return false;
	impl->stream = &_stream;

	auto read = impl->stream->readRaw (impl->internalBuffer.data (),
									   static_cast<uint32_t> (impl->internalBuffer.size ()));
	if (read == 0 || read == kStreamIOError)
		return false;

	impl->zstream.next_in = impl->internalBuffer.data ();
	impl->zstream.avail_in = read;

	if (inflateInit (&impl->zstream) != Z_OK)
	{
		impl->zstream = {};
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
uint32_t ZLibInputStream::readRaw (void* buffer, uint32_t size)
{
	if (impl->zstream.next_in == nullptr || !buffer)
		return kStreamIOError;
	impl->zstream.next_out = static_cast<Bytef*> (buffer);
	impl->zstream.avail_out = size;
	while (impl->zstream.avail_out > 0)
	{
		if (impl->zstream.avail_in == 0)
		{
			auto read = impl->stream->readRaw (
				impl->internalBuffer.data (), static_cast<uint32_t> (impl->internalBuffer.size ()));
			if (read > 0 && read != kStreamIOError)
			{
				impl->zstream.next_in = impl->internalBuffer.data ();
				impl->zstream.avail_in = read;
			}
		}
		auto zres = inflate (&impl->zstream, Z_SYNC_FLUSH);
		if (zres == Z_STREAM_END)
		{
			return size - impl->zstream.avail_out;
		}
		else if (zres != Z_OK)
			return kStreamIOError;
	}
	return size;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ZLibOutputStream::ZLibOutputStream (ByteOrder byteOrder) : OutputStream (byteOrder)
{
	impl = std::make_unique<Impl> ();
	impl->zstream.opaque = this;
}

//-----------------------------------------------------------------------------
ZLibOutputStream::~ZLibOutputStream () noexcept { close (); }

//-----------------------------------------------------------------------------
bool ZLibOutputStream::open (OutputStream& _stream, int32_t compressionLevel)
{
	if (impl->zstream.opaque != nullptr || impl->stream != nullptr)
		return false;
	impl->stream = &_stream;

	if (deflateInit (&impl->zstream, compressionLevel) != Z_OK)
	{
		impl->zstream = {};
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool ZLibOutputStream::close ()
{
	bool result = true;
	if (impl->zstream.opaque == this)
	{
		impl->zstream.next_in = nullptr;
		impl->zstream.avail_in = 0;
		while (true)
		{
			impl->zstream.next_out = impl->internalBuffer.data ();
			impl->zstream.avail_out = static_cast<unsigned int> (impl->internalBuffer.size ());
			auto zres = deflate (&impl->zstream, Z_FINISH);
			if (zres != Z_OK && zres != Z_BUF_ERROR && zres != Z_STREAM_END)
			{
				result = false;
				break;
			}
			else if (impl->zstream.avail_out != impl->internalBuffer.size ())
			{
				auto written = impl->stream->writeRaw (
					impl->internalBuffer.data (),
					static_cast<uint32_t> (impl->internalBuffer.size () - impl->zstream.avail_out));
				if (written != impl->internalBuffer.size () - impl->zstream.avail_out)
				{
					result = false;
					break;
				}
			}
			if (zres == Z_STREAM_END)
				break;
		}
		deflateEnd (&impl->zstream);
		impl->zstream = {};
	}
	return result;
}

//-----------------------------------------------------------------------------
uint32_t ZLibOutputStream::writeRaw (const void* buffer, uint32_t size)
{
	if (impl->zstream.opaque != this)
		return kStreamIOError;
	impl->zstream.next_in = const_cast<Bytef*> (static_cast<const Bytef*> (buffer));
	impl->zstream.avail_in = size;
	while (impl->zstream.avail_in > 0)
	{
		impl->zstream.next_out = impl->internalBuffer.data ();
		impl->zstream.avail_out = static_cast<unsigned int> (impl->internalBuffer.size ());
		auto zres = deflate (&impl->zstream, Z_NO_FLUSH);
		if (zres == Z_STREAM_ERROR)
		{
			return kStreamIOError;
		}
		if (impl->zstream.avail_out != impl->internalBuffer.size ())
		{
			auto written = impl->stream->writeRaw (
				impl->internalBuffer.data (),
				static_cast<uint32_t> (impl->internalBuffer.size () - impl->zstream.avail_out));
			if (written != impl->internalBuffer.size () - impl->zstream.avail_out)
				return kStreamIOError;
		}
	}
	return size;
}

//------------------------------------------------------------------------
} // VSTGUI
