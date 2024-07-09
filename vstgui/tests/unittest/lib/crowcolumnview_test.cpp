// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/crowcolumnview.h"
#include "../unittests.h"
#include <vector>
#include <map>

namespace VSTGUI {

using Rects = std::vector<CRect>;

static const CRect templateSize (0., 0., 100., 100.);
static const CRect layoutSize (0., 0., 80., 80.);

// clang-format off
static const Rects childrenDefaultSizes = {
    {0., 0., 10., 10.},
    {0., 0., 20., 20.},
    {0., 0., 30., 30.}
};

using ExpectedResults = std::map<CRowColumnView::LayoutStyle, Rects>;

static const ExpectedResults kRowLayoutChildrenResultSizes = {
    {
        CRowColumnView::kTopLeft, {
            {0., 0., 10., 10.},
            {0., 10., 20., 30.},
            {0., 30., 30., 60.}
        }
    },
    {
        CRowColumnView::kTopCenter, {
            {35., 0., 45., 10.},
            {30., 10., 50., 30.},
            {25., 30., 55., 60.}
        }
    },
    {
        CRowColumnView::kTopRight, {
            {70., 0., 80., 10.},
            {60., 10., 80., 30.},
            {50., 30., 80., 60.}
        }
    },
    {
        CRowColumnView::kMiddleLeft, {
            {0., 10., 10., 20.},
            {0., 20., 20., 40.},
            {0., 40., 30., 70.}
        }
    },
    {
        CRowColumnView::kMiddleCenter, {
            {35., 10., 45., 20.},
            {30., 20., 50., 40.},
            {25., 40., 55., 70.}
        }
    },
    {
        CRowColumnView::kMiddleRight, {
            {70., 10., 80., 20.},
            {60., 20., 80., 40.},
            {50., 40., 80., 70.}
        }
    },
    {
        CRowColumnView::kBottomLeft, {
            {0., 20., 10., 30.},
            {0., 30., 20., 50.},
            {0., 50., 30., 80.}
        }
    },
    {
        CRowColumnView::kBottomCenter, {
            {35., 20., 45., 30.},
            {30., 30., 50., 50.},
            {25., 50., 55., 80.}
        }
    },
    {
        CRowColumnView::kBottomRight, {
            {70., 20., 80., 30.},
            {60., 30., 80., 50.},
            {50., 50., 80., 80.}
        }
    }
};

static const ExpectedResults kColumnLayoutChildrenResultSizes = {
    {
        CRowColumnView::kTopLeft, {
            {0., 0., 10., 10.},
            {10., 0., 30., 20.},
            {30., 0., 60., 30.}
        }
    },
    {
        CRowColumnView::kTopCenter, {
            {10., 0., 20., 10.},
            {20., 0., 40., 20.},
            {40., 0., 70., 30.}
        }
    },
    {
        CRowColumnView::kTopRight, {
            {20., 0., 30., 10.},
            {30., 0., 50., 20.},
            {50., 0., 80., 30.}
        }
    },
    {
        CRowColumnView::kMiddleLeft, {
            {0., 35., 10., 45.},
            {10., 30., 30., 50.},
            {30., 25., 60., 55.}
        }
    },
    {
        CRowColumnView::kMiddleCenter, {
            {10., 35., 20., 45.},
            {20., 30., 40., 50.},
            {40., 25., 70., 55.}
        }
    },
    {
        CRowColumnView::kMiddleRight, {
            {20., 35., 30., 45.},
            {30., 30., 50., 50.},
            {50., 25., 80., 55.}
        }
    },
    {
        CRowColumnView::kBottomLeft, {
            {0., 70., 10., 80.},
            {10., 60., 30., 80.},
            {30., 50., 60., 80.}
        }
    },
    {
        CRowColumnView::kBottomCenter, {
            {10., 70., 20., 80.},
            {20., 60., 40., 80.},
            {40., 50., 70., 80.}
        }
    },
    {
        CRowColumnView::kBottomRight, {
            {20., 70., 30., 80.},
            {30., 60., 50., 80.},
            {50., 50., 80., 80.}
        }
    }
};

static const ExpectedResults kRowLayoutChildrenResultSizesWithSpacing = {
    {
        CRowColumnView::kMiddleCenter, {
            {35., 6., 45., 16.},
            {30., 20., 50., 40.},
            {25., 44., 55., 74.}
        }
    }
};
// clang-format on

struct TestData
{
	CRowColumnView::LayoutStyle layoutStyle = CRowColumnView::LayoutStyle::kTopLeft;
	CRowColumnView::Style style = CRowColumnView::Style::kRowStyle;
	double spacing = 0.;
	ExpectedResults expected;
};

auto testWithLayoutStyle (const TestData& testData) -> void
{
	const auto& expected = testData.expected.find (testData.layoutStyle)->second;
	auto rowColumnView = owned (new CRowColumnView (layoutSize));
	rowColumnView->setStyle (testData.style);
	rowColumnView->setLayoutStyle (testData.layoutStyle);
	rowColumnView->setSpacing (testData.spacing);

	for (auto& rect : childrenDefaultSizes)
	{
		auto child = new CView (rect);
		rowColumnView->CViewContainer::addView (child);
		rowColumnView->layoutViews ();
	}

	size_t i = 0;
	rowColumnView->forEachChild ([&] (CView* child) {
		const auto& childrenResults = testData.expected.find (testData.layoutStyle);
		auto viewSize = child->getViewSize ();
		EXPECT (viewSize == expected.at (i))
		i++;
	});
}

TEST_CASE (CRowColumnViewTest, RowLayoutTopLeftStyle)
{
	testWithLayoutStyle (
		{CRowColumnView::kTopLeft, CRowColumnView::kRowStyle, 0., kRowLayoutChildrenResultSizes});
}

TEST_CASE (CRowColumnViewTest, RowLayoutTopCenterStyle)
{
	testWithLayoutStyle (
		{CRowColumnView::kTopCenter, CRowColumnView::kRowStyle, 0., kRowLayoutChildrenResultSizes});
}

TEST_CASE (CRowColumnViewTest, RowLayoutTopRightStyle)
{
	testWithLayoutStyle (
		{CRowColumnView::kTopRight, CRowColumnView::kRowStyle, 0., kRowLayoutChildrenResultSizes});
}

TEST_CASE (CRowColumnViewTest, RowLayoutMiddleLeftStyle)
{
	testWithLayoutStyle ({CRowColumnView::kMiddleLeft, CRowColumnView::kRowStyle, 0.,
						  kRowLayoutChildrenResultSizes});
}

TEST_CASE (CRowColumnViewTest, RowLayoutMiddleCenterStyle)
{
	testWithLayoutStyle ({CRowColumnView::kMiddleCenter, CRowColumnView::kRowStyle, 0.,
						  kRowLayoutChildrenResultSizes});
}

TEST_CASE (CRowColumnViewTest, RowLayoutMiddleRightStyle)
{
	testWithLayoutStyle ({CRowColumnView::kMiddleRight, CRowColumnView::kRowStyle, 0.,
						  kRowLayoutChildrenResultSizes});
}

TEST_CASE (CRowColumnViewTest, RowLayoutBottomLeftStyle)
{
	testWithLayoutStyle ({CRowColumnView::kBottomLeft, CRowColumnView::kRowStyle, 0.,
						  kRowLayoutChildrenResultSizes});
}

TEST_CASE (CRowColumnViewTest, RowLayoutBottomCenterStyle)
{
	testWithLayoutStyle ({CRowColumnView::kBottomCenter, CRowColumnView::kRowStyle, 0.,
						  kRowLayoutChildrenResultSizes});
}

TEST_CASE (CRowColumnViewTest, RowLayoutBottomRightStyle)
{
	testWithLayoutStyle ({CRowColumnView::kBottomRight, CRowColumnView::kRowStyle, 0.,
						  kRowLayoutChildrenResultSizes});
}

TEST_CASE (CRowColumnViewTest, ColumnLayoutTopLeftStyle)
{
	testWithLayoutStyle ({CRowColumnView::kTopLeft, CRowColumnView::kColumnStyle, 0.,
						  kColumnLayoutChildrenResultSizes});
}

TEST_CASE (CRowColumnViewTest, ColumnLayoutTopCenterStyle)
{
	testWithLayoutStyle ({CRowColumnView::kTopCenter, CRowColumnView::kColumnStyle, 0.,
						  kColumnLayoutChildrenResultSizes});
}

TEST_CASE (CRowColumnViewTest, ColumnLayoutTopRightStyle)
{
	testWithLayoutStyle ({CRowColumnView::kTopRight, CRowColumnView::kColumnStyle, 0.,
						  kColumnLayoutChildrenResultSizes});
}

TEST_CASE (CRowColumnViewTest, ColumnLayoutMiddleLeftStyle)
{
	testWithLayoutStyle ({CRowColumnView::kMiddleLeft, CRowColumnView::kColumnStyle, 0.,
						  kColumnLayoutChildrenResultSizes});
}

TEST_CASE (CRowColumnViewTest, ColumnLayoutMiddleCenterStyle)
{
	testWithLayoutStyle ({CRowColumnView::kMiddleCenter, CRowColumnView::kColumnStyle, 0.,
						  kColumnLayoutChildrenResultSizes});
}

TEST_CASE (CRowColumnViewTest, ColumnLayoutMiddleRightStyle)
{
	testWithLayoutStyle ({CRowColumnView::kMiddleRight, CRowColumnView::kColumnStyle, 0.,
						  kColumnLayoutChildrenResultSizes});
}

TEST_CASE (CRowColumnViewTest, ColumnLayoutBottomLeftStyle)
{
	testWithLayoutStyle ({CRowColumnView::kBottomLeft, CRowColumnView::kColumnStyle, 0.,
						  kColumnLayoutChildrenResultSizes});
}

TEST_CASE (CRowColumnViewTest, ColumnLayoutBottomCenterStyle)
{
	testWithLayoutStyle ({CRowColumnView::kBottomCenter, CRowColumnView::kColumnStyle, 0.,
						  kColumnLayoutChildrenResultSizes});
}

TEST_CASE (CRowColumnViewTest, ColumnLayoutBottomRightStyle)
{
	testWithLayoutStyle ({CRowColumnView::kBottomRight, CRowColumnView::kColumnStyle, 0.,
						  kColumnLayoutChildrenResultSizes});
}

TEST_CASE (CRowColumnViewTest, RowLayoutMiddleCenterStyleWithSpacing)
{
	testWithLayoutStyle ({CRowColumnView::kMiddleCenter, CRowColumnView::kRowStyle, 4.,
						  kRowLayoutChildrenResultSizesWithSpacing});
}

} // VSTGUI
