//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins :
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __cdropsource__
#define __cdropsource__

#include "vstguibase.h"
#include "idatapackage.h"
#include <vector>

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CDropSource Declaration
//! @brief drop source
//!
//! @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class CDropSource : public IDataPackage
{
public:
	CDropSource ();
	CDropSource (const void* buffer, int32_t bufferSize, Type type);
	~CDropSource ();

	bool add (const void* buffer, int32_t bufferSize, Type type);

	// IDataPackage
	virtual int32_t getCount () VSTGUI_OVERRIDE_VMETHOD;
	virtual int32_t getDataSize (int32_t index) VSTGUI_OVERRIDE_VMETHOD { return getEntrySize (index); }
	virtual Type getDataType (int32_t index) VSTGUI_OVERRIDE_VMETHOD { return getEntryType (index); }
	virtual int32_t getData (int32_t index, const void*& buffer, Type& type) VSTGUI_OVERRIDE_VMETHOD { return getEntry (index, buffer, type); }

	// old interface
	int32_t getEntrySize (int32_t index); ///< \deprecated
	Type getEntryType (int32_t index); ///< \deprecated
	int32_t getEntry (int32_t index, const void*& buffer, Type& type); ///< \deprecated

	//-------------------------------------------
	CLASS_METHODS_NOCOPY(CDropSource, IDataPackage)
protected:
	/// @cond ignore
	struct CDropEntry {
		void* buffer;
		int32_t bufferSize;
		Type type;
	};
	/// @endcond
	std::vector<CDropEntry*> entries;
};

} // namespace

#endif // __cdropsource__
