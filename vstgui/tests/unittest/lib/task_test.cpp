// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/tasks.h"
#include "../unittests.h"
#include <atomic>
#include <thread>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
TEST_SUITE_SETUP (SerialQueueTest)
{
	TEST_SUITE_SET_STORAGE (Tasks::Queue, Tasks::makeSerialQueue ("Test Serial Queue"));
}

//------------------------------------------------------------------------
TEST_SUITE_TEARDOWN (SerialQueueTest)
{
	Tasks::releaseSerialQueue (TEST_SUITE_GET_STORAGE (Tasks::Queue));
}

//------------------------------------------------------------------------
TEST_CASE (SerialQueueTest, Validation)
{
	const auto& serialQueue = TEST_SUITE_GET_STORAGE (Tasks::Queue);
	EXPECT_NE (serialQueue, Tasks::InvalidQueue);
}

//------------------------------------------------------------------------
TEST_CASE (SerialQueueTest, SimpleTasks)
{
	uint32_t numIterations = 1000u;
	uint32_t counter {0u};
	const auto& serialQueue = TEST_SUITE_GET_STORAGE (Tasks::Queue);
	auto increaseCounterFunc = [&] () {
		++counter;
		std::this_thread::yield ();
	};
	for (auto i = 0u; i < numIterations; ++i)
		Tasks::schedule (serialQueue, increaseCounterFunc);
	Tasks::waitAllTasksExecuted (serialQueue);
	EXPECT_EQ (counter, numIterations)
}

//------------------------------------------------------------------------
TEST_CASE (SerialQueueTest, scheduleSerialTasksOnBackgroundQueue)
{
	uint32_t numIterations = 1000u;
	uint32_t counter {0u};
	const auto& serialQueue = TEST_SUITE_GET_STORAGE (Tasks::Queue);
	auto increaseCounterFunc = [&] () {
		++counter;
	};
	for (auto i = 0u; i < numIterations; ++i)
	{
		Tasks::schedule (Tasks::backgroundQueue (),
						 [&] () { Tasks::schedule (serialQueue, increaseCounterFunc); });
	}
	Tasks::waitAllTasksExecuted (Tasks::backgroundQueue ());
	Tasks::waitAllTasksExecuted (serialQueue);
	EXPECT_EQ (counter, numIterations)
}

//------------------------------------------------------------------------
} // VSTGUI
