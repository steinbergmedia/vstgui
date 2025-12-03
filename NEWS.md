# News

### Version 4.15

- add support for custom view layouts (see IViewLayouter and CViewContainer::setViewLayouter).
- add a grid view layouter that is similar to CSS Grid (see GridLayouter).
- add Scripting for UIDescription (see uidescription-scripting/uiscripting.md)
- add new text editor view (see lib/ctexteditor.h)
- a scroll view can now have a top and a left edge view (see CScrollView::setEdgeView)
- preliminary Wayland support

### Version 4.14

- add crosshair mouse cursor (kCursorCrosshair)
- customizable knob range (see CKnob::setKnobRange)
- new layouts for CRowColumnView

### Version 4.13

- support embedding platform views (HWND & NSView) as sub views (see CExternalView and ExternalView::IView) and examples in the contrib folder.

### Version 4.12.2

- make it possible to draw only frames in a range for views using multi frame bitmaps.

### Version 4.12.1

- make it possible to use the new multi frame bitmap feature with custom value to frame index mappings by subclassing.

### Version 4.12

- new multi frame bitmap representation that allows to support more frames per bitmap on current
hardware by supporting to layout the frames in the bitmap by rows and multiple columns instead of
one column as before. See CMultiFrameBitmap. All included controls are updated to support
it, while the old method is deprecated but still supported for now.
- the image stitcher tool was updated to support the creation of multi row/column frames bitmap

### Version 4.11

- Using DirectComposition on Windows now with support for CLayeredViewContainer
- Removed 32-bit Carbon support
- Reworked event handling
- Reworked unit test framework to be able to debug the tests

### Version 4.10

- VSTGUI now needs to be initialized and terminated explicitly. See VSTGUI::init() and VSTGUI::exit().
- UIDescription files are now written in JSON format and the old XML format is deprecated
- It's now possible to conditionally remove the XML parser and the expat library from building (set VSTGUI_ENABLE_XML_PARSER to 0)
- This is the last version not depending on c++17 compiler support.

### Version 4.9

- new control: CListControl in play with CStringList
- custom font support: VSTGUI now supports using fonts embedded in its Bundle/Package at Resources/Fonts. Note that this works on Windows only when building with the Windows 10 SDK and it does also only work on Windows 10. There's no such restriction on macOS or Linux.

### Version 4.8

- new CSegmentButton selection mode kSingleToggle and styles kHorizontalInverse and kVerticalInverse.

### Version 4.7

- redesigned drag'n drop
- drags with bitmaps are now supported on Windows
- standalone library support for Windows 7
- new ImageStitcher tool
- the GDI+ draw backend was removed, the Direct2D backend is the replacement

### Version 4.6

- new Control: KeyboardView
- cmake cleanup
- fix static object initialization order
- fix build warnings/errors depending on macOS SDK use
- remove warnings

### Version 4.5

- cmake build system
- preview of the standalone library
- new Controls: CMultiLineTextLabel, CSearchTextEdit
- adopt many c++11 language features

### Version 4.4

- preview Linux version
- support for Windows XP, Mac OS X 10.6 and non c++11 mode will be removed with version 4.5

### Version 4.3

- last version to support Windows XP, Mac OS X 10.6 and non c++11 mode
- HiDPI support (aka Retina support) for Quartz2D and Direct2D backends
- support for creating a graphics path from a string
- new Control : CSegmentButton
- add support for adding a custom view to the split view separator
- transformation matrix support in CDrawContext
- alternative c++11 callback functions for CFileSelector::run(), CVSTGUITimer, CParamDisplay::setValueToStringFunction, CTextEdit::setStringToValueFunction and CCommandMenuItem::setActions

Note: All current deprecated methods will be removed in the next version. So make sure that your code compiles with VSTGUI_ENABLE_DEPRECATED_METHODS=0

### Version 4.2

- iOS Support with Multi Touch handling.
- support drawing an icon on a CTextButton
- CGradientView
- CDataBrowser now supports multi row selections
- support compiling in c++11 mode with clang and visual studio
- VSTGUI_OVERRIDE_VMETHOD is now used throughout the vstgui sources to indicate methods which are expecting to override a virtual method of its base classes. (c++11 only)

### Version 4.1

- UIDescription Editor
- COpenGLView (only Windows & Mac Cocoa)
- CRowColumnView
- CShadowViewContainer
- BitmapFilter
- More VCScrollView styles

### Version 4.0

- VST3 Support : Complete inline VST3 Editor support.
- UIDescription : Building user interfaces via XML description files.
- Animation Support : Simple to use animations.
- Amalgamation : Easy integration in your projects via one or two source files
- Cleaned Code : Removed all deprecated methods and classes, splittet individual classes into different files
- Platform Abstraction : Platform dependent code was refactored and moved into its own files
- New notable classes : CCheckBox, CGraphicsPath, CNinePartTiledBitmap, IFocusDrawing
- Direct2D drawing on Windows (Windows Vista or Windows 7)

