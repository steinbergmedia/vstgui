// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/viewlayouter/gridlayouter.h"
#include "../../../lib/cviewcontainer.h"
#include "../../../lib/cview.h"
#include "../unittests.h"
#include <array>

namespace VSTGUI {

//------------------------------------------------------------------------
static bool rectNearlyEqual (const CRect& a, const CRect& b, double epsilon = 1e-6)
{
	return std::abs (a.left - b.left) < epsilon && std::abs (a.top - b.top) < epsilon &&
		   std::abs (a.right - b.right) < epsilon && std::abs (a.bottom - b.bottom) < epsilon;
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, BasicGrid)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.rowGap = 5;
	props.columnGap = 10;
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	// Check child positions and sizes
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 4);
	constexpr std::array expectedRects {CRect (0, 0, 45, 47.5), CRect (55, 0, 100, 47.5),
										CRect (0, 52.5, 45, 100), CRect (55, 52.5, 100, 100)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, AutoRowColumnSizes)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {CCoord (30), CCoord (70)};
	props.autoColumns = {CCoord (60), CCoord (40)};
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));

	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 4);
	constexpr std::array expectedRects {CRect (0, 0, 60, 30), CRect (60, 0, 100, 30),
										CRect (0, 30, 60, 100), CRect (60, 30, 100, 100)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, AutoRowColumnSizes_Percentage)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {GridLayoutProperties::Percentage {30.0},
					  GridLayoutProperties::Percentage {70.0}};
	props.autoColumns = {GridLayoutProperties::Percentage {60.0},
						 GridLayoutProperties::Percentage {40.0}};
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));

	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 4);
	constexpr std::array expectedRects {CRect (0, 0, 60, 30), CRect (60, 0, 100, 30),
										CRect (0, 30, 60, 100), CRect (60, 30, 100, 100)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, JustifyContentCenter)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 1;
	props.columns = 2;
	props.autoColumns = {CCoord (30), CCoord (30)};
	props.justifyContent = GridLayoutProperties::JustifyContent::Center;
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 100, 40));
	container->addView (new CView ({}));
	container->addView (new CView ({}));

	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 40));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));

	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 2);
	constexpr std::array expectedRects {CRect (20, 0, 50, 40), CRect (50, 0, 80, 40)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, RowColumnGaps)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.rowGap = 10;
	props.columnGap = 20;
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 4);
	constexpr std::array expectedRects {CRect (0, 0, 40, 45), CRect (60, 0, 100, 45),
										CRect (0, 55, 40, 100), CRect (60, 55, 100, 100)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, AlignContentCenter)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 1;
	props.autoRows = {CCoord (30), CCoord (30)};
	props.alignContent = GridLayoutProperties::AlignContent::Center;
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 2);
	constexpr std::array expectedRects {CRect (0, 20, 100, 50), CRect (0, 50, 100, 80)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, AlignContentEnd)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 1;
	props.autoRows = {CCoord (30), CCoord (30)};
	props.alignContent = GridLayoutProperties::AlignContent::End;
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 2);
	constexpr std::array expectedRects {CRect (0, 40, 100, 70), CRect (0, 70, 100, 100)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, JustifyContentEnd)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 1;
	props.columns = 2;
	props.autoColumns = {CCoord (30), CCoord (30)};
	props.justifyContent = GridLayoutProperties::JustifyContent::End;
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 100, 40));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 40));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 2);
	constexpr std::array expectedRects {CRect (40, 0, 70, 40), CRect (70, 0, 100, 40)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, AlignItemsCenter)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 1;
	props.columns = 1;
	props.autoRows = {CCoord (40)};
	props.autoColumns = {CCoord (60)};
	props.alignItems = GridLayoutProperties::AlignItems::Center;
	props.justifyItems = GridLayoutProperties::JustifyItems::Start;
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 1);
	constexpr std::array expectedRects {CRect (0, 30, 60, 70)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, JustifyItemsCenter)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 1;
	props.columns = 1;
	props.autoRows = {CCoord (40)};
	props.autoColumns = {CCoord (60)};
	props.justifyItems = GridLayoutProperties::JustifyItems::Center;
	props.alignItems =
		GridLayoutProperties::AlignItems::Start; // Ensure height is 40, not stretched
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 1);
	constexpr std::array expectedRects {CRect (20, 0, 80, 40)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, AlignItemsStart)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 1;
	props.columns = 1;
	props.autoRows = {CCoord (40)};
	props.autoColumns = {CCoord (60)};
	props.alignItems = GridLayoutProperties::AlignItems::Start;
	props.justifyItems = GridLayoutProperties::JustifyItems::Start;
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 1);
	constexpr std::array expectedRects {CRect (0, 0, 60, 40)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, AlignItemsEnd)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 1;
	props.columns = 1;
	props.autoRows = {CCoord (40)};
	props.autoColumns = {CCoord (60)};
	props.alignItems = GridLayoutProperties::AlignItems::End;
	props.justifyItems = GridLayoutProperties::JustifyItems::Start;
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 1);
	constexpr std::array expectedRects {CRect (0, 60, 60, 100)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, AlignItemsStretch)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 1;
	props.autoRows = {CCoord (30), CCoord (30)};
	props.alignContent = GridLayoutProperties::AlignContent::Stretch;
	props.justifyItems = GridLayoutProperties::JustifyItems::Start;
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 2);
	constexpr std::array expectedRects {CRect (0, 0, 100, 50), CRect (0, 50, 100, 100)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, AlignContentSpaceBetween)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 3;
	props.columns = 1;
	props.autoRows = {CCoord (20), CCoord (20), CCoord (20)};
	props.alignContent = GridLayoutProperties::AlignContent::SpaceBetween;
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 3);
	constexpr std::array expectedRects {CRect (0, 0, 100, 20), CRect (0, 40, 100, 60),
										CRect (0, 80, 100, 100)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, AlignContentSpaceAround)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 1;
	props.autoRows = {CCoord (30), CCoord (30)};
	props.alignContent = GridLayoutProperties::AlignContent::SpaceAround;
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));

	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 2);
	constexpr std::array expectedRects {CRect (0, 13.3333333333, 100, 43.3333333333),
										CRect (0, 56.6666666667, 100, 86.6666666667)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT (rectNearlyEqual ((*rects)[i].first, expectedRects[i]));
		EXPECT (rectNearlyEqual (container->getView (static_cast<int> (i))->getViewSize (),
								 expectedRects[i]));
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_SingleViewSpanning)
{
	// 2x2 grid, one view spanning all cells
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {CCoord (40), CCoord (60)};
	props.autoColumns = {CCoord (50), CCoord (70)};
	props.gridAreas = {{0, 0, 2, 2}};
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 120, 100));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 120, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 1);
	CRect expected (0, 0, 120, 100);
	EXPECT (rectNearlyEqual ((*rects)[0].first, expected));
	EXPECT (rectNearlyEqual (container->getView (0)->getViewSize (), expected));
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_MultipleViews)
{
	// 3x3 grid, three views with different areas
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 3;
	props.columns = 3;
	props.rowGap = 5;
	props.columnGap = 10;
	props.autoRows = {CCoord (30), CCoord (30), CCoord (40)};
	props.autoColumns = {CCoord (40), CCoord (30), CCoord (30)};
	props.gridAreas = {
		{0, 0, 2, 2}, // View 0: spans 2 rows, 2 columns (top-left 2x2)
		{0, 2, 1, 1}, // View 1: top-right cell
		{2, 0, 1, 3}  // View 2: bottom row, spans all columns
	};
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 120, 110));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 120, 110));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 3);
	// View 0: spans rows 0-1, columns 0-1
	constexpr std::array expectedRects {
		CRect (0, 0, 80, 65),	// 2 cols + gap, 2 rows + gap
		CRect (90, 0, 120, 30), // row 0, col 2
		CRect (0, 70, 120, 110) // bottom row, all columns
	};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT (rectNearlyEqual ((*rects)[i].first, expectedRects[i]));
		EXPECT (rectNearlyEqual (container->getView (static_cast<int> (i))->getViewSize (),
								 expectedRects[i]));
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_ClampedOutOfBounds)
{
	// 2x2 grid, view with area outside bounds
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {CCoord (40), CCoord (60)};
	props.autoColumns = {CCoord (50), CCoord (70)};
	props.gridAreas = {{1, 1, 2, 2}}; // Should clamp to (1,1,1,1)
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 120, 100));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 120, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 1);
	CRect expected (50, 40, 120, 100);
	EXPECT (rectNearlyEqual ((*rects)[0].first, expected));
	EXPECT (rectNearlyEqual (container->getView (0)->getViewSize (), expected));
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_DefaultBehavior)
{
	// 2x2 grid, no gridAreas, each view in one cell
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {CCoord (40), CCoord (60)};
	props.autoColumns = {CCoord (50), CCoord (70)};
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 120, 100));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 120, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 4);
	constexpr std::array expectedRects {CRect (0, 0, 50, 40), CRect (50, 0, 120, 40),
										CRect (0, 40, 50, 100), CRect (50, 40, 120, 100)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT (rectNearlyEqual ((*rects)[i].first, expectedRects[i]));
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, JustifyItemsStart)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 1;
	props.columns = 1;
	props.autoRows = {CCoord (40)};
	props.autoColumns = {CCoord (60)};
	props.justifyItems = GridLayoutProperties::JustifyItems::Start;
	props.alignItems = GridLayoutProperties::AlignItems::Start;
	layouter->setProperties (props);

	// Container wider than cell, so cell is left-aligned
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 1);
	constexpr std::array expectedRects {CRect (0, 0, 60, 40)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, JustifyItemsEnd)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 1;
	props.columns = 1;
	props.autoRows = {CCoord (40)};
	props.autoColumns = {CCoord (60)};
	props.justifyItems = GridLayoutProperties::JustifyItems::End;
	props.alignItems = GridLayoutProperties::AlignItems::Start;
	layouter->setProperties (props);

	// Container wider than cell, so cell is right-aligned
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 1);
	constexpr std::array expectedRects {CRect (40, 0, 100, 40)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, JustifyItemsEnd_MultipleViews)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 1;
	props.columns = 2;
	props.autoRows = {CCoord (40)};
	props.autoColumns = {CCoord (20), CCoord (20)};
	props.justifyContent = GridLayoutProperties::JustifyContent::Start;
	props.justifyItems = GridLayoutProperties::JustifyItems::End;
	props.alignItems = GridLayoutProperties::AlignItems::Start;
	layouter->setProperties (props);

	// Container wider than sum of columns, so each cell is 20 wide
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 100, 40));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 40));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 2);
	constexpr std::array expectedRects {CRect (0, 0, 20, 40), CRect (20, 0, 40, 40)};
	// Each cell is 20 wide, view is 20 wide, so right-aligned is the same as left-aligned:
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_JustifyContentBehavior)
{
	// Case 1: grid tracks exactly fit the container, justify-content should be ignored
	{
		GridLayoutProperties props;
		props.rows = 1;
		props.columns = 2;
		props.autoColumns = {CCoord (50), CCoord (50)}; // 100 total
		props.gridAreas = {{0, 0, 1, 1}, {0, 1, 1, 1}};
		props.justifyContent = GridLayoutProperties::JustifyContent::End;
		auto layouter = makeOwned<GridLayouter> (props);
		auto container = makeOwned<CViewContainer> (CRect (0, 0, 100, 40));
		container->addView (new CView ({}));
		container->addView (new CView ({}));
		container->setViewLayouter (layouter);
		auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 40));
		EXPECT (layoutOpt.has_value ());
		auto layout = layoutOpt.value ();
		EXPECT (container->applyViewLayout (layout));
		const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
		EXPECT (rects && rects->size () == 2);
		constexpr std::array expectedRects {CRect (0, 0, 50, 40), CRect (50, 0, 100, 40)};
		// Should be left-aligned, not offset
		for (size_t i = 0; i < expectedRects.size (); ++i)
		{
			EXPECT ((*rects)[i].first == expectedRects[i]);
			EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
		}
	}
	// Case 2: grid tracks smaller than container, justify-content should offset grid
	{
		GridLayoutProperties props;
		props.rows = 1;
		props.columns = 2;
		props.autoColumns = {CCoord (30), CCoord (30)}; // 60 total
		props.gridAreas = {{0, 0, 1, 1}, {0, 1, 1, 1}};
		props.justifyContent = GridLayoutProperties::JustifyContent::Center;
		auto layouter = makeOwned<GridLayouter> (props);
		auto container = makeOwned<CViewContainer> (CRect (0, 0, 100, 40));
		container->addView (new CView ({}));
		container->addView (new CView ({}));
		container->setViewLayouter (layouter);
		auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 40));
		EXPECT (layoutOpt.has_value ());
		auto layout = layoutOpt.value ();
		EXPECT (container->applyViewLayout (layout));
		const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
		EXPECT (rects && rects->size () == 2);
		constexpr std::array expectedRects {CRect (20, 0, 50, 40), CRect (50, 0, 80, 40)};
		// Should be centered: offset = (100 - 60) / 2 = 20
		for (size_t i = 0; i < expectedRects.size (); ++i)
		{
			EXPECT ((*rects)[i].first == expectedRects[i]);
			EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
		}
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_AlignJustifyItems_StartStart)
{
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {CCoord (40), CCoord (60)};
	props.autoColumns = {CCoord (50), CCoord (70)};
	props.gridAreas = {
		{0, 0, 1, 1}, // View 0: top-left cell
		{1, 1, 1, 1}  // View 1: bottom-right cell
	};
	props.justifyItems = GridLayoutProperties::JustifyItems::Start;
	props.alignItems = GridLayoutProperties::AlignItems::Start;
	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 120, 100));
	container->addView (new CView ({0., 0., 50., 40.}));
	container->addView (new CView ({0., 0., 70., 60.}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 120, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 2);
	constexpr std::array expectedRects {CRect (0, 0, 50, 40), CRect (50, 40, 120, 100)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_AlignJustifyItems_CenterCenter)
{
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {CCoord {40.}, CCoord {40.}};
	props.autoColumns = {CCoord {40.}, CCoord {40.}};
	props.gridAreas = {
		{0, 0, 1, 1}, // View 0: top-left cell
		{1, 1, 1, 1}  // View 1: bottom-right cell
	};
	props.justifyContent = GridLayoutProperties::JustifyContent::Center;
	props.alignContent = GridLayoutProperties::AlignContent::Center;
	props.justifyItems = GridLayoutProperties::JustifyItems::Center;
	props.alignItems = GridLayoutProperties::AlignItems::Center;

	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 120, 100));
	container->addView (new CView ({0., 0., 20., 20.}));
	container->addView (new CView ({0., 0., 40., 40.}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 400, 400));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 2);
	constexpr std::array expectedRects {CRect (170, 170, 190, 190), CRect (200, 200, 240, 240)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_AlignJustifyItems_EndEnd)
{
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {CCoord (40), CCoord (40)};
	props.autoColumns = {CCoord (50), CCoord (50)};
	props.gridAreas = {
		{0, 0, 1, 1}, // View 0: top-left cell
		{1, 1, 1, 1}  // View 1: bottom-right cell
	};
	props.justifyContent = GridLayoutProperties::JustifyContent::Center;
	props.alignContent = GridLayoutProperties::AlignContent::Center;
	props.justifyItems = GridLayoutProperties::JustifyItems::End;
	props.alignItems = GridLayoutProperties::AlignItems::End;
	auto layouter = makeOwned<GridLayouter> (props);
	// Container size changed to match expected rects
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 130, 120));
	container->addView (new CView ({0., 0., 50., 40.}));
	container->addView (new CView ({0., 0., 50., 40.}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 400, 400));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 2);
	constexpr std::array expectedRects {CRect (150, 160, 200, 200), CRect (200, 200, 250, 240)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_AlignJustifyItems_StretchStretch)
{
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {CCoord (40), CCoord (60)};
	props.autoColumns = {CCoord (50), CCoord (70)};
	props.gridAreas = {
		{0, 0, 1, 1}, // View 0: top-left cell
		{1, 1, 1, 1}  // View 1: bottom-right cell
	};
	props.justifyItems = GridLayoutProperties::JustifyItems::Stretch;
	props.alignItems = GridLayoutProperties::AlignItems::Stretch;
	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 120, 100));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 120, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 2);
	constexpr std::array expectedRects {CRect (0, 0, 50, 40), CRect (50, 40, 120, 100)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, JustifyItemsStretch_NoGridArea)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 1;
	props.columns = 1;
	props.autoRows = {CCoord (40)};
	props.autoColumns = {CCoord (60)};
	props.justifyItems = GridLayoutProperties::JustifyItems::Stretch;
	props.alignItems = GridLayoutProperties::AlignItems::Start;
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 1);
	constexpr std::array expectedRects {CRect (0, 0, 100, 40)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, AlignItemsStretch_NoGridArea)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 1;
	props.columns = 1;
	props.autoRows = {CCoord (40)};
	props.autoColumns = {CCoord (60)};
	props.justifyItems = GridLayoutProperties::JustifyItems::Start;
	props.alignItems = GridLayoutProperties::AlignItems::Stretch;
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 1);
	constexpr std::array expectedRects {CRect (0, 0, 60, 100)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, AlignContentStart_Explicit)
{
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 1;
	props.autoRows = {CCoord (30), CCoord (30)};
	props.alignContent = GridLayoutProperties::AlignContent::Start;
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 2);
	constexpr std::array expectedRects {CRect (0, 0, 100, 30), CRect (0, 30, 100, 60)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_AlignContentStretch)
{
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {CCoord (30), CCoord (30)};
	props.autoColumns = {CCoord (40), CCoord (40)};
	props.alignContent = GridLayoutProperties::AlignContent::Stretch;
	props.gridAreas = {{0, 0, 2, 2}};
	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 80, 100));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 80, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 1);
	CRect expected (0, 0, 80, 100);
	EXPECT (rectNearlyEqual ((*rects)[0].first, expected));
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_AlignContentSpaceBetween)
{
	GridLayoutProperties props;
	props.rows = 3;
	props.columns = 1;
	props.autoRows = {CCoord (20), CCoord (20), CCoord (20)};
	props.alignContent = GridLayoutProperties::AlignContent::SpaceBetween;
	props.gridAreas = {{0, 0, 1, 1}, {1, 0, 1, 1}, {2, 0, 1, 1}};
	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 3);
	// SpaceBetween: first at top, last at bottom, middle spaced
	EXPECT (rectNearlyEqual ((*rects)[0].first, CRect (0, 0, 100, 20)));
	EXPECT (rectNearlyEqual ((*rects)[1].first, CRect (0, 40, 100, 60)));
	EXPECT (rectNearlyEqual ((*rects)[2].first, CRect (0, 80, 100, 100)));
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_AlignContentSpaceAround)
{
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 1;
	props.autoRows = {CCoord (30), CCoord (30)};
	props.alignContent = GridLayoutProperties::AlignContent::SpaceAround;
	props.gridAreas = {{0, 0, 1, 1}, {1, 0, 1, 1}};
	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 2);
	EXPECT (rectNearlyEqual ((*rects)[0].first, CRect (0, 13.3333333333, 100, 43.3333333333)));
	EXPECT (rectNearlyEqual ((*rects)[1].first, CRect (0, 56.6666666667, 100, 86.6666666667)));
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_JustifyContentStretch)
{
	GridLayoutProperties props;
	props.rows = 1;
	props.columns = 2;
	props.autoColumns = {CCoord (30), CCoord (30)};
	props.justifyContent = GridLayoutProperties::JustifyContent::Stretch;
	props.gridAreas = {{0, 0, 1, 1}, {0, 1, 1, 1}};
	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 100, 40));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 40));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 2);
	// Should stretch grid to fill container
	EXPECT (rectNearlyEqual ((*rects)[0].first, CRect (0, 0, 50, 40)));
	EXPECT (rectNearlyEqual ((*rects)[1].first, CRect (50, 0, 100, 40)));
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_JustifyContentSpaceBetween)
{
	GridLayoutProperties props;
	props.rows = 1;
	props.columns = 3;
	props.autoColumns = {CCoord (20), CCoord (20), CCoord (20)};
	props.justifyContent = GridLayoutProperties::JustifyContent::SpaceBetween;
	props.gridAreas = {{0, 0, 1, 1}, {0, 1, 1, 1}, {0, 2, 1, 1}};
	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 100, 40));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 40));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 3);
	// SpaceBetween: first at left, last at right, middle spaced
	EXPECT (rectNearlyEqual ((*rects)[0].first, CRect (0, 0, 20, 40)));
	EXPECT (rectNearlyEqual ((*rects)[1].first, CRect (40, 0, 60, 40)));
	EXPECT (rectNearlyEqual ((*rects)[2].first, CRect (80, 0, 100, 40)));
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_JustifyContentSpaceAround)
{
	GridLayoutProperties props;
	props.rows = 1;
	props.columns = 2;
	props.autoColumns = {CCoord (30), CCoord (30)};
	props.justifyContent = GridLayoutProperties::JustifyContent::SpaceAround;
	props.gridAreas = {{0, 0, 1, 1}, {0, 1, 1, 1}};
	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 100, 40));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 40));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 2);
	EXPECT (rectNearlyEqual ((*rects)[0].first, CRect (13.3333333333, 0, 43.3333333333, 40)));
	EXPECT (rectNearlyEqual ((*rects)[1].first, CRect (56.6666666667, 0, 86.6666666667, 40)));
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_Spanning_CenterEndStretch)
{
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {CCoord (40), CCoord (60)};
	props.autoColumns = {CCoord (50), CCoord (70)};
	props.gridAreas = {{0, 0, 2, 2}};
	props.justifyItems = GridLayoutProperties::JustifyItems::Center;
	props.alignItems = GridLayoutProperties::AlignItems::End;
	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 120, 100));
	container->addView (new CView ({0., 0., 20, 20.}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 120, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 1);
	// Center horizontally, end vertically
	EXPECT (rectNearlyEqual ((*rects)[0].first, CRect (50, 80, 70, 100)));
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_PercentageAutoRowsColumns)
{
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {GridLayoutProperties::Percentage {40.0},
					  GridLayoutProperties::Percentage {60.0}};
	props.autoColumns = {GridLayoutProperties::Percentage {50.0},
						 GridLayoutProperties::Percentage {50.0}};
	props.gridAreas = {{0, 0, 1, 1}, {1, 1, 1, 1}};
	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 200, 100));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 200, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 2);
	EXPECT (rectNearlyEqual ((*rects)[0].first, CRect (0, 0, 100, 40)));
	EXPECT (rectNearlyEqual ((*rects)[1].first, CRect (100, 40, 200, 100)));
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, MultiColumnRow_JustifyItemsCenter)
{
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {GridLayoutProperties::Percentage {40.0},
					  GridLayoutProperties::Percentage {60.0}};
	props.autoColumns = {GridLayoutProperties::Percentage {50.0},
						 GridLayoutProperties::Percentage {50.0}};
	props.justifyItems = GridLayoutProperties::JustifyItems::Center;
	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 200, 100));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 200, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 4);
	EXPECT (rectNearlyEqual ((*rects)[0].first, CRect (0, 0, 100, 40)));
	EXPECT (rectNearlyEqual ((*rects)[1].first, CRect (100, 0, 200, 40)));
	EXPECT (rectNearlyEqual ((*rects)[2].first, CRect (0, 40, 100, 100)));
	EXPECT (rectNearlyEqual ((*rects)[3].first, CRect (100, 40, 200, 100)));
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, MultiColumnRow_JustifyItemsCenterAlignItemsCenter)
{
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {GridLayoutProperties::Percentage {40.0},
					  GridLayoutProperties::Percentage {60.0}};
	props.autoColumns = {GridLayoutProperties::Percentage {50.0},
						 GridLayoutProperties::Percentage {50.0}};
	props.justifyItems = GridLayoutProperties::JustifyItems::Center;
	props.alignItems = GridLayoutProperties::AlignItems::Center;
	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 200, 100));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 200, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 4);
	EXPECT (rectNearlyEqual ((*rects)[0].first, CRect (0, 0, 100, 40)));
	EXPECT (rectNearlyEqual ((*rects)[1].first, CRect (100, 0, 200, 40)));
	EXPECT (rectNearlyEqual ((*rects)[2].first, CRect (0, 40, 100, 100)));
	EXPECT (rectNearlyEqual ((*rects)[3].first, CRect (100, 40, 200, 100)));
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, AutoRowSizes)
{
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {GridLayoutProperties::Auto {}, CCoord {20}};
	props.autoColumns = {CCoord (50), CCoord (50)};
	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	// Min size logic skipped: CView does not support setMinSize
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 4);
	constexpr std::array expectedRects {CRect (0, 0, 50, 80), CRect (50, 0, 100, 80),
										CRect (0, 80, 50, 100), CRect (50, 80, 100, 100)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT (rectNearlyEqual ((*rects)[i].first, expectedRects[i]));
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, AutoColumnSizes)
{
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {CCoord (40), CCoord (60)};
	props.autoColumns = {GridLayoutProperties::Auto {}, GridLayoutProperties::Auto {}};
	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	// Min size logic skipped: CView does not support setMinSize
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 4);
	constexpr std::array expectedRects {CRect (0, 0, 50, 40), CRect (50, 0, 100, 40),
										CRect (0, 40, 50, 100), CRect (50, 40, 100, 100)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT (rectNearlyEqual ((*rects)[i].first, expectedRects[i]));
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_Spanning_AutoRowsColumns)
{
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {GridLayoutProperties::Auto {}, GridLayoutProperties::Auto {}};
	props.autoColumns = {GridLayoutProperties::Auto {}, GridLayoutProperties::Auto {}};
	props.gridAreas = {{0, 0, 2, 2}}; // One view spanning all cells
	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	CView* v0 = new CView ({0, 0, 80, 60});
	container->addView (v0);
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 1);
	// Should fill the container, but not smaller than min size
	CRect expected (0, 0, 100, 100);
	EXPECT (rectNearlyEqual ((*rects)[0].first, expected));
	EXPECT (rectNearlyEqual (container->getView (0)->getViewSize (), expected));
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_Spanning_AutoRowsColumns_MultiView)
{
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {GridLayoutProperties::Auto {}, CCoord {60.0}};
	props.autoColumns = {CCoord {30.0}, GridLayoutProperties::Auto {}};
	props.gridAreas = {
		{0, 0, 1, 2}, // View 0: spans first row, both columns
		{1, 0, 1, 1}, // View 1: second row, first column
		{1, 1, 1, 1}  // View 2: second row, second column
	};
	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	CView* v0 = new CView ({});
	CView* v1 = new CView ({});
	CView* v2 = new CView ({});
	container->addView (v0);
	container->addView (v1);
	container->addView (v2);
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 3);
	// Expected: first row height = 30, second row height = 70
	// First row spans both columns, so width = 100
	// Second row: first cell min width 40, second cell min width 60
	constexpr std::array expectedRects {
		CRect (0, 0, 100, 40),	 // View 0
		CRect (0, 40, 30, 100),	 // View 1
		CRect (30, 40, 100, 100) // View 2
	};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT (rectNearlyEqual ((*rects)[i].first, expectedRects[i]));
		EXPECT (rectNearlyEqual (container->getView (static_cast<int> (i))->getViewSize (),
								 expectedRects[i]));
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, LayoutEmptyContainer)
{
	auto layouter = makeOwned<GridLayouter> (GridLayoutProperties {});
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 0, 0));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 120, 120));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->empty () == true);
	EXPECT (rectNearlyEqual (CRect (0, 0, 120, 120), container->getViewSize ()))
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_SpaceAround_MultiRowCol_AutoTracks_ExtraItem)
{
	// 2x2 grid, 5 items, grid areas for each, align/justify content SpaceAround, auto tracks
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {GridLayoutProperties::Auto {}, GridLayoutProperties::Auto {}, CCoord (30)};
	props.autoColumns = {GridLayoutProperties::Auto {}, GridLayoutProperties::Auto {}, CCoord (30)};
	props.alignContent = GridLayoutProperties::AlignContent::SpaceAround;
	props.justifyContent = GridLayoutProperties::JustifyContent::SpaceAround;
	props.gridAreas = {
		{0, 0, 1, 1}, // Item 0: top-left
		{0, 1, 1, 1}, // Item 1: top-right
		{1, 0, 1, 1}, // Item 2: bottom-left
		{1, 1, 1, 1}, // Item 3: bottom-right
		{0, 0, 2, 2}  // Item 4: spans all
	};
	// Container is larger than grid, so space around is visible
	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 200, 200));
	for (int i = 0; i < 5; ++i)
		container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 200, 200));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));
	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 5);
	constexpr std::array expectedRects {
		CRect (10, 10, 95, 95),		// Item 0: top-left
		CRect (105, 10, 190, 95),	// Item 1: top-right
		CRect (10, 105, 95, 190),	// Item 2: bottom-left
		CRect (105, 105, 190, 190), // Item 3: bottom-right
		CRect (10, 10, 190, 190)	// Item 4: spans all
	};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT (rectNearlyEqual ((*rects)[i].first, expectedRects[i]));
		EXPECT (rectNearlyEqual (container->getView (static_cast<int> (i))->getViewSize (),
								 expectedRects[i]));
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_IntrinsicSize_Center_NoStretch)
{
	GridLayoutProperties props;
	props.rows = 1;
	props.columns = 1;
	props.autoRows = {CCoord (100)};
	props.autoColumns = {CCoord (200)};
	props.gridAreas = {{0, 0, 1, 1}};
	props.justifyItems = GridLayoutProperties::JustifyItems::Center;
	props.alignItems = GridLayoutProperties::AlignItems::Center;

	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 200, 100));

	// Child with intrinsic size 80x40
	CView* child = new CView ({0, 0, 80, 40});
	container->addView (child);

	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 200, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));

	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 1);

	// Centered within 200x100 area -> (60,30)-(140,70)
	constexpr CRect expected (60, 30, 140, 70);
	EXPECT (rectNearlyEqual ((*rects)[0].first, expected));
	EXPECT (rectNearlyEqual (container->getView (0)->getViewSize (), expected));
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_IntrinsicSize_Stretch)
{
	GridLayoutProperties props;
	props.rows = 1;
	props.columns = 1;
	props.autoRows = {CCoord (100)};
	props.autoColumns = {CCoord (200)};
	props.gridAreas = {{0, 0, 1, 1}};
	props.justifyItems = GridLayoutProperties::JustifyItems::Stretch;
	props.alignItems = GridLayoutProperties::AlignItems::Stretch;

	auto layouter = makeOwned<GridLayouter> (props);
	auto container = makeOwned<CViewContainer> (CRect (0, 0, 200, 100));

	// Child with intrinsic size 80x40 (ignored when stretching)
	CView* child = new CView ({0, 0, 80, 40});
	container->addView (child);

	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 200, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));

	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 1);

	// Full area
	constexpr CRect expected (0, 0, 200, 100);
	EXPECT (rectNearlyEqual ((*rects)[0].first, expected));
	EXPECT (rectNearlyEqual (container->getView (0)->getViewSize (), expected));
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_Mismatch_FewerChildrenThanAreas)
{
	// 2x2 grid, 3 defined areas, but only 2 children. Extra area is ignored.
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {CCoord (50), CCoord (50)};
	props.autoColumns = {CCoord (50), CCoord (50)};
	props.gridAreas = {
		{0, 0, 1, 1}, // Area for child 0: top-left
		{0, 1, 1, 1}, // Area for child 1: top-right
		{1, 0, 1, 2}  // Extra area spanning bottom row (no child)
	};
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 100, 100));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));

	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 2);
	constexpr std::array expectedRects {CRect (0, 0, 50, 50), CRect (50, 0, 100, 50)};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_Mismatch_MoreChildrenThanAreas_AutoPlacement)
{
	// 2x2 grid, 1 defined area for the first child, remaining children auto-placed.
	auto layouter = makeOwned<GridLayouter> ();
	GridLayoutProperties props;
	props.rows = 2;
	props.columns = 2;
	props.autoRows = {CCoord (50), CCoord (50)};
	props.autoColumns = {CCoord (50), CCoord (50)};
	props.gridAreas = {
		{0, 0, 1, 1} // Child 0: top-left cell
	};
	layouter->setProperties (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 100, 100));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 100, 100));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));

	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 4);
	constexpr std::array expectedRects {
		CRect (0, 0, 50, 50),	 // Child 0: explicit area
		CRect (50, 0, 100, 50),	 // Child 1: auto-placed to (0,1)
		CRect (0, 50, 50, 100),	 // Child 2: auto-placed to (1,0)
		CRect (50, 50, 100, 100) // Child 3: auto-placed to (1,1)
	};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

//------------------------------------------------------------------------
TEST_CASE (GridLayouter, GridAreas_MultiRowView_SpaceBetween)
{
	GridLayoutProperties props;
	props.rows = 4;
	props.columns = 4;
	props.rowGap = 0;
	props.columnGap = 0;
	props.alignItems = GridLayoutProperties::AlignItems::Stretch;
	props.justifyItems = GridLayoutProperties::JustifyItems::Stretch;
	props.alignContent = GridLayoutProperties::AlignContent::SpaceBetween;
	props.justifyContent = GridLayoutProperties::JustifyContent::SpaceBetween;
	props.autoRows.reserve (5);
	props.autoRows.push_back (GridLayoutProperties::Auto {});
	props.autoRows.push_back (GridLayoutProperties::Auto {});
	props.autoRows.push_back (GridLayoutProperties::Auto {});
	props.autoRows.push_back (GridLayoutProperties::Auto {});
	props.autoRows.push_back (CCoord {30});
	props.autoColumns.reserve (5);
	props.autoColumns.push_back (GridLayoutProperties::Auto {});
	props.autoColumns.push_back (GridLayoutProperties::Auto {});
	props.autoColumns.push_back (GridLayoutProperties::Auto {});
	props.autoColumns.push_back (GridLayoutProperties::Auto {});
	props.autoColumns.push_back (CCoord {30});
	props.gridAreas.reserve (4);
	props.gridAreas.push_back ({0, 0, 3, 1});
	props.gridAreas.push_back ({1, 1, 2, 1});
	props.gridAreas.push_back ({2, 2, 2, 1});
	props.gridAreas.push_back ({0, 1, 1, 2});
	auto layouter = makeOwned<GridLayouter> (props);

	auto container = makeOwned<CViewContainer> (CRect (0, 0, 100, 100));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->addView (new CView ({}));
	container->setViewLayouter (layouter);
	auto layoutOpt = container->calculateViewLayout (CRect (0, 0, 110, 110));
	EXPECT (layoutOpt.has_value ());
	auto layout = layoutOpt.value ();
	EXPECT (container->applyViewLayout (layout));

	const auto rects = std::any_cast<GridLayouter::LayoutData> (&layout.data);
	EXPECT (rects && rects->size () == 11);
	constexpr std::array expectedRects {
		CRect (0, 0, 20, 80),	  // 0
		CRect (30, 30, 50, 80),	  // 1
		CRect (60, 60, 80, 110),  // 2
		CRect (30, 0, 80, 20),	  // 3
		CRect (90, 0, 110, 20),	  // 4
		CRect (60, 30, 80, 50),	  // 5
		CRect (90, 30, 110, 50),  // 6
		CRect (90, 60, 110, 80),  // 7
		CRect (0, 90, 20, 110),	  // 8
		CRect (30, 90, 50, 110),  // 9
		CRect (90, 90, 110, 110), // 10
	};
	for (size_t i = 0; i < expectedRects.size (); ++i)
	{
		EXPECT ((*rects)[i].first == expectedRects[i]);
		EXPECT (container->getView (static_cast<int> (i))->getViewSize () == expectedRects[i]);
	}
}

} // namespace VSTGUI
