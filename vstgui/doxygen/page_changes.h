// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

/**
@page page_news_and_changes New stuff in VSTGUI 4

@tableofcontents

- @ref version4_introduction @n
- @ref new_stuff @n
- @ref code_changes @n
- @ref hidpi_support @n
- @ref ios_support @n
- @subpage page_previous_new_stuff

@section version4_introduction Introduction

Version 4 of VSTGUI is a new milestone release with a restructured code base with the focus of code conformity and easier future enhancements.
The result is that code written for any earlier version of VSTGUI is not always compatible.
It's recommended to start new projects with version 4 while old projects should stay with version 3.6.

@section new_stuff New Stuff

@subsection version4_11 Version 4.11

- Using DirectComposition on Windows now with support for CLayeredViewContainer
- Removed 32-bit Carbon support
- Reworked event handling, please see @ref code_changes_4_10_to_4_11
- Reworked unit test framework to be able to debug the tests

@subsection version4_10 Version 4.10

- VSTGUI now needs to be initialized and terminated explicitly. See VSTGUI::init() and VSTGUI::exit().
- UIDescription files are now written in JSON format and the old XML format is deprecated
- It's now possible to conditionally remove the XML parser and the expat library from building (set VSTGUI_ENABLE_XML_PARSER to 0)
- This is the last version not depending on c++17 compiler support.

@subsection version4_9 Version 4.9

- new control: VSTGUI::CListControl in play with VSTGUI::CStringList
- custom font support: VSTGUI now supports using fonts embedded in its Bundle/Package at Resources/Fonts. Note that this works on Windows only when building with the Windows 10 SDK and it does also only work on Windows 10. There's no such restriction on macOS or Linux.

@subsection version4_8 Version 4.8

- new VSTGUI::CSegmentButton selection mode \link VSTGUI::CSegmentButton::SelectionMode::kSingleToggle kSingleToggle\endlink and styles \link VSTGUI::CSegmentButton::Style::kHorizontalInverse kHorizontalInverse\endlink and  \link VSTGUI::CSegmentButton::Style::kVerticalInverse kVerticalInverse\endlink.

@subsection version4_7 Version 4.7

- redesigned drag'n drop
- drags with bitmaps are now supported on Windows
- standalone library support for Windows 7
- new ImageStitcher tool
- the GDI+ draw backend was removed, the Direct2D backend is the replacement

@subsection version4_6 Version 4.6

- new Control: VSTGUI::KeyboardView
- cmake cleanup
- fix static object initialization order
- fix build warnings/errors depending on macOS SDK use
- remove warnings

@subsection version4_5 Version 4.5

- cmake build system
- preview of @ref standalone_library @n
- new Controls: VSTGUI::CMultiLineTextLabel, VSTGUI::CSearchTextEdit
- adopt many c++11 language features
 
@subsection version4_4 Version 4.4

- preview Linux version
- support for Windows XP, Mac OS X 10.6 and non c++11 mode will be removed with version 4.5

@subsection version4_3 Version 4.3

- last version to support Windows XP, Mac OS X 10.6 and non c++11 mode
- HiDPI support (aka Retina support) for Quartz2D and Direct2D backends
- support for creating a graphics path from a string
- new Control : VSTGUI::CSegmentButton
- add support for adding a custom view to the split view separator
- transformation matrix support in VSTGUI::CDrawContext
- alternative c++11 callback functions for VSTGUI::CFileSelector::run(), VSTGUI::CVSTGUITimer, VSTGUI::CParamDisplay::setValueToStringFunction, VSTGUI::CTextEdit::setStringToValueFunction and VSTGUI::CCommandMenuItem::setActions

Note: All current deprecated methods will be removed in the next version. So make sure that your code compiles with VSTGUI_ENABLE_DEPRECATED_METHODS=0

@subsection version4_2 Version 4.2

- iOS Support with Multi Touch handling. See @ref ios_support
- support drawing an icon on a VSTGUI::CTextButton
- VSTGUI::CGradientView
- VSTGUI::CDataBrowser now supports multi row selections
- support compiling in c++11 mode with clang and visual studio
- VSTGUI_OVERRIDE_VMETHOD is now used throughout the vstgui sources to indicate methods which are expecting to override a virtual method of its base classes. (c++11 only)

@subsection version4_1 Version 4.1

- @ref page_uidescription_editor @n
- VSTGUI::COpenGLView (only Windows & Mac Cocoa)
- VSTGUI::CRowColumnView
- VSTGUI::CShadowViewContainer
- VSTGUI::BitmapFilter
- More @link VSTGUI::CScrollView::CScrollViewStyle VSTGUI::CScrollView styles @endlink

@subsection version4_0 Version 4.0

- VST3 Support : Complete inline VST3 Editor support. See @ref page_uidescription_editor @n
- UIDescription : Building user interfaces via XML description files. See @ref uidescription @n
- Animation Support : Simple to use animations. See @ref page_animation
- Amalgamation : Easy integration in your projects via one or two source files
- Cleaned Code : Removed all deprecated methods and classes, splittet individual classes into different files
- Platform Abstraction : Platform dependent code was refactored and moved into its own files
- New notable classes : VSTGUI::CCheckBox, VSTGUI::CGraphicsPath, VSTGUI::CNinePartTiledBitmap, VSTGUI::IFocusDrawing
- Direct2D drawing on Windows (Windows Vista or Windows 7)

@section code_changes Changes for existing VSTGUI code

@subsection code_changes_4_10_to_4_11 VSTGUI 4.10 -> VSTGUI 4.11

Changes due to event handling rework:
- IKeyboardHook changed its methods. If you inherit from it, you need to adopt to the new methods or use OldKeyboardHookAdapter
- IMouseObserver changed a few of its methods. If you inherit from it, you need to adopt to the new methods or use OldMouseObserverAdapter
- CViewContainer::onWheel is now marked final, you cannot inherit this method, please override the new CView::onMouseWheelEvent instead if you need to handle mouse wheel events in a custom view container
- DragEventData has changed it's modifiers type from CButtonState to Modifiers
- CView::hitTest uses an Event now instead of a CButtonState (the method with a CButtonState still works but is deprecated)
- CControl::checkDefaultValue(CButtonState) was removed and replaced by a generic method which uses
the static function CControl::CheckDefaultValueEventFunc to reset a control to its default value

CView has the following new methods:
- dispatchEvent
- onMouseDownEvent
- onMouseMoveEvent
- onMouseUpEvent
- onMouseCancelEvent
- onMouseEnterEvent
- onMouseExitEvent
- onMouseWheelEvent
- onZoomGestureEvent
- onKeyboardEvent

Which replaces the following old methods:
- onKeyDown
- onKeyUp
- onWheel

The old mouse methods (onMouseDown, onMouseUp, onMouseMoved, etc) are still supported but should be replaced with the new methods in the long run.

@subsection code_changes_4_9_to_4_10 VSTGUI 4.9 -> VSTGUI 4.10

- one has to use VSTGUI::init() before using VSTGUI and VSTGUI::exit() after use

@subsection code_changes_4_8_to_4_9 VSTGUI 4.8 -> VSTGUI 4.9

- removed method CView::onWheel (..) where the axis of the event was not included. You have to use the other onWheel method for your custom classes now.
- new IViewMouseListener interface method IViewMouseListener::viewOnMouseEnabled
- changed ModalViewSession type name to ModalViewSessionID and its type to an integer type
- changed the CFrame::beginModalViewSession return value to be an Optional<ModalViewSessionID> for safer use.

@subsection code_changes_4_7_to_4_8 VSTGUI 4.7 -> VSTGUI 4.8

- CCommandMenuItem constructor takes a CCommandMenuItem::Desc argument now. You will get compiler errors when not adopting to this change.
- removed Message sending for:
    - kMsgMenuItemValidate, kMsgMenuItemSelected -> use ICommandMenuItemTarget
    - kMessageValueChanged, kMessageBeginEdit, kMessageEndEdit -> use IControlListener
    - kMsgTruncatedTextChanged -> use ITextLabelListener
    - kMsgBeforePopup -> use IOptionMenuListener
- IDependency is deprecated. Please use explicit interfaces for communicating changes.
- removed "using namespace VSTGUI" from vstgui.h

@subsection code_changes_4_6_to_4_7 VSTGUI 4.6 -> VSTGUI 4.7

- CView::doDrag is deprecated, instead use the asynchronous variant of it : CView::doDrag ;-)
- CView don't has drop target methods (onDragEnter, onDragLeave, onDragMove and onDrop) anymore. Instead it has a method to return a drop target. See the documentation for IDropTarget on how to use it.
- the CControlEnum is gone and is moved into the classes where they are used: CParamDisplay/COptionMenu/CTextEdit/CSlider
- CControl::kMessageTagWillChange and CControl::kMessageTagDidChange is gone, use IControlListener instead
- COptionMenu::popup has changed behaviour and got a callback function that will be called when the popup is closed. The return of COptionMenu::popup now only indicates if the popup was shown.
- IDataBrowserDelegate is now a real virtual interface class, use DataBrowserDelegateAdapter instead if you get compile/linker errors.
- renamed the following interface adapter classes :
	- IViewListenerAdapter -> ViewListenerAdapter
	- IViewContainerListenerAdapter -> ViewContainerListenerAdapter
	- IViewMouseListenerAdapter -> ViewMouseListenerAdapter
	- IGenericStringListDataBrowserSourceSelectionChanged -> GenericStringListDataBrowserSourceSelectionChanged

@subsection code_changes_4_3_to_4_5 VSTGUI 4.3 -> VSTGUI 4.5

- COffscreenContext::create returns a SharedPointer<COffscreenContext> now, not a naked pointer.

@subsection code_changes_4_2_to_4_3 VSTGUI 4.2 -> VSTGUI 4.3

- CControlListener was renamed to IControlListener and moved into the VSTGUI namespace and its own header file. A typedef for CControlListener is available but marked as deprecated.
- the VSTGUI::CDrawContext::drawString methods don't set the clip to rect by itself anymore. If you call this method in your code, you need to set the clip yourself now.
- the interfaces for VSTGUI::IController and VSTGUI::IViewCreator have changed and if you have inherited from them you need to change your implementations accordingly.
- the enum DragResult was moved out of CView into VSTGUI namespace
- VSTGUI::CGradient can now be created without a VSTGUI::CDrawContext object
- VSTGUI::CGradientView takes now a VSTGUI::CGradient. Setting the gradient colors and start offsets are removed.
- VSTGUI::CTextButton takes now VSTGUI::CGradient objects instead of colors and start offsets.
- method signature change for: VSTGUI::CViewContainer::getViewAt, VSTGUI::CViewContainer::getViewsAt, VSTGUI::CViewContainer::getContainerAt
- Some methods changed its arguments or return types from a signed type to an unsigned type, check your overrides !

@subsection code_changes_4_1_to_4_2 VSTGUI 4.1 -> VSTGUI 4.2

- the class CDragContainer is replaced by IDataPackage. The class CDragContainerHelper is a helper class you can use to quickly get your code up and running again.
- the class IDataBrowser is renamed to IDataBrowserDelegate and the drag and drop methods have changed
- CView::getVisibleSize () was renamed to CView::getVisibleViewSize ()

@subsection code_changes_4_0_to_4_1 VSTGUI 4.0 -> VSTGUI 4.1

- the pBackground member of CView is now private. You must replace all read access with getDrawBackground () or getBackground () and all write access with setBackground ()

@subsection code_changes_3_6_to_4_0 VSTGUI 3.6 -> VSTGUI 4.0

- the variable types were changed to use C99 style types (int32_t, etc), you must do this for all your derivated VSTGUI classes too
- the buttons parameter has changed from long to CButtonState
- your custom views need to use the new mouse methods
- COptionMenuScheme is not available anymore
- VSTGUI::CFileSelector is gone, you have to use VSTGUI::CNewFileSelector
- VST extensions previously enabled via ENABLE_VST_EXTENSION_IN_VSTGUI is gone without replacement
- VSTGUI::CBitmap was completely changed and does not use a transparency color anymore, you need to use the alpha channel of a bitmap to get the same results
- VSTGUI::COffscreenContext is handled completely different. But in most cases you can simply remove all offscreens where you needed them to reduce flicker.
- On Windows graphics are entirely drawn with GDI+ or Direct2D (when available), GDI is not used anymore
- The internal string encoding is now always UTF-8
- The VSTGUI::CCoord type is now always a double
- on Mac OS X, embedding a CFrame into a non composited carbon window is not supported anymore
- on Mac OS X, when targeting Mac OS X 10.4 some of the graphics path methods are not implemented.

- Method signature changes which don't lead to compile errors:
	- CView::setViewSize (CRect& rect, bool invalid = true)
	- CView::hitTest (const CPoint& where, CButtonState& buttons = -1)
	- CView::invalidRect (CRect& rect)
	- CViewContainer::drawBackgroundRect (CDrawContext* pContext, CRect& _updateRect)
	- CViewContainer::addView (CView* pView, CRect& mouseableArea, bool mouseEnabled = true)

@section hidpi_support HiDPI notes
- HiDPI is supported on OSX, iOS and Windows (with Direct2D backend)
- Due to platform differences one need to call frame->setZoom (scaleFactor) on Windows, while on OSX and iOS this is not needed.
 
@section ios_support iOS support notes

- VSTGUI supports iOS 7 and later
- Currently COptionMenu, CScrollView and COpenGLView are not supported
- Support for a single MultiTouch View is not yet tested and the API may change in the future

@page page_previous_new_stuff New Stuff in VSTGUI 3.6 and earlier
 
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

*/

//------------------------------------------------------------------------
// Doxygen Group Definitions
//------------------------------------------------------------------------
/*! @defgroup new_in New classes
 */
//------------------------------------------------------------------------
/*! @defgroup new_in_4_0 Version 4.0
 *	@ingroup new_in
 */
//------------------------------------------------------------------------
/*! @defgroup new_in_4_1 Version 4.1
 *	@ingroup new_in
 */
//------------------------------------------------------------------------
/*! @defgroup new_in_4_2 Version 4.2
 *	@ingroup new_in
 */
//------------------------------------------------------------------------
/*! @defgroup new_in_4_3 Version 4.3
 *	@ingroup new_in
 */
//------------------------------------------------------------------------
/*! @defgroup new_in_4_5 Version 4.5
 *	@ingroup new_in
 */
//------------------------------------------------------------------------
/*! @defgroup new_in_4_7 Version 4.7
 *	@ingroup new_in
 */
//------------------------------------------------------------------------
/*! @defgroup new_in_4_9 Version 4.9
 *	@ingroup new_in
 */
//------------------------------------------------------------------------
/*! @defgroup new_in_4_10 Version 4.10
 *	@ingroup new_in
 */
//------------------------------------------------------------------------
/*! @defgroup views Views
 *	@ingroup viewsandcontrols
 */
//------------------------------------------------------------------------
/*! @defgroup controls Controls
 *	@ingroup views
 *	@brief Controls are views the user can interact with
 */
//------------------------------------------------------------------------
/*! @defgroup containerviews Container Views
 *	@ingroup views
 */
//------------------------------------------------------------------------
