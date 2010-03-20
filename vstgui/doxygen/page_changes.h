/**
@page changes_from_3_0 Changes from earlier versions of VSTGUI

- @ref version4_introduction @n
- @ref new_stuff @n
- @ref code_changes_for_3_6 @n
- @ref old_new_stuff @n
- @ref cocoa_support @n

@section version4_introduction Introduction

Version 4 of VSTGUI is a new milestone release with a restructured code base with the focus of code conformity and easier future enhancements.
The result is that code written for any earlier version of VSTGUI is not always compatible.
It's recomended to start new projects with version 4 while old projects should stay with version 3.6.

@section new_stuff New Stuff

- UIDescription : Building user interfaces via XML description files. See @ref uidescription @n
- VST3 Support : Complete inline VST3 Editor support. See @ref uidescription_vst3_support @n
- Animation Support : Simple to use animations. See @ref animation
- Amalgamation : Easy integration in your projects via one or two source files
- Cleaned Code : Removed all deprecated methods and classes, splittet individual classes into different files
- Platform Abstraction : Platform dependent code was refactored and moved into its own files
- New notable classes : VSTGUI::CCheckBox, VSTGUI::CGraphicsPath, VSTGUI::CNinePartTiledBitmap, VSTGUI::IFocusDrawing

@section code_changes_for_3_6 Code changes for existing VSTGUI 3.6 code

- your custom views need to use the new mouse methods
- COptionMenuScheme is not available anymore
- VSTGUI::CFileSelector is gone, you have to use VSTGUI::CNewFileSelector
- VST extensions previously enabled via ENABLE_VST_EXTENSION_IN_VSTGUI is gone without replacement
- VSTGUI::CBitmap was completely changed and does not use a transparency color anymore, you need to use the alpha channel of a bitmap to get the same results
- VSTGUI::CControl::setValue (value) changed to VSTGUI::CControl::setValue (value, updateSubListeners)
- VSTGUI::COffscreenContext is handled completely different. But in most cases you can simply remove all offscreens where you needed them to reduce flicker.
- On Windows graphics are entirely drawn with GDI+ or Direct2D (when available), GDI is not used anymore
- The internal string encoding is now always UTF-8
- The VSTGUI::CCoord type is now always a double

@section old_new_stuff Stuff that was new in VSTGUI 3.0 - VSTGUI 3.6

@section new_mouse_methods New mouse methods

In earlier versions there were only one method in CView for handling mouse events (VSTGUI::CView::mouse). 
In this version there are five new methods :
- VSTGUI::CView::onMouseDown	(new in 3.5)
- VSTGUI::CView::onMouseUp	(new in 3.5)
- VSTGUI::CView::onMouseMoved	(new in 3.5)
- VSTGUI::CView::onMouseEntered	(new in 3.5)
- VSTGUI::CView::onMouseExited	(new in 3.5)

For convenience the old method is still working, but should be replaced with the ones above.

@section other_new_things Other new things

- VSTGUI::CDataBrowser	(new in 3.5)
- VSTGUI::CScrollView	(new in 3.0)
- VSTGUI::CTabView		(new in 3.0)

- Mac OS X 64 bit support via Cocoa. (new in 3.6)
- New Fileselector class : VSTGUI::CNewFileSelector (new in 3.6)
- VSTGUI::COptionMenu refactored. Supports icons for menu items. (new in 3.6)
- View autoresizing support. (new in 3.6)
- Bitmaps can be loaded either by number or by name (see VSTGUI::CBitmap)	(new in 3.5)
- VSTGUI::CTooltipSupport	(new in 3.5)
- VSTGUI::CVSTGUITimer	(new in 3.5)
- System event driven drawing	(new in 3.5)
- Unicode support via UTF-8 (new in 3.5)
- New font implementation	(new in 3.5)
- Windows GDI+ support	(new in 3.5)
- Mac OS X Composited Window support (new in 3.0)

@section about_deprecation About deprecation in version 3.6

With VSTGUI 3.6 the VSTGUI_ENABLE_DEPRECATED_METHODS macro has changed to be zero per default. You should change your code so that
it compiles without changing the macro. All methods marked this way will be unavailable in the next version.

@section code_changes_for_3_5 Code changes for existing VSTGUI 3.5 code

- COptionMenu was refactored and uses the CMenuItem class for menu items. Item flags are not encoded in the item title anymore.
- CParamDisplay::setTxtFace () and CParamDisplay::getTxtFace () is gone. The text face is already in CFontRef.
- You need to use the new CNewFileSelector class instead of CFileSelector if you want to use it on Mac 64 bit.

@section code_changes_for_3_0 Code changes for existing VSTGUI 3.0 code

- Per default CBitmaps don't get a transparent color on creation. You must call bitmap->setTransparency (color) explicitly. And this may only works once depending on the internal implementation.
- CViewContainer addView and removeView returns a bool value now.
- Mouse methods moved from CDrawContext to CFrame.
- Custom views which override attached and removed must propagate the call to the parent.
- VST specific code is enclosed with the macro ENABLE_VST_EXTENSION_IN_VSTGUI, which per default is set to zero. If you need them you must enable it (best practice is to set it in the prefix header or the preprocessor panel in your compiler).
- Removed all CDrawContext parameters from CView methods except for draw and drawRect. You need to change this in your custom views and controls.
- Every usage of CFont must be changed to CFontRef.
- When using GDI+ or libpng on Windows there is no need in using any offscreen context for flicker reduction as VSTGUI uses a backbuffer for drawing.
- Custom controls must implement the CLASS_METHODS macro if it directly inherits from CControl. Otherwise you will get a compile error.
- Custom controls which don't implement the new mouse methods must override onMouseDown and return kMouseEventNotHandled so that the old mouse method is called.

@subsection cviewchanges CView method changes

For custom views you need to change the following methods because their parameters changed:

- onWheel
- onDrop
- onDragEnter
- onDragLeave
- onDragMove
- takeFocus
- looseFocus
- setViewSize

@subsection aeffguieditorchanges AEffGUIEditor method changes

- valueChanged

@section code_changes_for_2_3 Code changes for existing VSTGUI 2.3 code

please see the "Migrating from 2.3.rtf" file in the Documentation folder.

@section cocoa_support Cocoa notes

- To get cocoa support your minimum required Mac OS X version is 10.5.
- In 32 bit Cocoa and Carbon are available. You can switch between them with CFrame::setCocoaMode(bool state). You must do this before creating the CFrame.
- In 64 bit only Cocoa is available.
- The pSystemWindow pointer in the CFrame constructor must be a NSView not a NSWindow.

*/
