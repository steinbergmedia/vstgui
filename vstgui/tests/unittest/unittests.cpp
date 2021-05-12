// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "unittests.h"

#if ENABLE_UNIT_TESTS
#include "../../lib/vstguidebug.h"
#include "../../lib/vstguiinit.h"

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

}} // namespaces

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
