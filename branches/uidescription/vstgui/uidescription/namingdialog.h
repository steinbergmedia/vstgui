/*
 *  namingdialog.h
 *
 *  Created by Arne Scheffler on 6/3/09.
 *  Copyright 2009 Arne Scheffler. All rights reserved.
 *
 */

#ifndef __namingdialog__
#define __namingdialog__

#include "../vstgui.h"
#include "platformsupport.h"
#include <string>

/// \cond ignore

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
class NamingDialog : public CBaseObject, public CControlListener, public VSTGUIEditorInterface, public IKeyboardHook
{
public:
	static bool askForName (std::string& result, const char* dialogTitle);

protected:
	NamingDialog (const char* title);
	~NamingDialog ();

	bool run (std::string& result);
	void valueChanged (CControl* pControl);

	long onKeyDown (const VstKeyCode& code, CFrame* frame);
	long onKeyUp (const VstKeyCode& code, CFrame* frame);

	PlatformWindow* platformWindow;
	CTextEdit* textEdit;
	bool result;
};

END_NAMESPACE_VSTGUI

/// \endcond ignore

#endif
