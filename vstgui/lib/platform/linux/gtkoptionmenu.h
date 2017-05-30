// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformoptionmenu.h"
#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
class GTKOptionMenu : public IPlatformOptionMenu
{
public:
	GTKOptionMenu (void* parent);
	~GTKOptionMenu ();

	PlatformOptionMenuResult popup (COptionMenu* optionMenu) override;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
}
