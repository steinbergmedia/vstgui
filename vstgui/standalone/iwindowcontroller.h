#pragma once

#include "iwindowlistener.h"

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
	virtual CPoint constraintSize (const IWindow& window, const CPoint& newSize) = 0;
	virtual bool canClose (const IWindow& window) const = 0;
	virtual void beforeShow (IWindow& window) = 0;
	virtual void onSetContentView (IWindow& window, const SharedPointer<CFrame>& contentView) = 0;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
