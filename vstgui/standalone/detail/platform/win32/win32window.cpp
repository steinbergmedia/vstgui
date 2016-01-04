#include "win32window.h"
#include "../iplatformwindow.h"
#include "../../application.h"
#include "../../../iapplication.h"
#include "../../../iappdelegate.h"
#include "../../../../lib/platform/win32/winstring.h"
#include "../../../../lib/cvstguitimer.h"
#include "../../../../lib/platform/win32/direct2d/d2ddrawcontext.h"
#include <Windows.h>
#include <Dwmapi.h>
#include <d2d1.h>

#pragma comment(lib, "Dwmapi.lib")

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
	void activate () override;
	
	PlatformType getPlatformType () const override { return PlatformType::kHWND; }
	void* getPlatformHandle () const override { return hwnd; }

	void updateCommands () const override;
	void onQuit () override;
	LRESULT CALLBACK proc (UINT message, WPARAM wParam, LPARAM lParam);
private:
	void handleMenuCommand (const UTF8String& group, UINT index);
	void windowWillClose ();
	void registerWindowClasses ();

	HWND hwnd {nullptr};
	mutable HMENU mainMenu {nullptr};
	IWindowDelegate* delegate {nullptr};
	bool hasBorder {false};
	bool isTransparent {false};
	bool isPopup {false};
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

	WNDCLASSEX wcex{};

	wcex.cbSize = sizeof (WNDCLASSEX);

	wcex.style = 0; // Don't use CS_HREDRAW or CS_VREDRAW with a Ribbon
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = getHInstance ();
	wcex.hCursor = LoadCursor (getHInstance (), IDC_ARROW);
	wcex.hbrBackground = nullptr; // (HBRUSH) (COLOR_WINDOW + 1);
	wcex.lpszClassName = gWindowClassName;

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

	if (config.style.hasBorder ())
		hasBorder = true;
	if (config.style.isTransparent ())
		isTransparent = true;

	DWORD exStyle = 0;
	DWORD dwStyle = 0;
	if (config.type == WindowType::Popup)
	{
		isPopup = true;
		exStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
		if (hasBorder && config.style.canSize ())
			dwStyle |= WS_SIZEBOX | WS_MAXIMIZEBOX;
		if (hasBorder && config.style.canClose ())
			dwStyle |= WS_CAPTION | WS_SYSMENU;
	}
	else
	{
		exStyle = WS_EX_APPWINDOW;
		dwStyle = 0;
		if (!config.style.isTransparent ())
		{
			if (hasBorder && config.style.canSize ())
				dwStyle |= WS_SIZEBOX | WS_MAXIMIZEBOX;
			if (hasBorder && config.style.canClose ())
				dwStyle |= WS_CAPTION | WS_SYSMENU;
		}
		else
		{
			dwStyle = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
			exStyle = WS_EX_COMPOSITED | WS_EX_TRANSPARENT;
		}
	}

	auto winStr = dynamic_cast<WinString*> (config.title.getPlatformString ());

	hwnd = CreateWindowEx (exStyle, gWindowClassName, winStr ? winStr->getWideString () : nullptr, dwStyle, 0, 0,
						   static_cast<int> (config.size.x), static_cast<int> (config.size.y),
						   nullptr, nullptr, getHInstance (), nullptr);
	if (!hwnd)
		return false;
	delegate = &inDelegate;
	SetWindowLongPtr (hwnd, GWLP_USERDATA, (__int3264) (LONG_PTR) this);
	if (hasBorder)
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
				if (*it == Commands::About)
				{
					if (IApplication::instance ().getDelegate ().hasAboutDialog ())
					{
						IApplication::instance ().getDelegate ().showAboutDialog ();
						return;
					}
				}
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
			delegate->onSizeChanged (getSize ());
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
				newSize->right = newSize->left + static_cast<LONG> (constraintSize.x + clientFrameDiff.x);
				newSize->bottom = newSize->top + static_cast<LONG> (constraintSize.y + clientFrameDiff.y);
				return TRUE;
			}
			break;
		}
		case WM_ACTIVATE:
		{
			if (!hasBorder)
			{
				if (isTransparent)
				{
					DWM_BLURBEHIND bb = {};
					bb.dwFlags = DWM_BB_ENABLE;
					bb.fEnable = true;
					DwmEnableBlurBehindWindow (hwnd, &bb);
				}

				MARGINS margins = {0, 0, 0, 0};
				auto res = DwmExtendFrameIntoClientArea (hwnd, &margins);
				vstgui_assert (res == S_OK);
			}

			WORD action = LOWORD (wParam);
			if (action == WA_ACTIVE || action == WA_CLICKACTIVE)
			{
				delegate->onActivated ();
			}
			else
			{
				delegate->onDeactivated ();
				if (isPopup)
				{
					close ();
				}
			}
			return 0;
		}
		case WM_NCCALCSIZE:
		{
			if (!hasBorder)
			{
				return 0;
			}
			break;
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
		case WM_ERASEBKGND:
		{
			if (isTransparent)
			{
				CRect r;
				r.setSize (getSize ());
				D2DDrawContext context (hwnd, r);
				context.beginDraw ();
				context.clearRect (r);
				context.endDraw ();
				return true;
			}
			break;
		}
		case WM_PAINT:
		{
			if (isTransparent)
			{
				return true;
			}
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
					handleMenuCommand (*group, static_cast<UINT> (wParam));
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
	
	LONG width = static_cast<LONG> (newSize.x) + ((frameRect.right - frameRect.left) - clientRect.right);
	LONG height = static_cast<LONG> (newSize.y) + ((frameRect.bottom - frameRect.top) - clientRect.bottom);
	
	SetWindowPos (hwnd, HWND_TOP, 0, 0, width, height,
				  SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOACTIVATE);
}

//------------------------------------------------------------------------
void Window::setPosition (const CPoint& newPosition)
{
	RECT frameRect, clientRect;
	GetWindowRect (hwnd, &frameRect);
	GetClientRect (hwnd, &clientRect);
	LONG x = static_cast<LONG> (newPosition.x) + (frameRect.left - clientRect.left);
	LONG y = static_cast<LONG> (newPosition.y) + (frameRect.top - clientRect.top);
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
	auto call = [this] () {
		HWND temp = hwnd;
		windowWillClose ();
		CloseWindow (temp);
	};
	Call::later (call);
}

//------------------------------------------------------------------------
void Window::onQuit ()
{
	HWND temp = hwnd;
	windowWillClose ();
	CloseWindow (temp);
}

//------------------------------------------------------------------------
void Window::activate ()
{
	BringWindowToTop (hwnd);
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
