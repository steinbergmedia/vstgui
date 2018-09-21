// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformwindow.h"

extern "C"
{
	typedef union _GdkEvent GdkEvent;
	typedef struct _GdkWindow GdkWindow;
};

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace GDK {

//------------------------------------------------------------------------
class IGdkWindow : public Interface
{
public:
};

//------------------------------------------------------------------------
} // GDK
} // Platform
} // Standalone
} // VSTGUI
