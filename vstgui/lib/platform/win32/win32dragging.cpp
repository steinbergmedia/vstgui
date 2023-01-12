// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32dragging.h"
#include "win32support.h"
#include "winstring.h"
#include "../../cstring.h"
#include "../../cdrawcontext.h"
#include "../../cvstguitimer.h"
#include "win32dll.h"
#include <dwmapi.h>
#include <shlobj.h>

#ifdef _MSC_VER
#pragma comment(lib, "Dwmapi.lib")
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
class Win32DragBitmapWindow
{
public:
	Win32DragBitmapWindow (const SharedPointer<CBitmap>& bitmap, CPoint offset);
	~Win32DragBitmapWindow () noexcept;

	void updateBitmap (const SharedPointer<CBitmap>& bitmap, CPoint offset);
	void mouseChanged ();
private:
	static LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT proc (UINT message, WPARAM wParam, LPARAM lParam);

	void registerWindowClass ();
	void unregisterWindowClass ();

	void createWindow ();
	void releaseWindow ();
	void showWindow ();

	POINT calculateWindowPosition () const;
	void updateScaleFactor (POINT p);
	void updateWindowPosition (POINT where);
	void setAlphaTransparency (float alpha);

	void paint ();

	SharedPointer<CBitmap> bitmap;
	CPoint offset;
	UTF8String windowClassName;
	HWND hwnd {nullptr};
	CCoord scaleFactor {2.};
};

//-----------------------------------------------------------------------------
class Win32MouseObserverWhileDragging
{
public:
	using Callback = std::function<void ()>;

	Win32MouseObserverWhileDragging ()
	{
		gInstance = this;
		mouseHook = SetWindowsHookEx (WH_MOUSE, MouseHookProc, GetInstance (), GetCurrentThreadId ());
	}

	~Win32MouseObserverWhileDragging () noexcept
	{
		if (mouseHook)
			UnhookWindowsHookEx (mouseHook);
		gInstance = nullptr;
	}

	void registerCallback (Callback&& c) { callbacks.emplace_back (std::move (c)); }
private:
	static Win32MouseObserverWhileDragging* gInstance;
	static LRESULT CALLBACK MouseHookProc (int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (gInstance)
		{
			for (auto& c : gInstance->callbacks)
				c ();
		}
		return CallNextHookEx (nullptr, nCode, wParam, lParam);
	}
	HHOOK mouseHook {nullptr};
	std::vector<Callback> callbacks;
};
Win32MouseObserverWhileDragging* Win32MouseObserverWhileDragging::gInstance = nullptr;

//-----------------------------------------------------------------------------
Win32DraggingSession::Win32DraggingSession (Win32Frame* frame)
: frame (frame)
{
}

//-----------------------------------------------------------------------------
Win32DraggingSession::~Win32DraggingSession () noexcept = default;

//-----------------------------------------------------------------------------
bool Win32DraggingSession::setBitmap (const SharedPointer<CBitmap>& bitmap, CPoint offset)
{
	if (!dragBitmapWindow && bitmap)
	{
		dragBitmapWindow = std::make_unique<Win32DragBitmapWindow> (bitmap,
		                                                            offset);
		if (!mouseObserver)
			mouseObserver = std::make_unique<Win32MouseObserverWhileDragging> ();

		mouseObserver->registerCallback ([&] () { dragBitmapWindow->mouseChanged (); });
	}
	if (dragBitmapWindow)
	{
		dragBitmapWindow->updateBitmap (bitmap, offset);
	}
	return true;
}

//-----------------------------------------------------------------------------
bool Win32DraggingSession::doDrag (const DragDescription& dragDescription, const SharedPointer<IDragCallback>& callback)
{
	auto lastCursor = frame->getLastSetCursor ();
	frame->setMouseCursor (kCursorNotAllowed);

	if (callback)
		mouseObserver = std::make_unique<Win32MouseObserverWhileDragging> ();

	if (callback)
	{
		CPoint location;
		frame->getCurrentMousePosition (location);
		callback->dragWillBegin (this, location);

		if (mouseObserver)
		{
			mouseObserver->registerCallback ([callback, location, this] () mutable {
				CPoint newLocation;
				frame->getCurrentMousePosition (newLocation);
				if (newLocation != location)
				{
					callback->dragMoved (this, newLocation);
					location = newLocation;
				}
			});
		}
	}

	setBitmap (dragDescription.bitmap, dragDescription.bitmapOffset);

	auto dataObject = new Win32DataObject (dragDescription.data);
	auto dropSource = new Win32DropSource ();
	DWORD outEffect;

	auto hResult = DoDragDrop (dataObject, dropSource, DROPEFFECT_COPY | DROPEFFECT_MOVE, &outEffect);
	if (mouseObserver)
		mouseObserver = nullptr;
	if (dragBitmapWindow)
		dragBitmapWindow = nullptr;

	frame->setMouseCursor (lastCursor);

	if (callback)
	{
		CPoint location;
		frame->getCurrentMousePosition (location);
		if (hResult == DRAGDROP_S_DROP)
		{
			if (outEffect == DROPEFFECT_MOVE)
				callback->dragEnded (this, location, DragOperation::Move);
			else
				callback->dragEnded (this, location, DragOperation::Copy);
		}
		else
		{
			callback->dragEnded (this, location, DragOperation::None);
		}
	}

	dataObject->Release ();
	dropSource->Release ();

	return true;
}

//-----------------------------------------------------------------------------
CDropTarget::CDropTarget (Win32Frame* pFrame)
: refCount (0)
, pFrame (pFrame)
, dragData (nullptr)
{
}

//-----------------------------------------------------------------------------
CDropTarget::~CDropTarget () noexcept
{
}

//-----------------------------------------------------------------------------
COM_DECLSPEC_NOTHROW STDMETHODIMP CDropTarget::QueryInterface (REFIID riid, void** object)
{
	if (riid == IID_IDropTarget || riid == IID_IUnknown)
	{
		*object = this;
		AddRef ();
      return NOERROR;
	}
	*object = nullptr;
	return E_NOINTERFACE;
}

//-----------------------------------------------------------------------------
COM_DECLSPEC_NOTHROW STDMETHODIMP_(ULONG) CDropTarget::AddRef ()
{
	return static_cast<ULONG> (++refCount);
}

//-----------------------------------------------------------------------------
COM_DECLSPEC_NOTHROW STDMETHODIMP_(ULONG) CDropTarget::Release ()
{
	refCount--;
	if (refCount <= 0)
	{
		delete this;
		return 0;
	}
	return static_cast<ULONG> (refCount);
}

//-----------------------------------------------------------------------------
COM_DECLSPEC_NOTHROW STDMETHODIMP CDropTarget::DragEnter (IDataObject* dataObject, DWORD keyState, POINTL pt, DWORD* effect)
{
	if (dataObject && pFrame)
	{
		dragData = new Win32DataPackage (dataObject);

		DragEventData data;
		data.drag = dragData;
		pFrame->getCurrentMousePosition (data.pos);
		pFrame->getCurrentModifiers (data.modifiers);
		auto result = pFrame->getFrame ()->platformOnDragEnter (data);
		if (result == DragOperation::Copy)
			*effect = DROPEFFECT_COPY;
		else if (result == DragOperation::Move)
			*effect = DROPEFFECT_MOVE;
		else
			*effect = DROPEFFECT_NONE;
	}
	else
		*effect = DROPEFFECT_NONE;
	return S_OK;
}

//-----------------------------------------------------------------------------
COM_DECLSPEC_NOTHROW STDMETHODIMP CDropTarget::DragOver (DWORD keyState, POINTL pt, DWORD* effect)
{
	if (dragData && pFrame)
	{
		DragEventData data;
		data.drag = dragData;
		pFrame->getCurrentMousePosition (data.pos);
		pFrame->getCurrentModifiers (data.modifiers);
		auto result = pFrame->getFrame ()->platformOnDragMove (data);
		if (result == DragOperation::Copy)
			*effect = DROPEFFECT_COPY;
		else if (result == DragOperation::Move)
			*effect = DROPEFFECT_MOVE;
		else
			*effect = DROPEFFECT_NONE;
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
COM_DECLSPEC_NOTHROW STDMETHODIMP CDropTarget::DragLeave ()
{
	if (dragData && pFrame)
	{
		DragEventData data;
		data.drag = dragData;
		pFrame->getCurrentMousePosition (data.pos);
		pFrame->getCurrentModifiers (data.modifiers);
		pFrame->getFrame ()->platformOnDragLeave (data);
		dragData->forget ();
		dragData = nullptr;
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
COM_DECLSPEC_NOTHROW STDMETHODIMP CDropTarget::Drop (IDataObject* dataObject, DWORD keyState, POINTL pt, DWORD* effect)
{
	if (dragData && pFrame)
	{
		DragEventData data;
		data.drag = dragData;
		pFrame->getCurrentMousePosition (data.pos);
		pFrame->getCurrentModifiers (data.modifiers);
		pFrame->getFrame ()->platformOnDrop (data);
		dragData->forget ();
		dragData = nullptr;
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
// Win32DropSource
//-----------------------------------------------------------------------------
COM_DECLSPEC_NOTHROW STDMETHODIMP Win32DropSource::QueryInterface (REFIID riid, void** object)
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
COM_DECLSPEC_NOTHROW STDMETHODIMP Win32DropSource::QueryContinueDrag (BOOL escapePressed, DWORD keyState)
{
	if (escapePressed)
		return DRAGDROP_S_CANCEL;
	
	if ((keyState & (MK_LBUTTON|MK_RBUTTON)) == 0)
		return DRAGDROP_S_DROP;

	return S_OK;	
}

//-----------------------------------------------------------------------------
COM_DECLSPEC_NOTHROW STDMETHODIMP Win32DropSource::GiveFeedback (DWORD effect)
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
Win32DataObject::~Win32DataObject () noexcept
{
	dataPackage->forget ();
}

//-----------------------------------------------------------------------------
COM_DECLSPEC_NOTHROW STDMETHODIMP Win32DataObject::QueryInterface (REFIID riid, void** object)
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
COM_DECLSPEC_NOTHROW STDMETHODIMP Win32DataObject::GetData (FORMATETC* format, STGMEDIUM* medium)
{
	medium->tymed = 0;
	medium->hGlobal = nullptr;
	medium->pUnkForRelease = nullptr;

	if (format->cfFormat == CF_TEXT || format->cfFormat == CF_UNICODETEXT)
	{
		for (uint32_t i = 0; i < dataPackage->getCount (); i++)
		{
			if (dataPackage->getDataType (i) == IDataPackage::kText)
			{
				const void* buffer;
				IDataPackage::Type type;
				uint32_t bufferSize = dataPackage->getData (i, buffer, type);
				UTF8StringHelper utf8String (static_cast<const char*> (buffer), bufferSize);
				SIZE_T size = 0;
				const void* data = nullptr;
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
					if (!memoryHandle)
						return E_OUTOFMEMORY;
					if (void* memory = GlobalLock (memoryHandle))
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
		auto** wideStringFileNames = (UTF8StringHelper**)std::malloc (sizeof (UTF8StringHelper*) * dataPackage->getCount ());
		if (!wideStringFileNames)
			return result;
		
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
		if (HGLOBAL memoryHandle = GlobalAlloc (GMEM_MOVEABLE, bufferSizeNeeded))
		{
			if (void* memory = GlobalLock (memoryHandle))
			{
				auto* dropFiles = (DROPFILES*)memory;
				dropFiles->pFiles = sizeof (DROPFILES);
				dropFiles->pt.x = 0;
				dropFiles->pt.y = 0;
				dropFiles->fNC = FALSE;
				dropFiles->fWide = TRUE;
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
		}
		else
			result = E_OUTOFMEMORY;
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
				if (!memoryHandle)
					return E_OUTOFMEMORY;

				if (void* memory = GlobalLock (memoryHandle))
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
COM_DECLSPEC_NOTHROW STDMETHODIMP Win32DataObject::GetDataHere (FORMATETC *format, STGMEDIUM *pmedium)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
COM_DECLSPEC_NOTHROW STDMETHODIMP Win32DataObject::QueryGetData (FORMATETC *format)
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
COM_DECLSPEC_NOTHROW STDMETHODIMP Win32DataObject::GetCanonicalFormatEtc (FORMATETC *formatIn, FORMATETC *formatOut)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
COM_DECLSPEC_NOTHROW STDMETHODIMP Win32DataObject::SetData (FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
struct Win32DataObjectEnumerator : IEnumFORMATETC, AtomicReferenceCounted
{
	Win32DataObjectEnumerator (const SharedPointer<IDataPackage>& data) : data (data) {}

	COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE QueryInterface (REFIID riid, void** object) override
	{
		if (riid == ::IID_IEnumFORMATETC)
		{
			AddRef ();
			*object = (::IEnumFORMATETC*)this;
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

	COM_DECLSPEC_NOTHROW ULONG STDMETHODCALLTYPE AddRef () override
	{
		remember ();
		return static_cast<ULONG> (getNbReference ());
	}
	COM_DECLSPEC_NOTHROW ULONG STDMETHODCALLTYPE Release () override
	{
		ULONG refCount = static_cast<ULONG> (getNbReference ()) - 1;
		forget ();
		return refCount;
	}

	COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Next (ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched) override
	{
		if (!rgelt)
			return E_INVALIDARG;
		const void* buffer;
		IDataPackage::Type dataType;
		auto dataSize = data->getData (index++, buffer, dataType);
		if (dataSize)
		{
			if (dataType == IDataPackage::kText)
			{
				rgelt->cfFormat = CF_UNICODETEXT;
				rgelt->ptd = nullptr;
				rgelt->dwAspect = DVASPECT_CONTENT;
				rgelt->lindex = -1;
				rgelt->tymed = TYMED_HGLOBAL;
				return S_OK;
			}
			if (dataType == IDataPackage::kFilePath)
			{
#if DEBUG
				DebugPrint ("IDataPackage::kFilePath not yet tested!\n");
#endif
				rgelt->cfFormat = CF_HDROP;
				rgelt->ptd = nullptr;
				rgelt->dwAspect = DVASPECT_CONTENT;
				rgelt->lindex = -1;
				rgelt->tymed = TYMED_HGLOBAL;
				return S_OK;
			}
		}
		return S_FALSE;
	}

	COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Skip (ULONG celt) override
	{
		if (index + celt >= data->getCount ())
			return S_FALSE;
		index += celt;
		return S_OK;
	}

	COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Reset () override
	{
		index = 0;
		return S_OK;
	}

	COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Clone (IEnumFORMATETC** ppenum) override
	{
		return E_NOTIMPL;
	}

	SharedPointer<IDataPackage> data;
	uint32_t index {0};
};

//-----------------------------------------------------------------------------
COM_DECLSPEC_NOTHROW STDMETHODIMP Win32DataObject::EnumFormatEtc (DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
{
	if (dwDirection != DATADIR_GET)
		return E_INVALIDARG;

	auto enumerator = new Win32DataObjectEnumerator (dataPackage);
	*ppenumFormatEtc = enumerator;
	return S_OK;
}

//-----------------------------------------------------------------------------
COM_DECLSPEC_NOTHROW STDMETHODIMP Win32DataObject::DAdvise (FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
COM_DECLSPEC_NOTHROW STDMETHODIMP Win32DataObject::DUnadvise (DWORD dwConnection)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
COM_DECLSPEC_NOTHROW STDMETHODIMP Win32DataObject::EnumDAdvise (IEnumSTATDATA** ppenumAdvise)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
Win32DragBitmapWindow::Win32DragBitmapWindow (const SharedPointer<CBitmap>& bitmap, CPoint offset)
: bitmap (bitmap)
, offset (offset)
{
	registerWindowClass ();
	auto initialWindowPosition = calculateWindowPosition ();
	updateScaleFactor (initialWindowPosition);
	createWindow ();
	setAlphaTransparency (1.f);
	updateWindowPosition (initialWindowPosition);
	showWindow ();
}

//-----------------------------------------------------------------------------
Win32DragBitmapWindow::~Win32DragBitmapWindow () noexcept
{
	releaseWindow ();
	unregisterWindowClass ();
}

//-----------------------------------------------------------------------------
void Win32DragBitmapWindow::updateBitmap (const SharedPointer<CBitmap>& bitmap, CPoint offset)
{
	this->bitmap = bitmap;
	this->offset = offset;
	updateWindowPosition (calculateWindowPosition ());
	RECT r;
	GetClientRect (hwnd, &r);
	InvalidateRect (hwnd, &r, FALSE);
}

//-----------------------------------------------------------------------------
void Win32DragBitmapWindow::createWindow ()
{
	auto winString = dynamic_cast<WinString*> (windowClassName.getPlatformString ());

	DWORD exStyle = WS_EX_COMPOSITED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW;
	DWORD dwStyle = WS_POPUP;

	auto width = static_cast<int> (bitmap->getWidth () * scaleFactor);
	auto height = static_cast<int> (bitmap->getHeight () * scaleFactor);

	hwnd = CreateWindowEx (exStyle, winString->getWideString (), nullptr, dwStyle, -1000, -1000, width,
	                       height, nullptr, nullptr, GetInstance (), nullptr);
	SetWindowLongPtr (hwnd, GWLP_USERDATA, (__int3264) (LONG_PTR) this);

	MARGINS margin = { -1 };
	auto res = DwmExtendFrameIntoClientArea (hwnd, &margin);
	vstgui_assert (res == S_OK);
}

//-----------------------------------------------------------------------------
void Win32DragBitmapWindow::releaseWindow ()
{
	if (hwnd)
		DestroyWindow (hwnd);
}

//-----------------------------------------------------------------------------
void Win32DragBitmapWindow::showWindow ()
{
	ShowWindow (hwnd, SW_SHOWNOACTIVATE);
}

//-----------------------------------------------------------------------------
void Win32DragBitmapWindow::setAlphaTransparency (float alpha)
{
	if (!hwnd)
		return;
	SetLayeredWindowAttributes (hwnd, 0, static_cast<BYTE> (alpha * 255.), LWA_ALPHA);
}

//-----------------------------------------------------------------------------
POINT Win32DragBitmapWindow::calculateWindowPosition () const
{
	POINT where;
	GetCursorPos (&where);
	where.x += static_cast<int> (offset.x * scaleFactor);
	where.y += static_cast<int> (offset.y * scaleFactor);

	return where;
}

//-----------------------------------------------------------------------------
void Win32DragBitmapWindow::updateScaleFactor (POINT p)
{
	auto monitor = MonitorFromPoint (p, MONITOR_DEFAULTTONEAREST);
	UINT dpiX, dpiY;
	HiDPISupport::instance ().getDPIForMonitor (monitor, HiDPISupport::MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
	vstgui_assert (dpiX == dpiY);

	scaleFactor = static_cast<CCoord> (dpiX) / 96.;
}

//-----------------------------------------------------------------------------
void Win32DragBitmapWindow::updateWindowPosition (POINT where)
{
	auto width = bitmap ? static_cast<int> (bitmap->getWidth () * scaleFactor) : 0;
	auto height = bitmap ? static_cast<int> (bitmap->getHeight () * scaleFactor) : 0;

	SetWindowPos (hwnd, nullptr, where.x, where.y, width, height, SWP_NOCOPYBITS | SWP_NOACTIVATE | SWP_NOZORDER);
}

//-----------------------------------------------------------------------------
void Win32DragBitmapWindow::paint ()
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint (hwnd, &ps);

	RECT clientRect;
	GetClientRect (hwnd, &clientRect);

	CRect rect;
	rect.setWidth (clientRect.right - clientRect.left);
	rect.setHeight (clientRect.bottom - clientRect.top);

	if (auto drawContext = owned (createDrawContext (hwnd, hdc, rect)))
	{
		drawContext->beginDraw ();

		drawContext->clearRect (rect);
		drawContext->setGlobalAlpha (0.9f);
		CDrawContext::Transform t (*drawContext, CGraphicsTransform ().scale (scaleFactor, scaleFactor));
		bitmap->draw (drawContext, rect);

		drawContext->endDraw ();
	}
	EndPaint (hwnd, &ps);
}

//-----------------------------------------------------------------------------
LRESULT Win32DragBitmapWindow::proc (UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_DPICHANGED:
		{
			scaleFactor = static_cast<CCoord> (HIWORD (wParam)) / 96.;
			updateWindowPosition (calculateWindowPosition ());
			break;
		}
		case WM_GETMINMAXINFO:
		{
			auto minMax = reinterpret_cast<MINMAXINFO*> (lParam);
			minMax->ptMinTrackSize.x = 1;
			minMax->ptMinTrackSize.y = 1;
			break;
		}
		case WM_ERASEBKGND:
		{
			return 0;
		}
		case WM_PAINT:
		{
			paint ();
			return 0;
		}
		case WM_NCCALCSIZE:
		{
			return 0;
		}
		case WM_NCHITTEST:
		{
			return 0;
		}
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}

//-----------------------------------------------------------------------------
void Win32DragBitmapWindow::mouseChanged ()
{
	updateWindowPosition (calculateWindowPosition ());
}

//-----------------------------------------------------------------------------
LRESULT CALLBACK Win32DragBitmapWindow::WndProc (HWND hWnd, UINT message, WPARAM wParam,
                                                 LPARAM lParam)
{
	auto window = reinterpret_cast<Win32DragBitmapWindow*> ((LONG_PTR)GetWindowLongPtr (hWnd, GWLP_USERDATA));
	if (window)
		return window->proc (message, wParam, lParam);
	return DefWindowProc (hWnd, message, wParam, lParam);
}

//-----------------------------------------------------------------------------
void Win32DragBitmapWindow::registerWindowClass ()
{
	char tmp[32];
	sprintf_s (tmp, 32, "%p", this);
	windowClassName = "VSTGUI DragBitmap Window ";
	windowClassName += tmp;

	auto winString = dynamic_cast<WinString*> (windowClassName.getPlatformString ());

	WNDCLASSEX wcex {};

	wcex.cbSize = sizeof (WNDCLASSEX);

	wcex.style = 0;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = GetInstance ();
	wcex.hCursor = LoadCursor (GetInstance (), IDC_ARROW);
	wcex.hbrBackground = nullptr;
	wcex.lpszClassName = winString->getWideString ();

	RegisterClassEx (&wcex);
}

//-----------------------------------------------------------------------------
void Win32DragBitmapWindow::unregisterWindowClass ()
{
	auto winString = dynamic_cast<WinString*> (windowClassName.getPlatformString ());
	UnregisterClass (winString->getWideString (), GetInstance ());
}

} // VSTGUI
