#pragma once

#include "../iwindowlistener.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
/** %Window listener adapter
 *
 *	@ingroup standalone
 */
class WindowListenerAdapter : public IWindowListener
{
public:
	void onSizeChanged (const IWindow& window, const CPoint& newSize) override {}
	void onPositionChanged (const IWindow& window, const CPoint& newPosition) override {}
	void onShow (const IWindow& window) override {}
	void onHide (const IWindow& window) override {}
	void onClosed (const IWindow& window) override {}
	void onActivated (const IWindow& window) override {}
	void onDeactivated (const IWindow& window) override {}
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
