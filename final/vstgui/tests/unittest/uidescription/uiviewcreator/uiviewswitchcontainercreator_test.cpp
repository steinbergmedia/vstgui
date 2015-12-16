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

#include "../../unittests.h"
#include "../../../../uidescription/uiviewfactory.h"
#include "../../../../uidescription/uiattributes.h"
#include "../../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../../uidescription/uiviewswitchcontainer.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(UIViewSwitchContainerCreatorTest,

	TEST(templateNames,
		DummyUIDescription uidesc;
		testAttribute<UIViewSwitchContainer>(kUIViewSwitchContainer, kAttrTemplateNames, "temp1,temp2", &uidesc, [] (UIViewSwitchContainer* v) {
			auto controller = dynamic_cast<UIDescriptionViewSwitchController*> (v->getController ());
			EXPECT(controller);
			std::string str;
			controller->getTemplateNames(str);
			return str == "temp1,temp2";
		});
	);

	TEST(templateSwitchControl,
		DummyUIDescription uidesc;
		uidesc.tag = 12345;
		testAttribute<UIViewSwitchContainer>(kUIViewSwitchContainer, kAttrTemplateSwitchControl, kTagName, &uidesc, [&] (UIViewSwitchContainer* v) {
			auto controller = dynamic_cast<UIDescriptionViewSwitchController*> (v->getController ());
			EXPECT(controller);
			return controller->getSwitchControlTag() == uidesc.tag;
		}, true);
	);

	TEST(animationStyle,
		DummyUIDescription uidesc;
		testAttribute<UIViewSwitchContainer>(kUIViewSwitchContainer, kAttrAnimationStyle, "fade", &uidesc, [] (UIViewSwitchContainer* v) {
			return v->getAnimationStyle() == UIViewSwitchContainer::kFadeInOut;
		});
		testAttribute<UIViewSwitchContainer>(kUIViewSwitchContainer, kAttrAnimationStyle, "move", &uidesc, [] (UIViewSwitchContainer* v) {
			return v->getAnimationStyle() == UIViewSwitchContainer::kMoveInOut;
		});
		testAttribute<UIViewSwitchContainer>(kUIViewSwitchContainer, kAttrAnimationStyle, "push", &uidesc, [] (UIViewSwitchContainer* v) {
			return v->getAnimationStyle() == UIViewSwitchContainer::kPushInOut;
		});
	);

	TEST(animationTime,
		DummyUIDescription uidesc;
		testAttribute<UIViewSwitchContainer>(kUIViewSwitchContainer, kAttrAnimationTime, 1234, &uidesc, [] (UIViewSwitchContainer* v) {
			return v->getAnimationTime() == 1234;
		});
	);
	
	TEST(animationStyleValues,
		DummyUIDescription uidesc;
		testPossibleValues (kUIViewSwitchContainer, kAttrAnimationStyle, &uidesc, {"fade", "move", "push"});
	);
	
);

} // VSTGUI
