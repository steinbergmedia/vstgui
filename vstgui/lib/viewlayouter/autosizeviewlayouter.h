// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "baseviewlayouter.h"
#include <tuple>
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
/** autosize view layouter
 *
 *	a view layouter that uses the autosize flags of the views to determine the size and position
 *	of the child views in a view container.
 *
 *	@ingroup new_in_4_15
 */
struct AutoSizeViewLayouter final : BaseViewLayouter
{
private:
	AutoSizeViewLayouter () = default;

	std::optional<ViewLayout> calculateLayout (const CViewContainer& container,
											   const Children& children,
											   const CRect& newSize) override;

	void forget () final;
	void remember () final;

public:
	/** get the shared instance of the AutoSizeViewLayouter */
	static SharedPointer<IViewLayouter> get () noexcept;
};

//------------------------------------------------------------------------
} // VSTGUI
