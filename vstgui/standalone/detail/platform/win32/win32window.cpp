#include "win32window.h"
#include "../iplatformwindow.h"
#include "../../application.h"
#include "../../../iapplication.h"
#include "../../../iappdelegate.h"
#include "../../../../lib/platform/win32/winstring.h"
#include "../../../../lib/cvstguitimer.h"
#include "../../../../lib/platform/win32/direct2d/d2ddrawcontext.h"
#include "../../../../lib/platform/win32/win32frame.h"
#include <Windows.h>
#include <windowsx.h>
#include <Dwmapi.h>
#include <d2d1.h>
#include <ShellScalingAPI.h>

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
class Window : public IWindow, public IWin32Window, public std::enable_shared_from_this<Window>
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
	
	PlatformType getPlatformType () const override { return PlatformType::kHWNDTopLevel; }
	void* getPlatformHandle () const override { return hwnd; }
	void onSetContentView (CFrame* frame) override;

	void updateCommands () const override;
	void onQuit () override;
	LRESULT CALLBACK proc (UINT message, WPARAM wParam, LPARAM lParam);
private:
	void setNewDPI (uint32_t newDpi);
	void handleMenuCommand (const UTF8String& group, UINT index);
	void windowWillClose ();
	void registerWindowClasses ();
	bool isVisible () const { return IsWindowVisible (hwnd) != 0; }

	HWND hwnd {nullptr};
	mutable HMENU mainMenu {nullptr};
	IWindowDelegate* delegate {nullptr};
	CFrame* frame {nullptr};
	CPoint initialSize;
	double dpiScale {1.};
	DWORD exStyle {};
	DWORD dwStyle {};
	bool hasMenu {false};
	bool hasBorder {false};
	bool isTransparent {false};
	bool isPopup {false};
	std::function<LRESULT (HWND, UINT, WPARAM, LPARAM)> frameWindowProc;
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

	wcex.style = CS_DBLCLKS; // Don't use CS_HREDRAW or CS_VREDRAW with a Ribbon
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

	if (config.type == WindowType::Popup)
	{
		isPopup = true;
		exStyle = WS_EX_COMPOSITED | WS_EX_TRANSPARENT;
		dwStyle = WS_POPUP;
		if (config.style.hasBorder ())
		{
			if (config.style.canSize ())
				dwStyle |= WS_SIZEBOX | WS_MAXIMIZEBOX;
			if (config.style.canClose ())
				dwStyle |= WS_CAPTION | WS_SYSMENU;
		}
	}
	else
	{
		exStyle = WS_EX_APPWINDOW;
		dwStyle = 0;
		if (config.style.hasBorder ())
		{
			if (config.style.canSize ())
				dwStyle |= WS_SIZEBOX | WS_MAXIMIZEBOX;
			if (config.style.canClose ())
				dwStyle |= WS_CAPTION | WS_SYSMENU;
		}
		else
		{
			dwStyle = WS_POPUP;// | WS_THICKFRAME | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
			exStyle = WS_EX_COMPOSITED | WS_EX_TRANSPARENT;
		}
	}
	initialSize = config.size;
	auto winStr = dynamic_cast<WinString*> (config.title.getPlatformString ());
	hwnd = CreateWindowEx (exStyle, gWindowClassName, winStr ? winStr->getWideString () : nullptr, dwStyle, 0, 0,
						   500, 500,
						   nullptr, nullptr, getHInstance (), nullptr);
	if (!hwnd)
		return false;


	delegate = &inDelegate;
	SetWindowLongPtr (hwnd, GWLP_USERDATA, (__int3264) (LONG_PTR) this);
	if (hasBorder)
	{
		hasMenu = true;
		updateCommands ();
	}
	return true;
}

//------------------------------------------------------------------------
void Window::onSetContentView (CFrame* inFrame)
{
	frame = inFrame;
	if (frame)
	{
		auto win32Frame = dynamic_cast<Win32Frame*> (frame->getPlatformFrame ());
		frameWindowProc = [win32Frame] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
			return win32Frame->proc (hwnd, message, wParam, lParam);
		};
	}
	else
		frameWindowProc = nullptr;
}

//------------------------------------------------------------------------
void Window::setNewDPI (uint32_t newDpi)
{
	CPoint size = getSize ();
	dpiScale = static_cast<double> (newDpi) * (100. / 96.) / 100.;
	setSize (size);
	if (frame)
		frame->setZoom (dpiScale);
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
	if (!hasMenu)
		return;

	// TODO: cleanup memory leaks
	if (mainMenu)
		DestroyMenu (mainMenu);
	mainMenu = CreateMenu ();
	MENUINFO info {};
	info.cbSize = sizeof (MENUINFO);
	info.dwStyle = MNS_NOTIFYBYPOS;
	info.fMask = MIM_STYLE;
	SetMenuInfo (mainMenu, &info);

	const auto& appInfo = IApplication::instance ().getDelegate ().getInfo ();

	auto& commandList = getApplicationPlatformAccess ()->getCommandList ();
	for (auto& e : commandList)
	{
		auto subMenu = createSubMenu (e.first, e.second);
		if (subMenu)
		{
			auto isAppGroup = e.first == CommandGroup::Application;
			auto menuTitle = dynamic_cast<WinString*> (
			    isAppGroup ? appInfo.name.getPlatformString () :e.first.getPlatformString ());
			AppendMenu (mainMenu, MF_STRING | MF_POPUP | MF_ENABLED, (UINT_PTR)subMenu,
			            menuTitle->getWideString ());
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
	auto self = shared_from_this (); // make sure we live as long as this method executes
	switch (message)
	{
		case WM_MOVE:
		{
			delegate->onPositionChanged (getPosition ());
			break;
		}
		case WM_SIZE:
		{
			auto size = getSize ();
			delegate->onSizeChanged (size);
			if (frame)
			{
				size.x *= dpiScale;
				size.y *= dpiScale;
				frame->setSize (size.x, size.y);
			}
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
			return 1;
		}
		case WM_DESTROY:
		{
			vstgui_assert (false, "Should not be called!");
			return 1;
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
		case WM_NCHITTEST:
		{
			if (!hasBorder)
			{
				LONG x = GET_X_LPARAM (lParam);
				LONG y = GET_Y_LPARAM (lParam);
				POINT p {x, y};
				ScreenToClient (hwnd, &p);
				CPoint where {static_cast<CCoord> (p.x), static_cast<CCoord> (p.y)};
				frame->getTransform ().inverse ().transform (where);
				if (!frame->hitTestSubViews (where))
				{
					DebugPrint ("HTCaption(%d-%d)\n", p.x, p.y);
					return HTCAPTION;
				}
				return HTCLIENT;
			}
			break;
		}
		case WM_DPICHANGED:
		{
			setNewDPI (static_cast<uint32_t> (LOWORD (wParam)));
			break;
		}
	}
	if (frameWindowProc)
		return frameWindowProc (hwnd, message, wParam, lParam);
	return DefWindowProc (hwnd, message, wParam, lParam);
}

//------------------------------------------------------------------------
CPoint Window::getSize () const
{
	if (!isVisible ())
		return initialSize;
	RECT r;
	GetClientRect (hwnd, &r);
	return CPoint ((r.right - r.left) / dpiScale, (r.bottom - r.top) / dpiScale);
}

//------------------------------------------------------------------------
CPoint Window::getPosition () const
{
	POINT p {0, 0};
	MapWindowPoints (hwnd, nullptr, &p, 1);
	return {static_cast<CCoord> (p.x), static_cast<CCoord> (p.y)};
}

//------------------------------------------------------------------------
void Window::setSize (const CPoint& newSize)
{
	if (!isVisible ())
	{
		initialSize = newSize;
		return;
	}
	RECT clientRect {};
	clientRect.right = static_cast<LONG> (newSize.x * dpiScale);
	clientRect.bottom = static_cast<LONG> (newSize.y * dpiScale);
	AdjustWindowRectEx (&clientRect, dwStyle, hasMenu, exStyle);
	
	LONG width = clientRect.right - clientRect.left;
	LONG height = clientRect.bottom - clientRect.top;	
	SetWindowPos (hwnd, HWND_TOP, 0, 0, width, height,
				  SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOACTIVATE);
}

//------------------------------------------------------------------------
void Window::setPosition (const CPoint& newPosition)
{
	RECT clientRect {};
	clientRect.right = 100;
	clientRect.bottom = 100;
	AdjustWindowRectEx (&clientRect, dwStyle, hasMenu, exStyle);

	clientRect.left += static_cast<LONG> (newPosition.x);
	clientRect.top += static_cast<LONG> (newPosition.y);
	SetWindowPos (hwnd, HWND_TOP, clientRect.left, clientRect.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
}

//------------------------------------------------------------------------
void Window::setTitle (const UTF8String& newTitle)
{
}

//------------------------------------------------------------------------
void Window::show ()
{
	auto monitor = MonitorFromWindow (hwnd, MONITOR_DEFAULTTONEAREST);
	UINT x,y;
	GetDpiForMonitor (monitor, MDT_EFFECTIVE_DPI, &x, &y);
	setNewDPI (x);

	RECT clientRect {};
	clientRect.right = static_cast<LONG> (initialSize.x * dpiScale);
	clientRect.bottom = static_cast<LONG> (initialSize.y * dpiScale);
	AdjustWindowRectEx (&clientRect, dwStyle, hasMenu, exStyle);
	
	LONG width = clientRect.right - clientRect.left;
	LONG height = clientRect.bottom - clientRect.top;	
	SetWindowPos (hwnd, HWND_TOP, 0, 0, width, height,
				  SWP_NOMOVE | SWP_NOCOPYBITS | SWP_SHOWWINDOW);
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
		onQuit (); // TODO: rename method !
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
