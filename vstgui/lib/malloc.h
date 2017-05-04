// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
