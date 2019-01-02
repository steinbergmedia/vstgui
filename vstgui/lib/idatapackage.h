// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguibase.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
///	@brief interface for drag&drop and clipboard data
//-----------------------------------------------------------------------------
class IDataPackage : public AtomicReferenceCounted
{
public:
	enum Type {
		/** File type (UTF-8 C-String) */
		kFilePath = 0,
		/** Text type (UTF-8 C-String) */
		kText,
		/** Binary type */
		kBinary,

		kError = -1
	};

	virtual uint32_t getCount () const = 0;
	virtual uint32_t getDataSize (uint32_t index) const = 0;
	virtual Type getDataType (uint32_t index) const = 0;
	virtual uint32_t getData (uint32_t index, const void*& buffer, Type& type) const = 0;

protected:
	IDataPackage () {}
};

} // VSTGUI
