// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "baseviewlayouter.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
/** A view layouter that does not layout any views.
 *
 *	@ingroup new_in_4_15
 */
struct NoViewLayouter final : BaseViewLayouter,
							  NonAtomicReferenceCounted
{
	NoViewLayouter () = default;

	std::optional<ViewLayout> calculateLayout (const CViewContainer& container,
											   const Children& children,
											   const CRect& newSize) override
	{
		return {ViewLayout {newSize, LayoutData {}}};
	}
};

//------------------------------------------------------------------------
} // VSTGUI
