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
#include "../../../../lib/controls/cbuttons.h"
#include "../../../../lib/cstring.h"
#include "helpers.h"

namespace VSTGUI {
using namespace UIViewCreator;

TESTCASE(COnOffButtonCreatorTest,

	TEST(title,
		DummyUIDescription uidesc;
		testAttribute<CCheckBox> (kCCheckBox, kAttrTitle, "title", &uidesc, [] (CCheckBox* b) {
			return b->getTitle () == "title";
		});
	);

	TEST(font,
		DummyUIDescription uidesc;
		testAttribute<CCheckBox> (kCCheckBox, kAttrFont, kFontName, &uidesc, [&] (CCheckBox* b) {
			return uidesc.font == b->getFont ();
		}, true);
	);

	TEST(fontColor,
		DummyUIDescription uidesc;
		testAttribute<CCheckBox> (kCCheckBox, kAttrFontColor, kColorName, &uidesc, [&] (CCheckBox* b) {
			return b->getFontColor () == uidesc.color;
		});
	);

	TEST(boxFrameColor,
		DummyUIDescription uidesc;
		testAttribute<CCheckBox> (kCCheckBox, kAttrBoxframeColor, kColorName, &uidesc, [&] (CCheckBox* b) {
			return b->getBoxFrameColor () == uidesc.color;
		});
	);

	TEST(boxFillColor,
		DummyUIDescription uidesc;
		testAttribute<CCheckBox> (kCCheckBox, kAttrBoxfillColor, kColorName, &uidesc, [&] (CCheckBox* b) {
			return b->getBoxFillColor () == uidesc.color;
		});
	);

	TEST(checkmarkColor,
		DummyUIDescription uidesc;
		testAttribute<CCheckBox> (kCCheckBox, kAttrCheckmarkColor, kColorName, &uidesc, [&] (CCheckBox* b) {
			return b->getCheckMarkColor () == uidesc.color;
		});
	);

	TEST(drawCrossbox,
		DummyUIDescription uidesc;
		testAttribute<CCheckBox> (kCCheckBox, kAttrDrawCrossbox, true, &uidesc, [&] (CCheckBox* b) {
			return b->getStyle () & CCheckBox::kDrawCrossBox;
		});
	);

	TEST(autoSizeToFit,
		DummyUIDescription uidesc;
		testAttribute<CCheckBox> (kCCheckBox, kAttrAutosizeToFit, true, &uidesc, [&] (CCheckBox* b) {
			return b->getStyle () & CCheckBox::kAutoSizeToFit;
		});
	);
);

} // VSTGUI
