// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "documentcontroller.h"
#include "imageframesview.h"
#include "vstgui/lib/cbitmap.h"
#include "vstgui/lib/cdatabrowser.h"
#include "vstgui/lib/cgradientview.h"
#include "vstgui/lib/coffscreencontext.h"
#include "vstgui/lib/controls/cmoviebitmap.h"
#include "vstgui/lib/cscrollview.h"
#include "vstgui/lib/csplitview.h"
#include "vstgui/lib/platform/iplatformbitmap.h"
#include "vstgui/lib/platform/platformfactory.h"
#include "vstgui/standalone/include/helpers/uidesc/modelbinding.h"
#include "vstgui/standalone/include/helpers/value.h"
#include "vstgui/standalone/include/ialertbox.h"
#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/iasync.h"
#include "vstgui/uidescription/cstream.h"
#include "vstgui/uidescription/delegationcontroller.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/uiattributes.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ImageStitcher {

using namespace VSTGUI::Standalone;

static CFileExtension pngFileExtension ("PNG File", "png", "image/png", 0, "public.png");

//------------------------------------------------------------------------
class ImageViewController : public DelegationController
{
public:
	using Proc = std::function<void (ImageFramesView*)>;
	ImageViewController (Proc&& proc, IController* parent)
	: DelegationController (parent), proc (std::move (proc))
	{
	}

	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override
	{
		if (auto name = attributes.getAttributeValue (IUIDescription::kCustomViewName))
		{
			if (*name == "ImageView")
			{
				auto imageView = new ImageFramesView ();
				CColor color;
				if (description->getColor ("Focus", color))
					imageView->setSelectionColor (color);
				if (description->getColor ("font.color", color))
					imageView->setTextColor (color);
				return imageView;
			}
		}
		return controller->createView (attributes, description);
	}

	CView* verifyView (CView* view, const UIAttributes& attr, const IUIDescription* desc) override
	{
		if (auto name = attr.getAttributeValue (IUIDescription::kCustomViewName))
		{
			if (*name == "ImageView")
			{
				proc (dynamic_cast<ImageFramesView*> (view));
				return view;
			}
		}
		return controller->verifyView (view, attr, desc);
	}

private:
	Proc proc;
};

//------------------------------------------------------------------------
class MovieBitmapController : public DelegationController
{
public:
	using Proc = std::function<void (CMovieBitmap*)>;
	MovieBitmapController (Proc&& proc, IController* parent)
	: DelegationController (parent), proc (std::move (proc))
	{
	}

	CView* verifyView (CView* view, const UIAttributes& attr, const IUIDescription* desc) override
	{
		if (auto mb = dynamic_cast<CMovieBitmap*> (view))
		{
			proc (mb);
			return view;
		}
		return controller->verifyView (view, attr, desc);
	}

private:
	Proc proc;
};

//------------------------------------------------------------------------
class SplitViewController : public DelegationController, public ISplitViewController
{
public:
	SplitViewController (IController* parent, const IUIDescription* desc)
	: DelegationController (parent), desc (desc)
	{
	}

	bool getSplitViewSizeConstraint (int32_t index, CCoord& minSize, CCoord& maxSize,
	                                 CSplitView* splitView) override
	{
		if (index == 0)
		{
			minSize = 150;
			maxSize = -1;
			return true;
		}
		if (index == 1)
		{
			minSize = 250;
			maxSize = -1;
			return true;
		}
		return false;
	}
	ISplitViewSeparatorDrawer* getSplitViewSeparatorDrawer (CSplitView* splitView) override
	{
		return nullptr;
	}
	bool storeViewSize (int32_t index, const CCoord& size, CSplitView* splitView) override
	{
		return false;
	}
	bool restoreViewSize (int32_t index, CCoord& size, CSplitView* splitView) override
	{
		if (!gradientAdded)
		{
			if (auto view = desc->createView ("SplitViewSeperatorView", this))
			{
				if (auto container = view->asViewContainer ())
				{
					auto gradientView = container->getView (0);
					gradientView->removeAttribute ('cvcr');
					container->removeView (gradientView, false);
					auto viewSize = splitView->getViewSize ();
					auto sepWidth = splitView->getSeparatorWidth ();
					gradientView->setViewSize (CRect (0, 0, sepWidth, viewSize.getHeight ()));
					splitView->addViewToSeparator (0, gradientView);
					gradientAdded = true;
				}
				view->forget ();
			}
		}
		return false;
	}

private:
	const IUIDescription* desc {nullptr};
	bool gradientAdded {false};
};

//------------------------------------------------------------------------
std::shared_ptr<DocumentWindowController> DocumentWindowController::make (
    const DocumentContextPtr& doc)
{
	auto controller = std::make_shared<DocumentWindowController> (doc);

	UIDesc::Config config;
	config.uiDescFileName = "Window.uidesc";
	config.viewName = "Window";
	config.windowConfig.title = getDisplayFilename (doc->getPath ());
	config.windowConfig.autoSaveFrameName = "DocumentController";
	config.windowConfig.groupIdentifier = "Document";
	config.windowConfig.style.border ().close ().size ().centered ();
	config.customization = controller;
	config.modelBinding = controller->createModelBinding ();

	controller->window = UIDesc::makeWindow (config);
	return controller;
}

//------------------------------------------------------------------------
DocumentWindowController::DocumentWindowController (const DocumentContextPtr& doc)
: docContext (doc)
{
	for (auto index = 0u; index < docContext->getImagePaths ().size (); ++index)
		onImagePathAdded (docContext->getImagePaths ()[index], index);
	docContext->addListener (this);
	docIsDirty = false;
}

//------------------------------------------------------------------------
DocumentWindowController::~DocumentWindowController () noexcept
{
	docContext->removeListener (this);
}

//------------------------------------------------------------------------
void DocumentWindowController::showWindow ()
{
	if (window)
		window->show ();
}

//------------------------------------------------------------------------
void DocumentWindowController::closeWindow ()
{
	if (window)
		window->close ();
}

//------------------------------------------------------------------------
void DocumentWindowController::registerWindowListener (IWindowListener* listener)
{
	if (window)
		window->registerWindowListener (listener);
}

//------------------------------------------------------------------------
UIDesc::ModelBindingPtr DocumentWindowController::createModelBinding ()
{
	auto binding = UIDesc::ModelBindingCallbacks::make ();
	binding->addValue (Value::make ("AddPath"), UIDesc::ValueCalls::onAction ([this] (auto& v) {
		                   this->doAddPathCommand ();
		                   v.performEdit (0.);
	                   }));
	binding->addValue (Value::make ("RemovePath"), UIDesc::ValueCalls::onAction ([this] (auto& v) {
		                   this->doRemovePathCommand ();
		                   v.performEdit (0.);
	                   }));

	binding->addValue (Value::make ("Export"), UIDesc::ValueCalls::onAction ([this] (auto& v) {
						   this->doExport ();
						   v.performEdit (0.);
					   }));

	displayFrameValue = Value::makeStepValue ("DisplayFrame", 1, 1);
	binding->addValue (displayFrameValue);

	auto animationRunning = Value::make ("RunAnimation");
	binding->addValue (animationRunning, UIDesc::ValueCalls::onPerformEdit ([this] (auto& v) {
		                   if (v.getValue () >= 0.5)
			                   this->doStartAnimation ();
		                   else
			                   this->doStopAnimation ();
	                   }));
	animationTimeValue = Value::make ("AnimationTime", 0, Value::makeRangeConverter (16, 500, 0));
	binding->addValue (animationTimeValue,
	                   UIDesc::ValueCalls::onPerformEdit ([this, animationRunning] (auto& v) {
		                   if (animationRunning->getValue () >= 0.5)
			                   this->doStartAnimation ();
	                   }));
	numFramesPerRowValue =
		Value::make ("NumFramesPerRow", 0, Value::makeRangeConverter (1, 32767, 0));
	binding->addValue (numFramesPerRowValue, UIDesc::ValueCalls::onPerformEdit ([this] (auto& v) {
						   this->docContext->setNumFramesPerRow (static_cast<uint16_t> (
							   std::round (v.getConverter ().normalizedToPlain (v.getValue ()))));
					   }));
	return binding;
}

//------------------------------------------------------------------------
IController* DocumentWindowController::createController (const UTF8StringView& name,
                                                         IController* parent,
                                                         const IUIDescription* uiDesc)
{
	if (name == "ImageViewController")
		return new ImageViewController (
		    [&] (ImageFramesView* view) {
			    imageView = view;
			    imageView->setImageList (&imageList);
			    imageView->setDocContext (docContext);
		    },
		    parent);
	if (name == "MovieBitmapController")
		return new MovieBitmapController ([&] (CMovieBitmap* view) { movieBitmapView = view; },
		                                  parent);
	if (name == "SplitViewController")
		return new SplitViewController (parent, uiDesc);
	return nullptr;
}

//------------------------------------------------------------------------
void DocumentWindowController::onUIDescriptionParsed (const IUIDescription* uiDesc)
{
}

//------------------------------------------------------------------------
void DocumentWindowController::onSetContentView (IWindow& w, const SharedPointer<CFrame>& cv)
{
	contentView = cv;
	if (imageView)
		imageView->setImageList (&imageList);
}

//------------------------------------------------------------------------
void DocumentWindowController::onClosed (const IWindow& w)
{
	vstgui_assert (&w == window.get ());
	window = nullptr;
}

//------------------------------------------------------------------------
bool DocumentWindowController::canClose (const IWindow&)
{
	if (docIsDirty)
	{
		AlertBoxConfig alert;
		alert.headline = "Do you want to save the changes made to the document \"";
		alert.headline += getDisplayFilename (docContext->getPath ());
		alert.headline += "\"?";
		alert.description = "Your changes will be lost if you don't save them.";
		alert.defaultButton = "Save";
		alert.secondButton = "Cancel";
		alert.thirdButton = "Don't Save";
		auto result = IApplication::instance ().showAlertBox (alert);
		switch (result)
		{
			case AlertResult::DefaultButton:
			{
				if (pathIsAbsolute (docContext->getPath ()))
				{
					doSave ();
					return true;
				}
				doSaveAs ([this] (bool success) {
					if (success)
						window->close ();
				});
				return false;
			}
			case AlertResult::SecondButton: return false;
			default: return true;
		}
	}
	return true;
}

//------------------------------------------------------------------------
static bool exportImage (const SharedPointer<CBitmap>& image, UTF8StringPtr path)
{
	auto platformBitmap = image->getPlatformBitmap ();
	vstgui_assert (platformBitmap);
	auto buffer = getPlatformFactory ().createBitmapMemoryPNGRepresentation (platformBitmap);
	CFileStream stream;
	if (!stream.open (path, CFileStream::kWriteMode | CFileStream::kBinaryMode |
	                            CFileStream::kTruncateMode))
		return false;
	return stream.writeRaw (buffer.data (), static_cast<uint32_t> (buffer.size ())) ==
	       buffer.size ();
}

//------------------------------------------------------------------------
void DocumentWindowController::doExport ()
{
	auto fs =
	    owned (CNewFileSelector::create (contentView, CNewFileSelector::Style::kSelectSaveFile));
	if (!fs)
		return;
	fs->setTitle ("Export Stitched Image");
	fs->setDefaultExtension (pngFileExtension);
	// TODO: set filename depending on doc name
	fs->run ([this] (CNewFileSelector* fs) {
		if (fs->getNumSelectedFiles () == 0)
			return;
		if (auto image = createStitchedBitmap ())
		{
			if (!exportImage (image, fs->getSelectedFile (0)))
			{
				AlertBoxForWindowConfig alert;
				alert.window = window;
				alert.headline = "Export failed";
				IApplication::instance ().showAlertBoxForWindow (alert);
			}
		}
	});
}

//------------------------------------------------------------------------
void DocumentWindowController::doSave ()
{
	if (docContext->save ())
		docIsDirty = false;
}

//------------------------------------------------------------------------
void DocumentWindowController::doSaveAs (std::function<void (bool saved)>&& customAction)
{
	auto fs =
	    owned (CNewFileSelector::create (contentView, CNewFileSelector::Style::kSelectSaveFile));
	if (!fs)
		return;
	fs->setTitle ("Choose Save Destination");
	fs->setDefaultExtension (imageStitchExtension);
	fs->setInitialDirectory (docContext->getPath ().data ());
	fs->setDefaultSaveName (getDisplayFilename (docContext->getPath ()).data ());
	fs->run ([this, customAction = std::move (customAction)] (CNewFileSelector * fs) {
		if (fs->getNumSelectedFiles () == 0)
		{
			customAction (false);
			return;
		}
		docContext->setPath (fs->getSelectedFile (0));
		if (docContext->save ())
		{
			window->setTitle (getDisplayFilename (docContext->getPath ()));
			window->setRepresentedPath (UTF8String (docContext->getPath ()));
			docIsDirty = false;
			customAction (true);
		}
		else
			customAction (false);
	});
}

//------------------------------------------------------------------------
void DocumentWindowController::onImagePathAdded (const Path& newPath, size_t index)
{
	auto platformBitmap = getPlatformFactory ().createBitmapFromPath (newPath.data ());
	if (!platformBitmap)
	{
		CPoint size (docContext->getWidth (), docContext->getHeight ());
		platformBitmap = getPlatformFactory().createBitmap (size);
	}
	auto it = imageList.begin ();
	if (index >= imageList.size ())
		it = imageList.end ();
	else
		std::advance (it, index);
	imageList.insert (it, {makeOwned<CBitmap> (platformBitmap), newPath, false});
	setDirty ();
}

//------------------------------------------------------------------------
void DocumentWindowController::onImagePathRemoved (const Path& newPath, size_t index)
{
	auto it = imageList.begin ();
	std::advance (it, index);
	imageList.erase (it);
	setDirty ();
}

//------------------------------------------------------------------------
void DocumentWindowController::onNumFramesPerRowChanged (uint16_t newNumFramesPerRow)
{
	setDirty ();
}

//------------------------------------------------------------------------
void DocumentWindowController::doOpenDocument (std::function<void (bool saved)>&& customAction)
{
	auto fs = owned (CNewFileSelector::create (contentView));
	if (!fs)
		return;
	fs->setTitle ("Choose Document");
	fs->setDefaultExtension (imageStitchExtension);
	fs->run ([this, customAction = std::move (customAction)] (CNewFileSelector * fs) {
		if (fs->getNumSelectedFiles () == 0)
		{
			customAction (false);
			return;
		}
		if (auto newDocContext = DocumentContext::loadDocument (fs->getSelectedFile (0)))
		{
			docContext->replaceDocument (newDocContext->getDocument ());
			window->setTitle (getDisplayFilename (docContext->getPath ()));
			window->setRepresentedPath (UTF8String (docContext->getPath ()));
			docIsDirty = false;
			customAction (true);
		}
		else
			customAction (false);
	});
}

//------------------------------------------------------------------------
void DocumentWindowController::doAddPathCommand ()
{
	auto fs = owned (CNewFileSelector::create (contentView));
	if (!fs)
		return;
	fs->setAllowMultiFileSelection (true);
	fs->setTitle ("Choose Images");
	fs->setDefaultExtension (pngFileExtension);
	fs->run ([this] (CNewFileSelector* fs) {
		auto numFiles = fs->getNumSelectedFiles ();
		if (numFiles == 0)
			return;
		std::string alertDescription;
		size_t pos = lastSelectedPos ();
		doDeselectAllCommand ();
		for (auto i = 0u; i < numFiles; ++i)
		{
			auto path = fs->getSelectedFile (i);
			auto result = docContext->insertImagePathAtIndex (pos, path);
			if (result != DocumentContextResult::Success)
			{
				switch (result)
				{
					case DocumentContextResult::ImageSizeMismatch:
					{
						alertDescription += "Image Size Mismatch :";
						alertDescription += path;
						alertDescription += "\n";
						break;
					}
					case DocumentContextResult::InvalidImage:
					{
						alertDescription += "Invalid Image :";
						alertDescription += path;
						alertDescription += "\n";
						break;
					}
					case DocumentContextResult::InvalidIndex:
					{
						alertDescription +=
						    "Internal Error (DocumentContextResult::InvalidIndex) adding ";
						alertDescription += path;
						alertDescription += "\n";
						break;
					}
					case DocumentContextResult::Success: break;
				}
			}
			else
			{
				imageList[pos].selected = true;
				++pos;
			}
		}
		if (!alertDescription.empty ())
		{
			AlertBoxForWindowConfig alert;
			alert.window = window;
			alert.headline = "Error adding images!";
			alert.description = alertDescription;
			IApplication::instance ().showAlertBoxForWindow (alert);
		}
	});
}

//------------------------------------------------------------------------
void DocumentWindowController::doRemovePathCommand ()
{
	std::vector<size_t> indices;
	for (auto index = 0u; index < imageList.size (); ++index)
	{
		if (imageList[index].selected)
		{
			indices.push_back (index);
		}
	}
	for (auto it = indices.rbegin (); it != indices.rend (); ++it)
		docContext->removeImagePathAtIndex (*it);
}

//------------------------------------------------------------------------
void DocumentWindowController::doStartAnimation ()
{
	auto& converter = animationTimeValue->getConverter ();
	auto time =
	    static_cast<uint32_t> (converter.normalizedToPlain (animationTimeValue->getValue ()));
	timer = makeOwned<CVSTGUITimer> (
	    [this] (auto) {
		    auto v = displayFrameValue->getValue ();
		    v += 1. / imageList.size ();
		    if (v + std::numeric_limits<double>::epsilon () >= 1.)
			    v = 0.;
		    displayFrameValue->beginEdit ();
		    displayFrameValue->performEdit (v);
		    displayFrameValue->endEdit ();
	    },
	    time);
}

//------------------------------------------------------------------------
void DocumentWindowController::doStopAnimation ()
{
	if (!timer)
		return;

	timer->stop ();
	timer = nullptr;
}

//------------------------------------------------------------------------
void DocumentWindowController::doSelectAllCommand ()
{
	for (auto& image : imageList)
		image.selected = true;
	if (imageView)
		imageView->invalid ();
}

//------------------------------------------------------------------------
void DocumentWindowController::doDeselectAllCommand ()
{
	for (auto& image : imageList)
		image.selected = false;
	if (imageView)
		imageView->invalid ();
}

//------------------------------------------------------------------------
bool DocumentWindowController::somethingSelected () const
{
	for (auto& image : imageList)
	{
		if (image.selected)
			return true;
	}
	return false;
}

//------------------------------------------------------------------------
size_t DocumentWindowController::lastSelectedPos () const
{
	auto it =
	    std::find_if (imageList.rbegin (), imageList.rend (), [] (auto& e) { return e.selected; });
	if (it == imageList.rend ())
		return imageList.size ();
	return std::distance (imageList.begin (), it.base ());
}

//------------------------------------------------------------------------
bool DocumentWindowController::canHandleCommand (const Command& command)
{
	if (command == Commands::Delete)
		return somethingSelected ();
	if (command == Commands::SelectAll)
		return !imageList.empty ();
	if (command == ExportCommand)
		return !imageList.empty ();
	if (command == Commands::SaveDocumentAs)
		return !imageList.empty ();
	if (command == Commands::SaveDocument)
		return !imageList.empty () && pathIsAbsolute (docContext->getPath ());
	return false;
}

//------------------------------------------------------------------------
bool DocumentWindowController::handleCommand (const Command& command)
{
	if (command == Commands::Delete)
	{
		doRemovePathCommand ();
		return true;
	}
	if (command == Commands::SelectAll)
	{
		doSelectAllCommand ();
		return true;
	}
	if (command == ExportCommand)
	{
		doExport ();
		return true;
	}
	if (command == Commands::SaveDocument)
	{
		doSave ();
		return true;
	}
	if (command == Commands::SaveDocumentAs)
	{
		doSaveAs ();
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
SharedPointer<CBitmap> DocumentWindowController::createStitchedBitmap ()
{
	if (!contentView || docContext->getImagePaths ().empty ())
		return nullptr;
	auto numCols = docContext->getNumFramesPerRow ();
	auto numRows = std::ceil (static_cast<double> (docContext->getImagePaths ().size ()) / numCols);

	CRect r;
	CPoint size (docContext->getWidth (), docContext->getHeight ());
	r.setSize (size);
	size.x *= numCols;
	size.y *= numRows;

	auto offscreen = COffscreenContext::create (size);
	if (!offscreen)
		return nullptr;

	offscreen->beginDraw ();
	auto col = 0;
	for (const auto& image : imageList)
	{
		image.bitmap->draw (offscreen, r);
		if (++col >= numCols)
		{
			col = 0;
			r.left = 0;
			r.offset (0, docContext->getHeight ());
		}
		else
		{
			r.offset (docContext->getWidth (), 0);
		}
	}
	offscreen->endDraw ();

	auto multiFrameBitmap =
		makeOwned<CMultiFrameBitmap> (offscreen->getBitmap ()->getPlatformBitmap ());
	auto res = multiFrameBitmap->setMultiFrameDesc (
		{CPoint (docContext->getWidth (), docContext->getHeight ()),
		 static_cast<uint16_t> (imageList.size ()), numCols});
	vstgui_assert (res, "Multi Frame Bitmap Description invalid!");
	return multiFrameBitmap;
}

//------------------------------------------------------------------------
void DocumentWindowController::setDirty ()
{
	docIsDirty = true;
	if (asyncUpdateTriggered)
		return;
	asyncUpdateTriggered = true;
	Async::schedule (Async::mainQueue (), [this] () {
		if (auto v = displayFrameValue->dynamicCast<IMutableStepValue> ())
			v->setNumSteps (static_cast<uint32_t> (imageList.size ()));
		if (imageView)
			imageView->setImageList (&imageList);
		if (movieBitmapView)
		{
			movieBitmapView->setBackground (createStitchedBitmap ());
			auto size = movieBitmapView->getViewSize ();
			size.setWidth (docContext->getWidth ());
			size.setHeight (docContext->getHeight ());
			movieBitmapView->setViewSize (size);
		}
		asyncUpdateTriggered = false;
	});
}

//------------------------------------------------------------------------
} // ImageStitcher
} // VSTGUI
