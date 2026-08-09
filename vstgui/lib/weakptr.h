// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguibase.h"

#if VSTGUI_USE_STD_SHAREDPTR
//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
template<typename I>
struct WeakPointer : std::weak_ptr<I>
{
	using std::weak_ptr<I>::weak_ptr;

	template<typename T>
	WeakPointer (const SPtr<T>& sp) : std::weak_ptr<I> (std::static_pointer_cast<I> (sp))
	{
	}

	WeakPointer<I>& operator= (const SPtr<I>& sp)
	{
		std::weak_ptr<I>::operator= (static_cast<const std::shared_ptr<I>&> (sp));
		return *this;
	}

	SPtr<I> lock () const noexcept { return SPtr<I> (std::weak_ptr<I>::lock ()); }
};

//------------------------------------------------------------------------
template<typename T>
struct WeakPointerSupport
{

	WeakPointer<T> weakFromThis () const
	{
		return WeakPointer<T> (std::static_pointer_cast<T> (
			const_cast<T*> (static_cast<const T*> (this))->shared_from_this ()));
	}
};

//------------------------------------------------------------------------
} // VSTGUI

#else

#include <algorithm>
#include <mutex>
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
struct IWeakPointer
{
	virtual void onObjectDestructed () noexcept = 0;
};

//------------------------------------------------------------------------
struct IWeakPointerSupport
{
	virtual void registerWeakPointer (IWeakPointer* wp) noexcept = 0;
	virtual void unregisterWeakPointer (IWeakPointer* wp) noexcept = 0;
};

//------------------------------------------------------------------------
/** A non-owning, thread-safe weak reference wrapper for objects managed by VSTGUI's SPtr.

WeakPointer allows you to reference an object without extending its lifetime, preventing reference
cycles and enabling you to safely check whether the object still exists. It cooperates with
WeakPointerSupport embedded in the pointee type to be notified when the object is destroyed.

Key characteristics:
- Does not increase the reference count of the target object.
- Thread-safe access and state transitions via internal mutex protection.
- Can be constructed from either a SPtr<I> or another WeakPointer<I>.
- Provides lock() to obtain a temporary SPtr<I> if the object is still alive.
- Automatically invalidates itself when the target object is destroyed (expired()).

Usage:
- Hold WeakPointer<I> where you need a non-owning reference to an object managed by
  SPtr<I>.
- Call lock() to get a SPtr<I> before accessing the object; check for null to handle
  expiration.
- Use expired() to quickly test whether the underlying object has been destroyed.
- Call reset() to manually detach from the current target and unregister from its weak list.

Requirements for I:
- I must inherit from WeakPointerSupport<I> so that the object can register/unregister weak
  references and notify them upon destruction.

Threading:
- All operations synchronize on an internal mutex to ensure consistent state across threads.
- lock(), reset(), assignment, and destruction are safe to call concurrently with object
  destruction.

Lifecycle:
- Construction from SPtr<I> registers the WeakPointer with the target’s
  WeakPointerSupport<I>.
- Copy construction/assignment attempts to lock the source, then registers with the live target
  if available.
- Destruction or reset() unregisters the WeakPointer from the target if it is still alive.
- When the target object is destroyed, WeakPointerSupport<I> calls onObjectDestructed() so that
  the WeakPointer becomes expired() without needing explicit user action.

Common patterns:
- Break ownership cycles between objects that reference each other.
- Cache back-references or listeners without preventing deallocation.
- Safely attempt access to UI or engine objects that may be torn down on other threads.

Notes:
- Always check the result of lock() before dereferencing; it may return an empty SPtr
  if expired.
- expired() is a fast check, but lock() is the canonical way to access the object safely.
- Avoid long-lived locks or heavy work while holding the returned SPtr to minimize
  contention.
 */
template<class I>
struct WeakPointer final : IWeakPointer
{
	inline WeakPointer () noexcept {}
	template<typename T>
	inline WeakPointer (const SPtr<T>& object) noexcept;
	inline WeakPointer (const WeakPointer<I>& object) noexcept;
	template<typename T>
	inline WeakPointer (const WeakPointer<T>& object) noexcept;
	inline WeakPointer (WeakPointer<I>&& other) noexcept;
	inline ~WeakPointer () noexcept;

	template<typename T>
	inline WeakPointer<I>& operator= (const SPtr<T>& other) noexcept;
	inline WeakPointer<I>& operator= (const WeakPointer<I>& other) noexcept;
	template<typename T>
	inline WeakPointer<I>& operator= (const WeakPointer<T>& other) noexcept;
	inline WeakPointer<I>& operator= (WeakPointer<I>&& other) noexcept;

	inline SPtr<I> lock () const noexcept;
	inline void reset () noexcept;
	inline void swap (WeakPointer<I>& other) noexcept;
	inline bool expired () const noexcept;

protected:
	friend struct IWeakPointerSupport;

	inline void onObjectDestructed () noexcept override;

	mutable std::recursive_mutex m;
	IWeakPointerSupport* object {nullptr};
};

//------------------------------------------------------------------------
template<class I>
bool operator== (const WeakPointer<I>& lhs, const WeakPointer<I>& rhs)
{
	auto lhsp = lhs.lock ();
	auto rhsp = rhs.lock ();
	return lhsp.get () == rhsp.get ();
}

//------------------------------------------------------------------------
template<class I>
bool operator== (const WeakPointer<I>& lhs, const I* rhs)
{
	auto lhsp = lhs.lock ();
	return lhsp.get () == rhs;
}

//------------------------------------------------------------------------
template<class I>
bool operator== (const I* lhs, const WeakPointer<I>& rhs)
{
	auto rhsp = rhs.lock ();
	return rhsp.get () == lhs;
}

//------------------------------------------------------------------------
template<class I>
struct WeakPointerSupport : IWeakPointerSupport
{
	WeakPointerSupport () {}
	~WeakPointerSupport () noexcept
	{
		std::lock_guard<std::recursive_mutex> lockGuard (m);
		if (weakPointerList)
		{
			std::for_each (weakPointerList->begin (), weakPointerList->end (),
						   [] (auto& wp) { wp->onObjectDestructed (); });
		}
	}

	WeakPointer<I> weakFromThis ()
	{
		return {shared (static_cast<I*> (this))};
	}

private:
	void registerWeakPointer (IWeakPointer* wp) noexcept override
	{
		std::lock_guard<std::recursive_mutex> lockGuard (m);
		if (!weakPointerList)
			weakPointerList = std::make_unique<std::vector<IWeakPointer*>> ();
		weakPointerList->push_back (wp);
	}

	void unregisterWeakPointer (IWeakPointer* wp) noexcept override
	{
		std::lock_guard<std::recursive_mutex> lockGuard (m);
		if (!weakPointerList)
			return;
		auto it = std::find (weakPointerList->begin (), weakPointerList->end (), wp);
		if (it != weakPointerList->end ())
		{
			weakPointerList->erase (it);
			if (weakPointerList->empty ())
				weakPointerList.reset ();
		}
	}

	std::recursive_mutex m;
	std::unique_ptr<std::vector<IWeakPointer*>> weakPointerList;

	friend struct WeakPointer<I>;
	friend struct IWeakPointer;
};

//------------------------------------------------------------------------
template<class I>
template<typename T>
inline WeakPointer<I>::WeakPointer (const SPtr<T>& obj) noexcept : object (obj.get ())
{
	if (object)
		static_cast<IWeakPointerSupport*> (object)->registerWeakPointer (this);
}

//------------------------------------------------------------------------
template<class I>
inline WeakPointer<I>::WeakPointer (const WeakPointer<I>& other) noexcept
{
	if (auto spo = other.lock ())
	{
		object = spo.get ();
		static_cast<IWeakPointerSupport*> (object)->registerWeakPointer (this);
	}
}

//------------------------------------------------------------------------
template<class I>
template<typename T>
inline WeakPointer<I>::WeakPointer (const WeakPointer<T>& other) noexcept
{
	if (auto spo = other.lock ())
	{
		object = spo.get ();
		static_cast<IWeakPointerSupport*> (object)->registerWeakPointer (this);
	}
}

//------------------------------------------------------------------------
template<class I>
inline WeakPointer<I>::WeakPointer (WeakPointer<I>&& other) noexcept
{
	*this = std::move (other);
}

//------------------------------------------------------------------------
template<class I>
inline WeakPointer<I>::~WeakPointer () noexcept
{
	if (object)
		reset ();
}

//------------------------------------------------------------------------
template<class I>
template<typename T>
inline WeakPointer<I>& WeakPointer<I>::operator= (const SPtr<T>& other) noexcept
{
	reset ();
	std::lock_guard<std::recursive_mutex> lockGuard (m);
	if (other)
	{
		object = other.get ();
		object->registerWeakPointer (this);
	}
	return *this;
}

//------------------------------------------------------------------------
template<class I>
inline WeakPointer<I>& WeakPointer<I>::operator= (const WeakPointer<I>& other) noexcept
{
	reset ();
	std::lock_guard<std::recursive_mutex> lockGuard (m);
	if (auto spo = other.lock ())
	{
		object = spo.get ();
		object->registerWeakPointer (this);
	}
	return *this;
}

//------------------------------------------------------------------------
template<class I>
template<typename T>
inline WeakPointer<I>& WeakPointer<I>::operator= (const WeakPointer<T>& other) noexcept
{
	reset ();
	std::lock_guard<std::recursive_mutex> lockGuard (m);
	if (auto spo = other.lock ())
	{
		object = spo.get ();
		object->registerWeakPointer (this);
	}
	return *this;
}

//------------------------------------------------------------------------
template<class I>
inline WeakPointer<I>& WeakPointer<I>::operator= (WeakPointer<I>&& other) noexcept
{
	auto tsp = lock ();
	auto osp = other.lock ();
	if (tsp)
		object->unregisterWeakPointer (this);
	if (osp)
		other.object->unregisterWeakPointer (&other);
	object = other.object;
	other.object = nullptr;
	if (object)
		object->registerWeakPointer (this);
	return *this;
}

//------------------------------------------------------------------------
template<class I>
inline void WeakPointer<I>::swap (WeakPointer<I>& other) noexcept
{
	auto tsp = lock ();
	auto osp = other.lock ();
	if (tsp)
		object->unregisterWeakPointer (this);
	if (osp)
		other.object->unregisterWeakPointer (&other);
	std::swap (object, other.object);
	if (object)
		object->registerWeakPointer (this);
	if (other.object)
		other.object->registerWeakPointer (&other);
}

//------------------------------------------------------------------------
template<class I>
inline SPtr<I> WeakPointer<I>::lock () const noexcept
{
	std::lock_guard<std::recursive_mutex> lockGuard (m);
	if (!expired ())
		return shared (static_cast<I*> (object));
	return {};
}

//------------------------------------------------------------------------
template<class I>
inline void WeakPointer<I>::onObjectDestructed () noexcept
{
	std::lock_guard<std::recursive_mutex> lockGuard (m);
	object = nullptr;
}

//------------------------------------------------------------------------
template<class I>
inline void WeakPointer<I>::reset () noexcept
{
	std::lock_guard<std::recursive_mutex> lockGuard (m);
	if (auto spo = lock ())
	{
		object = nullptr;
		static_cast<IWeakPointerSupport*> (spo.get ())->unregisterWeakPointer (this);
	}
}

//------------------------------------------------------------------------
template<class I>
inline bool WeakPointer<I>::expired () const noexcept
{
	return object == nullptr;
}

//------------------------------------------------------------------------
} // VSTGUI

#endif
