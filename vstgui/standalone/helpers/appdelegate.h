#pragma once

#include "../iappdelegate.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Application {

//------------------------------------------------------------------------
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
//------------------------------------------------------------------------
private:
	Info appInfo;
};

//------------------------------------------------------------------------
} // Application
} // Standalone
} // VSTGUI
