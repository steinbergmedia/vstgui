// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32dragging.h"

namespace VSTGUI {

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

} // namespace
