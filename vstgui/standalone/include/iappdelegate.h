// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "fwd.h"
#include "../../lib/cstring.h"
#include "interface.h"
#include <memory>
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Application {

//------------------------------------------------------------------------
/** %Application info.
 *
 *	@ingroup standalone
 */
struct Info
{
	/** Name of the application */
	UTF8String name;
	/** Version of the application */
	UTF8String version;
	/** Uniform resource identifier for the application */
	UTF8String uri;
};

//------------------------------------------------------------------------
/** %Application delegate interface.
 *
 *	Every VSTGUI application needs a delegate. It's a global instance which handles
 *	custom application behaviour.
 *
 *	You define it via Application::Init (std::make_unique<YourDelegateClassType> ())
 *
 *	@ingroup standalone
 */
class IDelegate : public Interface
{
public:
	/** Called when the application has finished launching. */
	virtual void finishLaunching () = 0;
	/** Called when the application is terminating. */
	virtual void onQuit () = 0;
	/** Called to check if it is currently possible to quit. */
	virtual bool canQuit () = 0;
	/** The delegate should show the about dialog. */
	virtual void showAboutDialog () = 0;
	/** Is there an about dialog ? */
	virtual bool hasAboutDialog () = 0;
	/** The delegate should show the preference dialog. */
	virtual void showPreferenceDialog () = 0;
	/** Is there a preference dialog ? */
	virtual bool hasPreferenceDialog () = 0;
	/** Get the application info. */
	virtual const Info& getInfo () const = 0;
	/** Get the filename of the shared UI resources.
	 *
	 *	If this returns a name than all the UI resources are shared between
	 *	different uidesc files. If this returns a nullptr, every uidesc file
	 *	has its own resources.
	 */
	virtual UTF8StringPtr getSharedUIResourceFilename () const = 0;
	/** Called when the system wants the app to open files
	 *
	 *	@param paths UTF-8 encoded paths to the files
	 *	@return true on success
	 */
	virtual bool openFiles (const std::vector<UTF8String>& paths) = 0;
};

//------------------------------------------------------------------------
} // Application
} // Standalone
} // VSTGUI
