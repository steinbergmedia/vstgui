/**
@page changes_from_3_0 Changes from earlier versions of VSTGUI

- @ref new_stuff @n
- @ref code_changes_for_3_0 @n
- @ref code_changes_for_2_3 @n

@section new_stuff New Stuff

@subsection mouse New mouse handling
In earlier versions there were only one method in CView for handling mouse events (VSTGUI::CView::mouse). 
In this version there are five new methods :
- VSTGUI::CView::onMouseDown	(new in 3.5)
- VSTGUI::CView::onMouseUp	(new in 3.5)
- VSTGUI::CView::onMouseMoved	(new in 3.5)
- VSTGUI::CView::onMouseEntered	(new in 3.5)
- VSTGUI::CView::onMouseExited	(new in 3.5)

For convenience the old method is still working, but should be replaced with the ones above.

@subsection new_views New included views

- VSTGUI::CDataBrowser	(new in 3.5)
- VSTGUI::CScrollView	(new in 3.0)
- VSTGUI::CTabView		(new in 3.0)

@subsection and_more_new and ...

- Bitmaps can be loaded either by number or by name (see VSTGUI::CBitmap)	(new in 3.5)
- VSTGUI::CTooltipSupport	(new in 3.5)
- VSTGUI::CVSTGUITimer	(new in 3.5)
- System event driven drawing	(new in 3.5)
- Unicode support via UTF-8 (new in 3.5)
- New font implementation	(new in 3.5)
- Windows GDI+ support	(new in 3.5)
- Mac OS X Composited Window support (new in 3.0)

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
