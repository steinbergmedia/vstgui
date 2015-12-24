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
};

//------------------------------------------------------------------------
class IWindowController : public IWindowListener
{
public:
	virtual CPoint constraintSize (const IWindow& window, const CPoint& newSize) = 0;
	virtual bool canClose (const IWindow& window) const = 0;
};

//------------------------------------------------------------------------
using WindowControllerPtr = std::shared_ptr<IWindowController>;

//------------------------------------------------------------------------
class WindowControllerAdapter : public IWindowController
{
public:
	void onSizeChanged (const IWindow& window, const CPoint& newSize) override {}
	void onPositionChanged (const IWindow& window, const CPoint& newPosition) override {}
	void onShow (const IWindow& window) override {}
	void onHide (const IWindow& window) override {}
	void onClosed (const IWindow& window) override {}
	CPoint constraintSize (const IWindow& window, const CPoint& newSize) override { return newSize; }
	bool canClose (const IWindow& window) const override { return true; }
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
