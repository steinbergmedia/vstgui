// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "shareduiresources.h"
#include "../../lib/cbitmap.h"
#include "../../lib/ccolor.h"
#include "../../lib/cfileselector.h"
#include "../../lib/cframe.h"
#include "../../uidescription/compresseduidescription.h"
#include "../../uidescription/cstream.h"
#include "../../uidescription/uiattributes.h"
#include "../include/ialertbox.h"
#include "../include/iappdelegate.h"
#include "../include/iapplication.h"
#include "../include/helpers/preferences.h"
#include "application.h"
#include <unordered_map>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Detail {

#if VSTGUI_LIVE_EDITING

//------------------------------------------------------------------------
struct EditFileMap : IEditFileMap
{
	using Map = std::unordered_map<std::string, std::string>;
	Map fileMap;

	void set (const std::string& filename, const std::string& absolutePath) override
	{
		fileMap.emplace (filename, absolutePath);
	}

	Optional<const char*> get (const std::string& filename) const override
	{
		auto it = fileMap.find (filename);
		if (it == fileMap.end ())
		{
			return {};
		}
		return makeOptional (it->second.data ());
	}
};

//------------------------------------------------------------------------
IEditFileMap& getEditFileMap ()
{
	static EditFileMap gInstance;
	return gInstance;
}

#endif

//------------------------------------------------------------------------
class SharedUIResources : public ISharedUIResources
{
public:
	static SharedUIResources& instance () noexcept;

	SharedUIResources () noexcept;

	void cleanup ();

	Optional<CColor> getColor (const UTF8String& name) const override;
	Optional<CBitmap*> getBitmap (const UTF8String& name) const override;
	Optional<CGradient*> getGradient (const UTF8String& name) const override;
	Optional<CFontDesc*> getFont (const UTF8String& name) const override;

	SharedPointer<UIDescription> get () const
	{
		load ();
		return uiDesc;
	}

private:
	bool load () const;

	mutable bool loadDone {false};
	mutable SharedPointer<UIDescription> uiDesc;
};

//------------------------------------------------------------------------
SharedUIResources& SharedUIResources::instance () noexcept
{
	static SharedUIResources gInstance;
	return gInstance;
}

//------------------------------------------------------------------------
SharedUIResources::SharedUIResources () noexcept
{
}

//------------------------------------------------------------------------
void SharedUIResources::cleanup ()
{
	uiDesc = nullptr;
}

//------------------------------------------------------------------------
bool SharedUIResources::load () const
{
	if (loadDone)
		return uiDesc != nullptr;
	loadDone = true;
	if (auto filename = IApplication::instance ().getDelegate ().getSharedUIResourceFilename ())
	{

#if VSTGUI_LIVE_EDITING
		if (auto absPath = Detail::getEditFileMap ().get (filename))
			filename = *absPath;
#endif

		SharedPointer<UIDescription> description;
		if (Detail::getApplicationPlatformAccess ()
		        ->getConfiguration ()
		        .useCompressedUIDescriptionFiles)
			description = makeOwned<CompressedUIDescription> (filename);
		else
			description = makeOwned<UIDescription> (filename);
		if (!description->parse ())
		{
#if VSTGUI_LIVE_EDITING
			if (!initUIDescAsNew (*description, nullptr))
				return false;
			else
#endif
				return false;
		}
		auto settings = description->getCustomAttributes ("UIDescFilePath", true);
		auto filePath = settings->getAttributeValue ("path");
		if (filePath)
			description->setFilePath (filePath->data ());

		uiDesc = std::move (description);
#if VSTGUI_LIVE_EDITING
		auto res = Detail::checkAndUpdateUIDescFilePath (
		    *uiDesc, nullptr, "The resource ui desc file location cannot be found.");
		if (res == UIDescCheckFilePathResult::Cancel)
		{
			IApplication::instance ().quit ();
			return false;
		}
		Detail::getEditFileMap ().set (
		    IApplication::instance ().getDelegate ().getSharedUIResourceFilename (),
		    uiDesc->getFilePath ());
		if (res == UIDescCheckFilePathResult::NewPathSet)
			saveSharedUIDescription ();
#endif
	}
	return uiDesc != nullptr;
}

//------------------------------------------------------------------------
Optional<CColor> SharedUIResources::getColor (const UTF8String& name) const
{
	if (load ())
	{
		CColor c;
		if (uiDesc->getColor (name, c))
			return makeOptional (c);
	}
	return {};
}

//------------------------------------------------------------------------
Optional<CBitmap*> SharedUIResources::getBitmap (const UTF8String& name) const
{
	if (load ())
	{
		if (auto bitmap = uiDesc->getBitmap (name))
		{
			return makeOptional (bitmap);
		}
	}
	return {};
}

//------------------------------------------------------------------------
Optional<CGradient*> SharedUIResources::getGradient (const UTF8String& name) const
{
	if (load ())
	{
		if (auto gradient = uiDesc->getGradient (name))
		{
			return makeOptional (gradient);
		}
	}
	return {};
}

//------------------------------------------------------------------------
Optional<CFontDesc*> SharedUIResources::getFont (const UTF8String& name) const
{
	if (load ())
	{
		if (auto font = uiDesc->getFont (name))
		{
			return makeOptional (font);
		}
	}
	return {};
}

//------------------------------------------------------------------------
const ISharedUIResources& getSharedUIResources ()
{
	return SharedUIResources::instance ();
}

//------------------------------------------------------------------------
SharedPointer<UIDescription> getSharedUIDescription ()
{
	return SharedUIResources::instance ().get ();
}

//------------------------------------------------------------------------
void cleanupSharedUIResources ()
{
	SharedUIResources::instance ().cleanup ();
}

#if VSTGUI_LIVE_EDITING
//------------------------------------------------------------------------
static constexpr auto UIDescPathKey = "VSTGUI::Standalone|Debug|UIDescPath|" __FILE__;

//------------------------------------------------------------------------
static void updateUIDescFilePath (const char* path, UIDescription& uiDesc)
{
	uiDesc.setFilePath (path);
	auto settings = uiDesc.getCustomAttributes ("UIDescFilePath", true);
	settings->setAttribute ("path", uiDesc.getFilePath ());
}

//------------------------------------------------------------------------
UIDescCheckFilePathResult checkAndUpdateUIDescFilePath (UIDescription& uiDesc, CFrame* _frame,
                                                        UTF8StringPtr notFoundText)
{
	auto originalPath = std::string (uiDesc.getFilePath ());
	CFileStream stream;
	if (stream.open (originalPath.data (), CFileStream::kReadMode))
		return UIDescCheckFilePathResult::Exists;

	VSTGUI::Standalone::Preferences prefs;
	auto savedPath = prefs.get (UIDescPathKey);
	if (savedPath)
	{
		unixfyPath (originalPath);
		if (auto uiDescName = lastPathComponent (originalPath))
		{
			auto directory = savedPath->getString ();
			unixfyPath (directory);
			removeLastPathComponent (directory);
			directory += unixPathSeparator;
			directory += *uiDescName;
			if (stream.open (directory.data (), CFileStream::kReadMode))
			{
				updateUIDescFilePath (directory.data (), uiDesc);
				return UIDescCheckFilePathResult::NewPathSet;
			}
		}
	}
	SharedPointer<CFrame> frame (_frame);
	if (!frame)
		frame = makeOwned<CFrame> (CRect (), nullptr);

	AlertBoxConfig alertConfig;
	alertConfig.headline = notFoundText;
	alertConfig.description = uiDesc.getFilePath ();
	alertConfig.defaultButton = "Locate";
	alertConfig.secondButton = "Close";
	auto alertResult = IApplication::instance ().showAlertBox (alertConfig);
	if (alertResult == AlertResult::SecondButton)
	{
		return UIDescCheckFilePathResult::Cancel;
	}
	auto fs = owned (CNewFileSelector::create (frame, CNewFileSelector::kSelectFile));
	if (savedPath)
		fs->setInitialDirectory (*savedPath);
	fs->setDefaultExtension (CFileExtension ("UIDescription File", "uidesc"));
	if (fs->runModal ())
	{
		if (fs->getNumSelectedFiles () == 0)
		{
			return UIDescCheckFilePathResult::Cancel;
		}
		auto path = fs->getSelectedFile (0);
		updateUIDescFilePath (path, uiDesc);
		prefs.set (UIDescPathKey, path);
		return UIDescCheckFilePathResult::NewPathSet;
	}
	return UIDescCheckFilePathResult::Cancel;
}

//------------------------------------------------------------------------
bool initUIDescAsNew (UIDescription& uiDesc, CFrame* _frame)
{
	SharedPointer<CFrame> frame (_frame);
	if (!frame)
		frame = makeOwned<CFrame> (CRect (), nullptr);
	auto fs = owned (CNewFileSelector::create (frame, CNewFileSelector::kSelectSaveFile));
	vstgui_assert (fs, "create new FileSelector failed");
	VSTGUI::Standalone::Preferences prefs;
	if (auto initPath = prefs.get (UIDescPathKey))
		fs->setInitialDirectory (*initPath);
	fs->setDefaultSaveName (uiDesc.getFilePath ());
	fs->setDefaultExtension (CFileExtension ("UIDescription File", "uidesc"));
	fs->setTitle ("Save UIDescription File");
	if (fs->runModal ())
	{
		if (fs->getNumSelectedFiles () == 0)
		{
			return false;
		}
		auto path = fs->getSelectedFile (0);
		uiDesc.setFilePath (path);
		auto settings = uiDesc.getCustomAttributes ("UIDescFilePath", true);
		settings->setAttribute ("path", path);
		prefs.set (UIDescPathKey, path);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
void saveSharedUIDescription ()
{
	if (auto uiDesc = getSharedUIDescription ())
	{
		int32_t flags = UIDescription::kWriteImagesIntoUIDescFile |
		                CompressedUIDescription::kForceWriteCompressedDesc;
		if (uiDesc->save (uiDesc->getFilePath (), flags))
			return;
		AlertBoxConfig config;
		config.headline = "Saving the shared resources uidesc file failed.";
		IApplication::instance ().showAlertBox (config);
	}
}

#endif // VSTGUI_LIVE_EDITING

//------------------------------------------------------------------------
} // Detail
} // Standalone
} // VSTGUI
