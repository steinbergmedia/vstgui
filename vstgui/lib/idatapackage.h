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

//-----------------------------------------------------------------------------
/** IDataPackage iterator
 *
 *	@ingroup new_in_4_12
 */
struct DataPackageIterator
{
	struct Item
	{
		IDataPackage::Type type;
		uint32_t dataSize {0};
		const void* data {nullptr};
	};

	DataPackageIterator (IDataPackage* pkg, uint32_t index = 0u) : pkg (pkg), index (index) {}

	DataPackageIterator& operator++ ()
	{
		item = {};
		index++;
		return *this;
	}

	const Item& operator->() const
	{
		gatherItem ();
		return item;
	}

	const Item& operator* () const
	{
		gatherItem ();
		return item;
	}

	bool operator!= (const DataPackageIterator& other) const
	{
		return other.pkg != pkg || other.index != index;
	}

	uint32_t getIndex () const { return index; }

private:
	void gatherItem () const
	{
		if (item.data == nullptr && pkg && index < pkg->getCount ())
			item.dataSize = pkg->getData (index, item.data, item.type);
	}
	IDataPackage* pkg {nullptr};
	uint32_t index {0u};
	mutable Item item;
};

//-----------------------------------------------------------------------------
inline DataPackageIterator begin (IDataPackage* pkg) { return DataPackageIterator (pkg); }

//-----------------------------------------------------------------------------
inline DataPackageIterator end (IDataPackage* pkg)
{
	return DataPackageIterator (pkg, pkg->getCount () + 1);
}

} // VSTGUI
