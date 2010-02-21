//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
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

#include "win32frame.h"

#if WINDOWS

#include <commctrl.h>
#include <cmath>
#include "gdiplusdrawcontext.h"
#include "gdiplusbitmap.h"
#include "gdiplusgraphicspath.h"
#include "win32textedit.h"
#include "win32optionmenu.h"
#include "win32support.h"
#include "win32dragcontainer.h"
#include "../../cdropsource.h"

namespace VSTGUI {

#define DEBUG_DRAWING	0

//-----------------------------------------------------------------------------
static TCHAR gClassName[100];
static bool bSwapped_mouse_buttons = false; 
static OSVERSIONINFOEX gSystemVersion;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CDropTarget : public IDropTarget
{	
public:
	CDropTarget (Win32Frame* pFrame);
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
	Win32Frame* pFrame;
	WinDragContainer* gDragContainer;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class Win32DropSource : public CBaseObject, public ::IDropSource
{
public:
	Win32DropSource () {}

	// IUnknown
	STDMETHOD (QueryInterface) (REFIID riid, void** object);
	STDMETHOD_ (ULONG, AddRef) (void) { remember (); return getNbReference ();}
	STDMETHOD_ (ULONG, Release) (void) { ULONG refCount = getNbReference () - 1; forget (); return refCount; }
	
	// IDropSource
	STDMETHOD (QueryContinueDrag) (BOOL escapePressed, DWORD keyState);
	STDMETHOD (GiveFeedback) (DWORD effect);
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class Win32DataObject : public CBaseObject, public ::IDataObject
{
public:
	Win32DataObject (CDropSource* dropSource);
	~Win32DataObject ();

	// IUnknown
	STDMETHOD (QueryInterface) (REFIID riid, void** object);
	STDMETHOD_ (ULONG, AddRef) (void) { remember (); return getNbReference ();}
	STDMETHOD_ (ULONG, Release) (void) { ULONG refCount = getNbReference () - 1; forget (); return refCount; }

	// IDataObject
	STDMETHOD (GetData) (FORMATETC *format, STGMEDIUM *medium);
	STDMETHOD (GetDataHere) (FORMATETC *format, STGMEDIUM *medium);
	STDMETHOD (QueryGetData) (FORMATETC *format);
	STDMETHOD (GetCanonicalFormatEtc) (FORMATETC *formatIn, FORMATETC *formatOut);
	STDMETHOD (SetData) (FORMATETC *format, STGMEDIUM *medium, BOOL release);
	STDMETHOD (EnumFormatEtc) (DWORD direction, IEnumFORMATETC** enumFormat);
	STDMETHOD (DAdvise) (FORMATETC* format, DWORD advf, IAdviseSink* advSink, DWORD* connection);
	STDMETHOD (DUnadvise) (DWORD connection);
	STDMETHOD (EnumDAdvise) (IEnumSTATDATA** enumAdvise);
private:
	CDropSource* dropSource;
};

//-----------------------------------------------------------------------------
IPlatformFrame* IPlatformFrame::createPlatformFrame (IPlatformFrameCallback* frame, const CRect& size, void* parent)
{
	return new Win32Frame (frame, size, (HWND)parent);
}

//-----------------------------------------------------------------------------
Win32Frame::Win32Frame (IPlatformFrameCallback* frame, const CRect& size, HWND parent)
: IPlatformFrame (frame)
, windowHandle (0)
, parentWindow (parent)
, tooltipWindow (0)
, backBuffer (0)
, mouseInside (false)
{
	initWindowClass ();

	DWORD style = WS_EX_TRANSPARENT;
	#if !DEBUG_DRAWING
	if (gSystemVersion.dwMajorVersion >= 6) // Vista and above
		style |= WS_EX_COMPOSITED;
	else
		backBuffer = createOffscreenContext (size.getWidth (), size.getHeight ());
	#endif
	windowHandle = CreateWindowEx (style, gClassName, TEXT("Window"),
									WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 
									0, 0, (int)size.width (), (int)size.height (), 
									parentWindow, NULL, GetInstance (), NULL);

	if (windowHandle)
	{
		SetWindowLongPtr (windowHandle, GWLP_USERDATA, (__int3264)(LONG_PTR)this);
		RegisterDragDrop (windowHandle, new CDropTarget (this));
	}
}

//-----------------------------------------------------------------------------
Win32Frame::~Win32Frame ()
{
	if (tooltipWindow)
		DestroyWindow (tooltipWindow);
	if (windowHandle)
	{
		RevokeDragDrop (windowHandle);
		SetWindowLongPtr (windowHandle, GWLP_USERDATA, (LONG_PTR)NULL);
		DestroyWindow (windowHandle);
	}
	if (backBuffer)
		backBuffer->forget ();
	destroyWindowClass ();
}

//-----------------------------------------------------------------------------
long Win32Frame::gUseCount = 0;

//-----------------------------------------------------------------------------
void Win32Frame::initWindowClass ()
{
	gUseCount++;
	if (gUseCount == 1)
	{
		OleInitialize (0);

		// get OS version
		memset (&gSystemVersion, 0, sizeof (gSystemVersion));
		gSystemVersion.dwOSVersionInfoSize = sizeof (gSystemVersion);
		GetVersionEx ((OSVERSIONINFO *)&gSystemVersion);

		VSTGUI_SPRINTF (gClassName, TEXT("VSTGUI%p"), GetInstance ());
		
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

		bSwapped_mouse_buttons = GetSystemMetrics (SM_SWAPBUTTON) > 0;

		GDIPlusGlobals::enter ();
	}
}

//-----------------------------------------------------------------------------
void Win32Frame::destroyWindowClass ()
{
	gUseCount--;
	if (gUseCount == 0)
	{
		GDIPlusGlobals::exit ();

		UnregisterClass (gClassName, GetInstance ());
		OleUninitialize ();
	}
}

//-----------------------------------------------------------------------------
void Win32Frame::initTooltip ()
{
	if (tooltipWindow == 0 && windowHandle)
	{
		TOOLINFO    ti;
		// Create the ToolTip control.
		HWND hwndTT = CreateWindow (TOOLTIPS_CLASS, TEXT(""),
							  WS_POPUP,
							  CW_USEDEFAULT, CW_USEDEFAULT,
							  CW_USEDEFAULT, CW_USEDEFAULT,
							  NULL, (HMENU)NULL, GetInstance (),
							  NULL);

		// Prepare TOOLINFO structure for use as tracking ToolTip.
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd   = (HWND)windowHandle;
		ti.uId    = (UINT)0;
		ti.hinst  = GetInstance ();
		ti.lpszText  = TEXT("This is a tooltip");
		ti.rect.left = ti.rect.top = ti.rect.bottom = ti.rect.right = 0;

		// Add the tool to the control
		if (!SendMessage (hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti))
		{
			DestroyWindow (hwndTT);
			return;
		}

		tooltipWindow = hwndTT;
	}
}

//-----------------------------------------------------------------------------
HWND Win32Frame::getOuterWindow () const
{
	int diffWidth, diffHeight;
	RECT  rctTempWnd, rctPluginWnd;
	HWND  hTempWnd = windowHandle;
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

// IPlatformFrame
//-----------------------------------------------------------------------------
bool Win32Frame::getGlobalPosition (CPoint& pos) const
{
	RECT r;
	GetWindowRect (windowHandle, &r);
	pos.x = r.left;
	pos.y = r.top;
	return true;
	HWND wnd = getOuterWindow ();
	HWND wndParent = GetParent (wnd);

	RECT  rctTempWnd;
	GetWindowRect (wnd, &rctTempWnd);

	POINT point;
	point.x = rctTempWnd.left;
	point.y = rctTempWnd.top;

	MapWindowPoints (HWND_DESKTOP, wndParent, &point, 1);
	
	pos.x = (CCoord)point.x;
	pos.y = (CCoord)point.y;
	return true;
}

//-----------------------------------------------------------------------------
bool Win32Frame::setSize (const CRect& newSize)
{
	if (backBuffer)
	{
		backBuffer->forget ();
		backBuffer = createOffscreenContext (newSize.getWidth (), newSize.getHeight ());
	}
	// TODO for VST2: we only set the size of the window we own. In VST2 this was not the case, we also resized the parent window. This must be done upstream now.
	SetWindowPos (windowHandle, HWND_TOP, (int)newSize.left, (int)newSize.top, (int)newSize.getWidth (), (int)newSize.getHeight (), SWP_NOMOVE|SWP_NOCOPYBITS|SWP_NOREDRAW|SWP_DEFERERASE);
	invalidRect (newSize);
	return true;
}

//-----------------------------------------------------------------------------
bool Win32Frame::getSize (CRect& size) const
{
	RECT r;
	GetWindowRect (windowHandle, &r);
	POINT p;
	p.x = r.left;
	p.y = r.top;
	MapWindowPoints (HWND_DESKTOP, windowHandle, &p, 1);
	size.left = p.x;
	size.top = p.y;
	size.right = p.x + (r.right - r.left);
	size.bottom = p.y + (r.bottom - r.top);
	return true;

#if 0 // old code, returned other values, why ?
	// return the size relative to the client rect of this window
	// get the main window
	HWND wnd = GetParent (windowHandle);
	HWND wndParent = GetParent (wnd);
	HWND wndParentParent = GetParent (wndParent);

	RECT  rctTempWnd;
	GetWindowRect (wnd, &rctTempWnd);
	
	POINT point;
	point.x = rctTempWnd.left;
	point.y = rctTempWnd.top;

	MapWindowPoints (HWND_DESKTOP, wndParentParent, &point, 1);
	
	size.left   = (CCoord)point.x;
	size.top    = (CCoord)point.y;
	size.right  = (CCoord)size.left + rctTempWnd.right - rctTempWnd.left;
	size.bottom = (CCoord)size.top  + rctTempWnd.bottom - rctTempWnd.top;
	return true;
#endif
}

//-----------------------------------------------------------------------------
bool Win32Frame::getCurrentMousePosition (CPoint& mousePosition) const
{
	HWND hwnd = windowHandle;
	POINT _where;
	GetCursorPos (&_where);
	mousePosition ((CCoord)_where.x, (CCoord)_where.y);
	if (hwnd)
	{
		RECT rctTempWnd;
		GetWindowRect (hwnd, &rctTempWnd);
		mousePosition.offset ((CCoord)-rctTempWnd.left, (CCoord)-rctTempWnd.top);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool Win32Frame::getCurrentMouseButtons (long& buttons) const
{
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
	return true;
}

//-----------------------------------------------------------------------------
bool Win32Frame::setMouseCursor (CCursorType type)
{
	static HCURSOR defaultCursor = 0;
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
	return true;
}

//-----------------------------------------------------------------------------
bool Win32Frame::invalidRect (const CRect& rect)
{
	RECT r = {(LONG)rect.left, (LONG)rect.top, (LONG)ceil (rect.right), (LONG)ceil (rect.bottom)};
	InvalidateRect (windowHandle, &r, true);
	return true;
}

//-----------------------------------------------------------------------------
bool Win32Frame::scrollRect (const CRect& src, const CPoint& distance)
{
	return false;
}

//-----------------------------------------------------------------------------
unsigned long IPlatformFrame::getTicks ()
{
	return (unsigned long)GetTickCount ();
}

//-----------------------------------------------------------------------------
bool Win32Frame::showTooltip (const CRect& rect, const char* utf8Text)
{
	initTooltip ();
	if (tooltipWindow)
	{
		UTF8StringHelper tooltipText (utf8Text);
		RECT rc;
		rc.left = (LONG)rect.left;
		rc.top = (LONG)rect.top;
		rc.right = (LONG)rect.right;
		rc.bottom = (LONG)rect.bottom;
		TOOLINFO ti = {0};
		ti.cbSize = sizeof(TOOLINFO);
		ti.hwnd = windowHandle;
		ti.uId = 0;
		ti.rect = rc;
		ti.lpszText = (TCHAR*)(const TCHAR*)tooltipText;
		SendMessage (tooltipWindow, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
		SendMessage (tooltipWindow, TTM_NEWTOOLRECT, 0, (LPARAM)&ti);
		SendMessage (tooltipWindow, TTM_POPUP, 0, 0);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool Win32Frame::hideTooltip ()
{
	if (tooltipWindow)
	{
		TOOLINFO ti = {0};
		ti.cbSize = sizeof(TOOLINFO);
		ti.hwnd = windowHandle;
		ti.uId = 0;
		ti.lpszText = 0;
		SendMessage (tooltipWindow, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
		SendMessage (tooltipWindow, TTM_POP, 0, 0);
	}
	return false;
}

//-----------------------------------------------------------------------------
IPlatformTextEdit* Win32Frame::createPlatformTextEdit (IPlatformTextEditCallback* textEdit)
{
	return new Win32TextEdit (windowHandle, textEdit);
}

//-----------------------------------------------------------------------------
IPlatformOptionMenu* Win32Frame::createPlatformOptionMenu ()
{
	return new Win32OptionMenu (windowHandle);
}

//-----------------------------------------------------------------------------
COffscreenContext* Win32Frame::createOffscreenContext (CCoord width, CCoord height)
{
	GdiplusBitmap* bitmap = new GdiplusBitmap (CPoint (width, height));
	GdiplusDrawContext* context = new GdiplusDrawContext (bitmap);
	bitmap->forget ();
	return context;
}

//-----------------------------------------------------------------------------
CGraphicsPath* Win32Frame::createGraphicsPath ()
{
	return new GdiplusGraphicsPath ();
}

//------------------------------------------------------------------------------------
long Win32Frame::doDrag (CDropSource* source, const CPoint& offset, CBitmap* dragBitmap)
{
	// TODO: implement doDrag for Win32
	long result = 0;
	Win32DataObject* dataObject = new Win32DataObject (source);
	Win32DropSource* dropSource = new Win32DropSource;
	DWORD outEffect;
	HRESULT hResult = DoDragDrop (dataObject, dropSource, DROPEFFECT_COPY, &outEffect);
	dataObject->Release ();
	dropSource->Release ();
	if (hResult == DRAGDROP_S_DROP)
	{
		if (outEffect == DROPEFFECT_MOVE)
			result = -1;
		else
			result = 1;
	}
	return result;
}

//-----------------------------------------------------------------------------
LONG_PTR WINAPI Win32Frame::WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Win32Frame* win32Frame = (Win32Frame*)(LONG_PTR)GetWindowLongPtr (hwnd, GWLP_USERDATA);
	if (win32Frame)
	{
		IPlatformFrameCallback* pFrame = win32Frame->getFrame ();
		bool doubleClick = false;

		switch (message)
		{
			case WM_MOUSEWHEEL:
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
				pFrame->platformOnMouseWheel (where, kMouseWheelAxisY, (float)(zDelta / WHEEL_DELTA), buttons);
				break;
			}
			case WM_CTLCOLOREDIT:
			{
				Win32TextEdit* win32TextEdit = (Win32TextEdit*)(LONG_PTR) GetWindowLongPtr ((HWND)lParam, GWLP_USERDATA);
				if (win32TextEdit)
				{
					CColor fontColor = win32TextEdit->getTextEdit ()->platformGetFontColor ();
					SetTextColor ((HDC) wParam, RGB (fontColor.red, fontColor.green, fontColor.blue));
					#if 1 // TODO: I don't know why the transparent part does not work anymore. Needs more investigation.
					CColor backColor = win32TextEdit->getTextEdit ()->platformGetBackColor ();
					SetBkColor ((HDC) wParam, RGB (backColor.red, backColor.green, backColor.blue));
					return (LRESULT)(win32TextEdit->getPlatformBackColor ());

					#else
					SetBkMode ((HDC)wParam, TRANSPARENT);
					return (LRESULT) ::GetStockObject (HOLLOW_BRUSH);

					#endif
				}
				break;
			}

			case WM_PAINT:
			{
				HRGN rgn = CreateRectRgn (0, 0, 0, 0);
				if (GetUpdateRgn (hwnd, rgn, false) == NULLREGION)
				{
					DeleteObject (rgn);
					return 0;
				}

				PAINTSTRUCT ps;
				HDC hdc = BeginPaint (hwnd, &ps);

				#if 1
				if (hdc)
				{
					CRect frameSize;
					win32Frame->getSize (frameSize);
					frameSize.offset (-frameSize.left, -frameSize.top);
					GdiplusDrawContext context (hdc, frameSize);
					CRect updateRect ((CCoord)ps.rcPaint.left, (CCoord)ps.rcPaint.top, (CCoord)ps.rcPaint.right, (CCoord)ps.rcPaint.bottom);
					if (win32Frame->backBuffer)
					{
						pFrame->platformDrawRect (win32Frame->backBuffer, updateRect);
						win32Frame->backBuffer->copyFrom (&context, updateRect, CPoint (updateRect.left, updateRect.top));
					}
					else
						pFrame->platformDrawRect (&context, updateRect);
				}

				#else

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
					pFrame->platformDrawRect (context, updateRect);

				if (pFrame->getBackBuffer ())
				{
					CDrawContext localContext (pFrame, hdc, hwnd);
					pFrame->getBackBuffer ()->copyFrom (&localContext, updateRect, CPoint ((CCoord)ps.rcPaint.left, (CCoord)ps.rcPaint.top));
				}
				else
					context->forget ();

				#endif
				
				EndPaint (hwnd, &ps);
				DeleteObject (rgn);
				return 0;
			}

			case WM_RBUTTONDBLCLK:
			case WM_MBUTTONDBLCLK:
			case WM_LBUTTONDBLCLK:
			case WM_XBUTTONDBLCLK:
				doubleClick = true;
			case WM_RBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_LBUTTONDOWN:
			case WM_XBUTTONDOWN:
			{
				long buttons = 0;
				if (wParam & MK_LBUTTON)
					buttons |= kLButton;
				if (wParam & MK_RBUTTON)
					buttons |= kRButton;
				if (wParam & MK_MBUTTON)
					buttons |= kMButton;
				if (wParam & MK_XBUTTON1)
					buttons |= kButton4;
				if (wParam & MK_XBUTTON2)
					buttons |= kButton5;
				if (wParam & MK_CONTROL)
					buttons |= kControl;
				if (wParam & MK_SHIFT)
					buttons |= kShift;
				if (GetKeyState (VK_MENU)    < 0)
					buttons |= kAlt;
				if (doubleClick)
					buttons |= kDoubleClick;
				SetFocus (win32Frame->getPlatformWindow ());
				CPoint where ((CCoord)((int)(short)LOWORD(lParam)), (CCoord)((int)(short)HIWORD(lParam)));
				if (pFrame->platformOnMouseDown (where, buttons) == kMouseEventHandled)
					SetCapture (win32Frame->getPlatformWindow ());
				return 0;
			}
			case WM_MOUSELEAVE:
			{
				CPoint where;
				win32Frame->getCurrentMousePosition (where);
				long buttons;
				win32Frame->getCurrentMouseButtons (buttons);
				pFrame->platformOnMouseExited (where, buttons);
				win32Frame->mouseInside = false;
				return 0;
			}
			case WM_MOUSEMOVE:
			{
				long buttons = 0;
				if (wParam & MK_LBUTTON)
					buttons |= kLButton;
				if (wParam & MK_RBUTTON)
					buttons |= kRButton;
				if (wParam & MK_MBUTTON)
					buttons |= kMButton;
				if (wParam & MK_XBUTTON1)
					buttons |= kButton4;
				if (wParam & MK_XBUTTON2)
					buttons |= kButton5;
				if (wParam & MK_CONTROL)
					buttons |= kControl;
				if (wParam & MK_SHIFT)
					buttons |= kShift;
				if (GetKeyState (VK_MENU) < 0)
					buttons |= kAlt;
				if (!win32Frame->mouseInside)
				{
					// this makes sure that WM_MOUSELEAVE will be generated by the system
					win32Frame->mouseInside = true;
					TRACKMOUSEEVENT tme = {0};
					tme.cbSize = sizeof (tme);
					tme.dwFlags = TME_LEAVE;
					tme.hwndTrack = win32Frame->windowHandle;
					TrackMouseEvent (&tme);
				}
				CPoint where ((CCoord)((int)(short)LOWORD(lParam)), (CCoord)((int)(short)HIWORD(lParam)));
				pFrame->platformOnMouseMoved (where, buttons);
				return 0;
			}
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_MBUTTONUP:
			case WM_XBUTTONUP:
			{
				long buttons = 0;
				if (wParam & MK_LBUTTON)
					buttons |= kLButton;
				if (wParam & MK_RBUTTON)
					buttons |= kRButton;
				if (wParam & MK_MBUTTON)
					buttons |= kMButton;
				if (wParam & MK_XBUTTON1)
					buttons |= kButton4;
				if (wParam & MK_XBUTTON2)
					buttons |= kButton5;
				if (wParam & MK_CONTROL)
					buttons |= kControl;
				if (wParam & MK_SHIFT)
					buttons |= kShift;
				if (GetKeyState (VK_MENU) < 0)
					buttons |= kAlt;
				CPoint where ((CCoord)((int)(short)LOWORD(lParam)), (CCoord)((int)(short)HIWORD(lParam)));
				pFrame->platformOnMouseUp (where, buttons);
				ReleaseCapture ();
				return 0;
			}
			case WM_DESTROY:
			{
				#if DEBUG
				DebugPrint ("This should never get called. We are the owner of the window and we are responsible of destroying it.\n");
				#endif
				win32Frame->windowHandle = 0;
				break;
			}
			case WM_ERASEBKGND: return 1; // don't draw background
		}
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}

//-----------------------------------------------------------------------------
CDropTarget::CDropTarget (Win32Frame* pFrame)
: refCount (0)
, pFrame (pFrame)
, gDragContainer (0)
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
		CPoint where;
		pFrame->getCurrentMousePosition (where);
		pFrame->getFrame ()->platformOnDragEnter (gDragContainer, where);
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
		CPoint where;
		pFrame->getCurrentMousePosition (where);
		pFrame->getFrame ()->platformOnDragMove (gDragContainer, where);
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
		CPoint where;
		pFrame->getCurrentMousePosition (where);
		pFrame->getFrame ()->platformOnDragLeave (gDragContainer, where);
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
		CPoint where;
		pFrame->getCurrentMousePosition (where);
		pFrame->getFrame ()->platformOnDrop (gDragContainer, where);
		gDragContainer->forget ();
		gDragContainer = 0;
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
// Win32DropSource
//-----------------------------------------------------------------------------
STDMETHODIMP Win32DropSource::QueryInterface (REFIID riid, void** object)
{
	if (riid == ::IID_IDropSource)                        
	{                                                              
		AddRef ();                                                 
		*object = (::IDropSource*)this;                               
		return S_OK;                                          
	}
	else if (riid == ::IID_IUnknown)                        
	{                                                              
		AddRef ();                                                 
		*object = (::IUnknown*)this;                               
		return S_OK;                                          
	}
	return E_NOINTERFACE;
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DropSource::QueryContinueDrag (BOOL escapePressed, DWORD keyState)
{
	if (escapePressed)
		return DRAGDROP_S_CANCEL;
	
	if ((keyState & (MK_LBUTTON|MK_RBUTTON)) == 0)
		return DRAGDROP_S_DROP;

	return S_OK;	
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DropSource::GiveFeedback (DWORD effect)
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

//-----------------------------------------------------------------------------
// DataObject
//-----------------------------------------------------------------------------
Win32DataObject::Win32DataObject (CDropSource* dropSource)
: dropSource (dropSource)
{
	dropSource->remember ();
}

//-----------------------------------------------------------------------------
Win32DataObject::~Win32DataObject ()
{
	dropSource->forget ();
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DataObject::QueryInterface (REFIID riid, void** object)
{
	if (riid == ::IID_IDataObject)                        
	{                                                              
		AddRef ();                                                 
		*object = (::IDataObject*)this;                               
		return S_OK;                                          
	}
	else if (riid == ::IID_IUnknown)                        
	{                                                              
		AddRef ();                                                 
		*object = (::IUnknown*)this;                               
		return S_OK;                                          
	}
	return E_NOINTERFACE;
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DataObject::GetData (FORMATETC* format, STGMEDIUM* medium)
{
	medium->tymed = 0;
	medium->hGlobal = 0;
	medium->pUnkForRelease = 0;

	if (format->cfFormat == CF_TEXT || format->cfFormat == CF_UNICODETEXT)
	{
		for (long i = 0; i < dropSource->getCount (); i++)
		{
			if (dropSource->getEntryType (i) == CDropSource::kText)
			{
				const void* buffer;
				CDropSource::Type type;
				long bufferSize = dropSource->getEntry (i, buffer, type);
				UTF8StringHelper utf8String ((const char*)buffer);
				SIZE_T size = 0;
				const void* data = 0;
				if (format->cfFormat == CF_UNICODETEXT)
				{
					size = bufferSize * sizeof (WCHAR);
					data = utf8String.getWideString ();
				}
				else
				{
					size = bufferSize * sizeof (char);
					data = buffer;
				}
				if (data && size > 0)
				{
					HGLOBAL	memoryHandle = GlobalAlloc (GMEM_MOVEABLE, size); 
					void* memory = GlobalLock (memoryHandle);
					if (memory)
					{
						memcpy (memory, data, size);
						GlobalUnlock (memoryHandle);
					}

					medium->hGlobal = memoryHandle;						
					medium->tymed = TYMED_HGLOBAL;
					return S_OK;
				}
			}
		}
	}
	else if (format->cfFormat == CF_PRIVATEFIRST)
	{
		for (long i = 0; i < dropSource->getCount (); i++)
		{
			if (dropSource->getEntryType (i) == CDropSource::kBinary)
			{
				const void* buffer;
				CDropSource::Type type;
				long bufferSize = dropSource->getEntry (i, buffer, type);

				HGLOBAL	memoryHandle = GlobalAlloc (GMEM_MOVEABLE, bufferSize); 
				void* memory = GlobalLock (memoryHandle);
				if (memory)
				{
					memcpy (memory, buffer, bufferSize);
					GlobalUnlock (memoryHandle);
				}

				medium->hGlobal = memoryHandle;						
				medium->tymed = TYMED_HGLOBAL;
				return S_OK;
			}
		}
	}

#if 0
	if (format->cfFormat == CF_TEXT || format->cfFormat == CF_UNICODETEXT)
	{
		UTF8StringHelper utf8String (dataString.c_str ());
		SIZE_T size = 0;
		const void* data = 0;
		if (format->cfFormat == CF_UNICODETEXT)
		{
			size = (dataString.length () + 1) * sizeof (WCHAR);
			data = utf8String.getWideString ();
		}
		else
		{
			size = (dataString.length () + 1) * sizeof (char);
			data = dataString.c_str ();
		}

		if (data && size > 0)
		{
			HGLOBAL	memoryHandle = GlobalAlloc (GMEM_MOVEABLE, size); 
			void* memory = GlobalLock (memoryHandle);
			if (memory)
			{
				memcpy (memory, data, size);
				GlobalUnlock (memoryHandle);
			}

			medium->hGlobal = memoryHandle;						
			medium->tymed = TYMED_HGLOBAL;
			return S_OK;
		}
	}
#endif
	return E_UNEXPECTED;
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DataObject::GetDataHere (FORMATETC *format, STGMEDIUM *pmedium)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DataObject::QueryGetData (FORMATETC *format)
{
	if (format->cfFormat == CF_TEXT || format->cfFormat == CF_UNICODETEXT)
	{
		for (long i = 0; i < dropSource->getCount (); i++)
		{
			if (dropSource->getEntryType (i) == CDropSource::kText)
				return S_OK;
		}
	}
	else if (format->cfFormat == CF_PRIVATEFIRST)
	{
		for (long i = 0; i < dropSource->getCount (); i++)
		{
			if (dropSource->getEntryType (i) == CDropSource::kBinary)
				return S_OK;
		}
	}
	else if (format->cfFormat == CF_HDROP)
	{
	}
	return DV_E_FORMATETC;
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DataObject::GetCanonicalFormatEtc (FORMATETC *formatIn, FORMATETC *formatOut)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DataObject::SetData (FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DataObject::EnumFormatEtc (DWORD dwDirection, IEnumFORMATETC** enumFormat)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DataObject::DAdvise (FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DataObject::DUnadvise (DWORD dwConnection)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DataObject::EnumDAdvise (IEnumSTATDATA** ppenumAdvise)
{
	return E_NOTIMPL;
}

} // namespace

#endif // WINDOWS
