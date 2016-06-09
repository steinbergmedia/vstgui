//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins :
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

#pragma once

#include "../lib/vstguibase.h"
#include <cstring>

namespace VSTGUI {

//-----------------------------------------------------------------------------
struct MallocAllocator
{
	static void* allocate (size_t size) { return std::malloc (size); }
	static void deallocate (void* ptr, size_t size) { std::free (ptr); }
};

//-----------------------------------------------------------------------------
template <typename T, typename Allocator = MallocAllocator>
class Malloc
{
public:
	Malloc () = default;
	Malloc (size_t objectCount) : count (objectCount)
	{
		if (objectCount)
			allocate (objectCount);
	}
	Malloc (Malloc&& other) { *this = std::move (other); }
	Malloc& operator= (Malloc&& other)
	{
		buffer = other.buffer;
		count = other.count;
		other.buffer = nullptr;
		other.count = 0;
		return *this;
	}
	Malloc (const Malloc&) = delete;
	Malloc& operator= (const Malloc&) = delete;
	~Malloc () { deallocate (); }

	T* get () { return buffer; }
	const T* get () const { return buffer; }
	size_t size () const { return count; }

	void allocate (size_t objectCount)
	{
		if (buffer)
			deallocate ();
		if (objectCount)
			buffer = static_cast<T*> (Allocator::allocate (objectCount * sizeof (T)));
		count = objectCount;
	}

	void deallocate ()
	{
		if (buffer)
		{
			Allocator::deallocate (buffer, count * sizeof (T));
			buffer = nullptr;
			count = 0;
		}
	}

private:
	T* buffer {nullptr};
	size_t count {0};
};

} // VSTGUI
