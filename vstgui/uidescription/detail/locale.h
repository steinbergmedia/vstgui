// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

namespace VSTGUI {
namespace Detail {

//-----------------------------------------------------------------------------
struct Locale
{
	Locale ()
	{
		origLocal = std::locale ();
		std::locale::global (std::locale::classic ());
	}

	~Locale () noexcept
	{
		std::locale::global (origLocal);
	}

	std::locale origLocal;
};

//------------------------------------------------------------------------
} // Detail
} // VSTGUI
