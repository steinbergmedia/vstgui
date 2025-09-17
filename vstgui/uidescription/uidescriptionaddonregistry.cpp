// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uidescriptionaddonregistry.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
struct UIDescriptionAddOnRegistry::Impl
{
	AddOnID counter {0};
	using AddOns = std::vector<AddOn>;
	AddOns addOns;

	AddOnID addInternal (std::unique_ptr<IUIDescriptionAddOn>&& addOn);
	void removeInternal (AddOnID identifier);
};

//------------------------------------------------------------------------
auto UIDescriptionAddOnRegistry::Impl::addInternal (std::unique_ptr<IUIDescriptionAddOn>&& addOn)
	-> AddOnID
{
	AddOnID result = ++counter;
	addOns.push_back ({result, std::move (addOn)});
	return result;
}

//------------------------------------------------------------------------
void UIDescriptionAddOnRegistry::Impl::removeInternal (AddOnID identifier)
{
	auto it = std::find_if (addOns.begin (), addOns.end (),
							[&] (const auto& el) { return el.identifier == identifier; });
	if (it != addOns.end ())
		addOns.erase (it);
}

//------------------------------------------------------------------------
UIDescriptionAddOnRegistry::UIDescriptionAddOnRegistry () { impl = std::make_unique<Impl> (); }

//------------------------------------------------------------------------
UIDescriptionAddOnRegistry::~UIDescriptionAddOnRegistry () noexcept = default;

//------------------------------------------------------------------------
auto UIDescriptionAddOnRegistry::begin () -> iterator { return impl->addOns.begin (); }

//------------------------------------------------------------------------
auto UIDescriptionAddOnRegistry::end () -> iterator { return impl->addOns.end (); }

//------------------------------------------------------------------------
auto UIDescriptionAddOnRegistry::add (std::unique_ptr<IUIDescriptionAddOn>&& addOn) -> AddOnID
{
	return instance ().impl->addInternal (std::move (addOn));
}

//------------------------------------------------------------------------
void UIDescriptionAddOnRegistry::remove (AddOnID identifier)
{
	instance ().impl->removeInternal (identifier);
}

//------------------------------------------------------------------------
auto UIDescriptionAddOnRegistry::instance () -> UIDescriptionAddOnRegistry&
{
	static UIDescriptionAddOnRegistry obj;
	return obj;
}

//------------------------------------------------------------------------
} // VSTGUI
