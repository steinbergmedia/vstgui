// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "documentcontroller.h"
#include "vstgui/standalone/include/helpers/appdelegate.h"
#include "vstgui/standalone/include/helpers/menubuilder.h"
#include "vstgui/standalone/include/helpers/windowlistener.h"
#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/icommand.h"
#include "vstgui/standalone/include/iuidescwindow.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ImageStitcher {

using namespace VSTGUI::Standalone;
using namespace VSTGUI::Standalone::Application;

//------------------------------------------------------------------------
class ImageStitcherAppDelegate : public DelegateAdapter,
                                 public ICommandHandler,
                                 public MenuBuilderAdapter
{
public:
	ImageStitcherAppDelegate ()
	: DelegateAdapter ({"ImageStitcher", "1.0.0", "com.steinberg.vstgui.imagestitcher"})
	{
	}

	bool openFiles (const std::vector<UTF8String>& paths) override
	{
		for (auto& path : paths)
		{
			if (auto docContext = DocumentContext::loadDocument (path.getString ()))
			{
				if (auto window = DocumentWindowController::makeDocWindow (docContext))
				{
					window->show ();
				}
			}
		}
		return false;
	}

	void finishLaunching () override
	{
		auto& app = IApplication::instance ();
		app.registerCommand (Commands::NewDocument, 'n');
		app.registerCommand (Commands::OpenDocument, 'o');
		app.registerCommand (Commands::SaveDocument, 's');
		app.registerCommand (Commands::SaveDocumentAs, 'S');
		app.registerCommand (ExportCommand, 'e');
		if (app.getWindows ().empty ())
		{
			doNewDocumentCommand ();
		}
	}

	void doNewDocumentCommand ()
	{
		auto docContext = DocumentContext::makeEmptyDocument ();
		if (auto window = DocumentWindowController::makeDocWindow (docContext))
		{
			window->show ();
		}
	}

	void doOpenDocument ()
	{
		auto fs = owned (CNewFileSelector::create ());
		if (!fs)
			return;
		fs->setAllowMultiFileSelection (false);
		fs->setTitle ("Choose Document");
		fs->setDefaultExtension (imageStitchExtension);
		fs->run ([this] (CNewFileSelector* fs) {
			if (fs->getNumSelectedFiles () == 0)
				return;
			if (auto docContext = DocumentContext::loadDocument (fs->getSelectedFile (0)))
			{
				if (auto window = DocumentWindowController::makeDocWindow (docContext))
				{
					window->show ();
				}
			}
		});
	}

	bool canHandleCommand (const Command& command) override
	{
		if (command == Commands::NewDocument)
			return true;
		if (command == Commands::OpenDocument)
			return true;
		return false;
	}

	bool handleCommand (const Command& command) override
	{
		if (command == Commands::NewDocument)
		{
			doNewDocumentCommand ();
			return true;
		}
		if (command == Commands::OpenDocument)
		{
			doOpenDocument ();
		}
		return false;
	}

	bool prependMenuSeparator (const Interface& context, const Command& cmd) const override
	{
		if (cmd == ExportCommand || cmd == Commands::CloseWindow)
			return true;
		return false;
	}

	SortFunction getCommandGroupSortFunction (const Interface& context,
	                                          const UTF8String& group) const override
	{
		if (group == CommandGroup::File)
		{
			return [] (const UTF8String& lhs, const UTF8String& rhs) {
				static auto order = {Commands::NewDocument.name,  Commands::OpenDocument.name,
				                     Commands::SaveDocument.name, Commands::SaveDocumentAs.name,
				                     ExportCommand.name,          Commands::CloseWindow.name};
				auto leftIndex = std::find (order.begin (), order.end (), lhs);
				auto rightIndex = std::find (order.begin (), order.end (), rhs);
				return std::distance (leftIndex, rightIndex) > 0;
			};
		}
		return {};
	}
};

static Init gAppDelegate (std::make_unique<ImageStitcherAppDelegate> ());

//------------------------------------------------------------------------
} // ImageStitcher
} // VSTGUI
