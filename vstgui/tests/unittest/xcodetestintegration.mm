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

static std::string currentFile;
static std::string currentLineNo;

//------------------------------------------------------------------------
void setupFunc (id self, SEL _cmd)
{
	auto cls = [self class];
	std::string clsName (class_getName (cls) + 7);
	auto it =
	    std::find_if (UnitTestRegistry::instance ().begin (), UnitTestRegistry::instance ().end (),
	                  [&] (auto& testCase) { return testCase.getName () == clsName; });
	assert (it != UnitTestRegistry::instance ().end ());
	auto ivar = class_getInstanceVariable ([self class], "_testCase");
	object_setIvar (self, ivar, reinterpret_cast<id> (&(*it)));
}

//------------------------------------------------------------------------
void testFunc (id self, SEL _cmd)
{
	auto ivar = class_getInstanceVariable ([self class], "_testCase");
	auto testCase = reinterpret_cast<TestCase*> (object_getIvar (self, ivar));
	auto name = std::string (sel_getName (_cmd) + 4);
	auto it = std::find_if (testCase->begin (), testCase->end (),
	                        [&] (auto& pair) { return pair.first == name; });
	if (it == testCase->end ())
	{

		return;
	}
	struct TestContext : Context
	{
		void printRaw (const char* str) override { text += str; }
		std::string text;
	} context;

	if (auto setup = testCase->setup ())
		setup (&context);

	XCTSourceCodeLocation* sourceCodeLocation = nullptr;
	bool result;
	try
	{
		result = it->second (&context);
	}
	catch (const error& exc)
	{
		result = false;
		context.print ("%s", exc.what () ? exc.what () : "unknown");
		sourceCodeLocation = [[[XCTSourceCodeLocation alloc]
		    initWithFilePath:[NSString stringWithUTF8String:exc.filePath.data ()]
		          lineNumber:exc.lineNo] autorelease];
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
		if (!sourceCodeLocation)
		{
			auto lineNo = VSTGUI::UTF8StringView (currentLineNo.data ()).toInteger ();
			sourceCodeLocation = [[[XCTSourceCodeLocation alloc]
			    initWithFilePath:[NSString stringWithUTF8String:currentFile.data ()]
			          lineNumber:lineNo] autorelease];
		}
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
	}

	if (auto teardown = testCase->teardown ())
		teardown (&context);
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
		if (desc)
			throw std::logic_error (desc);
		currentFile = file;
		currentLineNo = line;
		std::string text = "assert: ";
		text += file;
		text += ":";
		text += line;
		throw std::logic_error (text.data ());
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
		class_addIvar (cls, "_testCase", sizeof (void*), (uint8_t)log2 (sizeof (void*)),
		               @encode (void*));
		class_addMethod (cls, @selector (setUp), reinterpret_cast<IMP> (setupFunc), "v@:");
		for (auto& test : testCase)
		{
			std::string name = "test" + test.first;
			name[4] = toupper(name[4]);
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
