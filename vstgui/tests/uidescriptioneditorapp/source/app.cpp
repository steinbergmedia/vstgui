#include "vstgui/standalone/include/helpers/appdelegate.h"
#include "vstgui/standalone/include/helpers/uidesc/modelbinding.h"
#include "vstgui/standalone/include/helpers/value.h"
#include "vstgui/standalone/include/helpers/windowcontroller.h"
#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/ialertbox.h"
#include "vstgui/standalone/include/icommand.h"
#include "vstgui/uidescription/cstream.h"
#include "vstgui/uidescription/uidescription.h"
#include "vstgui/uidescription/editing/uieditcontroller.h"
#include "vstgui/uidescription/editing/uiundomanager.h"
#include "vstgui/lib/cframe.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

using namespace Application;

//------------------------------------------------------------------------
class Controller : public WindowControllerAdapter, public ICommandHandler
{
public:
	Controller ()
	{
		descPath = __FILE__;
		unixfyPath (descPath);
		removeLastPathComponent (descPath);
		removeLastPathComponent (descPath);
		removeLastPathComponent (descPath);
		removeLastPathComponent (descPath);
		descPath += "/uidescription/editing/uidescriptioneditor.uidesc";
	}

	bool init ()
	{
		uidesc = makeOwned<UIDescription> (descPath.data ());
		if (!uidesc->parse())
		{
			// TODO: show alert about error
			IApplication::instance ().quit ();
			return false;
		}
		uidesc->setFilePath (descPath.data ());
		editController = makeOwned<UIEditController> (uidesc);

		IApplication::instance ().registerCommand (Commands::SaveDocument, 's');

		return true;
	}

	bool save ()
	{
		return uidesc->save (descPath.data (), UIDescription::kWriteImagesIntoXMLFile);
	}

	void beforeShow (IWindow& window) override
	{
		CRect r;
		r.setSize (window.getSize());
		auto frame = makeOwned<CFrame> (r, nullptr);
		if (auto view = editController->createEditView ())
		{
			editController->remember (); // view will forget it too
			view->setViewSize (r);
			frame->addView (view);
			window.setContentView (frame);
		}
	}

	bool canClose (const IWindow& window) override
	{
		if (!editController->getUndoManager()->isSavePosition ())
		{
			AlertBoxConfig config;
			config.headline = "There are unsaved changes.";
			config.description = "Do you want to save the changes ?";
			config.defaultButton = "Save";
			config.secondButton = "Cancel";
			config.thirdButton = "Don't Save";
			auto result = IApplication::instance().showAlertBox (config);
			switch (result)
			{
				case AlertResult::DefaultButton:
				{
					save ();
					return true;
				}
				case AlertResult::SecondButton:
				{
					return false;
				}
				default:
				{
					return true;
				}
			}
		}
		return true;
	}
	
	void onClosed (const IWindow& window) override
	{
		IApplication::instance ().quit ();
	}
	
	bool canHandleCommand (const Command& command) override
	{
		if (command == Commands::SaveDocument)
			return true;
		return false;
	}
	bool handleCommand (const Command& command) override
	{
		if (command == Commands::SaveDocument)
			return save ();
		return false;
	}

	std::string descPath;
	SharedPointer<UIDescription> uidesc;
	SharedPointer<UIEditController> editController;
};

//------------------------------------------------------------------------
class UIDescriptionEditorApp : public DelegateAdapter
{
public:
	UIDescriptionEditorApp () : DelegateAdapter ({"UIDescriptionEditorApp", "1.0.0", "vstgui.uidescriptioneditorapp"})
	{}

	void finishLaunching () override
	{
		auto controller = std::make_shared<Controller> ();
		if (controller->init ())
		{
			WindowConfiguration config;
			config.style.border().size().close().centered();
			config.title = "UIDescriptionEditor";
			config.autoSaveFrameName = "UIDescriptionEditorWindowFrame";
			config.size = {500, 500};
			if (auto window = IApplication::instance().createWindow (config, controller))
				window->show ();
			else
				IApplication::instance ().quit ();
		}
	}

};

static Init gAppDelegate (std::make_unique<UIDescriptionEditorApp> ());

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
