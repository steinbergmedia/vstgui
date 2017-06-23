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

	const SharedPointer<IRunLoop>& getRunLoop () const;
	void setRunLoop (const SharedPointer<IRunLoop>& runLoop);
private:
	Platform ();

	std::string path;
	SharedPointer<IRunLoop> runLoop;

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
