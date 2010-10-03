//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef __vst3editortemplates__
#define __vst3editortemplates__

#include "../lib/vstguibase.h"

#if VSTGUI_LIVE_EDITING

namespace VSTGUI {

static UTF8StringPtr vst3EditorTemplatesString = VSTGUI_MAKE_STRING(
<?xml version="1.0" encoding="UTF-8"?>
<vstgui-ui-description version="1">
	<fonts>
		<font font-name="Helvetica" name="labelfont" size="12"/>
	</fonts>
	<colors>
		<color name="transparent" rgba="#00000000"/>
		<color name="black" rgba="#000000ff"/>
		<color name="lightgrey" rgba="#cccccc66"/>
	</colors>
	<control-tags>
		<control-tag name="0" tag="0"/>
		<control-tag name="1" tag="1"/>
		<control-tag name="2" tag="2"/>
		<control-tag name="3" tag="3"/>
		<control-tag name="4" tag="4"/>
		<control-tag name="5" tag="5"/>
		<control-tag name="6" tag="6"/>
	</control-tags>
	<template background-color="black" class="CViewContainer" name="CreateNewTemplate" origin="0, 0" size="300, 110" transparent="true">
		<view back-color="lightgrey" background-offset="0, 0" class="CTextLabel" default-value="0.5" font="labelfont" font-antialias="true" font-color="black" frame-color="transparent" max-value="1" min-value="0" origin="15, 15" shadow-color="#ff0000ff" size="70, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="true" style-no-text="false" style-shadow-text="false" text-alignment="right" text-inset="5, 0" title="Name:" transparent="true" wheel-inc-value="0.1"/>
		<view back-color="lightgrey" background-offset="0, 0" class="CTextEdit" control-tag="0" default-value="0.5" font="labelfont" font-antialias="true" font-color="black" frame-color="black" max-value="1" min-value="0" origin="95, 15" shadow-color="#ff0000ff" size="190, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-shadow-text="false" text-alignment="center" text-inset="0, 0" transparent="false" wheel-inc-value="0.1"/>
		<view back-color="lightgrey" background-offset="0, 0" class="CTextLabel" default-value="0.5" font="labelfont" font-antialias="true" font-color="black" frame-color="transparent" max-value="1" min-value="0" origin="15, 45" shadow-color="#ff0000ff" size="70, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="true" style-no-text="false" style-shadow-text="false" text-alignment="right" text-inset="5, 0" title="Width:" transparent="true" wheel-inc-value="0.1"/>
		<view back-color="lightgrey" background-offset="0, 0" class="CTextEdit" control-tag="1" default-value="0.5" font="labelfont" font-antialias="true" font-color="black" frame-color="black" max-value="1" min-value="0" origin="95, 45" shadow-color="#ff0000ff" size="190, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-shadow-text="false" text-alignment="center" text-inset="0, 0" transparent="false" wheel-inc-value="0.1"/>
		<view back-color="lightgrey" background-offset="0, 0" class="CTextLabel" default-value="0.5" font="labelfont" font-antialias="true" font-color="black" frame-color="transparent" max-value="1" min-value="0" origin="15, 75" shadow-color="#ff0000ff" size="70, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="true" style-no-text="false" style-shadow-text="false" text-alignment="right" text-inset="5, 0" title="Height:" transparent="true" wheel-inc-value="0.1"/>
		<view back-color="lightgrey" background-offset="0, 0" class="CTextEdit" control-tag="2" default-value="0.5" font="labelfont" font-antialias="true" font-color="black" frame-color="black" max-value="1" min-value="0" origin="95, 75" shadow-color="#ff0000ff" size="190, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-shadow-text="false" text-alignment="center" text-inset="0, 0" transparent="false" wheel-inc-value="0.1"/>
	</template>
	<template background-color="transparent" class="CViewContainer" name="TemplateSettings" origin="0, 0" size="300, 250" transparent="true">
		<view background-color="lightgrey" class="CViewContainer" origin="15, 15" size="270, 115" transparent="false">
			<view back-color="transparent" background-offset="0, 0" class="CTextLabel" default-value="0.5" font="labelfont" font-antialias="true" font-color="black" frame-color="transparent" max-value="1" min-value="0" origin="10, 85" shadow-color="#ff0000ff" size="80, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-shadow-text="false" text-alignment="left" text-inset="5, 0" title="Max Height:" transparent="true" wheel-inc-value="0.1"/>
			<view back-color="transparent" background-offset="0, 0" class="CTextLabel" default-value="0.5" font="labelfont" font-antialias="true" font-color="black" frame-color="transparent" max-value="1" min-value="0" origin="10, 60" shadow-color="#ff0000ff" size="80, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-shadow-text="false" text-alignment="left" text-inset="5, 0" title="Max Width:" transparent="true" wheel-inc-value="0.1"/>
			<view back-color="transparent" background-offset="0, 0" class="CTextLabel" default-value="0.5" font="labelfont" font-antialias="true" font-color="black" frame-color="transparent" max-value="1" min-value="0" origin="10, 35" shadow-color="#ff0000ff" size="80, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-shadow-text="false" text-alignment="left" text-inset="5, 0" title="Min Height:" transparent="true" wheel-inc-value="0.1"/>
			<view back-color="transparent" background-offset="0, 0" class="CTextLabel" default-value="0.5" font="labelfont" font-antialias="true" font-color="black" frame-color="transparent" max-value="1" min-value="0" origin="10, 10" shadow-color="#ff0000ff" size="80, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-shadow-text="false" text-alignment="left" text-inset="5, 0" title="Min Width:" transparent="true" wheel-inc-value="0.1"/>
			<view back-color="transparent" background-offset="0, 0" class="CTextEdit" control-tag="0" default-value="0.5" font="labelfont" font-antialias="true" font-color="black" frame-color="black" max-value="1" min-value="0" origin="100, 10" shadow-color="#ff0000ff" size="160, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-shadow-text="false" text-alignment="center" text-inset="0, 0" transparent="false" wheel-inc-value="0.1"/>
			<view back-color="transparent" background-offset="0, 0" class="CTextEdit" control-tag="1" default-value="0.5" font="labelfont" font-antialias="true" font-color="black" frame-color="black" max-value="1" min-value="0" origin="100, 35" shadow-color="#ff0000ff" size="160, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-shadow-text="false" text-alignment="center" text-inset="0, 0" transparent="false" wheel-inc-value="0.1"/>
			<view back-color="transparent" background-offset="0, 0" class="CTextEdit" control-tag="2" default-value="0.5" font="labelfont" font-antialias="true" font-color="black" frame-color="black" max-value="1" min-value="0" origin="100, 60" shadow-color="#ff0000ff" size="160, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-shadow-text="false" text-alignment="center" text-inset="0, 0" transparent="false" wheel-inc-value="0.1"/>
			<view back-color="transparent" background-offset="0, 0" class="CTextEdit" control-tag="3" default-value="0.5" font="labelfont" font-antialias="true" font-color="black" frame-color="black" max-value="1" min-value="0" origin="100, 85" shadow-color="#ff0000ff" size="160, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-shadow-text="false" text-alignment="center" text-inset="0, 0" transparent="false" wheel-inc-value="0.1"/>
		</view>
		<view background-color="lightgrey" class="CViewContainer" origin="15, 145" size="270, 90" transparent="false">
			<view autosize-to-fit="false" background-offset="0, 0" boxfill-color="#ffffffff" boxframe-color="black" checkmark-color="#ff0000ff" class="CCheckBox" control-tag="6" default-value="0.5" draw-crossbox="false" font-color="black" max-value="1" min-value="0" origin="10, 5" size="150, 20" title="Enable Focus Drawing" transparent="false" wheel-inc-value="0.1"/>
			<view back-color="transparent" background-offset="0, 0" class="CTextLabel" default-value="0.5" font="labelfont" font-antialias="true" font-color="black" frame-color="transparent" max-value="1" min-value="0" origin="10, 30" shadow-color="#ff0000ff" size="80, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-shadow-text="false" text-alignment="right" text-inset="5, 0" title="Color:" transparent="true" wheel-inc-value="0.1"/>
			<view back-color="transparent" background-offset="0, 0" class="CTextLabel" default-value="0.5" font="labelfont" font-antialias="true" font-color="black" frame-color="transparent" max-value="1" min-value="0" origin="10, 55" shadow-color="#ff0000ff" size="80, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-shadow-text="false" text-alignment="right" text-inset="5, 0" title="Width:" transparent="true" wheel-inc-value="0.1"/>
			<view back-color="transparent" background-offset="0, 0" class="COptionMenu" control-tag="4" default-value="0.5" font="labelfont" font-antialias="true" font-color="black" frame-color="black" max-value="4.29497e+09" menu-check-style="true" menu-popup-style="true" min-value="0" origin="100, 30" shadow-color="#ff0000ff" size="160, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-shadow-text="false" text-alignment="center" text-inset="0, 0" transparent="false" wheel-inc-value="0.1"/>
			<view back-color="transparent" background-offset="0, 0" class="CTextEdit" control-tag="5" default-value="0.5" font="labelfont" font-antialias="true" font-color="black" frame-color="black" max-value="1" min-value="0" origin="100, 55" shadow-color="#ff0000ff" size="160, 20" style-3D-in="false" style-3D-out="false" style-no-draw="false" style-no-frame="false" style-no-text="false" style-shadow-text="false" text-alignment="center" text-inset="0, 0" transparent="false" wheel-inc-value="0.1"/>
		</view>
	</template>
</vstgui-ui-description>
);

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif
