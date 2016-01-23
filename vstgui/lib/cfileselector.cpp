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
CFileExtension::CFileExtension (const UTF8String& inDescription, const UTF8String& inExtension, const UTF8String& inMimeType, int32_t inMacType, const UTF8String& inUti)
: macType (inMacType)
{
	init (inDescription, inExtension, inMimeType, inUti);
}

//-----------------------------------------------------------------------------
CFileExtension::CFileExtension (const CFileExtension& ext)
: macType (ext.macType)
{
	init (ext.description, ext.extension, ext.mimeType, ext.uti);
}

//-----------------------------------------------------------------------------
CFileExtension::~CFileExtension ()
{
}

//-----------------------------------------------------------------------------
CFileExtension::CFileExtension (CFileExtension&& ext) noexcept
{
	*this = std::move (ext);
}

//-----------------------------------------------------------------------------
CFileExtension& CFileExtension::operator=(CFileExtension&& ext) noexcept
{
	description = std::move (ext.description);
	extension = std::move (ext.extension);
	mimeType = std::move (ext.mimeType);
	uti = std::move (ext.uti);
	macType = ext.macType;
	ext.macType = 0;
	return *this;
}

//-----------------------------------------------------------------------------
void CFileExtension::init (const UTF8String& inDescription, const UTF8String& inExtension, const UTF8String& inMimeType, const UTF8String& inUti)
{
	description = inDescription;
	extension = inExtension;
	mimeType = inMimeType;
	uti = inUti;

	if (description == nullptr && !extension.empty ())
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
	result = extension == ext.extension;
	if (!result)
		result = mimeType == ext.mimeType;
	if (!result)
		result = uti == ext.uti;
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
, defaultExtension (nullptr)
, title (nullptr)
, initialPath (nullptr)
, defaultSaveName (nullptr)
, allowMultiFileSelection (false)
{
}

//-----------------------------------------------------------------------------
CNewFileSelector::~CNewFileSelector ()
{
	setTitle (nullptr);
	setInitialDirectory (nullptr);
	setDefaultSaveName (nullptr);
}

//-----------------------------------------------------------------------------
bool CNewFileSelector::run (CBaseObject* delegate)
{
	if (delegate == nullptr)
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

//-----------------------------------------------------------------------------
class CNewFileSelectorCallback : public CBaseObject
{
public:
	CNewFileSelectorCallback (CNewFileSelector::CallbackFunc&& callback) : callbackFunc (std::move (callback)) {}
	~CNewFileSelectorCallback () override = default;
private:
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override
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
	auto fsCallback = makeOwned<CNewFileSelectorCallback> (std::move (callback));
	return runInternal (fsCallback);
}

//-----------------------------------------------------------------------------
void CNewFileSelector::setTitle (const UTF8String& inTitle)
{
	title = inTitle;
}

//-----------------------------------------------------------------------------
void CNewFileSelector::setInitialDirectory (const UTF8String& path)
{
	initialPath = path;
}

//-----------------------------------------------------------------------------
void CNewFileSelector::setDefaultSaveName (const UTF8String& name)
{
	defaultSaveName = name;
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
		++it;
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

//-----------------------------------------------------------------------------
void CNewFileSelector::addFileExtension (CFileExtension&& extension)
{
	extensions.push_back (std::move (extension));
}

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
	return nullptr;
}

} // namespace

