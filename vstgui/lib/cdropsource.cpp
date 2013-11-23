//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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
CDropSource::CDropSource ()
{
}

//-----------------------------------------------------------------------------
CDropSource::CDropSource (const void* buffer, int32_t bufferSize, Type type)
{
	add (buffer, bufferSize, type);
}

//-----------------------------------------------------------------------------
CDropSource::~CDropSource ()
{
	for (int32_t i = getCount ()-1; i >= 0; i--)
	{
		CDropEntry* entry = entries[i];
		free (entry->buffer);
		delete entry;
	}
}

//-----------------------------------------------------------------------------
bool CDropSource::add (const void* buffer, int32_t bufferSize, Type type)
{
	if (bufferSize > 0)
	{
		CDropEntry* entry = new CDropEntry;
		entry->buffer = malloc (bufferSize);
		if (entry->buffer)
		{
			memcpy (entry->buffer, buffer, bufferSize);
			entry->bufferSize = bufferSize;
			entry->type = type;
			entries.push_back (entry);
			return true;
		}
		delete entry;
	}
	return false;
}

//-----------------------------------------------------------------------------
int32_t CDropSource::getCount ()
{
	return (int32_t)entries.size ();
}

//-----------------------------------------------------------------------------
int32_t CDropSource::getEntrySize (int32_t index)
{
	CDropEntry* entry = index < getCount () ? entries[index] : 0;
	if (entry)
	{
		return entry->bufferSize;
	}
	return -1;
}

//-----------------------------------------------------------------------------
CDropSource::Type CDropSource::getEntryType (int32_t index)
{
	CDropEntry* entry = index < getCount () ? entries[index] : 0;
	if (entry)
	{
		return entry->type;
	}
	return kError;
}

//-----------------------------------------------------------------------------
int32_t CDropSource::getEntry (int32_t index, const void*& buffer, Type& type)
{
	CDropEntry* entry = index < getCount () ? entries[index] : 0;
	if (entry)
	{
		buffer = entry->buffer;
		type = entry->type;
		return entry->bufferSize;
	}
	return -1;
}

} // namespace
