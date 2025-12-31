// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguibase.h"
#include <algorithm>
#include <mutex>
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {

template<class I>
struct WeakPointerSupport;

//------------------------------------------------------------------------
/** A non-owning, thread-safe weak reference wrapper for objects managed by VSTGUI's SharedPointer.

WeakPointer allows you to reference an object without extending its lifetime, preventing reference
cycles and enabling you to safely check whether the object still exists. It cooperates with
WeakPointerSupport embedded in the pointee type to be notified when the object is destroyed.

Key characteristics:
- Does not increase the reference count of the target object.
- Thread-safe access and state transitions via internal mutex protection.
- Can be constructed from either a SharedPointer<I> or another WeakPointer<I>.
- Provides lock() to obtain a temporary SharedPointer<I> if the object is still alive.
- Automatically invalidates itself when the target object is destroyed (expired()).

Usage:
- Hold WeakPointer<I> where you need a non-owning reference to an object managed by
  SharedPointer<I>.
- Call lock() to get a SharedPointer<I> before accessing the object; check for null to handle
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
- Construction from SharedPointer<I> registers the WeakPointer with the target’s
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
- Always check the result of lock() before dereferencing; it may return an empty SharedPointer
  if expired.
- expired() is a fast check, but lock() is the canonical way to access the object safely.
- Avoid long-lived locks or heavy work while holding the returned SharedPointer to minimize
  contention.
 */
template<class I>
struct WeakPointer final
{
	inline WeakPointer () noexcept {}
	inline WeakPointer (const SharedPointer<I>& object) noexcept;
	inline WeakPointer (const WeakPointer<I>& object) noexcept;
	inline WeakPointer (WeakPointer<I>&& other) noexcept;
	inline ~WeakPointer () noexcept;

	inline WeakPointer<I>& operator= (const SharedPointer<I>& other) noexcept;
	inline WeakPointer<I>& operator= (const WeakPointer<I>& other) noexcept;
	inline WeakPointer<I>& operator= (WeakPointer<I>&& other) noexcept;

	inline SharedPointer<I> lock () const noexcept;
	inline void reset () noexcept;
	inline void swap (WeakPointer<I>& other) noexcept;
	inline bool expired () const noexcept;

protected:
	friend struct WeakPointerSupport<I>;

	inline void onObjectDestructed () noexcept;

	mutable std::mutex m;
	WeakPointerSupport<I>* object {nullptr};
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
struct WeakPointerSupport
{
	WeakPointerSupport () {}
	~WeakPointerSupport () noexcept
	{
		std::lock_guard<std::mutex> lockGuard (m);
		if (weakPointerList)
		{
			std::for_each (weakPointerList->begin (), weakPointerList->end (),
						   [] (auto& wp) { wp->onObjectDestructed (); });
		}
	}

protected:
	WeakPointer<I> weakFromThis ()
	{
		return {shared (static_cast<I*> (this))};
	}

	void registerWeakPointer (WeakPointer<I>* wp) noexcept
	{
		std::lock_guard<std::mutex> lockGuard (m);
		if (!weakPointerList)
			weakPointerList = std::make_unique<std::vector<WeakPointer<I>*>> ();
		weakPointerList->push_back (wp);
	}

	void unregisterWeakPointer (WeakPointer<I>* wp) noexcept
	{
		std::lock_guard<std::mutex> lockGuard (m);
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

private:
	std::mutex m;
	std::unique_ptr<std::vector<WeakPointer<I>*>> weakPointerList;

	friend struct WeakPointer<I>;
};

//------------------------------------------------------------------------
template<class I>
inline WeakPointer<I>::WeakPointer (const SharedPointer<I>& object) noexcept
: object (object.get ())
{
	if (object)
		object->registerWeakPointer (this);
}

//------------------------------------------------------------------------
template<class I>
inline WeakPointer<I>::WeakPointer (const WeakPointer<I>& other) noexcept
{
	if (auto spo = other.lock ())
	{
		object = spo.get ();
		object->registerWeakPointer (this);
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
inline WeakPointer<I>& WeakPointer<I>::operator= (const SharedPointer<I>& other) noexcept
{
	reset ();
	std::lock_guard<std::mutex> lockGuard (m);
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
	std::lock_guard<std::mutex> lockGuard (m);
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
inline SharedPointer<I> WeakPointer<I>::lock () const noexcept
{
	std::lock_guard<std::mutex> lockGuard (m);
	if (!expired ())
		return shared (static_cast<I*> (object));
	return {};
}

//------------------------------------------------------------------------
template<class I>
inline void WeakPointer<I>::onObjectDestructed () noexcept
{
	std::lock_guard<std::mutex> lockGuard (m);
	object = nullptr;
}

//------------------------------------------------------------------------
template<class I>
inline void WeakPointer<I>::reset () noexcept
{
	if (auto spo = lock ())
	{
		object = nullptr;
		spo->unregisterWeakPointer (this);
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
