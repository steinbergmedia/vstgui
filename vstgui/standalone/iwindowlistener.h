#pragma once

#include "fwd.h"
#include "interface.h"
#include "../lib/cpoint.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
class IWindowListener : public Interface
{
public:
	virtual void onSizeChanged (const IWindow& window, const CPoint& newSize) = 0;
	virtual void onPositionChanged (const IWindow& window, const CPoint& newPosition) = 0;
	virtual void onShow (const IWindow& window) = 0;
	virtual void onHide (const IWindow& window) = 0;
	virtual void onClosed (const IWindow& window) = 0;
	virtual void onActivated (const IWindow& window) = 0;
	virtual void onDeactivated (const IWindow& window) = 0;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
