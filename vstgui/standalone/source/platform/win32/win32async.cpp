// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32async.h"
#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <ppltasks.h>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Win32 {

static std::atomic<uint32_t> gBackgroundTaskCount {};
static HWND asyncMessageWindow {nullptr};
static LRESULT CALLBACK AsyncWndMessageProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static constexpr auto WM_USER_ASYNC = WM_USER;
static const WCHAR* gAsyncMessageWindowClassName = L"VSTGUI Standalone Async Message Window";

//------------------------------------------------------------------------
void initAsyncHandling (HINSTANCE instance)
{
	WNDCLASSEX wcex {};

	wcex.cbSize = sizeof (WNDCLASSEX);

	wcex.lpfnWndProc = AsyncWndMessageProc;
	wcex.hInstance = instance;
	wcex.lpszClassName = gAsyncMessageWindowClassName;

	RegisterClassEx (&wcex);
	asyncMessageWindow = CreateWindowEx (0, wcex.lpszClassName, nullptr, 0, 0, 0, 0, 0,
	                                     HWND_MESSAGE, nullptr, instance, nullptr);
}

//------------------------------------------------------------------------
void terminateAsyncHandling ()
{
	MSG msg;
	while (gBackgroundTaskCount != 0)
	{
		if (PeekMessage (&msg, asyncMessageWindow, 0, 0, PM_REMOVE))
		{
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
	}
	while (PeekMessage (&msg, asyncMessageWindow, 0, 0, PM_REMOVE))
	{
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}
	DestroyWindow (asyncMessageWindow);
	asyncMessageWindow = nullptr;
}

//------------------------------------------------------------------------
LRESULT CALLBACK AsyncWndMessageProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (hwnd == asyncMessageWindow)
	{
		if (message == WM_USER_ASYNC)
		{
			auto* task = reinterpret_cast<Async::Task*> (wParam);
			(*task) ();
			delete task;
			return 0;
		}
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}

//------------------------------------------------------------------------
void postAsyncMainTask (Async::Task&& t)
{
	auto task = new Async::Task (std::move (t));
	PostMessage (asyncMessageWindow, WM_USER_ASYNC, reinterpret_cast<WPARAM> (task), 0);
}

//------------------------------------------------------------------------
struct TaskWrapper : std::enable_shared_from_this<TaskWrapper>
{
	TaskWrapper (Async::Task&& t) : task (std::move (t)) {}

	void run ()
	{
		++gBackgroundTaskCount;
		f = std::make_shared<concurrency::task<void>> ([This = shared_from_this ()] () {
			This->task ();
			This->f = nullptr;
			--gBackgroundTaskCount;
		});
	}
	Async::Task task;
	std::shared_ptr<concurrency::task<void>> f;
};

//------------------------------------------------------------------------
} // Win32
} // Platform

//------------------------------------------------------------------------
namespace Async {

//------------------------------------------------------------------------
struct Queue
{
	virtual void schedule (Task&& task) = 0;
};

//------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------
struct MainQueue final : Queue
{
	void schedule (Task&& task) override { Platform::Win32::postAsyncMainTask (std::move (task)); }
};

//------------------------------------------------------------------------
struct BackgroundQueue final : Queue
{
	void schedule (Task&& task) override
	{
		auto p = std::make_shared<Platform::Win32::TaskWrapper> (std::move (task));
		p->run ();
	}
};

//------------------------------------------------------------------------
struct SerialQueue final : Queue, std::enable_shared_from_this<SerialQueue>
{
	SerialQueue (const char* n)
	{
		if (n)
			name = n;
	}

	void schedule (Task&& task) override
	{
		++Platform::Win32::gBackgroundTaskCount;
		auto t = [f = std::move (task), queue = shared_from_this ()] ()
		{
			f ();
			--Platform::Win32::gBackgroundTaskCount;
		};

		std::lock_guard<std::mutex> guard (mutex);
		if (hasTask)
		{
			ctask = ctask.then (std::move (t));
		}
		else
		{
			hasTask = true;
			ctask = concurrency::task<void> (std::move (t));
		}
	}

private:
	bool hasTask {false};
	std::string name;
	concurrency::task<void> ctask;
	std::mutex mutex;
};

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
const QueuePtr& mainQueue ()
{
	static QueuePtr q = std::make_shared<MainQueue> ();
	return q;
}

//------------------------------------------------------------------------
const QueuePtr& backgroundQueue ()
{
	static QueuePtr q = std::make_shared<BackgroundQueue> ();
	return q;
}

//------------------------------------------------------------------------
QueuePtr makeSerialQueue (const char* name)
{
	return std::make_shared<SerialQueue> (name);
}

//------------------------------------------------------------------------
void schedule (QueuePtr queue, Task&& task)
{
	queue->schedule (std::move (task));
}

//------------------------------------------------------------------------
} // Async
} // Standalone
} // VSTGUI
