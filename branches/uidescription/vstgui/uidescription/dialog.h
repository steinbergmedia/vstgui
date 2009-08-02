/*
 *  dialog.h
 *  VST3PlugIns
 *
 *  Created by Arne Scheffler on 7/16/09.
 *  Copyright 2009 Arne Scheffler. All rights reserved.
 *
 */

#ifndef __dialog__
#define __dialog__

#if VSTGUI_LIVE_EDITING

#include "../vstgui.h"
#include "platformsupport.h"
#include <string>

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
class Dialog : public CBaseObject, public CControlListener, public VSTGUIEditorInterface, public IKeyboardHook
{
public:
	enum {
		kOkCancelButtons = 0,
		kOkButton = 1
	};

	static bool runViewModal (CPoint& position, CView* view, long style = kOkButton, const char* title = 0);

protected:
	Dialog (const CPoint& position, CView* rootView, long style = kOkButton, const char* title = 0);
	~Dialog ();

	bool run ();
	void valueChanged (CControl* pControl);

	long onKeyDown (const VstKeyCode& code, CFrame* frame);
	long onKeyUp (const VstKeyCode& code, CFrame* frame);

	PlatformWindow* platformWindow;
	bool result;
};

END_NAMESPACE_VSTGUI

#endif // VSTGUI_LIVE_EDITING

#endif
