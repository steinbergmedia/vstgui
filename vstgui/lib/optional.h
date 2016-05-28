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

#include <utility>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
/** simplified optional */
template <typename T>
struct Optional
{
	static_assert (std::is_default_constructible<T>::value, "Type T must be default constructible");

	explicit Optional (T&& v);
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
} // VSTGUI