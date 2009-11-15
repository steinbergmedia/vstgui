
#include "win32frame.h"

#if WINDOWS && VSTGUI_PLATFORM_ABSTRACTION

#include "gdiplusdrawcontext.h"
#include "gdiplusbitmap.h"
#include "../../win32support.h"
namespace VSTGUI {

#define DEBUG_DRAWING	0

//-----------------------------------------------------------------------------
static TCHAR gClassName[100];
static bool bSwapped_mouse_buttons = false; 
static OSVERSIONINFOEX gSystemVersion;

//-----------------------------------------------------------------------------
static LONG_PTR WINAPI WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

//-----------------------------------------------------------------------------
IPlatformFrame* IPlatformFrame::createPlatformFrame (IPlatformFrameCallback* frame, const CRect& size, void* parent)
{
	return new Win32Frame (frame, size, (HWND)parent);
}

//-----------------------------------------------------------------------------
Win32Frame::Win32Frame (IPlatformFrameCallback* frame, const CRect& size, HWND parent)
: IPlatformFrame (frame)
, parentWindow (parent)
{
	initWindowClass ();

	DWORD style = 0;
	#if !DEBUG_DRAWING
	if (gSystemVersion.dwMajorVersion >= 6) // Vista and above
		style |= WS_EX_COMPOSITED;
	#endif
	windowHandle = CreateWindowEx (style, gClassName, TEXT("Window"),
									WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 
									0, 0, (int)size.width (), (int)size.height (), 
									parentWindow, NULL, GetInstance (), NULL);

	SetWindowLongPtr (windowHandle, GWLP_USERDATA, (__int3264)(LONG_PTR)this);

}

//-----------------------------------------------------------------------------
Win32Frame::~Win32Frame ()
{
	if (windowHandle)
	{
		SetWindowLongPtr (windowHandle, GWLP_USERDATA, (LONG_PTR)NULL);
		DestroyWindow (windowHandle);
	}
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
	RECT  rctTempWnd, rctParentWnd;
	HWND  hTempWnd;
	long   iFrame = (2 * GetSystemMetrics (SM_CYFIXEDFRAME));
	
	long diffWidth  = 0;
	long diffHeight = 0;
	
	hTempWnd = windowHandle;
	
	while ((diffWidth != iFrame) && (hTempWnd != NULL)) // look for FrameWindow
	{
		HWND hTempParentWnd = GetParent (hTempWnd);
		TCHAR buffer[1024];
		GetClassName (hTempParentWnd, buffer, 1024);
		if (!hTempParentWnd || !VSTGUI_STRCMP (buffer, TEXT("MDIClient")))
			break;
		GetWindowRect (hTempWnd, &rctTempWnd);
		GetWindowRect (hTempParentWnd, &rctParentWnd);
		
		SetWindowPos (hTempWnd, HWND_TOP, 0, 0, (int)newSize.getWidth () + diffWidth, (int)newSize.getHeight () + diffHeight, SWP_NOMOVE);
		
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
	{
		SetWindowPos (hTempWnd, HWND_TOP, 0, 0, (int)newSize.getWidth () + diffWidth, (int)newSize.getHeight () + diffHeight, SWP_NOMOVE);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool Win32Frame::getSize (CRect& size) const
{
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
	RECT r = {(LONG)rect.left, (LONG)rect.top, (LONG)rect.right, (LONG)rect.bottom};
	InvalidateRect (windowHandle, &r, true);
	return true;
}

//-----------------------------------------------------------------------------
bool Win32Frame::scrollRect (const CRect& src, const CPoint& distance)
{
	return false;
}

//-----------------------------------------------------------------------------
unsigned long Win32Frame::getTicks () const
{
	return (unsigned long)GetTickCount ();
}

//-----------------------------------------------------------------------------
bool Win32Frame::showTooltip (const CRect& rect, const char* utf8Text)
{
	return false;
}

//-----------------------------------------------------------------------------
bool Win32Frame::hideTooltip ()
{
	return false;
}

//-----------------------------------------------------------------------------
IPlatformTextEdit* Win32Frame::createPlatformTextEdit (IPlatformTextEditCallback* textEdit)
{
	return 0;
}

//-----------------------------------------------------------------------------
IPlatformOptionMenu* Win32Frame::createPlatformOptionMenu ()
{
	return 0;
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
LONG_PTR WINAPI WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
				#if 0 // TODO: ...
				CTextEdit* textEdit = (CTextEdit*)pFrame->getFocusView ();
				if (textEdit)
				{
					CColor fontColor = textEdit->getFontColor ();
					SetTextColor ((HDC) wParam, RGB (fontColor.red, fontColor.green, fontColor.blue));
					SetBkMode ((HDC)wParam, TRANSPARENT);
					return (LRESULT) ::GetStockObject (NULL_BRUSH);
				}
				#endif
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
				// added to achieve information from the ALT button
				if (GetKeyState (VK_MENU)    < 0)
					buttons |= kAlt;
				if (doubleClick)
					buttons |= kDoubleClick;
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
				// added to achieve information from the ALT button
				if (GetKeyState (VK_MENU)    < 0)
					buttons |= kAlt;
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
				// added to achieve information from the ALT button
				if (GetKeyState (VK_MENU)    < 0)
					buttons |= kAlt;
				CPoint where ((CCoord)((int)(short)LOWORD(lParam)), (CCoord)((int)(short)HIWORD(lParam)));
				pFrame->platformOnMouseUp (where, buttons);
				ReleaseCapture ();
				return 0;
			}
			case WM_DESTROY:
			{
				#if 0 // TODO: ...
				pFrame->setParentSystemWindow (0);
				#endif
				break;
			}
			case WM_ERASEBKGND: return 1; // don't draw background
		}
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}

} // namespace

#endif // WINDOWS && VSTGUI_PLATFORM_ABSTRACTION
