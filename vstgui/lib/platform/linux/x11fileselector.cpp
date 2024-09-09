// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "x11fileselector.h"
#include "../../vstguibase.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <string>
#include <vector>
#include <memory>
#include <cerrno>
#include <cassert>
extern "C" { extern char **environ; }

//------------------------------------------------------------------------
namespace VSTGUI {
namespace X11 {

#if LINUX
// Use execvpe on Linux to support also NixOS based distributions
static constexpr auto kdialogpath = "kdialog";
static constexpr auto zenitypath = "zenity";
#else
static constexpr auto kdialogpath = "/usr/bin/kdialog";
static constexpr auto zenitypath = "/usr/bin/zenity";
#endif

//------------------------------------------------------------------------
struct FileSelector : IPlatformFileSelector
{
	FileSelector (PlatformFileSelectorStyle style) : style (style) {}

	~FileSelector () noexcept { closeProcess (); }

	bool cancel () override
	{
		closeProcess ();
		return false;
	}

	bool runDialog (const PlatformFileSelectorConfig& config)
	{
		if (runZenity (config))
			return true;

		if (runKDialog (config))
			return true;

		return false;
	}

	bool run (const PlatformFileSelectorConfig& config) override
	{
		if (runDialog (config))
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

			std::vector<UTF8String> result;
			if (count != -1)
			{
				if (! path.empty () && path[0] == '/')
				{
					if (path.back () == '\n')
						path.pop_back ();
					result.emplace_back (path);
				}
			}
			if (config.doneCallback)
				config.doneCallback (std::move (result));
			return true;
		}
		return false;
	}

private:
	enum class ExDialogType
	{
		none,
		kdialog,
		zenity
	};

	bool runKDialog (const PlatformFileSelectorConfig& config)
	{
		std::vector<std::string> args;
		args.reserve (16);
		args.push_back (kdialogpath);
		if (style == PlatformFileSelectorStyle::SelectFile)
		{
			args.push_back ("--getopenfilename");
			args.push_back ("--separate-output");
		}
		else if (style == PlatformFileSelectorStyle::SelectSaveFile)
			args.push_back ("--getsavefilename");
		else if (style == PlatformFileSelectorStyle::SelectDirectory)
			args.push_back ("--getexistingdirectory");
		if (hasBit (config.flags, PlatformFileSelectorFlags::MultiFileSelection))
			args.push_back ("--multiple");
		if (!config.title.empty ())
		{
			args.push_back ("--title");
			args.push_back (config.title.getString ());
		}
		if (!config.initialPath.empty ())
			args.push_back (config.initialPath.getString ());
		if (startProcess (convertToArgv (args).data ()))
		{
			return true;
		}
		return false;
	}

	bool runZenity (const PlatformFileSelectorConfig& config)
	{
		std::vector<std::string> args;
		args.reserve (16);
		args.push_back (zenitypath);
		args.push_back ("--file-selection");
		if (style == PlatformFileSelectorStyle::SelectDirectory)
			args.push_back ("--directory");
		else if (style == PlatformFileSelectorStyle::SelectSaveFile)
		{
			args.push_back ("--save");
			args.push_back ("--confirm-overwrite");
		}
		if (!config.title.empty ())
			args.push_back ("--title=" + config.title.getString ());
		if (!config.initialPath.empty ())
			args.push_back ("--filename=" + config.initialPath.getString ());
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
			if (!execute (argv, envp, rw.fd))
				return false;

			assert (false);
		}

		spawnPid = forkPid;

		close (rw.fd[1]);
		rw.fd[1] = -1;
		readerFd = rw.fd[0];
		rw.fd[0] = -1;

		return true;
	}

	static bool execute (char* argv[], char* envp[], const int pipeFd[2])
	{
		close (pipeFd[0]);
		if (dup2 (pipeFd[1], STDOUT_FILENO) == -1)
			_exit (1);
		close (pipeFd[1]);
#if LINUX
		if (execvpe (argv[0], argv, envp) == -1)
#else
		if (execve (argv[0], argv, envp) == -1)
#endif
			return false;

		_exit (1);
		return true; // not reachable
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

	PlatformFileSelectorStyle style;
	ExDialogType exDialogType{ExDialogType::none};
	pid_t spawnPid = -1;
	int readerFd = -1;
};

//------------------------------------------------------------------------
PlatformFileSelectorPtr createFileSelector (PlatformFileSelectorStyle style, Frame* frame)
{
	return std::make_shared<FileSelector> (style);
}

//------------------------------------------------------------------------
} // X11
} // VSTGUI
