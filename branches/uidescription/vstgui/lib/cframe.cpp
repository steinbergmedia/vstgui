//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2008, Steinberg Media Technologies, All Rights Reserved
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

#include "cframe.h"
#include "coffscreencontext.h"

#if ENABLE_VST_EXTENSION_IN_VSTGUI
	#include "audioeffectx.h"
	#include "aeffguieditor.h"
#endif // ENABLE_VST_EXTENSION_IN_VSTGUI

#if MAC_COCOA
	#include "cocoasupport.h"
	#include <Carbon/Carbon.h>
#endif // MAC_COCOA

#if WINDOWS
	#include "vstcontrols.h"
	#include "win32support.h"
	static bool bSwapped_mouse_buttons = false; 
	OSVERSIONINFOEX	gSystemVersion;

	// Alpha blending for Windows using library : msimg32.dll
	#define DYNAMICALPHABLEND   !GDIPLUS

	#define WIN32_LEAN_AND_MEAN 1
	#include <windows.h>
	#include <shlobj.h>
	#include <shellapi.h>
	#include <zmouse.h>
	#include <commdlg.h>

	#if DYNAMICALPHABLEND
	typedef  BOOL (WINAPI* PFNALPHABLEND)(
	  HDC hdcDest,                 // handle to destination DC
	  int nXOriginDest,            // x-coord of upper-left corner
	  int nYOriginDest,            // y-coord of upper-left corner
	  int nWidthDest,              // destination width
	  int nHeightDest,             // destination height
	  HDC hdcSrc,                  // handle to source DC
	  int nXOriginSrc,             // x-coord of upper-left corner
	  int nYOriginSrc,             // y-coord of upper-left corner
	  int nWidthSrc,               // source width
	  int nHeightSrc,              // source height
	  BLENDFUNCTION blendFunction  // alpha-blending function
	);

	PFNALPHABLEND pfnAlphaBlend = NULL;

	typedef	BOOL (WINAPI* PFNTRANSPARENTBLT)(
	  HDC hdcDest,        // handle to destination DC
	  int nXOriginDest,   // x-coord of destination upper-left corner
	  int nYOriginDest,   // y-coord of destination upper-left corner
	  int nWidthDest,     // width of destination rectangle
	  int hHeightDest,    // height of destination rectangle
	  HDC hdcSrc,         // handle to source DC
	  int nXOriginSrc,    // x-coord of source upper-left corner
	  int nYOriginSrc,    // y-coord of source upper-left corner
	  int nWidthSrc,      // width of source rectangle
	  int nHeightSrc,     // height of source rectangle
	  UINT crTransparent  // color to make transparent
	);

	PFNTRANSPARENTBLT	pfnTransparentBlt = NULL;
	#endif // DYNAMICALPHABLEND

	BEGIN_NAMESPACE_VSTGUI
	static long   gUseCount = 0;
	static TCHAR   gClassName[100];
	static bool   InitWindowClass ();
	static void   ExitWindowClass ();
	LONG_PTR WINAPI WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	static HANDLE CreateMaskBitmap (CDrawContext* pContext, CRect& rect, CColor transparentColor);
	static void   DrawTransparent (CDrawContext* pContext, CRect& rect, const CPoint& offset, HDC hdcBitmap, POINT ptSize, HBITMAP pMask, COLORREF color);
	static bool   checkResolveLink (const TCHAR* nativePath, TCHAR* resolved);
	static void   *createDropTarget (CFrame* pFrame);

	END_NAMESPACE_VSTGUI

	#if USE_LIBPNG
		#include "png.h"
	#endif	// USE_LIBPNG

#endif // WINDOWS

BEGIN_NAMESPACE_VSTGUI

#if MAC
	static bool createNSViewMode = false;
	long pSystemVersion;

	#ifndef VSTGUI_NEW_BUNDLE_REF_DEFINITION	// You can define this in your preprocessor definitions and supply the following function somewhere in your code
		CFBundleRef getBundleRef () { return (CFBundleRef)gBundleRef; }
	#endif

	#if MAC_CARBON
		bool isWindowComposited (WindowRef window);
		void CRect2Rect (const CRect &cr, Rect &rr);
		void Rect2CRect (Rect &rr, CRect &cr);

		#if MAC_OLD_DRAG
			static void install_drop (CFrame* frame);
			static void remove_drop (CFrame* frame);
		#endif // MAC_OLD_DRAG

		//-----------------------------------------------------------------------------
		void CRect2Rect (const CRect &_cr, Rect &rr)
		{
			CRect cr (_cr);
			cr.normalize ();
			rr.left   = (short)cr.left;
			rr.right  = (short)cr.right;
			rr.top    = (short)cr.top;
			rr.bottom = (short)cr.bottom;
		}

		//-----------------------------------------------------------------------------
		void Rect2CRect (Rect &rr, CRect &cr)
		{
			cr.left   = rr.left;
			cr.right  = rr.right;
			cr.top    = rr.top;
			cr.bottom = rr.bottom;
		}

		#define MAC_OLD_DRAG	__ppc__

		#ifndef EMBED_HIVIEW	// automaticly add the CFrame HIView to the content view of the window
			#define EMBED_HIVIEW	!AU
		#endif // EMBED_HIVIEW

	#endif // MAC_CARBON
#endif // MAC

const char* kMsgNewFocusView = "kMsgNewFocusView";
const char* kMsgOldFocusView = "kMsgOldFocusView";

#define DEBUG_DRAWING			0	// set to 1 if you want to debug drawing on Windows

//-----------------------------------------------------------------------------
// CFrame Implementation
//-----------------------------------------------------------------------------
/*! @class CFrame
It creates a platform dependend view object. 

On Mac OS X it is a HIView.\n 
On Windows it's a WS_CHILD Window.

*/
//-----------------------------------------------------------------------------
/**
 * @param inSize size of frame
 * @param inSystemWindow parent platform window
 * @param inEditor editor
 */
CFrame::CFrame (const CRect &inSize, void* inSystemWindow, VSTGUIEditorInterface* inEditor)
: CViewContainer (inSize, 0, 0)
, pEditor (inEditor)
, pMouseObserver (0)
, pKeyboardHook (0)
, pSystemWindow (inSystemWindow)
, pModalView (0)
, pFocusView (0)
, pMouseOverView (0)
, bFirstDraw (true)
, bDropActive (false)
, pFrameContext (0)
#if ENABLE_VST_EXTENSION_IN_VSTGUI
, bAddedWindow (false)
, pVstWindow (0)
#endif // ENABLE_VST_EXTENSION_IN_VSTGUI
, defaultCursor (0)
{
	setOpenFlag (true);
	bIsAttached = true;
	
	pParentFrame = this;

#if WINDOWS
	pHwnd = 0;
	dropTarget = 0;
	backBuffer = 0;
	OleInitialize (0);
	bMouseInside = false;

	#if GDIPLUS
		GDIPlusGlobals::enter ();
	#endif // GDIPLUS

	// get OS version
	memset (&gSystemVersion, 0, sizeof (gSystemVersion));
	gSystemVersion.dwOSVersionInfoSize = sizeof (gSystemVersion);

	if (GetVersionEx ((OSVERSIONINFO *)&gSystemVersion))
	{
	}

	#if DYNAMICALPHABLEND
		pfnAlphaBlend = 0;
		pfnTransparentBlt = 0;

		hInstMsimg32dll = LoadLibrary ("msimg32.dll");
		if (hInstMsimg32dll)
		{
			pfnAlphaBlend = (PFNALPHABLEND)GetProcAddress (hInstMsimg32dll, "AlphaBlend");

			// Is this win NT or better?
			if (gSystemVersion.dwPlatformId >= VER_PLATFORM_WIN32_NT)
			{
				// Yes, then TransparentBlt doesn't have the memory-leak and can be safely used
				pfnTransparentBlt = (PFNTRANSPARENTBLT)GetProcAddress (hInstMsimg32dll, "TransparentBlt");
			}
		}
	#endif	// DYNAMICALPHABLEND
    
#endif // WINDOWS

#if MAC_COCOA
	nsView = 0;

	#if !MAC_CARBON
		createNSViewMode = true;
	#endif // !MAC_CARBON

#endif // MAC_COCOA

#if MAC_CARBON
	Gestalt (gestaltSystemVersion, &pSystemVersion);
	pFrameContext = 0;
	controlRef = 0;
	dragEventHandler = 0;
	mouseEventHandler = 0;
#endif // MAC_CARBON

	initFrame (inSystemWindow);

}

#if ENABLE_VST_EXTENSION_IN_VSTGUI
//-----------------------------------------------------------------------------
/**
 * creates a custom window if VST host supports it (only possible if ENABLE_VST_EXTENSION_IN_VSTGUI)
 * \note this is deprecated with VST 2.4
 * @param inSize size of frame
 * @param inTitle window title
 * @param inEditor editor
 * @param inStyle window style
 */
CFrame::CFrame (const CRect& inSize, const char* inTitle, VSTGUIEditorInterface* inEditor, const long inStyle)
: CViewContainer (inSize, 0, 0)
, pEditor (inEditor)
, pMouseObserver (0)
, pKeyboardHook (0)
, pSystemWindow (0)
, pModalView (0)
, pFocusView (0)
, pMouseOverView (0)
, bFirstDraw (true)
, bDropActive (false)
, pFrameContext (0)
, pVstWindow (0) 
, defaultCursor (0)
{
	bAddedWindow  = true;
	setOpenFlag (false);
	pParentFrame = this;
	bIsAttached = true;

#if WINDOWS
	pHwnd = 0;
	dropTarget = 0;
	backBuffer = 0;
	OleInitialize (0);
	bMouseInside = false;

	#if DYNAMICALPHABLEND
	pfnAlphaBlend = 0;
	pfnTransparentBlt = 0;

	hInstMsimg32dll = LoadLibrary ("msimg32.dll");
	if (hInstMsimg32dll)
	{
		pfnAlphaBlend = (PFNALPHABLEND)GetProcAddress (hInstMsimg32dll, "AlphaBlend");

		// get OS version
		OSVERSIONINFOEX	osvi;

		memset (&osvi, 0, sizeof (osvi));
		osvi.dwOSVersionInfoSize = sizeof (osvi);

		if (GetVersionEx ((OSVERSIONINFO *)&osvi))
		{
			// Is this win NT or better?
			if (osvi.dwPlatformId >= VER_PLATFORM_WIN32_NT)
			{
				// Yes, then TransparentBlt doesn't have the memory-leak and can be safely used
				pfnTransparentBlt = (PFNTRANSPARENTBLT)GetProcAddress (hInstMsimg32dll, "TransparentBlt");
			}
		}
	}
	#endif

#elif MAC_COCOA && !MAC_CARBON
	createNSViewMode = true;
	
#endif

	#if !VST_FORCE_DEPRECATED
	pVstWindow = (VstWindow*)malloc (sizeof (VstWindow));
	strcpy (((VstWindow*)pVstWindow)->title, inTitle);
	((VstWindow*)pVstWindow)->xPos   = (short)size.left;
	((VstWindow*)pVstWindow)->yPos   = (short)size.top;
	((VstWindow*)pVstWindow)->width  = (short)size.width ();
	((VstWindow*)pVstWindow)->height = (short)size.height ();
	((VstWindow*)pVstWindow)->style  = inStyle;
	((VstWindow*)pVstWindow)->parent     = 0;
	((VstWindow*)pVstWindow)->userHandle = 0;
	((VstWindow*)pVstWindow)->winHandle  = 0;
	#endif
}
#endif

//-----------------------------------------------------------------------------
CFrame::~CFrame ()
{
	if (pModalView)
		removeView (pModalView, false);

	setCursor (kCursorDefault);

	setDropActive (false);

	if (pFrameContext)
		pFrameContext->forget ();

	pParentFrame = 0;
	removeAll ();

#if WINDOWS
	OleUninitialize ();

	if (backBuffer)
		backBuffer->forget ();

	#if DYNAMICALPHABLEND
	if (hInstMsimg32dll)
		FreeLibrary (hInstMsimg32dll);
	#endif

	if (pHwnd)
	{
		SetWindowLongPtr ((HWND)pHwnd, GWLP_USERDATA, (LONG_PTR)NULL);
		DestroyWindow ((HWND)pHwnd);

		ExitWindowClass ();
	}
	#if GDIPLUS
	GDIPlusGlobals::exit ();
	#endif
#endif

	#if ENABLE_VST_EXTENSION_IN_VSTGUI
	if (bAddedWindow)
		close ();
	if (pVstWindow)
		free (pVstWindow);
	#endif
	

#if MAC_COCOA
	if (nsView)
		destroyNSView (nsView);
#endif

#if MAC_CARBON
	if (mouseEventHandler)
		RemoveEventHandler (mouseEventHandler);
	if (controlRef)
	{
		if (HIViewRemoveFromSuperview (controlRef) == noErr && isWindowComposited ((WindowRef)pSystemWindow))
			CFRelease (controlRef);
	}
#endif
}

#if MAC_COCOA && MAC_CARBON
void CFrame::setCocoaMode (bool state)
{
	createNSViewMode = state;
}

#endif


#if ENABLE_VST_EXTENSION_IN_VSTGUI
//-----------------------------------------------------------------------------
/**
 * open custom window
 * \note deprecated with VST 2.4
 * @param point location of left top position where to open the window
 * @return true on success
 */
bool CFrame::open (CPoint* point)
{
#if (!VST_FORCE_DEPRECATED)
	if (!bAddedWindow)
		return false;
	if (getOpenFlag ())
	{
		#if WINDOWS
		BringWindowToTop (GetParent (GetParent ((HWND)getSystemWindow ())));

		#endif
		return false;
	}

	if (pVstWindow)
	{
		if (point)
		{
			((VstWindow*)pVstWindow)->xPos = (short)point->h;
			((VstWindow*)pVstWindow)->yPos = (short)point->v;
		}
		AudioEffectX* pAudioEffectX = (AudioEffectX*)(((AEffGUIEditor*)pEditor)->getEffect ());
		pSystemWindow = pAudioEffectX->openWindow ((VstWindow*)pVstWindow);
	}

	if (pSystemWindow)
	{
		if (initFrame (pSystemWindow))
			setOpenFlag (true);
	}

	return getOpenFlag ();
#else
	return false;
#endif
}

//-----------------------------------------------------------------------------
/**
 * close custom window
 * \note deprecated with VST 2.4
 * @return true on success
 */
bool CFrame::close ()
{
#if (!VST_FORCE_DEPRECATED)
	if (!bAddedWindow || !getOpenFlag () || !pSystemWindow)
		return false;

	AudioEffectX* pAudioEffectX = (AudioEffectX*)(((AEffGUIEditor*)pEditor)->getEffect ());
	pAudioEffectX->closeWindow ((VstWindow*)pVstWindow);

	pSystemWindow = 0;

	return true;
#else
	return false;
#endif
}
#endif // ENABLE_VST_EXTENSION_IN_VSTGUI

//-----------------------------------------------------------------------------
bool CFrame::initFrame (void* systemWin)
{
	if (!systemWin)
		return false;
	
#if WINDOWS

	InitWindowClass ();
	DWORD style = 0;
	#if !DEBUG_DRAWING
	if (gSystemVersion.dwMajorVersion >= 6) // Vista and above
		style |= WS_EX_COMPOSITED;
	#endif
	pHwnd = CreateWindowEx (style, gClassName, TEXT("Window"),
			 WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 
			 0, 0, (int)size.width (), (int)size.height (), 
			 (HWND)pSystemWindow, NULL, GetInstance (), NULL);

	SetWindowLongPtr ((HWND)pHwnd, GWLP_USERDATA, (__int3264)(LONG_PTR)this);

#endif

#if MAC_COCOA
	#if MAC_CARBON
	if (createNSViewMode)
	{
		nsView = createNSView (this, size);
		size.offset (-size.left, -size.top);
		return true;
	}
	#else
	nsView = createNSView (this, size);
	size.offset (-size.left, -size.top);
	#endif

#endif

#if MAC_CARBON

	dragEventHandler = 0;

	hasFocus = false;
	Rect r = {(short)size.top, (short)size.left, (short)size.bottom, (short)size.right};
	UInt32 features =	kControlSupportsDragAndDrop
						| kControlSupportsFocus
						| kControlHandlesTracking
						| kControlSupportsEmbedding
						| kHIViewIsOpaque
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4
						| kHIViewFeatureDoesNotUseSpecialParts;
#else
						| kHIViewDoesNotUseSpecialParts;
#endif
	OSStatus status = CreateUserPaneControl (0, &r, features, &controlRef);
	if (status != noErr)
	{
		fprintf (stderr, "Could not create Control : %d\n", (int)status);
		return false;
	}
	const EventTypeSpec controlEventTypes[] = {	
		{kEventClassControl, kEventControlDraw},
		{kEventClassControl, kEventControlHitTest},
		{kEventClassControl, kEventControlClick},
		//{kEventClassControl, kEventControlTrack},
		//{kEventClassControl, kEventControlContextualMenuClick},
		{kEventClassKeyboard, kEventRawKeyDown},
		{kEventClassKeyboard, kEventRawKeyRepeat},
		{kEventClassMouse, kEventMouseWheelMoved},
		{kEventClassControl, kEventControlDragEnter},
		{kEventClassControl, kEventControlDragWithin},
		{kEventClassControl, kEventControlDragLeave},
		{kEventClassControl, kEventControlDragReceive},
		{kEventClassControl, kEventControlInitialize},
		{kEventClassControl, kEventControlGetClickActivation},
		{kEventClassControl, kEventControlGetOptimalBounds},
		{kEventClassScrollable, kEventScrollableGetInfo},
		{kEventClassScrollable, kEventScrollableScrollTo},
		{kEventClassControl, kEventControlSetFocusPart},
		{kEventClassControl, kEventControlGetFocusPart},
		#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4
		{kEventClassControl, kEventControlTrackingAreaExited},
		#endif
	};
	InstallControlEventHandler (controlRef, carbonEventHandler, GetEventTypeCount (controlEventTypes), controlEventTypes, this, NULL);
	
	const EventTypeSpec keyWorkaroundEvents[] = {
		{ kEventClassTextInput, kEventTextInputUnicodeForKeyEvent }
	};
	InstallWindowEventHandler ((WindowRef)systemWin, carbonEventHandler, GetEventTypeCount (keyWorkaroundEvents), keyWorkaroundEvents, this, NULL);
	const EventTypeSpec mouseEvents[] = {
		{ kEventClassMouse, kEventMouseDown },
		{ kEventClassMouse, kEventMouseUp },
		{ kEventClassMouse, kEventMouseMoved },
		{ kEventClassMouse, kEventMouseDragged },
	};
	InstallWindowEventHandler ((WindowRef)systemWin, carbonMouseEventHandler, GetEventTypeCount (mouseEvents), mouseEvents, this, &mouseEventHandler);
	
	SetControlDragTrackingEnabled (controlRef, true);
	SetAutomaticControlDragTrackingEnabledForWindow ((WindowRef)systemWin, true);
	#if EMBED_HIVIEW
	if (isWindowComposited ((WindowRef)systemWin)) 
	{
		HIViewRef contentView;
		HIViewRef rootView = HIViewGetRoot ((WindowRef)systemWin);
		if (HIViewFindByID (rootView, kHIViewWindowContentID, &contentView) != noErr)
			contentView = rootView;
		HIViewAddSubview (contentView, controlRef);
	}
	else
	{
		ControlRef rootControl;
		GetRootControl ((WindowRef)systemWin, &rootControl);
		if (rootControl == NULL)
			CreateRootControl ((WindowRef)systemWin, &rootControl);
		EmbedControl(controlRef, rootControl);	
	}
	#endif
	
	#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4
	HIViewTrackingAreaRef trackingAreaRef;	// will automatically removed if view is destroyed
	HIViewNewTrackingArea (controlRef, 0, 0, &trackingAreaRef);
	#endif
	
	size.offset (-size.left, -size.top);
	mouseableArea.offset (-size.left, -size.top);
	
#endif

	setDropActive (true);

	return true;
}

//-----------------------------------------------------------------------------
void* CFrame::getSystemWindow () const
{
	#if WINDOWS
	return pHwnd;

	#else
	return pSystemWindow;

	#endif
}

//-----------------------------------------------------------------------------
bool CFrame::setDropActive (bool val)
{	
	if (!bDropActive && !val)
		return true;

#if WINDOWS
	if (!pHwnd)
		return false;
	if (dropTarget)
	{
		RevokeDragDrop ((HWND)pHwnd);
		dropTarget = 0;
	}
	if (val)
	{
		dropTarget = createDropTarget (this);
		RegisterDragDrop ((HWND)pHwnd, (IDropTarget*)dropTarget);
	}

#elif MAC_CARBON && MAC_OLD_DRAG
	if (controlRef && !isWindowComposited ((WindowRef)pSystemWindow))
	{
		if (val)
			install_drop (this);
		else
			remove_drop (this);
	}
#endif

	bDropActive = val;
	return true;
}

//-----------------------------------------------------------------------------
CDrawContext* CFrame::createDrawContext ()
{
	if (pFrameContext)
	{
		pFrameContext->remember ();
		return pFrameContext;
	}

	CDrawContext* pContext = 0;
	#if WINDOWS || MAC
	pContext = new CDrawContext (this, NULL, getSystemWindow ());

	#endif
	
	return pContext;
}

//-----------------------------------------------------------------------------
void CFrame::draw (CDrawContext* pContext)
{
	return CFrame::drawRect (pContext, size);
}

//-----------------------------------------------------------------------------
void CFrame::drawRect (CDrawContext* pContext, const CRect& updateRect)
{
	if (bFirstDraw)
		bFirstDraw = false;

	if (!pContext)
		pContext = pFrameContext;

	if (pContext)
		pContext->remember ();
	else
		pContext = createDrawContext ();

	CRect oldClip;
	pContext->getClipRect (oldClip);
	CRect newClip (updateRect);
	newClip.bound (oldClip);
	pContext->setClipRect (newClip);
	
	// draw the background and the children
	if (updateRect.getWidth () > 0 && updateRect.getHeight () > 0)
	{
		CViewContainer::drawRect (pContext, updateRect);
	}

	pContext->setClipRect (oldClip);

	pContext->forget ();
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::onMouseDown (CPoint &where, const long& buttons)
{
	// reset views
	mouseDownView = 0;
	if (pFocusView)
		setFocusView (NULL);
	if (pMouseOverView)
	{
		pMouseOverView->onMouseExited (where, buttons);
		if (getMouseObserver ())
			getMouseObserver ()->onMouseExited (pMouseOverView, this);
		pMouseOverView = 0;
	}

	if (getMouseObserver ())
		getMouseObserver ()->onMouseDown (this, where);

	if (pModalView)
	{
		if (pModalView->hitTest (where, buttons))
		{
			CMouseEventResult result = pModalView->onMouseDown (where, buttons);
			if (result == kMouseEventHandled)
			{
				mouseDownView = pModalView;
				return kMouseEventHandled;
			}
		}
	}
	else
		return CViewContainer::onMouseDown (where, buttons);
	return kMouseEventNotHandled;
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::onMouseUp (CPoint &where, const long& buttons)
{
	return CViewContainer::onMouseUp (where, buttons);
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::onMouseMoved (CPoint &where, const long& buttons)
{
#if WINDOWS
	if (!bMouseInside)
	{
		bMouseInside = true;
		TRACKMOUSEEVENT tme = {0};
		tme.cbSize = sizeof (tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = (HWND)pHwnd;
		TrackMouseEvent (&tme);
	}
#endif
	if (getMouseObserver ())
		getMouseObserver ()->onMouseMoved (this, where);
	if (pModalView)
		return pModalView->onMouseMoved (where, buttons);
	else
	{
		CMouseEventResult result = CViewContainer::onMouseMoved (where, buttons);
		if (result == kMouseEventNotHandled)
		{
			CView* v = getViewAt (where, true);
			if (v != pMouseOverView)
			{
				if (pMouseOverView)
				{
					CPoint lr (where);
					pMouseOverView->frameToLocal (lr);
					pMouseOverView->onMouseExited (lr, buttons);
					if (getMouseObserver ())
						getMouseObserver ()->onMouseExited (pMouseOverView, this);
				}
				pMouseOverView = 0;
				if (v)
				{
					CPoint lr (where);
					v->frameToLocal (lr);
					v->onMouseEntered (lr, buttons);
					pMouseOverView = v;
					if (getMouseObserver ())
						getMouseObserver ()->onMouseEntered (pMouseOverView, this);
				}
				return kMouseEventHandled;
			}
			else if (pMouseOverView)
			{
				CPoint lr (where);
				pMouseOverView->frameToLocal (lr);
				return pMouseOverView->onMouseMoved (lr, mouseDownView ? buttons : 0);
			}
		}
		return result;
	}
}

//-----------------------------------------------------------------------------
CMouseEventResult CFrame::onMouseExited (CPoint &where, const long& buttons)
{ // this should only get called from the platform implementation
	if (pMouseOverView)
	{
		CPoint lr (where);
		pMouseOverView->frameToLocal (lr);
		pMouseOverView->onMouseExited (lr, buttons);
		if (getMouseObserver ())
			getMouseObserver ()->onMouseExited (pMouseOverView, this);
	}
	pMouseOverView = 0;

#if WINDOWS
	bMouseInside = false;
#endif
	return kMouseEventHandled;
}

//-----------------------------------------------------------------------------
long CFrame::onKeyDown (VstKeyCode& keyCode)
{
	long result = -1;

	if (getKeyboardHook ())
		result = getKeyboardHook ()->onKeyDown (keyCode, this);

	if (result == -1 && pFocusView)
		result = pFocusView->onKeyDown (keyCode);

	if (result == -1 && pModalView)
		result = pModalView->onKeyDown (keyCode);

	if (result == -1 && keyCode.virt == VKEY_TAB)
		result = advanceNextFocusView (pFocusView, (keyCode.modifier & MODIFIER_SHIFT) ? true : false) ? 1 : -1;

	return result;
}

//-----------------------------------------------------------------------------
long CFrame::onKeyUp (VstKeyCode& keyCode)
{
	long result = -1;

	if (getKeyboardHook ())
		result = getKeyboardHook ()->onKeyUp (keyCode, this);

	if (result == -1 && pFocusView)
		result = pFocusView->onKeyUp (keyCode);

	if (result == -1 && pModalView)
		result = pModalView->onKeyUp (keyCode);

	return result;
}

//------------------------------------------------------------------------
bool CFrame::onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const long &buttons)
{
	bool result = false;

	if (mouseDownView == 0)
	{
		CView* view = pModalView ? pModalView : getViewAt (where);
		if (view)
		{
			result = view->onWheel (where, axis, distance, buttons);
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
bool CFrame::onWheel (const CPoint &where, const float &distance, const long &buttons)
{
	return onWheel (where, kMouseWheelAxisY, distance, buttons);
}

//-----------------------------------------------------------------------------
void CFrame::idle ()
{
	if (!getOpenFlag ())
		return;

	invalidateDirtyViews ();
}

//-----------------------------------------------------------------------------
void CFrame::doIdleStuff ()
{
#if MAC
	if (pFrameContext)
		pFrameContext->synchronizeCGContext ();
#endif
	if (pEditor)
		pEditor->doIdleStuff ();
}

//-----------------------------------------------------------------------------
/**
 * @return tick count in milliseconds
 */
unsigned long CFrame::getTicks () const
{
	#if MAC_CARBON || MAC_COCOA
	return (TickCount () * 1000) / 60;
	
	#elif WINDOWS
	return (unsigned long)GetTickCount ();
	
	#endif

	return 0;
}

//-----------------------------------------------------------------------------
long CFrame::getKnobMode () const
{
	if (pEditor)
		return pEditor->getKnobMode ();
	return kCircularMode;
}

//-----------------------------------------------------------------------------
#if WINDOWS
COffscreenContext* CFrame::getBackBuffer ()
{
	#if USE_ALPHA_BLEND && !DEBUG_DRAWING
	if (gSystemVersion.dwMajorVersion < 6) // pre-Vista
	{
		if (!backBuffer)
			backBuffer = new COffscreenContext (this, (long)size.width (), (long)size.height ());
	}
	#endif

	return backBuffer;
}

HWND CFrame::getOuterWindow () const
{
	int diffWidth, diffHeight;
	RECT  rctTempWnd, rctPluginWnd;
	HWND  hTempWnd = (HWND)pHwnd;
	GetWindowRect (hTempWnd, &rctPluginWnd);
    
	while (hTempWnd != NULL)
	{
		// Looking for caption bar
		if (GetWindowLong (hTempWnd, GWL_STYLE) & WS_CAPTION)
			return hTempWnd;

		// Looking for last parent
		if (!GetParent (hTempWnd))
			return hTempWnd;
    
		// get difference between plugin-window and current parent
		GetWindowRect (GetParent (hTempWnd), &rctTempWnd);
	    
		diffWidth  = (rctTempWnd.right - rctTempWnd.left) - (rctPluginWnd.right - rctPluginWnd.left);
		diffHeight = (rctTempWnd.bottom - rctTempWnd.top) - (rctPluginWnd.bottom - rctPluginWnd.top);
		
		// Looking for size mismatch
		if ((abs (diffWidth) > 60) || (abs (diffHeight) > 60)) // parent belongs to host
			return (hTempWnd);

		if (diffWidth < 0)
			diffWidth = 0;
        if (diffHeight < 0)
			diffHeight = 0; 
		
		// get the next parent window
		hTempWnd = GetParent (hTempWnd);
	}

	return NULL;
}
#endif

//-----------------------------------------------------------------------------
/**
 * repositions the frame
 * @param x x coordinate
 * @param y y coordinate
 * @return true on success
 */
bool CFrame::setPosition (CCoord x, CCoord y)
{
	if (!getOpenFlag ())
		return false;
#if MAC_CARBON
	if (controlRef)
	{
		HIRect r;
		if (HIViewGetFrame (controlRef, &r) != noErr)
			return false;
		if (HIViewMoveBy (controlRef, x - r.origin.x, y - r.origin.y) != noErr)
			return false;
		return true;
	}
#elif MAC_COCOA
	// TODO COCOA

#elif WINDOWS
	// TODO: not implemented yet

#else
	// not implemented yet

#endif
	return false;
}

//-----------------------------------------------------------------------------
/**
 * get global position of frame
 * @param x x coordinate
 * @param y y coordinate
 * @return true on success
 */
bool CFrame::getPosition (CCoord &x, CCoord &y) const
{
	if (!getOpenFlag ())
		return false;
	
	// get the position of the Window including this frame in the main pWindow
#if WINDOWS
	HWND wnd = (HWND)getOuterWindow ();
	HWND wndParent = GetParent (wnd);

	RECT  rctTempWnd;
	GetWindowRect (wnd, &rctTempWnd);

	POINT point;
	point.x = rctTempWnd.left;
	point.y = rctTempWnd.top;

	MapWindowPoints (HWND_DESKTOP, wndParent, &point, 1);
	
	x = (CCoord)point.x;
	y = (CCoord)point.y;
#endif

#if MAC_COCOA

	// TODO COCOA
	if (nsView)
	{
		return false;
	}
#endif
	
#if MAC_CARBON
	Rect bounds;
	GetWindowBounds ((WindowRef)pSystemWindow, kWindowContentRgn, &bounds);
	
	x   = bounds.left;
	y   = bounds.top;

	if (isWindowComposited ((WindowRef)pSystemWindow))
	{
		HIPoint hip = { 0.f, 0.f };
		HIViewRef contentView;
		HIViewFindByID (HIViewGetRoot ((WindowRef)pSystemWindow), kHIViewWindowContentID, &contentView);
		if (contentView)
			HIViewConvertPoint (&hip, controlRef, contentView);
		x += (CCoord)hip.x;
		y += (CCoord)hip.y;
	}
	else
	{
		HIRect hirect;
		HIViewGetFrame ((HIViewRef)controlRef, &hirect);
		x += (CCoord)hirect.origin.x;
		y += (CCoord)hirect.origin.y;
	}
	x -= hiScrollOffset.x;
	y -= hiScrollOffset.y;

#endif
	return true;
}

//-----------------------------------------------------------------------------
void CFrame::setViewSize (CRect& rect, bool invalid)
{
	CViewContainer::setViewSize (rect, invalid);
}

//-----------------------------------------------------------------------------
/**
 * set size of frame (and the platform representation)
 * @param width new width
 * @param height new height
 * @return true on success
 */
bool CFrame::setSize (CCoord width, CCoord height)
{
	if (!getOpenFlag ())
		return false;
	
	if ((width == size.width ()) && (height == size.height ()))
		return false;

	CRect newSize (size);
	newSize.setWidth (width);
	newSize.setHeight (height);

#if WINDOWS
	if (backBuffer)
		backBuffer->forget ();
	backBuffer = 0;
#endif
#if ENABLE_VST_EXTENSION_IN_VSTGUI
	if (pEditor)
	{
		AudioEffectX* effect = (AudioEffectX*)((AEffGUIEditor*)pEditor)->getEffect ();
		if (effect && effect->canHostDo ("sizeWindow"))
		{
			if (effect->sizeWindow ((long)width, (long)height))
			{
				#if WINDOWS
				SetWindowPos ((HWND)pHwnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE);
				
				#elif MAC_CARBON
				Rect bounds;
				CRect2Rect (newSize, bounds);
				SetControlBounds (controlRef, &bounds);
				#endif
				
				CViewContainer::setViewSize (newSize);
				return true;
			}
		}
	}
#endif

#if WINDOWS
	RECT  rctTempWnd, rctParentWnd;
	HWND  hTempWnd;
	long   iFrame = (2 * GetSystemMetrics (SM_CYFIXEDFRAME));
	
	long diffWidth  = 0;
	long diffHeight = 0;
	
	hTempWnd = (HWND)pHwnd;
	
	while ((diffWidth != iFrame) && (hTempWnd != NULL)) // look for FrameWindow
	{
		HWND hTempParentWnd = GetParent (hTempWnd);
		TCHAR buffer[1024];
		GetClassName (hTempParentWnd, buffer, 1024);
		if (!hTempParentWnd || !VSTGUI_STRCMP (buffer, TEXT("MDIClient")))
			break;
		GetWindowRect (hTempWnd, &rctTempWnd);
		GetWindowRect (hTempParentWnd, &rctParentWnd);
		
		SetWindowPos (hTempWnd, HWND_TOP, 0, 0, (int)width + diffWidth, (int)height + diffHeight, SWP_NOMOVE);
		
		diffWidth  += (rctParentWnd.right - rctParentWnd.left) - (rctTempWnd.right - rctTempWnd.left);
		diffHeight += (rctParentWnd.bottom - rctParentWnd.top) - (rctTempWnd.bottom - rctTempWnd.top);
		
		if ((diffWidth > 80) || (diffHeight > 80)) // parent belongs to host
			return true;

		if (diffWidth < 0)
			diffWidth = 0;
        if (diffHeight < 0)
			diffHeight = 0;
		
		hTempWnd = hTempParentWnd;
	}
	
	if (hTempWnd)
		SetWindowPos (hTempWnd, HWND_TOP, 0, 0, (int)width + diffWidth, (int)height + diffHeight, SWP_NOMOVE);
#endif

#if MAC_COCOA
	if (nsView)
	{
		resizeNSView (nsView, newSize);
		CViewContainer::setViewSize (newSize);
		return true;
	}
#endif

#if MAC_CARBON
	// keep old values
	CCoord oldWidth  = size.width ();
	CCoord oldHeight = size.height ();

	if (getSystemWindow ())
	{
		if (!isWindowComposited ((WindowRef)getSystemWindow ()))
		{
			Rect bounds;
			GetPortBounds (GetWindowPort ((WindowRef)getSystemWindow ()), &bounds);
			SizeWindow ((WindowRef)getSystemWindow (), (short)((bounds.right - bounds.left) - oldWidth + width),
									(short)((bounds.bottom - bounds.top) - oldHeight + height), true);
		}
	}
	if (controlRef)
	{
		HIRect frameRect;
		HIViewGetFrame (controlRef, &frameRect);
		frameRect.size.width = width;
		frameRect.size.height = height;
		HIViewSetFrame (controlRef, &frameRect);
	}
#endif

	CViewContainer::setViewSize (newSize);

	return true;
}

//-----------------------------------------------------------------------------
/**
 * get size relative to parent
 * @param pRect size
 * @return true on success
 */
bool CFrame::getSize (CRect* pRect) const
{
	if (!getOpenFlag ())
		return false;

#if WINDOWS
	// return the size relative to the client rect of this window
	// get the main window
	HWND wnd = GetParent ((HWND)getSystemWindow ());
	HWND wndParent = GetParent (wnd);
	HWND wndParentParent = GetParent (wndParent);

	RECT  rctTempWnd;
	GetWindowRect (wnd, &rctTempWnd);
	
	POINT point;
	point.x = rctTempWnd.left;
	point.y = rctTempWnd.top;

	MapWindowPoints (HWND_DESKTOP, wndParentParent, &point, 1);
	
	pRect->left   = (CCoord)point.x;
	pRect->top    = (CCoord)point.y;
	pRect->right  = (CCoord)pRect->left + rctTempWnd.right - rctTempWnd.left;
	pRect->bottom = (CCoord)pRect->top  + rctTempWnd.bottom - rctTempWnd.top;
#endif

#if MAC_COCOA
	if (nsView)
	{
		getSizeOfNSView (nsView, pRect);
		return true;
	}
#endif
	
#if MAC_CARBON
	HIRect hiRect;
	if (HIViewGetFrame (controlRef, &hiRect) == noErr)
	{
		pRect->left = (CCoord)hiRect.origin.x;
		pRect->top = (CCoord)hiRect.origin.y;
		pRect->setWidth ((CCoord)hiRect.size.width);
		pRect->setHeight ((CCoord)hiRect.size.height);
		return true;
	}
	Rect bounds;
	GetPortBounds (GetWindowPort ((WindowRef)getSystemWindow ()), &bounds);

	pRect->left   = bounds.left;
	pRect->top    = bounds.top;
	pRect->right  = bounds.right;
	pRect->bottom = bounds.bottom;
#endif
	return true;
}

//-----------------------------------------------------------------------------
bool CFrame::getSize (CRect& outSize) const
{
	return getSize (&outSize);
}

//-----------------------------------------------------------------------------
/**
 * @param pView the view which should be set to modal.
 * @return true if view could be set as the modal view. false if there is a already a modal view or the view to be set as modal is already attached.
 */
bool CFrame::setModalView (CView* pView)
{
	// If there is a modal view or the view 
	if ((pView && pModalView) || (pView && pView->isAttached ()))
		return false;

	if (pModalView)
		removeView (pModalView, false);
	
	pModalView = pView;
	if (pModalView)
		return addView (pModalView);

	return true;
}

//-----------------------------------------------------------------------------
void CFrame::beginEdit (long index)
{
	if (pEditor)
		pEditor->beginEdit (index);
}

//-----------------------------------------------------------------------------
void CFrame::endEdit (long index)
{
	if (pEditor)
		pEditor->endEdit (index);
}

//-----------------------------------------------------------------------------
/**
 * @param where location of mouse
 * @return true on success
 */
bool CFrame::getCurrentMouseLocation (CPoint &where) const
{
	#if WINDOWS
	HWND hwnd = (HWND)this->getSystemWindow ();
	POINT _where;
	GetCursorPos (&_where);
	where ((CCoord)_where.x, (CCoord)_where.y);
	if (hwnd)
	{
		RECT rctTempWnd;
		GetWindowRect (hwnd, &rctTempWnd);
		where.offset ((CCoord)-rctTempWnd.left, (CCoord)-rctTempWnd.top);
	}
	return true;
	#endif
	
	#if MAC_COCOA
	if (nsView)
		return nsViewGetCurrentMouseLocation (nsView, where);
	#endif
	
	#if MAC_CARBON
	// no up-to-date API call available for this, so use old QuickDraw
	Point p;
	CGrafPtr savedPort;
	Boolean portChanged = QDSwapPort (GetWindowPort ((WindowRef)getSystemWindow ()), &savedPort);
	GetMouse (&p);
	if (portChanged)
		QDSwapPort (savedPort, NULL);
	where (p.h, p.v);

	HIPoint location;
	HIViewRef fromView = NULL;
	HIViewFindByID (HIViewGetRoot ((WindowRef)getSystemWindow ()), kHIViewWindowContentID, &fromView);
	location = CGPointMake (where.x, where.y);
	#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
	#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_4
	if (HIPointConvert != NULL)
	#else
	if (true)
	#endif
		HIPointConvert (&location, kHICoordSpaceView, fromView, kHICoordSpaceView, controlRef);
	else
	#endif
		HIViewConvertPoint (&location, fromView, controlRef);
	where.x = (CCoord)location.x;
	where.y = (CCoord)location.y;

	return true;

	#endif // MAC_CARBON

	return false;
}

//-----------------------------------------------------------------------------
/**
 * @return mouse and modifier state
 */
long CFrame::getCurrentMouseButtons () const
{
	long buttons = 0;
	
#if WINDOWS
	if (GetAsyncKeyState (VK_LBUTTON) < 0)
		buttons |= (bSwapped_mouse_buttons ? kRButton : kLButton);
	if (GetAsyncKeyState (VK_MBUTTON) < 0)
		buttons |= kMButton;
	if (GetAsyncKeyState (VK_RBUTTON) < 0)
		buttons |= (bSwapped_mouse_buttons ? kLButton : kRButton);
	
	if (GetAsyncKeyState (VK_SHIFT)   < 0)
		buttons |= kShift;
	if (GetAsyncKeyState (VK_CONTROL) < 0)
		buttons |= kControl;
	if (GetAsyncKeyState (VK_MENU)    < 0)
		buttons |= kAlt;
#endif

#if MAC
	UInt32 state = GetCurrentButtonState ();
	if (state == kEventMouseButtonPrimary)
		buttons |= kLButton;
	if (state == kEventMouseButtonSecondary)
		buttons |= kRButton;
	if (state == kEventMouseButtonTertiary)
		buttons |= kMButton;
	if (state == 4)
		buttons |= kButton4;
	if (state == 5)
		buttons |= kButton5;

	state = GetCurrentKeyModifiers ();
	if (state & cmdKey)
		buttons |= kControl;
	if (state & shiftKey)
		buttons |= kShift;
	if (state & optionKey)
		buttons |= kAlt;
	if (state & controlKey)
		buttons |= kApple;
	// for the one buttons
	if (buttons & kApple && buttons & kLButton)
	{
		buttons &= ~(kApple | kLButton);
		buttons |= kRButton;
	}

#endif
	
	return buttons;
}

#if MAC_CARBON
#define kThemeResizeUpDownCursor	21
#define kThemeNotAllowedCursor		18
#endif

//-----------------------------------------------------------------------------
/**
 * @param type cursor type see #CCursorType
 */
void CFrame::setCursor (CCursorType type)
{
	#if WINDOWS
	if (!defaultCursor)
		defaultCursor = GetCursor ();
	switch (type)
	{
		case kCursorWait:
			SetCursor (LoadCursor (0, IDC_WAIT));
			break;
		case kCursorHSize:
			SetCursor (LoadCursor (0, IDC_SIZEWE));
			break;
		case kCursorVSize:
			SetCursor (LoadCursor (0, IDC_SIZENS));
			break;
		case kCursorNESWSize:
			SetCursor (LoadCursor (0, IDC_SIZENESW));
			break;
		case kCursorNWSESize:
			SetCursor (LoadCursor (0, IDC_SIZENWSE));
			break;
		case kCursorSizeAll:
			SetCursor (LoadCursor (0, IDC_SIZEALL));
			break;
		case kCursorNotAllowed:
			SetCursor (LoadCursor (0, IDC_NO));
			break;
		case kCursorHand:
			SetCursor (LoadCursor (0, IDC_HAND));
			break;
		default:
			SetCursor ((HCURSOR)defaultCursor);
			break;
	}
	#endif
	
	#if MAC_COCOA
	if (nsView)
	{
		nsViewSetMouseCursor (type);
		return;
	}
	#endif
	
	#if MAC_CARBON
	switch (type)
	{
		case kCursorWait:
			SetThemeCursor (kThemeWatchCursor);
			break;
		case kCursorHSize:
			SetThemeCursor (pSystemVersion < 0x1030 ? kThemeCrossCursor : kThemeResizeLeftRightCursor);
			break;
		case kCursorVSize:
			SetThemeCursor (pSystemVersion < 0x1030 ? kThemeCrossCursor : kThemeResizeUpDownCursor);
			break;
		case kCursorNESWSize:
			SetThemeCursor (kThemeCrossCursor);
			break;
		case kCursorNWSESize:
			SetThemeCursor (kThemeCrossCursor);
			break;
		case kCursorSizeAll:
			SetThemeCursor (kThemeCrossCursor);
			break;
		case kCursorCopy:
			SetThemeCursor (kThemeCopyArrowCursor);
			break;
		case kCursorNotAllowed:
			SetThemeCursor (pSystemVersion < 0x1020 ? kThemeArrowCursor : kThemeNotAllowedCursor);
			break;
		case kCursorHand:
			SetThemeCursor (kThemeOpenHandCursor);
			break;
		default:
			SetThemeCursor (kThemeArrowCursor);
			break;
	}
	#endif
}

//-----------------------------------------------------------------------------
/**
 * @param pView view which was removed
 */
void CFrame::onViewRemoved (CView* pView)
{
	if (pMouseOverView == pView)
		pMouseOverView = 0;
	if (pFocusView == pView)
		setFocusView (0);
	if (pView->isTypeOf ("CViewContainer"))
	{
		CViewContainer* container = (CViewContainer*)pView;
		if (container->isChild (pMouseOverView, true))
			pMouseOverView = 0;
		if (container->isChild (pFocusView, true))
			setFocusView (0);
	}
}

//-----------------------------------------------------------------------------
/**
 * @param pView view which was added
 */
void CFrame::onViewAdded (CView* pView)
{
}

//-----------------------------------------------------------------------------
/**
 * @param pView new focus view
 */
void CFrame::setFocusView (CView *pView)
{
	static bool recursion = false;
	if (pView == pFocusView || (recursion && pFocusView != 0))
		return;

	recursion = true;

	CView *pOldFocusView = pFocusView;
	pFocusView = pView;
	if (pFocusView && pFocusView->wantsFocus ())
	{
		pFocusView->invalid ();

		CView* receiver = pFocusView->getParentView ();
		while (receiver != this && receiver != 0)
		{
			receiver->notify (pFocusView, kMsgNewFocusView);
			receiver = receiver->getParentView ();
		}
	}

	if (pOldFocusView)
	{
		if (pOldFocusView->wantsFocus ())
		{
			pOldFocusView->invalid ();

			CView* receiver = pOldFocusView->getParentView ();
			while (receiver != this && receiver != 0)
			{
				receiver->notify (pOldFocusView, kMsgOldFocusView);
				receiver = receiver->getParentView ();
			}
		}
		pOldFocusView->looseFocus ();
	}
	if (pFocusView && pFocusView->wantsFocus ())
		pFocusView->takeFocus ();
	recursion = false;
}

//-----------------------------------------------------------------------------
bool CFrame::advanceNextFocusView (CView* oldFocus, bool reverse)
{
	if (pModalView)
	{
		if (pModalView->isTypeOf("CViewContainer"))
		{
			return ((CViewContainer*)pModalView)->advanceNextFocusView (oldFocus, reverse);
		}
		else if (oldFocus != pModalView)
		{
			setFocusView (pModalView);
			return true;
		}
		return false; // currently not supported, but should be done sometime
	}
	if (oldFocus == 0)
	{
		if (pFocusView == 0)
			return CViewContainer::advanceNextFocusView (0, reverse);
		oldFocus = pFocusView;
	}
	if (isChild (oldFocus))
	{
		if (CViewContainer::advanceNextFocusView (oldFocus, reverse))
			return true;
		else
		{
			setFocusView (NULL);
			return false;
		}
	}
	CView* parentView = oldFocus->getParentView ();
	if (parentView && parentView->isTypeOf ("CViewContainer"))
	{
		CView* tempOldFocus = oldFocus;
		CViewContainer* vc = (CViewContainer*)parentView;
		while (vc)
		{
			if (vc->advanceNextFocusView (tempOldFocus, reverse))
				return true;
			else
			{
				tempOldFocus = vc;
				if (vc->getParentView () && vc->getParentView ()->isTypeOf ("CViewContainer"))
					vc = (CViewContainer*)vc->getParentView ();
				else
					vc = 0;
			}
		}
	}
	return CViewContainer::advanceNextFocusView (oldFocus, reverse);
}

//-----------------------------------------------------------------------------
bool CFrame::removeView (CView* pView, const bool &withForget)
{
	if (pModalView == pView)
		pModalView = 0;
	return CViewContainer::removeView (pView, withForget);
}

//-----------------------------------------------------------------------------
bool CFrame::removeAll (const bool &withForget)
{
	pModalView = 0;
	pFocusView = 0;
	pMouseOverView = 0;
	return CViewContainer::removeAll (withForget);
}

//-----------------------------------------------------------------------------
/**
 * @param src rect which to scroll
 * @param distance point of distance
 */
void CFrame::scrollRect (const CRect& src, const CPoint& distance)
{
	CRect rect (src);
	rect.offset (size.left, size.top);

	#if MAC_COCOA
	if (nsView)
	{
		nsViewScrollRect (nsView, src, distance);
		return;
	}
	#endif
	
	#if MAC_CARBON
	if (isWindowComposited ((WindowRef)pWindow))
	{
		if (distance.x > 0)
			rect.right += distance.x;
		else if (distance.x < 0)
			rect.left += distance.x;
		if (distance.y > 0)
			rect.bottom += distance.y;
		else if (distance.y < 0)
			rect.top += distance.y;
		CGRect cgRect = CGRectMake ((CGFloat)rect.left, (CGFloat)rect.top, (CGFloat)rect.getWidth (), (CGFloat)rect.getHeight ());
		if (HIViewScrollRect ((HIViewRef)getPlatformControl(), &cgRect, (CGFloat)distance.x, (CGFloat)distance.y) == noErr)
			return;
	}
	#endif

	#if WINDOWS
	// TODO: Windows native scrollRect implementation
	#endif
	
	invalidRect (src);
}

//-----------------------------------------------------------------------------
void CFrame::invalidate (const CRect &rect)
{
	CRect rectView;
	FOREACHSUBVIEW
	if (pV)
	{
		pV->getViewSize (rectView);
		if (rect.rectOverlap (rectView))
			pV->setDirty (true);
	}
	ENDFOR
}

//-----------------------------------------------------------------------------
void CFrame::invalidRect (CRect rect)
{
	if (!bVisible)
		return;
	#if MAC_COCOA
	if (nsView)
	{
		invalidNSViewRect (nsView, rect);
		return;
	}
	#endif
	
	#if MAC_CARBON
	if (isWindowComposited ((WindowRef)pSystemWindow))
	{
		#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
		#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_4
		if (HIViewSetNeedsDisplayInRect != NULL)
		#else
		if (true)
		#endif
		{
			HIRect r = { {rect.left, rect.top}, {rect.getWidth (), rect.getHeight ()} };
			HIViewSetNeedsDisplayInRect (controlRef, &r, true);
		}
		else
		#endif
		{
			RgnHandle region = NewRgn ();
			SetRectRgn (region, (short)rect.left, (short)rect.top, (short)rect.right, (short)rect.bottom);
			HIViewSetNeedsDisplayInRegion (controlRef, region, true);
			DisposeRgn(region);
		}
	}
	else
	{
		HIRect hiRect;
		HIViewGetFrame (controlRef, &hiRect);
		CRect _rect (rect);
		_rect.offset (size.left, size.top);
		_rect.offset ((CCoord)hiRect.origin.x, (CCoord)hiRect.origin.y);
		Rect r = {(short)_rect.top, (short)_rect.left, (short)_rect.bottom, (short)_rect.right};
		InvalWindowRect ((WindowRef)pSystemWindow, &r);
	}
	
	#endif // MAC_CARBON
	
	#if WINDOWS
	RECT r = {(LONG)rect.left, (LONG)rect.top, (LONG)rect.right, (LONG)rect.bottom};
	InvalidateRect ((HWND)pHwnd, &r, true);
	#endif
}

#if DEBUG
//-----------------------------------------------------------------------------
void CFrame::dumpHierarchy ()
{
	dumpInfo ();
	DebugPrint ("\n");
	CViewContainer::dumpHierarchy ();
}
#endif

#if MAC_CARBON
//-----------------------------------------------------------------------------
// MacDragContainer Declaration
//-----------------------------------------------------------------------------
class MacDragContainer : public CDragContainer
{
public:
	MacDragContainer (DragRef platformDrag);
	~MacDragContainer ();

	virtual void* first (long& size, long& type);		///< returns pointer on a char array if type is known
	virtual void* next (long& size, long& type);		///< returns pointer on a char array if type is known
	
	virtual long getType (long idx) const;
	virtual long getCount () const { return nbItems; }

	DragRef getPlatformDrag () const { return platformDrag; }

protected:
	DragRef platformDrag;
	PasteboardRef pasteboard;
	long nbItems;
	
	long iterator;
	void* lastItem;
};

static MacDragContainer* gDragContainer = 0;

//-----------------------------------------------------------------------------
// MacDragContainer Implementation
//-----------------------------------------------------------------------------
MacDragContainer::MacDragContainer (DragRef inPlatformDrag)
: platformDrag (inPlatformDrag)
, pasteboard (0)
, nbItems (0)
, iterator (0)
, lastItem (0)
{
	if (GetDragPasteboard (inPlatformDrag, &pasteboard) == noErr)
	{
		ItemCount numItems;
		if (PasteboardGetItemCount (pasteboard, &numItems) == noErr)
			nbItems = numItems;
	}
}

//-----------------------------------------------------------------------------
MacDragContainer::~MacDragContainer ()
{
	if (lastItem)
	{
		free (lastItem);
		lastItem = 0;
	}
}

//-----------------------------------------------------------------------------
long MacDragContainer::getType (long idx) const
{
	if (platformDrag == 0)
		return CDragContainer::kError;

	PasteboardItemID itemID;
	if (PasteboardGetItemIdentifier (pasteboard, idx+1, &itemID) == noErr)
	{
		CFArrayRef flavors = 0;
		if (PasteboardCopyItemFlavors (pasteboard, itemID, &flavors) == noErr)
		{
			long result = CDragContainer::kUnknown;
			for (CFIndex i = 0; i < CFArrayGetCount (flavors); i++)
			{
				CFStringRef flavorType = (CFStringRef)CFArrayGetValueAtIndex (flavors, i);
				if (flavorType == 0)
					continue;
				CFStringRef osTypeFlavorType = UTTypeCopyPreferredTagWithClass (flavorType, kUTTagClassOSType);
				if (osTypeFlavorType == 0)
					continue;
				if (CFStringCompare (osTypeFlavorType, CFSTR("utxt"), 0) == kCFCompareEqualTo)
					result = CDragContainer::kUnicodeText;
				else if (CFStringCompare (osTypeFlavorType, CFSTR("utf8"), 0) == kCFCompareEqualTo)
					result = CDragContainer::kUnicodeText;
				else if (CFStringCompare (osTypeFlavorType, CFSTR("furl"), 0) == kCFCompareEqualTo)
					result = CDragContainer::kFile;
				else if (CFStringCompare (osTypeFlavorType, CFSTR("TEXT"), 0) == kCFCompareEqualTo)
					result = CDragContainer::kText;
				else if (CFStringCompare (osTypeFlavorType, CFSTR("XML "), 0) == kCFCompareEqualTo)
					result = CDragContainer::kText;
				CFRelease (osTypeFlavorType);
				if (result != CDragContainer::kUnknown)
					break;
			}
			CFRelease (flavors);
			return result;
		}
	}
	return CDragContainer::kUnknown;
}

//-----------------------------------------------------------------------------
void* MacDragContainer::first (long& size, long& type)
{
	iterator = 0;
	return next (size, type);
}

//-----------------------------------------------------------------------------
void* MacDragContainer::next (long& size, long& type)
{
	if (platformDrag == 0)
	{
		type = CDragContainer::kError;
		return 0;
	}
	if (lastItem)
	{
		free (lastItem);
		lastItem = 0;
	}
	size = 0;
	type = CDragContainer::kUnknown;

	PasteboardItemID itemID;
	if (PasteboardGetItemIdentifier (pasteboard, ++iterator, &itemID) == noErr)
	{
		CFArrayRef flavors = 0;
		if (PasteboardCopyItemFlavors (pasteboard, itemID, &flavors) == noErr)
		{
			for (CFIndex i = 0; i < CFArrayGetCount (flavors); i++)
			{
				CFStringRef flavorType = (CFStringRef)CFArrayGetValueAtIndex (flavors, i);
				if (flavorType == 0)
					continue;
				CFStringRef osTypeFlavorType = UTTypeCopyPreferredTagWithClass (flavorType, kUTTagClassOSType);
				if (osTypeFlavorType == 0)
					continue;
				PasteboardFlavorFlags flavorFlags;
				PasteboardGetItemFlavorFlags (pasteboard, itemID, flavorType, &flavorFlags);
				CFDataRef flavorData = 0;
				if (PasteboardCopyItemFlavorData (pasteboard, itemID, flavorType, &flavorData) == noErr)
				{
					CFIndex flavorDataSize = CFDataGetLength (flavorData);
					const UInt8* data = CFDataGetBytePtr (flavorData);
					if (data)
					{
						if (CFStringCompare (osTypeFlavorType, CFSTR("utxt"), 0) == kCFCompareEqualTo)
						{
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4
							CFStringRef utf16String = CFStringCreateWithBytes(0, data, flavorDataSize, kCFStringEncodingUTF16, false);
#else
							CFStringRef utf16String = CFStringCreateWithBytes(0, data, flavorDataSize, kCFStringEncodingUnicode, false);
#endif
							if (utf16String)
							{
								CFIndex maxSize = CFStringGetMaximumSizeForEncoding (flavorDataSize/2, kCFStringEncodingUTF8);
								lastItem = malloc (maxSize+1);
								if (CFStringGetCString (utf16String, (char*)lastItem, maxSize, kCFStringEncodingUTF8))
								{
									type = CDragContainer::kUnicodeText;
									size = strlen ((const char*)lastItem);
								}
								else
								{
									free (lastItem);
									lastItem = 0;
								}
								CFRelease (utf16String);
							}
						}
						else if (CFStringCompare (osTypeFlavorType, CFSTR("furl"), 0) == kCFCompareEqualTo)
						{
							type = CDragContainer::kFile;
							CFURLRef url = CFURLCreateWithBytes (NULL, data, flavorDataSize, kCFStringEncodingUTF8, NULL);
							lastItem = malloc (PATH_MAX);
							CFURLGetFileSystemRepresentation (url, false, (UInt8*)lastItem, PATH_MAX);
							CFRelease (url);
							size = strlen ((const char*)lastItem);
						}
						else if (CFStringCompare (osTypeFlavorType, CFSTR("utf8"), 0) == kCFCompareEqualTo)
						{
							type = CDragContainer::kUnicodeText;
							size = flavorDataSize;
							lastItem = malloc (flavorDataSize + 1);
							((char*)lastItem)[flavorDataSize] = 0;
							memcpy (lastItem, data, flavorDataSize);
						}
						else if (CFStringCompare (osTypeFlavorType, CFSTR("TEXT"), 0) == kCFCompareEqualTo)
						{
							type = CDragContainer::kText;
							size = flavorDataSize;
							lastItem = malloc (flavorDataSize + 1);
							((char*)lastItem)[flavorDataSize] = 0;
							memcpy (lastItem, data, flavorDataSize);
						}
						else if (CFStringCompare (osTypeFlavorType, CFSTR("XML "), 0) == kCFCompareEqualTo)
						{
							type = CDragContainer::kText;
							size = flavorDataSize;
							lastItem = malloc (flavorDataSize + 1);
							((char*)lastItem)[flavorDataSize] = 0;
							memcpy (lastItem, data, flavorDataSize);
						}
					}
					CFRelease (flavorData);
				}
				CFRelease (osTypeFlavorType);
				if (type != CDragContainer::kUnknown)
					break;
			}
			CFRelease (flavors);
			return lastItem;
		}
	}
	return NULL;
}
#endif // MAC_CARBON

#if WINDOWS
//-----------------------------------------------------------------------------
// WinDragContainer Declaration
//-----------------------------------------------------------------------------
class WinDragContainer : public CDragContainer
{
public:
	WinDragContainer (void* platformDrag);
	~WinDragContainer ();

	virtual void* first (long& size, long& type);		///< returns pointer on a char array if type is known
	virtual void* next (long& size, long& type);		///< returns pointer on a char array if type is known
	
	virtual long getType (long idx) const;
	virtual long getCount () const { return nbItems; }

protected:
	void* platformDrag;
	long nbItems;
	
	long iterator;
	void* lastItem;
};
static WinDragContainer* gDragContainer = 0;

//-----------------------------------------------------------------------------
// WinDragContainer Implementation
//-----------------------------------------------------------------------------
WinDragContainer::WinDragContainer (void* platformDrag)
: platformDrag (platformDrag)
, nbItems (0)
, iterator (0)
, lastItem (0)
{
	if (!platformDrag)
		return;

	IDataObject* dataObject = (IDataObject*)platformDrag;
	STGMEDIUM medium;
	FORMATETC formatTEXTDrop = {
		#if VSTGUI_USES_UTF8
		CF_UNICODETEXT,
		#else
		CF_TEXT,
		#endif
		0, 
		DVASPECT_CONTENT, 
		-1, 
		TYMED_HGLOBAL
	};
	
	FORMATETC formatHDrop    = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	// todo : Support CF_UNICODETEXT

	long type = 0; // 0 = file, 1 = text

	HRESULT hr = dataObject->GetData (&formatTEXTDrop, &medium);
	if (hr != S_OK)
		hr = dataObject->GetData (&formatHDrop, &medium);
	else
		type = 1;
	
	if (type == 0)
		nbItems = (long)DragQueryFile ((HDROP)medium.hGlobal, 0xFFFFFFFFL, 0, 0);
	else
		nbItems = 1;
}

//-----------------------------------------------------------------------------
WinDragContainer::~WinDragContainer ()
{
	if (lastItem)
	{
		free (lastItem);
		lastItem = 0;
	}
}

//-----------------------------------------------------------------------------
long WinDragContainer::getType (long idx) const
{
	if (platformDrag == 0)
		return kError;

	IDataObject* dataObject = (IDataObject*)platformDrag;
	STGMEDIUM medium;
	FORMATETC formatTEXTDrop = {
		#if VSTGUI_USES_UTF8
		CF_UNICODETEXT,
		#else
		CF_TEXT,
		#endif
		0, 
		DVASPECT_CONTENT, 
		-1, 
		TYMED_HGLOBAL
	};
	FORMATETC formatHDrop    = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	long type = 0; // 0 = file, 1 = text

	HRESULT hr = dataObject->GetData (&formatTEXTDrop, &medium);
	if (hr != S_OK)
		hr = dataObject->GetData (&formatHDrop, &medium);
	else
		type = 1;
	if (type == 0)
		return kFile;
	else
		return kText;

	return kUnknown;
}

//-----------------------------------------------------------------------------
void* WinDragContainer::first (long& size, long& type)
{
	iterator = 0;
	return next (size, type);
}

//-----------------------------------------------------------------------------
void* WinDragContainer::next (long& size, long& type)
{
	if (platformDrag == 0)
	{
		type = kError;
		return 0;
	}
	if (lastItem)
	{
		free (lastItem);
		lastItem = 0;
	}
	size = 0;
	type = kUnknown;

	IDataObject* dataObject = (IDataObject*)platformDrag;
	void* hDrop = 0;
	STGMEDIUM medium;
	FORMATETC formatTEXTDrop = {
		#if VSTGUI_USES_UTF8
		CF_UNICODETEXT,
		#else
		CF_TEXT,
		#endif
		0, 
		DVASPECT_CONTENT, 
		-1, 
		TYMED_HGLOBAL
	};
	FORMATETC formatHDrop    = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	long wintype = 0; // 0 = file, 1 = text

	HRESULT hr = dataObject->GetData (&formatTEXTDrop, &medium);
	if (hr != S_OK)
		hr = dataObject->GetData (&formatHDrop, &medium);
	else
		wintype = 1;
	if (hr == S_OK)
		hDrop = medium.hGlobal;

	if (hDrop)
	{
		if (wintype == 0)
		{
			TCHAR fileDropped[1024];

			long nbRealItems = 0;
			if (DragQueryFile ((HDROP)hDrop, iterator++, fileDropped, sizeof (fileDropped))) 
			{
				// resolve link
				checkResolveLink (fileDropped, fileDropped);
				UTF8StringHelper path (fileDropped);
				lastItem = malloc (strlen (path)+1);
				strcpy ((char*)lastItem, path);
				size = (long)strlen ((const char*)lastItem);
				type = kFile;
				return lastItem;
			}
		}
		else if (iterator++ == 0)
		//---TEXT----------------------------
		{
			void* data = GlobalLock (medium.hGlobal);
			long dataSize = (long)GlobalSize (medium.hGlobal);
			if (data && dataSize)
			{
				#if VSTGUI_USES_UTF8
				UTF8StringHelper wideString ((const WCHAR*)data);
				size = strlen (wideString.getUTF8String ());
				lastItem = malloc (size+1);
				strcpy ((char*)lastItem, wideString.getUTF8String ());
				type = kUnicodeText;
				#else
				lastItem = malloc (dataSize+1);
				memcpy (lastItem, data, dataSize);
				size = dataSize;
				type = kText;
				#endif
			}

			GlobalUnlock (medium.hGlobal);
			if (medium.pUnkForRelease)
				medium.pUnkForRelease->Release ();
			else
				GlobalFree (medium.hGlobal);
			return lastItem;
		}
	}
	return NULL;
}
#endif // WINDOWS

END_NAMESPACE_VSTGUI


#if WINDOWS
BEGIN_NAMESPACE_VSTGUI

#if USE_MOUSE_HOOK
HHOOK MouseHook = 0L;

LRESULT CALLBACK MouseProc (int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)
		return CallNextHookEx (MouseHook, nCode, wParam, lParam);

	if (wParam == 522)
	{
		MOUSEHOOKSTRUCT* struct2 = (MOUSEHOOKSTRUCT*) lParam;
		if (struct2->hwnd == ???)
		{
			return -1;
		}
	}
	return CallNextHookEx (MouseHook, nCode, wParam, lParam);
}
#endif

//-----------------------------------------------------------------------------
bool InitWindowClass ()
{
	gUseCount++;
	if (gUseCount == 1)
	{
		VSTGUI_SPRINTF (gClassName, TEXT("Plugin%p"), GetInstance ());
		
		WNDCLASS windowClass;
		windowClass.style = CS_GLOBALCLASS | CS_DBLCLKS;//|CS_OWNDC; // add Private-DC constant 

		windowClass.lpfnWndProc = WindowProc; 
		windowClass.cbClsExtra  = 0; 
		windowClass.cbWndExtra  = 0; 
		windowClass.hInstance   = GetInstance ();
		windowClass.hIcon = 0; 

		windowClass.hCursor = LoadCursor (NULL, IDC_ARROW);
		#if DEBUG_DRAWING
		windowClass.hbrBackground = GetSysColorBrush (COLOR_BTNFACE);
		#else
		windowClass.hbrBackground = 0;
		#endif
		windowClass.lpszMenuName  = 0; 
		windowClass.lpszClassName = gClassName; 
		RegisterClass (&windowClass);

		#if USE_MOUSE_HOOK
		MouseHook = SetWindowsHookEx (WH_MOUSE, MouseProc, GetInstance (), 0);
		#endif

		bSwapped_mouse_buttons = GetSystemMetrics (SM_SWAPBUTTON) > 0;
	}
	return true;
}

//-----------------------------------------------------------------------------
void ExitWindowClass ()
{
	gUseCount--;
	if (gUseCount == 0)
	{
		UnregisterClass (gClassName, GetInstance ());

		#if USE_MOUSE_HOOK
		if (MouseHook)
		{
			UnhookWindowsHookEx (MouseHook);
			MouseHook = 0L;
		}
		#endif
	}
}

//-----------------------------------------------------------------------------
LONG_PTR WINAPI WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	USING_NAMESPACE_VSTGUI
	CFrame* pFrame = (CFrame*)(LONG_PTR)GetWindowLongPtr (hwnd, GWLP_USERDATA);

	bool doubleClick = false;

	switch (message)
	{
	case WM_MOUSEWHEEL:
	{
		if (pFrame)
		{
			long buttons = 0;
			if (GetKeyState (VK_SHIFT)   < 0)
				buttons |= kShift;
			if (GetKeyState (VK_CONTROL) < 0)
				buttons |= kControl;
			if (GetKeyState (VK_MENU)    < 0)
				buttons |= kAlt;
			CPoint where (LOWORD (lParam), HIWORD (lParam));
			short zDelta = (short) HIWORD(wParam);
			RECT rctWnd;
			GetWindowRect (hwnd, &rctWnd);
			where.offset ((CCoord)-rctWnd.left, (CCoord)-rctWnd.top);
			pFrame->onWheel (where, (float)(zDelta / WHEEL_DELTA), buttons);
		}
		break;
	}
	case WM_CTLCOLOREDIT:
	{
		if (pFrame)
		{
			CTextEdit* textEdit = (CTextEdit*)pFrame->getFocusView ();
			if (textEdit)
			{
				CColor fontColor = textEdit->getFontColor ();
				SetTextColor ((HDC) wParam, RGB (fontColor.red, fontColor.green, fontColor.blue));

				CColor backColor = textEdit->getBackColor ();
				SetBkColor ((HDC) wParam, RGB (backColor.red, backColor.green, backColor.blue));

				if (textEdit->platformFontColor)
					DeleteObject (textEdit->platformFontColor);
				textEdit->platformFontColor = CreateSolidBrush (RGB (backColor.red, backColor.green, backColor.blue));
				return (LRESULT)(textEdit->platformFontColor);
			}
		}
	}
	break;

	case WM_PAINT:
	{
		if (pFrame)// && GetUpdateRect (hwnd, &r, false))
		{
			HRGN rgn = CreateRectRgn (0, 0, 0, 0);
			if (GetUpdateRgn (hwnd, rgn, false) == NULLREGION)
			{
				DeleteObject (rgn);
				return 0;
			}

			PAINTSTRUCT ps;
			HDC hdc = BeginPaint (hwnd, &ps);

			CDrawContext* context = pFrame->getBackBuffer ();
			if (!context)
				context = new CDrawContext (pFrame, hdc, hwnd);
			
			CRect updateRect ((CCoord)ps.rcPaint.left, (CCoord)ps.rcPaint.top, (CCoord)ps.rcPaint.right, (CCoord)ps.rcPaint.bottom);

			#if 0
			int len = GetRegionData (rgn, 0, NULL);
			if (len)
			{
				RGNDATA* rlist = (RGNDATA* )new char[len];
				GetRegionData (rgn, len, rlist);
				if (rlist->rdh.nCount > 0)
				{
					RECT* rp = (RECT*)rlist->Buffer;
					for (unsigned int i = 0; i < rlist->rdh.nCount; i++)
					{
						CRect ur (rp->left, rp->top, rp->right, rp->bottom);
						pFrame->drawRect (context, ur);
						rp++;
					}
				}
				else
					pFrame->drawRect (context, updateRect);
				delete [] (char*)rlist;
			}
			else
			#endif
				pFrame->drawRect (context, updateRect);

			if (pFrame->getBackBuffer ())
			{
				CDrawContext localContext (pFrame, hdc, hwnd);
				pFrame->getBackBuffer ()->copyFrom (&localContext, updateRect, CPoint ((CCoord)ps.rcPaint.left, (CCoord)ps.rcPaint.top));
			}
			else
				context->forget ();


			EndPaint (hwnd, &ps);
			DeleteObject (rgn);
			return 0;
		}
	}
	break;
#if 0 // currently disabled because of COptionMenu rewrite
	case WM_MEASUREITEM :
	{
		MEASUREITEMSTRUCT* ms = (MEASUREITEMSTRUCT*)lParam;
		if (pFrame && ms && ms->CtlType == ODT_MENU && ms->itemData)
		{
			COptionMenu* optMenu = (COptionMenu*)pFrame->getFocusView ();
			if (optMenu && optMenu->getScheme ())
			{
				CPoint size;

				CDrawContext context (pFrame, 0, hwnd);
				optMenu->getScheme ()->getItemSize ((const char*)ms->itemData, &context, size);

				ms->itemWidth  = (UINT)size.h;
				ms->itemHeight = (UINT)size.v;
				return TRUE;
			}
		}
	}
	break;

	case WM_DRAWITEM :
	{
		DRAWITEMSTRUCT* ds = (DRAWITEMSTRUCT*)lParam;
		if (pFrame && ds && ds->CtlType == ODT_MENU && ds->itemData)
		{
			COptionMenu* optMenu = (COptionMenu*)pFrame->getFocusView ();
			if (optMenu && optMenu->getScheme ())
			{
				long state = 0;
				if (ds->itemState & ODS_CHECKED)
					state |= COptionMenuScheme::kChecked;
				if (ds->itemState & ODS_DISABLED) // ODS_GRAYED?
					state |= COptionMenuScheme::kDisabled;
				if (ds->itemState & ODS_SELECTED)
					state |= COptionMenuScheme::kSelected;
					
				CRect r ((CCoord)ds->rcItem.left, (CCoord)ds->rcItem.top, (CCoord)ds->rcItem.right, (CCoord)ds->rcItem.bottom);
				r.bottom++;
				
				CDrawContext* pContext = new CDrawContext (pFrame, ds->hDC, 0);
				optMenu->getScheme ()->drawItem ((const char*)ds->itemData, ds->itemID, state, pContext, r);
				delete pContext;
				return TRUE;
			}
		}
	}
	break;
#endif

#if 1
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_LBUTTONDBLCLK:
	#if (_WIN32_WINNT >= 0x0500)
	case WM_XBUTTONDBLCLK:
	#endif
		doubleClick = true;
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONDOWN:
	#if (_WIN32_WINNT >= 0x0500)
	case WM_XBUTTONDOWN:
	#endif
		if (pFrame)
		{
			long buttons = 0;
			if (wParam & MK_LBUTTON)
				buttons |= kLButton;
			if (wParam & MK_RBUTTON)
				buttons |= kRButton;
			if (wParam & MK_MBUTTON)
				buttons |= kMButton;
			#if (_WIN32_WINNT >= 0x0500)
			if (wParam & MK_XBUTTON1)
				buttons |= kButton4;
			if (wParam & MK_XBUTTON2)
				buttons |= kButton5;
			#endif
			if (wParam & MK_CONTROL)
				buttons |= kControl;
			if (wParam & MK_SHIFT)
				buttons |= kShift;
			// added to achieve information from the ALT button
			if (GetKeyState (VK_MENU)    < 0)
				buttons |= kAlt;
			if (doubleClick)
				buttons |= kDoubleClick;
			CPoint where ((CCoord)((int)(short)LOWORD(lParam)), (CCoord)((int)(short)HIWORD(lParam)));
			if (pFrame->onMouseDown (where, buttons) == kMouseEventHandled)
				SetCapture ((HWND)pFrame->getSystemWindow ());
			return 0;
		}
		break;
	case WM_MOUSELEAVE:
		{
			CPoint where;
			pFrame->getCurrentMouseLocation (where);
			pFrame->onMouseExited (where, pFrame->getCurrentMouseButtons ());
			return 0;
		}
		break;
	case WM_MOUSEMOVE:
		if (pFrame)
		{
			long buttons = 0;
			if (wParam & MK_LBUTTON)
				buttons |= kLButton;
			if (wParam & MK_RBUTTON)
				buttons |= kRButton;
			if (wParam & MK_MBUTTON)
				buttons |= kMButton;
			#if (_WIN32_WINNT >= 0x0500)
			if (wParam & MK_XBUTTON1)
				buttons |= kButton4;
			if (wParam & MK_XBUTTON2)
				buttons |= kButton5;
			#endif
			if (wParam & MK_CONTROL)
				buttons |= kControl;
			if (wParam & MK_SHIFT)
				buttons |= kShift;
			// added to achieve information from the ALT button
			if (GetKeyState (VK_MENU)    < 0)
				buttons |= kAlt;
			CPoint where ((CCoord)((int)(short)LOWORD(lParam)), (CCoord)((int)(short)HIWORD(lParam)));
			pFrame->onMouseMoved (where, buttons);
			return 0;
		}
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	#if (_WIN32_WINNT >= 0x0500)
	case WM_XBUTTONUP:
	#endif
		if (pFrame)
		{
			long buttons = 0;
			if (wParam & MK_LBUTTON)
				buttons |= kLButton;
			if (wParam & MK_RBUTTON)
				buttons |= kRButton;
			if (wParam & MK_MBUTTON)
				buttons |= kMButton;
			#if (_WIN32_WINNT >= 0x0500)
			if (wParam & MK_XBUTTON1)
				buttons |= kButton4;
			if (wParam & MK_XBUTTON2)
				buttons |= kButton5;
			#endif
			if (wParam & MK_CONTROL)
				buttons |= kControl;
			if (wParam & MK_SHIFT)
				buttons |= kShift;
			// added to achieve information from the ALT button
			if (GetKeyState (VK_MENU)    < 0)
				buttons |= kAlt;
			CPoint where ((CCoord)((int)(short)LOWORD(lParam)), (CCoord)((int)(short)HIWORD(lParam)));
			pFrame->onMouseUp (where, buttons);
			ReleaseCapture ();
			return 0;
		}
		break;
#else
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONDOWN:
		if (pFrame)
		{
		#if 1
			CDrawContext context (pFrame, 0, hwnd);
			CPoint where (LOWORD (lParam), HIWORD (lParam));
			pFrame->mouse (&context, where);
		#else
			CPoint where (LOWORD (lParam), HIWORD (lParam));
			pFrame->mouse ((CDrawContext*)0, where);
		#endif

			return 0;
		}
		break;
#endif
		
	case WM_DESTROY:
		if (pFrame)
		{
			pFrame->setOpenFlag (false);
			pFrame->setParentSystemWindow (0);
		}
		break;
		// don't draw background
	case WM_ERASEBKGND:
		return 1;
		break;
	}

	return DefWindowProc (hwnd, message, wParam, lParam);
}
END_NAMESPACE_VSTGUI

#if GDIPLUS
#else
BEGIN_NAMESPACE_VSTGUI
//-----------------------------------------------------------------------------
HANDLE CreateMaskBitmap (CDrawContext* pContext, CRect& rect, CColor transparentColor)
{
	HBITMAP pMask = CreateBitmap (rect.width (), rect.height (), 1, 1, 0);

	HDC hSrcDC = (HDC)pContext->getSystemContext ();
	HDC hDstDC = CreateCompatibleDC (hSrcDC);
	SelectObject (hDstDC, pMask);

	COLORREF oldBkColor = SetBkColor (hSrcDC, RGB (transparentColor.red, transparentColor.green, transparentColor.blue));
	
	BitBlt (hDstDC, 0, 0, rect.width (), rect.height (), hSrcDC, rect.left, rect.top, SRCCOPY);
	
	SetBkColor (hSrcDC, oldBkColor);
	DeleteDC (hDstDC);
	
	return pMask;
}

//-----------------------------------------------------------------------------
void DrawTransparent (CDrawContext* pContext, CRect& rect, const CPoint& offset,
					  HDC hdcBitmap, POINT ptSize, HBITMAP pMask, COLORREF color)
{
	if (pMask == NULL)
	{
		if (pfnTransparentBlt)
		{
			HDC		hdcSystemContext = (HDC)pContext->getSystemContext ();
			long	x, y;
			long	width  = rect.width ();
			long	height = rect.height ();

			x = rect.x + pContext->offset.x;
			y = rect.y + pContext->offset.y;

			pfnTransparentBlt (hdcSystemContext, x, y, width, height, hdcBitmap, offset.x, offset.y, width, height, color);
		}
		else
		{
			// OPTIMIZATION: we only do four instead of EIGHT blits
			HDC		hdcSystemContext = (HDC)pContext->getSystemContext ();
			HDC		hdcMask = CreateCompatibleDC (hdcSystemContext);

			COLORREF	crOldBack = SetBkColor (hdcSystemContext, 0xFFFFFF);
			COLORREF	crOldText = SetTextColor (hdcSystemContext, 0x000000);
			HBITMAP		bmMaskOld, maskMap;

			long	x, y;
			long	width  = rect.width ();
			long	height = rect.height ();

			x = rect.x + pContext->offset.x;
			y = rect.y + pContext->offset.y;

			// Create mask-bitmap in memory
			maskMap = CreateBitmap (width, height, 1, 1, NULL);
			bmMaskOld = (HBITMAP)SelectObject (hdcMask, maskMap);

			// Copy bitmap into mask-bitmap and converting it into a black'n'white mask
			SetBkColor (hdcBitmap, color);
			BitBlt (hdcMask, 0, 0, width, height, hdcBitmap, offset.x, offset.y, SRCCOPY);

			// Copy image masked to screen
			BitBlt (hdcSystemContext, x, y, width, height, hdcBitmap, offset.x, offset.y, SRCINVERT);
			BitBlt (hdcSystemContext, x, y, width, height, hdcMask, 0, 0, SRCAND);
			BitBlt (hdcSystemContext, x, y, width, height, hdcBitmap, offset.x, offset.y, SRCINVERT);

			DeleteObject (SelectObject (hdcMask, bmMaskOld));
			DeleteDC (hdcMask);

			SetBkColor (hdcSystemContext, crOldBack);
			SetTextColor (hdcSystemContext, crOldText);
		}
	}
	else
	{
		// OPTIMIZATION: we only do five instead of EIGHT blits
		HDC		hdcSystemContext = (HDC)pContext->getSystemContext ();
		HDC		hdcMask = CreateCompatibleDC (hdcSystemContext);
		HDC		hdcMem = CreateCompatibleDC (hdcSystemContext);
		HBITMAP	bmAndMem;
		HBITMAP	bmMemOld, bmMaskOld;

		long	x, y;
		long	width = rect.width ();
		long	height = rect.height ();

		x = rect.x + pContext->offset.x;
		y = rect.y + pContext->offset.y;

		bmAndMem = CreateCompatibleBitmap(hdcSystemContext, width, height);

		bmMaskOld   = (HBITMAP)SelectObject (hdcMask, pMask);
		bmMemOld    = (HBITMAP)SelectObject (hdcMem, bmAndMem);

		BitBlt (hdcMem, 0, 0, width, height, hdcSystemContext, x, y, SRCCOPY);
		BitBlt (hdcMem, 0, 0, width, height, hdcBitmap, offset.x, offset.y, SRCINVERT);
		BitBlt (hdcMem, 0, 0, width, height, hdcMask, offset.x, offset.y, SRCAND);
		BitBlt (hdcMem, 0, 0, width, height, hdcBitmap, offset.x, offset.y, SRCINVERT);
		BitBlt (hdcSystemContext, x, y, width, height, hdcMem, 0, 0, SRCCOPY);

		DeleteObject (SelectObject (hdcMem, bmMemOld));
		SelectObject (hdcMask, bmMaskOld);

		DeleteDC (hdcMem);
		DeleteDC(hdcMask);
	}
}
END_NAMESPACE_VSTGUI
#endif
#endif

//-----------------------------------------------------------------------------
#if MAC
BEGIN_NAMESPACE_VSTGUI
// return a degre value between [0, 360 * 64[
long convertPoint2Angle (CPoint &pm, CPoint &pt)
{
	long angle;
	if (pt.h == pm.h)
	{
		if (pt.v < pm.v)
			angle = 5760;	// 90 * 64
		else
			angle = 17280; // 270 * 64
	}
	else if (pt.v == pm.v)
	{
		if (pt.h < pm.h)
			angle = 11520;	// 180 * 64
		else
			angle = 0;	
	}
	else
	{
		// 3666.9299 = 180 * 64 / pi
		angle = (long)(3666.9298 * atan ((double)(pm.v - pt.v) / (double)(pt.h - pm.h)));
    
		if (pt.v < pm.v)
		{
			if (pt.h < pm.h)
				angle += 11520; // 180 * 64
		}
		else
		{
			if (pt.h < pm.h)
				angle += 11520; // 180 * 64
			else
				angle += 23040; // 360 * 64
		}
	}
	return angle;
}
END_NAMESPACE_VSTGUI
#endif

//-----------------------------------------------------------------------------
#if WINDOWS
BEGIN_NAMESPACE_VSTGUI
//-----------------------------------------------------------------------------
bool checkResolveLink (const TCHAR* nativePath, TCHAR* resolved)
{
	const TCHAR* ext = VSTGUI_STRRCHR (nativePath, '.');
	if (ext && VSTGUI_STRICMP (ext, TEXT(".lnk")) == NULL)
	{
		IShellLink* psl;
		IPersistFile* ppf;
		WIN32_FIND_DATA wfd;
		HRESULT hres;
		WORD wsz[2048];
		
		// Get a pointer to the IShellLink interface.
		hres = CoCreateInstance (CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
			IID_IShellLink, (void**)&psl);
		if (SUCCEEDED (hres))
		{
			// Get a pointer to the IPersistFile interface.
			hres = psl->QueryInterface (IID_IPersistFile, (void**)&ppf);
			if (SUCCEEDED (hres))
			{
				#if !VSTGUI_USES_UTF8
				// Ensure string is Unicode.
				MultiByteToWideChar (CP_ACP, 0, nativePath, -1, (LPWSTR)wsz, 2048);
				#endif
				// Load the shell link.
				hres = ppf->Load ((LPWSTR)wsz, STGM_READ);
				if (SUCCEEDED (hres))
				{					
					hres = psl->Resolve (0, MAKELONG (SLR_ANY_MATCH | SLR_NO_UI, 500));
					if (SUCCEEDED (hres))
					{
						// Get the path to the link target.
						hres = psl->GetPath (resolved, 2048, &wfd, SLGP_SHORTPATH);
					}
				}
				// Release pointer to IPersistFile interface.
				ppf->Release ();
			}
			// Release pointer to IShellLink interface.
			psl->Release ();
		}
		return SUCCEEDED(hres);
	}
	return false;	
}

//-----------------------------------------------------------------------------
// Drop Implementation
//-----------------------------------------------------------------------------
class CDropTarget : public IDropTarget
{	
public:
	CDropTarget (CFrame* pFrame);
	virtual ~CDropTarget ();

	// IUnknown
	STDMETHOD (QueryInterface) (REFIID riid, void** object);
	STDMETHOD_ (ULONG, AddRef) (void);
	STDMETHOD_ (ULONG, Release) (void);
   
	// IDropTarget
	STDMETHOD (DragEnter) (IDataObject* dataObject, DWORD keyState, POINTL pt, DWORD* effect);
	STDMETHOD (DragOver) (DWORD keyState, POINTL pt, DWORD* effect);
	STDMETHOD (DragLeave) (void);
	STDMETHOD (Drop) (IDataObject* dataObject, DWORD keyState, POINTL pt, DWORD* effect);
private:
	long refCount;
	bool accept;
	CFrame* pFrame;
};

//-----------------------------------------------------------------------------
// CDropTarget
//-----------------------------------------------------------------------------
void* createDropTarget (CFrame* pFrame)
{
	return new CDropTarget (pFrame);
}

//-----------------------------------------------------------------------------
CDropTarget::CDropTarget (CFrame* pFrame)
: refCount (0), pFrame (pFrame)
{
}

//-----------------------------------------------------------------------------
CDropTarget::~CDropTarget ()
{
}

//-----------------------------------------------------------------------------
STDMETHODIMP CDropTarget::QueryInterface (REFIID riid, void** object)
{
	if (riid == IID_IDropTarget || riid == IID_IUnknown)
	{
		*object = this;
		AddRef ();
      return NOERROR;
	}
	*object = 0;
	return E_NOINTERFACE;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CDropTarget::AddRef (void)
{
	return ++refCount;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CDropTarget::Release (void)
{
	refCount--;
	if (refCount <= 0)
	{
		delete this;
		return 0;
	}
	return refCount;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CDropTarget::DragEnter (IDataObject* dataObject, DWORD keyState, POINTL pt, DWORD* effect)
{
	if (dataObject && pFrame)
	{
		gDragContainer = new WinDragContainer (dataObject);
		CDrawContext* context = pFrame->createDrawContext ();
		CPoint where;
		pFrame->getCurrentMouseLocation (where);
		pFrame->onDragEnter (gDragContainer, where);
		context->forget ();
		if ((*effect) & DROPEFFECT_COPY) 

			*effect = DROPEFFECT_COPY;

		else if ((*effect) & DROPEFFECT_MOVE) 

			*effect = DROPEFFECT_MOVE;

		else if ((*effect) & DROPEFFECT_LINK) 

			*effect = DROPEFFECT_LINK;

	}
	else
	*effect = DROPEFFECT_NONE;
	return S_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CDropTarget::DragOver (DWORD keyState, POINTL pt, DWORD* effect)
{
	if (gDragContainer && pFrame)
	{
		CDrawContext* context = pFrame->createDrawContext ();
		CPoint where;
		pFrame->getCurrentMouseLocation (where);
		pFrame->onDragMove (gDragContainer, where);
		context->forget ();
		if ((*effect) & DROPEFFECT_COPY) 

			*effect = DROPEFFECT_COPY;

		else if ((*effect) & DROPEFFECT_MOVE) 

			*effect = DROPEFFECT_MOVE;

		else if ((*effect) & DROPEFFECT_LINK) 

			*effect = DROPEFFECT_LINK;

	}
	return S_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CDropTarget::DragLeave (void)
{
	if (gDragContainer && pFrame)
	{
		CDrawContext* context = pFrame->createDrawContext ();
		CPoint where;
		pFrame->getCurrentMouseLocation (where);
		pFrame->onDragLeave (gDragContainer, where);
		context->forget ();
		gDragContainer->forget ();
		gDragContainer = 0;
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CDropTarget::Drop (IDataObject* dataObject, DWORD keyState, POINTL pt, DWORD* effect)
{
	if (gDragContainer && pFrame)
	{
		CDrawContext* context = pFrame->createDrawContext ();
		CPoint where;
		pFrame->getCurrentMouseLocation (where);
		pFrame->onDrop (gDragContainer, where);
		context->forget ();
		gDragContainer->forget ();
		gDragContainer = 0;
	}
	return S_OK;
}
END_NAMESPACE_VSTGUI

#elif MAC_CARBON

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
static CPoint GetMacDragMouse (CFrame* frame)
{
	HIViewRef view = (HIViewRef)frame->getPlatformControl ();
	CPoint where;
	Point r;
	if (GetDragMouse ((DragRef)gDragContainer->getPlatformDrag (), NULL, &r) == noErr)
	{
		HIPoint location;
		#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
		#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_4
		if (HIPointConvert == 0)
		{
			WindowRef window = (WindowRef)frame->getSystemWindow ();
			QDGlobalToLocalPoint (GetWindowPort (window), &r);
			location = CGPointMake ((CGFloat)r.h, (CGFloat)r.v);
			HIViewRef fromView = NULL;
			HIViewFindByID (HIViewGetRoot (window), kHIViewWindowContentID, &fromView);
			HIViewConvertPoint (&location, fromView, view);
		}
		else
		#endif
		{
			location = CGPointMake ((CGFloat)r.h, (CGFloat)r.v);
			HIPointConvert (&location, kHICoordSpaceScreenPixel, NULL, kHICoordSpaceView, view);
		}
		where.x = (CCoord)location.x;
		where.y = (CCoord)location.y;
		#else
		QDGlobalToLocalPoint (GetWindowPort (window), &r);
		where.x = (CCoord)r.h;
		where.y = (CCoord)r.v;
		#endif
	}
	return where;
}

#if MAC_OLD_DRAG
//-----------------------------------------------------------------------------
// Drop Implementation
//-----------------------------------------------------------------------------
pascal static short drag_receiver (WindowPtr w, void* ref, DragReference drag);
pascal static OSErr drag_tracker (DragTrackingMessage message, WindowRef theWindow, void* handlerRefCon, DragRef theDrag);

static DragReceiveHandlerUPP drh;
static DragTrackingHandlerUPP dth;

static bool gEventDragWorks = false;

//-------------------------------------------------------------------------------------------
void install_drop (CFrame* frame)
{
	drh = NewDragReceiveHandlerUPP (drag_receiver);
	dth = NewDragTrackingHandlerUPP (drag_tracker);

	InstallReceiveHandler (drh, (WindowRef)(frame->getSystemWindow ()), (void*)frame);
	InstallTrackingHandler (dth, (WindowRef)(frame->getSystemWindow ()), (void*)frame);
}

//-------------------------------------------------------------------------------------------
void remove_drop (CFrame* frame)
{
	RemoveReceiveHandler (drh, (WindowRef)(frame->getSystemWindow ()));
	RemoveTrackingHandler (dth, (WindowRef)(frame->getSystemWindow ()));

	DisposeDragReceiveHandlerUPP (drh);
	DisposeDragTrackingHandlerUPP (dth);
}

// drag tracking for visual feedback
pascal OSErr drag_tracker (DragTrackingMessage message, WindowRef theWindow, void* handlerRefCon, DragRef dragRef)
{
	if (gEventDragWorks)
		return noErr;

	CFrame* frame = (CFrame*)handlerRefCon;
	switch (message)
	{
		case kDragTrackingEnterWindow:
		{
			if (gDragContainer)
				gDragContainer->forget ();
			gDragContainer = new MacDragContainer (dragRef);

			CPoint where = GetMacDragMouse (frame);
			frame->setCursor (kCursorNotAllowed);
			frame->onDragEnter (gDragContainer, where);
			break;
		}
		case kDragTrackingLeaveWindow:
		{
			CPoint where = GetMacDragMouse (frame);
			frame->onDragLeave (gDragContainer, where);
			frame->setCursor (kCursorDefault);
			gDragContainer->forget ();
			gDragContainer = NULL;
			break;
		}
		case kDragTrackingInWindow:
		{
			CPoint where = GetMacDragMouse (frame);
			frame->onDragMove (gDragContainer, where);

			break;
		}
	}
	return noErr;
}

//-------------------------------------------------------------------------------------------
// Drop has happened in one of our's windows.
// The data is either of our own type (flavour type stCA), or comes from
// another app. The only data from outside that is currently accepted are
// HFS-files
//-------------------------------------------------------------------------------------------
pascal short drag_receiver (WindowPtr w, void* ref, DragReference drag)
{
	if (gEventDragWorks)
		return noErr;

	if (!gDragContainer)
		return noErr;
	
	CFrame* frame = (CFrame*) ref;
	
	CPoint where = GetMacDragMouse (frame);
	frame->onDrop (gDragContainer, where);
	frame->setCursor (kCursorDefault);

	gDragContainer->forget ();
	gDragContainer = NULL;
	return noErr;
}
#endif // MAC_OLD_DRAG

//------------------------------------------------------------------------------
static short keyTable[] = {
	VKEY_BACK,		0x33, 
	VKEY_TAB,		0x30, 
	VKEY_RETURN,	0x24, 
	VKEY_PAUSE,		0x71, 
	VKEY_ESCAPE,	0x35, 
	VKEY_SPACE,		0x31, 

	VKEY_END,		0x77, 
	VKEY_HOME,		0x73, 

	VKEY_LEFT,		0x7B, 
	VKEY_UP,		0x7E, 
	VKEY_RIGHT,		0x7C, 
	VKEY_DOWN,		0x7D, 
	VKEY_PAGEUP,	0x74, 
	VKEY_PAGEDOWN,	0x79, 

	VKEY_PRINT,		0x69, 			
	VKEY_ENTER,		0x4C, 
	VKEY_HELP,		0x72, 
	VKEY_DELETE,	0x75, 
	VKEY_NUMPAD0,	0x52, 
	VKEY_NUMPAD1,	0x53, 
	VKEY_NUMPAD2,	0x54, 
	VKEY_NUMPAD3,	0x55, 
	VKEY_NUMPAD4,	0x56, 
	VKEY_NUMPAD5,	0x57, 
	VKEY_NUMPAD6,	0x58, 
	VKEY_NUMPAD7,	0x59, 
	VKEY_NUMPAD8,	0x5B, 
	VKEY_NUMPAD9,	0x5C, 
	VKEY_MULTIPLY,	0x43, 
	VKEY_ADD,		0x45, 
	VKEY_SUBTRACT,	0x4E, 
	VKEY_DECIMAL,	0x41, 
	VKEY_DIVIDE,	0x4B, 
	VKEY_F1,		0x7A, 
	VKEY_F2,		0x78, 
	VKEY_F3,		0x63, 
	VKEY_F4,		0x76, 
	VKEY_F5,		0x60, 
	VKEY_F6,		0x61, 
	VKEY_F7,		0x62, 
	VKEY_F8,		0x64, 
	VKEY_F9,		0x65, 
	VKEY_F10,		0x6D, 
	VKEY_F11,		0x67, 
	VKEY_F12,		0x6F, 
	VKEY_NUMLOCK,	0x47, 
	VKEY_EQUALS,	0x51
};

/// \cond ignore
class VSTGUIDrawRectsHelper
{
public:
	VSTGUIDrawRectsHelper (CFrame* inFrame, CDrawContext* inContext, bool inIsComposited) : frame (inFrame), context (inContext), isComposited (inIsComposited) {}
	
	CFrame* frame;
	CDrawContext* context;
	bool isComposited;
};

static OSStatus VSTGUIDrawRectsProc (UInt16 message, RgnHandle rgn, const Rect* rect, void* refCon)
{
	if (message == kQDRegionToRectsMsgParse)
	{
		VSTGUIDrawRectsHelper* h = (VSTGUIDrawRectsHelper*)refCon;
		CRect r;
		Rect2CRect ((Rect&)*rect, r);
		if (!h->isComposited)
			r.offset (-h->context->offsetScreen.x, -h->context->offsetScreen.y);
		h->frame->drawRect (h->context, r);
	}
	return noErr;
}
/// \endcond

#ifndef kHIViewFeatureGetsFocusOnClick
#define   kHIViewFeatureGetsFocusOnClick (1 << 8)
#endif

bool hiToolboxAllowFocusChange = true;

//---------------------------------------------------------------------------------------
pascal OSStatus CFrame::carbonEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData)
{
	OSStatus result = eventNotHandledErr;
	CFrame* frame = (CFrame*)inUserData;
	UInt32 eventClass = GetEventClass (inEvent);
	UInt32 eventKind = GetEventKind (inEvent);
	WindowRef window = (WindowRef)frame->getSystemWindow ();

	// WARNING :
	// I've not implemented the old style resource file handling.
	// Use the CFBundleCopyResourceURL... functions to get your resources.

	switch (eventClass)
	{
		case kEventClassScrollable:
		{
			switch (eventKind)
			{
				case kEventScrollableGetInfo:
				{
					HISize cs = {frame->getWidth (), frame->getHeight ()};
					SetEventParameter (inEvent, kEventParamImageSize, typeHISize, sizeof (HISize), &cs);
					HIPoint origin = {frame->hiScrollOffset.x, frame->hiScrollOffset.y};
					SetEventParameter (inEvent, kEventParamOrigin, typeHIPoint, sizeof (HIPoint), &origin);
					HISize lineSize = {50.0, 20.0};
					SetEventParameter(inEvent, kEventParamLineSize, typeHISize, sizeof(lineSize), &lineSize);
					HIRect bounds;
					HIViewGetBounds ((HIViewRef)frame->controlRef, &bounds);
					SetEventParameter(inEvent, kEventParamViewSize, typeHISize, sizeof(bounds.size), &bounds.size);
					result = noErr;
					break;
				}
				case kEventScrollableScrollTo:
				{
					HIPoint where;
					GetEventParameter(inEvent, kEventParamOrigin, typeHIPoint, NULL, sizeof(where), NULL, &where);
					frame->hiScrollOffset.x = (CCoord)where.x;
					frame->hiScrollOffset.y = (CCoord)where.y;
					HIViewSetBoundsOrigin((HIViewRef)frame->controlRef, where.x, where.y);
					HIViewSetNeedsDisplay((HIViewRef)frame->controlRef, true);
					result = noErr;
					break;
				}
			}
			break;
		}
		case kEventClassControl:
		{
			switch (eventKind)
			{
				case kEventControlInitialize:
				{
					UInt32 controlFeatures = kControlSupportsDragAndDrop | kControlSupportsFocus | kControlHandlesTracking | kControlSupportsEmbedding | kHIViewFeatureGetsFocusOnClick | kHIViewIsOpaque | kHIViewGetsFocusOnClick;
					SetEventParameter (inEvent, kEventParamControlFeatures, typeUInt32, sizeof (UInt32), &controlFeatures);
					result = noErr;
					break;
				}
				case kEventControlDraw:
				{
					CDrawContext* context = 0;
					CRect dirtyRect = frame->getViewSize ();
					if (frame->pFrameContext)
					{
						context = frame->pFrameContext;
						context->remember ();
						#if DEBUG
						DebugPrint ("This should not happen anymore\n");
						#endif
					}
					else
					{
						CGContextRef cgcontext = 0;
						OSStatus res = GetEventParameter (inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof (cgcontext), NULL, &cgcontext);
						context = new CDrawContext (frame, (res == noErr) ? cgcontext : NULL, window);
					}
					RgnHandle dirtyRegion;
					if (GetEventParameter (inEvent, kEventParamRgnHandle, typeQDRgnHandle, NULL, sizeof (RgnHandle), NULL, &dirtyRegion) == noErr)
					{
						VSTGUIDrawRectsHelper helper (frame, context, isWindowComposited (window));
						RegionToRectsUPP upp = NewRegionToRectsUPP (VSTGUIDrawRectsProc);
						QDRegionToRects (dirtyRegion, kQDParseRegionFromTopLeft, upp, &helper);
						DisposeRegionToRectsUPP (upp);
					}
					else
						frame->drawRect (context, dirtyRect);
					context->forget ();
					result = noErr;
					break;
				}
				case kEventControlGetClickActivation:
				{
					ClickActivationResult activation = kActivateAndHandleClick;
					SetEventParameter (inEvent, kEventParamClickActivation, typeClickActivationResult, sizeof (ClickActivationResult), &activation);
					result = noErr;
					break;
				}
				case kEventControlHitTest:
				{
					ControlPartCode code = kControlContentMetaPart;
					SetEventParameter (inEvent, kEventParamControlPart, typeControlPartCode, sizeof (ControlPartCode), &code);
					result = noErr;
					break;
				}
				case kEventControlClick:
				{
					return noErr;
					EventMouseButton buttonState;
					GetEventParameter (inEvent, kEventParamMouseButton, typeMouseButton, NULL, sizeof (EventMouseButton), NULL, &buttonState);
					if (buttonState == kEventMouseButtonPrimary)
					{
						result = CallNextEventHandler (inHandlerCallRef, inEvent);
						break;
					}
				}
				case kEventControlTrack:
				case kEventControlContextualMenuClick:
				{
					break;
				}
				case kEventControlGetOptimalBounds:
				{
					HIRect optimalBounds = { {0, 0}, { frame->getWidth (), frame->getHeight ()}};
					SetEventParameter (inEvent, kEventParamControlOptimalBounds, typeHIRect, sizeof (HIRect), &optimalBounds);
					result = noErr;
					break;
				}
				case kEventControlGetFocusPart:
				{
					if (hiToolboxAllowFocusChange)
					{
						ControlPartCode code = frame->hasFocus ? 127 : kControlFocusNoPart;
						SetEventParameter (inEvent, kEventParamControlPart, typeControlPartCode, sizeof (ControlPartCode), &code);
						result = noErr;
					}
					break;
				}
				case kEventControlSetFocusPart:
				{
					if (hiToolboxAllowFocusChange)
					{
						ControlPartCode code;
						GetEventParameter (inEvent, kEventParamControlPart, typeControlPartCode, NULL, sizeof (ControlPartCode), NULL, &code);
						if (code == kControlFocusNoPart)
						{
							frame->hasFocus = false;
						}
						else
						{
							bool anfResult = false;
							if (code == kControlFocusNextPart)
								anfResult = frame->advanceNextFocusView (frame->pFocusView);
							else if (code == kControlFocusPrevPart)
								anfResult = frame->advanceNextFocusView (frame->pFocusView, true);
							if (anfResult)
							{
								frame->hasFocus = true;
								code = 127;
							}
							else
							{
								frame->hasFocus = false;
								code = kControlFocusNoPart;
							}
							SetEventParameter (inEvent, kEventParamControlPart, typeControlPartCode, sizeof (code), &code);
						}
						result = noErr;
					}
					break;
				}
				case kEventControlDragEnter:
				{
					#if MAC_OLD_DRAG
					gEventDragWorks = true;
					#endif

					DragRef dragRef;
					if (GetEventParameter (inEvent, kEventParamDragRef, typeDragRef, NULL, sizeof (DragRef), NULL, &dragRef) == noErr)
					{
						gDragContainer = new MacDragContainer (dragRef);
						
						CPoint where = GetMacDragMouse (frame);
						frame->setCursor (kCursorNotAllowed);
						frame->onDragEnter (gDragContainer, where);

						Boolean acceptDrop = true;
						SetEventParameter (inEvent, kEventParamControlWouldAcceptDrop, typeBoolean, sizeof (Boolean), &acceptDrop);
					}
					result = noErr;
					break;
				}
				case kEventControlDragWithin:
				{
					if (gDragContainer)
					{
						CPoint where = GetMacDragMouse (frame);
						frame->onDragMove (gDragContainer, where);
					}
					result = noErr;
					break;
				}
				case kEventControlDragLeave:
				{
					if (gDragContainer)
					{
						CPoint where = GetMacDragMouse (frame);
						frame->onDragLeave (gDragContainer, where);
						frame->setCursor (kCursorDefault);
					}
					result = noErr;
					break;
				}
				case kEventControlDragReceive:
				{
					if (gDragContainer)
					{
						CPoint where = GetMacDragMouse (frame);
						frame->onDrop (gDragContainer, where);
						frame->setCursor (kCursorDefault);
						gDragContainer->forget ();
						gDragContainer = 0;
					}
					result = noErr;
					break;
				}
				#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4
				case kEventControlTrackingAreaExited:
				{
					HIPoint location = { 0.f, 0.f };
					if (GetEventParameter (inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof (HIPoint), NULL, &location) == noErr)
					{
						if (!isWindowComposited ((WindowRef)window))
						{
							HIRect viewRect;
							HIViewGetFrame (frame->controlRef, &viewRect);
							location.x -= viewRect.origin.x;
							location.y -= viewRect.origin.y;
						}
						CPoint point ((CCoord)location.x, (CCoord)location.y);
						frame->onMouseExited (point, 0);
					}
					break;
				}
				#endif
			}
			break;
		}
		case kEventClassMouse:
		{
			switch (eventKind)
			{
				case kEventMouseWheelMoved:
				{
					UInt32 modifiers;
					HIPoint windowHIPoint;
					SInt32 wheelDelta;
					EventMouseWheelAxis wheelAxis;
					WindowRef windowRef;
					GetEventParameter (inEvent, kEventParamWindowRef, typeWindowRef, NULL, sizeof (WindowRef), NULL, &windowRef);
					GetEventParameter (inEvent, kEventParamMouseWheelAxis, typeMouseWheelAxis, NULL, sizeof (EventMouseWheelAxis), NULL, &wheelAxis);
					GetEventParameter (inEvent, kEventParamMouseWheelDelta, typeSInt32, NULL, sizeof (SInt32), NULL, &wheelDelta);
					GetEventParameter (inEvent, kEventParamWindowMouseLocation, typeHIPoint, NULL, sizeof (HIPoint), NULL, &windowHIPoint);
					GetEventParameter (inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof (UInt32), NULL, &modifiers);
					long buttons = 0;
					if (modifiers & cmdKey)
						buttons |= kControl;
					if (modifiers & shiftKey)
						buttons |= kShift;
					if (modifiers & optionKey)
						buttons |= kAlt;
					if (modifiers & controlKey)
						buttons |= kApple;
					
					#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
					#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_4
					if (HIPointConvert != NULL)
					#else
					if (true)
					#endif
					{
						HIPointConvert (&windowHIPoint, kHICoordSpaceWindow, windowRef, kHICoordSpaceView, frame->controlRef);
					}
					else
					#endif
					HIViewConvertPoint (&windowHIPoint, HIViewGetRoot (windowRef), frame->controlRef);
					
					// non-compositing window controls need to handle offset themselves
					if (!isWindowComposited (windowRef))
					{
						HIRect viewRect;
						HIViewGetFrame(frame->controlRef, &viewRect);
						windowHIPoint.x -= viewRect.origin.x;
						windowHIPoint.y -= viewRect.origin.y;
					}
					
					CPoint p ((CCoord)windowHIPoint.x, (CCoord)windowHIPoint.y);
					float distance = wheelDelta;
					CMouseWheelAxis axis = kMouseWheelAxisY;
					if (wheelAxis == kEventMouseWheelAxisX)
						axis = kMouseWheelAxisX;
					frame->onWheel (p, axis, distance, buttons);
					result = noErr;
					break;
				}
			}
			break;
		}
		case kEventClassTextInput:
		{
			switch (eventKind)
			{
				case kEventTextInputUnicodeForKeyEvent:
				{
					// The "Standard Event Handler" of a window would return noErr even though no one has handled the key event. 
					// This prevents the "Standard Handler" to be called for this event, with the exception of the tab key as it is used for control focus changes.
					EventRef rawKeyEvent;
					GetEventParameter (inEvent, kEventParamTextInputSendKeyboardEvent, typeEventRef, NULL, sizeof (EventRef), NULL, &rawKeyEvent);
					if (rawKeyEvent)
					{
						UInt32 keyCode = 0;
						GetEventParameter (rawKeyEvent, kEventParamKeyCode, typeUInt32, NULL, sizeof (UInt32), NULL, &keyCode);
						if (keyCode == (UInt32)keyTable[VKEY_TAB+1])
							return result;
					}
					result = eventPassToNextTargetErr;
					break;
				}
			}
			break;
		}
		case kEventClassKeyboard:
		{
			if (frame->hasFocus)
			{
				switch (eventKind)
				{
					case kEventRawKeyDown:
					case kEventRawKeyRepeat:
					{
						// todo: make this work

						char character = 0;
						UInt32 keyCode = 0;
						UInt32 modifiers = 0;
						GetEventParameter (inEvent, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof (char), NULL, &character);
						GetEventParameter (inEvent, kEventParamKeyCode, typeUInt32, NULL, sizeof (UInt32), NULL, &keyCode);
						GetEventParameter (inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof (UInt32), NULL, &modifiers);
						char scanCode = keyCode;
						VstKeyCode vstKeyCode;
						memset (&vstKeyCode, 0, sizeof (VstKeyCode));
						KeyboardLayoutRef layout;
						if (KLGetCurrentKeyboardLayout (&layout) == noErr)
						{
							const void* pKCHR = 0;
							KLGetKeyboardLayoutProperty (layout, kKLKCHRData, &pKCHR);
							if (pKCHR)
							{
								static UInt32 keyTranslateState = 0;
								vstKeyCode.character = KeyTranslate (pKCHR, keyCode, &keyTranslateState);
								if (modifiers & shiftKey)
								{
									vstKeyCode.character = toupper (vstKeyCode.character);
								}
							}
						}
						short entries = sizeof (keyTable) / (sizeof (short));
						for (int i = 0; i < entries; i += 2)
						{
							if (keyTable[i + 1] == scanCode)
							{
								vstKeyCode.virt = keyTable[i];
								vstKeyCode.character = 0;
								break;
							}
						}
						if (modifiers & cmdKey)
							vstKeyCode.modifier |= MODIFIER_CONTROL;
						if (modifiers & shiftKey)
							vstKeyCode.modifier |= MODIFIER_SHIFT;
						if (modifiers & optionKey)
							vstKeyCode.modifier |= MODIFIER_ALTERNATE;
						if (modifiers & controlKey)
							vstKeyCode.modifier |= MODIFIER_COMMAND;
						if (frame->onKeyDown (vstKeyCode) != -1)
							result = noErr;
						
						break;
					}
				}
			}
			break;
		}
	}
	return result;
}

#define ENABLE_LOGGING 0

#if ENABLE_LOGGING
#define LOG_HIPOINT(text,point) fprintf (stdout, "%s%d, %d\n", text, (long)point.x, (long)point.y);
#define LOG(text) fprintf (stdout, "%s\n", text);
#else
#define LOG_HIPOINT(x,y)
#define LOG(x)
#endif

//---------------------------------------------------------------------------------------
pascal OSStatus CFrame::carbonMouseEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData)
{
	OSStatus result = eventNotHandledErr;
	CFrame* frame = (CFrame*)inUserData;
	UInt32 eventClass = GetEventClass (inEvent);
	UInt32 eventKind = GetEventKind (inEvent);
	WindowRef window = (WindowRef)frame->getSystemWindow ();
	HIViewRef hiView = frame->controlRef;

	HIViewRef view;
	if (HIViewGetViewForMouseEvent (HIViewGetRoot (window), inEvent, &view) == noErr)
	{
		if (view != hiView && !((eventKind == kEventMouseDragged || eventKind == kEventMouseUp) && frame->mouseDownView != 0))
			return result;
	}
	switch (eventClass)
	{
		case kEventClassMouse:
		{
			UInt32 modifiers = 0;
			EventMouseButton buttonState = 0;
			long buttons = 0;
			HIPoint location = { 0.f, 0.f };
			if (GetEventParameter (inEvent, kEventParamWindowMouseLocation, typeHIPoint, NULL, sizeof (HIPoint), NULL, &location) == noErr)
			{
				//LOG_HIPOINT("window :",location)
				#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
				#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_4
				if (HIPointConvert != NULL)
				#else
				if (true)
				#endif
				{
					HIPointConvert (&location, kHICoordSpaceWindow, window, kHICoordSpaceView, hiView);
				}
				else
				#endif
					HIViewConvertPoint (&location, HIViewGetRoot (window), hiView);
				//LOG_HIPOINT("view   :",location)
			}
			if (!isWindowComposited ((WindowRef)window))
			{
				HIRect viewRect;
				HIViewGetFrame(hiView, &viewRect);
				location.x -= viewRect.origin.x;
				location.y -= viewRect.origin.y;
			}
			GetEventParameter (inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof (UInt32), NULL, &modifiers);
			GetEventParameter (inEvent, kEventParamMouseButton, typeMouseButton, NULL, sizeof (EventMouseButton), NULL, &buttonState);
			if (buttonState == kEventMouseButtonPrimary)
				buttons |= kLButton;
			if (buttonState == kEventMouseButtonSecondary)
				buttons |= kRButton;
			if (buttonState == kEventMouseButtonTertiary)
				buttons |= kMButton;
			if (buttonState == 4)
				buttons |= kButton4;
			if (buttonState == 5)
				buttons |= kButton5;
			if (modifiers & cmdKey)
				buttons |= kControl;
			if (modifiers & shiftKey)
				buttons |= kShift;
			if (modifiers & optionKey)
				buttons |= kAlt;
			if (modifiers & controlKey)
				buttons |= kApple;
			CPoint point ((CCoord)location.x, (CCoord)location.y);
			switch (eventKind)
			{
				case kEventMouseDown:
				{
					LOG("Mouse Down")
					UInt32 clickCount = 0;
					GetEventParameter (inEvent, kEventParamClickCount, typeUInt32, NULL, sizeof (UInt32), NULL, &clickCount);
					if (clickCount > 1)
						buttons |= kDoubleClick;
					result = CallNextEventHandler (inHandlerCallRef, inEvent); // calls default handler, which activates the window if not already active, or sets the process to front
					#if 0
					WindowClass windowClass;
					if (GetWindowClass (window, &windowClass) == noErr)
					{
						if (windowClass == kDocumentWindowClass)
						{
							HIViewAdvanceFocus ((HIViewRef)frame->getPlatformControl (), 0);
						}
					}
					#endif
					if (frame->onMouseDown (point, buttons))
						result = noErr;
					break;
				}
				case kEventMouseUp:
				{
					LOG("Mouse Up")
					if (frame->onMouseUp (point, buttons))
						result = noErr;
					break;
				}
				case kEventMouseDragged:
				{
					//LOG("Mouse Dragged")
					if (frame->onMouseMoved (point, buttons))
						result = noErr;
					break;
				}
				case kEventMouseMoved:
				{
					//LOG("Mouse Moved")
					if (IsWindowActive (window))
					{
						if (frame->onMouseMoved (point, buttons))
							result = noErr;
					}
					break;
				}
			}
			break;
		}
	}
	return result;
}

/// \cond ignore

//-----------------------------------------------------------------------------
bool isWindowComposited (WindowRef window)
{
	WindowAttributes attr;
	GetWindowAttributes (window, &attr);
	if (attr & kWindowCompositingAttribute)
		return true;
	return false;
}

/// \endcond
END_NAMESPACE_VSTGUI

#endif // MAC_CARBON



