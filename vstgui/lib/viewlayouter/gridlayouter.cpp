// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "gridlayouter.h"
#include "../cviewcontainer.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
GridLayouter::GridLayouter (const GridLayoutProperties& props) : gridProps (props) {}

//------------------------------------------------------------------------
void GridLayouter::setProperties (const GridLayoutProperties& props) { gridProps = props; }

//------------------------------------------------------------------------
const GridLayoutProperties& GridLayouter::getProperties () const { return gridProps; }

//------------------------------------------------------------------------
GridLayoutProperties& GridLayouter::getProperties () { return gridProps; }

//------------------------------------------------------------------------
std::vector<CCoord> GridLayouter::calculateAutoRows (CCoord& usedRowHeight, CCoord availableHeight,
													 size_t& outNumAutoRows) const
{
	std::vector<CCoord> autoRows;
	if (!gridProps.autoRows.empty ())
	{
		for (const auto& el : gridProps.autoRows)
		{
			if (std::holds_alternative<CCoord> (el))
			{
				autoRows.push_back (std::get<CCoord> (el));
				usedRowHeight += autoRows.back ();
			}
			else if (std::holds_alternative<GridLayoutProperties::Percentage> (el))
			{
				autoRows.push_back (
					availableHeight *
					(std::get<GridLayoutProperties::Percentage> (el).value / 100.0));
				usedRowHeight += autoRows.back ();
			}
			else if (std::holds_alternative<GridLayoutProperties::Auto> (el))
			{
				autoRows.push_back (kAutoSize);
				++outNumAutoRows;
			}
		}
	}
	return autoRows;
}

//------------------------------------------------------------------------
std::vector<CCoord> GridLayouter::calculateAutoColumns (CCoord& usedColWidth, CCoord availableWidth,
														size_t& outNumAutoCols) const
{
	std::vector<CCoord> autoColumns;
	if (!gridProps.autoColumns.empty ())
	{
		for (const auto& el : gridProps.autoColumns)
		{
			if (std::holds_alternative<CCoord> (el))
			{
				autoColumns.push_back (std::get<CCoord> (el));
				usedColWidth += autoColumns.back ();
			}
			else if (std::holds_alternative<GridLayoutProperties::Percentage> (el))
			{
				autoColumns.push_back (
					availableWidth *
					(std::get<GridLayoutProperties::Percentage> (el).value / 100.0));
				usedColWidth += autoColumns.back ();
			}
			else if (std::holds_alternative<GridLayoutProperties::Auto> (el))
			{
				autoColumns.push_back (kAutoSize);
				++outNumAutoCols;
			}
		}
	}
	return autoColumns;
}

//------------------------------------------------------------------------
std::optional<ViewLayout> GridLayouter::calculateLayout (const CViewContainer& /*view*/,
														 const Children& children,
														 const CRect& newSize)
{
	if (children.empty ())
		return {{newSize, LayoutData {}}};

	size_t rows = gridProps.rows;
	size_t cols = gridProps.columns;
	CCoord rowGap = gridProps.rowGap;
	CCoord colGap = gridProps.columnGap;
	if (rows == 0 || cols == 0)
	{
		return std::nullopt;
	}

	if (gridProps.gridAreas.empty () == false && children.size () != gridProps.gridAreas.size ())
	{
		vstgui_assert (false, "GridLayouter: number of gridAreas must match number of children");
		return std::nullopt;
	}

	std::vector<CCoord> rowHeights (rows, 0.0);
	std::vector<CCoord> colWidths (cols, 0.0);
	CCoord totalRowGap = (rows - 1) * rowGap;
	CCoord totalColGap = (cols - 1) * colGap;
	CCoord availableHeight = newSize.getHeight () - totalRowGap;
	CCoord availableWidth = newSize.getWidth () - totalColGap;
	CCoord usedRowHeight = 0.0;
	size_t numAutoRows = 0u;
	CCoord usedColWidth = 0.0;
	size_t numAutoCols = 0u;

	std::vector<CCoord> autoRows = calculateAutoRows (usedRowHeight, availableHeight, numAutoRows);
	std::vector<CCoord> autoColumns =
		calculateAutoColumns (usedColWidth, availableWidth, numAutoCols);

	CCoord sumAutoRows = 0.0;
	for (size_t r = 0; r < rows; ++r)
	{
		if (r < autoRows.size ())
		{
			if (autoRows[r] == kAutoSize)
				autoRows[r] = (availableHeight - usedRowHeight) / numAutoRows;
			rowHeights[r] = autoRows[r];
			sumAutoRows += rowHeights[r];
		}
	}
	CCoord defaultRowHeight = (rows > autoRows.size ())
								  ? (availableHeight - sumAutoRows) / (rows - autoRows.size ())
								  : 0.0;
	for (size_t r = 0; r < rows; ++r)
	{
		if (r >= autoRows.size ())
			rowHeights[r] = defaultRowHeight;
	}

	CCoord sumAutoCols = 0.0;
	for (size_t c = 0; c < cols; ++c)
	{
		if (c < autoColumns.size ())
		{
			if (autoColumns[c] == kAutoSize)
				autoColumns[c] = (availableWidth - usedColWidth) / numAutoCols;
			colWidths[c] = autoColumns[c];
			sumAutoCols += colWidths[c];
		}
	}
	CCoord defaultColWidth = (cols > autoColumns.size ())
								 ? (availableWidth - sumAutoCols) / (cols - autoColumns.size ())
								 : 0.0;
	for (size_t c = 0; c < cols; ++c)
	{
		if (c >= autoColumns.size ())
			colWidths[c] = defaultColWidth;
	}

	CCoord gridHeight = sumAutoRows + defaultRowHeight * (rows - autoRows.size ()) + totalRowGap;
	CCoord gridWidth = sumAutoCols + defaultColWidth * (cols - autoColumns.size ()) + totalColGap;
	CCoord offsetY = 0.0;
	CCoord offsetX = 0.0;
	CCoord extraSpaceY = newSize.getHeight () - gridHeight;
	CCoord extraSpaceX = newSize.getWidth () - gridWidth;
	CCoord spaceBetweenY = 0.0;
	CCoord spaceAroundY = 0.0;
	CCoord spaceBetweenX = 0.0;
	CCoord spaceAroundX = 0.0;
	switch (gridProps.alignContent)
	{
		case GridLayoutProperties::AlignContent::Center:
		{
			offsetY = extraSpaceY / 2.0;
			break;
		}
		case GridLayoutProperties::AlignContent::End:
		{
			offsetY = extraSpaceY;
			break;
		}
		case GridLayoutProperties::AlignContent::Stretch:
		{
			if (rows > 0 && extraSpaceY > 0.0)
			{
				CCoord add = extraSpaceY / rows;
				for (size_t r = 0; r < rows; ++r)
					rowHeights[r] += add;
			}
			offsetY = 0.0;
			break;
		}
		case GridLayoutProperties::AlignContent::SpaceBetween:
		{
			spaceBetweenY = (rows > 1) ? (extraSpaceY / (rows - 1)) : 0.0;
			offsetY = 0.0;
			break;
		}
		case GridLayoutProperties::AlignContent::SpaceAround:
		{
			spaceAroundY = (rows > 0) ? (extraSpaceY / (rows + 1)) : 0.0;
			offsetY = spaceAroundY;
			break;
		}
		default:
		{
			offsetY = 0.0;
			break;
		}
	}
	switch (gridProps.justifyContent)
	{
		case GridLayoutProperties::JustifyContent::Center:
		{
			offsetX = (newSize.getWidth () - gridWidth) / 2.0;
			break;
		}
		case GridLayoutProperties::JustifyContent::End:
		{
			offsetX = newSize.getWidth () - gridWidth;
			break;
		}
		case GridLayoutProperties::JustifyContent::Stretch:
		{
			if (cols > 0 && extraSpaceX > 0.0)
			{
				CCoord add = extraSpaceX / cols;
				for (size_t c = 0; c < cols; ++c)
					colWidths[c] += add;
			}
			offsetX = 0.0;
			break;
		}
		case GridLayoutProperties::JustifyContent::SpaceBetween:
		{
			spaceBetweenX = (cols > 1) ? (extraSpaceX / (cols - 1)) : 0.0;
			offsetX = 0.0;
			break;
		}
		case GridLayoutProperties::JustifyContent::SpaceAround:
		{
			spaceAroundX = (cols > 0) ? (extraSpaceX / (cols + 1)) : 0.0;
			offsetX = spaceAroundX;
			break;
		}
		default:
		{
			offsetX = 0.0;
			break;
		}
	}

	LayoutData layout;
	layout.reserve (children.size ());
	auto child = children.begin ();

	if (!gridProps.gridAreas.empty ())
	{
		for (size_t childIdx = 0; childIdx < children.size (); ++childIdx, ++child)
		{
			const auto& area = gridProps.gridAreas[childIdx];

			size_t r0 = std::min (area.row, rows - 1);
			size_t c0 = std::min (area.column, cols - 1);
			size_t r1 = std::min (r0 + area.rowSpan, rows);
			size_t c1 = std::min (c0 + area.colSpan, cols);

			CCoord y = newSize.top + offsetY;
			for (size_t r = 0; r < r0; ++r)
			{
				y += rowHeights[r] + rowGap;
				if (r < rows - 1)
					y += spaceBetweenY + spaceAroundY;
			}
			CCoord x = newSize.left + offsetX;
			for (size_t c = 0; c < c0; ++c)
			{
				x += colWidths[c] + colGap;
				if (c < cols - 1)
					x += spaceBetweenX + spaceAroundX;
			}
			CCoord h = 0.0;
			for (size_t r = r0; r < r1; ++r)
				h += rowHeights[r];
			h += (r1 - r0 - 1) * (rowGap + spaceAroundY);
			CCoord w = 0.0;
			for (size_t c = c0; c < c1; ++c)
				w += colWidths[c];
			w += (c1 - c0 - 1) * (colGap + spaceAroundX);

			CCoord itemX = x;
			CCoord itemY = y;
			CCoord itemWidth = w;
			CCoord itemHeight = h;
			// Determine intrinsic size when not stretching
			CCoord intrinsicW = w;
			CCoord intrinsicH = h;
			if (auto view = child->get (); view)
			{
				CRect vs = view->getViewSize ();
				intrinsicW = std::max<CCoord> (0.0, std::min (vs.getWidth (), w));
				intrinsicH = std::max<CCoord> (0.0, std::min (vs.getHeight (), h));
			}

			// Horizontal sizing and positioning
			switch (gridProps.justifyItems)
			{
				case GridLayoutProperties::JustifyItems::Stretch:
				{
					itemWidth = w;
					itemX = x;
					break;
				}
				case GridLayoutProperties::JustifyItems::Center:
				{
					itemWidth = intrinsicW;
					itemX = x + (w - itemWidth) / 2.0;
					break;
				}
				case GridLayoutProperties::JustifyItems::End:
				{
					itemWidth = intrinsicW;
					itemX = x + (w - itemWidth);
					break;
				}
				case GridLayoutProperties::JustifyItems::Start:
				default:
				{
					itemWidth = intrinsicW;
					itemX = x;
					break;
				}
			}
			// Vertical sizing and positioning
			switch (gridProps.alignItems)
			{
				case GridLayoutProperties::AlignItems::Stretch:
				{
					itemHeight = h;
					itemY = y;
					break;
				}
				case GridLayoutProperties::AlignItems::Center:
				{
					itemHeight = intrinsicH;
					itemY = y + (h - itemHeight) / 2.0;
					break;
				}
				case GridLayoutProperties::AlignItems::End:
				{
					itemHeight = intrinsicH;
					itemY = y + (h - itemHeight);
					break;
				}
				case GridLayoutProperties::AlignItems::Start:
				default:
				{
					itemHeight = intrinsicH;
					itemY = y;
					break;
				}
			}
			CRect childRect (itemX, itemY, itemX + itemWidth, itemY + itemHeight);
			std::optional<ViewLayout> childLayout;
			if (auto childViewContainer = (*child)->asViewContainer ())
				childLayout = {childViewContainer->calculateViewLayout (childRect)};
			layout.emplace_back (childRect, childLayout);
		}
	}
	else
	{
		// Default: one cell per child
		CCoord y = newSize.top + offsetY;
		for (size_t r = 0; r < rows; ++r)
		{
			CCoord x = newSize.left + offsetX;
			for (size_t c = 0; c < cols; ++c)
			{
				if (child == children.end ())
					break;
				CCoord cellWidth = colWidths[c];
				CCoord cellHeight = rowHeights[r];
				// Use autoColumns/autoRows if provided
				CCoord itemWidth = autoColumns.size () > c ? autoColumns[c] : cellWidth;
				CCoord itemHeight = autoRows.size () > r ? autoRows[r] : cellHeight;
				CCoord itemX = x;
				CCoord itemY = y;
				if (gridProps.justifyItems == GridLayoutProperties::JustifyItems::Start)
					itemX = x;
				else if (gridProps.justifyItems == GridLayoutProperties::JustifyItems::Center)
					itemX = x + (cellWidth - itemWidth) / 2.0;
				else if (gridProps.justifyItems == GridLayoutProperties::JustifyItems::End)
					itemX = x + (cellWidth - itemWidth);
				else if (gridProps.justifyItems == GridLayoutProperties::JustifyItems::Stretch)
				{
					itemX = x;
					itemWidth = cellWidth;
				}
				if (gridProps.alignItems == GridLayoutProperties::AlignItems::Start)
					itemY = y;
				else if (gridProps.alignItems == GridLayoutProperties::AlignItems::Center)
					itemY = y + (cellHeight - itemHeight) / 2.0;
				else if (gridProps.alignItems == GridLayoutProperties::AlignItems::End)
					itemY = y + cellHeight - itemHeight;
				else if (gridProps.alignItems == GridLayoutProperties::AlignItems::Stretch)
				{
					itemY = y;
					itemHeight = cellHeight;
				}
				CRect childRect (itemX, itemY, itemX + itemWidth, itemY + itemHeight);
				std::optional<ViewLayout> childLayout;
				if (auto childViewContainer = (*child)->asViewContainer ())
					childLayout = {childViewContainer->calculateViewLayout (childRect)};
				layout.emplace_back (childRect, childLayout);

				x += cellWidth + colGap;
				if (spaceBetweenX > 0.0 && c < cols - 1)
					x += spaceBetweenX;
				else if (spaceAroundX > 0.0)
					x += spaceAroundX;
				++child;
			}
			y += rowHeights[r] + rowGap;
			// Add extra space for SpaceBetween/SpaceAround
			if (gridProps.alignContent == GridLayoutProperties::AlignContent::SpaceBetween &&
				r < rows - 1)
				y += spaceBetweenY;
			else if (gridProps.alignContent == GridLayoutProperties::AlignContent::SpaceAround &&
					 r < rows - 1)
				y += spaceAroundY;
		}
	}
	if (layout.size () != children.size ())
		return std::nullopt;
	return {{newSize, std::move (layout)}};
}

//------------------------------------------------------------------------
bool GridLayouter::applyLayout (CViewContainer& container, const Children& children,
								const ViewLayout& layout)
{
	if (gridProps.rows == 0 || gridProps.columns == 0)
		return false;

	const auto layoutData = std::any_cast<LayoutData> (&layout.data);
	if (!layoutData || layoutData->size () != children.size ())
		return false;

	size_t childIdx = 0;
	for (auto& child : children)
	{
		auto rect = (*layoutData)[childIdx].first;
		auto& childLayout = (*layoutData)[childIdx].second;
		if (auto childContainer = child->asViewContainer (); childContainer && childLayout)
		{
			childContainer->applyViewLayout (*childLayout);
		}
		else
		{
			child->setViewSize (rect, true);
			child->setMouseableArea (rect);
		}
		++childIdx;
	}
	container.setViewSize (layout.size);
	container.setMouseableArea (layout.size);
	return true;
}

//------------------------------------------------------------------------
} // namespace VSTGUI
