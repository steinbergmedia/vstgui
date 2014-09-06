//
//  cresourcedescription.h
//  vstgui
//
//  Created by Arne Scheffler on 06/09/14.
//
//

#ifndef __cresourcedescription__
#define __cresourcedescription__

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

	CResourceDescription () : type (kUnknownType) { u.name = 0; }
	CResourceDescription (int32_t id) : type (kIntegerType) { u.id = id; }
	CResourceDescription (UTF8StringPtr name) : type (kStringType) { u.name = name; }

	CResourceDescription& operator= (int32_t id) { u.id = id; type = kIntegerType; return *this; }
	CResourceDescription& operator= (const CResourceDescription& desc) { type = desc.type; u.id = desc.u.id; u.name = desc.u.name; return *this; }

	int32_t type;
	union {
		int32_t id;
		UTF8StringPtr name;
	} u;
};

}

#endif // __cresourcedescription__
