#pragma once

#include "vstgui/standalone/iwindowcontroller.h"
#include "vstgui/standalone/ivalue.h"

//------------------------------------------------------------------------
namespace MyApp {

//------------------------------------------------------------------------
class About : public VSTGUI::Standalone::WindowListenerAdapter, public VSTGUI::Standalone::ValueListenerAdapter
{
public:
	static void show ();
	
private:
	static std::unique_ptr<About> gInstance;

	using IWindow = VSTGUI::Standalone::IWindow;
	using IValue = VSTGUI::Standalone::IValue;
	using WindowPtr = VSTGUI::Standalone::WindowPtr;

	void onClosed (const IWindow& window) override;
	void onEndEdit (const IValue& value) override;

	WindowPtr window;
};

//------------------------------------------------------------------------
} // MyApp
