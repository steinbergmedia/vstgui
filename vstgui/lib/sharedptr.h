// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#ifndef VSTGUI_VERSION_MAJOR
#error "do not directly include this file, use vstguibase.h"
#endif

#define VSTGUI_BACKTRACE_REFCOUNT (DEBUG && 1)
#define VSTGUI_USE_STD_SHAREDPTR 1

#if VSTGUI_BACKTRACE_REFCOUNT
#include <vector>
#endif

#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
template<typename T>
struct shared_ptr : std::shared_ptr<T>
{
	using std::shared_ptr<T>::shared_ptr;
	using Type = T;

	shared_ptr (const std::shared_ptr<T>& other) : std::shared_ptr<T> (other) {}
	shared_ptr (std::shared_ptr<T>&& other) : std::shared_ptr<T> (std::move (other)) {}
	explicit shared_ptr (T* instance) : std::shared_ptr<T> (instance) {}

	template<typename I>
	shared_ptr<I> cast () const
	{
#if 0
		if constexpr (std::is_base_of_v<T, I> && !std::is_virtual_base_of_v<T,I>)
			return shared_ptr<I> (std::static_pointer_cast<I> (*this));
#endif
		return shared_ptr<I> (std::dynamic_pointer_cast<I> (*this));
	}
};

#if VSTGUI_USE_STD_SHAREDPTR

//------------------------------------------------------------------------
template<typename I>
using SharedPointer = shared_ptr<I>;

//------------------------------------------------------------------------
struct IReference : public std::enable_shared_from_this<IReference>
{
	virtual ~IReference () noexcept = default;
};

//------------------------------------------------------------------------
class ReferenceCounted : virtual public IReference
{
public:
	virtual void beforeDelete () {}

	int32_t getNbReference () const
	{
		return static_cast<int32_t> (shared_from_this ().use_count ()) - 1;
	}
};

//------------------------------------------------------------------------
struct NonAtomicReferenceCounted : public ReferenceCounted
{
};

//------------------------------------------------------------------------
struct AtomicReferenceCounted : public ReferenceCounted
{
};

//------------------------------------------------------------------------
template<typename T>
struct Deleter
{
	void operator() (T* p) const noexcept
	{
		if (ReferenceCounted* rc = static_cast<ReferenceCounted*> (p))
			rc->beforeDelete ();
		delete p;
	}
};

#define VSTGUI_SHAREDPTR_FRIEND(Class)                                                             \
	friend struct VSTGUI::Deleter<Class>;                                                          \
	template<class Class, typename... Args>                                                        \
	friend VSTGUI::SharedPointer<Class> VSTGUI::makeShared (Args&&... args);

//------------------------------------------------------------------------
template<class I>
inline SharedPointer<I> owned (I* p) noexcept
{
	return p ? shared_ptr<I> (p, Deleter<I> {}) : nullptr;
}

//------------------------------------------------------------------------
template<class I>
inline SharedPointer<I> shared (I* p) noexcept
{
	return p ? std::dynamic_pointer_cast<I> (p->shared_from_this ()) : nullptr;
}

//------------------------------------------------------------------------
template<class I, typename... Args>
inline SharedPointer<I> makeShared (Args&&... args)
{
	return shared_ptr<I> (new I (std::forward<Args> (args)...), Deleter<I> {});
}

#else

#define VSTGUI_SHAREDPTR_FRIEND(Class)                                                             \
	template<class Class, typename... Args>                                                        \
	friend VSTGUI::SharedPointer<Class> VSTGUI::makeShared (Args&&... args);

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
#if DEBUG && VSTGUI_BACKTRACE_REFCOUNT
		if (collectBacktrace)
			releaseBacktrace.push_back (Debug::backtrace (8));
#endif
		if (--nbReference == 0)
		{
			nbReference = std::numeric_limits<int32_t>::min () / 2;
			beforeDelete ();
			delete this;
		}
	}
	void remember () override
	{
		nbReference++;
#if DEBUG && VSTGUI_BACKTRACE_REFCOUNT
		if (collectBacktrace)
			addRefBacktrace.push_back (Debug::backtrace (8));
#endif
	}
	/** get refcount */
	virtual int32_t getNbReference () const { return nbReference; }
	//@}
private:
	virtual void beforeDelete () {}

	T nbReference {1};

#if DEBUG && VSTGUI_BACKTRACE_REFCOUNT
public:
	bool collectBacktrace {false};
	std::vector<Debug::Backtrace> addRefBacktrace;
	std::vector<Debug::Backtrace> releaseBacktrace;

#endif
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

	uint32_t use_count () const noexcept
	{
		if (ptr)
			return ptr->getNbReference ();
		return 0;
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

#endif

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
