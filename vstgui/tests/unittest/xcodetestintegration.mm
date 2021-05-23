// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import <XCTest/XCTest.h>
#import <objc/message.h>
#import <objc/runtime.h>

#import "../../lib/cstring.h"
#import "../../lib/vstguiinit.h"
#import "unittests.h"

#import <functional>
#import <string>
#import <unordered_map>
#import <vector>

using namespace VSTGUI::UnitTest;

//------------------------------------------------------------------------
void setupFunc (id self, SEL _cmd)
{
	auto cls = [self class];
	std::string clsName (class_getName (cls) + 7);
	auto testSuite = UnitTestRegistry::instance ().find (clsName);
	assert (testSuite);
	auto ivar = class_getInstanceVariable ([self class], "_testSuite");
	object_setIvar (self, ivar, reinterpret_cast<id> (testSuite));
}

//------------------------------------------------------------------------
void testFunc (id self, SEL _cmd)
{
	auto ivar = class_getInstanceVariable ([self class], "_testSuite");
	auto testSuite = reinterpret_cast<TestSuite*> (object_getIvar (self, ivar));
	auto name = std::string (sel_getName (_cmd) + 4);
	auto it = std::find_if (testSuite->begin (), testSuite->end (),
	                        [&] (auto& pair) { return pair.first == name; });
	assert (it != testSuite->end ());

	struct TestContext : Context
	{
		TestContext (TestSuite& suite) : suite (suite) {}
		void printRaw (const char* str) override { text += str; }
		std::any& storage () override { return suite.getStorage (); }
		TestSuite& suite;
		std::string text;
	} context (*testSuite);

	size_t assertLineNo = 0;
	std::string assertFilePath;

	bool result = true;

	try
	{
		if (auto setup = testSuite->setup ())
			setup (&context);

		it->second (&context);

		if (auto teardown = testSuite->teardown ())
			teardown (&context);
	}
	catch (const error& exc)
	{
		result = false;
		context.print ("%s", exc.what () ? exc.what () : "unknown");
		assertLineNo = exc.lineNo;
		assertFilePath = exc.filePath;
	}
	catch (const std::logic_error& exc)
	{
		result = false;
		context.print ("%s", exc.what () ? exc.what () : "unknown");
	}
	catch (const std::exception& exc)
	{
		result = false;
		context.print ("Exception: %s", exc.what () ? exc.what () : "unknown");
	}
	if (!result)
	{
#ifdef MAC_OS_X_VERSION_11
		auto sourceCodeLocation = [[[XCTSourceCodeLocation alloc]
		    initWithFilePath:[NSString stringWithUTF8String:assertFilePath.data ()]
		          lineNumber:assertLineNo] autorelease];
		auto sourceCodeContext =
		    [[[XCTSourceCodeContext alloc] initWithLocation:sourceCodeLocation] autorelease];
		auto issue =
		    [[[XCTIssue alloc] initWithType:XCTIssueTypeAssertionFailure
		                 compactDescription:[NSString stringWithUTF8String:context.text.data ()]
		                detailedDescription:nullptr
		                  sourceCodeContext:sourceCodeContext
		                    associatedError:nullptr
		                        attachments:@[]] autorelease];

		[self recordIssue:issue];
#else

		[self recordFailureWithDescription:[NSString stringWithUTF8String:context.text.data ()]
		                            inFile:[NSString stringWithUTF8String:assertFilePath.data ()]
		                            atLine:assertLineNo
		                          expected:NO];
#endif
	}
}

//------------------------------------------------------------------------
@interface VSTGUITestCaseFactory : XCTestCase
@end

@implementation VSTGUITestCaseFactory

//------------------------------------------------------------------------
+ (void)initialize
{
	VSTGUI::init (CFBundleGetMainBundle ());
	VSTGUI::setAssertionHandler ([] (const char* file, const char* line, const char* desc) {
		size_t lineNo = 0;
		if (line)
		{
			if (auto l = VSTGUI::UTF8StringView (line).toNumber<size_t>())
				lineNo = *l;
		}
		throw VSTGUI::UnitTest::error (file, lineNo, desc ? desc : "assert");
	});

	auto baseClass = objc_getClass ("XCTestCase");
	for (auto& testCase : UnitTestRegistry::instance ())
	{
		std::string className = "VSTGUI_" + testCase.getName ();
		auto cls = objc_allocateClassPair (baseClass, className.data (), 0);
		if (!cls)
		{
			NSLog (@"Duplicate TestCase name: %s", testCase.getName ().data ());
			continue;
		}
		class_addIvar (cls, "_testSuite", sizeof (void*), (uint8_t)log2 (sizeof (void*)),
		               @encode (void*));
		class_addMethod (cls, @selector (setUp), reinterpret_cast<IMP> (setupFunc), "v@:");
		for (auto& test : testCase)
		{
			std::string name = "test" + test.first;
			auto sel = sel_registerName (name.data ());
			if (!class_addMethod (cls, sel, reinterpret_cast<IMP> (testFunc), "v@:"))
			{
				NSLog (@"Duplicate Method name: %s [%s]", name.data (), className.data ());
			}
		}
		objc_registerClassPair (cls);
	}
}

@end
