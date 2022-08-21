// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstgui/lib/vstguibase.h"
#include "vstgui/lib/iexternalview.h"

#include <windows.h>
#include <cassert>
#include <functional>
#include <cstdint>
#include <string>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ExternalView {

//------------------------------------------------------------------------
inline void setWindowSize (HWND window, IntRect r)
{
	SetWindowPos (window, HWND_TOP, static_cast<int> (r.origin.x), static_cast<int> (r.origin.y),
				  static_cast<int> (r.size.width), static_cast<int> (r.size.height),
				  SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOREDRAW | SWP_DEFERERASE);
}

//------------------------------------------------------------------------
struct HWNDWindow final
{
	using WindowProcFunc =
		std::function<LONG_PTR (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)>;

	HWNDWindow (HINSTANCE instance) : instance (instance) {}

	~HWNDWindow () noexcept
	{
		if (window)
		{
			SetWindowLongPtr (window, GWLP_USERDATA, (__int3264)(LONG_PTR) nullptr);
			DestroyWindow (window);
		}
		destroyWindowClass ();
	}

	bool create (const TCHAR* title, const IntRect& frame, HWND parent, DWORD exStyle = 0,
				 DWORD style = WS_CHILD)
	{
		if (!initWindowClass ())
			return false;
		window = CreateWindowEx (
			exStyle, MAKEINTATOM (windowClassAtom), title, style, static_cast<int> (frame.origin.x),
			static_cast<int> (frame.origin.y), static_cast<int> (frame.size.width),
			static_cast<int> (frame.size.height), parent, nullptr, instance, nullptr);
		if (!window)
			return false;
		SetWindowLongPtr (window, GWLP_USERDATA, (__int3264)(LONG_PTR)this);
		return true;
	}

	void setWindowProc (WindowProcFunc&& func) { windowProc = std::move (func); }

	void setSize (const IntRect& r)
	{
		if (!window)
			return;
		setWindowSize (window, r);
	}

	void show (bool state) { ShowWindow (window, state ? SW_SHOW : SW_HIDE); }
	void setEnabled (bool state) { EnableWindow (window, state); }

	HWND getHWND () const { return window; }
	HINSTANCE getInstance () const { return instance; }

private:
	bool initWindowClass ()
	{
		assert (instance != nullptr);

		if (windowClassAtom != 0)
			return true;

		std::wstring windowClassName;
		windowClassName = TEXT ("VSTGUI ExternalView Container ");
		windowClassName += std::to_wstring (reinterpret_cast<uint64_t> (this));

		WNDCLASS windowClass;
		windowClass.style = CS_GLOBALCLASS;

		windowClass.lpfnWndProc = WindowProc;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = instance;
		windowClass.hIcon = 0;

		windowClass.hCursor = LoadCursor (NULL, IDC_ARROW);
		windowClass.hbrBackground = 0;

		windowClass.lpszMenuName = 0;
		windowClass.lpszClassName = windowClassName.data ();
		windowClassAtom = RegisterClass (&windowClass);
		return windowClassAtom != 0;
	}

	void destroyWindowClass ()
	{
		if (windowClassAtom == 0)
			return;
		UnregisterClass (MAKEINTATOM (windowClassAtom), instance);
		windowClassAtom = 0;
	}

	static LONG_PTR WINAPI WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == WM_ERASEBKGND)
			return 1;
		auto instance = reinterpret_cast<HWNDWindow*> (GetWindowLongPtr (hwnd, GWLP_USERDATA));
		if (instance && instance->windowProc)
			return instance->windowProc (hwnd, message, wParam, lParam);
		return DefWindowProc (hwnd, message, wParam, lParam);
	}

	WindowProcFunc windowProc;
	HWND window {nullptr};
	HINSTANCE instance {nullptr};
	ATOM windowClassAtom {0};
};

//------------------------------------------------------------------------
struct NonClientMetrics
{
	static const NONCLIENTMETRICS& get ()
	{
		static NonClientMetrics gInstance;
		return gInstance.nonClientMetrics;
	}

private:
	NonClientMetrics ()
	{
		nonClientMetrics.cbSize = sizeof (nonClientMetrics);
		SystemParametersInfoForDpi (SPI_GETNONCLIENTMETRICS, nonClientMetrics.cbSize,
									&nonClientMetrics, 0, 96);
	}

	NONCLIENTMETRICS nonClientMetrics {};
};

//------------------------------------------------------------------------
struct ExternalHWNDBase : ViewAdapter
{
	using Base = ExternalHWNDBase;
	using PlatformViewType = ExternalView::PlatformViewType;
	using IntRect = ExternalView::IntRect;

	HWNDWindow container;
	HWND child {nullptr};

	ExternalHWNDBase (HINSTANCE hInst) : container (hInst)
	{
		container.create (nullptr, {{0, 0}, {1, 1}}, HWND_MESSAGE,
						  WS_EX_NOPARENTNOTIFY | WS_EX_COMPOSITED, WS_CHILD | WS_VISIBLE);
	}

	virtual ~ExternalHWNDBase () noexcept
	{
		if (child)
			DestroyWindow (child);
	}

	bool platformViewTypeSupported (PlatformViewType type) override
	{
		return type == PlatformViewType::HWND;
	}

	bool attach (void* parent, PlatformViewType parentViewType) override
	{
		assert (container.getHWND ());
		if (parent == nullptr || parentViewType != PlatformViewType::HWND)
			return false;
		auto parentHWND = reinterpret_cast<HWND> (parent);
		SetParent (container.getHWND (), parentHWND);
		return true;
	}

	bool remove () override
	{
		assert (container.getHWND ());
		SetParent (container.getHWND (), HWND_MESSAGE);
		return true;
	}

	void setViewSize (IntRect frame, IntRect visible) override
	{
		assert (container.getHWND ());
		container.setSize (visible);
		if (child)
		{
			frame.origin.x -= visible.origin.x;
			frame.origin.y -= visible.origin.y;
			setWindowSize (child, frame);
		}
	}

	void setContentScaleFactor (double scaleFactor) override {}

	void setMouseEnabled (bool state) override { EnableWindow (container.getHWND (), state); }

	void takeFocus () override { SetFocus (child); }

	void looseFocus () override
	{
		if (GetFocus () == child)
		{
			SetFocus (GetParent (container.getHWND ()));
		}
	}
};

//------------------------------------------------------------------------
} // ExternalView
} // VSTGUI
