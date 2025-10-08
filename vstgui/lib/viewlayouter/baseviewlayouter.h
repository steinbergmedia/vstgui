// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iviewlayouter.h"
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
struct BaseViewLayouterEntry
{
	uint64_t viewId {};
	CRect viewSize {};
	CRect mouseSize {};
	std::optional<ViewLayout> childLayout {};
};

//------------------------------------------------------------------------
/** a base class for view layouters
 *
 *	@ingroup new_in_4_15
 */
class BaseViewLayouter : public IViewLayouter
{
public:
	using LayoutData = std::vector<BaseViewLayouterEntry>;

	bool applyLayout (CViewContainer& container, const Children& children,
					  const ViewLayout& layout) override;
};

//------------------------------------------------------------------------
inline bool BaseViewLayouter::applyLayout (CViewContainer& container, const Children& children,
										   const ViewLayout& layout)
{
	auto layoutData = std::any_cast<const LayoutData> (&layout.data);
	if (layoutData == nullptr)
	{
		vstgui_assert (false, "BaseViewLayouter: Unexpected layout data");
		return false;
	}
	auto childIt = children.begin ();
	auto childEnd = children.end ();
	for (const auto& entry : *layoutData)
	{
		while (childIt != childEnd && (*childIt)->getRuntimeID () != entry.viewId)
			++childIt;

		if (childIt == childEnd)
			continue;

		auto doSetViewSize = true;
		auto& child = *childIt;
		if (auto childContainer = child->asViewContainer ())
		{
			if (entry.childLayout)
			{
				if (!childContainer->applyViewLayout (*entry.childLayout))
					return false;
				doSetViewSize = false;
			}
		}
		if (doSetViewSize)
		{
			child->setViewSize (entry.viewSize, true);
			child->setMouseableArea (entry.mouseSize == entry.viewSize ? child->getViewSize ()
																	   : entry.mouseSize);
		}
	}
	container.setViewSize (layout.size);
	container.setMouseableArea (layout.size);
	return true;
}

//------------------------------------------------------------------------
} // VSTGUI
