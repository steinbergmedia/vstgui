/*
 *  compresseduidescription.cpp
 *
 *
 *  Created by Arne Scheffler on 10/19/10.
 *  Copyright 2010 Arne Scheffler. All rights reserved.
 *
 */

#include "compresseduidescription.h"
#include "vstgui/uidescription/xmlparser.h"
#include "vstgui/uidescription/cstream.h"

#include <zlib.h>

//------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
class ZLibInputStream : public InputStream
{
public:
	ZLibInputStream (ByteOrder byteOrder = kNativeByteOrder);
	~ZLibInputStream ();

	bool open (InputStream& stream);

	bool operator>> (std::string& string) VSTGUI_OVERRIDE_VMETHOD { return false; }
	uint32_t readRaw (void* buffer, uint32_t size) VSTGUI_OVERRIDE_VMETHOD;

protected:
	z_streamp zstream;
	InputStream* stream;
	void* internalBuffer;
	uint32_t internalBufferSize;
};

//-----------------------------------------------------------------------------
class ZLibOutputStream : public OutputStream
{
public:
	ZLibOutputStream (ByteOrder byteOrder = kNativeByteOrder);
	~ZLibOutputStream ();

	bool open (OutputStream& stream, int32_t compressionLevel = 6);
	bool close ();

	bool operator<< (const std::string& str) VSTGUI_OVERRIDE_VMETHOD
	{
		return writeRaw (str.c_str (), static_cast<uint32_t> (str.size ())) == str.size ();
	}
	uint32_t writeRaw (const void* buffer, uint32_t size) VSTGUI_OVERRIDE_VMETHOD;

protected:
	z_streamp zstream;
	OutputStream* stream;
	void* internalBuffer;
	uint32_t internalBufferSize;
};

//-----------------------------------------------------------------------------
static int64_t kUIDescIdentifier = 0x7072637365646975LL; // 8 byte identifier

//-----------------------------------------------------------------------------
CompressedUIDescription::CompressedUIDescription (const CResourceDescription& compressedUIDescFile)
: UIDescription (compressedUIDescFile)
{
}

//-----------------------------------------------------------------------------
bool CompressedUIDescription::parse ()
{
	if (nodes)
		return true;
	bool result = false;
	CResourceInputStream resStream (kLittleEndianByteOrder);
	if (resStream.open (xmlFile))
	{
		int64_t identifier;
		static_cast<InputStream&> (resStream) >> identifier;
		if (identifier == kUIDescIdentifier)
		{
			ZLibInputStream zin;
			if (zin.open (resStream))
			{
				Xml::InputStreamContentProvider compressedContentProvider (zin);
				xmlContentProvider = &compressedContentProvider;
				result = UIDescription::parse ();
				xmlContentProvider = nullptr;
			}
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
	if (fileStream.open (filename, CFileStream::kWriteMode | CFileStream::kBinaryMode |
									   CFileStream::kTruncateMode),
		kLittleEndianByteOrder)
	{
		static_cast<OutputStream&> (fileStream) << kUIDescIdentifier;
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
								CFileStream::kWriteMode | CFileStream::kTruncateMode),
			kLittleEndianByteOrder)
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
: InputStream (byteOrder)
, zstream (nullptr)
, stream (nullptr)
, internalBuffer (nullptr)
, internalBufferSize (0)
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
	if (internalBuffer)
		free (internalBuffer);
}

//-----------------------------------------------------------------------------
bool ZLibInputStream::open (InputStream& _stream)
{
	if (zstream != nullptr || stream != nullptr)
		return false;
	stream = &_stream;

	internalBufferSize = 1024;
	internalBuffer = malloc (internalBufferSize);
	uint32_t read = stream->readRaw (internalBuffer, internalBufferSize);
	if (read == 0)
	{
		free (internalBuffer);
		internalBuffer = nullptr;
		return false;
	}

	zstream = static_cast<z_streamp> (malloc (sizeof (z_stream)));
	memset (zstream, 0, sizeof (z_stream));

	zstream->next_in = static_cast<Bytef*> (internalBuffer);
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
			uint32_t read = stream->readRaw (internalBuffer, internalBufferSize);
			if (read > 0)
			{
				zstream->next_in = static_cast<Bytef*> (internalBuffer);
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
: OutputStream (byteOrder)
, zstream (nullptr)
, stream (nullptr)
, internalBuffer (nullptr)
, internalBufferSize (0)
{
}

//-----------------------------------------------------------------------------
ZLibOutputStream::~ZLibOutputStream () { close (); }

//-----------------------------------------------------------------------------
bool ZLibOutputStream::open (OutputStream& _stream, int32_t compressionLevel)
{
	if (zstream != nullptr || stream != nullptr)
		return false;
	stream = &_stream;

	internalBufferSize = 1024;
	internalBuffer = malloc (internalBufferSize);

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
			zstream->next_out = static_cast<Bytef*> (internalBuffer);
			zstream->avail_out = internalBufferSize;
			int zres = deflate (zstream, Z_FINISH);
			if (zres != Z_OK && zres != Z_BUF_ERROR && zres != Z_STREAM_END)
			{
				result = false;
				break;
			}
			else if (zstream->avail_out != internalBufferSize)
			{
				uint32_t written =
					stream->writeRaw (internalBuffer, internalBufferSize - zstream->avail_out);
				if (written != internalBufferSize - zstream->avail_out)
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
	if (internalBuffer)
	{
		free (internalBuffer);
		internalBuffer = nullptr;
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
		zstream->next_out = static_cast<Bytef*> (internalBuffer);
		zstream->avail_out = internalBufferSize;
		int zres = deflate (zstream, Z_NO_FLUSH);
		if (zres == Z_STREAM_ERROR)
		{
			return -1;
		}
		if (zstream->avail_out != internalBufferSize)
		{
			uint32_t written =
				stream->writeRaw (internalBuffer, internalBufferSize - zstream->avail_out);
			if (written != internalBufferSize - zstream->avail_out)
				return -1;
		}
	}
	return size;
}

//------------------------------------------------------------------------
} // namespace
