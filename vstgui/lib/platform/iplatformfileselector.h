// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../cstring.h"
#include <functional>
#include <vector>

//-----------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
struct PlatformFileExtension
{
	UTF8String description;
	UTF8String extension;
	UTF8String mimeType;
	UTF8String uti;
	int32_t macType {0};
};

static PlatformFileExtension PlatformAllFilesExtension = {"All Files", "", "", "", 0};
static PlatformFileExtension PlatformNoFileExtension = {};

//-----------------------------------------------------------------------------
enum class PlatformFileSelectorStyle : uint32_t
{
	SelectFile,
	SelectDirectory,
	SelectSaveFile,
};

//-----------------------------------------------------------------------------
enum class PlatformFileSelectorFlags : uint32_t
{
	MultiFileSelection = 1 << 0,
	RunModal = 1 << 1,
};

//-----------------------------------------------------------------------------
struct PlatformFileSelectorConfig
{
	using CallbackFunc = std::function<void (std::vector<UTF8String>&&)>;
	using FileExtensionList = std::vector<PlatformFileExtension>;

	UTF8String title;
	UTF8String initialPath;
	UTF8String defaultSaveName;
	FileExtensionList extensions;
	PlatformFileExtension defaultExtension;
	uint32_t flags {0};

	CallbackFunc doneCallback;
};

//-----------------------------------------------------------------------------
class IPlatformFileSelector
{
public:
	virtual bool run (const PlatformFileSelectorConfig& config) = 0;
	virtual bool cancel () = 0;

	virtual ~IPlatformFileSelector () noexcept = default;
};

//-----------------------------------------------------------------------------
inline bool operator== (const PlatformFileExtension& e1, const PlatformFileExtension& e2)
{
	return e1.macType == e2.macType && e1.uti == e2.uti && e1.mimeType == e2.mimeType &&
		   e1.extension == e2.extension && e1.description == e2.description;
}

//-----------------------------------------------------------------------------
inline bool operator!= (const PlatformFileExtension& e1, const PlatformFileExtension& e2)
{
	return e1.macType != e2.macType || e1.uti != e2.uti || e1.mimeType != e2.mimeType ||
		   e1.extension != e2.extension || e1.description != e2.description;
}

//-----------------------------------------------------------------------------
} // VSTGUI
