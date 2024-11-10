// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32taskexecutor.h"
#include "../../vstguidebug.h"
#include <atomic>
#include <ppltasks.h>
#include <string>
#include <thread>
#include <mutex>
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Detail {

//------------------------------------------------------------------------
struct TaskWrapper final : std::enable_shared_from_this<TaskWrapper>
{
	TaskWrapper (Tasks::Task&& t) : task (std::move (t)) {}

	void schedule ()
	{
		f = std::make_shared<concurrency::task<void>> ([This = shared_from_this ()] () {
			This->task ();
			This->f = nullptr;
		});
	}

	Tasks::Task task;
	std::shared_ptr<concurrency::task<void>> f;
};

//------------------------------------------------------------------------
struct Queue
{
	Queue (uint64_t queueID) : queueID ({queueID}) {}
	virtual ~Queue () noexcept { vstgui_assert (empty (), "Tasks queues must be empty on exit"); }

	virtual void schedule (Tasks::Task&& task) const = 0;
	uint32_t empty () const { return numTasks == 0u; }

	mutable std::atomic<uint32_t> numTasks {0u};
	mutable Tasks::Queue queueID;
};

//------------------------------------------------------------------------
struct MainQueue final : Queue
{
	static constexpr auto WM_USER_ASYNC = WM_USER;

	MainQueue (HINSTANCE instance) : Queue (0u), instance (instance)
	{
		std::wstring windowClassName;
		windowClassName = TEXT ("VSTGUI Tasks MessageWindow ");
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
				auto* task = reinterpret_cast<Tasks::Task*> (wParam);
				(*task) ();
				delete task;
				return 0;
			}
		}
		return DefWindowProc (hwnd, message, wParam, lParam);
	}

	void schedule (Tasks::Task&& t) const override
	{
		auto task = new Tasks::Task (std::move (t));
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
	BackgroundQueue () : Queue (1u) {}
	void schedule (Tasks::Task&& task) const override
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
	SerialQueue (const char* n, uint64_t queueId) : Queue (queueId)
	{
		if (n)
			name = n;
	}

	void schedule (Tasks::Task&& task) const override
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

//------------------------------------------------------------------------
} // Detail

//------------------------------------------------------------------------
struct Win32TaskExecutor::Impl
{
	std::unique_ptr<Detail::MainQueue> mainQueue;
	Detail::BackgroundQueue backgroundQueue;
	std::vector<std::shared_ptr<Detail::SerialQueue>> serialQueues;
	std::mutex serialQueueLock;
	std::thread::id mainThreadId {std::this_thread::get_id ()};
	uint64_t nextSerialQueueID {2u};

	const Detail::Queue* queueFromQueueID (const Tasks::Queue& queue)
	{
		if (queue.identifier == mainQueue->queueID.identifier)
			return mainQueue.get ();
		if (queue.identifier == backgroundQueue.queueID.identifier)
			return &backgroundQueue;

		std::lock_guard<std::mutex> guard (serialQueueLock);
		auto it = std::find_if (serialQueues.begin (), serialQueues.end (), [&] (const auto& el) {
			return el->queueID.identifier == queue.identifier;
		});
		if (it != serialQueues.end ())
			return it->get ();
		return nullptr;
	}
};

//------------------------------------------------------------------------
Win32TaskExecutor::Win32TaskExecutor (HINSTANCE instance)
{
	impl = std::make_unique<Impl> ();
	init (instance);
}

//------------------------------------------------------------------------
Win32TaskExecutor::~Win32TaskExecutor () noexcept
{
	while (!impl->backgroundQueue.empty ())
		impl->mainQueue->handleNextTask ();

	vstgui_assert (impl->serialQueues.empty (),
				   "Serial queues must all be destroyed at this point");
}

//------------------------------------------------------------------------
void Win32TaskExecutor::init (HINSTANCE instance)
{
	impl->mainQueue = std::make_unique<Detail::MainQueue> (instance);
}

//------------------------------------------------------------------------
const Tasks::Queue& Win32TaskExecutor::getMainQueue () const
{
	vstgui_assert (impl->mainQueue, "Not initialized!");
	return impl->mainQueue->queueID;
}

//------------------------------------------------------------------------
const Tasks::Queue& Win32TaskExecutor::getBackgroundQueue () const
{
	return impl->backgroundQueue.queueID;
}

//------------------------------------------------------------------------
Tasks::Queue Win32TaskExecutor::makeSerialQueue (const char* name) const
{
	impl->serialQueueLock.lock ();
	auto serialQueue = std::make_shared<Detail::SerialQueue> (name, impl->nextSerialQueueID++);
	impl->serialQueues.emplace_back (serialQueue);
	impl->serialQueueLock.unlock ();
	return serialQueue->queueID;
}

//------------------------------------------------------------------------
void Win32TaskExecutor::releaseSerialQueue (const Tasks::Queue& queue) const
{
	impl->serialQueueLock.lock ();
	auto it =
		std::find_if (impl->serialQueues.begin (), impl->serialQueues.end (),
					  [&] (const auto& el) { return el->queueID.identifier == queue.identifier; });
	if (it != impl->serialQueues.end ())
	{
		auto serialQueue = std::move (*it);
		impl->serialQueues.erase (it);
		impl->serialQueueLock.unlock ();
		while (serialQueue->numTasks > 0u)
		{
			std::this_thread::sleep_for (std::chrono::milliseconds (1));
		}
	}
	else
	{
		impl->serialQueueLock.unlock ();
	}
}

//------------------------------------------------------------------------
void Win32TaskExecutor::schedule (const Tasks::Queue& queue, Tasks::Task&& task) const
{
	if (auto q = impl->queueFromQueueID (queue))
		q->schedule (std::move (task));
}

//------------------------------------------------------------------------
void Win32TaskExecutor::waitAllTasksExecuted (const Tasks::Queue& queue) const
{
	vstgui_assert (std::this_thread::get_id () == impl->mainThreadId,
				   "Call this only on the main thread");

	if (auto q = impl->queueFromQueueID (queue))
	{
		while (q->numTasks > 0u)
		{
			if (!impl->mainQueue->handleNextTask ())
				std::this_thread::sleep_for (std::chrono::milliseconds (1));
		}
	}
}

//------------------------------------------------------------------------
void Win32TaskExecutor::waitAllTasksExecuted () const
{
	waitAllTasksExecuted (getBackgroundQueue ());
	waitAllTasksExecuted (getMainQueue ());
}

//------------------------------------------------------------------------
} // VSTGUI
