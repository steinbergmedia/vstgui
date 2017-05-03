// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstgui/standalone/include/helpers/windowlistener.h"

//------------------------------------------------------------------------
namespace MyApp {

//------------------------------------------------------------------------
class About : public VSTGUI::Standalone::WindowListenerAdapter
{
public:
	static void show ();

private:
	using Ptr = std::unique_ptr<About>;
	static Ptr gInstance;

	using IWindow = VSTGUI::Standalone::IWindow;
	using WindowPtr = VSTGUI::Standalone::WindowPtr;

	void onClosed (const IWindow& window) override;

	WindowPtr window;
};

//------------------------------------------------------------------------
} // MyApp
