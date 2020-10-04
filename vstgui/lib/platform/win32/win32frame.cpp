// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32frame.h"

#if WINDOWS

#include <commctrl.h>
#include <cmath>
#include <windowsx.h>
#include "direct2d/d2ddrawcontext.h"
#include "direct2d/d2dbitmap.h"
#include "direct2d/d2dgraphicspath.h"
#include "win32textedit.h"
#include "win32optionmenu.h"
#include "win32support.h"
#include "win32datapackage.h"
#include "win32dragging.h"
#include "../common/genericoptionmenu.h"
#include "../../cdropsource.h"
#include "../../cgradient.h"

#if VSTGUI_OPENGL_SUPPORT
#include "win32openglview.h"
#endif

// windows libraries VSTGUI depends on
#ifdef _MSC_VER
#pragma comment(lib, "Shlwapi.lib")
#endif

namespace VSTGUI {

#define DEBUG_DRAWING	0

//-----------------------------------------------------------------------------
static TCHAR gClassName[100];
static bool bSwapped_mouse_buttons = false; 

//-----------------------------------------------------------------------------
static bool isParentLayered (HWND parent)
{
	WINDOWINFO info;
	info.cbSize = sizeof (info);
	while (parent)
	{
		if (GetWindowInfo (parent, &info))
		{
			if (info.dwStyle & WS_CHILD)
				parent = GetParent (parent);
			else
				break;
		}
	}
	if (parent)
	{
		if (info.dwExStyle & WS_EX_LAYERED)
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
Win32Frame::Win32Frame (IPlatformFrameCallback* frame, const CRect& size, HWND parent, PlatformType parentType)
: IPlatformFrame (frame)
, parentWindow (parent)
, windowHandle (nullptr)
, tooltipWindow (nullptr)
, oldFocusWindow (nullptr)
, backBuffer (nullptr)
, deviceContext (nullptr)
, inPaint (false)
, mouseInside (false)
, updateRegionList (nullptr)
, updateRegionListSize (0)
{
	useD2D ();
	if (parentType == PlatformType::kHWNDTopLevel)
	{
		windowHandle = parent;
		parentWindow = nullptr;
		RegisterDragDrop (windowHandle, new CDropTarget (this));
	}
	else
	{
		initWindowClass ();

		DWORD style = isParentLayered (parent) ? WS_EX_TRANSPARENT : 0;
		windowHandle = CreateWindowEx (style, gClassName, TEXT("Window"),
										WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 
										0, 0, (int)size.getWidth (), (int)size.getHeight (), 
										parentWindow, NULL, GetInstance (), NULL);

		if (windowHandle)
		{
			SetWindowLongPtr (windowHandle, GWLP_USERDATA, (__int3264)(LONG_PTR)this);
			RegisterDragDrop (windowHandle, new CDropTarget (this));
		}
	}
	setMouseCursor (kCursorDefault);
}

//-----------------------------------------------------------------------------
Win32Frame::~Win32Frame () noexcept
{
	if (updateRegionList)
		std::free (updateRegionList);
	if (deviceContext)
		deviceContext->forget ();
	if (tooltipWindow)
		DestroyWindow (tooltipWindow);
	if (backBuffer)
		backBuffer->forget ();
	if (windowHandle)
		RevokeDragDrop (windowHandle);
	if (parentWindow)
	{
		if (windowHandle)
		{
			SetWindowLongPtr (windowHandle, GWLP_USERDATA, (LONG_PTR)NULL);
			DestroyWindow (windowHandle);
		}
		destroyWindowClass ();
	}

	unuseD2D ();
}

//-----------------------------------------------------------------------------
int32_t Win32Frame::gUseCount = 0;

//-----------------------------------------------------------------------------
void Win32Frame::initWindowClass ()
{
	gUseCount++;
	if (gUseCount == 1)
	{
		OleInitialize (0);

		VSTGUI_SPRINTF (gClassName, TEXT("VSTGUI%p"), GetInstance ());
		
		WNDCLASS windowClass;
		windowClass.style = CS_GLOBALCLASS | CS_DBLCLKS;//|CS_OWNDC; // add Private-DC constant 

		windowClass.lpfnWndProc = WindowProc; 
		windowClass.cbClsExtra  = 0; 
		windowClass.cbWndExtra  = 0; 
		windowClass.hInstance   = GetInstance ();
		windowClass.hIcon = nullptr; 

		windowClass.hCursor = LoadCursor (NULL, IDC_ARROW);
		#if DEBUG_DRAWING
		windowClass.hbrBackground = GetSysColorBrush (COLOR_BTNFACE);
		#else
		windowClass.hbrBackground = nullptr;
		#endif
		windowClass.lpszMenuName  = nullptr; 
		windowClass.lpszClassName = gClassName; 
		RegisterClass (&windowClass);

		bSwapped_mouse_buttons = GetSystemMetrics (SM_SWAPBUTTON) > 0;
	}
}

//-----------------------------------------------------------------------------
void Win32Frame::destroyWindowClass ()
{
	gUseCount--;
	if (gUseCount == 0)
	{
		UnregisterClass (gClassName, GetInstance ());
		OleUninitialize ();
	}
}

//-----------------------------------------------------------------------------
void Win32Frame::initTooltip ()
{
	if (tooltipWindow == nullptr && windowHandle)
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
		ti.lpszText  = const_cast<TCHAR*> (TEXT("This is a tooltip"));
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
}

//-----------------------------------------------------------------------------
bool Win32Frame::setSize (const CRect& newSize)
{
	if (deviceContext)
	{
		deviceContext->forget ();
		deviceContext = nullptr;
	}
	if (backBuffer)
	{
		backBuffer->forget ();
		backBuffer = createOffscreenContext (newSize.getWidth (), newSize.getHeight ());
	}
	if (!parentWindow)
		return true;
	SetWindowPos (windowHandle, HWND_TOP, (int)newSize.left, (int)newSize.top, (int)newSize.getWidth (), (int)newSize.getHeight (), SWP_NOZORDER|SWP_NOCOPYBITS|SWP_NOREDRAW|SWP_DEFERERASE);
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
	MapWindowPoints (windowHandle, GetParent (windowHandle), &p, 1);
	size.left = p.x;
	size.top = p.y;
	size.right = p.x + (r.right - r.left);
	size.bottom = p.y + (r.bottom - r.top);
	return true;
}

//-----------------------------------------------------------------------------
bool Win32Frame::getCurrentMousePosition (CPoint& mousePosition) const
{
	POINT _where;
	GetCursorPos (&_where);
	mousePosition (static_cast<CCoord> (_where.x), static_cast<CCoord> (_where.y));
	if (auto hwnd = getHWND ())
	{
		ScreenToClient (hwnd, &_where);
		mousePosition (static_cast<CCoord> (_where.x), static_cast<CCoord> (_where.y));
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool Win32Frame::getCurrentMouseButtons (CButtonState& buttons) const
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
	HCURSOR cursor = nullptr;
	switch (type)
	{
		case kCursorWait:
			cursor = LoadCursor (nullptr, IDC_WAIT);
			break;
		case kCursorHSize:
			cursor = LoadCursor (nullptr, IDC_SIZEWE);
			break;
		case kCursorVSize:
			cursor = LoadCursor (nullptr, IDC_SIZENS);
			break;
		case kCursorNESWSize:
			cursor = LoadCursor (nullptr, IDC_SIZENESW);
			break;
		case kCursorNWSESize:
			cursor = LoadCursor (nullptr, IDC_SIZENWSE);
			break;
		case kCursorSizeAll:
			cursor = LoadCursor (nullptr, IDC_SIZEALL);
			break;
		case kCursorNotAllowed:
			cursor = LoadCursor (nullptr, IDC_NO);
			break;
		case kCursorHand:
			cursor = LoadCursor (nullptr, IDC_HAND);
			break;
		default:
			cursor = LoadCursor (nullptr, IDC_ARROW);
			break;
	}
	lastSetCursor = type;
	SetClassLongPtr (getPlatformWindow (), GCLP_HCURSOR, (__int3264)(LONG_PTR)(cursor));
	return true;
}

//-----------------------------------------------------------------------------
bool Win32Frame::invalidRect (const CRect& rect)
{
	if (inPaint)
		return false;
	if (!rect.isEmpty ())
	{
		RECT r = {(LONG)rect.left, (LONG)rect.top, (LONG)ceil (rect.right), (LONG)ceil (rect.bottom)};
		InvalidateRect (windowHandle, &r, true);
	}
	return true;
}

//-----------------------------------------------------------------------------
bool Win32Frame::scrollRect (const CRect& src, const CPoint& distance)
{
	return false;
}

//-----------------------------------------------------------------------------
bool Win32Frame::showTooltip (const CRect& rect, const char* utf8Text)
{
	initTooltip ();
	if (tooltipWindow)
	{
		std::string str (utf8Text);
		size_t pos = 0;
		while ((pos = str.find ("\\n", pos)) != std::string::npos)
		{
			str.erase (pos, 2);
			str.insert (pos, "\r\n");
		}
		UTF8StringHelper tooltipText (str.data ());
		RECT rc;
		rc.left = (LONG)rect.left;
		rc.top = (LONG)rect.top;
		rc.right = (LONG)rect.right;
		rc.bottom = (LONG)rect.bottom;
		TOOLINFO ti = {};
		ti.cbSize = sizeof(TOOLINFO);
		ti.hwnd = windowHandle;
		ti.uId = 0;
		ti.rect = rc;
		ti.lpszText = (TCHAR*)(const TCHAR*)tooltipText;
		SendMessage (tooltipWindow, TTM_SETMAXTIPWIDTH, 0, 0);
		SendMessage (tooltipWindow, TTM_SETDELAYTIME, 0, 2000);
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
		TOOLINFO ti = {};
		ti.cbSize = sizeof(TOOLINFO);
		ti.hwnd = windowHandle;
		ti.uId = 0;
		ti.lpszText = nullptr;
		SendMessage (tooltipWindow, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
		SendMessage (tooltipWindow, TTM_POP, 0, 0);
	}
	return false;
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformTextEdit> Win32Frame::createPlatformTextEdit (IPlatformTextEditCallback* textEdit)
{
	return owned<IPlatformTextEdit> (new Win32TextEdit (windowHandle, textEdit));
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformOptionMenu> Win32Frame::createPlatformOptionMenu ()
{
	if (genericOptionMenuTheme)
	{
		CButtonState buttons;
		getCurrentMouseButtons (buttons);
		return makeOwned<GenericOptionMenu> (dynamic_cast<CFrame*> (frame), buttons,
		                                     *genericOptionMenuTheme.get ());
	}
	return owned<IPlatformOptionMenu> (new Win32OptionMenu (windowHandle));
}

#if VSTGUI_OPENGL_SUPPORT
//-----------------------------------------------------------------------------
SharedPointer<IPlatformOpenGLView> Win32Frame::createPlatformOpenGLView ()
{
	return owned<IPlatformOpenGLView> (new Win32OpenGLView (this));
}
#endif

//-----------------------------------------------------------------------------
SharedPointer<COffscreenContext> Win32Frame::createOffscreenContext (CCoord width, CCoord height, double scaleFactor)
{
	D2DBitmap* bitmap = new D2DBitmap (CPoint (width * scaleFactor, height * scaleFactor));
	bitmap->setScaleFactor (scaleFactor);
	auto context = owned<COffscreenContext> (new D2DDrawContext (bitmap));
	bitmap->forget ();
	return context;
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
class Win32LegacyDragSupport final : virtual public DragCallbackAdapter, virtual public NonAtomicReferenceCounted
{
public:
	void dragEnded (IDraggingSession*, CPoint, DragOperation r) final { result = r; }
	DragOperation result {DragOperation::None};
};

//------------------------------------------------------------------------------------
DragResult Win32Frame::doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap)
{
	Win32LegacyDragSupport dragSupport;

	Win32DraggingSession session (this);
	if (session.doDrag (DragDescription (source, offset, dragBitmap), &dragSupport))
	{
		switch (dragSupport.result)
		{
			case DragOperation::Copy: return kDragCopied;
			case DragOperation::Move: return kDragMoved;
			case DragOperation::None: return kDragRefused;
		}
	}
	return kDragRefused;
}
#endif

//-----------------------------------------------------------------------------
bool Win32Frame::doDrag (const DragDescription& dragDescription, const SharedPointer<IDragCallback>& callback)
{
	Win32DraggingSession session (this);
	return session.doDrag (dragDescription, callback);
}

//-----------------------------------------------------------------------------
void Win32Frame::setClipboard (const SharedPointer<IDataPackage>& data)
{
	auto dataObject = makeOwned<Win32DataObject> (data);
	auto hr = OleSetClipboard (dataObject);
	if (hr != S_OK)
	{
#if DEBUG
		DebugPrint ("Setting clipboard failed!\n");
#endif
	}
}

//-----------------------------------------------------------------------------
SharedPointer<IDataPackage> Win32Frame::getClipboard ()
{
	IDataObject* dataObject = nullptr;;
	if (OleGetClipboard (&dataObject) != S_OK)
		return nullptr;
	return makeOwned<Win32DataPackage> (dataObject);
}

//-----------------------------------------------------------------------------
void Win32Frame::onFrameClosed ()
{
	frame = nullptr;
}

//-----------------------------------------------------------------------------
bool Win32Frame::setupGenericOptionMenu (bool use, GenericOptionMenuTheme* theme)
{
	if (!use)
	{
		genericOptionMenuTheme = nullptr;
	}
	else
	{
		if (theme)
			genericOptionMenuTheme = std::unique_ptr<GenericOptionMenuTheme> (new GenericOptionMenuTheme (*theme));
		else
			genericOptionMenuTheme = std::unique_ptr<GenericOptionMenuTheme> (new GenericOptionMenuTheme);
	}
	return true;
}

//-----------------------------------------------------------------------------
void Win32Frame::paint (HWND hwnd)
{
	HRGN rgn = CreateRectRgn (0, 0, 0, 0);
	if (GetUpdateRgn (hwnd, rgn, false) == NULLREGION)
	{
		DeleteObject (rgn);
		return;
	}

	inPaint = true;
	
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint (hwnd, &ps);

	if (hdc)
	{
		CRect updateRect ((CCoord)ps.rcPaint.left, (CCoord)ps.rcPaint.top, (CCoord)ps.rcPaint.right, (CCoord)ps.rcPaint.bottom);
		CRect frameSize;
		getSize (frameSize);
		frameSize.offset (-frameSize.left, -frameSize.top);
		if (deviceContext == nullptr)
			deviceContext = createDrawContext (hwnd, hdc, frameSize);
		if (deviceContext)
		{
			deviceContext->setClipRect (updateRect);

			CDrawContext* drawContext = backBuffer ? backBuffer : deviceContext;
			drawContext->beginDraw ();
			DWORD len = GetRegionData (rgn, 0, NULL);
			if (len)
			{
				if (len > updateRegionListSize)
				{
					if (updateRegionList)
						std::free (updateRegionList);
					updateRegionListSize = len;
					updateRegionList = (RGNDATA*) std::malloc (updateRegionListSize);
				}
				GetRegionData (rgn, len, updateRegionList);
				if (updateRegionList->rdh.nCount > 1)
				{
					std::vector<CRect> dirtyRects;
					dirtyRects.reserve (updateRegionList->rdh.nCount);
					RECT* rp = (RECT*)updateRegionList->Buffer;
					dirtyRects.emplace_back (CRect (rp->left, rp->top, rp->right, rp->bottom));
					++rp;
					for (uint32_t i = 1; i < updateRegionList->rdh.nCount; ++i, ++rp)
					{
						CRect ur (rp->left, rp->top, rp->right, rp->bottom);
						auto mustAdd = true;
						for (auto& r : dirtyRects)
						{
							auto cr = ur;
							cr.unite (r);
							if (cr.getWidth () * cr.getHeight () ==
							    ur.getWidth () * ur.getHeight () + r.getWidth () * r.getHeight ())
							{
								r = cr;
								mustAdd = false;
								break;
							}
						}
						if (mustAdd)
							dirtyRects.emplace_back (ur);
					}
					for (auto& updateRect : dirtyRects)
					{
						drawContext->clearRect (updateRect);
						getFrame ()->platformDrawRect (drawContext, updateRect);
					}
				}
				else
				{
					drawContext->clearRect (updateRect);
					getFrame ()->platformDrawRect (drawContext, updateRect);
				}
			}
			drawContext->endDraw ();
			if (backBuffer)
			{
				deviceContext->beginDraw ();
				deviceContext->clearRect (updateRect);
				backBuffer->copyFrom (deviceContext, updateRect, CPoint (updateRect.left, updateRect.top));
				deviceContext->endDraw ();
			}
		}
	}

	EndPaint (hwnd, &ps);
	DeleteObject (rgn);
	
	inPaint = false;
}

static unsigned char translateWinVirtualKey (WPARAM winVKey)
{
	switch (winVKey)
	{
		case VK_BACK: return VKEY_BACK;
		case VK_TAB: return VKEY_TAB;
		case VK_CLEAR: return VKEY_CLEAR;
		case VK_RETURN: return VKEY_RETURN;
		case VK_PAUSE: return VKEY_PAUSE;
		case VK_ESCAPE: return VKEY_ESCAPE;
		case VK_SPACE: return VKEY_SPACE;
// TODO:		case VK_NEXT: return VKEY_NEXT;
		case VK_END: return VKEY_END;
		case VK_HOME: return VKEY_HOME;
		case VK_LEFT: return VKEY_LEFT;
		case VK_RIGHT: return VKEY_RIGHT;
		case VK_UP: return VKEY_UP;
		case VK_DOWN: return VKEY_DOWN;
		case VK_PRIOR: return VKEY_PAGEUP;
		case VK_NEXT: return VKEY_PAGEDOWN;
		case VK_SELECT: return VKEY_SELECT;
		case VK_PRINT: return VKEY_PRINT;
		case VK_SNAPSHOT: return VKEY_SNAPSHOT;
		case VK_INSERT: return VKEY_INSERT;
		case VK_DELETE: return VKEY_DELETE;
		case VK_HELP: return VKEY_HELP;
		case VK_NUMPAD0: return VKEY_NUMPAD0;
		case VK_NUMPAD1: return VKEY_NUMPAD1;
		case VK_NUMPAD2: return VKEY_NUMPAD2;
		case VK_NUMPAD3: return VKEY_NUMPAD3;
		case VK_NUMPAD4: return VKEY_NUMPAD4;
		case VK_NUMPAD5: return VKEY_NUMPAD5;
		case VK_NUMPAD6: return VKEY_NUMPAD6;
		case VK_NUMPAD7: return VKEY_NUMPAD7;
		case VK_NUMPAD8: return VKEY_NUMPAD8;
		case VK_NUMPAD9: return VKEY_NUMPAD9;
		case VK_MULTIPLY: return VKEY_MULTIPLY;
		case VK_ADD: return VKEY_ADD;
		case VK_SEPARATOR: return VKEY_SEPARATOR;
		case VK_SUBTRACT: return VKEY_SUBTRACT;
		case VK_DECIMAL: return VKEY_DECIMAL;
		case VK_DIVIDE: return VKEY_DIVIDE;
		case VK_F1: return VKEY_F1;
		case VK_F2: return VKEY_F2;
		case VK_F3: return VKEY_F3;
		case VK_F4: return VKEY_F4;
		case VK_F5: return VKEY_F5;
		case VK_F6: return VKEY_F6;
		case VK_F7: return VKEY_F7;
		case VK_F8: return VKEY_F8;
		case VK_F9: return VKEY_F9;
		case VK_F10: return VKEY_F10;
		case VK_F11: return VKEY_F11;
		case VK_F12: return VKEY_F12;
		case VK_NUMLOCK: return VKEY_NUMLOCK;
		case VK_SCROLL: return VKEY_SCROLL;
		case VK_SHIFT: return VKEY_SHIFT;
		case VK_CONTROL: return VKEY_CONTROL;
		case VK_MENU: return VKEY_ALT;
		case VKEY_EQUALS: return VKEY_EQUALS;
	}
	return 0;
}

//-----------------------------------------------------------------------------
LONG_PTR WINAPI Win32Frame::proc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (getFrame () == nullptr)
		return DefWindowProc (hwnd, message, wParam, lParam);

	SharedPointer<Win32Frame> lifeGuard (this);
	IPlatformFrameCallback* pFrame = getFrame ();
	bool doubleClick = false;
	
	switch (message)
	{
		case WM_MOUSEWHEEL:
		{
			CButtonState buttons = 0;
			if (GetAsyncKeyState (VK_SHIFT)   < 0)
				buttons |= kShift;
			if (GetAsyncKeyState (VK_CONTROL) < 0)
				buttons |= kControl;
			if (GetAsyncKeyState (VK_MENU)    < 0)
				buttons |= kAlt;
			short zDelta = (short) GET_WHEEL_DELTA_WPARAM(wParam);
			POINT p {GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam)};
			ScreenToClient (windowHandle, &p);
			CPoint where (p.x, p.y);
			if (pFrame->platformOnMouseWheel (where, kMouseWheelAxisY, ((float)zDelta / WHEEL_DELTA), buttons))
				return 0;
			break;
		}
		case WM_MOUSEHWHEEL:	// new since vista
		{
			CButtonState buttons = 0;
			if (GetAsyncKeyState (VK_SHIFT)   < 0)
				buttons |= kShift;
			if (GetAsyncKeyState (VK_CONTROL) < 0)
				buttons |= kControl;
			if (GetAsyncKeyState (VK_MENU)    < 0)
				buttons |= kAlt;
			short zDelta = (short) GET_WHEEL_DELTA_WPARAM(wParam);
			POINT p {GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam)};
			ScreenToClient (windowHandle, &p);
			CPoint where (p.x, p.y);
			if (pFrame->platformOnMouseWheel (where, kMouseWheelAxisX, ((float)-zDelta / WHEEL_DELTA), buttons))
				return 0;
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
			paint (hwnd);
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
			CButtonState buttons = 0;
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
			if (GetAsyncKeyState (VK_MENU)    < 0)
				buttons |= kAlt;
			if (doubleClick)
				buttons |= kDoubleClick;
			HWND oldFocus = SetFocus(getPlatformWindow());
			if(oldFocus != hwnd)
				oldFocusWindow = oldFocus;

			CPoint where (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
			if (pFrame->platformOnMouseDown (where, buttons) == kMouseEventHandled && getPlatformWindow ())
				SetCapture (getPlatformWindow ());
			return 0;
		}
		case WM_MOUSELEAVE:
		{
			CPoint where;
			getCurrentMousePosition (where);
			CButtonState buttons;
			getCurrentMouseButtons (buttons);
			pFrame->platformOnMouseExited (where, buttons);
			mouseInside = false;
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			CButtonState buttons = 0;
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
			if (GetAsyncKeyState (VK_MENU) < 0)
				buttons |= kAlt;
			if (!mouseInside)
			{
				// this makes sure that WM_MOUSELEAVE will be generated by the system
				mouseInside = true;
				TRACKMOUSEEVENT tme = {};
				tme.cbSize = sizeof (tme);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = windowHandle;
				TrackMouseEvent (&tme);
			}
			CPoint where (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
			pFrame->platformOnMouseMoved (where, buttons);
			return 0;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		{
			CButtonState buttons = 0;
			if (wParam & MK_LBUTTON || message == WM_LBUTTONUP)
				buttons |= kLButton;
			if (wParam & MK_RBUTTON || message == WM_RBUTTONUP)
				buttons |= kRButton;
			if (wParam & MK_MBUTTON || message == WM_MBUTTONUP)
				buttons |= kMButton;
			if (wParam & MK_XBUTTON1)
				buttons |= kButton4;
			if (wParam & MK_XBUTTON2)
				buttons |= kButton5;
			if (wParam & MK_CONTROL)
				buttons |= kControl;
			if (wParam & MK_SHIFT)
				buttons |= kShift;
			if (GetAsyncKeyState (VK_MENU) < 0)
				buttons |= kAlt;
			CPoint where (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
			pFrame->platformOnMouseUp (where, buttons);
			ReleaseCapture ();
			return 0;
		}
		case WM_KEYDOWN:
		{
			VstKeyCode key {};
			if (GetAsyncKeyState (VK_SHIFT)   < 0)
				key.modifier |= MODIFIER_SHIFT;
			if (GetAsyncKeyState (VK_CONTROL) < 0)
				key.modifier |= MODIFIER_CONTROL;
			if (GetAsyncKeyState (VK_MENU)    < 0)
				key.modifier |= MODIFIER_ALTERNATE;
			key.virt = translateWinVirtualKey (wParam);
			key.character = MapVirtualKey (static_cast<UINT> (wParam), MAPVK_VK_TO_CHAR);
			if (key.virt || key.character)
			{
				key.character = std::tolower (key.character);
				if (pFrame->platformOnKeyDown (key))
					return 0;
			}

			if (IsWindow (oldFocusWindow))
			{
				auto oldProc = reinterpret_cast<WNDPROC> (GetWindowLongPtr (oldFocusWindow, GWLP_WNDPROC));
				if (oldProc && oldProc != WindowProc)
					return CallWindowProc (oldProc, oldFocusWindow, message, wParam, lParam);
			}
			break;
		}
		case WM_KEYUP:
		{
			VstKeyCode key {};
			if (GetAsyncKeyState (VK_SHIFT)   < 0)
				key.modifier |= MODIFIER_SHIFT;
			if (GetAsyncKeyState (VK_CONTROL) < 0)
				key.modifier |= MODIFIER_CONTROL;
			if (GetAsyncKeyState (VK_MENU)    < 0)
				key.modifier |= MODIFIER_ALTERNATE;
			key.virt = translateWinVirtualKey (wParam);
			key.character = MapVirtualKey (static_cast<UINT> (wParam), MAPVK_VK_TO_CHAR);
			if (key.virt || key.character)
			{
				if (pFrame->platformOnKeyUp (key))
					return 0;
			}

			if (IsWindow (oldFocusWindow))
			{
				auto oldProc = reinterpret_cast<WNDPROC> (GetWindowLongPtr (oldFocusWindow, GWLP_WNDPROC));
				if (oldProc && oldProc != WindowProc)
					return CallWindowProc (oldProc, oldFocusWindow, message, wParam, lParam);
			}
			break;
		}
		case WM_SETFOCUS:
		{
			pFrame->platformOnActivate (true);
			break;
		}
		case WM_KILLFOCUS:
		{
			oldFocusWindow = nullptr;

			HWND focusWindow = GetFocus ();
			if (GetParent (focusWindow) != windowHandle)
				pFrame->platformOnActivate (false);
			break;
		}
		case WM_DESTROY:
		{
#if DEBUG
			DebugPrint ("This sometimes happens, only when we are currently processing a mouse down event and via a callback into the host the window gets destroyed. Otherwise this should never get called. We are the owner of the window and we are responsible of destroying it.\n");
#endif
			if (parentWindow)
				windowHandle = nullptr;
			break;
		}
		case WM_ERASEBKGND:
		{
			return 1; // don't draw background
		}
		case WM_COMMAND:
		{
			if (HIWORD (wParam) == EN_CHANGE)
			{
				// text control changes will be forwarded to the text control window proc
				HWND controlWindow = (HWND)lParam;
				WINDOWSPROC textEditWindowProc = (WINDOWSPROC)(LONG_PTR)GetWindowLongPtr (controlWindow, GWLP_WNDPROC);
				if (textEditWindowProc)
				{
					textEditWindowProc (controlWindow, WM_COMMAND, wParam, lParam);
				}
			}
			break;
		}
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}

//-----------------------------------------------------------------------------
LONG_PTR WINAPI Win32Frame::WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Win32Frame* win32Frame = (Win32Frame*)(LONG_PTR)GetWindowLongPtr (hwnd, GWLP_USERDATA);
	if (win32Frame)
	{
		return win32Frame->proc (hwnd, message, wParam, lParam);
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}

//-----------------------------------------------------------------------------
CGradient* CGradient::create (const ColorStopMap& colorStopMap)
{
	return new CGradient (colorStopMap);
}

} // VSTGUI

#endif // WINDOWS
