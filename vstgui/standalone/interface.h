#pragma once

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
	const T* dynamicCast () const
	{
		return dynamic_cast<const T*> (this);
	}

	template <typename T>
	T* dynamicCast ()
	{
		return dynamic_cast<T*> (this);
	}

};

//------------------------------------------------------------------------
template<typename Iface, typename T>
inline const Iface& asInterface (T* obj) { return *static_cast<Iface*> (obj); }

//------------------------------------------------------------------------
} // VSTGUI
