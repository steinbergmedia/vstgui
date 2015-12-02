//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "cdropsource.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
CDropSource::CDropEntry::CDropEntry (const void* inBuffer, uint32_t inBufferSize, Type inType)
: buffer (0)
, bufferSize (inBufferSize)
, type (inType)
{
	buffer = std::malloc (bufferSize);
	if (buffer)
		memcpy (buffer, inBuffer, bufferSize);
}

//-----------------------------------------------------------------------------
CDropSource::CDropEntry::CDropEntry (const CDropEntry& entry)
: buffer (0)
, bufferSize (entry.bufferSize)
, type (entry.type)
{
	buffer = std::malloc (bufferSize);
	if (buffer)
		memcpy (buffer, entry.buffer, bufferSize);
}

#if VSTGUI_RVALUE_REF_SUPPORT
//-----------------------------------------------------------------------------
CDropSource::CDropEntry::CDropEntry (CDropEntry&& entry) noexcept
{
	buffer = entry.buffer;
	bufferSize = entry.bufferSize;
	type = entry.type;
	entry.buffer = nullptr;
	entry.bufferSize = 0;
	entry.type = kError;
}

#endif
//-----------------------------------------------------------------------------
CDropSource::CDropEntry::~CDropEntry ()
{
	if (buffer)
		std::free (buffer);
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
	entries.push_back (CDropEntry (buffer, bufferSize, type));
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
	return index < getCount () ? entries[index].bufferSize : 0;
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
	buffer = entries[index].buffer;
	type = entries[index].type;
	return entries[index].bufferSize;
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
int32_t CDropSource::getEntrySize (int32_t index) const
{
	return static_cast<int32_t> (getDataSize (static_cast<uint32_t> (index)));
}

//-----------------------------------------------------------------------------
CDropSource::Type CDropSource::getEntryType (int32_t index) const
{
	return getDataType (static_cast<uint32_t> (index));
}

//-----------------------------------------------------------------------------
int32_t CDropSource::getEntry (int32_t index, const void*& buffer, Type& type) const
{
	return static_cast<int32_t> (getData (static_cast<uint32_t> (index), buffer, type));
}
#endif // VSTGUI_ENABLE_DEPRECATED_METHODS

} // namespace
