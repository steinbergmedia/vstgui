/*
 *  viewhierarchybrowser.h
 *  VST3PlugIns
 *
 *  Created by Arne Scheffler on 6/11/09.
 *  Copyright 2009 Arne Scheffler. All rights reserved.
 *
 */

#ifndef __viewhierarchybrowser__
#define __viewhierarchybrowser__

#if VSTGUI_LIVE_EDITING

#include "../vstgui.h"
#include "uidescription.h"
#include "platformsupport.h"

BEGIN_NAMESPACE_VSTGUI

class CDataBrowser;
class ViewHierarchyData;
class ViewHierarchyPathView;
class IActionOperator;

//-----------------------------------------------------------------------------
class ViewHierarchyBrowser : public CViewContainer
{
public:
	ViewHierarchyBrowser (const CRect& rect, CViewContainer* baseView, UIDescription* description, IActionOperator* actionOperator);
	~ViewHierarchyBrowser ();
	
	void setCurrentView (CViewContainer* newView);
	CViewContainer* getCurrentView () const { return currentView; }
	CViewContainer* getBaseView () const { return baseView; }

	void changeBaseView (CViewContainer* newBaseView);
	void notifyHierarchyChange (CView* view, bool wasRemoved = false);
protected:
	CViewContainer* baseView;
	CViewContainer* currentView;

	CDataBrowser* browser;
	ViewHierarchyData* data;
	ViewHierarchyPathView* pathView;
};

//-----------------------------------------------------------------------------
class ViewHierarchyBrowserWindow : public CBaseObject, public VSTGUIEditorInterface, public IPlatformWindowDelegate
{
public:
	ViewHierarchyBrowserWindow (CViewContainer* baseView, CBaseObject* owner, UIDescription* description);
	~ViewHierarchyBrowserWindow ();

	void changeBaseView (CViewContainer* newBaseView);
	void notifyHierarchyChange (CView* view, bool wasRemoved = false);
	
	static const char* kMsgWindowClosed;
protected:
	void windowSizeChanged (const CRect& newSize, PlatformWindow* platformWindow);
	void windowClosed (PlatformWindow* platformWindow);
	void checkWindowSizeConstraints (CPoint& size, PlatformWindow* platformWindow);

	CBaseObject* owner;
	PlatformWindow* platformWindow;
	ViewHierarchyBrowser* browser;
	UIDescription* description;
};


END_NAMESPACE_VSTGUI

#endif // VSTGUI_LIVE_EDITING

#endif
