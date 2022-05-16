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
TestSuite::TestSuite (std::string&& name, TestSuiteFunction&& testCase)
: name (name)
{
	tcf = std::move (testCase);
	tcf (this);
}

//----------------------------------------------------------------------------------------------------
TestSuite::TestSuite (TestSuite&& tc) noexcept
{
	*this = std::move (tc);
}

//----------------------------------------------------------------------------------------------------
TestSuite& TestSuite::operator=(TestSuite &&tc) noexcept
{
	name = std::move (tc.name);
	tests = std::move (tc.tests);
	tcf = std::move (tc.tcf);
	setupFunction = std::move (tc.setupFunction);
	teardownFunction = std::move (tc.teardownFunction);
	storage = std::move (tc.storage);
	return *this;
}

//----------------------------------------------------------------------------------------------------
void TestSuite::registerTest (std::string&& testName, TestFunction&& testFunction)
{
	tests.emplace_back (std::move (testName), std::move (testFunction));
}

//----------------------------------------------------------------------------------------------------
void TestSuite::setSetupFunction (SetupFunction&& _setupFunction)
{
	setupFunction = std::move (_setupFunction);
}

//----------------------------------------------------------------------------------------------------
void TestSuite::setTeardownFunction (TeardownFunction&& _teardownFunction)
{
	teardownFunction = std::move (_teardownFunction);
}

//----------------------------------------------------------------------------------------------------
void TestSuite::setStorage (std::any&& s)
{
	storage = std::move (s);
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
void UnitTestRegistry::registerTestSuite (TestSuite&& testCase)
{
	testSuites.push_back (std::move (testCase));
}

//------------------------------------------------------------------------
TestSuite* UnitTestRegistry::find (const std::string& name)
{
	auto it = std::find_if (testSuites.begin (), testSuites.end (),
	                        [&name] (auto& tc) { return tc.getName () == name; });
	if (it == testSuites.end ())
		return nullptr;
	return &(*it);
}

//------------------------------------------------------------------------
TestRegistrar::TestRegistrar (std::string&& suite, TestSuiteFunction&& testCase)
{
	UnitTestRegistry::instance ().registerTestSuite (
	    TestSuite (std::move (suite), std::move (testCase)));
}

//------------------------------------------------------------------------
TestRegistrar::TestRegistrar (std::string&& suite, std::string&& testName, TestFunction&& testFunction)
{
	auto& registry = UnitTestRegistry::instance ();
	if (auto tc = registry.find (suite))
	{
		tc->registerTest (std::move (testName), std::move (testFunction));
	}
	else
	{
		TestSuite ts (std::move (suite), [] (auto) {});
		ts.registerTest (std::move (testName), std::move (testFunction));
		registry.registerTestSuite (std::move (ts));
	}
}

//----------------------------------------------------------------------------------------------------
TestRegistrar::TestRegistrar (std::string&& suite, SetupFunction&& sotFunction, bool isSetupFunc)
{
	auto& registry = UnitTestRegistry::instance ();
	if (auto tc = registry.find (suite))
	{
		if (isSetupFunc)
			tc->setSetupFunction (std::move (sotFunction));
		else
			tc->setTeardownFunction (std::move (sotFunction));
	}
	else
	{
		TestSuite ts (std::move (suite), [] (auto) {});
		if (isSetupFunc)
			ts.setSetupFunction (std::move (sotFunction));
		else
			ts.setTeardownFunction (std::move (sotFunction));
		registry.registerTestSuite (std::move (ts));
	}
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

#endif
