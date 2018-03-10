// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../idatapackage.h"

#if WINDOWS

#include <windows.h>
#include <objidl.h>
#include <vector>
#include <string>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class Win32DataPackage : public IDataPackage
{
public:
	Win32DataPackage (::IDataObject* platformDataObject);
	~Win32DataPackage () noexcept;

	uint32_t getCount () const override;
	uint32_t getDataSize (uint32_t index) const override;
	Type getDataType (uint32_t index) const override;
	uint32_t getData (uint32_t index, const void*& buffer, Type& type) const override;

//-----------------------------------------------------------------------------
protected:
	static bool checkResolveLink (const TCHAR* nativePath, TCHAR* resolved);
	static FORMATETC formatTEXTDrop;
	static FORMATETC formatHDrop;
	static FORMATETC formatBinaryDrop;

	::IDataObject* platformDataObject;
	uint32_t nbItems;
	bool stringsAreFiles;
	std::vector<std::string> strings;
	void* data;
	uint32_t dataSize;
};

} // namespace

#endif // WINDOWS
