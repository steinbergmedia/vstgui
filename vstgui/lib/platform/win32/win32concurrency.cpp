// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32concurrency.h"
#include "../../vstguidebug.h"
#include <atomic>
#include <ppltasks.h>
#include <string>
#include <thread>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Concurrency {
namespace Detail {

//------------------------------------------------------------------------
struct TaskWrapper final : std::enable_shared_from_this<TaskWrapper>
{
	TaskWrapper (Task&& t) : task (std::move (t)) {}

	void schedule ()
	{
		f = std::make_shared<concurrency::task<void>> ([This = shared_from_this ()] () {
			This->task ();
			This->f = nullptr;
		});
	}

	Task task;
	std::shared_ptr<concurrency::task<void>> f;
};

} // Detail

//------------------------------------------------------------------------
struct Queue
{
	virtual ~Queue () noexcept
	{
		vstgui_assert (empty (), "Concurrency Queues must be empty on exit");
	}

	virtual void schedule (Task&& task) const = 0;
	uint32_t empty () const { return numTasks == 0u; }

	mutable std::atomic<uint32_t> numTasks;
};

//------------------------------------------------------------------------
struct MainQueue final : Queue
{
	static constexpr auto WM_USER_ASYNC = WM_USER;

	MainQueue (HINSTANCE instance) : instance (instance)
	{
		std::wstring windowClassName;
		windowClassName = TEXT ("VSTGUI MessageWindow ");
		windowClassName += std::to_wstring (reinterpret_cast<uint64_t> (this));

		WNDCLASSEX wcex {};
		wcex.cbSize = sizeof (WNDCLASSEX);
		wcex.lpfnWndProc = WndMessageProc;
		wcex.hInstance = instance;
		wcex.lpszClassName = windowClassName.data ();

		messageWindowAtom = RegisterClassEx (&wcex);
		messageWindow = CreateWindowEx (0, wcex.lpszClassName, nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE,
										nullptr, instance, nullptr);
		SetWindowLongPtr (messageWindow, GWLP_USERDATA, (__int3264)(LONG_PTR)this);
	}

	~MainQueue () noexcept override
	{
		if (messageWindow)
		{
			while (numTasks > 0u && handleNextTask ())
			{
			}
			DestroyWindow (messageWindow);
			UnregisterClass (MAKEINTATOM (messageWindowAtom), instance);
		}
	}

	static LRESULT CALLBACK WndMessageProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto instance = reinterpret_cast<MainQueue*> (GetWindowLongPtr (hwnd, GWLP_USERDATA));
		if (instance && hwnd == instance->messageWindow)
		{
			if (message == WM_USER_ASYNC)
			{
				auto* task = reinterpret_cast<Concurrency::Task*> (wParam);
				(*task) ();
				delete task;
				return 0;
			}
		}
		return DefWindowProc (hwnd, message, wParam, lParam);
	}

	void schedule (Task&& t) const override
	{
		auto task = new Concurrency::Task (std::move (t));
		PostMessage (messageWindow, WM_USER_ASYNC, reinterpret_cast<WPARAM> (task), 0);
	}

	bool handleNextTask ()
	{
		MSG msg {};
		if (PeekMessage (&msg, messageWindow, 0, 0, PM_REMOVE))
		{
			TranslateMessage (&msg);
			DispatchMessage (&msg);
			return true;
		}
		return false;
	}

	ATOM messageWindowAtom {};
	HINSTANCE instance {nullptr};
	HWND messageWindow {nullptr};
};

//------------------------------------------------------------------------
struct BackgroundQueue final : Queue
{
	void schedule (Task&& task) const override
	{
		++numTasks;
		auto tw = std::make_shared<Detail::TaskWrapper> ([this, task = std::move (task)] () {
			task ();
			--numTasks;
		});
		tw->schedule ();
	}
};

//------------------------------------------------------------------------
struct SerialQueue final : Queue,
						   std::enable_shared_from_this<SerialQueue>
{
	SerialQueue (const char* n)
	{
		if (n)
			name = n;
	}

	void schedule (Task&& task) const override
	{
		auto t = [f = std::move (task), queue = shared_from_this ()] () {
			f ();
			queue->numTasks--;
		};
		numTasks++;

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
	mutable bool hasTask {false};
	std::string name;
	mutable concurrency::task<void> ctask;
	mutable std::mutex mutex;
};

static std::atomic<uint32_t> numUserQueues {0u};

//------------------------------------------------------------------------
} // Concurrency

//------------------------------------------------------------------------
struct Win32Concurrency::Impl
{
	std::unique_ptr<Concurrency::MainQueue> mainQueue;
	Concurrency::BackgroundQueue backgroundQueue;
	std::atomic<uint32_t> numSerialQueues {0u};
	std::thread::id mainThreadId {std::this_thread::get_id ()};
};

//------------------------------------------------------------------------
Win32Concurrency::Win32Concurrency () { impl = std::make_unique<Impl> (); }

//------------------------------------------------------------------------
Win32Concurrency::~Win32Concurrency () noexcept
{
	while (!impl->backgroundQueue.empty ())
		impl->mainQueue->handleNextTask ();

	vstgui_assert (Concurrency::numUserQueues == 0u,
				   "Concurrency Queues must all be destroyed at this point");
}

//------------------------------------------------------------------------
void Win32Concurrency::init (HINSTANCE instance)
{
	impl->mainQueue = std::make_unique<Concurrency::MainQueue> (instance);
}

//------------------------------------------------------------------------
const Concurrency::Queue& Win32Concurrency::getMainQueue () const
{
	vstgui_assert (impl->mainQueue, "Not initialized!");
	return *impl->mainQueue.get ();
}

//------------------------------------------------------------------------
const Concurrency::Queue& Win32Concurrency::getBackgroundQueue () const
{
	return impl->backgroundQueue;
}

//------------------------------------------------------------------------
Concurrency::QueuePtr Win32Concurrency::makeSerialQueue (const char* name) const
{
	++Concurrency::numUserQueues;
	return std::shared_ptr<Concurrency::SerialQueue> (new Concurrency::SerialQueue (name),
													  [] (auto* queue) {
														  delete queue;
														  Concurrency::numUserQueues--;
													  });
}

//------------------------------------------------------------------------
void Win32Concurrency::schedule (const Concurrency::Queue& queue, Concurrency::Task&& task) const
{
	queue.schedule (std::move (task));
}

//------------------------------------------------------------------------
void Win32Concurrency::waitAllTasksExecuted (const Concurrency::Queue& queue) const
{
	vstgui_assert (std::this_thread::get_id () == impl->mainThreadId,
				   "Call this only on the main thread");

	while (queue.numTasks > 0u)
	{
		if (!impl->mainQueue->handleNextTask ())
			std::this_thread::sleep_for (std::chrono::milliseconds (1));
	}
}

//------------------------------------------------------------------------
void Win32Concurrency::waitAllTasksExecuted () const
{
	waitAllTasksExecuted (getBackgroundQueue ());
	waitAllTasksExecuted (getMainQueue ());
}

//------------------------------------------------------------------------
} // VSTGUI
