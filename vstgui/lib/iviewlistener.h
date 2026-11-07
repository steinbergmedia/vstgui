// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cbuttonstate.h"
#include "cpoint.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
/** @brief View Listener Interface
 */
//-----------------------------------------------------------------------------
class IViewListener
{
public:
	virtual ~IViewListener () noexcept = default;

	/** called when the view's size changed */
	virtual void viewSizeChanged (CView& view, const CRect& oldSize) = 0;
	/** called when a view was attached to a view hierarchy */
	virtual void viewAttached (CView& view) = 0;
	/** called when a view was removed from the view hierarchy */
	virtual void viewRemoved (CView& view) = 0;
	/** called when a view lost focus */
	virtual void viewLostFocus (CView& view) = 0;
	/** called when a view took focus */
	virtual void viewTookFocus (CView& view) = 0;
	/** called when a view is going to be destroyed */
	virtual void viewWillDelete (CView& view) = 0;
	/** called when a view's mouse handling is enabled or disabled
	 * @ingroup new_in_4_11
	 */
	virtual void viewOnMouseEnabled (CView& view, bool state) = 0;
};

//-----------------------------------------------------------------------------
/** @brief View Event Listener Interface
 *
 *	@ingroup new_in_4_11
 */
//-----------------------------------------------------------------------------
class IViewEventListener
{
public:
	virtual ~IViewEventListener () noexcept = default;

	/** called on an event on a view
	 *
	 *	whenever an event is dispatched to a view, the listener will be notified about it and can
	 *	mark the event as consumed if necessary to prevent that the event is handled by the view and
	 *	further dispatched in the view hierarchy.
	 */
	virtual void viewOnEvent (CView& view, Event& event) = 0;
};

//-----------------------------------------------------------------------------
/** @brief ViewContainer Listener Interface
 */
//-----------------------------------------------------------------------------
class IViewContainerListener
{
public:
	virtual ~IViewContainerListener () noexcept = default;

	/** called when a new view was added to the container */
	virtual void viewContainerViewAdded (CViewContainer& container, CView& view) = 0;
	/** called when a view was removed from the container */
	virtual void viewContainerViewRemoved (CViewContainer& container, CView& view) = 0;
	/** called when a view's z-order changed inside the container */
	virtual void viewContainerViewZOrderChanged (CViewContainer& container, CView& view) = 0;
	/** called when the transform matrix of the container changed */
	virtual void viewContainerTransformChanged (CViewContainer& container) = 0;
};

//-----------------------------------------------------------------------------
/** @brief View Listener Interface Adapter
 */
//-----------------------------------------------------------------------------
class ViewListenerAdapter : public IViewListener
{
public:
	void viewSizeChanged (CView& view, const CRect& oldSize) override {}
	void viewAttached (CView& view) override {}
	void viewRemoved (CView& view) override {}
	void viewLostFocus (CView& view) override {}
	void viewTookFocus (CView& view) override {}
	void viewWillDelete (CView& view) override {}
	void viewOnMouseEnabled (CView& view, bool state) override {}
};

//------------------------------------------------------------------------
/** @brief View Event Listener Interface Adapter
 */
class ViewEventListenerAdapter : public IViewEventListener
{
public:
	void viewOnEvent (CView& view, Event& event) override {}
};

//-----------------------------------------------------------------------------
/** @brief ViewContainer Listener Interface Adapter
 */
//-----------------------------------------------------------------------------
class ViewContainerListenerAdapter : public IViewContainerListener
{
public:
	void viewContainerViewAdded (CViewContainer& container, CView& view) override {}
	void viewContainerViewRemoved (CViewContainer& container, CView& view) override {}
	void viewContainerViewZOrderChanged (CViewContainer& container, CView& view) override {}
	void viewContainerTransformChanged (CViewContainer& container) override {}
};

//------------------------------------------------------------------------
struct LegacyViewListenerAdapter : ViewListenerAdapter
{
	void viewSizeChanged (CView& view, const CRect& oldSize) final
	{
		viewSizeChanged (&view, oldSize);
	}
	void viewAttached (CView& view) final { viewAttached (&view); }
	void viewRemoved (CView& view) final { viewRemoved (&view); }
	void viewLostFocus (CView& view) final { viewLostFocus (&view); }
	void viewTookFocus (CView& view) final { viewTookFocus (&view); }
	void viewWillDelete (CView& view) final { viewWillDelete (&view); }
	void viewOnMouseEnabled (CView& view, bool state) final { viewOnMouseEnabled (&view, state); }

	virtual void viewSizeChanged (CView* view, const CRect& oldSize) {}
	virtual void viewAttached (CView* view) {}
	virtual void viewRemoved (CView* view) {}
	virtual void viewLostFocus (CView* view) {}
	virtual void viewTookFocus (CView* view) {}
	virtual void viewWillDelete (CView* view) {}
	virtual void viewOnMouseEnabled (CView* view, bool state) {}
};

//------------------------------------------------------------------------
struct LegacyViewEventListenerAdapter : ViewEventListenerAdapter
{
	void viewOnEvent (CView& view, Event& event) final { viewOnEvent (&view, event); }

	virtual void viewOnEvent (CView* view, Event& event) {}
};

//------------------------------------------------------------------------
struct LegacyViewContainerListenerAdapter : ViewContainerListenerAdapter
{
	void viewContainerViewAdded (CViewContainer& container, CView& view) final
	{
		viewContainerViewAdded (&container, &view);
	}
	void viewContainerViewRemoved (CViewContainer& container, CView& view) final
	{
		viewContainerViewRemoved (&container, &view);
	}
	void viewContainerViewZOrderChanged (CViewContainer& container, CView& view) final
	{
		viewContainerViewZOrderChanged (&container, &view);
	}
	void viewContainerTransformChanged (CViewContainer& container) final
	{
		viewContainerTransformChanged (&container);
	}

	virtual void viewContainerViewAdded (CViewContainer* container, CView* view) {}
	virtual void viewContainerViewRemoved (CViewContainer* container, CView* view) {}
	virtual void viewContainerViewZOrderChanged (CViewContainer* container, CView* view) {}
	virtual void viewContainerTransformChanged (CViewContainer* container) {}
};

//------------------------------------------------------------------------
} // VSTGUI
