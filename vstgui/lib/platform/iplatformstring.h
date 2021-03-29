// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguibase.h"

/// @cond ignore

namespace VSTGUI {

//-----------------------------------------------------------------------------
class IPlatformString : public AtomicReferenceCounted
{
public:
	virtual void setUTF8String (UTF8StringPtr utf8String) = 0;
};

} // VSTGUI

/// @endcond
