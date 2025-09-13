// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cviewcontainer.h"
#include <optional>
#include <any>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
/** view layout
 *
 *	the view layout holds the information about the position and size of the child views in a view
 *	container as calculated by the view layouter.
 *
 *	@ingroup new_in_4_15
 */
struct ViewLayout
{
	/** the size of the layout */
	CRect size;
	/** opaque layout data */
	std::any data;
};

//-----------------------------------------------------------------------------
/** view layouter interface
 *
 *	a view layouter is used to calculate and set the size and position of the child views in a view
 *	container.
 *
 *	@ingroup new_in_4_15
 */
class IViewLayouter : virtual public IReference
{
public:
	using Children = CViewContainer::ViewList;

	/** calculate the layout of the view.
	 *
	 *	Note that the returned layout may have a different size than the newSize parameter when
	 *	the layout is not possible with the given size.
	 *
	 *	@param view the view to calculate the layout for
	 *	@param children the children of the view
	 *	@param newSize the new size of the view
	 *	@return the calculated layout or std::nullopt if the layout could not be calculated
	 *
	 */
	virtual std::optional<ViewLayout> calculateLayout (const CViewContainer& view,
													   const Children& children,
													   const CRect& newSize) = 0;
	/** apply the previously calculated layout
	 *
	 *	@param view the view to apply the layout to
	 *	@param children the children of the view
	 *	@param layout the layout to apply
	 *	@return true if the layout was applied successfully, false otherwise
	 */
	virtual bool applyLayout (CViewContainer& view, const Children& children,
							  const ViewLayout& layout) = 0;
};

//-----------------------------------------------------------------------------
} // VSTGUI
