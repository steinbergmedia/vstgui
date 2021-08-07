// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#if ENABLE_UNIT_TESTS

#include "../../lib/vstguifwd.h"
#include <string>
#include <string_view>
#include <list>
#include <functional>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <any>

/** @page How-to write tests

How-to write tests:

1) include this file
2) optional: put the test code into namespaces: namespace VSTGUI { namespace UnitTest {
3) write tests with the function macro TEST_CASE(SuiteName, TestName) { TESTCODE }

Simple Example:

	#include "unittests.h"

	TEST_CASE (Example, OnePlusOneIsTwo)
	{
		auto result = 1+1;
		EXPECT_EQ (result, 2)
	}

Using a setup and teardown function and custom variable storage:

	#include "unittests.h"
	
	TEST_SUITE_SETUP (CViewContainerTest)
	{
		SharedPointer<CViewContainer> container = makeOwned<CViewContainer> (CRect (0, 0, 200, 200));
		TEST_SUITE_SET_STORAGE (SharedPointer<CViewContainer>, container);
	}

	TEST_SUITE_TEARDOWN (CViewContainerTest)
	{
		TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>) = nullptr;
	}

	TEST_CASE (CViewContainerTest, ChangeViewZOrder)
	{
		SharedPointer<CViewContainer>& container = TEST_SUITE_GET_STORAGE (SharedPointer<CViewContainer>);
		...
	}
	
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
	static void test##suite##name (VSTGUI::UnitTest::Context* context); \
	static VSTGUI::UnitTest::TestRegistrar register##suite##name (VSTGUI_UNITTEST_MAKE_STRING(suite), \
		VSTGUI_UNITTEST_MAKE_STRING(name), [](VSTGUI::UnitTest::Context* context) {\
			test##suite##name (context); \
		});\
	void test##suite##name (VSTGUI::UnitTest::Context* context)

#define TEST_SUITE_SETUP(suite) \
	static void setup##suite (VSTGUI::UnitTest::Context* context); \
	static VSTGUI::UnitTest::TestRegistrar registerSetup##suite (VSTGUI_UNITTEST_MAKE_STRING(suite), \
		[](VSTGUI::UnitTest::Context* context) { setup##suite (context); }, true);\
	void setup##suite (VSTGUI::UnitTest::Context* context)

#define TEST_SUITE_TEARDOWN(suite) \
	static void teardown##suite (VSTGUI::UnitTest::Context* context); \
	static VSTGUI::UnitTest::TestRegistrar registerTeardown##suite (VSTGUI_UNITTEST_MAKE_STRING(suite), \
		[](VSTGUI::UnitTest::Context* context) { teardown##suite (context); }, false);\
	void teardown##suite (VSTGUI::UnitTest::Context* context)

#define TEST_SUITE_SET_STORAGE(Type, value) context->storage () = std::make_any<Type> (value)
#define TEST_SUITE_GET_STORAGE(Type) *std::any_cast<Type> (&context->storage ())

//----------------------------------------------------------------------------------------------------
#define EXPECT(condition) if (!(condition)) { throw VSTGUI::UnitTest::error (__FILE__, __LINE__, "Expected: " VSTGUI_UNITTEST_MAKE_STRING(condition)); }
#define FAIL(reason) { throw VSTGUI::UnitTest::error (__FILE__, __LINE__, "Failure: " reason); }

#define EXPECT_EXCEPTION(call, name)                                                               \
	{                                                                                              \
		bool b = false;                                                                            \
		try                                                                                        \
		{                                                                                          \
			call;                                                                                  \
		}                                                                                          \
		catch (const std::exception& error)                                                        \
		{                                                                                          \
			EXPECT (std::string_view (error.what ()) == std::string_view (name));                  \
			b = true;                                                                              \
		}                                                                                          \
		EXPECT (b);                                                                                \
	}

#define EXPECT_TRUE(condition) EXPECT(condition)
#define EXPECT_FALSE(condition) EXPECT(!condition)
#define EXPECT_EQ(var1, var2) EXPECT ((var1) == (var2))
#define EXPECT_NE(var1, var2) EXPECT ((var1) != (var2))

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
	void setStorage (std::any&& s);
	void registerTest (std::string&& name, TestFunction&& function);

	const std::string& getName () const { return name; }
	std::any& getStorage () const { return storage; }

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
	mutable std::any storage;
};

//----------------------------------------------------------------------------------------------------
class TestRegistrar
{
public:
	TestRegistrar (std::string&& suite, TestSuiteFunction&& testSuite);
	TestRegistrar (std::string&& suite, std::string&& testName, TestFunction&& testFunction);
	TestRegistrar (std::string&& suite, SetupFunction&& setupFunction, bool isSetupFunc = true);
};

//----------------------------------------------------------------------------------------------------
class Context
{
public:
	void print (const char* fmt, ...);
	virtual void printRaw (const char* str) = 0;
	virtual std::any& storage () = 0;
};

}} // namespaces

#else

#define TESTCASE(x,y)

#endif
