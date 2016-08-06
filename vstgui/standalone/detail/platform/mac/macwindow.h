#pragma once

#import "../../../interface.h"
#import "../iplatformwindow.h"

@class NSWindow;

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Mac {

//------------------------------------------------------------------------
class IMacWindow : public IWindow
{
public:
	virtual NSWindow* getNSWindow () const = 0;
	virtual bool isPopup () const = 0;
};

//------------------------------------------------------------------------
} // Mac
} // Platform
} // Standalone
} // VSTGUI
