# Breaking Changes

This document offers a detailed overview of the substantial changes made in each version.
The versions are presented in chronological order, allowing users to easily identify the
most recent updates. Users can navigate through the document, select the relevant version,
and begin applying the necessary modifications to their code.

### Version 4.xx

When compiling with the **VSTGUI_ENABLE_DEPRECATED_METHODS=0** flag, the SharedPointer
constructor is marked explicit now. Consequently, your code must be adapted in situations
where a shared pointer is assigned from a naked pointer.
Most likely in the following scenarios:

- All constructors having CBitmaps as arguments now take them as a SharedPointer.
- Changed ```IDraggingSession``` and related callback methods to use const references instead of pointers
- The following methods now take a ```SharedPointer<CBitmap>``` instead of a naked pointer
and their possible getter methods return also a ```SharedPointer<CBitmap>``` now:
	* CView::setBackground
	* CView::setDisabledBackground
	* CVUMeter::setOnBitmap
	* CVUMeter::setOffBitmap
	* CTextButton::setIcon
	* CTextButton::setIconHighlighted
	* CKnob::setHandleBitmap
	* CSlider::setHandle
	* CAnimationSplashScreen::setSplashBitmap
	* COptionMenu::setIcon
	* CParamDisplay::drawBack
	* CTabView::addTab
	* CDrawMethods::drawIconAndText
	* CDrawContext::fillRectWithBitmap
	* CDrawContext::drawBitmapNinePartTiled
	* CDrawContext::drawBitmap
	* CBitmapPixelAccess::create
	* IUIDescription::lookupBitmapName
	* UIViewCreator::bitmapToString
	* UIViewCreator::stringToBitmap

- The following methods return a SharedPointer<...> now instead of a naked pointer
	* CDrawContext::createGraphicsPath
	* CDrawContext::createTextPath
	* CDrawContext::createRoundRectGraphicsPath
	* CGradient::create
	* CGraphicsPath::createGradient
	* COffscreenContext::getBitmap
	* CBitmapPixelAccess::create
	* CTextButton::getGradient
	* CTextButton::getGradientHighlighted
	* CSegmentButton::getGradient
	* CSegmentButton::getGradientHighlighted
	* CGradientView::getGradient
	* IUIDescription::getBitmap
	* IUIDescription::getGradient
	* IUIDescription::getFont
	* BitmapFilter::FilterBase::getInputBitmap
	* Standalone::ISharedUIResources::getBitmap
	* Standalone::ISharedUIResources::getGradient
	* Standalone::ISharedUIResources::getFont

- The global fonts (kSystemFont, kNormalFont, etc) are now ```SharedPointer<CFontDesc>``` instead of naked pointers.
- The following methods now take ```SharedPointer<CFontDesc>``` instead of a naked pointer:
	* CParamDisplay::setFont
	* CCheckBox::setFont
	* CTextButton::setFont
	* CDrawContext::setFont
	* CFontChooser::CFontChooser
	* CFontChooser::setFont
	* CSegmentButton::setFont
	* StringListControlDrawer::setFont
	* CTabView::setTabFontStyle
	* CDrawContext::createTextPath
	* CDrawMethods::createTruncatedText
	* CDrawMethods::drawIconAndText
	* IFontChooserDelegate::fontChanged
	* GenericStringListDataBrowserSource::setupUI
	* IUIDescription::lookupFontName

- The following methods now take ```SharedPointer<CGradient>``` instead of a naked pointer:
	* CTextButton::setGradient
	* CTextButton::setGradientHighlighted
	* CSegmentButton::setGradient
	* CSegmentButton::setGradientHighlighted
	* CGradientView::setGradient
	* IUIDescription::lookupGradientName
	* UIViewCreator::addGradientToUIDescription

- The following methods now take ```SharedPointer<CGraphicsPath>``` instead of a naked pointer:
	* CDrawContext::drawGraphicsPath
	* CDrawContext::fillLinearGradient
	* CDrawContext::fillRadialGradient

### Version 4.14

- In CParamDisplay::drawPlatformText(..) the string argument changed from IPlatformString to UTF8Text

### Version 4.13

- the context argument of IFontPainter has changed to use the new platform graphics device context

### Version 4.12.2

- The following constructors have lost their offset parameter:
	- CKickButton
	- CAnimKnob
	- CMovieBitmap
	- CMovieButton
	- CSwitchBase
	- CVerticalSwitch
	- CHorizontalSwitch
	- CRockerSwitch

### Version 4.12

- The CMultiFrameBitmap change deprecated the IMultiBitmapControl class. If you use it,
update your uses and use a CMultiFrameBitmap instead.
- If you compile with VSTGUI_ENABLE_DEPRECATED_METHODS=0 you need to update your multi frame bitmaps
to use CMultiFrameBitmap.

### Version 4.11

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

### Version 4.10

- one has to use VSTGUI::init() before using VSTGUI and VSTGUI::exit() after use

### Version 4.9

- removed method CView::onWheel (..) where the axis of the event was not included. You have to use the other onWheel method for your custom classes now.
- new IViewMouseListener interface method IViewMouseListener::viewOnMouseEnabled
- changed ModalViewSession type name to ModalViewSessionID and its type to an integer type
- changed the CFrame::beginModalViewSession return value to be an Optional<ModalViewSessionID> for safer use.

### Version 4.8

- CCommandMenuItem constructor takes a CCommandMenuItem::Desc argument now. You will get compiler errors when not adopting to this change.
- removed Message sending for:
    - kMsgMenuItemValidate, kMsgMenuItemSelected -> use ICommandMenuItemTarget
    - kMessageValueChanged, kMessageBeginEdit, kMessageEndEdit -> use IControlListener
    - kMsgTruncatedTextChanged -> use ITextLabelListener
    - kMsgBeforePopup -> use IOptionMenuListener
- IDependency is deprecated. Please use explicit interfaces for communicating changes.
- removed "using namespace VSTGUI" from vstgui.h

### Version 4.7

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

### Version 4.5

- COffscreenContext::create returns a SharedPointer<COffscreenContext> now, not a naked pointer.

### Version 4.3

- CControlListener was renamed to IControlListener and moved into the VSTGUI namespace and its own header file. A typedef for CControlListener is available but marked as deprecated.
- the CDrawContext::drawString methods don't set the clip to rect by itself anymore. If you call this method in your code, you need to set the clip yourself now.
- the interfaces for IController and IViewCreator have changed and if you have inherited from them you need to change your implementations accordingly.
- the enum DragResult was moved out of CView into VSTGUI namespace
- CGradient can now be created without a CDrawContext object
- CGradientView takes now a CGradient. Setting the gradient colors and start offsets are removed.
- CTextButton takes now CGradient objects instead of colors and start offsets.
- method signature change for: CViewContainer::getViewAt, CViewContainer::getViewsAt, CViewContainer::getContainerAt
- Some methods changed its arguments or return types from a signed type to an unsigned type, check your overrides !

### Version 4.2

- the class CDragContainer is replaced by IDataPackage. The class CDragContainerHelper is a helper class you can use to quickly get your code up and running again.
- the class IDataBrowser is renamed to IDataBrowserDelegate and the drag and drop methods have changed
- CView::getVisibleSize () was renamed to CView::getVisibleViewSize ()

### Version 4.1

- the pBackground member of CView is now private. You must replace all read access with getDrawBackground () or getBackground () and all write access with setBackground ()

### Version 4.0

- the variable types were changed to use C99 style types (int32_t, etc), you must do this for all your derivated VSTGUI classes too
- the buttons parameter has changed from long to CButtonState
- your custom views need to use the new mouse methods
- COptionMenuScheme is not available anymore
- CFileSelector is gone, you have to use CNewFileSelector
- VST extensions previously enabled via ENABLE_VST_EXTENSION_IN_VSTGUI is gone without replacement
- CBitmap was completely changed and does not use a transparency color anymore, you need to use the alpha channel of a bitmap to get the same results
- COffscreenContext is handled completely different. But in most cases you can simply remove all offscreens where you needed them to reduce flicker.
- On Windows graphics are entirely drawn with GDI+ or Direct2D (when available), GDI is not used anymore
- The internal string encoding is now always UTF-8
- The CCoord type is now always a double
- on Mac OS X, embedding a CFrame into a non composited carbon window is not supported anymore
- on Mac OS X, when targeting Mac OS X 10.4 some of the graphics path methods are not implemented.

- Method signature changes which don't lead to compile errors:
	- CView::setViewSize (CRect& rect, bool invalid = true)
	- CView::hitTest (const CPoint& where, CButtonState& buttons = -1)
	- CView::invalidRect (CRect& rect)
	- CViewContainer::drawBackgroundRect (CDrawContext* pContext, CRect& _updateRect)
	- CViewContainer::addView (CView* pView, CRect& mouseableArea, bool mouseEnabled = true)

### Version 3.6

With VSTGUI 3.6 the VSTGUI_ENABLE_DEPRECATED_METHODS macro has changed to be zero per default. You should change your code so that
it compiles without changing the macro. All methods marked this way will be unavailable in the next version.

### Version 3.5

- COptionMenu was refactored and uses the CMenuItem class for menu items. Item flags are not encoded in the item title anymore.
- CParamDisplay::setTxtFace () and CParamDisplay::getTxtFace () is gone. The text face is already in CFontRef.
- You need to use the new CNewFileSelector class instead of CFileSelector if you want to use it on Mac 64 bit.

### Version 3.0

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
