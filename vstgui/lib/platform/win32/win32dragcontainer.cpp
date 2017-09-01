// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32dragcontainer.h"

#if WINDOWS

#include <shlobj.h>
#include <shellapi.h>
#include "win32support.h"

namespace VSTGUI {

FORMATETC WinDragContainer::formatTEXTDrop		= {CF_UNICODETEXT,0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
FORMATETC WinDragContainer::formatHDrop			= {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
FORMATETC WinDragContainer::formatBinaryDrop	= {CF_PRIVATEFIRST, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

//-----------------------------------------------------------------------------
WinDragContainer::WinDragContainer (IDataObject* platformDrag)
: platformDrag (platformDrag)
, nbItems (0)
, stringsAreFiles (false)
, data (0)
, dataSize (0)
{
	if (!platformDrag)
		return;

	STGMEDIUM medium = {0};
	HRESULT hr = platformDrag->QueryGetData (&formatTEXTDrop);
	if (hr == S_OK) // text
	{
		hr = platformDrag->GetData (&formatTEXTDrop, &medium);
		if (hr == S_OK)
		{
			void* data = GlobalLock (medium.hGlobal);
			uint32_t dataSize = static_cast<uint32_t> (GlobalSize (medium.hGlobal));
			if (data && dataSize)
			{
				UTF8StringHelper wideString ((const WCHAR*)data);
				strings.emplace_back (wideString);
				nbItems = 1;
			}
			GlobalUnlock (medium.hGlobal);
			if (medium.pUnkForRelease)
				medium.pUnkForRelease->Release ();
			else
				GlobalFree (medium.hGlobal);
		}
	}
	else if (hr != S_OK)
	{
		hr = platformDrag->QueryGetData (&formatHDrop);
		if (hr == S_OK)
		{
			hr = platformDrag->GetData (&formatHDrop, &medium);
			if (hr == S_OK)
			{
				nbItems = DragQueryFile ((HDROP)medium.hGlobal, 0xFFFFFFFFL, 0, 0);
				stringsAreFiles = true;

				TCHAR fileDropped[1024];
				for (uint32_t index = 0; index < nbItems; index++)
				{
					if (DragQueryFile ((HDROP)medium.hGlobal, index, fileDropped, sizeof (fileDropped) / 2)) 
					{
						// resolve link
						checkResolveLink (fileDropped, fileDropped);
						UTF8StringHelper path (fileDropped);
						strings.emplace_back (path);
					}
				}
			}
		}
		else if (platformDrag->QueryGetData (&formatBinaryDrop) == S_OK)
		{
			if (platformDrag->GetData (&formatBinaryDrop, &medium) == S_OK)
			{
				const void* blob = GlobalLock (medium.hGlobal);
				dataSize = static_cast<uint32_t> (GlobalSize (medium.hGlobal));
				if (blob && dataSize)
				{
					data = std::malloc (dataSize);
					memcpy (data, blob, dataSize);
					nbItems = 1;
				}
				GlobalUnlock (medium.hGlobal);
				if (medium.pUnkForRelease)
					medium.pUnkForRelease->Release ();
				else
					GlobalFree (medium.hGlobal);
			}
		}
	}
}

//-----------------------------------------------------------------------------
WinDragContainer::~WinDragContainer () noexcept
{
	if (data)
	{
		std::free (data);
	}
}

//-----------------------------------------------------------------------------
uint32_t WinDragContainer::getCount () const
{
	return nbItems;
}

//-----------------------------------------------------------------------------
uint32_t WinDragContainer::getDataSize (uint32_t index) const
{
	if (index < nbItems)
	{
		if (data)
			return dataSize;
		return static_cast<uint32_t> (strings[index].length ());
	}
	return 0;
}

//-----------------------------------------------------------------------------
WinDragContainer::Type WinDragContainer::getDataType (uint32_t index) const
{
	if (index < nbItems)
	{
		if (data)
			return kBinary;
		if (stringsAreFiles)
			return kFilePath;
		return kText;
	}
	return kError;
}

//-----------------------------------------------------------------------------
uint32_t WinDragContainer::getData (uint32_t index, const void*& buffer, Type& type) const
{
	if (index < nbItems)
	{
		if (data)
		{
			buffer = data;
			type = kBinary;
			return dataSize;
		}
		buffer = strings[index].c_str ();
		type = stringsAreFiles ? kFilePath : kText;
		return static_cast<uint32_t> (strings[index].length ());
	}
	return 0;
}

//-----------------------------------------------------------------------------
bool WinDragContainer::checkResolveLink (const TCHAR* nativePath, TCHAR* resolved)
{
	const TCHAR* ext = VSTGUI_STRRCHR (nativePath, '.');
	if (ext && VSTGUI_STRICMP (ext, TEXT(".lnk")) == 0)
	{
		IShellLink* psl;
		IPersistFile* ppf;
		WIN32_FIND_DATA wfd;
		HRESULT hres;
		
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
				hres = ppf->Load (nativePath, STGM_READ);
				if (SUCCEEDED (hres))
				{					
					hres = psl->Resolve (0, static_cast<DWORD> (MAKELONG (SLR_ANY_MATCH | SLR_NO_UI, 500)));
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
