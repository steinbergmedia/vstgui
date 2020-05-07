// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstgui/lib/cviewcontainer.h"
#include "vstgui/lib/iviewlistener.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Minesweeper {

//------------------------------------------------------------------------
inline void keepChildViewsCentered (CViewContainer* view)
{
//------------------------------------------------------------------------
	class CenteredViewHandler : public ViewListenerAdapter
	{
	public:
		CenteredViewHandler (CViewContainer* container) : parent (container)
		{
			parent->registerViewListener (this);
		}

		void viewSizeChanged (CView* view, const CRect& oldSize) override
		{
			if (!view->isAttached ())
				return;
			auto viewSize = view->getViewSize ();
			auto diffX = viewSize.getWidth () - oldSize.getWidth ();
			auto diffY = viewSize.getHeight () - oldSize.getHeight ();
			if (diffX == 0. && diffY == 0.)
				return;
			diffX *= 0.5;
			diffY *= 0.5;
			parent->forEachChild ([&] (auto& child) {
				auto vs = child->getViewSize ();
				vs.offset (diffX, diffY);
				child->setViewSize (vs);
				child->setMouseableArea (vs);
			});
		}

	private:
		void viewWillDelete (CView* view) override
		{
			view->unregisterViewListener (this);
			delete this;
		}

		CViewContainer* parent {nullptr};
	};

	new CenteredViewHandler (view);
}

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
