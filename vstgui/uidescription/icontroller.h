// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../lib/controls/icontrollistener.h"
#include "../lib/cviewcontainer.h"

namespace VSTGUI {

class UIAttributes;
class IUIDescription;

//-----------------------------------------------------------------------------
/// @brief extension to IControlListener used by UIDescription
/// @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class IController : public IControlListener
{
public:
	virtual int32_t getTagForName (UTF8StringPtr name, int32_t registeredTag) const { return registeredTag; }
	virtual IControlListener* getControlListener (UTF8StringPtr controlTagName) { return this; }
	virtual CView* createView (const UIAttributes& attributes, const IUIDescription* description) { return nullptr; }
	virtual CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) { return view; }
	virtual IController* createSubController (UTF8StringPtr name, const IUIDescription* description) { return nullptr; }
};

//-----------------------------------------------------------------------------
//! @brief extension to IController
//!			The VST3Editor checks all controllers of the views under the mouse on a right click if they have
//!			this interface implemented and calls the appendContextMenuItems before showing the context menu to the user
//! @ingroup new_in_4_3
//-----------------------------------------------------------------------------
class IContextMenuController
{
public:
	virtual ~IContextMenuController () noexcept = default;
	
	virtual void appendContextMenuItems (COptionMenu& contextMenu, const CPoint& where) = 0;
};

//-----------------------------------------------------------------------------
class IContextMenuController2
{
public:
	virtual ~IContextMenuController2 () noexcept = default;
	
	virtual void appendContextMenuItems (COptionMenu& contextMenu, CView* view, const CPoint& where) = 0;
};

//-----------------------------------------------------------------------------
/** helper method to get the controller of a view */
inline IController* getViewController (const CView* view, bool deep = false)
{
	IController* controller = nullptr;
	if (!view->getAttribute (kCViewControllerAttribute, controller) && deep)
	{
		if (view->getParentView () && view->getParentView () != view)
		{
			return getViewController (view->getParentView (), deep);
		}
	}
	return controller;
}

//-----------------------------------------------------------------------------
/** helper method to find a specific controller inside a view hierarchy */
template<typename T>
inline T* findViewController (const CViewContainer* view)
{
	if (auto ctrler = dynamic_cast<T*> (getViewController (view)))
		return ctrler;
	ViewIterator iterator (view);
	while (*iterator)
	{
		if (auto ctrler = dynamic_cast<T*> (getViewController (*iterator)))
			return ctrler;
		if (auto container = (*iterator)->asViewContainer ())
		{
			if (auto ctrler = findViewController<T> (container))
				return ctrler;
		}
		++iterator;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
} // VSTGUI
