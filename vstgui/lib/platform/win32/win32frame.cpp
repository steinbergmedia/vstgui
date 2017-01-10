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

#include "win32frame.h"

#if WINDOWS

#include <shlobj.h>
#include <commctrl.h>
#include <cmath>
#include <windowsx.h>

#include "gdiplusdrawcontext.h"
#include "gdiplusbitmap.h"
#include "gdiplusgraphicspath.h"
#include "direct2d/d2ddrawcontext.h"
#include "direct2d/d2dbitmap.h"
#include "direct2d/d2dgraphicspath.h"
#include "win32textedit.h"
#include "win32optionmenu.h"
#include "win32support.h"
#include "win32dragcontainer.h"
#include "../../cdropsource.h"
#include "../../cgradient.h"

#if VSTGUI_OPENGL_SUPPORT
#include "win32openglview.h"
#endif

// windows libraries VSTGUI depends on
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "gdiplus.lib")

namespace VSTGUI {

#define DEBUG_DRAWING	0

//-----------------------------------------------------------------------------
static TCHAR gClassName[100];
static bool bSwapped_mouse_buttons = false; 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CDropTarget : public ::IDropTarget
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
	int32_t refCount;
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
	Win32DataObject (IDataPackage* dataPackage);
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
	IDataPackage* dataPackage;
};

//-----------------------------------------------------------------------------
IPlatformFrame* IPlatformFrame::createPlatformFrame (IPlatformFrameCallback* frame, const CRect& size, void* parent, PlatformType parentType)
{
	return new Win32Frame (frame, size, (HWND)parent);
}

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
Win32Frame::Win32Frame (IPlatformFrameCallback* frame, const CRect& size, HWND parent)
: IPlatformFrame (frame)
, windowHandle (0)
, parentWindow (parent)
, tooltipWindow (0)
, backBuffer (0)
, deviceContext (0)
, inPaint (false)
, mouseInside (false)
, updateRegionList (0)
, updateRegionListSize (0)
{
	initWindowClass ();
	useD2D ();

	DWORD style = isParentLayered (parent) ? WS_EX_TRANSPARENT : 0;
	#if !DEBUG_DRAWING
	if (getD2DFactory ()) // workaround for Direct2D hotfix (KB2028560)
	{
		// when WS_EX_COMPOSITED is set drawing does not work correctly. This seems like a bug in Direct2D wich happens with this hotfix
	}
	else if (getSystemVersion ().dwMajorVersion >= 6) // Vista and above
		style |= WS_EX_COMPOSITED;
	else
		backBuffer = createOffscreenContext (size.getWidth (), size.getHeight ());
	#endif
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

//-----------------------------------------------------------------------------
Win32Frame::~Win32Frame ()
{
	if (updateRegionList)
		std::free (updateRegionList);
	if (deviceContext)
		deviceContext->forget ();
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
		deviceContext = 0;
	}
	if (backBuffer)
	{
		backBuffer->forget ();
		backBuffer = createOffscreenContext (newSize.getWidth (), newSize.getHeight ());
	}
	// TODO for VST2: we only set the size of the window we own. In VST2 this was not the case, we also resized the parent window. This must be done upstream now.
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
	HCURSOR cursor = 0;
	switch (type)
	{
		case kCursorWait:
			cursor = LoadCursor (0, IDC_WAIT);
			break;
		case kCursorHSize:
			cursor = LoadCursor (0, IDC_SIZEWE);
			break;
		case kCursorVSize:
			cursor = LoadCursor (0, IDC_SIZENS);
			break;
		case kCursorNESWSize:
			cursor = LoadCursor (0, IDC_SIZENESW);
			break;
		case kCursorNWSESize:
			cursor = LoadCursor (0, IDC_SIZENWSE);
			break;
		case kCursorSizeAll:
			cursor = LoadCursor (0, IDC_SIZEALL);
			break;
		case kCursorNotAllowed:
			cursor = LoadCursor (0, IDC_NO);
			break;
		case kCursorHand:
			cursor = LoadCursor (0, IDC_HAND);
			break;
		default:
			cursor = LoadCursor (0, IDC_ARROW);
			break;
	}
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
uint32_t IPlatformFrame::getTicks ()
{
	return (uint32_t)GetTickCount ();
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
		UTF8StringHelper tooltipText (str.c_str ());
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

#if VSTGUI_OPENGL_SUPPORT
//-----------------------------------------------------------------------------
IPlatformOpenGLView* Win32Frame::createPlatformOpenGLView ()
{
	return new Win32OpenGLView (this);
}
#endif

//-----------------------------------------------------------------------------
COffscreenContext* Win32Frame::createOffscreenContext (CCoord width, CCoord height, double scaleFactor)
{
#if VSTGUI_DIRECT2D_SUPPORT
	if (getD2DFactory ())
	{
		D2DBitmap* bitmap = new D2DBitmap (CPoint (width, height));
		D2DDrawContext* context = new D2DDrawContext (bitmap);
		bitmap->forget ();
		return context;
	}
#endif
	GdiplusBitmap* bitmap = new GdiplusBitmap (CPoint (width, height));
	GdiplusDrawContext* context = new GdiplusDrawContext (bitmap);
	bitmap->forget ();
	return context;
}

//------------------------------------------------------------------------------------
DragResult Win32Frame::doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap)
{
	DragResult result = kDragRefused;
	Win32DataObject* dataObject = new Win32DataObject (source);
	Win32DropSource* dropSource = new Win32DropSource;
	DWORD outEffect;
	HRESULT hResult = DoDragDrop (dataObject, dropSource, DROPEFFECT_COPY, &outEffect);
	dataObject->Release ();
	dropSource->Release ();
	if (hResult == DRAGDROP_S_DROP)
	{
		if (outEffect == DROPEFFECT_MOVE)
			result = kDragMoved;
		else
			result = kDragCopied;
	}
	return result;
}

//-----------------------------------------------------------------------------
void Win32Frame::setClipboard (IDataPackage* data)
{
	// TODO: Implementation
}

//-----------------------------------------------------------------------------
IDataPackage* Win32Frame::getClipboard ()
{
	// TODO: Implementation
	return 0;
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
		if (deviceContext == 0)
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
				if (updateRegionList->rdh.nCount > 0)
				{
					RECT* rp = (RECT*)updateRegionList->Buffer;
					for (uint32_t i = 0; i < updateRegionList->rdh.nCount; i++)
					{
						CRect ur (rp->left, rp->top, rp->right, rp->bottom);
						paintRect = ur;
						drawContext->clearRect (ur);
						getFrame ()->platformDrawRect (drawContext, ur);
						rp++;
					}
				}
				else
				{
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
LONG_PTR WINAPI Win32Frame::WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Win32Frame* win32Frame = (Win32Frame*)(LONG_PTR)GetWindowLongPtr (hwnd, GWLP_USERDATA);
	if (win32Frame)
	{
		SharedPointer<Win32Frame> lifeGuard (win32Frame);
		IPlatformFrameCallback* pFrame = win32Frame->getFrame ();
		bool doubleClick = false;

		switch (message)
		{
			case WM_MOUSEWHEEL:
			{
				CButtonState buttons = 0;
				if (GetKeyState (VK_SHIFT)   < 0)
					buttons |= kShift;
				if (GetKeyState (VK_CONTROL) < 0)
					buttons |= kControl;
				if (GetKeyState (VK_MENU)    < 0)
					buttons |= kAlt;
				CPoint where (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
				short zDelta = (short) HIWORD(wParam);
				RECT rctWnd;
				GetWindowRect (hwnd, &rctWnd);
				where.offset ((CCoord)-rctWnd.left, (CCoord)-rctWnd.top);
				pFrame->platformOnMouseWheel (where, kMouseWheelAxisY, ((float)zDelta / WHEEL_DELTA), buttons);
				break;
			}
			case WM_MOUSEHWHEEL:	// new since vista
			{
				CButtonState buttons = 0;
				if (GetKeyState (VK_SHIFT)   < 0)
					buttons |= kShift;
				if (GetKeyState (VK_CONTROL) < 0)
					buttons |= kControl;
				if (GetKeyState (VK_MENU)    < 0)
					buttons |= kAlt;
				CPoint where (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
				short zDelta = (short) HIWORD(wParam);
				RECT rctWnd;
				GetWindowRect (hwnd, &rctWnd);
				where.offset ((CCoord)-rctWnd.left, (CCoord)-rctWnd.top);
				pFrame->platformOnMouseWheel (where, kMouseWheelAxisX, ((float)-zDelta / WHEEL_DELTA), buttons);
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
				win32Frame->paint (hwnd);
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
				if (GetKeyState (VK_MENU)    < 0)
					buttons |= kAlt;
				if (doubleClick)
					buttons |= kDoubleClick;
				SetFocus (win32Frame->getPlatformWindow ());
				CPoint where (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
				if (pFrame->platformOnMouseDown (where, buttons) == kMouseEventHandled && win32Frame->getPlatformWindow ())
					SetCapture (win32Frame->getPlatformWindow ());
				return 0;
			}
			case WM_MOUSELEAVE:
			{
				CPoint where;
				win32Frame->getCurrentMousePosition (where);
				CButtonState buttons;
				win32Frame->getCurrentMouseButtons (buttons);
				pFrame->platformOnMouseExited (where, buttons);
				win32Frame->mouseInside = false;
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
				if (message & MK_LBUTTON || message == WM_LBUTTONUP)
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
				if (GetKeyState (VK_MENU) < 0)
					buttons |= kAlt;
				CPoint where (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
				pFrame->platformOnMouseUp (where, buttons);
				ReleaseCapture ();
				return 0;
			}
			case WM_KEYDOWN:
			{
				VstKeyCode key;
				key.virt = translateWinVirtualKey (wParam);
				if (key.virt)
				{
					if (pFrame->platformOnKeyDown (key))
						return 0;
				}
				break;
			}
			case WM_KEYUP:
			{
				VstKeyCode key;
				key.virt = translateWinVirtualKey (wParam);
				if (key.virt)
				{
					if (pFrame->platformOnKeyUp (key))
						return 0;
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
				HWND focusWindow = GetFocus ();
				if (GetParent (focusWindow) != win32Frame->windowHandle)
					pFrame->platformOnActivate (false);
				break;
			}
			case WM_DESTROY:
			{
				#if DEBUG
				DebugPrint ("This sometimes happens, only when we are currently processing a mouse down event and via a callback into the host the window gets destroyed. Otherwise this should never get called. We are the owner of the window and we are responsible of destroying it.\n");
				#endif
				win32Frame->windowHandle = 0;
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
Win32DataObject::Win32DataObject (IDataPackage* dataPackage)
: dataPackage (dataPackage)
{
	dataPackage->remember ();
}

//-----------------------------------------------------------------------------
Win32DataObject::~Win32DataObject ()
{
	dataPackage->forget ();
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
		for (uint32_t i = 0; i < dataPackage->getCount (); i++)
		{
			if (dataPackage->getDataType (i) == IDataPackage::kText)
			{
				const void* buffer;
				IDataPackage::Type type;
				uint32_t bufferSize = dataPackage->getData (i, buffer, type);
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
	else if (format->cfFormat == CF_HDROP)
	{
		HRESULT result = E_UNEXPECTED;
		UTF8StringHelper** wideStringFileNames = (UTF8StringHelper**)std::malloc (sizeof (UTF8StringHelper*) * dataPackage->getCount ());
		memset (wideStringFileNames, 0, sizeof (UTF8StringHelper*) * dataPackage->getCount ());
		uint32_t fileNamesIndex = 0;
		uint32_t bufferSizeNeeded = 0;
		for (uint32_t i = 0; i < dataPackage->getCount (); i++)
		{
			if (dataPackage->getDataType (i) == IDataPackage::kFilePath)
			{
				const void* buffer;
				IDataPackage::Type type;
				dataPackage->getData (i, buffer, type);

				wideStringFileNames[fileNamesIndex] = new UTF8StringHelper ((UTF8StringPtr)buffer);
				bufferSizeNeeded += static_cast<uint32_t> (wcslen (*wideStringFileNames[fileNamesIndex])) + 1;
				fileNamesIndex++;
			}
		}
		bufferSizeNeeded++;
		bufferSizeNeeded *= sizeof (WCHAR);
		bufferSizeNeeded += sizeof (DROPFILES);
		HGLOBAL	memoryHandle = GlobalAlloc (GMEM_MOVEABLE, bufferSizeNeeded); 
		void* memory = GlobalLock (memoryHandle);
		if (memory)
		{
			DROPFILES* dropFiles = (DROPFILES*)memory;
			dropFiles->pFiles = sizeof (DROPFILES);
			dropFiles->pt.x   = 0; 
			dropFiles->pt.y   = 0;
			dropFiles->fNC    = FALSE;
			dropFiles->fWide  = TRUE;
			int8_t* memAddr = ((int8_t*)memory) + sizeof (DROPFILES);
			for (uint32_t i = 0; i < fileNamesIndex; i++)
			{
				size_t len = (wcslen (wideStringFileNames[i]->getWideString ()) + 1) * 2;
				memcpy (memAddr, wideStringFileNames[i]->getWideString (), len);
				memAddr += len;
			}
			*memAddr = 0;
			memAddr++;
			*memAddr = 0;
			memAddr++;
			GlobalUnlock (memoryHandle);
			medium->hGlobal = memoryHandle;
			medium->tymed = TYMED_HGLOBAL;
			result = S_OK;
		}
		for (uint32_t i = 0; i < fileNamesIndex; i++)
			delete wideStringFileNames[i];
		std::free (wideStringFileNames);
		return result;
	}
	else if (format->cfFormat == CF_PRIVATEFIRST)
	{
		for (uint32_t i = 0; i < dataPackage->getCount (); i++)
		{
			if (dataPackage->getDataType (i) == IDataPackage::kBinary)
			{
				const void* buffer;
				IDataPackage::Type type;
				uint32_t bufferSize = dataPackage->getData (i, buffer, type);

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
		for (uint32_t i = 0; i < dataPackage->getCount (); i++)
		{
			if (dataPackage->getDataType (i) == IDataPackage::kText)
				return S_OK;
		}
	}
	else if (format->cfFormat == CF_PRIVATEFIRST)
	{
		for (uint32_t i = 0; i < dataPackage->getCount (); i++)
		{
			if (dataPackage->getDataType (i) == IDataPackage::kBinary)
				return S_OK;
		}
	}
	else if (format->cfFormat == CF_HDROP)
	{
		for (uint32_t i = 0; i < dataPackage->getCount (); i++)
		{
			if (dataPackage->getDataType (i) == IDataPackage::kFilePath)
				return S_OK;
		}
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
STDMETHODIMP Win32DataObject::EnumFormatEtc (DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
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

//-----------------------------------------------------------------------------
CGradient* CGradient::create (const ColorStopMap& colorStopMap)
{
	return new CGradient (colorStopMap);
}

} // namespace

#endif // WINDOWS
