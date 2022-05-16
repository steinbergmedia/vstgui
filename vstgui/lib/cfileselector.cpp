// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "platform/iplatformfileselector.h"
#include "platform/platformfactory.h"
#include "cfileselector.h"
#include "cframe.h"
#include "cstring.h"
#include <algorithm>

namespace VSTGUI {

//-----------------------------------------------------------------------------
struct CFileExtension::Impl : PlatformFileExtension
{
};

//-----------------------------------------------------------------------------
CFileExtension::CFileExtension ()
{
	impl = std::make_unique<Impl> ();
}

//-----------------------------------------------------------------------------
CFileExtension::CFileExtension (const UTF8String& inDescription, const UTF8String& inExtension,
								const UTF8String& inMimeType, int32_t inMacType,
								const UTF8String& inUti)
: CFileExtension ()
{
	init (inDescription, inExtension, inMimeType, inUti);
}

//-----------------------------------------------------------------------------
CFileExtension::CFileExtension (const CFileExtension& ext) : CFileExtension ()
{
	*impl = *ext.impl;
}

//-----------------------------------------------------------------------------
CFileExtension::~CFileExtension () noexcept = default;

//-----------------------------------------------------------------------------
CFileExtension::CFileExtension (CFileExtension&& ext) noexcept
{
	*this = std::move (ext);
}

//-----------------------------------------------------------------------------
CFileExtension& CFileExtension::operator= (CFileExtension&& ext) noexcept
{
	std::swap (impl, ext.impl);
	return *this;
}

//-----------------------------------------------------------------------------
void CFileExtension::init (const UTF8String& inDescription, const UTF8String& inExtension,
						   const UTF8String& inMimeType, const UTF8String& inUti)
{
	impl->description = inDescription;
	impl->extension = inExtension;
	impl->mimeType = inMimeType;
	impl->uti = inUti;

	if (impl->description == nullptr && !impl->extension.empty ())
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
	result = impl->extension == ext.impl->extension;
	if (!result)
		result = impl->mimeType == ext.impl->mimeType;
	if (!result)
		result = impl->uti == ext.impl->uti;
	if (!result && impl->macType != 0 && ext.impl->macType != 0)
		result = (impl->macType == ext.impl->macType);
	return result;
}

//-----------------------------------------------------------------------------
const UTF8String& CFileExtension::getDescription () const
{
	return impl->description;
}

//-----------------------------------------------------------------------------
const UTF8String& CFileExtension::getExtension () const
{
	return impl->extension;
}

//-----------------------------------------------------------------------------
const UTF8String& CFileExtension::getMimeType () const
{
	return impl->mimeType;
}

//-----------------------------------------------------------------------------
const UTF8String& CFileExtension::getUTI () const
{
	return impl->uti;
}

//-----------------------------------------------------------------------------
int32_t CFileExtension::getMacType () const
{
	return impl->macType;
}

//-----------------------------------------------------------------------------
CFileExtension::CFileExtension (const PlatformFileExtension& ext) : CFileExtension ()
{
	impl->description = ext.description;
	impl->extension = ext.extension;
	impl->mimeType = ext.mimeType;
	impl->uti = ext.uti;
	impl->macType = ext.macType;
}

//-----------------------------------------------------------------------------
const PlatformFileExtension& CFileExtension::getPlatformFileExtension () const
{
	return *impl;
}

//-----------------------------------------------------------------------------
const CFileExtension& CNewFileSelector::getAllFilesExtension ()
{
	static CFileExtension allFilesExtension (PlatformAllFilesExtension);
	return allFilesExtension;
}

//-----------------------------------------------------------------------------
IdStringPtr CNewFileSelector::kSelectEndMessage = "CNewFileSelector Select End Message";

//-----------------------------------------------------------------------------
// CNewFileSelector Implementation
//-----------------------------------------------------------------------------
struct CNewFileSelector::Impl : PlatformFileSelectorConfig
{
	PlatformFileSelectorPtr platformFileSelector;
	CFrame* frame {nullptr};
	std::vector<UTF8String> result;
};

//-----------------------------------------------------------------------------
CNewFileSelector::CNewFileSelector (PlatformFileSelectorPtr&& platformFileSelector, CFrame* parent)
{
	impl = std::make_unique<Impl> ();
	impl->platformFileSelector = std::move (platformFileSelector);
	impl->frame = parent;
}

//-----------------------------------------------------------------------------
CNewFileSelector::~CNewFileSelector () noexcept = default;

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
	if (impl->frame)
		impl->frame->onStartLocalEventLoop ();

	impl->doneCallback = [this, del = shared (delegate)] (std::vector<UTF8String>&& files) {
		impl->result = std::move (files);
		del->notify (this, CNewFileSelector::kSelectEndMessage);
	};

	setBit (impl->flags, PlatformFileSelectorFlags::RunModal, false);
	return impl->platformFileSelector->run (*impl);
}

//-----------------------------------------------------------------------------
bool CNewFileSelector::run (CallbackFunc&& callback)
{
	if (impl->frame)
		impl->frame->onStartLocalEventLoop ();

	impl->doneCallback = [Self = shared (this),
						  cb = std::move (callback)] (std::vector<UTF8String>&& files) {
		Self->impl->result = std::move (files);
		cb (Self);
	};

	setBit (impl->flags, PlatformFileSelectorFlags::RunModal, false);
	return impl->platformFileSelector->run (*impl);
}

//-----------------------------------------------------------------------------
void CNewFileSelector::cancel ()
{
	impl->platformFileSelector->cancel ();
}

//-----------------------------------------------------------------------------
bool CNewFileSelector::runModal ()
{
	if (impl->frame)
		impl->frame->onStartLocalEventLoop ();
	setBit (impl->flags, PlatformFileSelectorFlags::RunModal, true);
	impl->doneCallback = [&] (std::vector<UTF8String>&& files) {
		impl->result = std::move (files);
	};

	return impl->platformFileSelector->run (*impl);
}

//-----------------------------------------------------------------------------
void CNewFileSelector::setTitle (const UTF8String& inTitle)
{
	impl->title = inTitle;
}

//-----------------------------------------------------------------------------
void CNewFileSelector::setInitialDirectory (const UTF8String& path)
{
	impl->initialPath = path;
}

//-----------------------------------------------------------------------------
void CNewFileSelector::setDefaultSaveName (const UTF8String& name)
{
	impl->defaultSaveName = name;
}

//-----------------------------------------------------------------------------
void CNewFileSelector::setAllowMultiFileSelection (bool state)
{
	setBit (impl->flags, PlatformFileSelectorFlags::MultiFileSelection, state);
}

//-----------------------------------------------------------------------------
void CNewFileSelector::setDefaultExtension (const CFileExtension& extension)
{
	if (impl->defaultExtension != PlatformNoFileExtension)
	{
#if DEBUG
		DebugPrint ("VSTGUI Warning: It's not allowed to set a default extension twice on a "
					"CFileSelector instance\n");
#endif
		return;
	}

	auto it = std::find (impl->extensions.begin (), impl->extensions.end (),
						 extension.getPlatformFileExtension ());
	if (it == impl->extensions.end ())
		addFileExtension (extension);
	impl->defaultExtension = extension.getPlatformFileExtension ();
}

//-----------------------------------------------------------------------------
void CNewFileSelector::addFileExtension (const CFileExtension& extension)
{
	impl->extensions.emplace_back (extension.getPlatformFileExtension ());
}

//-----------------------------------------------------------------------------
void CNewFileSelector::addFileExtension (CFileExtension&& extension)
{
	impl->extensions.emplace_back (std::move (extension.getPlatformFileExtension ()));
}

//-----------------------------------------------------------------------------
uint32_t CNewFileSelector::getNumSelectedFiles () const
{
	return static_cast<uint32_t> (impl->result.size ());
}

//-----------------------------------------------------------------------------
UTF8StringPtr CNewFileSelector::getSelectedFile (uint32_t index) const
{
	if (index < impl->result.size ())
		return impl->result[index];
	return nullptr;
}

//------------------------------------------------------------------------
CNewFileSelector* CNewFileSelector::create (CFrame* parent, Style style)
{
	PlatformFileSelectorStyle platformStyle;
	switch (style)
	{
		case Style::kSelectFile:
			platformStyle = PlatformFileSelectorStyle::SelectFile;
			break;
		case Style::kSelectDirectory:
			platformStyle = PlatformFileSelectorStyle::SelectDirectory;
			break;
		case Style::kSelectSaveFile:
			platformStyle = PlatformFileSelectorStyle::SelectSaveFile;
			break;
		default:
			vstgui_assert (false);
			return nullptr;
	}
	if (auto platformSelector = getPlatformFactory ().createFileSelector (
			platformStyle, parent ? parent->getPlatformFrame () : nullptr))
	{
		return new CNewFileSelector (std::move (platformSelector), parent);
	}
	return nullptr;
}

} // VSTGUI
