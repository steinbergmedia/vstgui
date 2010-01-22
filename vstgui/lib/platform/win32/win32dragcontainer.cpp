//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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

#include "win32dragcontainer.h"

#if WINDOWS

#include <shlobj.h>
#include <shellapi.h>
#include "win32support.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
WinDragContainer::WinDragContainer (IDataObject* platformDrag)
: platformDrag (platformDrag)
, nbItems (0)
, iterator (0)
, lastItem (0)
{
	if (!platformDrag)
		return;

	IDataObject* dataObject = platformDrag;
	STGMEDIUM medium;
	FORMATETC formatTEXTDrop = {
		CF_UNICODETEXT,
		0, 
		DVASPECT_CONTENT, 
		-1, 
		TYMED_HGLOBAL
	};
	
	FORMATETC formatHDrop    = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	// todo : Support CF_UNICODETEXT

	long type = 0; // 0 = file, 1 = text

	HRESULT hr = dataObject->GetData (&formatTEXTDrop, &medium);
	if (hr != S_OK)
		hr = dataObject->GetData (&formatHDrop, &medium);
	else
		type = 1;
	
	if (type == 0)
		nbItems = (long)DragQueryFile ((HDROP)medium.hGlobal, 0xFFFFFFFFL, 0, 0);
	else
		nbItems = 1;
}

//-----------------------------------------------------------------------------
WinDragContainer::~WinDragContainer ()
{
	if (lastItem)
	{
		free (lastItem);
		lastItem = 0;
	}
}

//-----------------------------------------------------------------------------
long WinDragContainer::getType (long idx) const
{
	if (platformDrag == 0)
		return kError;

	IDataObject* dataObject = platformDrag;
	STGMEDIUM medium;
	FORMATETC formatTEXTDrop = {
		CF_UNICODETEXT,
		0, 
		DVASPECT_CONTENT, 
		-1, 
		TYMED_HGLOBAL
	};
	FORMATETC formatHDrop    = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	long type = 0; // 0 = file, 1 = text

	HRESULT hr = dataObject->GetData (&formatTEXTDrop, &medium);
	if (hr != S_OK)
		hr = dataObject->GetData (&formatHDrop, &medium);
	else
		type = 1;
	if (type == 0)
		return kFile;
	else
		return kText;

	return kUnknown;
}

//-----------------------------------------------------------------------------
void* WinDragContainer::first (long& size, long& type)
{
	iterator = 0;
	return next (size, type);
}

//-----------------------------------------------------------------------------
void* WinDragContainer::next (long& size, long& type)
{
	if (platformDrag == 0)
	{
		type = kError;
		return 0;
	}
	if (lastItem)
	{
		free (lastItem);
		lastItem = 0;
	}
	size = 0;
	type = kUnknown;

	IDataObject* dataObject = platformDrag;
	void* hDrop = 0;
	STGMEDIUM medium;
	FORMATETC formatTEXTDrop = {
		CF_UNICODETEXT,
		0, 
		DVASPECT_CONTENT, 
		-1, 
		TYMED_HGLOBAL
	};
	FORMATETC formatHDrop    = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	long wintype = 0; // 0 = file, 1 = text

	HRESULT hr = dataObject->GetData (&formatTEXTDrop, &medium);
	if (hr != S_OK)
		hr = dataObject->GetData (&formatHDrop, &medium);
	else
		wintype = 1;
	if (hr == S_OK)
		hDrop = medium.hGlobal;

	if (hDrop)
	{
		if (wintype == 0)
		{
			TCHAR fileDropped[1024];

			long nbRealItems = 0;
			if (DragQueryFile ((HDROP)hDrop, iterator++, fileDropped, sizeof (fileDropped))) 
			{
				// resolve link
				checkResolveLink (fileDropped, fileDropped);
				UTF8StringHelper path (fileDropped);
				lastItem = malloc (strlen (path)+1);
				strcpy ((char*)lastItem, path);
				size = (long)strlen ((const char*)lastItem);
				type = kFile;
				return lastItem;
			}
		}
		else if (iterator++ == 0)
		//---TEXT----------------------------
		{
			void* data = GlobalLock (medium.hGlobal);
			long dataSize = (long)GlobalSize (medium.hGlobal);
			if (data && dataSize)
			{
				UTF8StringHelper wideString ((const WCHAR*)data);
				size = strlen (wideString.getUTF8String ());
				lastItem = malloc (size+1);
				strcpy ((char*)lastItem, wideString.getUTF8String ());
				type = kUnicodeText;
			}

			GlobalUnlock (medium.hGlobal);
			if (medium.pUnkForRelease)
				medium.pUnkForRelease->Release ();
			else
				GlobalFree (medium.hGlobal);
			return lastItem;
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
bool WinDragContainer::checkResolveLink (const TCHAR* nativePath, TCHAR* resolved)
{
	const TCHAR* ext = VSTGUI_STRRCHR (nativePath, '.');
	if (ext && VSTGUI_STRICMP (ext, TEXT(".lnk")) == NULL)
	{
		IShellLink* psl;
		IPersistFile* ppf;
		WIN32_FIND_DATA wfd;
		HRESULT hres;
		WORD wsz[2048];
		
		// Get a pointer to the IShellLink interface.
		hres = CoCreateInstance (CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
			IID_IShellLink, (void**)&psl);
		if (SUCCEEDED (hres))
		{
			// Get a pointer to the IPersistFile interface.
			hres = psl->QueryInterface (IID_IPersistFile, (void**)&ppf);
			if (SUCCEEDED (hres))
			{
				// Load the shell link.
				hres = ppf->Load ((LPWSTR)wsz, STGM_READ);
				if (SUCCEEDED (hres))
				{					
					hres = psl->Resolve (0, MAKELONG (SLR_ANY_MATCH | SLR_NO_UI, 500));
					if (SUCCEEDED (hres))
					{
						// Get the path to the link target.
						hres = psl->GetPath (resolved, 2048, &wfd, SLGP_SHORTPATH);
					}
				}
				// Release pointer to IPersistFile interface.
				ppf->Release ();
			}
			// Release pointer to IShellLink interface.
			psl->Release ();
		}
		return SUCCEEDED(hres);
	}
	return false;	
}

} // namespace

#endif // WINDOWS