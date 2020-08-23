// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "iwindowlistener.h"
#include "../../lib/platform/iplatformframecallback.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
/** Window controller interface
 *
 *	@ingroup standalone
 */
class IWindowController : public IWindowListener
{
public:
	/** Constraint the size of the window. */
	virtual CPoint constraintSize (const IWindow& window, const CPoint& newSize) = 0;
	/** Can window close? */
	virtual bool canClose (const IWindow& window) = 0;
	/** Window will show. */
	virtual void beforeShow (IWindow& window) = 0;
	/** Create the platform frame configuration object. Can be nullptr. */
	virtual PlatformFrameConfigPtr createPlatformFrameConfig (PlatformType platformType) = 0;
	/** Content view of window is changed. */
	virtual void onSetContentView (IWindow& window, const SharedPointer<CFrame>& contentView) = 0;
	/** Get the menu builder for this window. */
	virtual const IMenuBuilder* getWindowMenuBuilder (const IWindow& window) const = 0;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
