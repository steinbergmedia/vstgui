// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguibase.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
class IReference
{
public:
	/** decrease refcount and delete object if refcount == 0 */
	virtual void forget () = 0;
	/** increase refcount */
	virtual void remember () = 0;
};

//-----------------------------------------------------------------------------
template<typename T>
class ReferenceCounted : virtual public IReference
{
public:
	ReferenceCounted () = default;
	virtual ~ReferenceCounted () noexcept = default;

	ReferenceCounted (const ReferenceCounted&) {}
	ReferenceCounted& operator= (const ReferenceCounted&) { return *this; }

	//-----------------------------------------------------------------------------
	/// @name Reference Counting Methods
	//-----------------------------------------------------------------------------
	//@{
	void forget () override
	{
		if (--nbReference == 0)
		{
			nbReference = std::numeric_limits<int32_t>::min () / 2;
			beforeDelete ();
			delete this;
		}
	}
	void remember () override { nbReference++; }
	/** get refcount */
	virtual int32_t getNbReference () const { return nbReference; }
	//@}
private:
	virtual void beforeDelete () {}

	T nbReference {1};
};

using AtomicReferenceCounted = ReferenceCounted<std::atomic<int32_t>>;
using NonAtomicReferenceCounted = ReferenceCounted<int32_t>;

//------------------------------------------------------------------------
template<class I>
class SharedPointer
{
public:
	using Type = I;
	//------------------------------------------------------------------------
	inline explicit SharedPointer (I* ptr) noexcept;
	inline SharedPointer (std::nullptr_t ptr) noexcept;
	inline SharedPointer (I* ptr, bool remember) noexcept;
	inline SharedPointer (const SharedPointer&) noexcept;
	inline SharedPointer () noexcept;
	inline ~SharedPointer () noexcept;

	inline SharedPointer<I>& operator= (const SharedPointer<I>&) noexcept;

	inline explicit operator bool () const noexcept { return get () != nullptr; }
	inline I* operator->() const noexcept { return ptr; } // act as I*

	inline I* get () const noexcept { return ptr; }

	inline void reset () noexcept
	{
		if (ptr)
			ptr->forget ();
		ptr = nullptr;
	}

	template<class T>
	SharedPointer<T> cast () const
	{
		if constexpr (std::is_base_of_v<T, I>)
			return SharedPointer<T> (static_cast<T*> (ptr));
		else
			return SharedPointer<T> (dynamic_cast<T*> (ptr));
	}

	inline SharedPointer (SharedPointer<I>&& mp) noexcept;
	inline SharedPointer<I>& operator= (SharedPointer<I>&& mp) noexcept;

	template<typename T>
	inline SharedPointer (const SharedPointer<T>& op) noexcept
	{
		*this = shared (static_cast<I*> (op.get ()));
	}

	template<typename T>
	inline SharedPointer& operator= (const SharedPointer<T>& op) noexcept
	{
		*this = shared (static_cast<I*> (op.get ()));
		return *this;
	}

	template<typename T>
	inline SharedPointer (SharedPointer<T>&& op) noexcept
	{
		*this = std::move (op);
	}

	template<typename T>
	inline SharedPointer& operator= (SharedPointer<T>&& op) noexcept
	{
		if (ptr)
			ptr->forget ();
		ptr = static_cast<T*> (op.ptr);
		op.ptr = nullptr;
		return *this;
	}

	//------------------------------------------------------------------------
protected:
	template<typename T>
	friend class SharedPointer;

	I* ptr {nullptr};
};

//------------------------------------------------------------------------
template<typename T>
bool operator== (const SharedPointer<T>& lhs, std::nullptr_t rhs)
{
	return lhs.get () == rhs;
}

//------------------------------------------------------------------------
template<typename T>
bool operator== (std::nullptr_t lhs, const SharedPointer<T>& rhs)
{
	return lhs == rhs.get ();
}

//------------------------------------------------------------------------
template<typename T1, typename T2>
bool operator== (const SharedPointer<T1>& lhs, const SharedPointer<T2>& rhs)
{
	return lhs.get () == rhs.get ();
}

//------------------------------------------------------------------------
template<typename T>
bool operator<(const SharedPointer<T>& lhs, const SharedPointer<T>& rhs)
{
	return lhs.get () < rhs.get ();
}

#if 1 // C++17
//------------------------------------------------------------------------
template<typename T1, typename T2>
bool operator!= (const SharedPointer<T1>& lhs, const SharedPointer<T2>& rhs)
{
	return lhs.get () != rhs.get ();
}

//------------------------------------------------------------------------
template<typename T>
bool operator!= (const SharedPointer<T>& lhs, std::nullptr_t rhs)
{
	return lhs.get () != rhs;
}

//------------------------------------------------------------------------
template<typename T>
bool operator!= (std::nullptr_t lhs, const SharedPointer<T>& rhs)
{
	return lhs != rhs.get ();
}

#endif

//------------------------------------------------------------------------
template<class I>
inline SharedPointer<I>::SharedPointer (I* _ptr) noexcept : ptr (_ptr)
{
	if (ptr)
		ptr->remember ();
}

//------------------------------------------------------------------------
template<class I>
inline SharedPointer<I>::SharedPointer (std::nullptr_t _ptr) noexcept
{
}

//------------------------------------------------------------------------
template<class I>
inline SharedPointer<I>::SharedPointer (I* _ptr, bool remember) noexcept : ptr (_ptr)
{
	if (ptr && remember)
		ptr->remember ();
}

//------------------------------------------------------------------------
template<class I>
inline SharedPointer<I>::SharedPointer (const SharedPointer<I>& other) noexcept : ptr (other.ptr)
{
	if (ptr)
		ptr->remember ();
}

//------------------------------------------------------------------------
template<class I>
inline SharedPointer<I>::SharedPointer () noexcept : ptr (nullptr)
{
}

//------------------------------------------------------------------------
template<class I>
inline SharedPointer<I>::~SharedPointer () noexcept
{
	if (ptr)
		ptr->forget ();
}

//------------------------------------------------------------------------
template<class I>
inline SharedPointer<I>::SharedPointer (SharedPointer<I>&& mp) noexcept : ptr (nullptr)
{
	*this = std::move (mp);
}

//------------------------------------------------------------------------
template<class I>
inline SharedPointer<I>& SharedPointer<I>::operator= (SharedPointer<I>&& mp) noexcept
{
	if (ptr)
		ptr->forget ();
	ptr = mp.ptr;
	mp.ptr = nullptr;
	return *this;
}

//------------------------------------------------------------------------
template<class I>
inline SharedPointer<I>& SharedPointer<I>::operator= (const SharedPointer<I>& _ptr) noexcept
{
	if (_ptr.get () != ptr)
	{
		if (ptr)
			ptr->forget ();
		ptr = _ptr.get ();
		if (ptr)
			ptr->remember ();
	}
	return *this;
}

//------------------------------------------------------------------------
template<class I>
inline SharedPointer<I> owned (I* p) noexcept
{
	return SharedPointer<I> (p, false);
}

//------------------------------------------------------------------------
template<class I>
inline SharedPointer<I> shared (I* p) noexcept
{
	return SharedPointer<I> (p, true);
}

//------------------------------------------------------------------------
#if VSTGUI_ENABLE_DEPRECATED_METHODS
template<class I, typename... Args>
inline SharedPointer<I> makeOwned (Args&&... args)
{
	return SharedPointer<I> (new I (std::forward<Args> (args)...), false);
}
#endif // VSTGUI_ENABLE_DEPRECATED_METHODS

//------------------------------------------------------------------------
template<class I, typename... Args>
inline SharedPointer<I> makeShared (Args&&... args)
{
	return SharedPointer<I> (new I (std::forward<Args> (args)...), false);
}

//-----------------------------------------------------------------------------
// CBaseObject Declaration
//! @brief Base Object with reference counter
//-----------------------------------------------------------------------------
class CBaseObject : public NonAtomicReferenceCounted
{
public:
	CBaseObject () = default;
	~CBaseObject () noexcept override = default;

	CBaseObject (const CBaseObject&) {}
	CBaseObject& operator= (const CBaseObject&) { return *this; }

	//-----------------------------------------------------------------------------
	/// @name Message Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual CMessageResult notify ([[maybe_unused]] CBaseObject* sender,
								   [[maybe_unused]] IdStringPtr message)
	{
		return kMessageUnknown;
	}
	//@}

	/// @cond ignore
	virtual CBaseObject* newCopy () const { return nullptr; }
	/// @endcond
};

//-----------------------------------------------------------------------------
class CBaseObjectGuard
{
public:
	explicit CBaseObjectGuard (CBaseObject* _obj) : obj (_obj) {}

protected:
	SharedPointer<CBaseObject> obj;
};

//------------------------------------------------------------------------
} // VSTGUI
