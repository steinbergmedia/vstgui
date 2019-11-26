// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "document.h"
#include "vstgui/lib/cbitmap.h"
#include "vstgui/lib/cfileselector.h"
#include "vstgui/lib/cvstguitimer.h"
#include "vstgui/standalone/include/helpers/windowcontroller.h"
#include "vstgui/standalone/include/icommand.h"
#include "vstgui/standalone/include/iuidescwindow.h"
#include "vstgui/standalone/include/ivalue.h"
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ImageStitcher {

class ImageFramesView;

//------------------------------------------------------------------------
static constexpr IdStringPtr ExportStr = "Export...";
static const Standalone::Command ExportCommand {Standalone::CommandGroup::File, ExportStr};
static CFileExtension imageStitchExtension ("Image Stitch File", "imagestitch", "", 0, "");

//------------------------------------------------------------------------
struct Image
{
	SharedPointer<CBitmap> bitmap;
	Path path;
	bool selected {false};

#if defined(_MSC_VER) && _MSC_VER < 1910 // Can be removed when dropping VS 2015 Support
	Image (SharedPointer<CBitmap> bitmap, Path path, bool selected)
	: bitmap (bitmap), path (path), selected (selected)
	{
	}
#endif
};
using ImageList = std::vector<Image>;

//------------------------------------------------------------------------
class DocumentWindowController : public Standalone::WindowControllerAdapter,
                                 public Standalone::UIDesc::ICustomization,
                                 public Standalone::ICommandHandler,
                                 public IDocumentListener
{
public:
	static std::shared_ptr<DocumentWindowController> make (const DocumentContextPtr& doc);

	DocumentWindowController (const DocumentContextPtr& doc);
	~DocumentWindowController () noexcept;

	const DocumentContextPtr& getDoc () const noexcept { return docContext; }
	SharedPointer<CBitmap> createStitchedBitmap ();

	void showWindow ();
	void closeWindow ();
	void registerWindowListener (Standalone::IWindowListener* listener);
	
	void doSaveAs (std::function<void(bool saved)>&& customAction = [] (bool) {});
	void doOpenDocument (std::function<void(bool saved)>&& customAction = [] (bool) {});

private:
	void onImagePathAdded (const Path& newPath, size_t index) override;
	void onImagePathRemoved (const Path& newPath, size_t index) override;

	IController* createController (const UTF8StringView& name, IController* parent,
	                               const IUIDescription* uiDesc) override;
	void onUIDescriptionParsed (const IUIDescription* uiDesc) override;
	void onSetContentView (Standalone::IWindow& w, const SharedPointer<CFrame>& cv) override;
	void onClosed (const Standalone::IWindow& window) override;
	bool canClose (const Standalone::IWindow& window) override;
	Standalone::UIDesc::ModelBindingPtr createModelBinding ();

	bool canHandleCommand (const Standalone::Command& command) override;
	bool handleCommand (const Standalone::Command& command) override;

	void doAddPathCommand ();
	void doRemovePathCommand ();
	void doSelectAllCommand ();
	void doDeselectAllCommand ();
	void doStartAnimation ();
	void doStopAnimation ();
	void doExport ();
	void doSave ();

	bool somethingSelected () const;
	size_t lastSelectedPos () const;

	void setDirty ();

	DocumentContextPtr docContext;
	CFrame* contentView {nullptr};
	ImageFramesView* imageView {nullptr};
	CMovieBitmap* movieBitmapView {nullptr};
	Standalone::WindowPtr window;
	Standalone::ValuePtr displayFrameValue;
	Standalone::ValuePtr animationTimeValue;
	SharedPointer<CVSTGUITimer> timer;
	ImageList imageList;
	bool asyncUpdateTriggered {false};
	bool docIsDirty {false};
};

//------------------------------------------------------------------------
inline std::string getDisplayFilename (const Path& path)
{
	if (path.empty ())
		return "Untitled";
	auto pos = path.find_last_of (PathSeparator);
	if (pos == Path::npos)
		return path;
	return path.substr (pos + 1);
}

//------------------------------------------------------------------------
} // ImageStitcher
} // VSTGUI
