// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32dragging.h"
#include "win32support.h"
#include "../../cvstguitimer.h"
#include <shlobj.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
Win32DraggingSession::Win32DraggingSession (Win32Frame* frame)
: frame (frame)
{
}

//-----------------------------------------------------------------------------
bool Win32DraggingSession::setBitmap (const SharedPointer<CBitmap>& bitmap, CPoint offset)
{
	return false;
}

//-----------------------------------------------------------------------------
bool Win32DraggingSession::doDrag (const DragDescription& dragDescription, const SharedPointer<IDragCallback>& callback)
{
	auto lastCursor = frame->getLastSetCursor ();
	frame->setMouseCursor (kCursorNotAllowed);

	// TODO: implement drag bitmap
	if (callback)
	{
		CPoint location;
		frame->getCurrentMousePosition (location);
		callback->dragWillBegin (this, location);
	}

	auto dataObject = new Win32DataObject (dragDescription.data);
	auto dropSource = new Win32DropSource ();
	DWORD outEffect;

	SharedPointer<CVSTGUITimer> timer;
	if (callback)
	{
		CPoint location;
		frame->getCurrentMousePosition (location);
		timer = makeOwned<CVSTGUITimer> ([&] (CVSTGUITimer* timer) {
			CPoint newLocation;
			frame->getCurrentMousePosition (newLocation);
			if (newLocation != location)
			{
				callback->dragMoved (this, newLocation);
				location = newLocation;
			}
		}, 16);
	}

	auto hResult = DoDragDrop (dataObject, dropSource, DROPEFFECT_COPY, &outEffect);
	if (timer)
		timer = nullptr;

	if (callback)
	{
		CPoint location;
		frame->getCurrentMousePosition (location);
		if (hResult == DRAGDROP_S_DROP)
		{
			if (outEffect == DROPEFFECT_MOVE)
				callback->dragEnded (this, location, kDragMoved);
			else
				callback->dragEnded (this, location, kDragCopied);
		}
		else
		{
			callback->dragEnded (this, location, kDragRefused);
		}
	}

	dataObject->Release ();
	dropSource->Release ();

	frame->setMouseCursor (lastCursor);
	return true;
}

//-----------------------------------------------------------------------------
CDropTarget::CDropTarget (Win32Frame* pFrame)
: refCount (0)
, pFrame (pFrame)
, gDragContainer (0)
{
}

//-----------------------------------------------------------------------------
CDropTarget::~CDropTarget () noexcept
{
}

//-----------------------------------------------------------------------------
STDMETHODIMP CDropTarget::QueryInterface (REFIID riid, void** object)
{
	if (riid == IID_IDropTarget || riid == IID_IUnknown)
	{
		*object = this;
		AddRef ();
      return NOERROR;
	}
	*object = 0;
	return E_NOINTERFACE;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CDropTarget::AddRef (void)
{
	return static_cast<ULONG> (++refCount);
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CDropTarget::Release (void)
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
STDMETHODIMP CDropTarget::DragEnter (IDataObject* dataObject, DWORD keyState, POINTL pt, DWORD* effect)
{
	if (dataObject && pFrame)
	{
		gDragContainer = new WinDragContainer (dataObject);
		CPoint where;
		pFrame->getCurrentMousePosition (where);
		pFrame->getFrame ()->platformOnDragEnter (gDragContainer, where);
		if ((*effect) & DROPEFFECT_COPY) 
			*effect = DROPEFFECT_COPY;
		else if ((*effect) & DROPEFFECT_MOVE) 
			*effect = DROPEFFECT_MOVE;
		else if ((*effect) & DROPEFFECT_LINK) 
			*effect = DROPEFFECT_LINK;
		if (pFrame->getLastSetCursor () == kCursorNotAllowed)
			*effect = DROPEFFECT_NONE;
	}
	else
		*effect = DROPEFFECT_NONE;
	return S_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CDropTarget::DragOver (DWORD keyState, POINTL pt, DWORD* effect)
{
	if (gDragContainer && pFrame)
	{
		CPoint where;
		pFrame->getCurrentMousePosition (where);
		pFrame->getFrame ()->platformOnDragMove (gDragContainer, where);
		if ((*effect) & DROPEFFECT_COPY) 
			*effect = DROPEFFECT_COPY;
		else if ((*effect) & DROPEFFECT_MOVE) 
			*effect = DROPEFFECT_MOVE;
		else if ((*effect) & DROPEFFECT_LINK) 
			*effect = DROPEFFECT_LINK;
		if (pFrame->getLastSetCursor () == kCursorNotAllowed)
			*effect = DROPEFFECT_NONE;
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CDropTarget::DragLeave (void)
{
	if (gDragContainer && pFrame)
	{
		CPoint where;
		pFrame->getCurrentMousePosition (where);
		pFrame->getFrame ()->platformOnDragLeave (gDragContainer, where);
		gDragContainer->forget ();
		gDragContainer = 0;
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CDropTarget::Drop (IDataObject* dataObject, DWORD keyState, POINTL pt, DWORD* effect)
{
	if (gDragContainer && pFrame)
	{
		CPoint where;
		pFrame->getCurrentMousePosition (where);
		pFrame->getFrame ()->platformOnDrop (gDragContainer, where);
		gDragContainer->forget ();
		gDragContainer = 0;
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
// Win32DropSource
//-----------------------------------------------------------------------------
STDMETHODIMP Win32DropSource::QueryInterface (REFIID riid, void** object)
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
STDMETHODIMP Win32DropSource::QueryContinueDrag (BOOL escapePressed, DWORD keyState)
{
	if (escapePressed)
		return DRAGDROP_S_CANCEL;
	
	if ((keyState & (MK_LBUTTON|MK_RBUTTON)) == 0)
		return DRAGDROP_S_DROP;

	return S_OK;	
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DropSource::GiveFeedback (DWORD effect)
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
STDMETHODIMP Win32DataObject::QueryInterface (REFIID riid, void** object)
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
STDMETHODIMP Win32DataObject::GetData (FORMATETC* format, STGMEDIUM* medium)
{
	medium->tymed = 0;
	medium->hGlobal = 0;
	medium->pUnkForRelease = 0;

	if (format->cfFormat == CF_TEXT || format->cfFormat == CF_UNICODETEXT)
	{
		for (uint32_t i = 0; i < dataPackage->getCount (); i++)
		{
			if (dataPackage->getDataType (i) == IDataPackage::kText)
			{
				const void* buffer;
				IDataPackage::Type type;
				uint32_t bufferSize = dataPackage->getData (i, buffer, type);
				UTF8StringHelper utf8String ((const char*)buffer);
				SIZE_T size = 0;
				const void* data = 0;
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
		}
	}
	else if (format->cfFormat == CF_HDROP)
	{
		HRESULT result = E_UNEXPECTED;
		UTF8StringHelper** wideStringFileNames = (UTF8StringHelper**)std::malloc (sizeof (UTF8StringHelper*) * dataPackage->getCount ());
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
		HGLOBAL	memoryHandle = GlobalAlloc (GMEM_MOVEABLE, bufferSizeNeeded); 
		void* memory = GlobalLock (memoryHandle);
		if (memory)
		{
			DROPFILES* dropFiles = (DROPFILES*)memory;
			dropFiles->pFiles = sizeof (DROPFILES);
			dropFiles->pt.x   = 0; 
			dropFiles->pt.y   = 0;
			dropFiles->fNC    = FALSE;
			dropFiles->fWide  = TRUE;
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
				void* memory = GlobalLock (memoryHandle);
				if (memory)
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
STDMETHODIMP Win32DataObject::GetDataHere (FORMATETC *format, STGMEDIUM *pmedium)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DataObject::QueryGetData (FORMATETC *format)
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
STDMETHODIMP Win32DataObject::GetCanonicalFormatEtc (FORMATETC *formatIn, FORMATETC *formatOut)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DataObject::SetData (FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DataObject::EnumFormatEtc (DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DataObject::DAdvise (FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DataObject::DUnadvise (DWORD dwConnection)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP Win32DataObject::EnumDAdvise (IEnumSTATDATA** ppenumAdvise)
{
	return E_NOTIMPL;
}

} // namespace
