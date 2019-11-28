// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#if ENABLE_UNIT_TESTS

#include "../../lib/vstguifwd.h"
#include <string>
#include <list>
#include <functional>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

/* 
	How-to write tests:
	
	1) include this file
	2) optional: put the test code into namespaces: namespace VSTGUI { namespace UnitTest {
	3) open testcase : TESTCASE (MySuite,
	4) optional : declare SETUP and TEARDOWN functions
	5) declare tests : TEST (MyTest, testing code);
	6) inside of "testing code" use EXPECT or FAIL macros
	7) close testcase: );
	8) optional: close namespaces: }}

	Complete Example:
	
		#include "unittests.h"
		
		TESTCASE(Example,

			int result;

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

*/

namespace VSTGUI {
namespace UnitTest {

//----------------------------------------------------------------------------------------------------
class error : public std::logic_error
{
public:
	error (const char* str) : std::logic_error (str) {}
};

#define	VSTGUI_UNITTEST_MAKE_STRING_PRIVATE_DONT_USE(x)	# x
#define	VSTGUI_UNITTEST_MAKE_STRING(x) VSTGUI_UNITTEST_MAKE_STRING_PRIVATE_DONT_USE(x)

//----------------------------------------------------------------------------------------------------
#define TESTCASE(name,function) static VSTGUI::UnitTest::TestCaseRegistrar name##TestCaseRegistrar (VSTGUI_UNITTEST_MAKE_STRING(name), [](VSTGUI::UnitTest::TestCase* testCase) { function })
#define TEST(name,function) testCase->registerTest (VSTGUI_UNITTEST_MAKE_STRING(name), [](VSTGUI::UnitTest::Context* context) { { function } return true; });
#define EXPECT(condition) if (!(condition)) { throw VSTGUI::UnitTest::error (__FILE__ ":" VSTGUI_UNITTEST_MAKE_STRING(__LINE__) ": Expected: " VSTGUI_UNITTEST_MAKE_STRING(condition)); }
#define FAIL(reason) { context->print (__FILE__ ":" VSTGUI_UNITTEST_MAKE_STRING(__LINE__) ": Failure: " reason); return false; }

#define EXPECT_EXCEPTION(call, name) \
{ \
	bool b = false; \
	try { \
		call; \
	} catch (const std::exception& error) {\
		EXPECT(error.what() == std::string(name));\
		b = true;\
	} \
	EXPECT(b);\
}

#define SETUP(function) testCase->setSetupFunction ([](VSTGUI::UnitTest::Context* context) { function } )
#define TEARDOWN(function) testCase->setTeardownFunction ([](VSTGUI::UnitTest::Context* context) { function } )
//----------------------------------------------------------------------------------------------------
class Context;
class TestCase;

//----------------------------------------------------------------------------------------------------
using TestFunction = std::function<bool(Context*)>;
using SetupFunction = std::function<void(Context*)>;
using TeardownFunction = std::function<void(Context*)>;
using TestCaseFunction = std::function<void(TestCase*)>;

//----------------------------------------------------------------------------------------------------
class UnitTestRegistry
{
	using TestCases = std::list<TestCase>;
	using Iterator = TestCases::const_iterator;
public:
	static UnitTestRegistry& instance ();

	void registerTestCase (TestCase&& testCase);

	Iterator begin () const { return testCases.begin (); }
	Iterator end () const { return testCases.end (); }
private:
	TestCases testCases;
};

//----------------------------------------------------------------------------------------------------
class TestCase
{
	using TestPair = std::pair<std::string, TestFunction>;
	using Tests = std::list<TestPair>;
	using Iterator = Tests::const_iterator;
public:
	TestCase (std::string&& name, TestCaseFunction&& testCase);
	TestCase (TestCase&& tc) noexcept;

	void setSetupFunction (SetupFunction&& setupFunction);
	void setTeardownFunction (TeardownFunction&& teardownFunction);
	void registerTest (std::string&& name, TestFunction&& function);

	const std::string& getName () const { return name; }

	Iterator begin () const { return tests.begin (); }
	Iterator end () const { return tests.end (); }
	
	const SetupFunction& setup () const { return setupFunction; }
	const TeardownFunction& teardown () const { return teardownFunction; }
	
	TestCase& operator= (TestCase&& tc) noexcept;
private:
	Tests tests;
	std::string name;
	TestCaseFunction tcf;
	SetupFunction setupFunction;
	TeardownFunction teardownFunction;
};

//----------------------------------------------------------------------------------------------------
class TestCaseRegistrar
{
public:
	TestCaseRegistrar (std::string&& name, TestCaseFunction&& testCase)
	{
		UnitTestRegistry::instance().registerTestCase (TestCase (std::move (name), std::move (testCase)));
	}
};

//----------------------------------------------------------------------------------------------------
class Context
{
public:
	void print (const char* fmt, ...);
	virtual void printRaw (const char* str) = 0;
};

}} // namespaces

#else

#define TESTCASE(x,y)

#endif
