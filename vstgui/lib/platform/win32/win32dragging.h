// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../vstguifwd.h"

#if WINDOWS
struct IUnknown;

#include "../../dragging.h"
#include "win32frame.h"
#include "win32datapackage.h"
#include <windows.h>
#include <oleidl.h>
#include <memory>

namespace VSTGUI {

class Win32DragBitmapWindow;
class Win32MouseObserverWhileDragging;

//-----------------------------------------------------------------------------
class Win32DraggingSession final : public IDraggingSession
{
public:
	Win32DraggingSession (Win32Frame* frame);
	~Win32DraggingSession () noexcept;

	bool setBitmap (const SharedPointer<CBitmap>& bitmap, CPoint offset) const final;

	bool doDrag (const DragDescription& dragDescription, const SharedPointer<IDragCallback>& callback);
private:
	Win32Frame* frame;
	mutable std::unique_ptr<Win32DragBitmapWindow> dragBitmapWindow;
	mutable std::unique_ptr<Win32MouseObserverWhileDragging> mouseObserver;
};

//------------------------------------------------------------------------
struct COMBase
{
	virtual ~COMBase () = default;

	ULONG addRef () noexcept
	{
		++refCount;
		return refCount;
	}
	ULONG release () noexcept
	{
		if (--refCount == 0u)
		{
			delete this;
			return 0u;
		}
		return refCount.load ();
	}

	std::atomic<ULONG> refCount {1};
};

//-----------------------------------------------------------------------------
class CDropTarget final : private COMBase,
						  public ::IDropTarget
{
public:
	CDropTarget (Win32Frame* pFrame);
	~CDropTarget () noexcept override;

	// IUnknown
	STDMETHOD (QueryInterface) (REFIID riid, void** object) override;
	STDMETHOD_ (ULONG, AddRef) () override { return addRef (); }
	STDMETHOD_ (ULONG, Release) () override { return release (); }

	// IDropTarget
	STDMETHOD (DragEnter) (IDataObject* dataObject, DWORD keyState, POINTL pt, DWORD* effect) override;
	STDMETHOD (DragOver) (DWORD keyState, POINTL pt, DWORD* effect) override;
	STDMETHOD (DragLeave) () override;
	STDMETHOD (Drop) (IDataObject* dataObject, DWORD keyState, POINTL pt, DWORD* effect) override;

private:
	Win32Frame* pFrame;
	SharedPointer<Win32DataPackage> dragData;
};

//-----------------------------------------------------------------------------
class Win32DropSource final : private COMBase,
							  public ::IDropSource
{
public:
	// IUnknown
	STDMETHOD (QueryInterface) (REFIID riid, void** object) override;
	STDMETHOD_ (ULONG, AddRef) () override { return addRef (); }
	STDMETHOD_ (ULONG, Release) () override { return release (); }

	// IDropSource
	STDMETHOD (QueryContinueDrag) (BOOL escapePressed, DWORD keyState) override;
	STDMETHOD (GiveFeedback) (DWORD effect) override;
};

//-----------------------------------------------------------------------------
class Win32DataObject final : private COMBase,
							  public ::IDataObject
{
public:
	Win32DataObject (SharedPointer<IDataPackage> dataPackage);
	~Win32DataObject () noexcept override;

	// IUnknown
	STDMETHOD (QueryInterface) (REFIID riid, void** object) override;
	STDMETHOD_ (ULONG, AddRef) () override { return addRef (); }
	STDMETHOD_ (ULONG, Release) () override { return release (); }

	// IDataObject
	STDMETHOD (GetData) (FORMATETC *format, STGMEDIUM *medium) override;
	STDMETHOD (GetDataHere) (FORMATETC *format, STGMEDIUM *medium) override;
	STDMETHOD (QueryGetData) (FORMATETC *format) override;
	STDMETHOD (GetCanonicalFormatEtc) (FORMATETC *formatIn, FORMATETC *formatOut) override;
	STDMETHOD (SetData) (FORMATETC *format, STGMEDIUM *medium, BOOL release) override;
	STDMETHOD (EnumFormatEtc) (DWORD direction, IEnumFORMATETC** enumFormat) override;
	STDMETHOD (DAdvise) (FORMATETC* format, DWORD advf, IAdviseSink* advSink, DWORD* connection) override;
	STDMETHOD (DUnadvise) (DWORD connection) override;
	STDMETHOD (EnumDAdvise) (IEnumSTATDATA** enumAdvise) override;
private:
	SharedPointer<IDataPackage> dataPackage;
};

} // VSTGUI

#endif
