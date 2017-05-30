// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <utility>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
/** simplified optional 
 *
 *	@ingroup new_in_4_5
 */
template <typename T>
struct Optional
{
	static_assert (std::is_default_constructible<T>::value, "Type T must be default constructible");

	Optional (T&& v);
	explicit Optional (const T& v);
	Optional ();

	Optional (Optional&&) = default;
	Optional& operator= (Optional&&) = default;
	Optional (const Optional&) = delete;
	Optional& operator= (const Optional&) = delete;

	explicit operator bool () const;

	const T* operator-> () const;
	T* operator-> ();

	const T& operator* () const&;
	T& operator* () &;

	T&& value ();
	const T& value () const;

	void reset ();
private:
	std::pair<bool, T> _value;
};

//------------------------------------------------------------------------
template <typename T>
inline Optional<typename std::decay<T>::type> makeOptional (T&& value)
{
	return Optional<typename std::decay<T>::type> (std::forward<T> (value));
}

//------------------------------------------------------------------------
template <typename T>
inline Optional<T>::Optional (T&& v)
: _value {true, std::move (v)}
{
}

//------------------------------------------------------------------------
template <typename T>
inline Optional<T>::Optional (const T& v)
: _value {true, v}
{
}

//------------------------------------------------------------------------
template <typename T>
inline Optional<T>::Optional ()
{
	_value.first = false;
}

//------------------------------------------------------------------------
template <typename T>
inline Optional<T>::operator bool () const
{
	return _value.first;
}

//------------------------------------------------------------------------
template <typename T>
inline const T* Optional<T>::operator-> () const
{
	return _value.second;
}

//------------------------------------------------------------------------
template <typename T>
inline T* Optional<T>::operator-> ()
{
	return &_value.second;
}

//------------------------------------------------------------------------
template <typename T>
inline const T& Optional<T>::operator* () const&
{
	return _value.second;
}

//------------------------------------------------------------------------
template <typename T>
inline T& Optional<T>::operator* () &
{
	return _value.second;
}

//------------------------------------------------------------------------
template <typename T>
inline T&& Optional<T>::value ()
{
	assert (_value.first);
	return std::move (_value.second);
}

//------------------------------------------------------------------------
template <typename T>
inline const T& Optional<T>::value () const
{
	assert (_value.first);
	return _value.second;
}

//------------------------------------------------------------------------
template <typename T>
inline void Optional<T>::reset ()
{
	_value.first = false;
	_value.second = {};
}

//------------------------------------------------------------------------
} // VSTGUI
