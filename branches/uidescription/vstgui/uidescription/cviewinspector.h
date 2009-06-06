/*
 *  cviewinspector.h
 *
 *  Created by Arne Scheffler on 5/10/09.
 *  Copyright 2009 Arne Scheffler. All rights reserved.
 *
 */

#ifndef __cviewinspector__
#define __cviewinspector__

#if VSTGUI_LIVE_EDITING

#include "../vstgui.h"
#include "uidescription.h"
#include "ceditframe.h"
#include "platformsupport.h"
#include <list>

BEGIN_NAMESPACE_VSTGUI

class CSelection;
class PlatformWindow;
class CScrollView;

//-----------------------------------------------------------------------------
class CViewInspector : public VSTGUIEditorInterface, public CControlListener, public CBaseObject, public IPlatformWindowDelegate
{
public:
	CViewInspector (CSelection* selection, IActionOperator* actionOperator);
	~CViewInspector ();

	void show ();
	void hide ();
	bool isVisible () { return platformWindow ? true : false; }

	void setUIDescription (UIDescription* desc);

	void valueChanged (CControl* pControl);
	CMessageResult notify (CBaseObject* sender, const char* message);
protected:
	CView* createAttributesView (CCoord width);
	void updateAttributeViews ();
	CView* createViewForAttribute (const std::string& attrName, CCoord width);
	void updateAttributeValueView (const std::string& attrName);

	void windowSizeChanged (const CRect& newSize, PlatformWindow* platformWindow);
	void windowClosed (PlatformWindow* platformWindow);
	void checkWindowSizeConstraints (CPoint& size, PlatformWindow* platformWindow);

	CSelection* selection;
	IActionOperator* actionOperator;
	UIDescription* description;
	CFrame* frame;
	CScrollView* scrollView;
	PlatformWindow* platformWindow;
	CRect windowSize;
	std::list<CView*> attributeViews;
};

END_NAMESPACE_VSTGUI

#endif // VSTGUI_LIVE_EDITING

#endif
