#pragma once

#import "../../../interface.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Mac {

//------------------------------------------------------------------------
class IMacWindow : public Interface
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
