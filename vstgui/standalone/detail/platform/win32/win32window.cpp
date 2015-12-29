#include "win32window.h"
#include "../iplatformwindow.h"
#include "../../application.h"
#include "../../../iapplication.h"
#include "../../../iappdelegate.h"
#include "../../../../lib/platform/win32/winstring.h"
#include <Windows.h>

extern void* hInstance;

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Win32 {

static const WCHAR* gWindowClassName = L"VSTGUI_Standalone_WindowClass";

static HINSTANCE getHInstance () { return static_cast<HINSTANCE> (hInstance); }

//------------------------------------------------------------------------
static Detail::IApplicationPlatformAccess* getApplicationPlatformAccess ()
{
	return IApplication::instance ().dynamicCast<Detail::IApplicationPlatformAccess> ();
}

//------------------------------------------------------------------------
class Window : public IWindow, public IWin32Window
{
public:
	~Window ();
	bool init (const WindowConfiguration& config, IWindowDelegate& delegate);

	CPoint getSize () const override;
	CPoint getPosition () const override;
	
	void setSize (const CPoint& newSize) override;
	void setPosition (const CPoint& newPosition) override;
	void setTitle (const UTF8String& newTitle) override;
	
	void show () override;
	void hide () override;
	void close () override;
	
	PlatformType getPlatformType () const override { return PlatformType::kHWND; }
	void* getPlatformHandle () const override { return hwnd; }

	void updateCommands () const override;
	LRESULT CALLBACK proc (UINT message, WPARAM wParam, LPARAM lParam);
private:
	void handleMenuCommand (const UTF8String& group, UINT index);
	void windowWillClose ();
	void registerWindowClasses ();

	HWND hwnd {nullptr};
	mutable HMENU mainMenu {nullptr};
	IWindowDelegate* delegate {nullptr};
};

//------------------------------------------------------------------------
static LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Window* window = reinterpret_cast<Window*> ((LONG_PTR)GetWindowLongPtr (hWnd, GWLP_USERDATA));
	if (window)
		return window->proc (message, wParam, lParam);
	return DefWindowProc (hWnd, message, wParam, lParam);
}

//------------------------------------------------------------------------
void Window::registerWindowClasses ()
{
	static bool once = true;
	if (!once)
		return;
	once = true;

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof (WNDCLASSEX);

	wcex.style = 0; // Don't use CS_HREDRAW or CS_VREDRAW with a Ribbon
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = getHInstance ();
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor (NULL, IDC_ARROW);
	wcex.hbrBackground = NULL; // (HBRUSH) (COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = gWindowClassName;
	wcex.hIconSm = NULL;

	RegisterClassEx (&wcex);
}

//------------------------------------------------------------------------
Window::~Window ()
{
	if (hwnd)
	{
		SetWindowLongPtr (hwnd, GWLP_USERDATA, (__int3264) (LONG_PTR) nullptr);
		DestroyWindow (hwnd);
	}
}

//------------------------------------------------------------------------
bool Window::init (const WindowConfiguration& config, IWindowDelegate& inDelegate)
{
	registerWindowClasses ();

	DWORD exStyle = 0;
	DWORD dwStyle = 0;
	if (config.flags.isPopup ())
	{
		return false; // TODO: implement me 
	}
	else
	{
		exStyle = WS_EX_APPWINDOW;
		dwStyle = 0;
		if (config.flags.canSize ())
			dwStyle |= WS_SIZEBOX | WS_MAXIMIZEBOX;
		if (config.flags.canClose ())
			dwStyle |= WS_CAPTION | WS_SYSMENU;
	}

	auto winStr = dynamic_cast<WinString*> (config.title.getPlatformString ());

	hwnd = CreateWindowEx (exStyle, gWindowClassName, winStr ? winStr->getWideString () : nullptr, dwStyle, 0, 0,
						   static_cast<int> (config.size.x), static_cast<int> (config.size.y),
						   nullptr, nullptr, getHInstance (), nullptr);
	if (hwnd)
	{
		SetWindowLongPtr (hwnd, GWLP_USERDATA, (__int3264) (LONG_PTR) this);
	}
	delegate = &inDelegate;
	updateCommands ();
	return true;
}

//------------------------------------------------------------------------
static HMENU createSubMenu (const UTF8String& group, const Detail::IApplicationPlatformAccess::CommandWithKeyList& commands)
{
	// TODO: cleanup memory leaks
	HMENU menu = CreateMenu ();
	MENUINFO info {};
	info.cbSize = sizeof (MENUINFO);
	info.fMask = MIM_MENUDATA;
	info.dwMenuData = reinterpret_cast<ULONG_PTR> (new UTF8String (group));
	SetMenuInfo (menu, &info);

	UINT index = 0;
	for (auto& e : commands)
	{
		auto itemTitle = dynamic_cast<WinString*> (e.name.getPlatformString ());
		AppendMenu (menu, MF_STRING, index, itemTitle->getWideString ());

		++index;
	}
	return menu;
}

//------------------------------------------------------------------------
void Window::updateCommands () const
{
	// TODO: cleanup memory leaks
	mainMenu = CreateMenu ();
	MENUINFO info {};
	info.cbSize = sizeof (MENUINFO);
	info.dwStyle = MNS_NOTIFYBYPOS;
	info.fMask = MIM_STYLE;
	SetMenuInfo (mainMenu, &info);

	auto& commandList = getApplicationPlatformAccess ()->getCommandList ();
	for (auto& e : commandList)
	{
		auto subMenu = createSubMenu (e.first, e.second);
		if (subMenu)
		{
			auto menuTitle = dynamic_cast<WinString*> (e.first.getPlatformString ());
			AppendMenu (mainMenu, MF_STRING|MF_POPUP|MF_ENABLED, (UINT_PTR)subMenu, menuTitle->getWideString ());
		}
	}

	SetMenu (hwnd, mainMenu);
}

//------------------------------------------------------------------------
void Window::handleMenuCommand (const UTF8String& group, UINT index)
{
	auto& commandList = getApplicationPlatformAccess ()->getCommandList ();
	for (auto& e : commandList)
	{
		if (e.first == group)
		{
			auto it = e.second.begin ();
			std::advance (it, index);
			if (it != e.second.end ())
			{
				if (delegate->canHandleCommand (*it))
					delegate->handleCommand (*it);
				else
				{
					if (auto commandHandler = getApplicationPlatformAccess ()->dynamicCast<ICommandHandler> ())
					{
						if (commandHandler->canHandleCommand (*it))
							commandHandler->handleCommand (*it);
					}
				}
			}
			break;
		}
	}
}

//------------------------------------------------------------------------
static CPoint getRectSize (const RECT& r)
{
	return {static_cast<CCoord> (r.right - r.left), static_cast<CCoord> (r.bottom - r.top)};
}

//------------------------------------------------------------------------
LRESULT CALLBACK Window::proc (UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_MOVE:
		{
			delegate->onPositionChanged (getPosition ());
			break;
		}
		case WM_SIZE:
		{
			RECT clientSize;
			GetClientRect (hwnd, &clientSize);
			delegate->onSizeChanged (getRectSize (clientSize));
			break;
		}
		case WM_SIZING:
		{
			RECT* newSize = reinterpret_cast<RECT*> (lParam);
			RECT oldSize;
			GetWindowRect (hwnd, &oldSize);
			RECT clientSize;
			GetClientRect (hwnd, &clientSize);
			
			CPoint diff = getRectSize (*newSize) - getRectSize (oldSize);
			CPoint newClientSize = getRectSize (clientSize) + diff;

			CPoint constraintSize = delegate->constraintSize (newClientSize);
			if (constraintSize != newClientSize)
			{
				CPoint clientFrameDiff = getRectSize (oldSize) - getRectSize (clientSize);
				newSize->right = newSize->left + constraintSize.x + clientFrameDiff.x;
				newSize->bottom = newSize->top + constraintSize.y + clientFrameDiff.y;
				return TRUE;
			}
			break;
		}
		case WM_ACTIVATE:
		{
			WORD action = LOWORD (wParam);
			if (action == WA_ACTIVE || action == WA_CLICKACTIVE)
			{
				delegate->onActivated ();
			}
			else
			{
				delegate->onDeactivated ();
			}
			return 0;
		}
		case WM_CLOSE:
		{
			windowWillClose ();
			break;
		}
		case WM_DESTROY:
		{
			break;
		}
		case WM_MENUCOMMAND:
		{
			HMENU menu = reinterpret_cast<HMENU> (lParam);
			if (menu)
			{
				MENUINFO info {};
				info.cbSize = sizeof (MENUINFO);
				info.fMask = MIM_MENUDATA;
				GetMenuInfo (menu, &info);
				auto group = reinterpret_cast<UTF8String*> (info.dwMenuData);
				if (group)
				{
					handleMenuCommand (*group, wParam);
				}
			}
			break;
		}

	}
	return DefWindowProc (hwnd, message, wParam, lParam);
	
}

//------------------------------------------------------------------------
CPoint Window::getSize () const
{
	RECT r;
	GetClientRect (hwnd, &r);
	return CPoint (r.right - r.left, r.bottom - r.top);
}

//------------------------------------------------------------------------
CPoint Window::getPosition () const
{
	RECT frameRect, clientRect;
	GetWindowRect (hwnd, &frameRect);
	GetClientRect (hwnd, &clientRect);
	CPoint result;
	result.x = frameRect.left - clientRect.left;
	result.y = frameRect.top - clientRect.top;
	return result;
}

//------------------------------------------------------------------------
void Window::setSize (const CPoint& newSize)
{
	RECT frameRect, clientRect;
	GetWindowRect (hwnd, &frameRect);
	GetClientRect (hwnd, &clientRect);
	
	LONG width = newSize.x + ((frameRect.right - frameRect.left) - clientRect.right);
	LONG height = newSize.y + ((frameRect.bottom - frameRect.top) - clientRect.bottom);
	
	SetWindowPos (hwnd, HWND_TOP, 0, 0, width, height,
				  SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOACTIVATE);
}

//------------------------------------------------------------------------
void Window::setPosition (const CPoint& newPosition)
{
	RECT frameRect, clientRect;
	GetWindowRect (hwnd, &frameRect);
	GetClientRect (hwnd, &clientRect);
	LONG x = newPosition.x + (frameRect.left - clientRect.left);
	LONG y = newPosition.y + (frameRect.top - clientRect.top);
	SetWindowPos (hwnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
}

//------------------------------------------------------------------------
void Window::setTitle (const UTF8String& newTitle)
{
}

//------------------------------------------------------------------------
void Window::show ()
{
	ShowWindow (hwnd, true);
	HWND childWindow = GetWindow (hwnd, GW_CHILD);
	if (childWindow)
		SetFocus (childWindow);
}

//------------------------------------------------------------------------
void Window::hide ()
{
	ShowWindow (hwnd, false);
}

//------------------------------------------------------------------------
void Window::close ()
{
	HWND temp = hwnd;
	windowWillClose ();
	CloseWindow (temp);
}

//------------------------------------------------------------------------
void Window::windowWillClose ()
{
	delegate->onClosed ();
	// we are now destroyed ! at least we should !
}

} // Win32

//------------------------------------------------------------------------
WindowPtr makeWindow (const WindowConfiguration& config, IWindowDelegate& delegate)
{
	auto window = std::make_shared<Win32::Window> ();
	if (window->init (config, delegate))
		return window;
	return nullptr;
}

} // Platform
} // Standalone
} // VSTGUI
