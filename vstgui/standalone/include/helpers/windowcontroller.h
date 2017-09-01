// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iwindowcontroller.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
/** %Window controller adapter
 *
 *	@ingroup standalone
 */
class WindowControllerAdapter : public IWindowController
{
public:
	void onSizeChanged (const IWindow& window, const CPoint& newSize) override {}
	void onPositionChanged (const IWindow& window, const CPoint& newPosition) override {}
	void onShow (const IWindow& window) override {}
	void onHide (const IWindow& window) override {}
	void onClosed (const IWindow& window) override {}
	void onActivated (const IWindow& window) override {}
	void onDeactivated (const IWindow& window) override {}
	CPoint constraintSize (const IWindow& window, const CPoint& newSize) override
	{
		return newSize;
	}
	bool canClose (const IWindow& window) override { return true; }
	void beforeShow (IWindow& window) override {}
	void onSetContentView (IWindow& window, const SharedPointer<CFrame>& contentView) override {}
	IMenuBuilder* getWindowMenuBuilder (const IWindow& window) const override { return nullptr; }
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
