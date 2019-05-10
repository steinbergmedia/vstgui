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
class Buffer final
{
public:
	Buffer () = default;
	Buffer (size_t objectCount)
	{
		allocate (objectCount);
	}
	Buffer (Buffer&& other) { *this = std::move (other); }
	Buffer& operator= (Buffer&& other)
	{
		buffer = other.buffer;
		count = other.count;
		other.buffer = nullptr;
		other.count = 0;
		return *this;
	}
	Buffer (const Buffer&) = delete;
	Buffer& operator= (const Buffer&) = delete;
	~Buffer () noexcept { deallocate (); }

	T* data () { return buffer; }
	const T* data () const { return buffer; }
	T* get () { return data (); }
	const T* get () const { return data (); }
	T& operator[] (size_t index) { vstgui_assert (index < count); return buffer[index]; }
	const T& operator[] (size_t index) const { vstgui_assert (index < count); return buffer[index]; }
	T* begin () noexcept { return buffer; }
	T* end () noexcept { return buffer + count; }
	const T* begin () const noexcept { return buffer; }
	const T* end () const noexcept { return buffer + count; }

	size_t size () const { return count; }
	bool empty () const { return count == 0; }

	void allocate (size_t objectCount)
	{
		if (count == objectCount)
			return;
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
