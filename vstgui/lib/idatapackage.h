// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __idatapackage__
#define __idatapackage__

#include "vstguibase.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
///	@brief interface for drag&drop and clipboard data
//-----------------------------------------------------------------------------
class IDataPackage : public AtomicReferenceCounted
{
public:
	enum Type {
		kFilePath = 0,	///< File type (UTF-8 C-String)
		kText,			///< Text type (UTF-8 C-String)
		kBinary,		///< Binary type

		kError = -1
	};

	virtual uint32_t getCount () const = 0;
	virtual uint32_t getDataSize (uint32_t index) const = 0;
	virtual Type getDataType (uint32_t index) const = 0;
	virtual uint32_t getData (uint32_t index, const void*& buffer, Type& type) const = 0;

protected:
	IDataPackage () {}
};

}

#endif // __idatapackage__
