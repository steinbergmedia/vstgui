// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

/**

@defgroup standalone Standalone Library

List of classes for the standalone library.

See @ref standalone_library "this page" for an introduction.

@page standalone_library Standalone Library

@tableofcontents

@section standalone_about About

@note The Standalone Library is a preview. The API may change in the future !

The standalone library adds a minimal set of classes to write simple cross-platform UI applications.
See @ref standalone "this page" for a list of classes.

Here's a minimal sample just showing one window:

@code{.cpp}

#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/iuidescwindow.h"
#include "vstgui/standalone/include/helpers/appdelegate.h"
#include "vstgui/standalone/include/helpers/windowlistener.h"

using namespace VSTGUI::Standalone;
using namespace VSTGUI::Standalone::Application;

class MyApplication : public DelegateAdapter, public WindowListenerAdapter
{
public:
    MyApplication ()
    : DelegateAdapter ({"simple_standalone", "1.0.0", "com.mycompany.simplestandalone"})
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

static Init gAppDelegate (std::make_unique<MyApplication> ());

@endcode

Adding a real user interface to the window is done via a "What You See Is What You Get" editor
at runtime as known from the VST3 inline editor. Bindings are done via the
@link VSTGUI::Standalone::UIDesc::IModelBinding IModelBinding @endlink interface.

@section standalone_modelbinding Bindings

Binding the user interface with your code is done via @link VSTGUI::Standalone::IValue IValue @endlink objects. A list
of value objects are exposed via an object that implements the
@link VSTGUI::Standalone::UIDesc::IModelBinding IModelBinding @endlink interface.

As an example implementation there is the @link VSTGUI::Standalone::UIDesc::ModelBindingCallbacks ModelBindingCallbacks @endlink class in the helpers sub directory.
For example you want to have a button in the UI which triggers a function, you can implement it
like this :

@code{.cpp}

auto binding = UIDesc::ModelBindingCallbacks::make ();
binding->addValue (Value::make ("MyFunction"),
                   UIDesc::ValueCalls::onAction ([&] (IValue& v) {
                        executeMyFunction ();
                   }));
config.modelBinding = binding;

@endcode

After you have set the binding as the @link VSTGUI::Standalone::UIDesc::Config::modelBinding UIDesc::Config::modelBinding @endlink parameter
when you create the window, you can start your program, enable the inline editor, create a button
and bind that button to the "control-tag" of "MyFunction" as written in the above code. When the
button is clicked, the function 'executeMyFunction' is called.

@section standalone_window_customization Customization

If you need deeper control of the UI you can supply an object implementing the
@link VSTGUI::Standalone::UIDesc::ICustomization ICustomization @endlink interface as the
@link VSTGUI::Standalone::UIDesc::Config::customization UIDesc::Config::customization @endlink parameter.
In the UI editor you can set the 'sub-controller' attribute of a view container to create a new 
@link VSTGUI::IController IController @endlink object out of your @link VSTGUI::Standalone::UIDesc::ICustomization ICustomization @endlink object. This controller can alter the UI in
many ways, it can even create custom views if you don't want to use the way described in
@link VSTGUI::IViewCreator IViewCreator @endlink to support the view inside the UI editor.

@section standalone_supported_os Supported operating systems

- Microsoft Windows 64bit
    - minimum supported version : 7
- Apple macOS 64bit
    - minimum supported version : 10.10

@section standalone_compiler_requirements Compiler requirements

To compile and use the library you need a compiler supporting most of c++14.

As of this writing the following compilers work (others not tested):
- Visual Studio 15
- Xcode 7.3

*/

//------------------------------------------------------------------------
namespace VSTGUI {
/** %Standalone Library
 *
 *	See @ref standalone_library "this page"
 *	@ingroup new_in_4_5
 */
namespace Standalone {
	
//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
