//
//  testwindow.cpp
//  VSTGUIStandalone
//
//  Created by Arne Scheffler on 23.12.15.
//  Copyright © 2015 vstgui. All rights reserved.
//

#include "testwindow.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/crect.h"
#include "vstgui/lib/cfileselector.h"
#include "vstgui/lib/cvstguitimer.h"
#include "vstgui/lib/controls/coptionmenu.h"
#include "vstgui/standalone/iapplication.h"
#include "vstgui/uidescription/uidescription.h"
#include "vstgui/uidescription/uiattributes.h"
#include "vstgui/uidescription/detail/uiviewcreatorattributes.h"
#include "vstgui/uidescription/editing/uieditcontroller.h"
#include "vstgui/uidescription/editing/uieditmenucontroller.h"

namespace MyApp {

using namespace VSTGUI::Standalone;
using namespace VSTGUI;

//------------------------------------------------------------------------
struct WindowController::Impl : public IController, public ICommandHandler
{
	Impl (WindowController& controller) : controller (controller) {}
	
	virtual bool init (WindowPtr& inWindow, const char* fileName, const char* templateName)
	{
		window = inWindow.get ();
		if (!initUIDesc (fileName))
			return false;
		frame = owned (new CFrame ({}, nullptr));
		frame->setTransparency (true);
		this->templateName = templateName;

		showView ();
		
		window->setContentView (frame);
		return true;
	}

	bool initUIDesc (const char* fileName)
	{
		uiDesc = owned (new UIDescription (fileName));
		if (!uiDesc->parse ())
		{
			return false;
		}
		return true;
	}

	void showView ()
	{
		auto view = uiDesc->createView (templateName, this);
		if (!view)
		{
			return;
		}
		frame->setSize (view->getWidth (), view->getHeight ());
		frame->addView (view);
		
		frame->setFocusDrawingEnabled (false);
		window->setSize (frame->getViewSize ().getSize ());
	}

	bool canHandleCommand (const Command& command) override { return false; }
	bool handleCommand (const Command& command) override { return false; }

	// IController
	void valueChanged (CControl* pControl) override {}
	int32_t controlModifierClicked (CControl* pControl, CButtonState button) override { return 0; }
	void controlBeginEdit (CControl* pControl) override {}
	void controlEndEdit (CControl* pControl) override {}
	void controlTagWillChange (CControl* pControl) override {}
	void controlTagDidChange (CControl* pControl) override {}
	int32_t getTagForName (UTF8StringPtr name, int32_t registeredTag) const override { return registeredTag; }
	IControlListener* getControlListener (UTF8StringPtr controlTagName) override { return this; }
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override { return nullptr; }
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override { return view; }
	IController* createSubController (UTF8StringPtr name, const IUIDescription* description) override { return nullptr; }

	WindowController& controller;
	IWindow* window {nullptr};
	SharedPointer<UIDescription> uiDesc;
	SharedPointer<CFrame> frame;
	UTF8String templateName;
};

#if VSTGUI_LIVE_EDITING
static Command ToggleEditingCommand {"Debug", "Toggle Inline Editor"};
	
//------------------------------------------------------------------------
struct WindowController::EditImpl : WindowController::Impl
{
	EditImpl (WindowController& controller) : Impl (controller)
	{
		IApplication::instance ().registerCommand (ToggleEditingCommand, 'e');
	}
	
	~EditImpl ()
	{
		save ();
	}

	bool init (WindowPtr& inWindow, const char* fileName, const char* templateName) override
	{
		window = inWindow.get ();
		if (!initUIDesc(fileName))
		{
			UIAttributes* attr = new UIAttributes ();
			attr->setAttribute (UIViewCreator::kAttrClass, "CViewContainer");
			attr->setAttribute ("size", "300, 300");
			uiDesc->addNewTemplate (templateName, attr);
			
			Call::later ([this] () {
				auto fs = owned (CNewFileSelector::create (frame, CNewFileSelector::kSelectSaveFile));
				fs->setDefaultSaveName (uiDesc->getFilePath ());
				fs->setTitle ("Save UIDescription File");
				fs->run ([this] (CNewFileSelector* fileSelector) {
					if (fileSelector->getNumSelectedFiles () == 0)
					{
						Call::later ([this] () {
							window->close ();
						});
						return;
					}
					auto path = fileSelector->getSelectedFile (0);
					uiDesc->setFilePath (path);
					save (true);
				});
			});
		}
		frame = owned (new CFrame ({}, nullptr));
		frame->setTransparency (true);
		this->templateName = templateName;
		
		enableEditing (true);
		
		window->setContentView (frame);
		return true;
	}
	
	void save (bool force = false)
	{
		if (uiEditController)
		{
			if (force || uiEditController->getUndoManager ()->isSavePosition () == false)
				uiDesc->save (uiDesc->getFilePath (), uiEditController->getSaveOptions ());
		}
	}
	
	void enableEditing (bool state)
	{
		if (isEditing == state && frame->getNbViews () != 0)
			return;
		isEditing = state;
		if (!state)
		{
			save ();
			uiEditController = nullptr;
		}

		frame->removeAll ();
		if (state)
		{
			uiDesc->setController (this);
			uiEditController = new UIEditController (uiDesc);
			auto view = uiEditController->createEditView ();
			frame->setSize (view->getWidth (), view->getHeight ());
			frame->addView (view);
			frame->enableTooltips (true);
			CColor focusColor = kBlueCColor;
			uiEditController->getEditorDescription ().getColor ("focus", focusColor);
			frame->setFocusColor (focusColor);
			frame->setFocusDrawingEnabled (true);
			frame->setFocusWidth (1);
			window->setSize (frame->getViewSize ().getSize ());
		}
		else
		{
			showView ();
		}
	}

	bool canHandleCommand (const Command& command) override
	{
		if (command == ToggleEditingCommand)
			return true;
		return false;
	}
	
	bool handleCommand (const Command& command) override
	{
		if (command == ToggleEditingCommand)
		{
			enableEditing (!isEditing);
			return true;
		}
		return false;
	}
	
	SharedPointer<UIEditController> uiEditController;
	bool isEditing {false};
};
#endif

//------------------------------------------------------------------------
WindowController::WindowController ()
{
#if VSTGUI_LIVE_EDITING
	impl = std::unique_ptr<Impl> (new EditImpl (*this));
#else
	impl = std::unique_ptr<Impl> (new Impl (*this));
#endif
}

//------------------------------------------------------------------------
bool WindowController::canHandleCommand (const Command& command)
{
	return impl->canHandleCommand (command);
}

//------------------------------------------------------------------------
bool WindowController::handleCommand (const Command& command)
{
	return impl->handleCommand (command);
}

//------------------------------------------------------------------------
WindowPtr WindowController::makeWindow ()
{
	auto controller = std::make_shared<WindowController> ();

	WindowConfiguration config;
	config.title = "Test Window";
	config.flags.border ().close ().size ();
	config.size = {100, 100};
	auto window = IApplication::instance ().createWindow (config, controller);
	if (window)
	{
		if (!controller->impl->init (window, "test.uidesc", "view"))
		{
			return nullptr;
		}
		window->show ();
	}
	return window;
}

//------------------------------------------------------------------------
}