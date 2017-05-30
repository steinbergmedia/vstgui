// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __iviewlistener__
#define __iviewlistener__

#include "vstguifwd.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
/// @brief View Listener Interface
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
/// @brief ViewContainer Listener Interface
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
/// @brief View Listener Interface Adapter
//-----------------------------------------------------------------------------
class IViewListenerAdapter : public IViewListener
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
/// @brief ViewContainer Listener Interface Adapter
//-----------------------------------------------------------------------------
class IViewContainerListenerAdapter : public IViewContainerListener
{
public:
	void viewContainerViewAdded (CViewContainer* container, CView* view) override {}
	void viewContainerViewRemoved (CViewContainer* container, CView* view) override {}
	void viewContainerViewZOrderChanged (CViewContainer* container, CView* view) override {}
	void viewContainerTransformChanged (CViewContainer* container) override {}
};

}


#endif // __iviewlistener__
