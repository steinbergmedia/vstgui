//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2016, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "x11platform.h"
#include "../../cfileselector.h"
#include "../../cframe.h"
#include "../../cstring.h"
#include "x11frame.h"
#include <cassert>
#include <chrono>
#include <dlfcn.h>
#include <gtkmm.h>
#include <iostream>
#include <link.h>

//------------------------------------------------------------------------
namespace VSTGUI {

struct GtkFileSelector : CNewFileSelector
{
	GtkFileSelector (CFrame* parent, Style style)
	: CNewFileSelector (parent), style (style), dialog (Gtk::FileChooserDialog (*getParent (), ""))
	{
	}

	Gtk::Window* getParent () const
	{
		if (!frame)
			return nullptr;
		if (auto x11Frame = dynamic_cast<X11::Frame*> (frame->getPlatformFrame ()))
		{
			return reinterpret_cast<Gtk::Window*> (x11Frame->getGtkWindow ());
		}
		return nullptr;
	}

	static std::string getDirectory (UTF8StringPtr str)
	{
		if (access (str, 0) != 0)
			return {};
		struct stat s;
		if (stat (str, &s) != 0)
			return {};
		if (s.st_mode & S_IFDIR)
			return {str};
		if (s.st_mode & S_IFREG)
		{
			std::string s {str};
			auto pos = s.find_last_of ("/");
			if (pos == std::string::npos)
				return {};
			s.erase (pos);
			return s;
		}
		return {};
	}

	void prepareRunning ()
	{
		if (title)
			dialog.set_title (title);
		dialog.add_button ("_Cancel", 0);
		switch (style)
		{
			case kSelectFile:
			{
				dialog.set_action (Gtk::FILE_CHOOSER_ACTION_OPEN);
				dialog.add_button ("_Open", 1);
				if (allowMultiFileSelection)
				{
					dialog.set_select_multiple (true);
				}
				break;
			}
			case kSelectSaveFile:
			{
				dialog.set_action (Gtk::FILE_CHOOSER_ACTION_SAVE);
				dialog.add_button ("_Save", 1);
				dialog.set_do_overwrite_confirmation (true);
				if (defaultSaveName)
					dialog.set_current_name (defaultSaveName);
				break;
			}
			case kSelectDirectory:
			{
				dialog.set_action (Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
				dialog.add_button ("_Open", 1);
				break;
			}
		}
		dialog.signal_file_activated ().connect ([&] () {
			dialogResult = true;
			dialog.close ();
		});
		dialog.signal_response ().connect ([&] (int code) {
			if (code == 0 || code == 1)
			{
				dialogResult = code == 1;
				dialog.close ();
			}
		});
		if (initialPath)
		{
			auto directory = getDirectory (initialPath);
			if (!directory.empty ())
				dialog.set_current_folder (directory);
		}
		for (auto& ext : extensions)
		{
			auto fileFilter = Gtk::FileFilter::create ();
			if (ext == getAllFilesExtension ())
			{
				fileFilter->set_name (ext.getDescription ());
				fileFilter->add_pattern ("*");
			}
			else
			{
				if (auto desc = ext.getDescription ())
					fileFilter->set_name (desc);
				if (auto mimeType = ext.getMimeType ())
					fileFilter->add_mime_type (mimeType);
				if (auto fileExt = ext.getExtension ())
				{
					Glib::ustring s ("*.");
					s += fileExt;
					fileFilter->add_pattern (s);
				}
			}
			dialog.add_filter (fileFilter);
		}
	}

	void postRun ()
	{
		if (!dialogResult)
			return;
		for (auto str : dialog.get_filenames ())
		{
			result.push_back (String::newWithString (str.data ()));
		}
	}

	bool runInternal (CBaseObject* delegate) override
	{
		this->delegate = delegate;
		prepareRunning ();
		dialog.show ();
		dialog.signal_unmap ().connect ([&] () {
			if (delegate)
			{
				postRun ();
				delegate->notify (this, kSelectEndMessage);
			}
		});
		return true;
	}

	void cancelInternal () override { dialog.close (); }

	bool runModalInternal () override
	{
		prepareRunning ();
		dialog.set_modal (true);
		dialog.show ();

		X11::LocalEventLoop eventLoop;
		dialog.signal_unmap ().connect ([&] () { eventLoop.stop (); });

		eventLoop.run ();

		postRun ();
		return dialogResult;
	}

	Style style;
	Gtk::FileChooserDialog dialog;
	bool dialogResult {false};
	SharedPointer<CBaseObject> delegate;
};

//------------------------------------------------------------------------
CNewFileSelector* CNewFileSelector::create (CFrame* parent, Style style)
{
	return new GtkFileSelector (parent, style);
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
}

//------------------------------------------------------------------------
Platform::~Platform ()
{
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
void LocalEventLoop::run ()
{
	running = true;
	gtk_main ();
	running = false;
}

//------------------------------------------------------------------------
void LocalEventLoop::stop ()
{
	gtk_main_quit ();
}

//------------------------------------------------------------------------
} // X11
} // VSTGUI
