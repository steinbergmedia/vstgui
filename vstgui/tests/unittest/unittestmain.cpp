// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "unittests.h"

#if ENABLE_UNIT_TESTS
#include "../../lib/vstguidebug.h"
#include "../../lib/vstguiinit.h"
#include "../../lib/cstring.h"

#include <chrono>
#include <cstdarg>
#include <cstdio>

#if MAC
#include <CoreFoundation/CoreFoundation.h>
#endif

#if WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <objbase.h>
#endif

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UnitTest {

using namespace std::chrono;

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class StdOutContext : public Context
{
private:
	struct Result {
		int succeded;
		int failed;

		Result () : succeded (0), failed (0) {}

		Result& operator +=(const Result& r) { succeded += r.succeded; failed += r.failed; return *this; }
	};
public:
	StdOutContext () : intend (0) {}

	void printRaw (const char* str) override
	{
		testOutput += str;
		testOutput += "\n";
	}

	void printOutput ()
	{
		if (testOutput.empty () == false)
		{
			printf ("%s", testOutput.c_str ());
			testOutput = "";
		}
	}
	void printIntend ()
	{
		for (int i = 0; i < intend; i++) printf ("\t");
	}

	Result runTestSuite (const TestSuite& testSuite)
	{
		currentTestSuite = &testSuite;
		Result result;
		printf ("%s\n", testSuite.getName ().c_str());
		intend++;
		for (auto& it : testSuite)
		{
			try {
				if (testSuite.setup ())
				{
					testSuite.setup () (this);
				}
				if (runTest (it.first, it.second))
				{
					result.succeded++;
				}
				else
				{
					result.failed++;
				}
				if (testSuite.teardown ())
				{
					testSuite.teardown () (this);
				}
			} catch (const std::exception&)
			{
				result.failed++;
			}
		}
		intend--;
		return result;
	}

	bool runTest (const std::string& testName, const TestFunction& f)
	{
		time_point<system_clock> start, end;
		printIntend ();
		printf ("%s", testName.c_str());
		intend++;
		start = system_clock::now ();
		bool result = true;
		try {
			f (this);
		} catch (const error& exc)
		{
			result = false;
			print ("%s", exc.what () ? exc.what () : "unknown");
		} catch (const std::exception& exc)
		{
			result = false;
			printf ("Exception: %s", exc.what () ? exc.what () : "unknown");
		}
		end = system_clock::now ();
		intend--;
		printf (" [%s] -> %lld µs\n", result ? "OK" : "Failed", duration_cast<microseconds> (end-start).count ());
		printOutput ();
		return result;
	}

	int run ()
	{
		Result result;
		time_point<system_clock> start, end;
		start = system_clock::now ();
		for (auto& it : UnitTestRegistry::instance ())
		{
			result += runTestSuite (it);
		}
		end = system_clock::now ();
		print ("\nDone running %d tests in %lldms. [%d Failed]\n", result.succeded+result.failed, duration_cast<milliseconds> (end-start).count (), result.failed);
		printOutput ();
		return result.failed;
	}
	
	std::any& storage () override
	{
		return currentTestSuite->getStorage ();
	}

private:
	int intend;
	std::string testOutput;
	const TestSuite* currentTestSuite {nullptr};
};

static int RunTests ()
{
	StdOutContext context;
	return context.run ();
}

//------------------------------------------------------------------------
} // UnitTest
} // VSTGUI

int main ()
{
	VSTGUI::setAssertionHandler (
		[] (const char* file, const char* line, const char* condition, const char* desc) {
			size_t lineNo = 0;
			if (line)
			{
				if (auto l = VSTGUI::UTF8StringView (line).toNumber<size_t> ())
					lineNo = *l;
			}
			throw VSTGUI::UnitTest::error (file, lineNo, desc ? desc : condition);
		});
#if MAC
	VSTGUI::init (CFBundleGetMainBundle ());
#elif WINDOWS
	CoInitialize (nullptr);
	VSTGUI::init (GetModuleHandle (nullptr));
#elif LINUX
	VSTGUI::init (nullptr);
#endif
	auto result = VSTGUI::UnitTest::RunTests ();
	VSTGUI::exit ();
	return result;
}

#endif

