// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "autosizeviewlayouter.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
SharedPointer<IViewLayouter> AutoSizeViewLayouter::get () noexcept
{
	static AutoSizeViewLayouter instance;
	return {&instance};
}

//------------------------------------------------------------------------
std::optional<ViewLayout> AutoSizeViewLayouter::calculateLayout (const CViewContainer& container,
																 const Children& children,
																 const CRect& newSize)
{
	if (container.getAutosizingEnabled () == false)
		return {ViewLayout {newSize, LayoutData {}}};

	auto oldSize = container.getViewSize ();
	auto widthDelta = newSize.getWidth () - oldSize.getWidth ();
	auto heightDelta = newSize.getHeight () - oldSize.getHeight ();
	container.getTransform ().inverse ().transform (widthDelta, heightDelta);
	if (widthDelta == 0 && heightDelta == 0)
		return {ViewLayout {newSize, LayoutData {}}};

	LayoutData layoutData;

	auto numSubviews = children.size ();
	auto counter = 0u;
	auto treatAsColumn = (container.getAutosizeFlags () & kAutosizeColumn) != 0;
	auto treatAsRow = (container.getAutosizeFlags () & kAutosizeRow) != 0;

	layoutData.reserve (numSubviews);

	for (const auto& child : children)
	{
		int32_t autosize = child->getAutosizeFlags ();
		CRect viewSize (child->getViewSize ());
		CRect mouseSize (child->getMouseableArea ());
		if (treatAsColumn)
		{
			if (counter)
			{
				viewSize.offset (counter * (widthDelta / (numSubviews)), 0);
				mouseSize.offset (counter * (widthDelta / (numSubviews)), 0);
			}
			viewSize.setWidth (viewSize.getWidth () + (widthDelta / (numSubviews)));
			mouseSize.setWidth (mouseSize.getWidth () + (widthDelta / (numSubviews)));
		}
		else if (widthDelta != 0 && autosize & kAutosizeRight)
		{
			viewSize.right += widthDelta;
			mouseSize.right += widthDelta;
			if (!(autosize & kAutosizeLeft))
			{
				viewSize.left += widthDelta;
				mouseSize.left += widthDelta;
			}
		}
		if (treatAsRow)
		{
			if (counter)
			{
				viewSize.offset (0, counter * (heightDelta / (numSubviews)));
				mouseSize.offset (0, counter * (heightDelta / (numSubviews)));
			}
			viewSize.setHeight (viewSize.getHeight () + (heightDelta / (numSubviews)));
			mouseSize.setHeight (mouseSize.getHeight () + (heightDelta / (numSubviews)));
		}
		else if (heightDelta != 0 && autosize & kAutosizeBottom)
		{
			viewSize.bottom += heightDelta;
			mouseSize.bottom += heightDelta;
			if (!(autosize & kAutosizeTop))
			{
				viewSize.top += heightDelta;
				mouseSize.top += heightDelta;
			}
		}
		if (viewSize != child->getViewSize ())
		{
			std::optional<ViewLayout> childLayout;
			if (auto childContainer = child->asViewContainer ())
				childLayout = childContainer->calculateViewLayout (viewSize);
			layoutData.push_back (
				{child->getRuntimeID (), viewSize, mouseSize, std::move (childLayout)});
		}
		counter++;
	}

	return std::make_optional (ViewLayout {newSize, std::move (layoutData)});
}

//------------------------------------------------------------------------
// there's only one stateless static instance of this object so disable reference counting:
void AutoSizeViewLayouter::forget () {}
void AutoSizeViewLayouter::remember () {};

//------------------------------------------------------------------------
} // VSTGUI
