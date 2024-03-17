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

static const std::map<CRowColumnView::LayoutStyle, Rects> kChildrenResultSizes = {
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
// clang-format on

auto testWithLayoutStyle (const CRowColumnView::LayoutStyle layoutStyle) -> void
{
	auto rowColumnView = owned (new CRowColumnView (layoutSize));
	rowColumnView->setStyle (CRowColumnView::kRowStyle);
	rowColumnView->setLayoutStyle (layoutStyle);

	for (auto& rect : childrenDefaultSizes)
	{
		auto child = new CView (rect);
		rowColumnView->CViewContainer::addView (child);
		rowColumnView->layoutViews ();
	}

	// rowColumnView->layoutViews();
	size_t i = 0;
	rowColumnView->forEachChild ([&] (CView* child) {
		const auto& results = kChildrenResultSizes.find (layoutStyle);
		const auto& childrenResults = results->second;
		auto viewSize = child->getViewSize ();
		EXPECT (viewSize == childrenResults.at (i))
		i++;
	});
}

TEST_CASE (CRowColumnViewTest, LayoutTopLeftStyle)
{
	// testWithLayoutStyle(CRowColumnView::kTopLeft);
}

TEST_CASE (CRowColumnViewTest, LayoutTopCenterStyle)
{
	testWithLayoutStyle (CRowColumnView::kTopCenter);
}

TEST_CASE (CRowColumnViewTest, LayoutTopRightStyle)
{
	testWithLayoutStyle (CRowColumnView::kTopRight);
}

TEST_CASE (CRowColumnViewTest, LayoutMiddleLeftStyle)
{
	testWithLayoutStyle (CRowColumnView::kMiddleLeft);
}

TEST_CASE (CRowColumnViewTest, LayoutMiddleCenterStyle)
{
	testWithLayoutStyle (CRowColumnView::kMiddleCenter);
}

TEST_CASE (CRowColumnViewTest, LayoutMiddleRightStyle)
{
	testWithLayoutStyle (CRowColumnView::kMiddleRight);
}

TEST_CASE (CRowColumnViewTest, LayoutBottomLeftStyle)
{
	testWithLayoutStyle (CRowColumnView::kBottomLeft);
}

TEST_CASE (CRowColumnViewTest, LayoutBottomCenterStyle)
{
	testWithLayoutStyle (CRowColumnView::kBottomCenter);
}

TEST_CASE (CRowColumnViewTest, LayoutBottomRightStyle)
{
	testWithLayoutStyle (CRowColumnView::kBottomRight);
}

} // VSTGUI
