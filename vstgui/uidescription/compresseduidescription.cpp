// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../lib/cresourcedescription.h"
#include "compresseduidescription.h"
#include "cstream_zlib.h"
#include "uicontentprovider.h"
#include <array>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
class ZLibInputContentProvider : public IContentProvider
{
public:
	ZLibInputContentProvider (InputStream& source) : source (source)
	{
		if (auto seekStream = dynamic_cast<SeekableStream*>(&source))
			startPos = seekStream->tell ();
	}

	bool open ()
	{
		zin = std::make_unique<ZLibInputStream>();
		return zin->open (source);
	}

	uint32_t readRawData (int8_t* buffer, uint32_t size) override
	{
		if (zin)
			return zin->readRaw (buffer, size);
		return 0;
	}

	void rewind () override
	{
		if (auto seekStream = dynamic_cast<SeekableStream*>(&source))
		{
			seekStream->seek (startPos, SeekableStream::SeekMode::kSeekSet);
			open ();
		}
	}

	InputStream& source;
	std::unique_ptr<ZLibInputStream> zin;
	int64_t startPos {0};
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
		ZLibInputContentProvider zin (stream);
		if (zin.open ())
		{
			setContentProvider (&zin);
			result = UIDescription::parse ();
			setContentProvider (nullptr);
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
	if (resStream.open (getUIDescFile ()))
	{
		result = parseWithStream (resStream);
	}
	else if (getUIDescFile ().type == CResourceDescription::kStringType)
	{
		CFileStream fileStream;
		if (fileStream.open (getUIDescFile ().u.name,
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
	originalIsCompressed = true;
	return result;
}

//-----------------------------------------------------------------------------
bool CompressedUIDescription::save (UTF8StringPtr filename, int32_t flags,
									AttributeSaveFilterFunc func)
{
	bool result = false;
	if (originalIsCompressed || (flags & kForceWriteCompressedDesc))
	{
		CFileStream fileStream;
		if (fileStream.open (filename,
		                     CFileStream::kWriteMode | CFileStream::kBinaryMode |
		                         CFileStream::kTruncateMode,
		                     kLittleEndianByteOrder))
		{
			fileStream << kUIDescIdentifier;
			ZLibOutputStream zout;
			if (zout.open (fileStream, compressionLevel))
			{
				if (saveToStream (zout, flags, func))
				{
					result = zout.close ();
				}
			}
		}
	}
	if (!(flags & kNoPlainUIDescFileBackup))
	{
		// make a xml backup
		std::string backupFileName (filename);
		if (originalIsCompressed|| (flags & kForceWriteCompressedDesc))
		{
			if (flags & kWriteAsXML)
				backupFileName.append (".xml");
			else
				backupFileName.append (".json");
		}
		CFileStream xmlFileStream;
		if (xmlFileStream.open (backupFileName.data (),
		                        CFileStream::kWriteMode | CFileStream::kTruncateMode,
		                        kLittleEndianByteOrder))
		{
			result = saveToStream (xmlFileStream, flags, func);
		}
	}
	return result;
}

//------------------------------------------------------------------------
} // VSTGUI
