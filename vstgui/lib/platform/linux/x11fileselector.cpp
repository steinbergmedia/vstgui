// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../cfileselector.h"
#include <unistd.h>
#include <string>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace X11 {

static constexpr auto kdialogpath = "/usr/bin/kdialog";
static constexpr auto zenitypath = "/usr/bin/zenity";

//------------------------------------------------------------------------
struct FileSelector : CNewFileSelector
{
	FileSelector (CFrame* parent, Style style) : CNewFileSelector (parent), style (style)
	{
		identifiyExDialogType ();
	}

	~FileSelector ()
	{
        closeProcess ();
	}

	bool runInternal (CBaseObject* delegate) override
	{
		this->delegate = delegate;
		switch (exDialogType)
		{
			case ExDialogType::kdialog:
				return runKDialog ();
			case ExDialogType::zenity:
				return runZenity ();
			case ExDialogType::none:
				break;
		}
		return false;
	}

	void cancelInternal () override { closeProcess (); }

	bool runModalInternal () override
	{
		if (runInternal (nullptr))
		{
			while (feof (pipe) == 0)
			{
				char* line = nullptr;
				size_t count = 0;
				if (getline (&line, &count, pipe) < 0)
				{
					break;
				}
				if (line)
				{
					if (line[0] == '/')
					{
						std::string path (line);
						path.erase (path.size () - 1);
						result.emplace_back (path);
					}
					free (line);
				}
			}
		}
		return !result.empty ();
	}

private:
	enum class ExDialogType
	{
		none,
		kdialog,
		zenity
	};

	void identifiyExDialogType ()
	{
		if (auto file = fopen (kdialogpath, "r"))
		{
			fclose (file);
			exDialogType = ExDialogType::kdialog;
		}
		else if (auto file = fopen (zenitypath, "r"))
		{
			fclose (file);
			exDialogType = ExDialogType::zenity;
		}
	}

	bool runKDialog ()
	{
		std::string command = kdialogpath;
		command += " ";
		if (style == Style::kSelectFile)
			command += "--getopenfilename --separate-output";
		else if (style == Style::kSelectSaveFile)
			command += "--getsavefilename";
		else if (style == Style::kSelectDirectory)
			command += "--getexistingdirectory";
        if (allowMultiFileSelection)
            command += " --multiple";
		if (!title.empty ())
			command += " --title '" + title.getString () + "'";
        if (!initialPath.empty ())
            command += " \"" + initialPath.getString () + "\"";
		if (startProcess (command.data ()))
		{
			return true;
		}
		return false;
	}

	bool runZenity ()
	{
		std::string command = zenitypath;
		command += " --file-selection ";
		if (style == Style::kSelectDirectory)
			command += "--directory";
		else if (style == Style::kSelectSaveFile)
			command += "--save --confirm-overwrite";
		if (!title.empty ())
			command += "--title=\"" + title.getString () + "\"";
		if (!initialPath.empty ())
			command += "--filename=\"" + initialPath.getString () + "\"";

		if (startProcess (command.data ()))
		{
			return true;
		}
		return false;
	}

	bool startProcess (const char* command)
	{
		pipe = popen (command, "re");
		return pipe != nullptr;
	}

	void closeProcess ()
	{
		if (pipe)
			pclose (pipe);
		pipe = nullptr;
	}
	Style style;
	SharedPointer<CBaseObject> delegate;
	ExDialogType exDialogType{ExDialogType::none};
	FILE* pipe{nullptr};
};

//------------------------------------------------------------------------
} // X11

//------------------------------------------------------------------------
CNewFileSelector* CNewFileSelector::create (CFrame* parent, Style style)
{
	return new X11::FileSelector (parent, style);
}

//------------------------------------------------------------------------
} // VSTGUI
