// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "document.h"
#include "vstgui/lib/cpoint.h"
#include "vstgui/uidescription/cstream.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ImageStitcher {

//------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------
Optional<Path> getRelativePath (const Path& root, const Path& path)
{
	if (path.find (root) == 0)
	{
		return {path.substr (root.size ())};
	}
	return {};
}

//------------------------------------------------------------------------
Optional<Path> getDirectoryName (const Path& path)
{
	auto pos = path.find_last_of (PathSeparator);
	if (pos == Path::npos || pos == path.size ())
		return {};
	return {path.substr (0, pos + strlen (PathSeparator))};
}

static constexpr uint32_t PersistentIdentifer = 'imst';
static constexpr uint32_t PersistentIdentiferNew = 'ist2';
static constexpr uint32_t PersistentVersion = 1;

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
Optional<std::pair<uint32_t, uint32_t>> getImageSize (const Path& path)
{
	CFileStream stream;
	if (!stream.open (path.data (), CFileStream::kReadMode | CFileStream::kBinaryMode,
	                  kBigEndianByteOrder))
		return {};
	uint32_t value;
	if (!(stream >> value) || value != 0x89504E47)
		return {};
	if (!(stream >> value) || value != 0x0D0A1A0A)
		return {};
	if (!(stream >> value) || value != 0x0000000D)
		return {};
	if (!(stream >> value) || value != 0x49484452)
		return {};
	uint32_t width = 0;
	if (!(stream >> width))
		return {};
	uint32_t height = 0;
	if (!(stream >> height))
		return {};

	return {{width, height}};
}

//------------------------------------------------------------------------
DocumentContextPtr DocumentContext::makeEmptyDocument ()
{
	auto doc = std::make_shared<Document> ();
	doc->path = "Untitled.imagestitch";
	auto docContext = std::make_shared<DocumentContext> (doc);
	return docContext;
}

//------------------------------------------------------------------------
DocumentContextPtr DocumentContext::loadDocument (const Path& path)
{
	auto rootPath = getDirectoryName (path);
	if (!rootPath)
		return nullptr;

	CFileStream stream;
	if (!stream.open (path.data (), CFileStream::kReadMode | CFileStream::kBinaryMode,
	                  kLittleEndianByteOrder))
		return nullptr;

	uint32_t identifier;
	if (!(stream >> identifier))
		return nullptr;
	if (!(identifier == PersistentIdentifer || identifier == PersistentIdentiferNew))
		return nullptr;

	auto doc = std::make_shared<Document> ();
	doc->path = path;

	if (identifier == PersistentIdentiferNew)
	{
		uint32_t persistentVersion = 0;
		if (!(stream >> persistentVersion))
			return nullptr;
		if (!(stream >> doc->numFramesPerRow))
			return nullptr;
	}
	if (!(stream >> doc->width))
		return nullptr;
	if (!(stream >> doc->height))
		return nullptr;
	uint32_t numPaths;
	if (!(stream >> numPaths))
		return nullptr;
	for (uint32_t i = 0; i < numPaths; ++i)
	{
		Path p;
		if (!(stream >> p))
			return nullptr;
		Path fullPath;
		if (!pathIsAbsolute (p))
			fullPath += *rootPath;
		fullPath += p;
		doc->imagePaths.emplace_back (std::move (fullPath));
	}

	auto docContext = std::make_shared<DocumentContext> (doc);
	return docContext;
}

//------------------------------------------------------------------------
bool DocumentContext::save ()
{
	auto rootPath = getDirectoryName (doc->path);
	if (!rootPath)
		return false;

	CFileStream stream;
	if (!stream.open (doc->path.data (),
	                  CFileStream::kWriteMode | CFileStream::kBinaryMode |
	                      CFileStream::kTruncateMode,
	                  kLittleEndianByteOrder))
		return false;

	if (!(stream << PersistentIdentiferNew))
		return false;
	if (!(stream << PersistentVersion))
		return false;
	if (!(stream << doc->numFramesPerRow))
		return false;
	if (!(stream << doc->width))
		return false;
	if (!(stream << doc->height))
		return false;
	uint32_t numImages = static_cast<uint32_t> (doc->imagePaths.size ());
	if (!(stream << numImages))
		return false;
	for (auto& path : doc->imagePaths)
	{
		if (auto relPath = getRelativePath (*rootPath, path))
			stream << *relPath;
		else
			stream << path;
	}

	return true;
}

//------------------------------------------------------------------------
DocumentContext::DocumentContext (const DocumentPtr& doc) : doc (doc)
{
}

//------------------------------------------------------------------------
void DocumentContext::replaceDocument (const DocumentPtr& newDoc)
{
	doc = newDoc;
	size_t index = 0;
	for (auto& path : doc->imagePaths)
	{
		listeners.forEach ([&] (auto& l) { l->onImagePathAdded (path, index); });
		++index;
	}
}

//------------------------------------------------------------------------
bool DocumentContext::setPath (const std::string& p)
{
	doc->path = p;
	return true;
}

//------------------------------------------------------------------------
bool DocumentContext::setNumFramesPerRow (uint16_t numFrames)
{
	doc->numFramesPerRow = numFrames;
	listeners.forEach ([&] (auto& l) { l->onNumFramesPerRowChanged (numFrames); });
	return true;
}

//------------------------------------------------------------------------
auto DocumentContext::removeImagePathAtIndex (size_t index) -> Result
{
	if (doc->imagePaths.size () < index)
		return Result::InvalidIndex;
	auto it = doc->imagePaths.begin ();
	std::advance (it, index);
	auto path = *it;
	doc->imagePaths.erase (it);
	if (doc->imagePaths.empty ())
		doc->width = doc->height = 0;
	listeners.forEach ([&] (auto& l) { l->onImagePathRemoved (path, index); });
	return Result::Success;
}

//------------------------------------------------------------------------
auto DocumentContext::insertImagePathAtIndex (size_t index, const Path& path) -> Result
{
	auto imageSize = getImageSize (path);
	if (!imageSize)
		return Result::InvalidImage;
	if (!doc->imagePaths.empty ())
	{
		if (!(imageSize->first == doc->width && imageSize->second == doc->height))
			return Result::ImageSizeMismatch;
	}
	auto it = doc->imagePaths.begin ();
	if (index >= doc->imagePaths.size ())
		it = doc->imagePaths.end ();
	else
		std::advance (it, index);
	doc->imagePaths.insert (it, path);
	if (doc->imagePaths.size () == 1)
	{
		doc->width = imageSize->first;
		doc->height = imageSize->second;
	}
	listeners.forEach ([&] (auto& l) { l->onImagePathAdded (path, index); });
	return Result::Success;
}

//------------------------------------------------------------------------
void DocumentContext::addListener (IDocumentListener* listener)
{
	listeners.add (listener);
}

//------------------------------------------------------------------------
void DocumentContext::removeListener (IDocumentListener* listener)
{
	listeners.remove (listener);
}

//------------------------------------------------------------------------
} // ImageStitcher
} // VSTGUI
