// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../vstguifwd.h"
#include "x11frame.h"
#include <atomic>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <poll.h>
#include <unistd.h>
#include <unordered_map>

//------------------------------------------------------------------------
namespace VSTGUI {
extern void* soHandle; // shared library handle

//------------------------------------------------------------------------
namespace X11 {

class Frame;
class Timer;

//------------------------------------------------------------------------
class Platform
{
public:
	~Platform ();

	static Platform& getInstance ();
	static uint64_t getCurrentTimeMs ();

	std::string getPath ();

private:
	Platform ();

	std::string path;
};

//------------------------------------------------------------------------
struct RunLoop
{
	static void init (const SharedPointer<IRunLoop>& runLoop)
	{
		if (++instance ().useCount == 1)
		{
			instance ().runLoop = runLoop;
		}
	}

	static void exit ()
	{
		if (--instance ().useCount == 0)
		{
			instance ().runLoop = nullptr;
		}
	}

	static const SharedPointer<IRunLoop> get ()
	{
		return instance ().runLoop;
	}

private:
	static RunLoop& instance ()
	{
		static RunLoop gInstance;
		return gInstance;
	}
	SharedPointer<IRunLoop> runLoop;
	std::atomic<uint32_t> useCount {0};
};

//------------------------------------------------------------------------
struct LocalEventLoop
{
	void run ();
	void stop ();

	bool isRunning () const { return running; }
private:
	bool running {false};
};

//------------------------------------------------------------------------
} // X11
} // VSTGUI
