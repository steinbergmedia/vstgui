// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../lib/cfileselector.h"
#include "../../lib/cframe.h"
#include "../../lib/controls/coptionmenu.h"
#include "../../lib/controls/csegmentbutton.h"
#include "../../lib/controls/ctextedit.h"
#include "../../lib/crect.h"
#include "../../lib/cvstguitimer.h"
#include "../../lib/iviewlistener.h"
#include "../../uidescription/cstream.h"
#include "../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../uidescription/editing/uieditcontroller.h"
#include "../../uidescription/editing/uieditmenucontroller.h"
#include "../../uidescription/uiattributes.h"
#include "../../uidescription/uidescription.h"
#include "../../uidescription/icontroller.h"
#include "../include/helpers/menubuilder.h"
#include "../include/helpers/valuelistener.h"
#include "../include/helpers/windowcontroller.h"
#include "../include/ialertbox.h"
#include "../include/iappdelegate.h"
#include "../include/iapplication.h"
#include "../include/iasync.h"
#include "../include/iuidescwindow.h"
#include "application.h"
#include "shareduiresources.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace /* Anonymous */ {

using UIDesc::ModelBindingPtr;
using UIDesc::CustomizationPtr;

//------------------------------------------------------------------------
class WindowController : public IWindowController, public ICommandHandler
{
public:
	bool init (const UIDesc::Config& config, WindowPtr& window);
	bool initStatic (const UIDesc::Config& config, WindowPtr& window);

	void onSizeChanged (const IWindow& window, const CPoint& newSize) override;
	void onPositionChanged (const IWindow& window, const CPoint& newPosition) override;
	void onShow (const IWindow& window) override;
	void onHide (const IWindow& window) override;
	void onClosed (const IWindow& window) override;
	void onActivated (const IWindow& window) override;
	void onDeactivated (const IWindow& window) override;
	CPoint constraintSize (const IWindow& window, const CPoint& newSize) override;
	bool canClose (const IWindow& window) override;
	void beforeShow (IWindow& window) override;
	void onSetContentView (IWindow& window, const SharedPointer<CFrame>& contentView) override;

	bool canHandleCommand (const Command& command) override;
	bool handleCommand (const Command& command) override;

	IMenuBuilder* getWindowMenuBuilder (const IWindow& window) const override;

private:
	struct Impl;
	struct EditImpl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
class ValueWrapper : public ValueListenerAdapter,
                     public IControlListener,
                     public IViewListenerAdapter
{
public:
	explicit ValueWrapper (const ValuePtr& value = nullptr) : value (value)
	{
		if (value)
			value->registerListener (this);
	}
	~ValueWrapper () override
	{
		if (value)
			value->unregisterListener (this);
		for (auto& control : controls)
		{
			control->unregisterViewListener (this);
			control->unregisterControlListener (this);
		}
	}

	const UTF8String& getID () const { return value->getID (); }

	void onBeginEdit (IValue& value) override
	{
		for (auto& c : controls)
		{
			if (auto listener = c->getListener ())
				listener->controlBeginEdit (c);
		}
	}

	void onEndEdit (IValue& value) override
	{
		for (auto& c : controls)
		{
			if (auto listener = c->getListener ())
				listener->controlEndEdit (c);
		}
	}

	void onPerformEdit (IValue& value, IValue::Type newValue) override
	{
		auto newControlValue = static_cast<float> (newValue);
		for (auto& c : controls)
		{
			if (c->getValueNormalized () != newControlValue)
			{
				c->setValueNormalized (newControlValue);
				c->invalid ();
			}
		}
	}

	void updateControlOnStateChange (CControl* control) const
	{
		control->setMin (0.f);
		control->setMax (1.f);
		control->setMouseEnabled (value->isActive ());
		auto stepValue = dynamicPtrCast<const IStepValue> (value);
		if (!stepValue)
			return;
		const auto& valueConverter = value->getConverter ();
		if (auto menu = dynamic_cast<COptionMenu*> (control))
		{
			menu->removeAllEntry ();
			for (IStepValue::StepType i = 0; i < stepValue->getSteps (); ++i)
			{
				auto title = valueConverter.valueAsString (stepValue->stepToValue (i));
				menu->addEntry (title);
			}
		}
		if (auto segmentButton = dynamic_cast<CSegmentButton*> (control))
		{
			segmentButton->removeAllSegments ();
			for (IStepValue::StepType i = 0; i < stepValue->getSteps (); ++i)
			{
				auto title = valueConverter.valueAsString (stepValue->stepToValue (i));
				segmentButton->addSegment ({title});
			}
		}
	}

	void onStateChange (IValue& value) override
	{
		for (auto& c : controls)
		{
			updateControlOnStateChange (c);
			c->valueChanged ();
		}
	}

	void valueChanged (CControl* control) override
	{
		auto preValue = static_cast<float> (value->getValue ());
		auto newValue = control->getValueNormalized ();
		if (preValue != newValue || dynamic_cast<CTextEdit*>(control))
			value->performEdit (newValue);
		else
			onPerformEdit (*value, value->getValue ());
	}

	void controlBeginEdit (CControl* control) override { value->beginEdit (); }
	void controlEndEdit (CControl* control) override { value->endEdit (); }

	void addControl (CControl* control)
	{
		if (auto paramDisplay = dynamic_cast<CParamDisplay*> (control))
		{
			paramDisplay->setValueToStringFunction2 (
			    [this] (float value, std::string& utf8String, CParamDisplay* display) {
				    utf8String = this->value->getConverter ().valueAsString (value);
				    return true;
				});
			if (auto textEdit = dynamic_cast<CTextEdit*> (paramDisplay))
			{
				textEdit->setStringToValueFunction (
				    [&] (UTF8StringPtr txt, float& result, CTextEdit* textEdit) {
					    auto v = value->getConverter ().stringAsValue (txt);
					    if (v == IValue::InvalidValue)
						    v = value->getValue ();
					    result = static_cast<float> (v);
					    return true;
					});
			}
		}
		updateControlOnStateChange (control);
		control->beginEdit ();
		control->setValueNormalized (static_cast<float> (value->getValue ()));
		control->valueChanged ();
		control->endEdit ();
		control->invalid ();
		control->registerControlListener (this);
		control->registerViewListener (this);
		controls.emplace_back (control);
	}

	void removeControl (CControl* control)
	{
		if (auto paramDisplay = dynamic_cast<CParamDisplay*> (control))
		{
			paramDisplay->setValueToStringFunction (nullptr);
			if (auto textEdit = dynamic_cast<CTextEdit*> (paramDisplay))
				textEdit->setStringToValueFunction (nullptr);
		}
		control->unregisterViewListener (this);
		control->unregisterControlListener (this);
		auto it = std::find (controls.begin (), controls.end (), control);
		vstgui_assert (it != controls.end ());
		controls.erase (it);
	}

	void viewWillDelete (CView* view) override { removeControl (dynamic_cast<CControl*> (view)); }

protected:
	using ControlList = std::vector<CControl*>;

	ValuePtr value;
	ControlList controls;
};

using ValueWrapperPtr = std::unique_ptr<ValueWrapper>;

//------------------------------------------------------------------------
struct WindowController::Impl : public IController, public ICommandHandler
{
	using ValueWrapperList = std::vector<ValueWrapperPtr>;

	Impl (WindowController& controller, const ModelBindingPtr& modelHandler,
	      const CustomizationPtr& customization)
	: controller (controller), modelBinding (modelHandler), customization (customization)
	{
		initModelValues (modelHandler);
	}

	~Impl () override
	{
		if (uiDesc && uiDesc->getSharedResources ())
		{
			uiDesc->setSharedResources (nullptr);
		}
	}

	virtual bool init (WindowPtr& inWindow, const char* fileName, const char* templateName)
	{
		window = inWindow.get ();
		if (!initUIDesc (fileName))
			return false;
		frame = makeOwned<CFrame> (CRect (), nullptr);
		frame->setTransparency (true);
		this->templateName = templateName;

		showView ();

		window->setContentView (frame);
		return true;
	}

	bool initStatic (WindowPtr& inWindow, UTF8String xml, const char* templateName)
	{
		window = inWindow.get ();
		Xml::MemoryContentProvider xmlContentProvider (xml, static_cast<uint32_t> (xml.length ()));
		uiDesc = makeOwned<UIDescription> (&xmlContentProvider);
		if (!uiDesc->parse ())
			return false;
		frame = makeOwned<CFrame> (CRect (), nullptr);
		frame->setTransparency (true);
		this->templateName = templateName;

		showView ();

		window->setContentView (frame);
		return true;
	}

	virtual CPoint constraintSize (const CPoint& newSize)
	{
		if (customization)
		{
			if (auto customController = dynamicPtrCast<IWindowController> (customization))
			{
				auto p = customController->constraintSize (*window, newSize);
				if (p != newSize)
					return p;
			}
		}
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

	virtual void beforeShow ()
	{
		if (customization)
		{
			if (auto customController = dynamicPtrCast<IWindowController> (customization))
				customController->beforeShow (*window);
		}
	}

	virtual bool canClose ()
	{
		if (customization)
		{
			if (auto customController = dynamicPtrCast<IWindowController> (customization))
				return customController->canClose (*window);
		}
		return true;
	}

	void onSetContentView (const SharedPointer<CFrame>& contentView)
	{
		if (customization)
		{
			if (auto customController = dynamicPtrCast<IWindowController> (customization))
				customController->onSetContentView (*window, contentView);
		}
	}

	virtual void onSizeChanged (const CPoint& newSize)
	{
		if (customization)
		{
			if (auto customController = dynamicPtrCast<IWindowController> (customization))
				customController->onSizeChanged (*window, newSize);
		}
	}

	void onPositionChanged (const CPoint& newPos)
	{
		if (customization)
		{
			if (auto customController = dynamicPtrCast<IWindowController> (customization))
				customController->onPositionChanged (*window, newPos);
		}
	}

	void onShow ()
	{
		if (customization)
		{
			if (auto customController = dynamicPtrCast<IWindowController> (customization))
				customController->onShow (*window);
		}
	}

	void onHide ()
	{
		if (customization)
		{
			if (auto customController = dynamicPtrCast<IWindowController> (customization))
				customController->onHide (*window);
		}
	}

	virtual void onClosed ()
	{
		if (customization)
		{
			if (auto customController = dynamicPtrCast<IWindowController> (customization))
				customController->onClosed (*window);
		}
	}

	void onActivated ()
	{
		if (customization)
		{
			if (auto customController = dynamicPtrCast<IWindowController> (customization))
				customController->onActivated (*window);
		}
	}

	void onDeactivated ()
	{
		if (customization)
		{
			if (auto customController = dynamicPtrCast<IWindowController> (customization))
				customController->onDeactivated (*window);
		}
	}

	bool initUIDesc (const char* fileName)
	{
		uiDesc = makeOwned<UIDescription> (fileName);
		uiDesc->setSharedResources (Detail::getSharedUIDescription ());
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
		auto viewSize = view->getViewSize ().getSize ();
		frame->getTransform ().transform (viewSize);
		frame->setSize (viewSize.x, viewSize.y);
		frame->addView (view);

		auto focusDrawing = uiDesc->getFocusDrawingSettings ();
		frame->setFocusDrawingEnabled (focusDrawing.enabled);
		if (focusDrawing.enabled)
		{
			frame->setFocusWidth (focusDrawing.width);
			CColor focusColor;
			if (uiDesc->getColor (focusDrawing.colorName.data (), focusColor))
				frame->setFocusColor (focusColor);
		}

		window->setSize (view->getViewSize ().getSize ());
	}

	bool canHandleCommand (const Command& command) override
	{
		if (modelBinding)
		{
			if (auto commandHandler = dynamicPtrCast<ICommandHandler> (modelBinding))
				return commandHandler->canHandleCommand (command);
		}
		if (customization)
		{
			if (auto commandHandler = dynamicPtrCast<ICommandHandler> (customization))
				return commandHandler->canHandleCommand (command);
		}
		return false;
	}

	bool handleCommand (const Command& command) override
	{
		if (modelBinding)
		{
			if (auto commandHandler = dynamicPtrCast<ICommandHandler> (modelBinding))
				return commandHandler->handleCommand (command);
		}
		if (customization)
		{
			if (auto commandHandler = dynamicPtrCast<ICommandHandler> (customization))
				return commandHandler->handleCommand (command);
		}
		return false;
	}

	void initModelValues (const ModelBindingPtr& modelHandler)
	{
		if (!modelHandler)
			return;
		valueWrappers.reserve (modelHandler->getValues ().size ());
		for (auto& value : modelHandler->getValues ())
		{
			valueWrappers.emplace_back (std::make_unique<ValueWrapper> (value));
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
		auto index = static_cast<ValueWrapperList::size_type> (control->getTag ());
		if (index < valueWrappers.size ())
			valueWrappers[index]->removeControl (control);
	}
	void controlTagDidChange (CControl* control) override
	{
		if (control->getTag () < 0)
			return;
		auto index = static_cast<ValueWrapperList::size_type> (control->getTag ());
		if (index < valueWrappers.size ())
			valueWrappers[index]->addControl (control);
	}

	int32_t getTagForName (UTF8StringPtr name, int32_t registeredTag) const override
	{
		auto it = std::find_if (valueWrappers.begin (), valueWrappers.end (),
		                        [&] (const ValueWrapperPtr& v) { return v->getID () == name; });
		if (it != valueWrappers.end ())
			return static_cast<int32_t> (std::distance (valueWrappers.begin (), it));
		return registeredTag;
	}

	IControlListener* getControlListener (UTF8StringPtr controlTagName) override { return this; }
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override
	{
		return nullptr;
	}
	CView* verifyView (CView* view, const UIAttributes& attributes,
	                   const IUIDescription* description) override
	{
		auto control = dynamic_cast<CControl*> (view);
		if (control)
		{
			auto index = static_cast<ValueWrapperList::size_type> (control->getTag ());
			if (index < valueWrappers.size ())
			{
				valueWrappers[index]->updateControlOnStateChange (control);
			}
		}
		return view;
	}
	IController* createSubController (UTF8StringPtr name,
	                                  const IUIDescription* description) override
	{
		if (customization)
			return customization->createController (name, this, description);
		return nullptr;
	}

	WindowController& controller;
	IWindow* window {nullptr};
	SharedPointer<VSTGUI::UIDescription> uiDesc;
	SharedPointer<CFrame> frame;
	UTF8String templateName;
	CPoint minSize;
	CPoint maxSize;
	ModelBindingPtr modelBinding;
	CustomizationPtr customization;
	ValueWrapperList valueWrappers;
};

#if VSTGUI_LIVE_EDITING
static const Command ToggleEditingCommand {"Debug", "Toggle Inline Editor"};

//------------------------------------------------------------------------
struct WindowController::EditImpl : WindowController::Impl
{
	EditImpl (WindowController& controller, const ModelBindingPtr& modelBinding,
	          const CustomizationPtr& customization)
	: Impl (controller, modelBinding, customization)
	{
		IApplication::instance ().registerCommand (ToggleEditingCommand, 'E');
	}

	bool init (WindowPtr& inWindow, const char* fileName, const char* templateName) override
	{
		this->filename = fileName;
		if (auto absPath = Detail::getEditFileMap ().get (fileName))
			fileName = *absPath;
		window = inWindow.get ();
		frame = makeOwned<CFrame> (CRect (), nullptr);
		frame->setTransparency (true);

		if (!initUIDesc (fileName))
		{
			UIAttributes* attr = new UIAttributes ();
			attr->setAttribute (UIViewCreator::kAttrClass, "CViewContainer");
			attr->setAttribute (UIViewCreator::kAttrSize, "300, 300");
			attr->setAttribute (UIViewCreator::kAttrAutosize, "left right top bottom");
			uiDesc->addNewTemplate (templateName, attr);

			initAsNew ();
		}
		else
		{
			auto settings = uiDesc->getCustomAttributes ("UIDescFilePath", true);
			auto filePath = settings->getAttributeValue ("path");
			if (filePath)
				uiDesc->setFilePath (filePath->data ());
		}
		this->templateName = templateName;

		syncTags ();
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

	void onSizeChanged (const CPoint& newSize) override
	{
		if (isEditing)
			return;
		Impl::onSizeChanged (newSize);
	}

	void onClosed () override
	{
		enableEditing (false);
		Impl::onClosed ();
	}

	bool canClose () override { return Impl::canClose (); }

	void initAsNew ()
	{
		Detail::PreventPopupClose ppc (*window);

		if (Detail::initUIDescAsNew (*uiDesc, frame))
		{
			enableEditing (true, true);
			save (true);
			enableEditing (false);
		}
		else
		{
			Async::perform (Async::Context::Main, [this] () { window->close (); });
		}
	}

	void checkFileExists ()
	{
		Detail::PreventPopupClose ppc (*window);

		auto result = Detail::checkAndUpdateUIDescFilePath (*uiDesc, frame);
		if (result == Detail::UIDescCheckFilePathResult::Exists)
			return;
		if (result == Detail::UIDescCheckFilePathResult::NewPathSet)
		{
			save (true);
			return;
		}
		enableEditing (false);
	}

	void save (bool force = false)
	{
		if (!uiEditController)
			return;
		if (force || uiEditController->getUndoManager ()->isSavePosition () == false)
		{
			if (uiEditController->getUndoManager ()->isSavePosition () == false)
				Detail::saveSharedUIDescription ();
			if (!uiDesc->save (uiDesc->getFilePath (), UIDescription::kWriteImagesIntoXMLFile))
			{
				AlertBoxConfig config;
				config.headline = "Saving the uidesc file failed.";
				IApplication::instance ().showAlertBox (config);
			}
			else
				Detail::getEditFileMap ().set (filename, uiDesc->getFilePath ());
		}
	}

	void syncTags ()
	{
		std::list<const std::string*> tagNames;
		uiDesc->collectControlTagNames (tagNames);
		int32_t index = 0;
		for (auto& v : valueWrappers)
		{
			auto it = std::find_if (tagNames.begin (), tagNames.end (),
			                        [&] (const std::string* name) { return v->getID () == *name; });
			auto create = it == tagNames.end ();
			uiDesc->changeControlTagString (v->getID (), std::to_string (index), create);
			++index;
		}
		// now remove all old tags
		for (auto& name : tagNames)
		{
			auto it = std::find_if (valueWrappers.begin (), valueWrappers.end (),
			                        [&] (const ValueWrapperList::value_type& value) {
				                        return value->getID () == *name;
				                    });
			if (it == valueWrappers.end ())
				uiDesc->removeTag (name->data ());
		}
	}

	void enableEditing (bool state, bool ignoreCheckFileExist = false)
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
			auto viewSize = view->getViewSize ().getSize ();
			frame->getTransform ().transform (viewSize);
			frame->setSize (viewSize.x, viewSize.y);
			frame->addView (view);
			frame->enableTooltips (true);
			CColor focusColor = kBlueCColor;
			uiEditController->getEditorDescription ()->getColor ("focus", focusColor);
			frame->setFocusColor (focusColor);
			frame->setFocusDrawingEnabled (true);
			frame->setFocusWidth (1);
			window->setSize (view->getViewSize ().getSize ());
			if (auto menuController = uiEditController->getMenuController ())
			{
				if (auto menu = menuController->getFileMenu ())
				{
					menu->removeAllEntry ();
					menu->setMouseEnabled (false);
				}
			}
			if (!ignoreCheckFileExist)
				checkFileExists ();
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
		else if (uiEditController &&
		         uiEditController->getMenuController ()->canHandleCommand (command.group,
		                                                                   command.name))
			return true;
		return Impl::canHandleCommand (command);
	}

	bool handleCommand (const Command& command) override
	{
		if (command == ToggleEditingCommand)
		{
			enableEditing (!isEditing);
			return true;
		}
		else if (uiEditController &&
		         uiEditController->getMenuController ()->handleCommand (command.group,
		                                                                command.name))
			return true;
		return Impl::handleCommand (command);
	}

	SharedPointer<UIEditController> uiEditController;
	bool isEditing {false};
	std::string filename;
};
#endif

//------------------------------------------------------------------------
bool WindowController::initStatic (const UIDesc::Config& config, WindowPtr& window)
{
	impl = std::unique_ptr<Impl> (new Impl (*this, config.modelBinding, config.customization));
	return impl->initStatic (window, config.uiDescFileName, config.viewName);
}

//------------------------------------------------------------------------
bool WindowController::init (const UIDesc::Config& config, WindowPtr& window)
{
#if VSTGUI_LIVE_EDITING
	impl = std::unique_ptr<Impl> (new EditImpl (*this, config.modelBinding, config.customization));
#else
	impl = std::unique_ptr<Impl> (new Impl (*this, config.modelBinding, config.customization));
#endif
	return impl->init (window, config.uiDescFileName, config.viewName);
}

//------------------------------------------------------------------------
CPoint WindowController::constraintSize (const IWindow& window, const CPoint& newSize)
{
	return impl ? impl->constraintSize (newSize) : newSize;
}

//------------------------------------------------------------------------
bool WindowController::canClose (const IWindow& window)
{
	return impl ? impl->canClose () : true;
}

//------------------------------------------------------------------------
void WindowController::onClosed (const IWindow& window)
{
	if (impl)
		impl->onClosed ();
	impl = nullptr;
}

//------------------------------------------------------------------------
void WindowController::beforeShow (IWindow& window)
{
	if (impl)
		impl->beforeShow ();
}

//------------------------------------------------------------------------
void WindowController::onSetContentView (IWindow& window, const SharedPointer<CFrame>& contentView)
{
	if (impl)
		impl->onSetContentView (contentView);
}

//------------------------------------------------------------------------
void WindowController::onSizeChanged (const IWindow& window, const CPoint& newSize)
{
	if (impl)
		impl->onSizeChanged (newSize);
}

//------------------------------------------------------------------------
void WindowController::onPositionChanged (const IWindow& window, const CPoint& newPosition)
{
	if (impl)
		impl->onPositionChanged (newPosition);
}

//------------------------------------------------------------------------
void WindowController::onShow (const IWindow& window)
{
	if (impl)
		impl->onShow ();
}

//------------------------------------------------------------------------
void WindowController::onHide (const IWindow& window)
{
	if (impl)
		impl->onHide ();
}

//------------------------------------------------------------------------
void WindowController::onActivated (const IWindow& window)
{
	if (impl)
		impl->onActivated ();
}

//------------------------------------------------------------------------
void WindowController::onDeactivated (const IWindow& window)
{
	if (impl)
		impl->onDeactivated ();
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
IMenuBuilder* WindowController::getWindowMenuBuilder (const IWindow& window) const
{
	if (auto menuBuilder = dynamicPtrCast<IMenuBuilder> (impl->customization))
		return menuBuilder.get ();
	return nullptr;
}

//------------------------------------------------------------------------
} // Anonymous

//------------------------------------------------------------------------
namespace UIDesc {

static const UTF8StringView staticXmlIdentifier ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");

//------------------------------------------------------------------------
WindowPtr makeWindow (const Config& config)
{
	vstgui_assert (config.viewName.empty () == false);
	vstgui_assert (config.uiDescFileName.empty () == false);

	auto controller = std::make_shared<WindowController> ();

	if (UTF8StringView (config.uiDescFileName).startsWith (staticXmlIdentifier))
	{
		auto window = IApplication::instance ().createWindow (config.windowConfig, controller);
		if (!window)
			return nullptr;

		if (!controller->initStatic (config, window))
			return nullptr;

		return window;
	}

#if VSTGUI_LIVE_EDITING
	WindowConfiguration windowConfig = config.windowConfig;
	windowConfig.style.size ();
#else
	auto& windowConfig = config.windowConfig;
#endif

	auto window = IApplication::instance ().createWindow (windowConfig, controller);
	if (!window)
		return nullptr;

	if (!controller->init (config, window))
		return nullptr;

	return window;
}

//------------------------------------------------------------------------
} // UIDesc
} // Standalone
} // VSTGUI
