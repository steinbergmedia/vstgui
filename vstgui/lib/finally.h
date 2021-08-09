// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
template <typename Proc>
class FinalAction
{
public:
	explicit FinalAction (Proc action) noexcept : action (std::move (action)) {}
	FinalAction (FinalAction&& other) noexcept
	{
		action = std::move (other.action);
		other.invoke (false);
	}
	FinalAction (const FinalAction&) = delete;
	FinalAction& operator= (const FinalAction&) = delete;
	FinalAction& operator= (FinalAction&&) = delete;

	~FinalAction () noexcept
	{
		if (invoke)
			action ();
	}

private:
	Proc action;
	bool invoke {true};
};

//------------------------------------------------------------------------
template <class F>
auto finally (F&& f) noexcept
{
	return FinalAction<typename std::remove_cv<typename std::remove_reference<F>::type>::type> (
	    std::forward<F> (f));
}
//------------------------------------------------------------------------
} // VSTGUI
