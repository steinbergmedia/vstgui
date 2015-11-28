//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins :
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
	CDropSource (const void* buffer, uint32_t bufferSize, Type type);

	bool add (const void* buffer, uint32_t bufferSize, Type type);

	// IDataPackage
	virtual uint32_t getCount () const VSTGUI_FINAL_VMETHOD;
	virtual uint32_t getDataSize (uint32_t index) const VSTGUI_FINAL_VMETHOD;
	virtual Type getDataType (uint32_t index) const VSTGUI_FINAL_VMETHOD;
	virtual uint32_t getData (uint32_t index, const void*& buffer, Type& type) const VSTGUI_FINAL_VMETHOD;

	// old interface
	VSTGUI_DEPRECATED(int32_t getEntrySize (int32_t index) const;) ///< \deprecated
	VSTGUI_DEPRECATED(Type getEntryType (int32_t index) const;) ///< \deprecated
	VSTGUI_DEPRECATED(int32_t getEntry (int32_t index, const void*& buffer, Type& type) const;) ///< \deprecated

	//-------------------------------------------
	CLASS_METHODS_NOCOPY(CDropSource, IDataPackage)
protected:
	/// @cond ignore
	struct CDropEntry {
		void* buffer;
		uint32_t bufferSize;
		Type type;
		
		CDropEntry (const void* buffer, uint32_t bufferSize, Type type);
		CDropEntry (const CDropEntry& entry);
	#if VSTGUI_RVALUE_REF_SUPPORT
		CDropEntry (CDropEntry&& entry) noexcept;
	#endif
		~CDropEntry ();
	};
	/// @endcond
	typedef std::vector<CDropEntry> DropEntryVector;
	DropEntryVector entries;
};

} // namespace

#endif // __cdropsource__
