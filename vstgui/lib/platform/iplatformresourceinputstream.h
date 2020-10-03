// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguifwd.h"
#include <functional>
#include <memory>

//-----------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
class IPlatformResourceInputStream
{
public:
	virtual ~IPlatformResourceInputStream () noexcept = default;

	virtual uint32_t readRaw (void* buffer, uint32_t size) = 0;
	virtual int64_t seek (int64_t pos, SeekMode mode) = 0;
	virtual int64_t tell () = 0;
};

//-----------------------------------------------------------------------------
} // VSTGUI
