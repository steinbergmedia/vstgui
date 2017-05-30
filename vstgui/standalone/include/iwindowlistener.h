// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "fwd.h"
#include "../../lib/cpoint.h"
#include "interface.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
/** Window listener interface
 *
 *	@ingroup standalone
 */
class IWindowListener : public Interface
{
public:
	/** Size of window is changed. */
	virtual void onSizeChanged (const IWindow& window, const CPoint& newSize) = 0;
	/** Position of window is changed. */
	virtual void onPositionChanged (const IWindow& window, const CPoint& newPosition) = 0;
	/** Window is shown. */
	virtual void onShow (const IWindow& window) = 0;
	/** Window is hidden. */
	virtual void onHide (const IWindow& window) = 0;
	/** Window is closed. */
	virtual void onClosed (const IWindow& window) = 0;
	/** Window is activated. */
	virtual void onActivated (const IWindow& window) = 0;
	/** Window is deactivated. */
	virtual void onDeactivated (const IWindow& window) = 0;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
