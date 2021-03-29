// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iwindowlistener.h"
#include <functional>

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
/** %Window closed listener
 *
 *	@ingroup standalone
 */
class WindowClosedListener : public WindowListenerAdapter
{
public:
	using Func = std::function<void (const IWindow&)>;

	WindowClosedListener () {}

	template <typename Func>
	WindowClosedListener (Func func) : func (std::forward<Func> (func))
	{
	}

	void onClosed (const IWindow& window) override
	{
		if (func)
		{
			func (window);
			Func e {};
			func.swap (e);
			func = nullptr;
		}
	}

	Func func;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
