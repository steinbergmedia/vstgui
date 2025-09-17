// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformtaskexecutor.h"
#include "../../vstguifwd.h"
#include <atomic>
#include <condition_variable>
#include <functional>
#include <queue>
#include <mutex>
#include <thread>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Tasks {
namespace Detail {

//------------------------------------------------------------------------
struct ThreadPool
{
	explicit ThreadPool (size_t numThreads) : workers (numThreads) {}

	~ThreadPool () noexcept
	{
		if (started)
		{
			stopThreads ();
			condition.notify_all ();
			joinAllThreads ();
		}
	}

	void enqueue (Task&& task) noexcept
	{
		std::unique_lock<std::mutex> lock (queueMutex);
		vstgui_assert (!stop, "task is not executed, because the thread pool is already stopped");
		if (stop)
			return;
		++numTasks;
		taskQueue.emplace (std::move (task));
		if (!started)
			startThreads ();
		lock.unlock ();
		condition.notify_one ();
	}

	bool empty () const noexcept { return numTasks == 0u; }

private:
	void startThreads () noexcept
	{
		started = true;
		for (size_t i = 0; i < workers.size (); ++i)
		{
			workers[i] = std::thread ([this] () { workerLoop (); });
		}
	}

	void stopThreads () noexcept
	{
		std::lock_guard<std::mutex> lock (queueMutex);
		stop = true;
		started = false;
	}

	void joinAllThreads () noexcept
	{
		for (auto& worker : workers)
		{
			worker.join ();
		}
	}

	void workerLoop () noexcept
	{
		while (!stop)
		{
			Task task;
			std::unique_lock<std::mutex> lock (queueMutex);
			condition.wait (lock, [this] () { return stop || !taskQueue.empty (); });
			if (!stop && !taskQueue.empty ())
			{
				task = std::move (taskQueue.front ());
				taskQueue.pop ();
			}
			lock.unlock ();
			if (task)
			{
				task ();
				--numTasks;
			}
		}
	}

	std::vector<std::thread> workers;
	std::queue<Task> taskQueue;
	std::atomic<uint64_t> numTasks {0u};
	std::atomic<bool> stop {false};
	std::atomic<bool> started {false};
	std::mutex queueMutex;
	std::condition_variable condition;
};

//------------------------------------------------------------------------
struct SerialQueue
{
	SerialQueue (ThreadPool& pool, uint64_t inIdentifier, const char* inName)
	: threadPool (pool), identifier (inIdentifier), name (inName)
	{
	}

	~SerialQueue () noexcept { vstgui_assert (empty (), "Serial Queue is destroyed non empty"); }

	uint64_t getIdentifier () const noexcept { return identifier; }

	void schedule (Task&& task) noexcept
	{
		std::lock_guard<std::mutex> lock (queueMutex);
		taskQueue.push (std::move (task));
		if (!scheduled)
		{
			scheduled = true;
			threadPool.enqueue ([this] () { runAndScheduleNextTask (); });
		}
	}

	bool empty () const noexcept
	{
		std::lock_guard<std::mutex> lock (queueMutex);
		return taskQueue.empty ();
	}

private:
	void runAndScheduleNextTask () noexcept
	{
		Task task;
		{
			std::lock_guard<std::mutex> lock (queueMutex);
			task = std::move (taskQueue.front ());
			taskQueue.pop ();
		}
		task ();
		std::lock_guard<std::mutex> lock (queueMutex);
		if (taskQueue.empty ())
		{
			scheduled = false;
		}
		else
		{
			threadPool.enqueue ([this] () { runAndScheduleNextTask (); });
		}
	}

	ThreadPool& threadPool;
	uint64_t identifier;
	std::queue<Task> taskQueue;
	std::string name;
	std::atomic_bool scheduled {false};
	mutable std::mutex queueMutex;
};

//------------------------------------------------------------------------
} // Detail

//------------------------------------------------------------------------
struct ThreadPoolTaskExecutor : IPlatformTaskExecutor
{
	using SerialQueueVector = std::vector<std::unique_ptr<Detail::SerialQueue>>;

	ThreadPoolTaskExecutor (PlatformTaskExecutorPtr&& inPlatformTaskExecutor)
	: backgroundQueue ({inPlatformTaskExecutor->getMainQueue ().identifier + 1})
	, platformTaskExecutor (std::move (inPlatformTaskExecutor))
	{
		queueIdentifierCounter = backgroundQueue.identifier;
	}

	~ThreadPoolTaskExecutor () noexcept override
	{
		std::lock_guard<std::mutex> lock (serialQueueMutex);
		serialQueues.clear ();
	}

	const Queue& getMainQueue () const override { return platformTaskExecutor->getMainQueue (); }

	const Queue& getBackgroundQueue () const override { return backgroundQueue; }

	Queue makeSerialQueue (const char* name) const override
	{
		std::lock_guard<std::mutex> lock (serialQueueMutex);
		serialQueues.emplace_back (
			std::make_unique<Detail::SerialQueue> (threadPool, ++queueIdentifierCounter, name));
		return {queueIdentifierCounter};
	}

	void releaseSerialQueue (const Queue& queue) const override
	{
		std::lock_guard<std::mutex> lock (serialQueueMutex);
		auto it = findQueue (queue.identifier);
		if (it != serialQueues.end ())
		{
			waitAllTasksExecuted (it);
			serialQueues.erase (it);
		}
	}

	void schedule (const Queue& queue, Task&& task) const override
	{
		if (queue == getMainQueue ())
		{
			platformTaskExecutor->schedule (queue, std::move (task));
		}
		else if (queue == backgroundQueue)
		{
			threadPool.enqueue (std::move (task));
		}
		else
		{
			std::lock_guard<std::mutex> lock (serialQueueMutex);
			auto it = findQueue (queue.identifier);
			if (it != serialQueues.end ())
				(*it)->schedule (std::move (task));
		}
	}

	void waitAllTasksExecuted (const Queue& queue) const override
	{
		if (queue == getMainQueue ())
		{
			platformTaskExecutor->waitAllTasksExecuted (queue);
		}
		else if (queue == backgroundQueue)
		{
			while (!threadPool.empty ())
				std::this_thread::sleep_for (std::chrono::milliseconds (1));
		}
		else
		{
			std::lock_guard<std::mutex> lock (serialQueueMutex);
			auto it = findQueue (queue.identifier);
			if (it != serialQueues.end ())
				waitAllTasksExecuted (it);
		}
	}

	void waitAllTasksExecuted () const override
	{
		{
			std::lock_guard<std::mutex> lock (serialQueueMutex);
			for (auto it = serialQueues.begin (); it != serialQueues.end (); ++it)
				waitAllTasksExecuted (it);
		}
		waitAllTasksExecuted (backgroundQueue);
		waitAllTasksExecuted (getMainQueue ());
		platformTaskExecutor->waitAllTasksExecuted ();
	}

private:
	void waitAllTasksExecuted (SerialQueueVector::const_iterator it) const
	{
		while ((*it)->empty () == false)
			std::this_thread::sleep_for (std::chrono::milliseconds (1));
	}

	SerialQueueVector::const_iterator findQueue (uint64_t identifier) const
	{
		return std::find_if (serialQueues.begin (), serialQueues.end (),
							 [&] (const auto& el) { return el->getIdentifier (); });
	}

	Queue backgroundQueue;
	PlatformTaskExecutorPtr platformTaskExecutor;
	mutable Detail::ThreadPool threadPool {std::thread::hardware_concurrency ()};
	mutable uint64_t queueIdentifierCounter {};
	mutable SerialQueueVector serialQueues;
	mutable std::mutex serialQueueMutex;
};

//------------------------------------------------------------------------
} // Tasks
} // VSTGUI
