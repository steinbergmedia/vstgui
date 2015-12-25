#pragma once

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
class Interface
{
public:
	virtual ~Interface () {}
	
	Interface () = default;
	Interface (const Interface&) = delete;
	Interface& operator=(const Interface&) = delete;

	template<typename T>
	const T* dynamicCast () const
	{
		return dynamic_cast<T*> (this);
	}

	template<typename T>
	T* dynamicCast ()
	{
		return dynamic_cast<T*> (this);
	}
};

//------------------------------------------------------------------------
} // VSTGUI
