// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "compresseduidescription.h"
#include "cstream.h"
#include "xmlparser.h"
#include "../lib/cresourcedescription.h"
#include <array>

//------------------------------------------------------------------------
namespace VSTGUI {

namespace {
#define MINIZ_NO_STDIO
#define MINIZ_NO_ARCHIVE_APIS
#define MINIZ_NO_ARCHIVE_WRITING_APIS
#include "miniz/miniz.c"
};

using z_streamp = mz_streamp;

//-----------------------------------------------------------------------------
class ZLibInputStream : public InputStream
{
public:
	ZLibInputStream (ByteOrder byteOrder = kNativeByteOrder);
	~ZLibInputStream ();

	bool open (InputStream& stream);

	bool operator>> (std::string& string) override { return false; }
	uint32_t readRaw (void* buffer, uint32_t size) override;

protected:
	z_streamp zstream;
	InputStream* stream;
	std::array<Bytef, 4096> internalBuffer;
};

//-----------------------------------------------------------------------------
class ZLibOutputStream : public OutputStream
{
public:
	ZLibOutputStream (ByteOrder byteOrder = kNativeByteOrder);
	~ZLibOutputStream ();

	bool open (OutputStream& stream, int32_t compressionLevel = 6);
	bool close ();

	bool operator<< (const std::string& str) override
	{
		return writeRaw (str.c_str (), static_cast<uint32_t> (str.size ())) == str.size ();
	}
	uint32_t writeRaw (const void* buffer, uint32_t size) override;

protected:
	z_streamp zstream;
	OutputStream* stream;
	std::array<Bytef, 4096> internalBuffer;
};

//-----------------------------------------------------------------------------
static constexpr int64_t kUIDescIdentifier = 0x7072637365646975LL; // 8 byte identifier

//-----------------------------------------------------------------------------
CompressedUIDescription::CompressedUIDescription (const CResourceDescription& compressedUIDescFile)
: UIDescription (compressedUIDescFile)
{
}

//-----------------------------------------------------------------------------
bool CompressedUIDescription::parseWithStream (InputStream& stream)
{
	bool result = false;
	int64_t identifier;
	stream >> identifier;
	if (identifier == kUIDescIdentifier)
	{
		ZLibInputStream zin;
		if (zin.open (stream))
		{
			Xml::InputStreamContentProvider compressedContentProvider (zin);
			setXmlContentProvider (&compressedContentProvider);
			result = UIDescription::parse ();
			setXmlContentProvider (nullptr);
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
bool CompressedUIDescription::parse ()
{
	if (parsed ())
		return true;
	bool result = false;
	CResourceInputStream resStream (kLittleEndianByteOrder);
	if (resStream.open (getXmlFile ()))
	{
		result = parseWithStream (resStream);
	}
	else if (getXmlFile ().type == CResourceDescription::kStringType)
	{
		CFileStream fileStream;
		if (fileStream.open (getXmlFile ().u.name,
		                     CFileStream::kReadMode | CFileStream::kBinaryMode,
		                     kLittleEndianByteOrder))
		{
			result = parseWithStream (fileStream);
		}
	}
	if (!result)
	{
		// fallback, check if it is an uncompressed UIDescription file
		return UIDescription::parse ();
	}
	return result;
}

//-----------------------------------------------------------------------------
bool CompressedUIDescription::save (UTF8StringPtr filename, int32_t flags)
{
	bool result = false;
	CFileStream fileStream;
	if (fileStream.open (filename,
	                     CFileStream::kWriteMode | CFileStream::kBinaryMode |
	                         CFileStream::kTruncateMode,
	                     kLittleEndianByteOrder))
	{
		fileStream << kUIDescIdentifier;
		ZLibOutputStream zout;
		if (zout.open (fileStream))
		{
			if (saveToStream (zout, flags))
			{
				result = zout.close ();
			}
		}
	}
	if (result)
	{
		// make a xml backup
		std::string xmlFileName (filename);
		xmlFileName.append (".xml");
		CFileStream xmlFileStream;
		if (xmlFileStream.open (xmlFileName.c_str (),
		                        CFileStream::kWriteMode | CFileStream::kTruncateMode,
		                        kLittleEndianByteOrder))
		{
			saveToStream (xmlFileStream, flags);
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ZLibInputStream::ZLibInputStream (ByteOrder byteOrder)
: InputStream (byteOrder), zstream (nullptr), stream (nullptr)
{
}

//-----------------------------------------------------------------------------
ZLibInputStream::~ZLibInputStream ()
{
	if (zstream)
	{
		inflateEnd (zstream);
		free (zstream);
	}
}

//-----------------------------------------------------------------------------
bool ZLibInputStream::open (InputStream& _stream)
{
	if (zstream != nullptr || stream != nullptr)
		return false;
	stream = &_stream;

	uint32_t read = stream->readRaw (internalBuffer.data (), internalBuffer.size ());
	if (read == 0)
		return false;

	zstream = static_cast<z_streamp> (malloc (sizeof (z_stream)));
	memset (zstream, 0, sizeof (z_stream));

	zstream->next_in = internalBuffer.data ();
	zstream->avail_in = read;

	if (inflateInit (zstream) != Z_OK)
	{
		free (zstream);
		zstream = nullptr;
	}

	return zstream != nullptr;
}

//-----------------------------------------------------------------------------
uint32_t ZLibInputStream::readRaw (void* buffer, uint32_t size)
{
	zstream->next_out = static_cast<Bytef*> (buffer);
	zstream->avail_out = size;
	while (zstream->avail_out > 0)
	{
		if (zstream->avail_in == 0)
		{
			uint32_t read = stream->readRaw (internalBuffer.data (), internalBuffer.size ());
			if (read > 0)
			{
				zstream->next_in = internalBuffer.data ();
				zstream->avail_in = read;
			}
		}
		int zres = inflate (zstream, Z_SYNC_FLUSH);
		if (zres == Z_STREAM_END)
		{
			return size - zstream->avail_out;
		}
		else if (zres != Z_OK)
			return -1;
	}
	return size;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ZLibOutputStream::ZLibOutputStream (ByteOrder byteOrder)
: OutputStream (byteOrder), zstream (nullptr), stream (nullptr)
{
}

//-----------------------------------------------------------------------------
ZLibOutputStream::~ZLibOutputStream ()
{
	close ();
}

//-----------------------------------------------------------------------------
bool ZLibOutputStream::open (OutputStream& _stream, int32_t compressionLevel)
{
	if (zstream != nullptr || stream != nullptr)
		return false;
	stream = &_stream;

	zstream = static_cast<z_streamp> (malloc (sizeof (z_stream)));
	memset (zstream, 0, sizeof (z_stream));

	if (deflateInit (zstream, compressionLevel) != Z_OK)
	{
		free (zstream);
		zstream = nullptr;
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool ZLibOutputStream::close ()
{
	bool result = true;
	if (zstream)
	{
		zstream->next_in = 0;
		zstream->avail_in = 0;
		while (true)
		{
			zstream->next_out = internalBuffer.data ();
			zstream->avail_out = internalBuffer.size ();
			int zres = deflate (zstream, Z_FINISH);
			if (zres != Z_OK && zres != Z_BUF_ERROR && zres != Z_STREAM_END)
			{
				result = false;
				break;
			}
			else if (zstream->avail_out != internalBuffer.size ())
			{
				uint32_t written = stream->writeRaw (internalBuffer.data (),
				                                     internalBuffer.size () - zstream->avail_out);
				if (written != internalBuffer.size () - zstream->avail_out)
				{
					result = false;
					break;
				}
			}
			if (zres == Z_STREAM_END)
				break;
		}
		deflateEnd (zstream);
		free (zstream);
		zstream = nullptr;
	}
	return result;
}

//-----------------------------------------------------------------------------
uint32_t ZLibOutputStream::writeRaw (const void* buffer, uint32_t size)
{
	zstream->next_in = const_cast<Bytef*> (static_cast<const Bytef*> (buffer));
	zstream->avail_in = size;
	while (zstream->avail_in > 0)
	{
		zstream->next_out = internalBuffer.data ();
		zstream->avail_out = internalBuffer.size ();
		int zres = deflate (zstream, Z_NO_FLUSH);
		if (zres == Z_STREAM_ERROR)
		{
			return -1;
		}
		if (zstream->avail_out != internalBuffer.size ())
		{
			uint32_t written = stream->writeRaw (internalBuffer.data (),
			                                     internalBuffer.size () - zstream->avail_out);
			if (written != internalBuffer.size () - zstream->avail_out)
				return -1;
		}
	}
	return size;
}

//------------------------------------------------------------------------
} // namespace
