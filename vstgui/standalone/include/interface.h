// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
class Interface
{
public:
	virtual ~Interface () noexcept {}

	Interface () = default;
	Interface (const Interface&) = delete;
	Interface (Interface&&) = delete;
	Interface& operator= (const Interface&) = delete;
	Interface& operator= (Interface&&) = delete;

	template <typename T>
	const auto dynamicCast () const
	{
		return dynamic_cast<const T*> (this);
	}

	template <typename T>
	auto dynamicCast ()
	{
		return dynamic_cast<T*> (this);
	}
};

//------------------------------------------------------------------------
using InterfacePtr = std::shared_ptr<Interface>;

//------------------------------------------------------------------------
template <typename Iface, typename T>
inline auto dynamicPtrCast (std::shared_ptr<T>& obj)
{
	return std::dynamic_pointer_cast<Iface> (obj);
}

//------------------------------------------------------------------------
template <typename Iface, typename T>
inline const auto dynamicPtrCast (const std::shared_ptr<T>& obj)
{
	return std::dynamic_pointer_cast<Iface> (obj);
}

//------------------------------------------------------------------------
template <typename Iface, typename T>
inline auto staticPtrCast (std::shared_ptr<T>& obj)
{
	return std::static_pointer_cast<Iface> (obj);
}

//------------------------------------------------------------------------
template <typename Iface, typename T>
inline const auto staticPtrCast (const std::shared_ptr<T>& obj)
{
	return std::static_pointer_cast<Iface> (obj);
}

//------------------------------------------------------------------------
template <typename Iface, typename T>
inline const auto& asInterface (const T& obj)
{
	return static_cast<const Iface&> (obj);
}

//------------------------------------------------------------------------
} // VSTGUI
