// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __cdropsource__
#define __cdropsource__

#include "vstguibase.h"
#include "idatapackage.h"
#include "malloc.h"
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
	static SharedPointer<IDataPackage> create (const void* buffer, uint32_t bufferSize, Type type);

	CDropSource ();
	CDropSource (const void* buffer, uint32_t bufferSize, Type type);

	bool add (const void* buffer, uint32_t bufferSize, Type type);

	// IDataPackage
	uint32_t getCount () const final;
	uint32_t getDataSize (uint32_t index) const final;
	Type getDataType (uint32_t index) const final;
	uint32_t getData (uint32_t index, const void*& buffer, Type& type) const final;
protected:
	/// @cond ignore
	struct CDropEntry {
		Malloc<int8_t> buffer;
		Type type;
		
		CDropEntry (const void* buffer, uint32_t bufferSize, Type type);
		CDropEntry (const CDropEntry& entry);
		CDropEntry (CDropEntry&& entry) noexcept;
	};
	/// @endcond
	using DropEntryVector = std::vector<CDropEntry>;
	DropEntryVector entries;
};

} // namespace

#endif // __cdropsource__
