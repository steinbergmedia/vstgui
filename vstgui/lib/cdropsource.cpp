// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cdropsource.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
CDropSource::CDropEntry::CDropEntry (const void* inBuffer, uint32_t inBufferSize, Type inType)
: type (inType)
{
	buffer.allocate (inBufferSize);
	if (buffer.get ())
		memcpy (buffer.get (), inBuffer, buffer.size ());
}

//-----------------------------------------------------------------------------
CDropSource::CDropEntry::CDropEntry (const CDropEntry& entry)
: type (entry.type)
{
	buffer.allocate (entry.buffer.size ());
	if (buffer.get ())
		memcpy (buffer.get (), entry.buffer.get (), buffer.size ());
}

//-----------------------------------------------------------------------------
CDropSource::CDropEntry::CDropEntry (CDropEntry&& entry) noexcept
{
	buffer = std::move (entry.buffer);
	type = entry.type;
	entry.type = kError;
}

//-----------------------------------------------------------------------------
CDropSource::CDropSource ()
{
}

//-----------------------------------------------------------------------------
CDropSource::CDropSource (const void* buffer, uint32_t bufferSize, Type type)
{
	add (buffer, bufferSize, type);
}

//-----------------------------------------------------------------------------
bool CDropSource::add (const void* buffer, uint32_t bufferSize, Type type)
{
	if (entries.size () == entries.max_size ())
		return false;
	entries.emplace_back (buffer, bufferSize, type);
	return true;
}

//-----------------------------------------------------------------------------
uint32_t CDropSource::getCount () const
{
	return static_cast<uint32_t> (entries.size ());
}

//-----------------------------------------------------------------------------
uint32_t CDropSource::getDataSize (uint32_t index) const
{
	return index < getCount () ? static_cast<uint32_t> (entries[index].buffer.size ()) : 0;
}

//-----------------------------------------------------------------------------
CDropSource::Type CDropSource::getDataType (uint32_t index) const
{
	return index < getCount () ? entries[index].type : kError;
}

//-----------------------------------------------------------------------------
uint32_t CDropSource::getData (uint32_t index, const void*& buffer, Type& type) const
{
	if (index >= getCount ())
		return 0;
	buffer = entries[index].buffer.get ();
	type = entries[index].type;
	return static_cast<uint32_t> (entries[index].buffer.size ());
}

//-----------------------------------------------------------------------------
SharedPointer<IDataPackage> CDropSource::create (const void* buffer, uint32_t bufferSize, Type type)
{
	return makeOwned<CDropSource> (buffer, bufferSize, type);
}

} // VSTGUI
