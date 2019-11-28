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
	
	virtual void viewSizeChanged (CView* view, const CRect& oldSize) = 0;
	virtual void viewAttached (CView* view) = 0;
	virtual void viewRemoved (CView* view) = 0;
	virtual void viewLostFocus (CView* view) = 0;
	virtual void viewTookFocus (CView* view) = 0;
	virtual void viewWillDelete (CView* view) = 0;
};

//-----------------------------------------------------------------------------
/** @brief ViewContainer Listener Interface
 */
//-----------------------------------------------------------------------------
class IViewContainerListener
{
public:
	virtual ~IViewContainerListener () noexcept = default;

	virtual void viewContainerViewAdded (CViewContainer* container, CView* view) = 0;
	virtual void viewContainerViewRemoved (CViewContainer* container, CView* view) = 0;
	virtual void viewContainerViewZOrderChanged (CViewContainer* container, CView* view) = 0;
	virtual void viewContainerTransformChanged (CViewContainer* container) = 0;
};

//-----------------------------------------------------------------------------
/** @brief View Mouse Listener Interface
 *
 *	@ingroup new_in_4_7
 */
//-----------------------------------------------------------------------------
class IViewMouseListener
{
public:
	virtual ~IViewMouseListener () noexcept = default;

	virtual CMouseEventResult viewOnMouseDown (CView* view, CPoint pos, CButtonState buttons) = 0;
	virtual CMouseEventResult viewOnMouseUp (CView* view, CPoint pos, CButtonState buttons) = 0;
	virtual CMouseEventResult viewOnMouseMoved (CView* view, CPoint pos, CButtonState buttons) = 0;
	virtual CMouseEventResult viewOnMouseCancel (CView* view) = 0;
	virtual void viewOnMouseEntered (CView* view) = 0;
	virtual void viewOnMouseExited (CView* view) = 0;
	virtual void viewOnMouseEnabled (CView* view, bool state) = 0;
};

//-----------------------------------------------------------------------------
/** @brief View Listener Interface Adapter
 */
//-----------------------------------------------------------------------------
class ViewListenerAdapter : public IViewListener
{
public:
	void viewSizeChanged (CView* view, const CRect& oldSize) override {}
	void viewAttached (CView* view) override {}
	void viewRemoved (CView* view) override {}
	void viewLostFocus (CView* view) override {}
	void viewTookFocus (CView* view) override {}
	void viewWillDelete (CView* view) override {}
};

//-----------------------------------------------------------------------------
/** @brief ViewContainer Listener Interface Adapter
 */
//-----------------------------------------------------------------------------
class ViewContainerListenerAdapter : public IViewContainerListener
{
public:
	void viewContainerViewAdded (CViewContainer* container, CView* view) override {}
	void viewContainerViewRemoved (CViewContainer* container, CView* view) override {}
	void viewContainerViewZOrderChanged (CViewContainer* container, CView* view) override {}
	void viewContainerTransformChanged (CViewContainer* container) override {}
};

//-----------------------------------------------------------------------------
/** @brief View Mouse Listener Interface Adapter
 *
 *	@ingroup new_in_4_7
 */
//-----------------------------------------------------------------------------
class ViewMouseListenerAdapter : public IViewMouseListener
{
public:
	CMouseEventResult viewOnMouseDown (CView* view, CPoint pos, CButtonState buttons) override
	{
		return kMouseEventNotImplemented;
	}
	CMouseEventResult viewOnMouseUp (CView* view, CPoint pos, CButtonState buttons) override
	{
		return kMouseEventNotImplemented;
	}
	CMouseEventResult viewOnMouseMoved (CView* view, CPoint pos, CButtonState buttons) override
	{
		return kMouseEventNotImplemented;
	}
	CMouseEventResult viewOnMouseCancel (CView* view) override { return kMouseEventNotImplemented; }
	void viewOnMouseEntered (CView* view) override {}
	void viewOnMouseExited (CView* view) override {}
	void viewOnMouseEnabled (CView* view, bool state) override {}
};

} // VSTGUI
