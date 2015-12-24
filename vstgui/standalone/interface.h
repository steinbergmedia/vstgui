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
};

//------------------------------------------------------------------------
} // VSTGUI
