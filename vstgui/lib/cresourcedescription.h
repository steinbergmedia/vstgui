// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguibase.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CResourceDescription Declaration
//! @brief Describes a resource by name or by ID
//-----------------------------------------------------------------------------
class CResourceDescription
{
public:
	enum { kIntegerType, kStringType, kUnknownType };

	CResourceDescription () = default;
	CResourceDescription (UTF8StringPtr name) : type (kStringType) { u.name = name; }
	explicit CResourceDescription (int32_t id) : type (kIntegerType) { u.id = id; }

	CResourceDescription& operator= (int32_t id) { u.id = id; type = kIntegerType; return *this; }
	CResourceDescription& operator= (const CResourceDescription& desc) { type = desc.type; u.id = desc.u.id; u.name = desc.u.name; return *this; }

	int32_t type {kUnknownType};
	union {
		int32_t id;
		UTF8StringPtr name {nullptr};
	} u;
};

} // VSTGUI
