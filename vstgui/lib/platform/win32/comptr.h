// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

namespace VSTGUI {
namespace COM {

//-----------------------------------------------------------------------------
template <typename T>
inline T* IUnknownSafeRef (T* i)
{
	if (i)
		i->AddRef ();
	return i;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T* IUnknownSafeRelease (T* i)
{
	if (i)
		i->Release ();
	return i;
}

//-----------------------------------------------------------------------------
template <typename T>
inline void IUnknownSafeAssign (T*& dst, T* src)
{
	if (src)
		src->AddRef ();
	if (dst)
		dst->Release ();
	dst = src;
}

//-----------------------------------------------------------------------------
template <typename T>
class Ptr
{
public:
//-----------------------------------------------------------------------------
	Ptr () = default;
	Ptr (Ptr&& other) { *this = std::move (other); }
	Ptr (const Ptr& other) : ptr {other.get ()} { IUnknownSafeRef (ptr); }

	template <typename S>
	Ptr (const Ptr<S>& other) : ptr {other.get ()}
	{
		IUnknownSafeRef (ptr);
	}

	~Ptr () { reset (); }

	Ptr& operator= (const Ptr& other)
	{
		IUnknownSafeAssign (ptr, other.get ());
		return *this;
	}

	Ptr& operator= (Ptr&& other)
	{
		std::swap (ptr, other.ptr);
		return *this;
	}

	template <typename S>
	Ptr& operator= (const Ptr<S>& other)
	{
		IUnknownSafeAssign (ptr, other.get ());
		return *this;
	}

	void reset ()
	{
		IUnknownSafeRelease (ptr);
		ptr = nullptr;
	}

	T* get () const { return ptr; }
	T* operator-> () const { return ptr; }
	T& operator* () const { return *ptr; }

	operator bool () const { return ptr != nullptr; }

	T** adoptPtr ()
	{
		reset ();
		return &ptr;
	}

//-----------------------------------------------------------------------------
private:
	Ptr (T* ptr) : ptr {ptr} {}

	T* ptr {nullptr};

	template <typename S>
	friend Ptr<S> adopt (S* ptr);

	template <typename S>
	friend Ptr<S> share (S* ptr);
};

//-----------------------------------------------------------------------------
template <typename S>
Ptr<S> adopt (S* ptr)
{
	return Ptr<S> {ptr};
}

//-----------------------------------------------------------------------------
template <typename S>
Ptr<S> share (S* ptr)
{
	return Ptr<S> {IUnknownSafeRef (ptr)};
}

} // COM
} // VSTGUI
