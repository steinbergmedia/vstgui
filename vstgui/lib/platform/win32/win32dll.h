// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <windows.h>

//------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
struct DllBase
{
	DllBase (const char* dllName) { module = LoadLibraryA (dllName); }
	~DllBase () noexcept
	{
		if (module)
			FreeLibrary (module);
	}

	template <typename T>
	T getProcAddress (const char* name) const
	{
		if (!module)
			return nullptr;
		return reinterpret_cast<T> (GetProcAddress (module, name));
	}

private:
	HINSTANCE module {nullptr};
};

//-----------------------------------------------------------------------------
struct HiDPISupport : DllBase
{
	static HiDPISupport& instance ()
	{
		static HiDPISupport singleton;
		return singleton;
	}

	UINT getDPIForWindow (HWND hwnd)
	{
		if (getDPIForWindowFunc)
			return getDPIForWindowFunc (hwnd);
		return 96;
	}
	enum MONITOR_DPI_TYPE
	{
		MDT_EFFECTIVE_DPI = 0,
		MDT_ANGULAR_DPI = 1,
		MDT_RAW_DPI = 2,
		MDT_DEFAULT = MDT_EFFECTIVE_DPI
	};

	HRESULT getDPIForMonitor (_In_ HMONITOR hmonitor, _In_ MONITOR_DPI_TYPE dpiType,
	                          _Out_ UINT* dpiX, _Out_ UINT* dpiY)
	{
		if (getDpiForMonitorFunc)
			return getDpiForMonitorFunc (hmonitor, dpiType, dpiX, dpiY);
		if (dpiX)
			*dpiX = 96;
		if (*dpiY)
			*dpiY = 96;
		return S_OK;
	}

private:
	using GetDpiForWindowFunc = UINT (WINAPI*) (HWND hWnd);
	using GetDpiForMonitorFunc = HRESULT (WINAPI*) (_In_ HMONITOR hmonitor,
	                                                _In_ MONITOR_DPI_TYPE dpiType, _Out_ UINT* dpiX,
	                                                _Out_ UINT* dpiY);

	GetDpiForWindowFunc getDPIForWindowFunc {nullptr};
	GetDpiForMonitorFunc getDpiForMonitorFunc {nullptr};
	DllBase shCore {"Shcore.dll"};

	HiDPISupport () : DllBase ("User32.dll")
	{
		getDPIForWindowFunc = getProcAddress<GetDpiForWindowFunc> ("GetDpiForWindow");
		getDpiForMonitorFunc = shCore.getProcAddress<GetDpiForMonitorFunc> ("GetDpiForMonitor");
	}
};

//-----------------------------------------------------------------------------
struct Dwm : DllBase
{
	struct MARGINS
	{
		int cxLeftWidth; // width of left border that retains its size
		int cxRightWidth; // width of right border that retains its size
		int cyTopHeight; // height of top border that retains its size
		int cyBottomHeight; // height of bottom border that retains its size
	};

	static Dwm& instance ()
	{
		static Dwm singleton;
		return singleton;
	}

	HRESULT extendFrameIntoClientArea (HWND hwnd, const MARGINS* marInset)
	{
		if (extendFrameIntoClientAreaFunc)
			return extendFrameIntoClientAreaFunc (hwnd, marInset);
		return S_FALSE;
	}

private:
	using DwmExtendFrameIntoClientAreaFunc = HRESULT (WINAPI*) (HWND hWnd,
	                                                            _In_ const MARGINS* pMarInset);

	Dwm () : DllBase ("Dwmapi.dll")
	{
		extendFrameIntoClientAreaFunc =
		    getProcAddress<DwmExtendFrameIntoClientAreaFunc> ("DwmExtendFrameIntoClientArea");
	}

	DwmExtendFrameIntoClientAreaFunc extendFrameIntoClientAreaFunc {nullptr};
};

//------------------------------------------------------------------------
} // VSTGUI
