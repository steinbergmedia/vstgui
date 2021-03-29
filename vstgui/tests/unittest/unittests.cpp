// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "unittests.h"

#if ENABLE_UNIT_TESTS
#include "../../lib/vstguidebug.h"
#include "../../lib/vstguiinit.h"

#if MAC
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <chrono>
#include <cstdarg>
#include <cstdio>

#if WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <objbase.h>
#endif

namespace VSTGUI {
namespace UnitTest {

#if WINDOWS
static void printf (const char* fmt, ...)
{
	va_list args;
	va_start (args, fmt);
	auto numBytes = vsnprintf (nullptr, 0, fmt, args);
	if (numBytes <= 0)
		return;
	numBytes++;
	auto buffer = new char[numBytes];
	if (vsnprintf (buffer, numBytes, fmt, args) > 0)
	{
		if (IsDebuggerPresent ())
			OutputDebugStringA (buffer);
		else
			::printf_s ("%s", buffer);
	}

	delete[] buffer;
}
#endif

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
TestCase::TestCase (std::string&& name, TestCaseFunction&& testCase)
: name (name)
{
	tcf = std::move (testCase);
	tcf (this);
}

//----------------------------------------------------------------------------------------------------
TestCase::TestCase (TestCase&& tc) noexcept
{
	*this = std::move (tc);
}

//----------------------------------------------------------------------------------------------------
TestCase& TestCase::operator=(TestCase &&tc) noexcept
{
	name = std::move (tc.name);
	tests = std::move (tc.tests);
	tcf = std::move (tc.tcf);
	setupFunction = std::move (tc.setupFunction);
	teardownFunction = std::move (tc.teardownFunction);
	return *this;
}

//----------------------------------------------------------------------------------------------------
void TestCase::registerTest (std::string&& testName, TestFunction&& testFunction)
{
	tests.emplace_back (std::move (testName), std::move (testFunction));
}

//----------------------------------------------------------------------------------------------------
void TestCase::setSetupFunction (SetupFunction&& _setupFunction)
{
	setupFunction = std::move (_setupFunction);
}

//----------------------------------------------------------------------------------------------------
void TestCase::setTeardownFunction (TeardownFunction&& _teardownFunction)
{
	teardownFunction = std::move (_teardownFunction);
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UnitTestRegistry& UnitTestRegistry::instance ()
{
	static UnitTestRegistry gUnitTestRegistry;
	return gUnitTestRegistry;
}

//----------------------------------------------------------------------------------------------------
void UnitTestRegistry::registerTestCase (TestCase&& testCase)
{
	testCases.push_back (std::move (testCase));
}

//----------------------------------------------------------------------------------------------------
void Context::print (const char* fmt, ...)
{
	va_list args;
	va_start (args, fmt);
#if WINDOWS
	auto numBytes = vsnprintf(nullptr, 0, fmt, args);
	if (numBytes <= 0)
		return;
	auto buffer = new char[numBytes];
	if (vsnprintf(buffer, numBytes, fmt, args) > 0)
		printRaw (buffer);

	delete[] buffer;
#else
	char* str = nullptr;
	if (vasprintf (&str, fmt, args) >= 0 && str != nullptr)
	{
		printRaw (str);
		std::free (str);
	}
#endif
}

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

	Result runTestCase (const TestCase& testCase)
	{
		Result result;
		printf ("%s\n", testCase.getName ().c_str());
		intend++;
		for (auto& it : testCase)
		{
			try {
				if (testCase.setup ())
				{
					testCase.setup () (this);
				}
				if (runTest (it.first, it.second))
				{
					result.succeded++;
				}
				else
				{
					result.failed++;
				}
				if (testCase.teardown ())
				{
					testCase.teardown () (this);
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
		bool result;
		try {
			result = f (this);
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
			result += runTestCase (std::move (it));
		}
		end = system_clock::now ();
		print ("\nDone running %d tests in %lldms. [%d Failed]\n", result.succeded+result.failed, duration_cast<milliseconds> (end-start).count (), result.failed);
		printOutput ();
		return result.failed;
	}
private:
	int intend;
	std::string testOutput;
};

static int RunTests ()
{
	StdOutContext context;
	return context.run ();
}

}} // namespaces

int main ()
{
	VSTGUI::setAssertionHandler ([] (const char* file, const char* line, const char* desc) {
		throw std::logic_error (desc ? desc : "unknown");
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

TESTCASE(Example,
		 
	static int result;

	SETUP(
		result = 0;
	);
	 
	TEST(OnePlusOneIsTwo,
		result = 1+1;
		EXPECT (result == 2)
	);
	 
	TEST(ThreeMinusOneIsTwo,
		result = 3-1;
		if (result != 2)
		{
			FAIL ("result is not two")
		}
	);

);

#endif
