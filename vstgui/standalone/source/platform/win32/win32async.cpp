// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32async.h"
#include <deque>
#include <memory>
#include <mutex>
#include <ppltasks.h>
#include <atomic>

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
			Async::Task* task = reinterpret_cast<Async::Task*> (wParam);
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
void perform (Context context, Task&& task)
{
	switch (context)
	{
		case Context::Main:
		{
			Platform::Win32::postAsyncMainTask (std::move (task));
			break;
		}
		case Context::Background:
		{
			auto p = std::make_shared<Platform::Win32::TaskWrapper> (std::move (task));
			p->run ();
			break;
		}
	}
}

//------------------------------------------------------------------------
} // Async
} // Standalone
} // VSTGUI
