// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __win32dragcontainer__
#define __win32dragcontainer__

#include "../../idatapackage.h"

#if WINDOWS

#include <windows.h>
#include <vector>
#include <string>

namespace VSTGUI {

class WinDragContainer : public IDataPackage
{
public:
	WinDragContainer (IDataObject* platformDrag);
	~WinDragContainer () noexcept;

	uint32_t getCount () const override;
	uint32_t getDataSize (uint32_t index) const override;
	Type getDataType (uint32_t index) const override;
	uint32_t getData (uint32_t index, const void*& buffer, Type& type) const override;

protected:
	static bool checkResolveLink (const TCHAR* nativePath, TCHAR* resolved);
	static FORMATETC formatTEXTDrop;
	static FORMATETC formatHDrop;
	static FORMATETC formatBinaryDrop;

	IDataObject* platformDrag;
	uint32_t nbItems;
	bool stringsAreFiles;
	std::vector<std::string> strings;
	void* data;
	uint32_t dataSize;
};

} // namespace

#endif // WINDOWS

#endif // __win32dragcontainer__