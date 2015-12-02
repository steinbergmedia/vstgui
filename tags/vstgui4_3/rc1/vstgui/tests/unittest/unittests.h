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

#ifndef vstgui_unittests_h
#define vstgui_unittests_h

#if ENABLE_UNIT_TESTS

#include <string>
#include <list>
#include <functional>
#include <cstdio>
#include <cstdlib>

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
typedef std::function<bool(Context*)> TestFunction;
typedef std::function<void(Context*)> SetupFunction;
typedef std::function<void(Context*)> TeardownFunction;
typedef std::function<void(TestCase*)> TestCaseFunction;

//----------------------------------------------------------------------------------------------------
class UnitTestRegistry
{
	typedef std::list<TestCase> TestCases;
	typedef TestCases::const_iterator Iterator;
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
	typedef std::pair<std::string, TestFunction> TestPair;
	typedef std::list<TestPair> Tests;
	typedef Tests::const_iterator Iterator;
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

#endif
