//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

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
