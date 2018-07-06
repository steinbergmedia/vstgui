// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "x11platform.h"
#include "../../cfileselector.h"
#include "../../cframe.h"
#include "../../cstring.h"
#include "x11frame.h"
#include "cairobitmap.h"
#include <cassert>
#include <chrono>
#include <dlfcn.h>
#include <gtkmm.h>
#include <iostream>
#include <link.h>

//------------------------------------------------------------------------
namespace VSTGUI {

struct X11FileSelector : CNewFileSelector
{
	X11FileSelector (CFrame* parent, Style style) : CNewFileSelector (parent), style (style) {}

	bool runInternal (CBaseObject* delegate) override
	{
		this->delegate = delegate;
		return false;
	}

	void cancelInternal () override {}

	bool runModalInternal () override { return false; }

	Style style;
	SharedPointer<CBaseObject> delegate;
};

//------------------------------------------------------------------------
CNewFileSelector* CNewFileSelector::create (CFrame* parent, Style style)
{
	return new X11FileSelector (parent, style);
}

//------------------------------------------------------------------------
namespace X11 {

//------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
Platform& Platform::getInstance ()
{
	static Platform gInstance;
	return gInstance;
}

//------------------------------------------------------------------------
Platform::Platform ()
{
	Cairo::Bitmap::setGetResourcePathFunc ([this]() {
		auto path = getPath ();
		path += "/Contents/Resources/";
		return path;
	});
}

//------------------------------------------------------------------------
Platform::~Platform ()
{
	Cairo::Bitmap::setGetResourcePathFunc ([]() { return std::string (); });
}

//------------------------------------------------------------------------
uint64_t Platform::getCurrentTimeMs ()
{
	using namespace std::chrono;
	return duration_cast<milliseconds> (steady_clock::now ().time_since_epoch ()).count ();
}

//------------------------------------------------------------------------
std::string Platform::getPath ()
{
	if (path.empty () && soHandle)
	{
		struct link_map* map;
		if (dlinfo (soHandle, RTLD_DI_LINKMAP, &map) == 0)
		{
			path = map->l_name;
			for (int i = 0; i < 3; i++)
			{
				int delPos = path.find_last_of ('/');
				if (delPos == -1)
				{
					fprintf (stderr, "Could not determine bundle location.\n");
					return {}; // unexpected
				}
				path.erase (delPos, path.length () - delPos);
			}
			auto rp = realpath (path.data (), nullptr);
			path = rp;
			free (rp);
		}
	}
	return path;
}

//------------------------------------------------------------------------
} // X11
} // VSTGUI
