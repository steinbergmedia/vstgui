/*
 *  platformsupport.h
 *
 *  Created by Arne Scheffler on 5/10/09.
 *  Copyright 2009 Arne Scheffler. All rights reserved.
 *
 */

#ifndef __platformsupport__
#define __platformsupport__

#include "../vstgui.h"
#include <string>
#include <list>

BEGIN_NAMESPACE_VSTGUI

class PlatformWindow;

//-----------------------------------------------------------------------------
class IPlatformWindowDelegate
{
public:
	virtual void windowSizeChanged (const CRect& newSize, PlatformWindow* platformWindow) = 0;
	virtual void windowClosed (PlatformWindow* platformWindow) = 0;
	virtual void checkWindowSizeConstraints (CPoint& size, PlatformWindow* platformWindow) = 0;
};

//-----------------------------------------------------------------------------
class PlatformWindow : public CBaseObject
{
public:
	enum WindowType {
		kPanelType,
		kWindowType
	};
	enum WindowStyleFlags {
		kClosable = 1 << 0,
		kResizable  = 1 << 1,
	};
	
	static PlatformWindow* create (const CRect& size, const char* title = 0, WindowType type = kPanelType, long styleFlags = 0, IPlatformWindowDelegate* delegate = 0);
	
	virtual void* getPlatformHandle () const = 0;
	virtual void show () = 0;
	virtual void center () = 0;
	virtual CRect getSize () = 0;
	virtual void setSize (const CRect& size) = 0;
	
	virtual void runModal () = 0;
	virtual void stopModal () = 0;
};

//-----------------------------------------------------------------------------
class IPlatformColorChangeCallback
{
public:
	virtual void colorChanged (const CColor& color) = 0;
};

//-----------------------------------------------------------------------------
class PlatformUtilities
{
public:
	static bool collectPlatformFontNames (std::list<std::string*>& fontNames);
	static bool startDrag (CFrame* frame, const CPoint& location, const char* string, CBitmap* dragBitmap, bool localOnly = true);
	static void colorChooser (const CColor* oldColor, IPlatformColorChangeCallback* callback);
};

END_NAMESPACE_VSTGUI

#endif
