// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cclipboard.h"
#include "platform/platformfactory.h"

#include <string_view>

namespace VSTGUI {
namespace CClipboardDetail {

//-----------------------------------------------------------------------------
template<bool AsFile>
struct StringDataPackage : IDataPackage
{
	StringDataPackage (std::string_view str) : str (str) {}

	uint32_t getCount () const final { return 1; }
	uint32_t getDataSize (uint32_t index) const final
	{
		return static_cast<uint32_t> (str.size ());
	}
	Type getDataType (uint32_t index) const final { return AsFile ? Type::kFilePath : Type::kText; }
	uint32_t getData (uint32_t index, const void*& buffer, Type& type) const final
	{
		buffer = str.data ();
		type = getDataType (index);
		return getDataSize (index);
	}

	std::string str;
};

//-----------------------------------------------------------------------------
template<bool AsFile>
Optional<UTF8String> getString (IDataPackage* cb)
{
	for (auto i = 0u, count = cb->getCount (); i < count; i++)
	{
		if (cb->getDataType (i) == (AsFile ? IDataPackage::Type::kFilePath
										   : IDataPackage::Type::kText))
		{
			IDataPackage::Type type;
			const void* data = nullptr;
			auto size = cb->getData (i, data, type);
			if (size > 0)
			{
				return {UTF8String (std::string (static_cast<const char*> (data), size))};
			}
		}
	}
	return {};
}

} // CClipboardDetail

//-----------------------------------------------------------------------------
SharedPointer<IDataPackage> CClipboard::get ()
{
	return getPlatformFactory ().getClipboard ();
}

//-----------------------------------------------------------------------------
bool CClipboard::set (const SharedPointer<IDataPackage>& data)
{
	return getPlatformFactory ().setClipboard (data);
}

//-----------------------------------------------------------------------------
bool CClipboard::setString (UTF8StringPtr str)
{
	return set (makeOwned<CClipboardDetail::StringDataPackage<false>> (
		std::string_view (str, strlen (str))));
}

//-----------------------------------------------------------------------------
bool CClipboard::setFilePath (UTF8StringPtr str)
{
	return set (makeOwned<CClipboardDetail::StringDataPackage<true>> (
		std::string_view (str, strlen (str))));
}

//-----------------------------------------------------------------------------
Optional<UTF8String> CClipboard::getString ()
{
	if (auto cb = get ())
	{
		return CClipboardDetail::getString<false> (cb);
	}
	return {};
}

//-----------------------------------------------------------------------------
Optional<UTF8String> CClipboard::getFilePath ()
{
	if (auto cb = get ())
	{
		return CClipboardDetail::getString<true> (cb);
	}
	return {};
}

} // VSTGUI
