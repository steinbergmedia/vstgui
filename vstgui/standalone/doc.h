#pragma once

/**

@defgroup standalone Standalone Library

@page standalone_library Standalone Library

- @ref standalone_about @n
- @ref standalone_supported_os @n
- @ref standalone_compiler_requirements @n

@section standalone_about About

The standalone library adds a minimal set of classes to write simple cross-platform applications.

Here's a minimal sample just showing one window:

@code{.cpp}

#include "vstgui/standalone/iapplication.h"
#include "vstgui/standalone/uidescwindow.h"
#include "vstgui/standalone/helpers/appdelegate.h"
#include "vstgui/standalone/helpers/windowlistener.h"

using namespace VSTGUI::Standalone;
using namespace VSTGUI::Standalone::Application;

class MyApplication : public DelegateAdapter, public WindowListenerAdapter
{
public:
	MyApplication ()
	: DelegateAdapter ({"Sample App", "1.0.0", "com.mycompany.sampleapp"})
	{}
	
	void finishLaunching () override
	{
		UIDesc::Config config;
		config.uiDescFileName = "Window.uidesc";
		config.viewName = "Window";
		config.windowConfig.title = "Sample App";
		config.windowConfig.autoSaveFrameName = "SampleAppWindow";
		config.windowConfig.style.border ().close ().size ().centered ();
		if (auto window = UIDesc::makeWindow (config))
		{
			window->show ();
			window->registerWindowListener (this);
		}
		else
		{
			IApplication::instance ().quit ();
		}
	}
	void onClosed (const IWindow& window) override
	{
		IApplication::instance ().quit ();
	}

};

static Init gAppDelegate (std::make_shared<MyApplication> ());

@endcode

@section standalone_supported_os Supported operating systems

- Microsoft Windows
	- minimum supported version : 8.1
- Apple macOS
	- minimum supported version : 10.10

@section standalone_compiler_requirements Compiler requirements

To compile the library you need a compiler supporting most of c++14.

As of this writing these compilers work (others not tested):
- Visual Studio 15
- Xcode 7.3

*/
