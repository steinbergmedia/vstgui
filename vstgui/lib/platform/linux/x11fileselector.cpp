// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../cfileselector.h"
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <vector>
#include <memory>
#include <cerrno>
#include <cassert>
extern "C" { extern char **environ; }

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
			std::string path;
			path.reserve (1024);

			ssize_t count;
			char buffer[1024];
			while ((count = read (readerFd, buffer, sizeof (buffer))) > 0 ||
				   (count == -1 && errno == EINTR))
			{
				if (count > 0)
					path.append (buffer, count);
			}

			if (count != -1)
			{
				if (! path.empty () && path[0] == '/')
				{
					if (path.back () == '\n')
						path.pop_back ();
					result.emplace_back (path);
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
		if (access (zenitypath, X_OK) != -1)
			exDialogType = ExDialogType::zenity;
		if (access (kdialogpath, X_OK) != -1)
			exDialogType = ExDialogType::kdialog;
	}

	bool runKDialog ()
	{
		std::vector<std::string> args;
		args.reserve (16);
		args.push_back (kdialogpath);
		if (style == Style::kSelectFile) {
			args.push_back ("--getopenfilename");
			args.push_back ("--separate-output");
		}
		else if (style == Style::kSelectSaveFile)
			args.push_back ("--getsavefilename");
		else if (style == Style::kSelectDirectory)
			args.push_back ("--getexistingdirectory");
		if (allowMultiFileSelection)
			args.push_back ("--multiple");
		if (!title.empty ()) {
			args.push_back ("--title");
			args.push_back (title.getString ());
		}
		args.push_back (initialPath.getString ());
		if (style == Style::kSelectFile || style == Style::kSelectSaveFile)
		{
			std::string filterString;
			filterString.reserve (256);
			for (auto const &ext : extensions)
			{
				filterString += "*.";
				filterString += ext.getExtension ().getString ();
				filterString += ' ';
			}
			args.push_back (filterString);
		}
		if (startProcess (convertToArgv (args).data ()))
		{
			return true;
		}
		return false;
	}

	bool runZenity ()
	{
		std::vector<std::string> args;
		args.reserve (16);
		args.push_back (zenitypath);
		args.push_back ("--file-selection");
		if (style == Style::kSelectDirectory)
			args.push_back ("--directory");
		else if (style == Style::kSelectSaveFile)
		{
			args.push_back ("--save");
			args.push_back ("--confirm-overwrite");
		}
		if (!title.empty ())
			args.push_back ("--title=" + title.getString ());
		if (!initialPath.empty ())
			args.push_back ("--filename=" + initialPath.getString ());
		if (!extensions.empty ())
		{
			std::string filterString;
			filterString.reserve (256);
			for (auto const &ext : extensions)
			{
				filterString = "--file-filter=";
				filterString += ext.getDescription ().getString ();
				filterString += " (";
				filterString += ext.getExtension ().getString ();
				filterString += ") | *.";
				filterString += ext.getExtension ().getString ();
				args.push_back (filterString);
			}
		}
		if (startProcess (convertToArgv (args).data ()))
		{
			return true;
		}
		return false;
	}

	static std::vector<char*> convertToArgv (const std::vector<std::string>& args)
	{
		std::vector<char*> argv (args.size () + 1);
		for (size_t i = 0, n = args.size (); i < n; ++i)
			argv[i] = const_cast<char*>(args[i].c_str ());
		return argv;
	}

	bool startProcess (char* argv[])
	{
		closeProcess ();

		struct PipePair
		{
			int fd[2] = { -1, -1 };
			~PipePair ()
			{
				if (fd[0] != -1) close (fd[0]);
				if (fd[1] != -1) close (fd[1]);
			}
		};

		PipePair rw;
		if (pipe (rw.fd) != 0)
			return false;

#if 0
		char** envp = environ;
#else
		std::vector<char*> cleanEnviron;
		cleanEnviron.reserve (256);
		for (char** envp = environ; *envp; ++envp)
		{
			// ensure the process will link with system libraries,
			// and not these from the Ardour bundle.
			if (strncmp (*envp, "LD_LIBRARY_PATH=", 16) == 0)
				continue;
			cleanEnviron.push_back (*envp);
		}
		cleanEnviron.push_back (nullptr);
		char** envp = cleanEnviron.data ();
#endif

		pid_t forkPid = vfork ();
		if (forkPid == -1)
			return false;

		if (forkPid == 0) {
			execute (argv, envp, rw.fd);
			assert (false);
		}

		spawnPid = forkPid;

		close (rw.fd[1]);
		rw.fd[1] = -1;
		readerFd = rw.fd[0];
		rw.fd[0] = -1;

		return true;
	}

	[[noreturn]]
	static void execute (char* argv[], char* envp[], const int pipeFd[2])
	{
		close (pipeFd[0]);
		if (dup2 (pipeFd[1], STDOUT_FILENO) == -1)
			_exit (1);
		close (pipeFd[1]);
		execve (argv[0], argv, envp);
		_exit (1);
	}

	void closeProcess ()
	{
		if (spawnPid != -1)
		{
			if (waitpid (spawnPid, nullptr, WNOHANG) == 0)
			{
				kill (spawnPid, SIGTERM);
				waitpid (spawnPid, nullptr, 0);
			}
			spawnPid = -1;
		}

		if (readerFd != -1)
		{
			close (readerFd);
			readerFd = -1;
		}
	}

	Style style;
	SharedPointer<CBaseObject> delegate;
	ExDialogType exDialogType{ExDialogType::none};
	pid_t spawnPid = -1;
	int readerFd = -1;
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
