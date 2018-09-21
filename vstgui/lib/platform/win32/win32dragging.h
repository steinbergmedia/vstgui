// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../vstguifwd.h"

#if WINDOWS
#include "../../dragging.h"
#include "win32frame.h"
#include "win32datapackage.h"
#include <windows.h>
#include <memory>

namespace VSTGUI {

class Win32DragBitmapWindow;
class Win32MouseObserverWhileDragging;

//-----------------------------------------------------------------------------
class Win32DraggingSession : public IDraggingSession
{
public:
	Win32DraggingSession (Win32Frame* frame);
	~Win32DraggingSession () noexcept;

	bool setBitmap (const SharedPointer<CBitmap>& bitmap, CPoint offset) final;

	bool doDrag (const DragDescription& dragDescription, const SharedPointer<IDragCallback>& callback);
private:
	Win32Frame* frame;
	std::unique_ptr<Win32DragBitmapWindow> dragBitmapWindow;
	std::unique_ptr<Win32MouseObserverWhileDragging> mouseObserver;
};

//-----------------------------------------------------------------------------
class CDropTarget : public ::IDropTarget
{	
public:
	CDropTarget (Win32Frame* pFrame);
	~CDropTarget () noexcept;

	// IUnknown
	STDMETHOD (QueryInterface) (REFIID riid, void** object);
	STDMETHOD_ (ULONG, AddRef) (void);
	STDMETHOD_ (ULONG, Release) (void);
   
	// IDropTarget
	STDMETHOD (DragEnter) (IDataObject* dataObject, DWORD keyState, POINTL pt, DWORD* effect);
	STDMETHOD (DragOver) (DWORD keyState, POINTL pt, DWORD* effect);
	STDMETHOD (DragLeave) (void);
	STDMETHOD (Drop) (IDataObject* dataObject, DWORD keyState, POINTL pt, DWORD* effect);
private:
	int32_t refCount;
	Win32Frame* pFrame;
	Win32DataPackage* dragData;
};

//-----------------------------------------------------------------------------
class Win32DropSource : public AtomicReferenceCounted, public ::IDropSource
{
public:
	// IUnknown
	STDMETHOD (QueryInterface) (REFIID riid, void** object);
	STDMETHOD_ (ULONG, AddRef) (void) { remember (); return static_cast<ULONG> (getNbReference ());}
	STDMETHOD_ (ULONG, Release) (void) { ULONG refCount = static_cast<ULONG> (getNbReference ()) - 1; forget (); return refCount; }
	
	// IDropSource
	STDMETHOD (QueryContinueDrag) (BOOL escapePressed, DWORD keyState);
	STDMETHOD (GiveFeedback) (DWORD effect);
};

//-----------------------------------------------------------------------------
class Win32DataObject : public AtomicReferenceCounted, public ::IDataObject
{
public:
	Win32DataObject (IDataPackage* dataPackage);
	~Win32DataObject () noexcept;

	// IUnknown
	STDMETHOD (QueryInterface) (REFIID riid, void** object);
	STDMETHOD_ (ULONG, AddRef) (void) { remember (); return static_cast<ULONG> (getNbReference ());}
	STDMETHOD_ (ULONG, Release) (void) { ULONG refCount = static_cast<ULONG> (getNbReference ()) - 1; forget (); return refCount; }

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
	IDataPackage* dataPackage;
};

} // namespace

#endif
