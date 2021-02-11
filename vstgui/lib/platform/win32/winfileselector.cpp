// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../cfileselector.h"

#if WINDOWS

#include "win32support.h"
#include "../platform_win32.h"
#include "../../cstring.h"
#include "../../cframe.h"
#include <shobjidl.h>
#include <shlobj.h>
#include <commdlg.h>
#include <string>

#define IID_PPV_ARG(IType, ppType) IID_##IType, (void**)ppType

extern OSVERSIONINFOEX	gSystemVersion;

namespace VSTGUI {

#define kAllSupportedFileTypesString	L"All supported file types"

//-----------------------------------------------------------------------------
static COMDLG_FILTERSPEC* buildExtensionFilter (std::list<CFileExtension>& extensions, const CFileExtension* defaultExtension, DWORD& numExtensions, DWORD& defaultFileTypeIndex)
{
	if (extensions.empty () == false)
	{
		DWORD i = extensions.size () > 1 ? 1u : 0u;
		auto* filters = new COMDLG_FILTERSPEC[extensions.size () + 1 + i];
		size_t allExtensionCharCount = 0;
		std::list<CFileExtension>::iterator it = extensions.begin ();
		while (it != extensions.end ())
		{
			UTF8StringHelper desc ((*it).getDescription ().data ());
			UTF8StringHelper ext ((*it).getExtension ().data ());
			WCHAR* wDesc = (WCHAR*)std::malloc ((wcslen (desc) + 1) * sizeof (WCHAR));
			WCHAR* wSpec = (WCHAR*)std::malloc ((wcslen (ext) + 3) * sizeof (WCHAR));
			if (wDesc && wSpec)
			{
				memcpy (wDesc, desc.getWideString (), (wcslen (desc) + 1) * sizeof (WCHAR));
				memcpy (wSpec + 2, ext.getWideString (), (wcslen (ext) + 1) * sizeof (WCHAR));
				wSpec[0] = 0x002a; // *
				wSpec[1] = 0x002e; // .

				filters[i].pszName = wDesc;
				filters[i].pszSpec = wSpec;
				if (defaultExtension && *defaultExtension == (*it))
					defaultFileTypeIndex = i + 1;
				allExtensionCharCount += wcslen (filters[i].pszSpec) + 1;
			}
			else
			{
				filters[i].pszName = nullptr;
				filters[i].pszSpec = nullptr;
				break;
			}

			it++;
			i++;
		}
		if (extensions.size () > 1)
		{
			WCHAR* wAllName =
			    (WCHAR*)std::malloc ((wcslen (kAllSupportedFileTypesString) + 1) * sizeof (WCHAR));
			WCHAR* wAllSpec = (WCHAR*)std::malloc (allExtensionCharCount * sizeof (WCHAR));
			if (wAllName && wAllSpec)
			{
				wcscpy (wAllName, kAllSupportedFileTypesString);
				wAllSpec[0] = 0;
				for (DWORD j = 1; j < i; j++)
				{
					wcscat (wAllSpec, filters[j].pszSpec);
					if (j != i - 1)
						wcscat (wAllSpec, L";");
				}
			}
			filters[0].pszName = wAllName;
			filters[0].pszSpec = wAllSpec;
		}
		numExtensions = i;
		filters[i].pszName = nullptr;
		filters[i].pszSpec = nullptr;
		return filters;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
static void freeExtensionFilter (COMDLG_FILTERSPEC* filters)
{
	if (filters)
	{
		int32_t i = 0;
		while (filters[i].pszName)
		{
			std::free ((void*)filters[i].pszName);
			std::free ((void*)filters[i].pszSpec);
			i++;
		}
		delete[] filters;
	}
}

//-----------------------------------------------------------------------------
class VistaFileSelector final : public CNewFileSelector
{
public:
	VistaFileSelector (CFrame* frame, Style style);
	~VistaFileSelector () noexcept;

	bool runInternal (CBaseObject* delegate) override;
	void cancelInternal () override;
	bool runModalInternal () override;

protected:
	Style style;
	IFileDialog* fileDialog;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CNewFileSelector* CNewFileSelector::create (CFrame* parent, Style style)
{
	if (parent == nullptr)
	{
		#if DEBUG
		DebugPrint ("Need frame for CNewFileSelector\n");
		#endif
		return nullptr;
	}
	return new VistaFileSelector (parent, style);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef __GNUC__
using SHCreateItemFromParsingNameProc = HRESULT (STDAPICALLTYPE *) (__in PCWSTR pszPath, __in_opt IBindCtx *pbc, __in REFIID riid, __deref_out void **ppv);
#else
using SHCreateItemFromParsingNameProc = HRESULT (STDAPICALLTYPE *) (PCWSTR pszPath, IBindCtx *pbc, REFIID riid, void **ppv);
#endif
SHCreateItemFromParsingNameProc _SHCreateItemFromParsingName = nullptr;

//-----------------------------------------------------------------------------
VistaFileSelector::VistaFileSelector (CFrame* frame, Style style)
: CNewFileSelector (frame)
, style (style)
, fileDialog (nullptr)
{
	if (_SHCreateItemFromParsingName == nullptr)
	{
		HINSTANCE shell32Instance = LoadLibraryA ("shell32.dll");
		if (shell32Instance)
		{
			_SHCreateItemFromParsingName = (SHCreateItemFromParsingNameProc)GetProcAddress (shell32Instance, "SHCreateItemFromParsingName");
		}
	}
}

//-----------------------------------------------------------------------------
VistaFileSelector::~VistaFileSelector () noexcept
{
}

//-----------------------------------------------------------------------------
bool VistaFileSelector::runInternal (CBaseObject* delegate)
{
	bool result = runModalInternal ();
	if (delegate)
	{
		delegate->notify (this, kSelectEndMessage);
	}
	return result;
}

//-----------------------------------------------------------------------------
void VistaFileSelector::cancelInternal ()
{
	if (fileDialog)
		fileDialog->Close (-1);
}

//-----------------------------------------------------------------------------
bool VistaFileSelector::runModalInternal ()
{
	fileDialog = nullptr;
	HRESULT hr = E_UNEXPECTED;
	if (style == kSelectSaveFile)
	{
		hr = CoCreateInstance (CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IFileDialog, &fileDialog));
		if (!defaultSaveName.empty ())
		{
			fileDialog->SetFileName (UTF8StringHelper (defaultSaveName.data ()));
		}
	}
	else
	{
		hr = CoCreateInstance (CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IFileDialog, &fileDialog));
		if (SUCCEEDED (hr))
		{
			if (style == kSelectDirectory)
			{
				DWORD dwOptions = 0;
				hr = fileDialog->GetOptions (&dwOptions);
				if (SUCCEEDED (hr))
					hr = fileDialog->SetOptions (dwOptions | FOS_PICKFOLDERS);
				if (FAILED (hr))
				{
					fileDialog->Release ();
					fileDialog = nullptr;
					return false;
				}
			}
			if (allowMultiFileSelection)
			{
				DWORD dwOptions = 0;
				hr = fileDialog->GetOptions (&dwOptions);
				if (SUCCEEDED (hr))
					hr = fileDialog->SetOptions (dwOptions | FOS_ALLOWMULTISELECT);
				if (FAILED (hr))
				{
					fileDialog->Release ();
					fileDialog = nullptr;
					return false;
				}
			}
		}
	}
	if (FAILED (hr))
	{
		fileDialog = nullptr;
		return false;
	}

	if (!title.empty ())
		hr = fileDialog->SetTitle (UTF8StringHelper (title.data ()));

	DWORD numExtensions = 0;
	DWORD defaultFileTypeIndex = 0;
	COMDLG_FILTERSPEC* filters = buildExtensionFilter (extensions, defaultExtension, numExtensions, defaultFileTypeIndex);
	if (filters)
	{
		fileDialog->SetFileTypes (numExtensions, filters);
		if (defaultFileTypeIndex)
			fileDialog->SetFileTypeIndex (defaultFileTypeIndex);
	}
	if (!initialPath.empty () && _SHCreateItemFromParsingName)
	{
		IShellItem* shellItem;
		hr = _SHCreateItemFromParsingName (UTF8StringHelper (initialPath.data ()), nullptr, IID_PPV_ARG (IShellItem, &shellItem));
		if (SUCCEEDED (hr))
		{
			fileDialog->SetFolder (shellItem);
			shellItem->Release ();
		}
	}
	auto win32Frame = dynamic_cast<IWin32PlatformFrame*> (frame->getPlatformFrame ());
	hr = fileDialog->Show (win32Frame ? win32Frame->getHWND () : nullptr);
	if (SUCCEEDED (hr))
	{
		if (allowMultiFileSelection)
		{
			IFileOpenDialog* openFileDialog = nullptr;
			hr = fileDialog->QueryInterface (IID_PPV_ARG(IFileOpenDialog, &openFileDialog));
			if (SUCCEEDED (hr))
			{
				IShellItemArray* items;
				hr = openFileDialog->GetResults (&items);
				if (SUCCEEDED (hr))
				{
					DWORD count;
					hr = items->GetCount (&count);
					for (DWORD i = 0; i < count; i++)
					{
						IShellItem* item;
						hr = items->GetItemAt (i, &item);
						if (SUCCEEDED (hr))
						{
							LPWSTR filesysPath = nullptr;
							hr = item->GetDisplayName (SIGDN_FILESYSPATH, &filesysPath);
							if (SUCCEEDED (hr))
							{
								UTF8StringHelper str (filesysPath);
								result.emplace_back (str.getUTF8String ());
							}
							item->Release ();
						}
					}
					items->Release ();
				}
				openFileDialog->Release ();
			}
		}
		else
		{
			IShellItem *item;
			hr = fileDialog->GetResult (&item);
			if (SUCCEEDED (hr))
			{
				LPWSTR filesysPath = nullptr;
				hr = item->GetDisplayName (SIGDN_FILESYSPATH, &filesysPath);
				if (SUCCEEDED (hr))
				{
					UTF8StringHelper str (filesysPath);
					result.emplace_back (str.getUTF8String ());
				}
				item->Release ();
			}
		}
	}
	fileDialog->Release ();
	fileDialog = nullptr;
	freeExtensionFilter (filters);
	return SUCCEEDED (hr);
}

} // VSTGUI

#endif // WINDOWS
