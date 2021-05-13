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
	error (const char* file, size_t line, const char* str)
	: std::logic_error (str), filePath (file), lineNo (line)
	{
	}

	std::string filePath;
	size_t lineNo;
};

#define	VSTGUI_UNITTEST_MAKE_STRING_PRIVATE_DONT_USE(x)	# x
#define	VSTGUI_UNITTEST_MAKE_STRING(x) VSTGUI_UNITTEST_MAKE_STRING_PRIVATE_DONT_USE(x)

//----------------------------------------------------------------------------------------------------
#define TEST_CASE(suite, name) \
	static bool test##suite##name (VSTGUI::UnitTest::Context* context); \
	static VSTGUI::UnitTest::TestRegistrar register##suite##name (VSTGUI_UNITTEST_MAKE_STRING(suite), \
		VSTGUI_UNITTEST_MAKE_STRING(name), [](VSTGUI::UnitTest::Context* context) {\
			return test##suite##name (context); \
		});\
	bool test##suite##name (VSTGUI::UnitTest::Context* context)

//----------------------------------------------------------------------------------------------------
#define TESTCASE(name,function) static VSTGUI::UnitTest::TestRegistrar name##TestRegistrar (VSTGUI_UNITTEST_MAKE_STRING(name), [](VSTGUI::UnitTest::TestSuite* testSuite) { function })
#define TEST(name,function) testSuite->registerTest (VSTGUI_UNITTEST_MAKE_STRING(name), [](VSTGUI::UnitTest::Context* context) { { function } });
#define SETUP(function) testSuite->setSetupFunction ([](VSTGUI::UnitTest::Context* context) { function } )
#define TEARDOWN(function) testSuite->setTeardownFunction ([](VSTGUI::UnitTest::Context* context) { function } )

//------------------------------------------------------------------------
#define EXPECT(condition) if (!(condition)) { throw VSTGUI::UnitTest::error (__FILE__, __LINE__, "Expected: " VSTGUI_UNITTEST_MAKE_STRING(condition)); }
#define FAIL(reason) { throw VSTGUI::UnitTest::error (__FILE__, __LINE__, "Failure: " reason); }

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

#define EXPECT_TRUE(condition) EXPECT(condition)
#define EXPECT_FALSE(condition) EXPECT(!condition)
#define EXPECT_EQ(var1, var2) EXPECT (var1 == var2)
#define EXPECT_NE(var1, var2) EXPECT (var1 != var2)

//----------------------------------------------------------------------------------------------------
class Context;
class TestSuite;

//----------------------------------------------------------------------------------------------------
using TestFunction = std::function<void(Context*)>;
using SetupFunction = std::function<void(Context*)>;
using TeardownFunction = std::function<void(Context*)>;
using TestSuiteFunction = std::function<void(TestSuite*)>;

//----------------------------------------------------------------------------------------------------
class UnitTestRegistry
{
	using TestSuites = std::list<TestSuite>;
	using Iterator = TestSuites::const_iterator;
public:
	static UnitTestRegistry& instance ();

	void registerTestSuite (TestSuite&& testSuite);

	Iterator begin () const { return testSuites.begin (); }
	Iterator end () const { return testSuites.end (); }

	TestSuite* find (const std::string& name);
private:
	TestSuites testSuites;
};

//----------------------------------------------------------------------------------------------------
class TestSuite
{
	using TestPair = std::pair<std::string, TestFunction>;
	using Tests = std::list<TestPair>;
	using Iterator = Tests::const_iterator;
public:
	TestSuite (std::string&& name, TestSuiteFunction&& testCase);
	TestSuite (TestSuite&& tc) noexcept;

	void setSetupFunction (SetupFunction&& setupFunction);
	void setTeardownFunction (TeardownFunction&& teardownFunction);
	void registerTest (std::string&& name, TestFunction&& function);

	const std::string& getName () const { return name; }

	Iterator begin () const { return tests.begin (); }
	Iterator end () const { return tests.end (); }

	const SetupFunction& setup () const { return setupFunction; }
	const TeardownFunction& teardown () const { return teardownFunction; }

	TestSuite& operator= (TestSuite&& tc) noexcept;
private:
	Tests tests;
	std::string name;
	TestSuiteFunction tcf;
	SetupFunction setupFunction;
	TeardownFunction teardownFunction;
};

//----------------------------------------------------------------------------------------------------
class TestRegistrar
{
public:
	TestRegistrar (std::string&& suite, TestSuiteFunction&& testSuite);
	TestRegistrar (std::string&& suite, std::string&& testName, TestFunction&& testFunction);
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
