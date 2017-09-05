// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32window.h"
#include "win32menu.h"
#include "win32async.h"
#include "../../../../lib/platform/win32/direct2d/d2ddrawcontext.h"
#include "../../../../lib/platform/win32/win32frame.h"
#include "../../../../lib/platform/win32/winstring.h"
#include "../../../include/iappdelegate.h"
#include "../../../include/iapplication.h"
#include "../../application.h"
#include "../iplatformwindow.h"
#include <Dwmapi.h>
#include <ShellScalingAPI.h>
#include <VersionHelpers.h>
#include <Windows.h>
#include <d2d1.h>
#include <windowsx.h>

#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "Comctl32.lib")

extern void* hInstance;

//------------------------------------------------------------------------
struct User32Lib
{
	static User32Lib& instance ()
	{
		static User32Lib gInstance;
		return gInstance;
	}

	template<typename T>
	T getProcAddress (const char* name)
	{
		return reinterpret_cast<T> (GetProcAddress (module, name));
	}

	bool enableNonClientDpiScaling (HWND window)
	{
		if (enableNonClientDpiScalingFunc)
			return enableNonClientDpiScalingFunc (window);
		return false;
	}

private:
	using EnableNonClientDpiScalingFunc = BOOL (WINAPI*) (_In_ HWND hwnd);

	User32Lib ()
	{
		module = LoadLibrary (L"user32.dll");
		enableNonClientDpiScalingFunc = getProcAddress<EnableNonClientDpiScalingFunc> ("EnableNonClientDpiScaling");
	}

	EnableNonClientDpiScalingFunc enableNonClientDpiScalingFunc;
	HMODULE module;
};

//------------------------------------------------------------------------
struct WindowComposition
{
	WindowComposition ()
	{
		auto user32lib = User32Lib::instance ();
		get = user32lib.getProcAddress<GetProc> ("GetWindowCompositionAttribute");
		set = user32lib.getProcAddress<SetProc> ("SetWindowCompositionAttribute");
	}

	bool setWindowTransparent (HWND hwnd)
	{
		// TODO: need manifest file to actually get the running version of Windows 8.1 or greater
		auto accentState = IsWindowsVersionOrGreater (10, 0, 0) ?
		                       AccentState::ACCENT_ENABLE_TRANSPARENT :
		                       AccentState::ACCENT_ENABLE_BLURBEHIND;
		return setAccentState (hwnd, accentState);
	}

//------------------------------------------------------------------------
private:
	enum AccentState
	{
		ACCENT_DISABLED = 0,
		ACCENT_ENABLE_GRADIENT = 1,
		ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
		ACCENT_ENABLE_BLURBEHIND = 3, // use on Windows 8
		ACCENT_ENABLE_TRANSPARENT = 4, // use on Windows 10
		ACCENT_INVALID_STATE = 5
	};

	struct AccentPolicy
	{
		AccentState AccentState;
		int32_t AccentFlags;
		int32_t GradientColor;
		int32_t AnimationId;
	};

	enum Attribute
	{
		// ...
		WCA_ACCENT_POLICY = 19
		// ...
	};

	struct Data
	{
		Attribute attribute;
		PVOID pData;
		ULONG dataSize;
	};

	typedef HRESULT (WINAPI* GetProc) (HWND, Data*);
	typedef HRESULT (WINAPI* SetProc) (HWND, Data*);

	GetProc get = nullptr;
	SetProc set = nullptr;

	bool setAccentState (HWND hwnd, AccentState state)
	{
		if (set)
		{
			AccentPolicy policy {};
			policy.AccentState = state;
			Data d;
			d.attribute = Attribute::WCA_ACCENT_POLICY;
			d.dataSize = sizeof (AccentPolicy);
			d.pData = &policy;
			auto result = set (hwnd, &d);
			return result == 0;
		}
		return false;
	}
};
static WindowComposition gWindowComposition;

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace Win32 {

static const WCHAR* gWindowClassName = L"VSTGUI Standalone WindowClass";

static HINSTANCE getHInstance ()
{
	return static_cast<HINSTANCE> (hInstance);
}

static LRESULT CALLBACK childWindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam,
                                         UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

//------------------------------------------------------------------------
class Window : public IWin32Window, public std::enable_shared_from_this<Window>
{
public:
	Window () = default;
	~Window ();
	bool init (const WindowConfiguration& config, IWindowDelegate& delegate);

	CPoint getSize () const override;
	CPoint getPosition () const override;
	double getScaleFactor () const override;

	void setSize (const CPoint& newSize) override;
	void setPosition (const CPoint& newPosition) override;
	void setTitle (const UTF8String& newTitle) override;

	void show () override;
	void hide () override;
	void close () override;
	void activate () override;
	void center () override;

	PlatformType getPlatformType () const override { return PlatformType::kHWNDTopLevel; }
	void* getPlatformHandle () const override { return hwnd; }
	void onSetContentView (CFrame* frame) override;

	void updateCommands () const override;
	void onQuit () override;
	HWND getHWND () const override { return hwnd; }
	void setModalWindow (const VSTGUI::Standalone::WindowPtr& window) override
	{
		modalWindow = window;
	}

	bool nonClientHitTest (LPARAM& lParam, LRESULT& result);
	LRESULT CALLBACK proc (UINT message, WPARAM wParam, LPARAM lParam);

	enum class HandleCommandResult
	{
		CommandHandled,
		CommandRejected,
		CommandUnknown
	};
	HandleCommandResult handleCommand (const WORD& cmdID);

private:
	void makeTransparent ();
	void updateDPI ();
	void setNewDPI (uint32_t newDpi);
	void validateMenu (Win32Menu* menu);
	void handleMenuCommand (const UTF8String& group, const UTF8String& name);
	void windowWillClose ();
	void registerWindowClasses ();
	bool isVisible () const { return IsWindowVisible (hwnd) != 0; }

	HWND hwnd {nullptr};
	VSTGUI::Standalone::WindowPtr modalWindow;
	mutable std::shared_ptr<Win32Menu> mainMenu;
	IWindowDelegate* delegate {nullptr};
	CFrame* frame {nullptr};
	mutable Detail::IPlatformApplication::CommandList menuCommandList;
	CPoint initialSize;
	double dpiScale {1.};
	DWORD exStyle {};
	DWORD dwStyle {};
	bool hasMenu {false};
	bool hasBorder {false};
	bool isTransparent {false};
	bool isPopup {false};
	bool movableByWindowBackground {false};
	bool sizable {false};
	std::function<LRESULT (HWND, UINT, WPARAM, LPARAM)> frameWindowProc;
};

//------------------------------------------------------------------------
static LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Window* window = reinterpret_cast<Window*> ((LONG_PTR)GetWindowLongPtr (hWnd, GWLP_USERDATA));
	if (window)
		return window->proc (message, wParam, lParam);
	if (message == WM_NCCREATE)
		User32Lib::instance ().enableNonClientDpiScaling (hWnd);
	return DefWindowProc (hWnd, message, wParam, lParam);
}

//------------------------------------------------------------------------
void Window::registerWindowClasses ()
{
	static bool once = true;
	if (!once)
		return;
	once = true;

	WNDCLASSEX wcex {};

	wcex.cbSize = sizeof (WNDCLASSEX);

	wcex.style = CS_DBLCLKS;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = getHInstance ();
	wcex.hCursor = LoadCursor (getHInstance (), IDC_ARROW);
	wcex.hbrBackground = nullptr;
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
		mainMenu.reset ();
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
	if (config.style.isMovableByWindowBackground ())
		movableByWindowBackground = true;
	if (config.style.canSize ())
		sizable = true;

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
				dwStyle |= WS_SIZEBOX | WS_MAXIMIZEBOX; // TODO: WS_MINIMIZEBOX
			if (config.style.canClose ())
				dwStyle |= WS_CAPTION | WS_SYSMENU;
		}
		else
		{
			dwStyle = WS_POPUP | WS_CLIPCHILDREN;
			exStyle = WS_EX_COMPOSITED | WS_EX_TRANSPARENT;
		}
	}
	initialSize = config.size;
	auto winStr = dynamic_cast<WinString*> (config.title.getPlatformString ());
	hwnd = CreateWindowEx (exStyle, gWindowClassName, winStr ? winStr->getWideString () : nullptr,
	                       dwStyle, 0, 0, 500, 500, nullptr, nullptr, getHInstance (), nullptr);
	if (!hwnd)
		return false;

	delegate = &inDelegate;
	SetWindowLongPtr (hwnd, GWLP_USERDATA, (__int3264) (LONG_PTR) this);
	if (hasBorder)
		hasMenu = true;
	if (isTransparent)
		makeTransparent ();
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
		updateDPI ();
	}
	else
		frameWindowProc = nullptr;
}

//------------------------------------------------------------------------
void Window::setNewDPI (uint32_t newDpi)
{
	CPoint size = getSize ();
	dpiScale = static_cast<double> (newDpi) * (100. / 96.) / 100.;
	if (frame)
		frame->setZoom (dpiScale);
	setSize (size);
}

//------------------------------------------------------------------------
static std::shared_ptr<Win32Menu> createSubMenu (const UTF8String& group,
                            const Detail::IPlatformApplication::CommandWithKeyList& commands)
{
	auto menu = std::make_shared<Win32Menu> (group);
	for (auto& e : commands)
	{
		if (e.name == CommandName::MenuSeparator)
			menu->addSeparator ();
		else
			menu->addItem (e.name, e.defaultKey, e.id);
	}
	return menu;
}

//------------------------------------------------------------------------
void Window::updateCommands () const
{
	if (!hasMenu)
		return;

	menuCommandList = Detail::getApplicationPlatformAccess ()->getCommandList (this);
	if (menuCommandList.empty ())
	{
		if (mainMenu)
			SetMenu (hwnd, nullptr);
		mainMenu.reset ();
		return;
	}

	mainMenu = std::make_shared<Win32Menu> ("");

	std::shared_ptr<Win32Menu> fileMenu = nullptr;
	const Detail::IPlatformApplication::CommandWithKeyList* appCommands = nullptr;

	for (auto& e : menuCommandList)
	{
		if (e.first == CommandGroup::Application)
		{
			appCommands = &e.second;
			continue;
		}
		auto subMenu = createSubMenu (e.first, e.second);
		if (subMenu)
		{
			mainMenu->addSubMenu (subMenu);
			if (e.first == CommandGroup::File)
				fileMenu = subMenu;
		}
	}
	if (appCommands)
	{
		auto menu = std::make_shared<Win32Menu> ("Help");
		for (auto& e : *appCommands)
		{
			if (fileMenu)
			{
				if (e.name == CommandName::Preferences)
				{
					fileMenu->addSeparator ();
					fileMenu->addItem (e.name, e.defaultKey, e.id);
					continue;
				}
				if (e.name == CommandName::Quit)
				{
					fileMenu->addSeparator ();
					fileMenu->addItem ("Exit", e.defaultKey, e.id);
					continue;
				}
			}
			if (e.name == CommandName::MenuSeparator)
			{
				menu->addSeparator ();
			}
			else
			{
				menu->addItem (e.name, e.defaultKey, e.id);
			}
		}
		mainMenu->addSubMenu (menu);
	}
	SetMenu (hwnd, *mainMenu);
}

//------------------------------------------------------------------------
Command mapCommand (const Command& cmd)
{
	Command mappedCommand = cmd;
	if (cmd.group == "Help")
	{
		mappedCommand.group = CommandGroup::Application;
	}
	else if (cmd.group == CommandGroup::File)
	{
		if (cmd.name == "Exit")
		{
			mappedCommand = Commands::Quit;
		}
		else if (cmd.name == CommandName::Preferences)
		{
			mappedCommand = Commands::Preferences;
		}
	}
	return mappedCommand;
}

//------------------------------------------------------------------------
void Window::validateMenu (Win32Menu* menu)
{
	auto appCommandHandler = Detail::getApplicationPlatformAccess ();
	Command command;
	command.group = menu->title;
	menu->validateMenuItems ([&] (auto& item) {
		if (auto subMenu = item.asMenu ())
		{
			validateMenu (subMenu);
		}
		else
		{
			command.name = item.title;
			auto cmd = mapCommand (command);
			auto canHandle = delegate->canHandleCommand (cmd);
			if (!canHandle)
				canHandle = appCommandHandler->canHandleCommand (cmd);
			canHandle ? item.enable () : item.disable ();
			return true;
		}
		return false;
	});
}

//------------------------------------------------------------------------
void Window::handleMenuCommand (const UTF8String& group, const UTF8String& name)
{
	Command command = mapCommand ({group, name});
	if (delegate->canHandleCommand (command))
		delegate->handleCommand (command);
	else
	{
		if (auto commandHandler = Detail::getApplicationPlatformAccess ())
		{
			if (commandHandler->canHandleCommand (command))
				commandHandler->handleCommand (command);
		}
	}
}

//------------------------------------------------------------------------
static CPoint getRectSize (const RECT& r)
{
	return {static_cast<CCoord> (r.right - r.left), static_cast<CCoord> (r.bottom - r.top)};
}

//------------------------------------------------------------------------
void Window::makeTransparent ()
{
	gWindowComposition.setWindowTransparent (hwnd);
}

//------------------------------------------------------------------------
LRESULT CALLBACK Window::proc (UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!delegate)
		return DefWindowProc (hwnd, message, wParam, lParam);
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

			frame->getTransform ().inverse ().transform (newClientSize);
			CPoint constraintSize = delegate->constraintSize (newClientSize);
			if (constraintSize != newClientSize)
			{
				frame->getTransform ().transform (constraintSize);
				CPoint clientFrameDiff = getRectSize (oldSize) - getRectSize (clientSize);
				newSize->right =
				    newSize->left + static_cast<LONG> (constraintSize.x + clientFrameDiff.x);
				newSize->bottom =
				    newSize->top + static_cast<LONG> (constraintSize.y + clientFrameDiff.y);
				return TRUE;
			}
			break;
		}
		case WM_MOUSEACTIVATE:
		{
			if (modalWindow)
			{
				return MA_NOACTIVATEANDEAT;
			}
			break;
		}
		case WM_ACTIVATE:
		{
			WORD action = LOWORD (wParam);
			if (modalWindow && (action == WA_ACTIVE || action == WA_CLICKACTIVE))
			{
				modalWindow->activate ();
				return 0;
			}
			if (!hasBorder)
			{
				MARGINS margins = {0};
				auto res = DwmExtendFrameIntoClientArea (hwnd, &margins);
				vstgui_assert (res == S_OK);
			}

			if (action == WA_ACTIVE || action == WA_CLICKACTIVE)
			{
				SetFocus (hwnd);
				delegate->onActivated ();
			}
			else
			{
				delegate->onDeactivated ();
				if (isPopup)
				{
					if (!Detail::getApplicationPlatformAccess ()->dontClosePopupOnDeactivation (this))
						close ();
				}
			}
			return 0;
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
		case WM_INITMENUPOPUP:
		{
			if (wParam == 0)
				return 0;
			auto menu = Win32Menu::fromHMENU (reinterpret_cast<HMENU> (wParam));
			if (!menu)
				return 0;
			validateMenu (menu);
			return 0;
		}
		case WM_MENUCOMMAND:
		{
			if (lParam == 0)
				return 0;
			auto menu = Win32Menu::fromHMENU (reinterpret_cast<HMENU> (lParam));
			if (!menu)
				return 0;
			auto index = static_cast<size_t> (wParam);
			if (const auto& item = menu->itemAtIndex (index))
			{
				const auto& group = menu->title;
				handleMenuCommand (group, item->title);
			}
			break;
		}
		case WM_COMMAND:
		{
			if (HIWORD (wParam) == EN_CHANGE)
				break;
			auto cmdID = LOWORD (wParam);
			auto result = handleCommand (cmdID);
			switch (result)
			{
				case HandleCommandResult::CommandHandled: return 1;
				case HandleCommandResult::CommandRejected: return 0;
				case HandleCommandResult::CommandUnknown: break;
			}
			break;
		}
		case WM_NCACTIVATE:
		{
			if (frame)
			{
				auto fc = static_cast<IPlatformFrameCallback*> (frame);
				fc->platformOnWindowActivate (wParam ? true : false);
			}
			break;
		}
		case WM_NCCALCSIZE:
		{
			if (!hasBorder)
			{
				return 0;
			}
			break;
		}
		case WM_NCHITTEST:
		{
			LRESULT res {};
			if (nonClientHitTest (lParam, res))
				return res;
			break;
		}
		case WM_DPICHANGED:
		{
			setNewDPI (static_cast<uint32_t> (LOWORD (wParam)));
			break;
		}
		case WM_PARENTNOTIFY:
		{
			switch (LOWORD (wParam))
			{
				case WM_CREATE:
				{
					auto child = reinterpret_cast<HWND> (lParam);
					SetWindowSubclass (child, childWindowProc, 0,
					                   reinterpret_cast<DWORD_PTR> (this));
					break;
				}
				case WM_DESTROY:
				{
					auto child = reinterpret_cast<HWND> (lParam);
					RemoveWindowSubclass (child, childWindowProc, 0);
					break;
				}
			}
			break;
		}
	}
	if (frameWindowProc)
		return frameWindowProc (hwnd, message, wParam, lParam);
	return DefWindowProc (hwnd, message, wParam, lParam);
}

//------------------------------------------------------------------------
auto Window::handleCommand (const WORD& cmdID) -> HandleCommandResult
{
	auto app = Detail::getApplicationPlatformAccess ();
	for (const auto& grp : app->getKeyCommandList ())
	{
		for (const auto& e : grp.second)
		{
			if (e.id == cmdID)
			{
				if (delegate->canHandleCommand (e))
				{
					if (delegate->handleCommand (e))
						return HandleCommandResult::CommandHandled;
				}
				else
				{
					if (auto commandHandler = Detail::getApplicationPlatformAccess ())
					{
						if (commandHandler->canHandleCommand (e))
						{
							if (commandHandler->handleCommand (e))
								return HandleCommandResult::CommandHandled;
						}
					}
				}
				return HandleCommandResult::CommandRejected;
			}
		}
	}
	return HandleCommandResult::CommandUnknown;
}

//------------------------------------------------------------------------
LRESULT CALLBACK childWindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam,
                                  UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	auto window = reinterpret_cast<Window*> (dwRefData);
	if (message == WM_PARENTNOTIFY)
	{
		switch (LOWORD (wParam))
		{
			case WM_CREATE:
			{
				auto child = reinterpret_cast<HWND> (lParam);
				SetWindowSubclass (child, childWindowProc, 0, reinterpret_cast<DWORD_PTR> (window));
				break;
			}
			case WM_DESTROY:
			{
				auto child = reinterpret_cast<HWND> (lParam);
				RemoveWindowSubclass (child, childWindowProc, 0);
				break;
			}
			default: return DefSubclassProc (hWnd, message, wParam, lParam);
		}
	}
	else if (message == WM_NCHITTEST)
	{
		if (window)
		{
			LRESULT res {};
			if (window->nonClientHitTest (lParam, res) && res != HTCLIENT)
				return HTTRANSPARENT;
		}
	}
	else if (message == WM_COMMAND && HIWORD (wParam) != EN_CHANGE)
	{
		auto cmdID = LOWORD (wParam);
		auto result = window->handleCommand (cmdID);
		switch (result)
		{
			case Window::HandleCommandResult::CommandHandled: return 1;
			case Window::HandleCommandResult::CommandRejected: return 0;
			case Window::HandleCommandResult::CommandUnknown: break;
		}
	}
	return DefSubclassProc (hWnd, message, wParam, lParam);
}

//------------------------------------------------------------------------
bool Window::nonClientHitTest (LPARAM& lParam, LRESULT& result)
{
	if (movableByWindowBackground)
	{
		LONG x = GET_X_LPARAM (lParam);
		LONG y = GET_Y_LPARAM (lParam);
		POINT p {x, y};
		auto size = getSize ();
		frame->getTransform ().transform (size);
		if (ScreenToClient (hwnd, &p) && p.y > 0 && p.x > 0 && p.y < size.y && p.x < size.x)
		{
			if (sizable && !hasBorder)
			{
				const auto edgeSizeWidth = 5 * dpiScale;
				if (p.x > size.x - edgeSizeWidth)
				{
					if (p.y > size.y - edgeSizeWidth)
						result = HTBOTTOMRIGHT;
					else if (p.y < edgeSizeWidth)
						result = HTTOPRIGHT;
					else
						result = HTRIGHT;
					return true;
				}
				if (p.x < edgeSizeWidth)
				{
					if (p.y > size.y - edgeSizeWidth)
						result = HTBOTTOMLEFT;
					else if (p.y < edgeSizeWidth)
						result = HTTOPLEFT;
					else
						result = HTLEFT;
					return true;
				}
				if (p.y < edgeSizeWidth)
				{
					result = HTTOP;
					return true;
				}
				if (p.y > size.y - edgeSizeWidth)
				{
					result = HTBOTTOM;
					return true;
				}
				// TODO: add other edges
			}
			CPoint where {static_cast<CCoord> (p.x), static_cast<CCoord> (p.y)};
			if (!frame->hitTestSubViews (where))
			{
				result = HTCAPTION;
				return true;
			}
			result = HTCLIENT;
			return true;
		}
	}
	return false;
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
	SetWindowPos (hwnd, HWND_TOP, clientRect.left, clientRect.top, 0, 0,
	              SWP_NOSIZE | SWP_NOACTIVATE);
}

//------------------------------------------------------------------------
void Window::setTitle (const UTF8String& newTitle)
{
}

//------------------------------------------------------------------------
void Window::updateDPI ()
{
	auto monitor = MonitorFromWindow (hwnd, MONITOR_DEFAULTTONEAREST);
	UINT x, y;
	GetDpiForMonitor (monitor, MDT_EFFECTIVE_DPI, &x, &y);
	setNewDPI (x);
}

//------------------------------------------------------------------------
double Window::getScaleFactor () const
{
	if (!hwnd)
		return 1.;
	auto monitor = MonitorFromWindow (hwnd, MONITOR_DEFAULTTONEAREST);
	UINT x, y;
	GetDpiForMonitor (monitor, MDT_EFFECTIVE_DPI, &x, &y);
	return static_cast<CCoord> (x) * (100. / 96.) / 100.;
}

//------------------------------------------------------------------------
void Window::show ()
{
	if (hasMenu)
		updateCommands ();
	updateDPI ();

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
	auto self = shared_from_this ();
	auto call = [self] () {
		self->onQuit (); // TODO: rename method !
	};
	Async::perform (Async::Context::Main, call);
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
void Window::center ()
{
	updateDPI ();

	auto monitor = MonitorFromWindow (hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO info;
	info.cbSize = sizeof (MONITORINFO);
	GetMonitorInfo (monitor, &info);
	CRect monitorRect {
	    static_cast<CCoord> (info.rcWork.left), static_cast<CCoord> (info.rcWork.top),
	    static_cast<CCoord> (info.rcWork.right), static_cast<CCoord> (info.rcWork.bottom)};
	CRect windowRect;
	windowRect.setWidth (getSize ().x * dpiScale);
	windowRect.setHeight (getSize ().y * dpiScale);
	windowRect.centerInside (monitorRect);
	setPosition (windowRect.getTopLeft ());
}

//------------------------------------------------------------------------
void Window::windowWillClose ()
{
	auto d = delegate;
	delegate = nullptr;
	if (d)
		d->onClosed ();
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
