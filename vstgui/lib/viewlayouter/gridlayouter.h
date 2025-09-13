// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iviewlayouter.h"
#include <vector>
#include <variant>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
/** Properties for configuring a grid layout, compatible with CSS Grid Layout.
 *
 *  The GridProperties struct allows you to define a grid layout similar to CSS Grid.
 *
 *  - rows: Number of grid rows (default: 1).
 *  - columns: Number of grid columns (default: 1).
 *  - autoRows: Defines the height of each row, similar to 'grid-template-rows' in CSS.
 *  - autoColumns: Defines the width of each column, similar to 'grid-template-columns' in CSS.
 *  - rowGap: Space between rows, equivalent to 'row-gap' in CSS.
 *  - columnGap: Space between columns, equivalent to 'column-gap' in CSS.
 *  - alignItems: Controls alignment of items within their grid area (CSS: 'align-items').
 *      Options: Start, Center, End, Stretch.
 *  - justifyItems: Controls horizontal alignment of items within their grid area (CSS:
 *      'justify-items'). Options: Start, Center, End, Stretch.
 *  - alignContent: Controls vertical alignment of the grid as a whole (CSS: 'align-content').
 *      Options: Start, Center, End, Stretch, SpaceBetween, SpaceAround.
 *  - justifyContent: Controls horizontal alignment of the grid as a whole (CSS: 'justify-content').
 *      Options: Start, Center, End, Stretch, SpaceBetween, SpaceAround.
 *  - gridAreas: Optional. Defines the grid area (row, column, rowSpan, colSpan) for each
 *                   child/view. If provided, each child can span multiple rows/columns, similar to
 *                   CSS grid-area. If not provided, each child is placed in a single cell by
 *                   default.
 *
 *    Note: justify-content is ignored when grid areas are used and the grid tracks exactly fit the
 *        content (no extra free space). If grid areas are used and the grid is smaller than the
 *        container, justify-content can affect the grid's position within the container.
 *
 *  This struct is designed to be compatible with the CSS Grid Layout specification:
 *  https://developer.mozilla.org/en-US/docs/Web/CSS/CSS_grid_layout
 *
 *	@ingroup new_in_4_15
 */
struct GridLayoutProperties
{
	enum class AlignItems
	{
		Start,
		Center,
		End,
		Stretch
	};
	enum class JustifyItems
	{
		Start,
		Center,
		End,
		Stretch
	};
	enum class AlignContent
	{
		Start,
		Center,
		End,
		Stretch,
		SpaceBetween,
		SpaceAround
	};
	enum class JustifyContent
	{
		Start,
		Center,
		End,
		Stretch,
		SpaceBetween,
		SpaceAround
	};
	struct GridArea
	{
		// 0-based indices
		size_t row {0};
		size_t column {0};
		size_t rowSpan {1};
		size_t colSpan {1};
	};
	struct Percentage
	{
		double value {0.0}; // 0.0 - 100.0
	};
	struct Auto
	{
	};
	using SizeSpec = std::variant<Auto, CCoord, Percentage>;

	size_t rows {1};
	size_t columns {1};
	std::vector<SizeSpec> autoRows;
	std::vector<SizeSpec> autoColumns;
	CCoord rowGap {0.0};
	CCoord columnGap {0.0};
	AlignItems alignItems {AlignItems::Stretch};
	JustifyItems justifyItems {JustifyItems::Stretch};
	AlignContent alignContent {AlignContent::Stretch};
	JustifyContent justifyContent {JustifyContent::Stretch};

	std::vector<GridArea> gridAreas; // Optional: one per child, defines grid area (span)
};

//------------------------------------------------------------------------
/** grid view layouter
 *
 *	@ingroup new_in_4_15
 */
class GridLayouter : public IViewLayouter,
					 public NonAtomicReferenceCounted
{
public:
	using LayoutData = std::vector<std::pair<CRect, std::optional<ViewLayout>>>;

	GridLayouter (const GridLayoutProperties& props = {});

	void setProperties (const GridLayoutProperties& props);
	const GridLayoutProperties& getProperties () const;
	GridLayoutProperties& getProperties ();

	bool applyLayout (CViewContainer& container, const Children& children,
					  const ViewLayout& layout) override;
	std::optional<ViewLayout> calculateLayout (const CViewContainer& view, const Children& children,
											   const CRect& newSize) override;

private:
	static constexpr auto kAutoSize = std::numeric_limits<CCoord>::min ();

	std::vector<CCoord> calculateAutoRows (CCoord& usedRowHeight, CCoord availableHeight,
										   size_t& outNumAutoRows) const;
	std::vector<CCoord> calculateAutoColumns (CCoord& usedColWidth, CCoord availableWidth,
											  size_t& outNumAutoCols) const;

	GridLayoutProperties gridProps;
};

//------------------------------------------------------------------------
} // namespace VSTGUI
