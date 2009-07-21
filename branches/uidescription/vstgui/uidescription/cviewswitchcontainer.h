/*
 *  cviewswitchcontainer.h
 *
 *  Created by Arne Scheffler on 7/19/09.
 *  Copyright 2009 Arne Scheffler. All rights reserved.
 *
 */

#ifndef __cviewswitchcontainer__
#define __cviewswitchcontainer__

#include "../vstgui.h"
#include "viewfactory.h"
#include <vector>

BEGIN_NAMESPACE_VSTGUI
class IViewSwitchController;

//-----------------------------------------------------------------------------
class CViewSwitchContainer : public CViewContainer
{
public:
	CViewSwitchContainer (const CRect& size);
	~CViewSwitchContainer ();

	IViewSwitchController* getController () const { return controller; }
	void setController (IViewSwitchController* controller);	// owns controller if it is a CBaseObject

	void setCurrentViewIndex (long viewIndex);
	long getCurrentViewIndex () const { return currentViewIndex; }

	// overrides, views can not be directly added or removed, use the controller
	bool addView (CView *pView) { return false; }
	bool addView (CView *pView, CRect &mouseableArea, bool mouseEnabled = true) { return false; }
	bool addView (CView *pView, CView* pBefore) { return false; }
	bool removeView (CView *pView, const bool &withForget = true) { return false; }
	bool removeAll (const bool &withForget = true) { return false; }

//-----------------------------------------------------------------------------
	CLASS_METHODS (CViewSwitchContainer, CViewContainer)
protected:
	bool attached (CView* parent);

	IViewSwitchController* controller;
	long currentViewIndex;
};

//-----------------------------------------------------------------------------
class IViewSwitchController
{
public:
	IViewSwitchController (CViewSwitchContainer* viewSwitch) : viewSwitch (viewSwitch) {}
	virtual ~IViewSwitchController () {}

	void init () { viewSwitch->setController (this); }

	CViewSwitchContainer* getViewSwitchContainer () const { return viewSwitch; }

	virtual CView* createViewForIndex (long index) = 0;
	virtual void switchContainerAttached () {}
protected:
	CViewSwitchContainer* viewSwitch;
};

//-----------------------------------------------------------------------------
class UIDescriptionViewSwitchController : public CBaseObject, public IViewSwitchController, public CControlListener
{
public:
	UIDescriptionViewSwitchController (CViewSwitchContainer* viewSwitch, UIDescription* uiDescription, IController* uiController);

	CView* createViewForIndex (long index);
	void switchContainerAttached ();

	void valueChanged (CControl* pControl);

	void setTemplateNames (const char* templateNames); // comma separated
	void getTemplateNames (std::string& str); // comma separated

	void setSwitchControlTag (long tag) { switchControlTag = tag; }
	long getSwitchControlTag () const { return switchControlTag; }
protected:
	UIDescription* uiDescription;
	IController* uiController;
	long switchControlTag;
	std::vector<std::string> templateNames;
};

END_NAMESPACE_VSTGUI

#endif
