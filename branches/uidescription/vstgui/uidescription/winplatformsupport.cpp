//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2009, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#if VSTGUI_LIVE_EDITING

#include "platformsupport.h"

#if WINDOWS

#include "ccolorchooserpanel.h"
#include "../lib/platform/win32/win32support.h"
#include <wingdi.h>
#include <oleidl.h>
#include <ole2.h>
#include <shlobj.h>
#include <shellapi.h>
#include <richedit.h>
#include <shobjidl.h>
#include <string>

static TCHAR   gClassName[100];
extern void* hInstance;
inline HINSTANCE GetInstance () { return (HINSTANCE)hInstance; }

namespace VSTGUI {

//-----------------------------------------------------------------------------
class Win32Window : public PlatformWindow
{
public:
	Win32Window (const CRect& size, const char* title, WindowType type, long styleFlags, IPlatformWindowDelegate* delegate, void* parentWindow);
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
PlatformWindow* PlatformWindow::create (const CRect& size, const char* title, WindowType type, long styleFlags, IPlatformWindowDelegate* delegate, void* parentWindow)
{
	return new Win32Window (size, title, type, styleFlags, delegate, parentWindow);
}

//-----------------------------------------------------------------------------
Win32Window::Win32Window (const CRect& size, const char* title, WindowType type, long styleFlags, IPlatformWindowDelegate* delegate, void* parentWindow)
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
	HWND baseWindow = parentWindow ? (HWND)parentWindow : GetForegroundWindow ();
#if 0
	while (baseWindow)
	{
		HWND temp = GetWindow (baseWindow, GW_OWNER);
		if (temp == 0)
			break;
		baseWindow = temp;
		break;
	}
#endif
	platformWindow = CreateWindowEx (exStyle, gClassName, titleStr, wStyle, r.left, r.top, r.right - r.left, r.bottom - r.top, baseWindow, 0, GetInstance (), 0);
	SetWindowLongPtr (platformWindow, GWLP_USERDATA, (LONG_PTR)this);
}

//-----------------------------------------------------------------------------
Win32Window::~Win32Window ()
{
	delegate = 0;
	if (platformWindow)
		DestroyWindow (platformWindow);
}

//-----------------------------------------------------------------------------
void Win32Window::getWindowFlags (DWORD& wStyle, DWORD& exStyle)
{
	exStyle = WS_EX_COMPOSITED|WS_EX_LAYERED;
	wStyle = WS_CAPTION|WS_CLIPCHILDREN;
	if (type == kPanelType)
	{
		wStyle |= WS_POPUP;
		exStyle |= WS_EX_TOOLWINDOW;
	}
	else
	{
		exStyle |= WS_EX_DLGMODALFRAME;
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
	PostMessage (platformWindow, WM_NCACTIVATE, true, 0);
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
			case WM_DESTROY:
			{
				if (window->delegate)
					window->delegate->windowClosed (window);
				break;
			}

			case WM_SIZE:
			{
				if (window->delegate && !window->recursiveGuard)
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
class DropSource : public CBaseObject, public ::IDropSource
{
public:
	DropSource () {}

	// IUnknown
	STDMETHOD (QueryInterface) (REFIID riid, void** object);
	STDMETHOD_ (ULONG, AddRef) (void) { remember (); return getNbReference ();}
	STDMETHOD_ (ULONG, Release) (void) { ULONG refCount = getNbReference () - 1; forget (); return refCount; }
	
	// IDropSource
	STDMETHOD (QueryContinueDrag) (BOOL escapePressed, DWORD keyState);
	STDMETHOD (GiveFeedback) (DWORD effect);
};

//-----------------------------------------------------------------------------
class DataObject : public CBaseObject, public ::IDataObject
{
public:
	DataObject (const char* string);
	~DataObject ();

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
	std::string dataString;
};

//-----------------------------------------------------------------------------
bool PlatformUtilities::startDrag (CFrame* frame, const CPoint& location, const char* string, CBitmap* dragBitmap, bool localOnly)
{
	// TODO: Windows start drag support
	DataObject* dataObject = new DataObject (string);
	DropSource* dropSource = new DropSource;
	DWORD outEffect;
	HRESULT result = DoDragDrop (dataObject, dropSource, DROPEFFECT_COPY, &outEffect);
	dataObject->Release ();
	dropSource->Release ();
	return result == DRAGDROP_S_DROP;
}

//-----------------------------------------------------------------------------
#if !NATIVE_COLOR_CHOOSER
class ColorChooserWindowOwner : public CBaseObject
{
public:
	static ColorChooserWindowOwner* instance () { return &ccwo; }

	CColorChooserPanel* getPanel (bool create = true)
	{
		if (panel == 0 && create)
		{
			panel = new CColorChooserPanel (this);
		}
		return panel;
	}
	
	void closePanel ()
	{
		if (panel)
			panel->forget ();
		panel = 0;
	}
	
protected:
	static ColorChooserWindowOwner ccwo; 

	ColorChooserWindowOwner () : panel (0) {}
	~ColorChooserWindowOwner ()
	{
		closePanel ();
	}
	
	CMessageResult notify (CBaseObject* sender, const char* message)
	{
		if (message == CColorChooserPanel::kMsgWindowClosed)
			panel = 0;
		return kMessageNotified;
	}
	
	CColorChooserPanel* panel;
};
ColorChooserWindowOwner ColorChooserWindowOwner::ccwo; 
#endif

//-----------------------------------------------------------------------------
void PlatformUtilities::colorChooser (const CColor* oldColor, IPlatformColorChangeCallback* callback)
{
	#if NATIVE_COLOR_CHOOSER
	if (oldColor)
	{
		// TODO: Windows Colorchooser support (this implementation lacks alpha color support
		static COLORREF acrCustClr[16];
		CHOOSECOLOR cc = {0};
		cc.lStructSize = sizeof (CHOOSECOLOR);
		cc.Flags = CC_FULLOPEN|CC_ANYCOLOR|CC_RGBINIT;
		cc.lpCustColors = (LPDWORD) acrCustClr;
		cc.rgbResult = RGB(oldColor->red, oldColor->blue, oldColor->green);
		if (ChooseColor (&cc))
		{
			CColor color = MakeCColor (GetRValue (cc.rgbResult), GetGValue (cc.rgbResult), GetBValue (cc.rgbResult));
			callback->colorChanged (color);
		}
	}
	#else
	if (oldColor)
	{
		ColorChooserWindowOwner* owner = ColorChooserWindowOwner::instance ();
		CColorChooserPanel* panel = owner->getPanel ();
		panel->setColor (*oldColor);
		panel->setColorChangeCallback (callback);
	}
	else
	{
		ColorChooserWindowOwner* owner = ColorChooserWindowOwner::instance ();
		CColorChooserPanel* panel = owner->getPanel (false);
		if (panel)
		{
			panel->setColorChangeCallback (0);
			owner->closePanel ();
		}
	}
	#endif

}

//-----------------------------------------------------------------------------
// DropSource
//-----------------------------------------------------------------------------
STDMETHODIMP DropSource::QueryInterface (REFIID riid, void** object)
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
STDMETHODIMP DropSource::QueryContinueDrag (BOOL escapePressed, DWORD keyState)
{
	if (escapePressed)
		return DRAGDROP_S_CANCEL;
	
	if ((keyState & (MK_LBUTTON|MK_RBUTTON)) == 0)
		return DRAGDROP_S_DROP;

	return S_OK;	
}

//-----------------------------------------------------------------------------
STDMETHODIMP DropSource::GiveFeedback (DWORD effect)
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

//-----------------------------------------------------------------------------
// DataObject
//-----------------------------------------------------------------------------
DataObject::DataObject (const char* string)
{
	dataString = string;
}

//-----------------------------------------------------------------------------
DataObject::~DataObject ()
{
}

//-----------------------------------------------------------------------------
STDMETHODIMP DataObject::QueryInterface (REFIID riid, void** object)
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
STDMETHODIMP DataObject::GetData (FORMATETC* format, STGMEDIUM* medium)
{
	medium->tymed = 0;
	medium->hGlobal = 0;
	medium->pUnkForRelease = 0;

	if (format->cfFormat == CF_TEXT || format->cfFormat == CF_UNICODETEXT)
	{
		UTF8StringHelper utf8String (dataString.c_str ());
		SIZE_T size = 0;
		const void* data = 0;
		if (format->cfFormat == CF_UNICODETEXT)
		{
			size = (dataString.length () + 1) * sizeof (WCHAR);
			data = utf8String.getWideString ();
		}
		else
		{
			size = (dataString.length () + 1) * sizeof (char);
			data = dataString.c_str ();
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
	return E_UNEXPECTED;
}

//-----------------------------------------------------------------------------
STDMETHODIMP DataObject::GetDataHere (FORMATETC *format, STGMEDIUM *pmedium)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP DataObject::QueryGetData (FORMATETC *format)
{
	if (format->cfFormat == CF_TEXT || format->cfFormat == CF_UNICODETEXT)
	{
		return S_OK;
	}
	return DV_E_FORMATETC;
}

//-----------------------------------------------------------------------------
STDMETHODIMP DataObject::GetCanonicalFormatEtc (FORMATETC *formatIn, FORMATETC *formatOut)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP DataObject::SetData (FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP DataObject::EnumFormatEtc (DWORD dwDirection, IEnumFORMATETC** enumFormat)
{
	return E_NOTIMPL;
//	*enumFormat = NEW SmtgEnumFormat (source);
//	if (*enumFormat)
//		return NO_ERROR;
//	return STG_E_UNKNOWN;
}

//-----------------------------------------------------------------------------
STDMETHODIMP DataObject::DAdvise (FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP DataObject::DUnadvise (DWORD dwConnection)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP DataObject::EnumDAdvise (IEnumSTATDATA** ppenumAdvise)
{
	return E_NOTIMPL;
}

} // namespace

#endif // WINDOWS

#endif // VSTGUI_LIVE_EDITING
