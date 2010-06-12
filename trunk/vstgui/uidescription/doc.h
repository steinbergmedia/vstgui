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

// VSTGUI UI Description Documentation

/*
	Inline editing todo:
		- Cut, copy, paste of selection (no high priority as you can copy views by alt dragging)
		- changing z-order of views (can be done with the hierarchy browser, but an inline solution would also be nice)
		- Point attributes should be editable as points not as string
		- A lasso selection would also be nice
		- delete templates
		- move views out of parents action
		
	UIDescription todo:
		- CTabView support (skipped, UIViewSwitchContainer is the replacement)
		- CNinePartTiledBitmap support
		- alternate font support (allow more than one font family for fallback when first font not available on system)

	Platform support:
		- complete Win32 support
*/


/**
@page uidescription Creating User Interfaces via XML

@section vstguiandxml VSTGUI and XML
It is now possible to create VSTGUI based interfaces via a XML description.

- @ref examplexml @n
- @ref creatingbycode @n
- @ref customviews @n
- @ref defbitmaps @n
- @ref deffonts @n
- @ref defcolors @n
- @ref deftags @n
- @ref deftemplates @n
- @subpage uidescription_attributes @n
- @subpage uidescription_vst3_support @n

@section examplexml Example XML file

First let us see a simple example of XML text describing a VSTGUI view hierarchy:
@verbatim
<?xml version="1.0" encoding="UTF-8"?>
<vstgui-ui-description version="1">
  <bitmaps>
    <bitmap name="background" path="background.png"/>
    <bitmap name="slider-handle" path="slider-handle.png"/>
    <bitmap name="slider-background" path="slider-background.png"/>
  </bitmaps>

  <fonts>
    <font name="labelfont" font-name="Arial" size="11" bold="false" italic="false"/>
  </fonts>

  <colors>
    <color name="labelcolor" red="255" green="0" blue="255" alpha="255"/>
  </colors>

  <control-tags>
    <control-tag name="tag 1" tag="0"/>
    <control-tag name="tag 2" tag="1"/>
  </control-tags>

  <template name="MyEditor" size="500, 320" background-color="#000000DD" minSize="500, 320" maxSize="1000, 320" autosize="left right">
    <view class="CViewContainer" origin="10, 10" size="480, 90" background-color="#FFFFFF22" autosize="left right">
      <view class="CTextLabel" title="Test Label" origin="10, 10" size="80,20" transparent="true" font-color="labelcolor" font="labelfont"/>
      <view class="CSlider" control-tag="tag 1" origin="110, 10" size="260,20" handle-offset="3,3" bitmap="slider-background" handle-bitmap="slider-handle" autosize="left right"/>
      <view class="CTextEdit" control-tag="tag 2" origin="390, 10" size="80,20" back-color="slider-back" frame-color="slider-frame" font-color="labelcolor" font="labelfont" autosize="right"/>
    </view>
  </template>
</vstgui-ui-description>
@endverbatim

@section creatingbycode Creating a view
You need to write a XML text file like the one in the example shown above.
On Mac OS X this xml file must be placed into the Resources folder of the bundle and on Windows it must be declared in the .rc file.
To use the xml file to create views you have to write this code :
@code
UIDescription description ("myview.xml");
if (description.parse ())
{
  CView* view = description.createView ("MyEditor", 0);
}
@endcode

If view is non-null it was successfully created and you can add it to your CFrame object.

@section customviews Creating custom views
If you want to create your own custom views, you have two options:
-# Create view factory methods for your custom views (look into viewcreator.cpp how this is done for the built in views)
-# Inherit a class from VSTGUI::IController and provide the view in the VSTGUI::IController::createView method.
An instance of this class must be passed as second argument to the createView method of VSTGUI::UIDescription.

@section defbitmaps Defining Bitmaps
Any bitmap you want to use with your views must be declared inside the \b bitmaps tag. Recognized attributes for the \b bitmap tag are:
- \b name
<br/>you refer to this name later on when you want to use this bitmap
- \b path
<br/>the path to the bitmap (On Mac OS X this is the path inside the Resource directory of the bundle and on Windows this is the name used in the .rc file)
.
Example:
@verbatim
<bitmaps>
  <bitmap name="background" path="background.png"/>
</bitmaps>
@endverbatim

@section deffonts Defining Fonts
Any font you want to use with your views must be declared inside the \b fonts tag. Recognized attributes for the \b font tag are:
- \b name
<br/>you refer to this name later on when you want to use this font
- \b font-name
<br/>the system font name
- \b size
<br/>size of the font
- \b bold
<br/>true or false
- \b italic
<br/>true or false
- \b underline
<br/>true or false
.
Example:
@verbatim
<fonts>
  <font name="labelfont" font-name="Arial" size="11" bold="false" italic="false"/>
</fonts>
@endverbatim

@section defcolors Defining Colors
You can define global colors within the \b colors tag. Recognized attributes for the \b color tag are:
- \b name
<br/>you refer to this name later on when you want to use this color
- \b red
<br/>the red value of this color in the range from 0 to 255
- \b green
<br/>the green value of this color in the range from 0 to 255
- \b blue
<br/>the blue value of this color in the range from 0 to 255
- \b alpha
<br/>the alpha value of this color in the range from 0 to 255
- \b rgb
<br/>the red, green and blue values in hex notation known from HTML and CSS: #0055BB (the alpha value of this color is always 255, and it overrides any previous attribute)
- \b rgba
<br/>the red, green, blue and alpha values in hex notation known from HTML and CSS: #005566FF (any previous attribute will be ignored)
.
Example:
@verbatim
<colors>
  <color name="labelcolor" rgba="#005566FF"/>
  <color name="labelcolor2" rgb="#005566"/>
  <color name="labelcolor3" red="0" green="85" blue="102" alpha="255"/>
  <color name="labelcolor4" green="85" blue="102"/>
  <!-- by the way, these colors have all the same rgba values -->
</colors>
@endverbatim
Colors can also be declared within the \b view tag for any color tag with one of the two hex notations.

@section deftags Defining Tags
VSTGUI controls are identified by tags. In the \b control-tags tag you map control tags to names. Recognized attributes in the \b control-tag tag are:
- \b name
<br/>you refer to this name later on when you want to use this control tag
- \b tag
<br/>an integer tag or a tag defined like 'abcd'
.
Example:
@verbatim
<control-tags>
  <control-tag name="tag 1" tag="0"/>
  <control-tag name="tag 2" tag="'abcd'"/>
</control-tags>
@endverbatim

@section deftemplates Defining Templates
Templates are the main views in XML. You can have more than one.
Per default the \b template tag will create a CViewContainer view, but you can use the \b class attribute to create any view class you want.
(If the template should have subviews, the class must be an inherited class from CViewContainer like CScrollView)<br/>

@page uidescription_vst3_support Inline Editing support for VST3.1

VSTGUI now supports easy and fast UI creation for VST3.1 plug-ins.

Note that you need at least VST SDK 3.1, any earlier version will not work.

@section vst3_setup Setup

First you need to add vstgui_uidescription_win32.cpp or vstgui_uidescription_mac.mm and vst3editor.cpp to your project
and define a preprocessor definition for VSTGUI_LIVE_EDITING=1.
Then you have to modify your edit controller class to create a VSTGUI::VST3Editor instance when asked to create it's view :
@code
IPlugView* PLUGIN_API MyEditController::createView (FIDString name)
{
	if (strcmp (name, ViewType::kEditor) == 0)
	{
		return new VST3Editor (this, "view", "myEditor.uidesc");
	}
	return 0;
}
@endcode
Next you have to create an empty myEditor.uidesc file.
For Windows you add a line to your .rc file to include it :
@verbatim
myEditor.uidesc DATA "realtive/path/to/myEditor.uidesc"
@endverbatim
On Mac OS X you just add the uidesc file to your resources so that it is placed into the Resources subfolder of the vst3 bundle.

Next you can build your plug-in and start your VST3 host and open an instance of your plug-in. If you open the editor you will see that
an empty black editor was automatically constructed. You can now make a right click on your editor and enable inline editing.

@section vst3_misc Misc

If you have enabled editing, a new window will open, where you can define tags, colors, fonts, bitmaps and edit view attributes.

To add new views just open the context menu (via right click) in your editor and choose the view type in the "Insert Subview" submenu.

You have automatic VST3 parameter support if you use the Steinberg::Vst::Parameter class in your edit controller. This way you just have to
define a tag in the "VSTGUI Inspector" window with the same value as the VST3 parameter ID. Now you can set any VSTGUI control's tag
to this tag and it will automatically be synced with the VST3 parameter.

If you need to create custom views, you can implement the VSTGUI::VST3EditorDelegate interface in your edit controller class.
The createCustomView method will be called if you set the 'custom-view-name' attribute in one of the views.

For your release versions make sure to set the VSTGUI_LIVE_EDITING definition to zero.
*/
