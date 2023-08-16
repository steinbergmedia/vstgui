// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "testappdelegate.h"
#include "testmodel.h"
#include "testabout.h"
#include "AlertBoxDesign.h"
#include "vstgui/standalone/include/helpers/appdelegate.h"
#include "vstgui/standalone/include/helpers/windowlistener.h"
#include "vstgui/standalone/include/helpers/uidesc/customization.h"
#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/iwindow.h"
#include "vstgui/standalone/include/icommand.h"
#include "vstgui/standalone/include/iuidescwindow.h"
#include "vstgui/standalone/include/ialertbox.h"
#include "vstgui/uidescription/delegationcontroller.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/crect.h"
#include "vstgui/lib/iviewlistener.h"
#include "vstgui/lib/controls/ccontrol.h"
#include "vstgui/lib/controls/clistcontrol.h"

#include "vstgui/standalone/source/genericalertbox.h"

#include "vstgui/uidescription/uiattributes.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/lib/cexternalview.h"
#include "vstgui/contrib/datepicker.h"
#include "vstgui/contrib/evbutton.h"

#include <memory>

#if MAC
#include "metalwindow.h"
#elif WINDOWS
#include "direct3dwindow.h"
#endif

//------------------------------------------------------------------------
namespace MyApp {

using namespace VSTGUI;
using namespace VSTGUI::Standalone;

//------------------------------------------------------------------------
class Delegate : public Application::DelegateAdapter,
				 public ICommandHandler,
				 public WindowListenerAdapter
{
public:
	Delegate ();

	// Application::IDelegate
	void finishLaunching () override;
	void showAboutDialog () override;
	bool hasAboutDialog () override;
	UTF8StringPtr getSharedUIResourceFilename () const override;

	// ICommandHandler
	bool canHandleCommand (const Command& command) override;
	bool handleCommand (const Command& command) override;

	// WindowListenerAdapter
	void onClosed (const IWindow& window) override;

private:
	std::shared_ptr<TestModel> model;
};

//------------------------------------------------------------------------
Application::Init gAppDelegate (std::make_unique<Delegate> (),
								{{Application::ConfigKey::ShowCommandsInContextMenu, 1}});

static Command NewPopup {CommandGroup::File, "New Popup"};
static Command ShowAlertBoxDesign {CommandGroup::File, "Show AlertBox Design"};
#if MAC
static Command NewMetalExampleWindow {CommandGroup::File, "New Metal Example Window"};
#elif WINDOWS
static Command NewDirect3DExampleWindow {CommandGroup::File, "New Direct3D Example Window"};
#endif

//------------------------------------------------------------------------
class DisabledControlsController : public DelegationController,
								   public ViewListenerAdapter
{
public:
	DisabledControlsController (IController* parent) : DelegationController (parent) {}
	~DisabledControlsController ()
	{
		for (auto control : controls)
		{
			control->unregisterViewListener (this);
		}
		controls.clear ();
	}

	CView* verifyView (CView* view, const UIAttributes& attributes,
					   const IUIDescription* description) override
	{
		if (auto control = dynamic_cast<CControl*> (view))
		{
			control->registerViewListener (this);
			controls.push_back (control);
		}
		return controller->verifyView (view, attributes, description);
	}

	void viewOnMouseEnabled (CView* view, bool state) override
	{
		view->setAlphaValue (state ? 1.f : 0.5f);
	}

	void viewWillDelete (CView* view) override
	{
		if (auto control = dynamic_cast<CControl*> (view))
		{
			auto it = std::find (controls.begin (), controls.end (), control);
			if (it != controls.end ())
			{
				control->unregisterViewListener (this);
				controls.erase (it);
			}
		}
	}

	std::vector<CControl*> controls;
};

//------------------------------------------------------------------------
class WeekdaysListConfigurator : public StaticListControlConfigurator
{
public:
	WeekdaysListConfigurator (const StaticListControlConfigurator& c)
	: StaticListControlConfigurator (c.getRowHeight (), c.getFlags ())
	{
	}

	CListControlRowDesc getRowDesc (int32_t row) const override
	{
		if (row == 0)
			return {getRowHeight () * 2., 0};
		return {getRowHeight (), getFlags ()};
	}
};

//------------------------------------------------------------------------
class WeekdaysController : public DelegationController
{
public:
	WeekdaysController (IController* parent) : DelegationController (parent) {}

	CView* verifyView (CView* view, const UIAttributes& attributes,
					   const IUIDescription* description) override
	{
		if (auto listControl = dynamic_cast<CListControl*> (view))
		{
			auto configurator =
				dynamic_cast<StaticListControlConfigurator*> (listControl->getConfigurator ());
			if (configurator)
			{
				listControl->setConfigurator (makeOwned<WeekdaysListConfigurator> (*configurator));
			}
		}
		return controller->verifyView (view, attributes, description);
	}
};

//------------------------------------------------------------------------
class DatePickerController : public DelegationController
{
public:
	DatePickerController (IController* parent) : DelegationController (parent) {}

#if MAC || WINDOWS
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override
	{
		if (auto customViewName = attributes.getAttributeValue (IUIDescription::kCustomViewName))
		{
			if (*customViewName == "DatePicker")
			{
				auto datePicker = std::make_shared<ExternalView::DatePicker> ();
				datePicker->setDate ({12, 8, 2023});
				datePicker->setChangeCallback ([] (auto date) {
#if DEBUG
					DebugPrint ("%d.%d.%d\n", date.day, date.month, date.year);
#endif
				});
				return new CExternalView (CRect (), datePicker);
			}
			if (*customViewName == "Native Checkbox")
			{
				auto checkbox = std::make_shared<ExternalView::Button> (
					ExternalView::Button::Type::Checkbox, "Checkbox");
				return new CExternalControl (CRect (), checkbox);
			}
			if (*customViewName == "Native Push Button")
			{
				auto checkbox = std::make_shared<ExternalView::Button> (
					ExternalView::Button::Type::Push, "Push");
				return new CExternalControl (CRect (), checkbox);
			}
			if (*customViewName == "Native OnOff Button")
			{
				auto checkbox = std::make_shared<ExternalView::Button> (
					ExternalView::Button::Type::OnOff, "OnOff");
				return new CExternalControl (CRect (), checkbox);
			}
			if (*customViewName == "Native Radio Button")
			{
				auto checkbox = std::make_shared<ExternalView::Button> (
					ExternalView::Button::Type::Radio, "Radio");
				return new CExternalControl (CRect (), checkbox);
			}
		}
		return controller->createView (attributes, description);
	}
#endif
};

//------------------------------------------------------------------------
Delegate::Delegate ()
: Application::DelegateAdapter ({"VSTGUI Standalone", "1.0.0", VSTGUI_STANDALONE_APP_URI})
{
}

//------------------------------------------------------------------------
void Delegate::finishLaunching ()
{
	model = std::make_shared<TestModel> ();
	IApplication::instance ().registerCommand (Commands::NewDocument, 'n');
	IApplication::instance ().registerCommand (NewPopup, 'N');
	IApplication::instance ().registerCommand (ShowAlertBoxDesign, 'b');
#if MAC
	IApplication::instance ().registerCommand (NewMetalExampleWindow, 'M');
#elif WINDOWS
	IApplication::instance ().registerCommand (NewDirect3DExampleWindow, 'D');
#endif
	handleCommand (Commands::NewDocument);
}

//------------------------------------------------------------------------
void Delegate::onClosed (const IWindow& window)
{
	if (IApplication::instance ().getWindows ().empty ())
	{
		IApplication::instance ().quit ();
	}
}

//------------------------------------------------------------------------
bool Delegate::canHandleCommand (const Command& command)
{
#if MAC
	if (command == NewMetalExampleWindow)
		return true;
#elif WINDOWS
	if (command == NewDirect3DExampleWindow)
		return true;
#endif
	return command == Commands::NewDocument || command == NewPopup || command == ShowAlertBoxDesign;
}

//------------------------------------------------------------------------
bool Delegate::handleCommand (const Command& command)
{
	if (command == Commands::NewDocument || command == NewPopup)
	{
		UIDesc::Config config;
		config.windowConfig.title = "Test Window";
		config.windowConfig.autoSaveFrameName = "TestWindowFrame";
		config.windowConfig.style.close ();
		config.windowConfig.size = {100, 100};
		config.viewName = "view";
		config.modelBinding = model;
		if (command == NewPopup)
		{
			config.uiDescFileName = "testpopup.uidesc";
			config.windowConfig.type = WindowType::Popup;
			config.windowConfig.style.movableByWindowBackground ();
		}
		else
		{
			config.uiDescFileName = "test.uidesc";
			config.windowConfig.style.border ().size ();
			config.windowConfig.style.movableByWindowBackground ();
			auto customization = UIDesc::Customization::make ();
			customization->addCreateViewControllerFunc (
				"DisabledControlsController",
				[] (const UTF8StringView&, IController* parent, const IUIDescription*) {
					return new DisabledControlsController (parent);
				});
			customization->addCreateViewControllerFunc (
				"WeekdaysController",
				[] (const UTF8StringView&, IController* parent, const IUIDescription*) {
					return new WeekdaysController (parent);
				});
			customization->addCreateViewControllerFunc (
				"DatePickerController",
				[] (const UTF8StringView&, IController* parent, const IUIDescription*) {
					return new DatePickerController (parent);
				});
			config.customization = customization;
		}
		if (auto window = UIDesc::makeWindow (config))
		{
			window->show ();
			window->registerWindowListener (this);
		}
		return true;
	}
	else if (command == ShowAlertBoxDesign)
	{
		if (auto window = VSTGUI::Standalone::makeAlertBoxDesignWindow ())
			window->show ();
		return true;
	}
#if MAC
	else if (command == NewMetalExampleWindow)
	{
		if (auto window = makeNewMetalExampleWindow ())
			window->show ();
		return true;
	}
#elif WINDOWS
	else if (command == NewDirect3DExampleWindow)
	{
		if (auto window = makeNewDirect3DExampleWindow ())
			window->show ();
		return true;
	}
#endif
	return false;
}

//------------------------------------------------------------------------
void Delegate::showAboutDialog () { About::show (); }

//------------------------------------------------------------------------
bool Delegate::hasAboutDialog () { return true; }

//------------------------------------------------------------------------
VSTGUI::UTF8StringPtr Delegate::getSharedUIResourceFilename () const { return "resources.uidesc"; }

} // MyApp
