// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../lib/vstguibase.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
class IContentProvider
{
public:
	virtual uint32_t readRawData (int8_t* buffer, uint32_t size) = 0;
	virtual void rewind () = 0;

	virtual ~IContentProvider () noexcept = default;
};

//------------------------------------------------------------------------
} // VSTGUI
