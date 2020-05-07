// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <windows.h>
#include <windef.h>

#ifndef _DPI_AWARENESS_CONTEXTS_

DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);

typedef enum DPI_AWARENESS {
    DPI_AWARENESS_INVALID           = -1,
    DPI_AWARENESS_UNAWARE           = 0,
    DPI_AWARENESS_SYSTEM_AWARE      = 1,
    DPI_AWARENESS_PER_MONITOR_AWARE = 2
} DPI_AWARENESS;

#define DPI_AWARENESS_CONTEXT_UNAWARE              ((DPI_AWARENESS_CONTEXT)-1)
#define DPI_AWARENESS_CONTEXT_SYSTEM_AWARE         ((DPI_AWARENESS_CONTEXT)-2)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE    ((DPI_AWARENESS_CONTEXT)-3)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)

#endif

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
		if (dpiY)
			*dpiY = 96;
		return S_OK;
	}

	enum PROCESS_DPI_AWARENESS
	{
		PROCESS_DPI_UNAWARE = 0,
		PROCESS_SYSTEM_DPI_AWARE = 1,
		PROCESS_PER_MONITOR_DPI_AWARE = 2
	};

	HRESULT setProcessDpiAwareness (PROCESS_DPI_AWARENESS value)
	{
		if (setProcessDpiAwarenessFunc)
			return setProcessDpiAwarenessFunc (value);
		return S_FALSE;
	}

	bool enableNonClientDpiScaling (HWND window)
	{
		if (enableNonClientDpiScalingFunc)
			return enableNonClientDpiScalingFunc (window) != 0;
		return false;
	}

	enum AWARENESS_CONTEXT
	{
		AWARENESS_CONTEXT_UNAWARE = -1,
		AWARENESS_CONTEXT_SYSTEM_AWARE = -2,
		AWARENESS_CONTEXT_PER_MONITOR_AWARE = -3,
		AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 = -4
	};

	bool setProcessDpiAwarnessContext (AWARENESS_CONTEXT context)
	{
		if (setProcessDpiAwarenessContextFunc)
			return setProcessDpiAwarenessContextFunc (reinterpret_cast<DPI_AWARENESS_CONTEXT> (context)) != 0;
		return false;
	}

private:
	using GetDpiForWindowFunc = UINT (WINAPI*) (HWND hWnd);
	using GetDpiForMonitorFunc = HRESULT (WINAPI*) (_In_ HMONITOR hmonitor,
	                                                _In_ MONITOR_DPI_TYPE dpiType, _Out_ UINT* dpiX,
	                                                _Out_ UINT* dpiY);

	using SetProcessDpiAwarnessFunc = HRESULT (WINAPI*) (_In_ PROCESS_DPI_AWARENESS value);
	using EnableNonClientDpiScalingFunc = BOOL (WINAPI*) (_In_ HWND hwnd);
	using SetProcessDpiAwarenessContextFunc = BOOL (WINAPI*) (_In_ DPI_AWARENESS_CONTEXT value);

	GetDpiForWindowFunc getDPIForWindowFunc {nullptr};
	GetDpiForMonitorFunc getDpiForMonitorFunc {nullptr};
	SetProcessDpiAwarnessFunc setProcessDpiAwarenessFunc {nullptr};
	EnableNonClientDpiScalingFunc enableNonClientDpiScalingFunc {nullptr};
	SetProcessDpiAwarenessContextFunc setProcessDpiAwarenessContextFunc {nullptr};
	DllBase shCore {"Shcore.dll"};

	HiDPISupport () : DllBase ("User32.dll")
	{
		getDPIForWindowFunc = getProcAddress<GetDpiForWindowFunc> ("GetDpiForWindow");
		enableNonClientDpiScalingFunc =
		    getProcAddress<EnableNonClientDpiScalingFunc> ("EnableNonClientDpiScaling");
		getDpiForMonitorFunc = shCore.getProcAddress<GetDpiForMonitorFunc> ("GetDpiForMonitor");
		setProcessDpiAwarenessFunc =
		    shCore.getProcAddress<SetProcessDpiAwarnessFunc> ("SetProcessDpiAwareness");
		setProcessDpiAwarenessContextFunc = getProcAddress<SetProcessDpiAwarenessContextFunc> ("SetProcessDpiAwarenessContext");
	}
};

//------------------------------------------------------------------------
} // VSTGUI
