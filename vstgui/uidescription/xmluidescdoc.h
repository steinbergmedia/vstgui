// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

// VSTGUI UI Description Documentation

/*
	Inline editing todo:
		- Cut, copy, paste of selection (no high priority as you can copy views by alt dragging)
		- changing z-order of views (can be done with the hierarchy browser, but an inline solution would also be nice)
		- Point attributes should be editable as points not as string
		- A lasso selection would also be nice
		- delete templates
		
	UIDescription todo:
		- CTabView support (skipped, UIViewSwitchContainer is the replacement)
		- alternate font support (allow more than one font family for fallback when first font not available on system)

*/


/**
@page uidescriptionXML Creating User Interfaces via XML

@note The use of XML as description format is deprecated (since Version 4.10).

@section vstguiandxml VSTGUI and XML
It is possible to create VSTGUI based interfaces via a XML description.

- @ref examplexml @n
- @ref creatingbycode @n
- @ref customviews @n
- @ref defbitmaps @n
- @ref deffonts @n
- @ref defcolors @n
- @ref deftags @n
- @ref deftemplates @n
- @subpage uidescription_attributes @n

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

*/
