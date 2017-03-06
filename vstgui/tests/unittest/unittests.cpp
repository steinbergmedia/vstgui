//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "unittests.h"

#if ENABLE_UNIT_TESTS

#include <chrono>
#include <cstdarg>
#include <cstdio>

#if WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
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
void TestCase::registerTest (std::string&& name, TestFunction&& function)
{
	tests.push_back (TestPair (std::move (name), std::move (function)));
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
	char* str = 0;
	if (vasprintf (&str, fmt, args) >= 0 && str != 0)
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

	virtual void printRaw (const char* str)
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
			} catch (const std::exception& exc)
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
		printf (" [%s] -> %ld µs\n", result ? "OK" : "Failed", duration_cast<microseconds> (end-start).count ());
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

#if __APPLE_CC__
#include <CoreFoundation/CoreFoundation.h>
namespace VSTGUI { void* gBundleRef = CFBundleGetMainBundle (); }
#elif __linux__
namespace VSTGUI { void* soHandle = nullptr; }
#endif

#if WINDOWS
void* hInstance = nullptr;
#endif

int main ()
{
#if WINDOWS
	CoInitialize (nullptr);
	hInstance = GetModuleHandle (nullptr);
#endif
	return VSTGUI::UnitTest::RunTests ();
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
