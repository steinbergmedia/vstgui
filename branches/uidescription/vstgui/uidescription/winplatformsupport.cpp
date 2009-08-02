#include "platformsupport.h"

static TCHAR   gClassName[100];
extern void* hInstance;
inline HINSTANCE GetInstance () { return (HINSTANCE)hInstance; }

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
class Win32Window : public PlatformWindow
{
public:
	Win32Window (const CRect& size, const char* title, WindowType type, long styleFlags, IPlatformWindowDelegate* delegate);
	~Win32Window ();

	void* getPlatformHandle () const;
	void show ();
	void center ();
	CRect getSize ();
	void setSize (const CRect& size);
	
	void runModal ();
	void stopModal ();
protected:
	void registerClass ();
	void getWindowFlags (DWORD& wStyle, DWORD& exStyle);
	static LONG_PTR WINAPI windowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	HWND platformWindow;
	IPlatformWindowDelegate* delegate;
	WindowType type;
	long styleFlags;
	bool recursiveGuard;
	bool runModalMode;
};

//-----------------------------------------------------------------------------
PlatformWindow* PlatformWindow::create (const CRect& size, const char* title, WindowType type, long styleFlags, IPlatformWindowDelegate* delegate)
{
	return new Win32Window (size, title, type, styleFlags, delegate);
}

//-----------------------------------------------------------------------------
Win32Window::Win32Window (const CRect& size, const char* title, WindowType type, long styleFlags, IPlatformWindowDelegate* delegate)
: platformWindow (0)
, delegate (delegate)
, type (type)
, styleFlags (styleFlags)
, recursiveGuard (false)
, runModalMode (false)
{
	registerClass ();
	UTF8StringHelper titleStr (title);
	RECT r = {(LONG)size.left, (LONG)size.top, (LONG)size.right, (LONG)size.bottom};
	DWORD exStyle = 0;
	DWORD wStyle = 0;
	getWindowFlags (wStyle, exStyle);
	AdjustWindowRectEx (&r, wStyle, FALSE, exStyle);
	HWND baseWindow = GetTopWindow (0);
#if 0
	while (baseWindow)
	{
		HWND temp = GetWindow (baseWindow, GW_OWNER);
		if (temp == 0)
			break;
		baseWindow = temp;
	}
#endif
	platformWindow = CreateWindowEx (exStyle, gClassName, titleStr, wStyle, r.left, r.top, r.right - r.left, r.bottom - r.top, baseWindow, 0, GetInstance (), 0);
	SetWindowLongPtr (platformWindow, GWLP_USERDATA, (LONG_PTR)this);
}

//-----------------------------------------------------------------------------
Win32Window::~Win32Window ()
{
	if (platformWindow)
		DestroyWindow (platformWindow);
}

//-----------------------------------------------------------------------------
void Win32Window::getWindowFlags (DWORD& wStyle, DWORD& exStyle)
{
	exStyle = WS_EX_COMPOSITED;
	wStyle = WS_CAPTION|WS_CLIPCHILDREN;
	if (type == kPanelType)
	{
		exStyle |= WS_EX_PALETTEWINDOW;
		//wStyle |= WS_OVERLAPPED;
	}
	if (styleFlags & kClosable)
		wStyle |= WS_SYSMENU;
	if (styleFlags & kResizable)
		wStyle |= WS_SIZEBOX;
}

//-----------------------------------------------------------------------------
void Win32Window::registerClass ()
{
	static bool once = true;
	if (once)
	{
		once = false;
		VSTGUI_SPRINTF (gClassName, TEXT("PluginWin32Window%p"), GetInstance ());
		
		WNDCLASS windowClass;
		windowClass.style = CS_DBLCLKS;

		windowClass.lpfnWndProc = windowProc; 
		windowClass.cbClsExtra  = 0; 
		windowClass.cbWndExtra  = 0; 
		windowClass.hInstance   = GetInstance ();
		windowClass.hIcon = 0; 

		windowClass.hCursor = LoadCursor (NULL, IDC_ARROW);
		windowClass.hbrBackground = CreateSolidBrush (RGB(0, 0, 0));
		windowClass.lpszMenuName  = 0; 
		windowClass.lpszClassName = gClassName; 
		RegisterClass (&windowClass);
	}
}

//-----------------------------------------------------------------------------
void* Win32Window::getPlatformHandle () const
{
	return platformWindow;
}

//-----------------------------------------------------------------------------
void Win32Window::show ()
{
	ShowWindow (platformWindow, SW_SHOW);
}

//-----------------------------------------------------------------------------
void Win32Window::center ()
{
	HMONITOR monitor = MonitorFromWindow (platformWindow, MONITOR_DEFAULTTOPRIMARY);
	if (monitor)
	{
		MONITORINFO mi = {0};
		mi.cbSize = sizeof (MONITORINFO);
		if (GetMonitorInfo (monitor, &mi))
		{
			CRect r = getSize ();
			r.offset (-r.left, -r.top);
			r.offset (((mi.rcWork.right - mi.rcWork.left) / 2) - r.getWidth () / 2, ((mi.rcWork.bottom - mi.rcWork.top) / 2) - r.getHeight () / 2);
			setSize (r);
		}
	}
}

//-----------------------------------------------------------------------------
CRect Win32Window::getSize ()
{
	POINT p = {0};
	RECT client;
	GetClientRect (platformWindow, &client);
	ClientToScreen (platformWindow, &p);

	CRect result (client.left, client.top, client.right, client.bottom);
	result.offset (p.x, p.y);
	return result;
}

//-----------------------------------------------------------------------------
void Win32Window::setSize (const CRect& size)
{
	DWORD exStyle = 0;
	DWORD wStyle = 0;
	getWindowFlags (wStyle, exStyle);
	RECT r = {(LONG)size.left, (LONG)size.top, (LONG)size.right, (LONG)size.bottom};
	AdjustWindowRectEx (&r, wStyle, FALSE, exStyle);

	SetWindowPos (platformWindow, 0, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOACTIVATE | SWP_NOREPOSITION | SWP_NOZORDER | SWP_DEFERERASE);
	if (delegate)
		delegate->windowSizeChanged (size, this);
}

//-----------------------------------------------------------------------------
void Win32Window::runModal ()
{
	if (runModalMode)
		return;
	runModalMode = true;
	HWND oldCapture = GetCapture ();
	if (oldCapture)
		ReleaseCapture ();
	while (runModalMode)
	{
		MSG msg;
		GetMessage (&msg, 0 , 0, 0);
		if (msg.hwnd == platformWindow || IsChild (platformWindow, msg.hwnd))
		{
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
	}
	if (oldCapture)
		SetCapture (oldCapture);
}

//-----------------------------------------------------------------------------
void Win32Window::stopModal ()
{
	runModalMode = false;
}

//-----------------------------------------------------------------------------
LONG_PTR WINAPI Win32Window::windowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Win32Window* window = (Win32Window*)GetWindowLongPtr (hWnd, GWLP_USERDATA);
	if (window && window->delegate)
	{
		switch (message)
		{
			case WM_SIZE:
			{
				if (!window->recursiveGuard)
				{
					window->recursiveGuard = true;
					CRect r = window->getSize ();
					window->delegate->windowSizeChanged (r, window);
					window->recursiveGuard = false;
				}
				break;
			}
		}
	}
	return DefWindowProc (hWnd, message, wParam, lParam);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool PlatformUtilities::collectPlatformFontNames (std::list<std::string*>& fontNames)
{
	Gdiplus::InstalledFontCollection fonts;
	if (fonts.GetFamilyCount () > 0)
	{
		Gdiplus::FontFamily* families = new Gdiplus::FontFamily[fonts.GetFamilyCount ()];
		INT numFonts = fonts.GetFamilyCount ();
		if (fonts.GetFamilies (fonts.GetFamilyCount (), families, &numFonts) == Gdiplus::Ok)
		{
			WCHAR familyName[LF_FACESIZE];
			for (INT i = 0; i < numFonts; i++)
			{
				families[i].GetFamilyName (familyName);
				UTF8StringHelper str (familyName);
				fontNames.push_back (new std::string (str));
			}
		}
		delete [] families;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool PlatformUtilities::startDrag (CFrame* frame, const CPoint& location, const char* string, CBitmap* dragBitmap, bool localOnly)
{
	return false;
}

//-----------------------------------------------------------------------------
void PlatformUtilities::colorChooser (const CColor* oldColor, IPlatformColorChangeCallback* callback)
{
}

END_NAMESPACE_VSTGUI
