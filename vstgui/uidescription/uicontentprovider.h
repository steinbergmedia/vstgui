// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "icontentprovider.h"
#include "cstream.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
class MemoryContentProvider : public CMemoryStream, public IContentProvider
{
public:
	MemoryContentProvider (const void* data, uint32_t dataSize);		// data must be valid the whole lifetime of this object
	uint32_t readRawData (int8_t* buffer, uint32_t size) override;
	void rewind () override;
};

//-----------------------------------------------------------------------------
class InputStreamContentProvider : public IContentProvider
{
public:
	explicit InputStreamContentProvider (InputStream& stream);

	uint32_t readRawData (int8_t* buffer, uint32_t size) override;
	void rewind () override;
protected:
	InputStream& stream;
	int64_t startPos;
};

//------------------------------------------------------------------------
} // VSTGUI
