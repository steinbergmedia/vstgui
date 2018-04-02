// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstgui/lib/dispatchlist.h"
#include "vstgui/lib/optional.h"
#include "vstgui/lib/vstguibase.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ImageStitcher {

using Path = std::string;
using PathList = std::vector<Path>;

#if WINDOWS
static constexpr const auto PathSeparator = "\\";
#else
static constexpr const auto PathSeparator = "/";
#endif

//------------------------------------------------------------------------
Optional<std::pair<uint32_t, uint32_t>> getImageSize (const Path& path);

//------------------------------------------------------------------------
struct Document
{
	Path path;
	PathList imagePaths;
	uint32_t width {0};
	uint32_t height {0};
};
using DocumentPtr = std::shared_ptr<Document>;

//------------------------------------------------------------------------
struct IDocumentListener
{
	virtual void onImagePathAdded (const Path& newPath, size_t index) = 0;
	virtual void onImagePathRemoved (const Path& newPath, size_t index) = 0;
};

//------------------------------------------------------------------------
enum class DocumentContextResult
{
	Success,
	InvalidIndex,
	InvalidImage,
	ImageSizeMismatch,
};

struct DocumentContext;
using DocumentContextPtr = std::shared_ptr<DocumentContext>;

//------------------------------------------------------------------------
struct DocumentContext
{
	using Result = DocumentContextResult;

	static DocumentContextPtr makeEmptyDocument ();
	static DocumentContextPtr loadDocument (const Path& path);

	DocumentContext (const DocumentPtr& doc);

	void replaceDocument (const DocumentPtr& doc);

	bool save ();

	bool setPath (const Path& p);
	Result removeImagePathAtIndex (size_t index);
	Result insertImagePathAtIndex (size_t index, const Path& path);

	const DocumentPtr& getDocument () const { return doc; }
	const Path& getPath () const noexcept { return doc->path; }
	const PathList& getImagePaths () const noexcept { return doc->imagePaths; }
	uint32_t getWidth () const noexcept { return doc->width; }
	uint32_t getHeight () const noexcept { return doc->height; }

	void addListener (IDocumentListener* listener);
	void removeListener (IDocumentListener* listener);

private:
	DocumentPtr doc;
	DispatchList<IDocumentListener*> listeners;
};
using DocumentContextPtr = std::shared_ptr<DocumentContext>;

//------------------------------------------------------------------------
} // ImageStitcher
} // VSTGUI
