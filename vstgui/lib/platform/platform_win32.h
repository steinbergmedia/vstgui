// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "iplatformframe.h"

#if WINDOWS

#include "iplatformviewlayer.h"

#include <windows.h>

//-----------------------------------------------------------------------------
namespace VSTGUI {

struct IIMETextInputClient;

//-----------------------------------------------------------------------------
// extens IPlatformFrame on Microsoft Windows
class IWin32PlatformFrame
{
public:
	virtual HWND getHWND () const = 0;
	virtual void setTextInputClient (IIMETextInputClient* client) = 0;
};

//-----------------------------------------------------------------------------
} // VSTGUI

#endif

