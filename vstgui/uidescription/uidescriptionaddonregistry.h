// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "iuidescriptionaddon.h"
#include <memory>
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
class UIDescriptionAddOnRegistry
{
public:
	using AddOnID = uint64_t;
	static AddOnID add (std::unique_ptr<IUIDescriptionAddOn>&& addOn);
	static void remove (AddOnID identifier);

	template<typename Proc>
	static void forEach (Proc p)
	{
		for (auto& el : instance ())
		{
			p (*el.addOn.get ());
		}
	}

private:
	static UIDescriptionAddOnRegistry& instance ();

	UIDescriptionAddOnRegistry ();
	~UIDescriptionAddOnRegistry () noexcept;

	struct AddOn
	{
		AddOnID identifier;
		std::unique_ptr<IUIDescriptionAddOn> addOn;
	};
	using iterator = std::vector<AddOn>::iterator;

	iterator begin ();
	iterator end ();

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // VSTGUI
