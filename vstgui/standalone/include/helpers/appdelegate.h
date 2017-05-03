// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iappdelegate.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Application {

//------------------------------------------------------------------------
/** %Application delegate adapter
 *
 *	@ingroup standalone
 */
class DelegateAdapter : public IDelegate
{
public:
	DelegateAdapter (Info&& info) : appInfo (std::move (info)) {}

	void finishLaunching () override {}
	void onQuit () override {}
	bool canQuit () override { return true; }
	void showAboutDialog () override {}
	bool hasAboutDialog () override { return false; }
	void showPreferenceDialog () override {}
	bool hasPreferenceDialog () override { return false; }
	const Info& getInfo () const override { return appInfo; }
	UTF8StringPtr getSharedUIResourceFilename () const override { return nullptr; }
	bool openFiles (const std::vector<UTF8String>& paths) override { return false; }
//------------------------------------------------------------------------
private:
	Info appInfo;
};

//------------------------------------------------------------------------
} // Application
} // Standalone
} // VSTGUI
