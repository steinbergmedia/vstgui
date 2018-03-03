// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../vstguifwd.h"

#if WINDOWS
#include "../../dragging.h"
#include "win32frame.h"
#include "win32dragcontainer.h"
#include <windows.h>

namespace VSTGUI {


//-----------------------------------------------------------------------------
class Win32DraggingSession : public IDraggingSession
{
public:
	Win32DraggingSession (const SharedPointer<IDragCallback>& callback)
	: callback (callback)
	{}

	bool setBitmap (const SharedPointer<CBitmap>& bitmap, CPoint offset) final
	{
		return false;
	}

	SharedPointer<IDragCallback> callback;
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
	WinDragContainer* gDragContainer;
};

//-----------------------------------------------------------------------------
class Win32DropSource : public AtomicReferenceCounted, public ::IDropSource
{
public:
	Win32DropSource () = default;

	// IUnknown
	STDMETHOD (QueryInterface) (REFIID riid, void** object);
	STDMETHOD_ (ULONG, AddRef) (void) { remember (); return static_cast<ULONG> (getNbReference ());}
	STDMETHOD_ (ULONG, Release) (void) { ULONG refCount = static_cast<ULONG> (getNbReference ()) - 1; forget (); return refCount; }
	
	// IDropSource
	STDMETHOD (QueryContinueDrag) (BOOL escapePressed, DWORD keyState);
	STDMETHOD (GiveFeedback) (DWORD effect);
};


} // namespace

#endif
