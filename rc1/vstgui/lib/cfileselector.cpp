//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "cfileselector.h"
#include "cframe.h"
#include "cstring.h"
#include <algorithm>

namespace VSTGUI {

//-----------------------------------------------------------------------------
CFileExtension::CFileExtension (UTF8StringPtr inDescription, UTF8StringPtr inExtension, UTF8StringPtr inMimeType, int32_t inMacType, UTF8StringPtr inUti)
: description (0)
, extension (0)
, mimeType (0)
, uti (0)
, macType (inMacType)
{
	init (inDescription, inExtension, inMimeType, inUti);
}

//-----------------------------------------------------------------------------
CFileExtension::CFileExtension (const CFileExtension& ext)
: description (0)
, extension (0)
, mimeType (0)
, uti (0)
, macType (ext.macType)
{
	init (ext.description, ext.extension, ext.mimeType, ext.uti);
}

//-----------------------------------------------------------------------------
CFileExtension::~CFileExtension ()
{
	String::free (description);
	String::free (extension);
	String::free (mimeType);
	String::free (uti);
}

#if VSTGUI_RVALUE_REF_SUPPORT
//-----------------------------------------------------------------------------
CFileExtension::CFileExtension (CFileExtension&& ext) noexcept
: description (nullptr)
, extension (nullptr)
, mimeType (nullptr)
, uti (nullptr)
{
	*this = std::move (ext);
}

//-----------------------------------------------------------------------------
CFileExtension& CFileExtension::operator=(CFileExtension&& ext) noexcept
{
	String::free (description);
	String::free (extension);
	String::free (mimeType);
	String::free (uti);
	description = ext.description;
	extension = ext.extension;
	mimeType = ext.mimeType;
	uti = ext.uti;
	macType = ext.macType;
	ext.description = nullptr;
	ext.extension = nullptr;
	ext.mimeType = nullptr;
	ext.uti = nullptr;
	ext.macType = 0;
	return *this;
}
#endif

//-----------------------------------------------------------------------------
void CFileExtension::init (UTF8StringPtr inDescription, UTF8StringPtr inExtension, UTF8StringPtr inMimeType, UTF8StringPtr inUti)
{
	description = String::newWithString (inDescription);
	extension = String::newWithString (inExtension);
	mimeType = String::newWithString (inMimeType);
	uti = String::newWithString (inUti);

	if (description == 0 && extension)
	{
		// TODO: query system for file type description
		// Win32: AssocGetPerceivedType
		// Mac: Uniform Type Identifier
	}
}

//-----------------------------------------------------------------------------
bool CFileExtension::operator== (const CFileExtension& ext) const
{
	bool result = false;
	if (extension && ext.extension)
		result = (std::strcmp (extension, ext.extension) == 0);
	if (!result && mimeType && ext.mimeType)
		result = (std::strcmp (mimeType, ext.mimeType) == 0);
	if (!result && uti && ext.uti)
		result = (std::strcmp (uti, ext.uti) == 0);
	if (!result && macType != 0 && ext.macType != 0)
		result = (macType == ext.macType);
	return result;
}

//-----------------------------------------------------------------------------
const CFileExtension& CNewFileSelector::getAllFilesExtension ()
{
	static CFileExtension allFilesExtension ("All Files", "");
	return allFilesExtension;
}

//-----------------------------------------------------------------------------
IdStringPtr CNewFileSelector::kSelectEndMessage = "CNewFileSelector Select End Message";

//-----------------------------------------------------------------------------
// CNewFileSelector Implementation
//-----------------------------------------------------------------------------
CNewFileSelector::CNewFileSelector (CFrame* frame)
: frame (frame)
, defaultExtension (0)
, title (0)
, initialPath (0)
, defaultSaveName (0)
, allowMultiFileSelection (false)
{
}

//-----------------------------------------------------------------------------
CNewFileSelector::~CNewFileSelector ()
{
	setTitle (0);
	setInitialDirectory (0);
	setDefaultSaveName (0);
	std::for_each (result.begin (), result.end (), String::free);
}

//-----------------------------------------------------------------------------
bool CNewFileSelector::run (CBaseObject* delegate)
{
	if (delegate == 0)
	{
		#if DEBUG
		DebugPrint ("You need to specify a delegate in CNewFileSelector::run (CBaseObject* delegate, void* parentWindow)\n");
		#endif
		return false;
	}
	if (frame)
		frame->onStartLocalEventLoop ();
	return runInternal (delegate);
}

//-----------------------------------------------------------------------------
void CNewFileSelector::cancel ()
{
	cancelInternal ();
}

//-----------------------------------------------------------------------------
bool CNewFileSelector::runModal ()
{
	if (frame)
		frame->onStartLocalEventLoop ();
	return runModalInternal ();
}

#if VSTGUI_HAS_FUNCTIONAL
//-----------------------------------------------------------------------------
class CNewFileSelectorCallback : public CBaseObject
{
public:
	CNewFileSelectorCallback (CNewFileSelector::CallbackFunc&& callback) : callbackFunc (std::move (callback)) {}
	~CNewFileSelectorCallback () {}
private:
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD
	{
		if (message == CNewFileSelector::kSelectEndMessage)
		{
			callbackFunc (dynamic_cast<CNewFileSelector*> (sender));
			return kMessageNotified;
		}
		return kMessageUnknown;
	}
	
	CNewFileSelector::CallbackFunc callbackFunc;
};

//-----------------------------------------------------------------------------
bool CNewFileSelector::run (CallbackFunc&& callback)
{
	if (frame)
		frame->onStartLocalEventLoop ();
	OwningPointer<CNewFileSelectorCallback> fsCallback = new CNewFileSelectorCallback (std::move (callback));
	return runInternal (fsCallback);
}

#endif

//-----------------------------------------------------------------------------
void CNewFileSelector::setTitle (UTF8StringPtr inTitle)
{
	String::free (title);
	title = String::newWithString (inTitle);
}

//-----------------------------------------------------------------------------
void CNewFileSelector::setInitialDirectory (UTF8StringPtr path)
{
	String::free (initialPath);
	initialPath = String::newWithString (path);
}

//-----------------------------------------------------------------------------
void CNewFileSelector::setDefaultSaveName (UTF8StringPtr name)
{
	String::free (defaultSaveName);
	defaultSaveName = String::newWithString (name);
}

//-----------------------------------------------------------------------------
void CNewFileSelector::setAllowMultiFileSelection (bool state)
{
	allowMultiFileSelection = state;
}

//-----------------------------------------------------------------------------
void CNewFileSelector::setDefaultExtension (const CFileExtension& extension)
{
	if (defaultExtension)
	{
		#if DEBUG
		DebugPrint ("VSTGUI Warning: It's not allowed to set a default extension twice on a CFileSelector instance\n");
		#endif
		return;
	}

	bool found = false;
	FileExtensionList::const_iterator it = extensions.begin ();
	while (it != extensions.end ())
	{
		if ((*it) == extension)
		{
			defaultExtension = &(*it);
			found = true;
			break;
		}
		it++;
	}
	if (!found)
	{
		addFileExtension (extension);
		setDefaultExtension (extension);
	}
}

//-----------------------------------------------------------------------------
void CNewFileSelector::addFileExtension (const CFileExtension& extension)
{
	extensions.push_back (extension);
}

#if VSTGUI_RVALUE_REF_SUPPORT
//-----------------------------------------------------------------------------
void CNewFileSelector::addFileExtension (CFileExtension&& extension)
{
	extensions.push_back (std::move (extension));
}
#endif

//-----------------------------------------------------------------------------
uint32_t CNewFileSelector::getNumSelectedFiles () const
{
	return static_cast<uint32_t> (result.size ());
}

//-----------------------------------------------------------------------------
UTF8StringPtr CNewFileSelector::getSelectedFile (uint32_t index) const
{
	if (index < result.size ())
		return result[index];
	return 0;
}

} // namespace

