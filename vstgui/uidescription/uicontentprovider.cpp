// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uicontentprovider.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
MemoryContentProvider::MemoryContentProvider (const void* data, uint32_t dataSize)
: CMemoryStream ((const int8_t*)data, dataSize, false)
{
}

//------------------------------------------------------------------------
uint32_t MemoryContentProvider::readRawData (int8_t* buffer, uint32_t size)
{
	return readRaw (buffer, size);
}

//------------------------------------------------------------------------
void MemoryContentProvider::rewind ()
{
	CMemoryStream::rewind ();
}

//------------------------------------------------------------------------
InputStreamContentProvider::InputStreamContentProvider (InputStream& stream)
: stream (stream)
, startPos (0)
{
	auto* seekStream = dynamic_cast<SeekableStream*> (&stream);
	if (seekStream)
		startPos = seekStream->tell ();
}

//------------------------------------------------------------------------
uint32_t InputStreamContentProvider::readRawData (int8_t* buffer, uint32_t size)
{
	return stream.readRaw (buffer, size);
}

//------------------------------------------------------------------------
void InputStreamContentProvider::rewind ()
{
	auto* seekStream = dynamic_cast<SeekableStream*> (&stream);
	if (seekStream)
		seekStream->seek (startPos, SeekableStream::kSeekSet);
}


//------------------------------------------------------------------------
} // VSTGUI
