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

@page page_vst3_inline_editing Inline UI Editing for VST3 (WYSIWYG)

- @ref vst3_intro @n
- @ref vst3_setup @n
- @ref building_editing @n
	- @ref vstgui_inspector @n
	- @ref contextmenu @n
- @ref parameter_binding @n
- @ref custom_view_creation @n
- @ref sub_controllers @n
- @ref templates @n
- @ref vst3_misc @n

<hr/>

@section vst3_intro Introduction

VSTGUI now supports easy and fast UI creation for VST3 plug-ins.

It will automatically support the following VST3 features (only if you have met the requirements for @ref parameter_binding):
- IParameterFinder (find parameter under mouse)
- IContextMenu (show host context menu on right click for an automatable parameter) [VST3.5 SDK required]

Note that you need at least VST SDK 3.1, any earlier version will not work.

<hr/>

@section vst3_setup Setup

- Add the following files to your project (in addition to the other vstgui files, see @ref page_setup)
	- plugin-bindings/vst3editor.cpp
	- Windows only: vstgui_uidescription_win32.cpp
	- Mac OS X only: vstgui_uidescription_mac.mm
- Add VSTGUI_LIVE_EDITING=1 to your preprocessor definitions (only for your debug builds)
- Modify your VST Edit Controller class to add the createView(..) method :
@code
IPlugView* PLUGIN_API MyEditController::createView (FIDString name)
{
	if (strcmp (name, ViewType::kEditor) == 0)
	{
		return new VSTGUI::VST3Editor (this, "view", "myEditor.uidesc");
	}
	return 0;
}
@endcode
- Create an empty myEditor.uidesc file and save it somewhere in your project file hierarchy
- On Windows add a line to your .rc file :
@verbatim
myEditor.uidesc DATA "realtive/path/to/myEditor.uidesc"
@endverbatim
- On Mac OS X add the myEditor.uidesc file to your Xcode project and make sure that it is copied to the Resources folder of the vst3 bundle

<hr/>

@section building_editing Building and editing

Now you can build your project, start any VST3 host and open an instance of your plugin.

If you open the editor of your plug-in you will initially see a black editor.

To start editing the user interface you need to make a right click on your editor and choose the "Enable Editing" menu item.
When you do this the @ref vstgui_inspector window will open and you can start editing.

If you are done with it, make sure to use the "Save..." command in the @ref contextmenu to save the changes to the myEditor.uidesc.
Then you can choose the "Disable Editing" command to test your user interface.

<hr/>

<img src="inspector_overview.png" align="right"/>

@subsection vstgui_inspector VSTGUI Inspector

The @ref vstgui_inspector is used for defining Bitmaps, Colors, Fonts and Tags and to edit view attributes.

On the top of the window are 5 tabs:
- @ref vstgui_inspector_attributes
- @ref vstgui_inspector_bitmaps
- @ref vstgui_inspector_colors
- @ref vstgui_inspector_fonts
- @ref vstgui_inspector_tags

@subsection vstgui_inspector_attributes Attributes Tab

This tab contains the attributes of the view or views currently selected in your editor.
The shown attributes depend on the selected view. If you have multiple views selected only those attributes are shown which are valid for all views.
If the values of the attributes differ between the views they are shown in a different color.

@subsection vstgui_inspector_bitmaps Bitmaps Tab

In this tab you define your bitmaps.

- the name column describes the name you use in the view attributes.
- the bitmap column describes the name in your resources.
- the NinePartsOffset is used for bitmaps which will use the VSTGUI::CNinePartTiledBitmap class to draw a tiled bitmap.
The value needs to be 4 integers describing the left, top, right, bottom offsets separated by commas. 

@subsection vstgui_inspector_colors Colors Tab

In this tab you define your colors.

- the name column describes the name you use in the view attributes.
- the color column describes the color value. If you click on it, a color chooser window will open and you can change the color.
You can switch to any other tab while the color chooser is open and you can still change the color.

There are predefined colors whose names start with a '~'. You cannot change any of them.

@subsection vstgui_inspector_fonts Fonts Tab

In this tab you define your fonts.

- the name column describes the name you use in the view attributes.
- the font column describes the font. If you click on it, a font chooser window will open and you can change the font.
You can switch to any other tab while the font chooser is open and you can still change the font.

There are predefined fonts whose names start with a '~'. You cannot change any of them.

@subsection vstgui_inspector_tags Tags Tab

In this tab you define your tags.

- the name column describes the name you use in the view attributes.
- the tag column describes the tags used in VSTGUI::CControl. See also @ref parameter_binding.

<hr/>

<img src="contextmenu.png" align="right"/>

@subsection contextmenu Context Menu

Here is a brief description of the items in the context menu :

- <b>Undo</b> : Undo last action
- <b>Redo</b> : Redo last undo action

- <b>Size To Fit</b> : Size views to fit according to its attributes (calls CView::sizeToFit() internally).
- <b>Unembed Views</b> : If you have a container view selected, it will move all children out of it and delete it.
- <b>Delete</b> : Delete all selected views.

- <b>Insert Subview</b> : Insert the view chosen in the submenu at the current mouse location. If a container view is selected it is placed into it, otherwise it uses the deepest container view found under the mouse.
- <b>Embed Into</b> : Insert the selected views into the container view chosen in the submenu.
- <b>Insert Template</b> : Insert the template chosen in the submenu at the current mouse location. If a container view is selected it is placed into it, otherwise it uses the deepest container view found under the mouse. See @ref templates.
- <b>Transform View Type</b> : Transform the selected view into the view chosen in the submenu.

- <b>Grid</b> : Set the grid option.

- <b>Save...</b> : Save the xml file to disk.

- <b>Show Hierarchy Browser</b> : Show the view hierarchy browser.

- <b>Template Settings...</b> : Open the template settings dialog.
- <b>Change Template</b> : Change the editor to another template.
- <b>Add New Template</b> : Create a new template and show it.

- <b>Sync Parameter Tags</b> : Sync the parameter ids of the VST Edit Controller with the tags. This is not undoable.

- <b>Disable Editing</b> : Disable editing. This also erases the undo stack.

<hr/>

@section parameter_binding Parameter Binding

If you've used the Parameter class provided by the VST3 SDK, you will get automatic parameter bindings between the controls of your editor
and the parameters in your VST Edit Controller.

The only thing you need to do is to declare the ids of the parameters as tags in the \ref vstgui_inspector (or use the 'Sync Parameter Tags' command in the @ref contextmenu) and set the tags of your controls to these ids.
After you've done this your VST Edit Controller will receive the beginEdit(..)/performEdit(..)/endEdit(..) calls when the user changes the controls and if the host
automates the parameter the control will also reflect these changes.

As an addition you can modify your VST Edit Controller to return specific parameter objects in the getParameterObject(int32 paramID) method for UI only needs, which are not
parameters of your VST audio processor. This way you can store view settings like the tab which is open when the user closes the editor so that you can restore it
when the user opens the editor again. You can look at the sources of the included 'uidescription test' project how this works.

<hr/>

@section custom_view_creation Creating Custom Views

If you need to create custom views, you can implement the VSTGUI::VST3EditorDelegate interface in your edit controller class.
The createCustomView method will be called if you set the 'custom-view-name' attribute in one of the views.

Another way to use your own views is to register them at runtime with the UIViewFactory. This method requires more work but has the advantage
that the view will be listed like the built-in views and changing attributes work on the fly. See \ref VSTGUI::IViewCreator.

<hr/>

@section sub_controllers Sub-Controllers

@ref sub_controllers are useful if you need finer control of your views.
You can define @ref sub_controllers for views with the 'sub-controller' attribute.
@ref sub_controllers will be created via the VSTGUI::VST3EditorDelegate interface. When they are created they are owned by the VSTGUI::VST3Editor object.

The VSTGUI::DelegationController is a helper class if you don't want to control every aspect of the views by forwarding every call to its parent controller.
You only overwrite the methods you need in your inherited class.

If you want to be notified about value changes for controls in your sub-controller but don't want to loose the
@ref parameter_binding you can add your sub-controller as dependent of the control:

@code
class MyController : public DelegationController, public CBaseObject
{
public:
	MyController (IController* baseController) : DelegationController (baseController), controlView (0) {}
	~MyController ()
	{
		if (controlView)
		{
			controlView->removeDependent (this);
			controlView->forget ();
		}
	}
	
	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
	{
		CControl* control = dynamic_cast<CControl*> (view);
		if (control && control->getTag () == 20)
		{
			controlView = control;
			controlView->addDependent (this);
			controlView->remember ();
		}
		return controller->verifyView (view, attributes, description);
	}
	
	CMessageResult notify (CBaseObject* sender, IdStringPtr message)
	{
		if (sender == controlView)
		{
			if (message == CControl::kMessageValueChanged)
			{
				// do something
			}
			return kMessageNotified;
		}
		return kMessageUnknown;
	}
	
protected:
	CControl* controlView;
};
@endcode

<hr/>

@section templates Templates

@ref Templates are root views where you can group controls in logical entities.
You can embed @ref templates into other @ref templates.
Some views like the VSTGUI::UIViewSwitchContainer shows different @ref templates depending on a control value.

<hr/>

@section vst3_misc Misc

- For your deployment versions make sure to set the VSTGUI_LIVE_EDITING definition to zero.

*/
