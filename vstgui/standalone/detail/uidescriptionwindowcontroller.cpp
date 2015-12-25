#include "../iuidescwindow.h"
#include "../iwindowcontroller.h"
#include "../iapplication.h"
#include "../../lib/iviewlistener.h"
#include "../../lib/cframe.h"
#include "../../lib/crect.h"
#include "../../lib/cfileselector.h"
#include "../../lib/cvstguitimer.h"
#include "../../lib/controls/coptionmenu.h"
#include "../../lib/controls/ctextedit.h"
#include "../../uidescription/uidescription.h"
#include "../../uidescription/uiattributes.h"
#include "../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../uidescription/editing/uieditcontroller.h"
#include "../../uidescription/editing/uieditmenucontroller.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace /* Anonymous */ {

using UIDescription::ModelHandlerPtr;

//------------------------------------------------------------------------
class WindowController : public WindowControllerAdapter, public ICommandHandler
{
public:
	bool init (const UIDescription::Config& config, WindowPtr& window);
	
	CPoint constraintSize (const IWindow& window, const CPoint& newSize) override;
	void onClosed (const IWindow& window) override;
	bool canClose (const IWindow& window) const override;
	
	bool canHandleCommand (const Command& command) override;
	bool handleCommand (const Command& command) override;
private:
	
	struct Impl;
	struct EditImpl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
class ValueWrapper : public IValueListener, public IControlListener, public IViewListenerAdapter
{
public:
	ValueWrapper (const ValuePtr& value = nullptr)
	: value (value)
	{
		if (value)
			value->registerListener (this);
	}
	~ValueWrapper ()
	{
		if (value)
			value->unregisterListener (this);
		for (auto& control : controls)
		{
			control->unregisterViewListener (this);
			control->unregisterControlListener (this);
		}
	}

	IdStringPtr getID () const { return value->getID (); }
	
	bool hasID (UTF8StringPtr id) const
	{
		return strcmp (getID (), id) == 0;
	}

	void onBeginEdit (const IValue& value) override {}
	void onPerformEdit (const IValue& value, IValue::Type newValue) override
	{
		for (auto& c : controls)
		{
			c->setValueNormalized (static_cast<float> (newValue));
			c->invalid ();
		}
	}
	void onEndEdit (const IValue& value) override {}

	void updateControlOnStateChange (CControl* control) const
	{
		control->setMin (0.f);
		control->setMax (1.f);
		control->setMouseEnabled (value->isActive ());
		auto stepValue = value->dynamicCast<const IStepValue> ();
		if (!stepValue)
			return;
		if (auto menu = dynamic_cast<COptionMenu*>(control))
		{
			menu->removeAllEntry ();
			if (stepValue)
			{
				for (IStepValue::StepType i = 0; i < stepValue->getSteps (); ++i)
				{
					auto title = value->getStringConverter ().valueAsString (stepValue->stepToValue (i));
					menu->addEntry (title);
				}
			}
		}
	}

	void onStateChange (const IValue& value) override
	{
		for (auto& c : controls)
		{
			updateControlOnStateChange (c);
		}
	}

	void valueChanged (CControl* control) override
	{
		auto preValue = value->getValue ();
		value->performEdit (control->getValueNormalized ());
		if (value->getValue () == preValue)
			onPerformEdit (*value, value->getValue ());
	}

	void controlBeginEdit (CControl* control) override { value->beginEdit (); }
	void controlEndEdit (CControl* control) override { value->endEdit (); }

	void addControl (CControl* control)
	{
		if (auto paramDisplay = dynamic_cast<CParamDisplay*> (control))
		{
			paramDisplay->setValueToStringFunction ([this] (float value, char utf8String[256], CParamDisplay* display) {
				auto string = this->value->getStringConverter ().valueAsString (value);
				auto numBytes = std::min<size_t> (string.getByteCount (), 255);
				if (numBytes)
					strncpy (utf8String, string.get (), numBytes);
				else
					utf8String[0] = 0;
				return true;
			});
			if (auto textEdit = dynamic_cast<CTextEdit*>(paramDisplay))
			{
				textEdit->setStringToValueFunction ([&] (UTF8StringPtr txt, float& result, CTextEdit* textEdit) {
					auto v = value->getStringConverter ().stringAsValue (txt);
					if (v == IValue::InvalidValue)
						v = value->getValue ();
					result = static_cast<float> (v);
					return true;
				});
			}
		}
		updateControlOnStateChange (control);
		control->setValueNormalized (static_cast<float> (value->getValue ()));
		control->invalid ();
		control->registerControlListener (this);
		control->registerViewListener (this);
		controls.push_back (control);
	}

	void removeControl (CControl* control)
	{
		if (auto paramDisplay = dynamic_cast<CParamDisplay*> (control))
		{
			paramDisplay->setValueToStringFunction (nullptr);
			if (auto textEdit = dynamic_cast<CTextEdit*>(paramDisplay))
				textEdit->setStringToValueFunction (nullptr);
		}
		control->unregisterViewListener (this);
		control->unregisterControlListener (this);
		auto it = std::find (controls.begin (), controls.end (), control);
		assert (it != controls.end ());
		controls.erase (it);
	}
	
	void viewWillDelete (CView* view) override
	{
		removeControl (dynamic_cast<CControl*> (view));
	}

protected:

	using ControlList = std::vector<CControl*>;
	
	ValuePtr value;
	ControlList controls;
};

using ValueWrapperPtr = std::shared_ptr<ValueWrapper>;

//------------------------------------------------------------------------
struct WindowController::Impl : public IController, public ICommandHandler
{
	using ValueWrapperList = std::vector<ValueWrapperPtr>;
	
	Impl (WindowController& controller, const ModelHandlerPtr& modelHandler)
	: controller (controller), modelHandler (modelHandler)
	{
		initModelValues (modelHandler);
	}
	
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
	
	virtual CPoint constraintSize (const CPoint& newSize)
	{
		CPoint p (newSize);
		if (minSize.x > 0 && minSize.y > 0)
		{
			p.x = std::max (p.x, minSize.x);
			p.y = std::max (p.y, minSize.y);
		}
		if (maxSize.x > 0 && maxSize.y > 0)
		{
			p.x = std::min (p.x, maxSize.x);
			p.y = std::min (p.y, maxSize.y);
		}
		return p;
	}
	
	virtual bool canClose () { return true; }
	
	bool initUIDesc (const char* fileName)
	{
		uiDesc = owned (new VSTGUI::UIDescription (fileName));
		if (!uiDesc->parse ())
		{
			return false;
		}
		return true;
	}
	
	void updateMinMaxSizes ()
	{
		const UIAttributes* attr = uiDesc->getViewAttributes (templateName);
		if (!attr)
			return;
		CPoint p;
		if (attr->getPointAttribute ("minSize", p))
			minSize = p;
		else
			minSize = {};
		if (attr->getPointAttribute ("maxSize", p))
			maxSize = p;
		else
			maxSize = {};
	}
	
	void showView ()
	{
		updateMinMaxSizes ();
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
	
	void initModelValues (const ModelHandlerPtr& modelHandler)
	{
		for (auto& value : modelHandler->getValues ())
		{
			valueWrappers.emplace_back (std::make_shared<ValueWrapper> (value));
		}
	}

	// IController
	void valueChanged (CControl* control) override {}
	int32_t controlModifierClicked (CControl* control, CButtonState button) override { return 0; }
	void controlBeginEdit (CControl* control) override {}
	void controlEndEdit (CControl* control) override {}
	void controlTagWillChange (CControl* control) override
	{
		if (control->getTag () < 0)
			return;
		auto index = static_cast<ValueWrapperList::size_type>(control->getTag ());
		if (index < valueWrappers.size ())
			valueWrappers[index]->removeControl (control);
	}
	void controlTagDidChange (CControl* control) override
	{
		if (control->getTag () < 0)
			return;
		auto index = static_cast<ValueWrapperList::size_type>(control->getTag ());
		if (index < valueWrappers.size ())
			valueWrappers[index]->addControl (control);
	}
	
	int32_t getTagForName (UTF8StringPtr name, int32_t registeredTag) const override
	{
		auto it = std::find_if(valueWrappers.begin(), valueWrappers.end(), [&] (const ValueWrapperPtr& v) {
			return v->hasID (name);
		});
		if (it != valueWrappers.end())
			return static_cast<int32_t> (std::distance (valueWrappers.begin (), it));
		return registeredTag;
	}
	
	IControlListener* getControlListener (UTF8StringPtr controlTagName) override { return this; }
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override { return nullptr; }
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override { return view; }
	IController* createSubController (UTF8StringPtr name, const IUIDescription* description) override { return nullptr; }
	
	WindowController& controller;
	IWindow* window {nullptr};
	SharedPointer<VSTGUI::UIDescription> uiDesc;
	SharedPointer<CFrame> frame;
	UTF8String templateName;
	CPoint minSize;
	CPoint maxSize;
	ModelHandlerPtr modelHandler;
	ValueWrapperList valueWrappers;
};

#if VSTGUI_LIVE_EDITING
static const Command ToggleEditingCommand {"Debug", "Toggle Inline Editor"};

//------------------------------------------------------------------------
struct WindowController::EditImpl : WindowController::Impl
{
	EditImpl (WindowController& controller, const ModelHandlerPtr& modelHandler) : Impl (controller, modelHandler)
	{
		IApplication::instance ().registerCommand (ToggleEditingCommand, 'e');
	}
	
	bool init (WindowPtr& inWindow, const char* fileName, const char* templateName) override
	{
		window = inWindow.get ();
		bool initialEditing = false;
		if (!initUIDesc (fileName))
		{
			initialEditing = true;
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
					auto settings = uiDesc->getCustomAttributes ("UIDescFilePath", true);
					settings->setAttribute ("path", path);
					save (true);
				});
			});
		}
		else
		{
			auto settings = uiDesc->getCustomAttributes ("UIDescFilePath", true);
			auto filePath = settings->getAttributeValue ("path");
			if (filePath)
				uiDesc->setFilePath (filePath->data ());
		}
		frame = owned (new CFrame ({}, nullptr));
		frame->setTransparency (true);
		this->templateName = templateName;
		
		syncTags ();
		if (initialEditing)
			enableEditing (true);
		else
			showView ();
		
		window->setContentView (frame);
		return true;
	}
	
	CPoint constraintSize (const CPoint& newSize) override
	{
		if (isEditing)
			return newSize;
		return Impl::constraintSize (newSize);
	}
	
	bool canClose () override
	{
		enableEditing (false);
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

	void syncTags ()
	{
		std::list<const std::string*> tagNames;
		uiDesc->collectControlTagNames (tagNames);
		int32_t index = 0;
		for (auto& v : valueWrappers)
		{
			auto it = std::find_if (tagNames.begin(), tagNames.end(), [&] (const std::string* name) {
				return v->hasID (name->data ());
			});
			auto create = it == tagNames.end ();
			uiDesc->changeControlTagString (v->getID (), std::to_string (index), create);
			++index;
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
bool WindowController::init (const UIDescription::Config &config, WindowPtr &window)
{
#if VSTGUI_LIVE_EDITING
	impl = std::unique_ptr<Impl> (new EditImpl (*this, config.modelHandler));
#else
	impl = std::unique_ptr<Impl> (new Impl (*this, config.modelHandler));
#endif
	return impl->init (window, config.fileName, config.viewName);
}

//------------------------------------------------------------------------
CPoint WindowController::constraintSize (const IWindow& window, const CPoint& newSize)
{
	return impl->constraintSize (newSize);
}

//------------------------------------------------------------------------
bool WindowController::canClose (const IWindow& window) const
{
	return impl->canClose ();
}

//------------------------------------------------------------------------
void WindowController::onClosed (const IWindow& window)
{
	impl = nullptr;
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
} // Anonymous

//------------------------------------------------------------------------
namespace UIDescription {

//------------------------------------------------------------------------
WindowPtr makeWindow (const Config& config)
{
	assert (config.modelHandler);
	assert (config.viewName.empty () == false);
	assert (config.fileName.empty () == false);

	auto controller = std::make_shared<WindowController> ();

	WindowConfiguration windowConfig = config.windowConfig;
#if VSTGUI_LIVE_EDITING
	windowConfig.flags.size ();
#endif

	auto window = IApplication::instance ().createWindow (windowConfig, controller);
	if (!window)
		return nullptr;

	if (!controller->init (config, window))
		return nullptr;

	return window;
}

//------------------------------------------------------------------------
} // UIDescription
} // Standalone
} // VSTGUI
