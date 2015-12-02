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

#include "../unittests.h"
#include "../../../uidescription/uidescription.h"
#include "../../../uidescription/uiattributes.h"
#include "../../../uidescription/xmlparser.h"
#include "../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../lib/ccolor.h"
#include "../../../lib/cgradient.h"
#include "../../../lib/cviewcontainer.h"

namespace VSTGUI {

namespace {

constexpr auto emptyUIDesc = R"(
<vstgui-ui-description version="1">
</vstgui-ui-description>
)";

constexpr auto colorNodesUIDesc = R"(
<vstgui-ui-description version="1">
	<colors>
		<color name="c1" rgba="#000000ff"/>
		<color name="c2" rgb="#ffffff"/>
		<color name="c3" red="255" green="0" blue="0" alpha="100"/>
		<color name="c4" red="0" green="255" blue="0" alpha="150"/>
		<color name="c5" red="255" green="0" blue="255" alpha="100"/>
	</colors>
</vstgui-ui-description>
)";

constexpr auto fontNodesUIDesc = R"(
<vstgui-ui-description version="1">
	<fonts>
		<font font-name="Arial" name="f1" size="8"/>
		<font font-name="Arial" name="f2" size="8" bold="true"/>
		<font font-name="Arial" name="f3" size="8" italic="true"/>
		<font font-name="Arial" name="f4" size="8" underline="true"/>
		<font font-name="Arial" name="f5" size="8" strike-through="true"/>
		<font font-name="bla" name="f6" size="8" alternative-font-names="Arial, Courier"/>
	</fonts>
</vstgui-ui-description>
)";

constexpr auto bitmapNodesUIDesc = R"(
<vstgui-ui-description version="1">
	<bitmaps>
		<bitmap name="b1" path="b1.png"/>
		<bitmap name="b1#2.0x" path="b1#2.0x.png" scale-factor="2"/>
	</bitmaps>
</vstgui-ui-description>
)";

constexpr auto tagNodesUIDesc = R"(
<vstgui-ui-description version="1">
	<control-tags>
		<control-tag name="t1" tag="1234"/>
		<control-tag name="t2" tag="4321"/>
		<control-tag name="t3" tag="'mytg'"/>
	</control-tags>
</vstgui-ui-description>
)";

constexpr auto calculateTagNodesUIDesc = R"(
<vstgui-ui-description version="1">
	<control-tags>
		<control-tag name="t1" tag="1+2"/>
	</control-tags>
</vstgui-ui-description>
)";

constexpr auto gradientNodesUIDesc = R"(
<vstgui-ui-description version="1">
	<gradients>
		<gradient name="g1">
			<color-stop rgba="#000000ff" start="0"/>
			<color-stop rgba="#ff0000ff" start="0.5"/>
			<color-stop rgba="#ffffffff" start="1"/>
		</gradient>
	</gradients>
</vstgui-ui-description>
)";

constexpr auto variableNodesUIDesc = R"(
<vstgui-ui-description version="1">
	<variables>
		<var name="v1" type="number" value="10"/>
		<var name="v2" type="string" value="string"/>
		<var name="v3" value="string"/>
		<var name="v4" value="20.5"/>
		<var name="v5" type="string" value="2*var.v1"/>
		<var name="v6"/>
	</variables>
</vstgui-ui-description>
)";

constexpr auto withAllNodesUIDesc = R"(<?xml version="1.0" encoding="UTF-8"?>
<vstgui-ui-description version="1">
	<colors>
		<color name="c1" rgba="#000000ff"/>
		<color name="c2" rgb="#ffffff"/>
		<color alpha="100" blue="0" green="0" name="c3" red="255"/>
	</colors>
	<fonts>
		<font font-name="Arial" name="f1" size="8"/>
		<font bold="true" font-name="Arial" name="f2" size="8"/>
	</fonts>
	<!-- a comment -->
	<bitmaps>
		<bitmap name="b1" path="b1.png"/>
		<bitmap name="b1#2.0x" path="b1#2.0x.png" scale-factor="2"/>
		<bitmap name="dataBitmap" path="dataBitmap.png">
			<data encoding="base64">
				iVBORw0KGgoAAAANSUhEUgAAAA4AAAAHCAYAAAA4R3wZAAAAAXNSR0IArs4c6QAAACxJREFUGBlj/A8EDG
				QAFqAeRjL0MYA0IgOibUfXSLTtII1E24LsNKr4kSSbASPLBxbAIufgAAAAAElFTkSuQmCC
			</data>
		</bitmap>
	</bitmaps>
	<control-tags>
		<control-tag name="t1" tag="1234"/>
		<control-tag name="t2" tag="4321"/>
	</control-tags>
	<gradients>
		<gradient name="g1">
			<color-stop rgba="#000000ff" start="0"/>
			<color-stop rgba="#ff0000ff" start="0.5"/>
			<color-stop rgba="#ffffffff" start="1"/>
		</gradient>
	</gradients>
	<variables>
		<var name="test" type="number" value="10"/>
		<var name="test" type="string" value="this is a string"/>
	</variables>
</vstgui-ui-description>
)";

struct SaveUIDescription : public UIDescription
{
	SaveUIDescription (Xml::IContentProvider* xmlContentProvider)
	: UIDescription (xmlContentProvider) {}

	using UIDescription::saveToStream;
};

struct Controller : public IController
{
	void valueChanged (CControl* pControl) override {};
	int32_t getTagForName (UTF8StringPtr name, int32_t registeredTag) const override
	{
		return registeredTag;
	}
	IControlListener* getControlListener (UTF8StringPtr controlTagName) override
	{
		return this;
	}
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override
	{
		return nullptr;
	}
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override
	{
		return view;
	}
	IController* createSubController (UTF8StringPtr name, const IUIDescription* description) override
	{
		return nullptr;
	}

};

constexpr auto createViewUIDesc = R"(
<vstgui-ui-description version="1">
	<template background-color="~ TransparentCColor" background-color-draw-style="filled and stroked" class="CViewContainer" mouse-enabled="true" name="view" opacity="1" origin="0, 0" size="400, 235" transparent="false">
		<view class="CView" mouse-enabled="true" opacity="1" origin="4, 10" size="392, 40" transparent="false"/>
	</template>
</vstgui-ui-description>
)";

constexpr auto restoreViewUIDesc = R"(
<vstgui-ui-description version="1">
	<template background-color="~ TransparentCColor" background-color-draw-style="filled and stroked" class="CViewContainer" mouse-enabled="true" name="view" opacity="1" origin="0, 0" size="400, 235" transparent="false">
		<view class="CViewContainer" mouse-enabled="true" opacity="1" origin="4, 10" size="392, 40" transparent="false"/>
		<view class="CViewContainer" mouse-enabled="true" opacity="1" origin="4, 10" size="392, 40" transparent="false">
			<view class="CView" mouse-enabled="true" opacity="1" origin="4, 10" size="392, 40" transparent="false"/>
		</view>
	</template>
</vstgui-ui-description>
)";

constexpr auto completeExample = R"(
<vstgui-ui-description version="1">
	<colors>
	</colors>
	<custom>
		<attributes name="UIViewInspector" windowSize="71, 194, 471, 709"/>
		<attributes name="UIViewHierarchyBrowser" windowSize="513, 194, 813, 694"/>
		<attributes name="FocusDrawing"/>
		<attributes name="UIGridController"/>
		<attributes SelectedTemplate="tab1" name="UITemplateController"/>
		<attributes EditViewScale="1" EditorSize="0, 0, 1244, 755" SplitViewSize_0_0="0.8228882833787466433150825650955084711313" SplitViewSize_0_1="0.1498637602179836436633308949240017682314" SplitViewSize_1_0="0.480926430517711167578198683258960954845" SplitViewSize_1_1="0.5122615803814714041664046817459166049957" SplitViewSize_2_0="0.7033762057877813722583937305898871272802" SplitViewSize_2_1="0.2926045016077170601853651987767079845071" Version="1" name="UIEditController"/>
		<attributes name="UIAttributesController"/>
		<attributes SelectedRow="20" name="UIViewCreatorDataSource"/>
	</custom>
	<bitmaps>
		<bitmap name="animation_knob" path="animation_knob.png"/>
		<bitmap name="FrameBackground" nineparttiled-offsets="10, 20, 10, 10" path="FrameBackground.png"/>
		<bitmap name="TabController" path="TabController.png"/>
		<bitmap name="horizontal_slider_back" path="horizontal_slider_back.bmp"/>
		<bitmap name="onoff_button" path="onoff_button.bmp"/>
		<bitmap name="rocker_switch" path="rocker_switch.bmp"/>
		<bitmap name="slider_handle" path="slider_handle.bmp"/>
		<bitmap name="switch_horizontal" path="switch_horizontal.bmp"/>
		<bitmap name="switch_vertical" path="switch_vertical.bmp"/>
		<bitmap name="vertical_slider_back" path="vertical_slider_back.bmp"/>
		<bitmap name="vumeter_back" path="vumeter_back.bmp"/>
		<bitmap name="vumeter_front" path="vumeter_front.bmp"/>
	</bitmaps>
	<fonts>
	</fonts>
	<control-tags>
		<control-tag name="Switch" tag="20000"/>
	</control-tags>
	<variables/>
	<template autosize="left right top bottom " background-color="~ WhiteCColor" background-color-draw-style="filled and stroked" class="CViewContainer" maxSize="300, 500" minSize="150, 300" mouse-enabled="true" name="view" opacity="1" origin="0, 0" size="300, 500" transparent="false">
		<view animation-style="fade" animation-time="120" background-color="~ BlackCColor" background-color-draw-style="filled and stroked" class="UIViewSwitchContainer" mouse-enabled="true" opacity="1" origin="10, 10" size="280, 480" transparent="false"/>
	</template>
	<template autosize="left right top bottom " background-color="~ BlackCColor" background-color-draw-style="filled and stroked" class="CViewContainer" mouse-enabled="true" name="tab1" opacity="1" origin="0, 0" size="280, 480" transparent="true">
		<view background-color="~ BlackCColor" background-color-draw-style="filled and stroked" class="CLayeredViewContainer" mouse-enabled="true" opacity="1" origin="10, 20" size="100, 100" transparent="false" z-index="0"/>
		<view background-color="~ BlackCColor" background-color-draw-style="filled and stroked" bitmap="0" class="CShadowViewContainer" mouse-enabled="true" opacity="1" origin="130, 20" shadow-blur-size="4" shadow-intensity="0.3" shadow-offset="0, 0" size="200, 200" transparent="false"/>
		<view auto-drag-scrolling="false" auto-hide-scrollbars="false" background-color="~ BlackCColor" background-color-draw-style="filled and stroked" bordered="true" class="CScrollView" container-size="200, 200" follow-focus-view="false" horizontal-scrollbar="true" mouse-enabled="true" opacity="1" origin="40, 240" overlay-scrollbars="false" scrollbar-background-color="#ffffffc8" scrollbar-frame-color="~ BlackCColor" scrollbar-scroller-color="~ BlueCColor" scrollbar-width="16" size="100, 100" transparent="false" vertical-scrollbar="true"/>
		<view animate-view-resizing="false" background-color="~ BlackCColor" background-color-draw-style="filled and stroked" class="CRowColumnView" equal-size-layout="left-top" margin="0,0,0,0" mouse-enabled="true" opacity="1" origin="60, 360" row-style="true" size="100, 100" spacing="0" transparent="false" view-resize-animation-time="200"/>
		<view background-color="~ BlackCColor" background-color-draw-style="filled and stroked" class="CSplitView" mouse-enabled="true" opacity="1" orientation="horizontal" origin="160, 280" resize-method="last" separator-width="10" size="100, 100" transparent="false"/>
	</template>
	<template autosize="left right top bottom row " background-color="~ BlackCColor" background-color-draw-style="filled and stroked" class="CViewContainer" mouse-enabled="true" name="tab2" opacity="1" origin="0, 0" size="280, 480" transparent="true">
		<view angle-range="270" angle-start="135" background-offset="0, 0" circle-drawing="false" class="CAnimKnob" corona-color="~ WhiteCColor" corona-dash-dot="false" corona-drawing="false" corona-from-center="false" corona-inset="0" corona-inverted="false" corona-outline="false" default-value="0.5" handle-color="~ WhiteCColor" handle-line-width="1" handle-shadow-color="~ GreyCColor" height-of-one-image="0" max-value="1" min-value="0" mouse-enabled="true" opacity="1" origin="10, 10" size="20, 20" sub-pixmaps="0" transparent="false" value-inset="0" wheel-inc-value="0.1" zoom-factor="1.5"/>
		<view animation-index="0" animation-time="500" background-offset="0, 0" class="CAnimationSplashScreen" default-value="0.5" max-value="1" min-value="0" mouse-enabled="true" opacity="1" origin="50, 10" size="20, 20" splash-origin="0, 0" splash-size="0, 0" transparent="false" wheel-inc-value="0.1"/>
		<view autosize-to-fit="false" background-offset="0, 0" boxfill-color="~ WhiteCColor" boxframe-color="~ BlackCColor" checkmark-color="~ RedCColor" class="CCheckBox" default-value="0.5" draw-crossbox="false" font="~ SystemFont" font-color="~ WhiteCColor" max-value="1" min-value="0" mouse-enabled="true" opacity="1" origin="110, 10" size="100, 20" title="Title" transparent="false" wheel-inc-value="0.1"/>
		<view background-offset="0, 0" class="CControl" default-value="0.5" max-value="1" min-value="0" mouse-enabled="true" opacity="1" origin="20, 50" size="20, 20" transparent="false" wheel-inc-value="0.1"/>
		<view class="CGradientView" draw-antialiased="true" frame-color="~ BlackCColor" frame-width="1" gradient-angle="0" gradient-style="linear" mouse-enabled="true" opacity="1" origin="70, 50" radial-center="0.5, 0.5" radial-radius="1" round-rect-radius="5" size="100, 100" transparent="false"/>
		<view background-offset="0, 0" class="CHorizontalSwitch" default-value="0" height-of-one-image="0" max-value="1" min-value="0" mouse-enabled="true" opacity="1" origin="20, 160" size="20, 20" sub-pixmaps="0" transparent="false" wheel-inc-value="0.1"/>
		<view background-offset="0, 0" class="CKickButton" default-value="0.5" height-of-one-image="0" max-value="1" min-value="0" mouse-enabled="true" opacity="1" origin="70, 150" size="20, 20" sub-pixmaps="0" transparent="false" wheel-inc-value="0.1"/>
		<view angle-range="270" angle-start="135" background-offset="0, 0" circle-drawing="false" class="CKnob" corona-color="~ WhiteCColor" corona-dash-dot="false" corona-drawing="false" corona-from-center="false" corona-inset="0" corona-inverted="false" corona-outline="false" default-value="0.5" handle-color="~ WhiteCColor" handle-line-width="1" handle-shadow-color="~ GreyCColor" max-value="1" min-value="0" mouse-enabled="true" opacity="1" origin="120, 150" size="20, 20" transparent="false" value-inset="3" wheel-inc-value="0.1" zoom-factor="1.5"/>
		<view background-offset="0, 0" class="CMovieBitmap" default-value="0.5" height-of-one-image="0" max-value="1" min-value="0" mouse-enabled="true" opacity="1" origin="170, 160" size="20, 20" sub-pixmaps="0" transparent="false" wheel-inc-value="0.1"/>
		<view background-offset="0, 0" class="CMovieButton" default-value="0.5" height-of-one-image="0" max-value="1" min-value="0" mouse-enabled="true" opacity="1" origin="30, 200" size="20, 20" sub-pixmaps="0" transparent="false" wheel-inc-value="0.1"/>
		<view background-offset="0, 0" class="COnOffButton" default-value="0.5" max-value="1" min-value="0" mouse-enabled="true" opacity="1" origin="80, 190" size="20, 20" transparent="false" wheel-inc-value="0.1"/>
		<view back-color="~ BlackCColor" background-offset="0, 0" class="COptionMenu" default-value="0.5" font="~ NormalFont" font-antialias="true" font-color="~ WhiteCColor" frame-color="~ BlackCColor" frame-width="1" max-value="1.84467e+19" menu-check-style="false" menu-popup-style="false" min-value="0" mouse-enabled="true" opacity="1" origin="140, 190" round-rect-radius="6" shadow-color="~ RedCColor" size="20, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-round-rect="false" style-shadow-text="false" text-alignment="center" text-inset="0, 0" text-rotation="0" transparent="false" value-precision="2" wheel-inc-value="0.1"/>
		<view back-color="~ BlackCColor" background-offset="0, 0" class="CParamDisplay" default-value="0.5" font="~ NormalFont" font-antialias="true" font-color="~ WhiteCColor" frame-color="~ BlackCColor" frame-width="1" max-value="1" min-value="0" mouse-enabled="true" opacity="1" origin="30, 230" round-rect-radius="6" shadow-color="~ RedCColor" size="20, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-round-rect="false" style-shadow-text="false" text-alignment="center" text-inset="0, 0" text-rotation="0" transparent="false" value-precision="2" wheel-inc-value="0.1"/>
		<view background-offset="0, 0" class="CRockerSwitch" default-value="0.5" height-of-one-image="0" max-value="1" min-value="-1" mouse-enabled="true" opacity="1" origin="70, 230" size="20, 20" sub-pixmaps="3" transparent="false" wheel-inc-value="0.1"/>
		<view background-offset="0, 0" class="CSegmentButton" default-value="0.5" font="~ NormalFont" frame-color="~ BlackCColor" frame-width="1" icon-text-margin="0" max-value="1" min-value="0" mouse-enabled="true" opacity="1" origin="120, 230" round-radius="5" segment-names="Segment 1,Segment 2,Segment 3,Segment 4" size="200, 20" style="horizontal" text-alignment="center" text-color="~ BlackCColor" text-color-highlighted="~ WhiteCColor" transparent="false" wheel-inc-value="0.1"/>
		<view background-offset="0, 0" bitmap-offset="0, 0" class="CSlider" default-value="0.5" draw-back="false" draw-back-color="~ WhiteCColor" draw-frame="false" draw-frame-color="~ WhiteCColor" draw-value="false" draw-value-color="~ WhiteCColor" draw-value-from-center="false" draw-value-inverted="false" handle-offset="0, 0" max-value="1" min-value="0" mode="free click" mouse-enabled="true" opacity="1" orientation="horizontal" origin="20, 270" reverse-orientation="false" size="20, 20" transparent="false" transparent-handle="true" wheel-inc-value="0.1" zoom-factor="10"/>
		<view background-offset="0, 0" class="CTextButton" default-value="0.5" font="~ SystemFont" frame-color="~ BlackCColor" frame-color-highlighted="~ BlackCColor" frame-width="1" gradient="Default TextButton Gradient" gradient-highlighted="Default TextButton Gradient Highlighted" icon-position="left" icon-text-margin="0" kick-style="true" max-value="1" min-value="0" mouse-enabled="true" opacity="1" origin="60, 270" round-radius="6" size="100, 20" text-alignment="center" text-color="~ BlackCColor" text-color-highlighted="~ WhiteCColor" transparent="false" wheel-inc-value="0.1"/>
		<view back-color="~ BlackCColor" background-offset="0, 0" class="CTextEdit" default-value="0.5" font="~ NormalFont" font-antialias="true" font-color="~ WhiteCColor" frame-color="~ BlackCColor" frame-width="1" immediate-text-change="false" max-value="1" min-value="0" mouse-enabled="true" opacity="1" origin="180, 270" round-rect-radius="6" shadow-color="~ RedCColor" size="100, 20" style-3D-in="false" style-3D-out="false" style-doubleclick="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-round-rect="false" style-shadow-text="false" text-alignment="center" text-inset="0, 0" text-rotation="0" transparent="false" value-precision="2" wheel-inc-value="0.1"/>
		<view back-color="~ BlackCColor" background-offset="0, 0" class="CTextLabel" default-value="0.5" font="~ NormalFont" font-antialias="true" font-color="~ WhiteCColor" frame-color="~ BlackCColor" frame-width="1" max-value="1" min-value="0" mouse-enabled="true" opacity="1" origin="20, 300" round-rect-radius="6" shadow-color="~ RedCColor" size="100, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-round-rect="false" style-shadow-text="false" text-alignment="center" text-inset="0, 0" text-rotation="0" transparent="false" value-precision="2" wheel-inc-value="0.1"/>
		<view background-offset="0, 0" class="CVerticalSwitch" default-value="0" height-of-one-image="0" max-value="1" min-value="0" mouse-enabled="true" opacity="1" origin="20, 330" size="20, 20" sub-pixmaps="0" transparent="false" wheel-inc-value="0.1"/>
		<view class="CView" mouse-enabled="true" opacity="1" origin="50, 330" size="20, 20" transparent="false"/>
		<view background-offset="0, 0" class="CVuMeter" decrease-step-value="0.1" default-value="0.5" max-value="1" min-value="0" mouse-enabled="true" num-led="100" opacity="1" orientation="vertical" origin="90, 330" size="20, 20" transparent="false" wheel-inc-value="0.1"/>
		<view back-color="~ BlackCColor" background-offset="0, 0" class="CXYPad" default-value="0.5" font="~ NormalFont" font-antialias="true" font-color="~ WhiteCColor" frame-color="~ BlackCColor" frame-width="1" max-value="2" min-value="0" mouse-enabled="true" opacity="1" origin="140, 330" round-rect-radius="6" shadow-color="~ RedCColor" size="100, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-round-rect="false" style-shadow-text="false" text-alignment="center" text-inset="0, 0" text-rotation="0" transparent="false" value-precision="2" wheel-inc-value="0.1"/>
	</template>
	<gradients>
		<gradient name="Default TextButton Gradient">
			<color-stop rgba="#dcdcdcff" start="0"/>
			<color-stop rgba="#b4b4b4ff" start="1"/>
		</gradient>
		<gradient name="Default TextButton Gradient Highlighted">
			<color-stop rgba="#b4b4b4ff" start="0"/>
			<color-stop rgba="#646464ff" start="1"/>
		</gradient>
	</gradients>
</vstgui-ui-description>
)";

} // anonymous

using StringPtrList = std::list<const std::string*>;

TESTCASE(UIDescriptionTests,

	TEST(parseEmpty,
		Xml::MemoryContentProvider provider (emptyUIDesc, strlen(emptyUIDesc));
		UIDescription desc (&provider);
		EXPECT(desc.parse () == true);
		EXPECT(desc.getGradient ("t") == nullptr);
		EXPECT(desc.getBitmap ("b") == nullptr);
		EXPECT(desc.getFont ("f") == nullptr);
		EXPECT(desc.getTagForName ("t") == -1);
		CColor c;
		EXPECT(desc.getColor("c", c) == false);
		EXPECT(desc.getControlListener ("t") == nullptr);
		EXPECT(desc.getController () == nullptr);
	);
	
	TEST(colors,
		Xml::MemoryContentProvider provider (colorNodesUIDesc, strlen(colorNodesUIDesc));
		UIDescription desc (&provider);
		EXPECT(desc.parse () == true);
		CColor c;
		EXPECT(desc.getColor ("c1", c));
		EXPECT(c == CColor (0, 0, 0, 255));
		EXPECT(desc.getColor ("c2", c));
		EXPECT(c == CColor (255, 255, 255, 255));
		EXPECT(desc.getColor ("c3", c));
		EXPECT(c == CColor (255, 0, 0, 100));
		EXPECT(desc.getColor ("c4", c));
		EXPECT(c == CColor (0, 255, 0, 150));
		EXPECT(desc.getColor ("c5", c));
		EXPECT(c == CColor (255, 0, 255, 100));

		StringPtrList names;
		desc.collectColorNames (names);
		uint32_t numNames = 0;
		for (auto& name : names)
		{
			if (name->at (0) != '~')
				numNames++;
		}
		EXPECT(numNames == 5);

		desc.changeColor ("c5", CColor (0, 255, 0, 255));
		EXPECT(desc.getColor ("c5", c));
		EXPECT(c == CColor (0, 255, 0, 255));

		desc.changeColor ("added color node", CColor (1, 2, 3, 4));
		EXPECT(desc.hasColorName ("added color node"));

		auto name = desc.lookupColorName (CColor (0, 255, 0, 255));
		EXPECT(name == std::string ("c5"));
		desc.changeColorName ("c5", "new color");
		EXPECT(desc.hasColorName ("new color"));
		desc.removeColor ("new color");
		EXPECT(desc.getColor ("new color", c) == false);
	);

	TEST(fonts,
		Xml::MemoryContentProvider provider (fontNodesUIDesc, strlen(fontNodesUIDesc));
		UIDescription desc (&provider);
		EXPECT(desc.parse () == true);
		EXPECT(desc.hasFontName ("f1"));
		auto font = desc.getFont ("f1");
		EXPECT(font->getName () == std::string ("Arial"));
		EXPECT(font->getSize() == 8);
		EXPECT(font->getStyle() == kNormalFace);
		font = desc.getFont("f2");
		EXPECT(font->getName () == std::string ("Arial"));
		EXPECT(font->getSize() == 8);
		EXPECT(font->getStyle() == kBoldFace);
		font = desc.getFont("f3");
		EXPECT(font->getName () == std::string ("Arial"));
		EXPECT(font->getSize() == 8);
		EXPECT(font->getStyle() == kItalicFace);
		font = desc.getFont("f4");
		EXPECT(font->getName () == std::string ("Arial"));
		EXPECT(font->getSize() == 8);
		EXPECT(font->getStyle() == kUnderlineFace);
		font = desc.getFont("f5");
		EXPECT(font->getName () == std::string ("Arial"));
		EXPECT(font->getSize() == 8);
		EXPECT(font->getStyle() == kStrikethroughFace);
		font = desc.getFont("f6");
		EXPECT(font->getName () == std::string ("Arial"));
		EXPECT(font->getSize() == 8);
		std::string altFontNames;
		EXPECT(desc.getAlternativeFontNames("f5", altFontNames) == false);
		EXPECT(desc.getAlternativeFontNames("f6", altFontNames));
		EXPECT(altFontNames == "Arial, Courier");
		desc.changeAlternativeFontNames("f6", "Courier");
		desc.getAlternativeFontNames("f6", altFontNames);
		EXPECT(altFontNames == "Courier");
		auto name = desc.lookupFontName (font);
		EXPECT(name == std::string ("f6"));
		StringPtrList names;
		desc.collectFontNames (names);
		uint32_t numNames = 0;
		for (auto& name : names)
		{
			if (name->at (0) != '~')
				numNames++;
		}
		EXPECT(numNames == 6);
		desc.changeFontName ("f1", "font");
		EXPECT(desc.hasFontName ("font"));
		auto newFont = owned (new CFontDesc (*font));
		desc.changeFont ("font", newFont);
		desc.changeFont ("font2", newFont);
		EXPECT(desc.getFont("font") == newFont);
		EXPECT(desc.getFont("font2") == newFont);
		desc.removeFont("font");
		EXPECT(desc.hasFontName ("font") == false);
	);
	
	TEST(bitmaps,
		Xml::MemoryContentProvider provider (bitmapNodesUIDesc, strlen(bitmapNodesUIDesc));
		UIDescription desc (&provider);
		EXPECT(desc.parse () == true);
		EXPECT(desc.hasBitmapName ("b1"));
		auto bitmap = desc.getBitmap ("b1");
		EXPECT(bitmap);
		auto name = desc.lookupBitmapName (bitmap);
		EXPECT(name == std::string ("b1"));
		StringPtrList names;
		desc.collectBitmapNames (names);
		EXPECT(names.size () == 2);
		desc.changeBitmapName ("b1", "new bitmap");
		EXPECT(desc.hasBitmapName ("new bitmap"));
		desc.removeBitmap ("new bitmap");
		EXPECT(desc.hasBitmapName ("new bitmap") == false);
		CRect ninePartTiledOffset (10, 10, 10, 10);
		desc.changeBitmap ("added bitmap node", "path to bitmap", &ninePartTiledOffset);
		EXPECT(desc.hasBitmapName ("added bitmap node"));
		bitmap = desc.getBitmap ("added bitmap node");
		EXPECT(dynamic_cast<CNinePartTiledBitmap*>(bitmap));
		auto& offsets = dynamic_cast<CNinePartTiledBitmap*>(bitmap)->getPartOffsets();
		EXPECT(offsets.left == 10 && offsets.top == 10 && offsets.right == 10 && offsets.bottom == 10);
		desc.changeBitmap ("added bitmap node", "added bitmap node", nullptr);
		bitmap = desc.getBitmap ("added bitmap node");
		EXPECT(dynamic_cast<CNinePartTiledBitmap*>(bitmap) == nullptr);
	);
	
	TEST(tags,
		Xml::MemoryContentProvider provider (tagNodesUIDesc, strlen(tagNodesUIDesc));
		UIDescription desc (&provider);
		EXPECT(desc.parse () == true);
		EXPECT(desc.hasTagName ("t1"));
		EXPECT(desc.getTagForName ("t1") == 1234);
		EXPECT(desc.getTagForName ("t3") == 1836676199);
		auto name = desc.lookupControlTagName (1234);
		EXPECT(name == std::string ("t1"));
		StringPtrList names;
		desc.collectControlTagNames (names);
		EXPECT(names.size () == 3);
		desc.changeTagName("t1", "control tag");
		EXPECT(desc.hasTagName("control tag"));
		desc.changeControlTagString ("control tag", "4567 - 5");
		EXPECT(desc.getTagForName ("control tag") == 4562);
		std::string tagString;
		EXPECT(desc.getControlTagString ("control not existing", tagString) == false);
		EXPECT(desc.getControlTagString ("control tag", tagString));
		EXPECT(tagString == "4567 - 5");
		desc.removeTag ("control tag");
		EXPECT(desc.hasTagName("control tag") == false);
		desc.changeControlTagString ("new control tag", "2*2", true);
		EXPECT(desc.getTagForName ("new control tag") == 4);
	);

	TEST(lookupTagsCalculateTag,
		Xml::MemoryContentProvider provider (calculateTagNodesUIDesc, strlen(calculateTagNodesUIDesc));
		UIDescription desc (&provider);
		EXPECT(desc.parse () == true);
		auto name = desc.lookupControlTagName (3);
		EXPECT(name);
		EXPECT(std::string (name) == "t1");
	);
	
	TEST(gradient,
		Xml::MemoryContentProvider provider (gradientNodesUIDesc, strlen(gradientNodesUIDesc));
		UIDescription desc (&provider);
		EXPECT(desc.parse () == true);
		EXPECT(desc.hasGradientName ("g1"));
		auto gradient = desc.getGradient ("g1");
		const auto& colorStops = gradient->getColorStops ();
		EXPECT(colorStops.size() == 3);
		auto it = colorStops.find(0.);
		EXPECT(it != colorStops.end ());
		EXPECT(it->second == CColor (0, 0, 0, 255));
		it = colorStops.find(0.5);
		EXPECT(it != colorStops.end ());
		EXPECT(it->second == CColor (255, 0, 0, 255));
		it = colorStops.find(1.);
		EXPECT(it != colorStops.end ());
		EXPECT(it->second == CColor (255, 255, 255, 255));
		auto name = desc.lookupGradientName (gradient);
		EXPECT(name == std::string ("g1"));
		StringPtrList names;
		desc.collectGradientNames(names);
		EXPECT(names.size() == 1);
		desc.changeGradientName ("g1", "gradient");
		EXPECT(desc.hasGradientName ("gradient"));
		EXPECT(desc.hasGradientName ("g1") == false);
		auto newGradient = owned (CGradient::create(0., 1., kWhiteCColor, kBlackCColor));
		desc.changeGradient ("gradient", newGradient);
		EXPECT(desc.getGradient ("gradient") == newGradient);
		desc.changeGradient ("gradientnew", newGradient);
		EXPECT(desc.hasGradientName ("gradientnew"));
		desc.removeGradient ("gradientnew");
		EXPECT(desc.hasGradientName ("gradientnew") == false);
	);

	TEST(variables,
		Xml::MemoryContentProvider provider (variableNodesUIDesc, strlen(variableNodesUIDesc));
		UIDescription desc (&provider);
		EXPECT(desc.parse () == true);
		double value;
		EXPECT(desc.getVariable ("v1", value));
		EXPECT(value == 10.);
		std::string strValue;
		EXPECT(desc.getVariable ("v2", strValue));
		EXPECT(strValue == "string");
		EXPECT(desc.getVariable ("v3", strValue));
		EXPECT(strValue == "string");
		EXPECT(desc.getVariable ("v4", value));
		EXPECT(value == 20.5);
		EXPECT(desc.getVariable ("v5", value));
		EXPECT(value == 20.);
		EXPECT(desc.getVariable ("v6", strValue));
		EXPECT(strValue == "");
	);
	
	TEST(calculations,
		Xml::MemoryContentProvider provider (tagNodesUIDesc, strlen(tagNodesUIDesc));
		UIDescription desc (&provider);
		EXPECT(desc.parse () == true);
		double value;
		EXPECT(desc.calculateStringValue ("1", value));
		EXPECT(value == 1.);
		EXPECT(desc.calculateStringValue ("1+1", value));
		EXPECT(value == 2.);
		EXPECT(desc.calculateStringValue ("(1+1)*2", value));
		EXPECT(value == 4.);
		EXPECT(desc.calculateStringValue ("(1+1)*2-(3/3 + (0.5+0.5))", value));
		EXPECT(value == 2.);
		EXPECT(desc.calculateStringValue ("tag.t1 - 4", value));
		EXPECT(value == 1230.);
		EXPECT(desc.calculateStringValue ("(1+5*3", value) == false);
		EXPECT(desc.calculateStringValue ("tag.unknown - 4", value) == false);
		EXPECT(desc.calculateStringValue ("var.unknown", value) == false);
		EXPECT(desc.calculateStringValue ("unknown", value) == false);
	);

	TEST(writeToStream,
		std::string str (withAllNodesUIDesc);
		Xml::MemoryContentProvider provider (str.data (), static_cast<uint32_t> (str.size ()));
		SaveUIDescription desc (&provider);
		EXPECT(desc.parse () == true);
		CMemoryStream outputStream (1024, 1024, false);
		EXPECT(desc.saveToStream (outputStream, SaveUIDescription::kWriteImagesIntoXMLFile));
		outputStream.end ();
		std::string result (reinterpret_cast<const char*> (outputStream.getBuffer ()));
		EXPECT(result.size() == str.size ());
		EXPECT(result == str);
	);
	
	TEST(templates,
		Xml::MemoryContentProvider provider (createViewUIDesc, strlen(createViewUIDesc));
		UIDescription desc (&provider);
		EXPECT(desc.parse () == true);
		
		auto attributes = desc.getViewAttributes ("view");
		EXPECT(attributes);
		auto classAttr = attributes->getAttributeValue (UIViewCreator::kAttrClass);
		EXPECT(classAttr);
		EXPECT(*classAttr == "CViewContainer");
		attributes = desc.getViewAttributes ("view not existing");
		EXPECT(attributes == nullptr);
		
		Controller controller;
		auto view = owned (desc.createView ("view", &controller));
		EXPECT(view);
		auto viewContainer = view.cast<CViewContainer>();
		EXPECT(viewContainer);
		EXPECT(viewContainer->getNbViews () == 1);
		StringPtrList names;
		desc.collectTemplateViewNames(names);
		EXPECT(names.size() == 1);
		EXPECT(desc.duplicateTemplate ("view not existing", "viewcopy") == false);
		EXPECT(desc.duplicateTemplate ("view", "viewcopy"));
		EXPECT(desc.addNewTemplate ("view", nullptr) == false);
		names.clear();
		desc.collectTemplateViewNames(names);
		EXPECT(names.size() == 2);
		EXPECT(desc.changeTemplateName ("viewcopy", "copyOfView"));
		EXPECT(desc.changeTemplateName ("viewcopy", "copyOfView") == false);
		view = owned (desc.createView ("copyOfView", &controller));
		EXPECT(view);

		std::string name;
		desc.getTemplateNameFromView (view, name);
		EXPECT(name == "copyOfView");
		EXPECT(desc.removeTemplate ("view which does not exirt") == false);
		EXPECT(desc.removeTemplate ("copyOfView"));
		view = owned (desc.createView ("copyOfView", &controller));
		EXPECT(view == nullptr);

		auto a = new UIAttributes ();
		a->setAttribute (UIViewCreator::kAttrClass, "CViewContainer");
		EXPECT(desc.addNewTemplate ("addNewTemplate", a));
		EXPECT(desc.getViewAttributes ("addNewTemplate"));
	);

	TEST(storeRestoreViews,
		Xml::MemoryContentProvider provider (createViewUIDesc, strlen(createViewUIDesc));
		UIDescription desc (&provider);
		EXPECT(desc.parse () == true);

		Controller controller;
		auto view = owned (desc.createView ("view", &controller));
		EXPECT(view);

		CMemoryStream memoryStream (1024, 1024, false);
		std::list<SharedPointer<CView>> restoredView;

		UIAttributes customAttributes;
		customAttributes.setAttribute ("Test", "Value");

		EXPECT(desc.storeViews ({view.cast<CViewContainer>()->getView (0)}, memoryStream, &customAttributes));
		memoryStream.rewind ();

		UIAttributes* customAttributesRestored = nullptr;
		EXPECT(desc.restoreViews (memoryStream, restoredView, &customAttributesRestored));
		EXPECT(customAttributesRestored);
		EXPECT(*customAttributesRestored->getAttributeValue("Test") == "Value");
	);

	TEST(storeRestoreViewsAttached,
		Xml::MemoryContentProvider provider (restoreViewUIDesc, strlen(restoreViewUIDesc));
		UIDescription desc (&provider);
		EXPECT(desc.parse () == true);

		Controller controller;
		auto view = owned (desc.createView ("view", &controller));
		EXPECT(view);

		CMemoryStream memoryStream (1024, 1024, false);
		std::list<SharedPointer<CView>> restoredView;

		auto parentContainer = owned (new CViewContainer (CRect (0, 0, 10, 10)));
		view->attached (parentContainer);
		auto viewToRestore = SharedPointer<CView> (view.cast<CViewContainer>()->getView (1));
		viewToRestore = viewToRestore.cast<CViewContainer> ()->getView (0);
		
		memoryStream.rewind ();
		EXPECT(desc.storeViews ({viewToRestore}, memoryStream));
		memoryStream.rewind ();
		restoredView.clear ();
		EXPECT(desc.restoreViews(memoryStream, restoredView));
		view->removed (parentContainer);
	);

	TEST(updateViewDescription,
		Xml::MemoryContentProvider provider (createViewUIDesc, strlen(createViewUIDesc));
		UIDescription desc (&provider);
		EXPECT(desc.parse () == true);
		Controller controller;
		auto view = owned (desc.createView ("view", &controller));
		EXPECT(view);
		EXPECT(view->getTransparency () == false);
		view->setTransparency (true);
		desc.updateViewDescription ("view", view);
		auto attr = desc.getViewAttributes ("view");
		bool value;
		EXPECT(attr->getBooleanAttribute ("transparent", value));
		EXPECT(value == true);
	);

	TEST(customAttributes,
		Xml::MemoryContentProvider provider (createViewUIDesc, strlen(createViewUIDesc));
		UIDescription desc (&provider);
		EXPECT(desc.parse () == true);
		auto attr = desc.getCustomAttributes("Test", false);
		EXPECT(attr == nullptr);
		attr = desc.getCustomAttributes("Test", true);
		EXPECT(attr);
		EXPECT(desc.getCustomAttributes("Test", false) == attr);
		EXPECT(desc.setCustomAttributes("Test", nullptr) == false);
	);

);

#if 0
	TEST(completeExample,
		Xml::MemoryContentProvider provider (completeExample, strlen(completeExample));
		SaveUIDescription desc (&provider);
		EXPECT(desc.parse () == true);
		auto view = owned (desc.createView ("view", nullptr));
		view = owned (desc.createView ("tab1", nullptr));
		view = owned (desc.createView ("tab2", nullptr));
		CMemoryStream outputStream (1024, 1024, false);
		EXPECT(desc.saveToStream (outputStream, SaveUIDescription::kWriteImagesIntoXMLFile));
	);
#endif

} // VSTGUI
