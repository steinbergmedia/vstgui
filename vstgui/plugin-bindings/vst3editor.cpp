// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "vst3editor.h"
#include "../vstgui.h"
#include "../lib/cvstguitimer.h"
#include "../lib/animation/timingfunctions.h"
#include "../lib/animation/animations.h"
#include "../lib/platform/platformfactory.h"
#include "../uidescription/detail/uiviewcreatorattributes.h"
#include "../uidescription/editing/uieditcontroller.h"
#include "../uidescription/editing/uieditmenucontroller.h"
#include "../uidescription/uiattributes.h"
#include "../uidescription/uiviewfactory.h"
#include "../uidescription/cstream.h"
#include "base/source/fstring.h"
#include "base/source/updatehandler.h"
#include "pluginterfaces/base/keycodes.h"
#include <algorithm>
#include <cassert>
#include <list>
#include <sstream>

#if LINUX
#include "../lib/platform/linux/x11frame.h"
#include "../lib/platform/linux/waylandframe.h"
#include "pluginterfaces/gui/iplugview.h"
#include "pluginterfaces/gui/iwaylandframe.h"
DEF_CLASS_IID (Steinberg::IWaylandFrame);
DEF_CLASS_IID (Steinberg::IWaylandHost);
#endif

#if defined(kVstVersionMajor) && defined(kVstVersionMinor)
#define VST3_SUPPORTS_CONTEXTMENU \
	(kVstVersionMajor > 3 || (kVstVersionMajor == 3 && kVstVersionMinor > 1))
#if VST3_SUPPORTS_CONTEXTMENU
#include "pluginterfaces/vst/ivstcontextmenu.h"
#endif
#else
#define VST3_SUPPORTS_CONTEXTMENU 0
#endif

/// @cond ignore
namespace Steinberg {

//-----------------------------------------------------------------------------
class UpdateHandlerInit
{
public:
	UpdateHandlerInit () { get (); }

	UpdateHandler* get () { return UpdateHandler::instance (); }
};

static UpdateHandlerInit gUpdateHandlerInit;

//-----------------------------------------------------------------------------
class IdleUpdateHandler
{
public:
	static void start ()
	{
		auto& instance = get ();
		if (++instance.users == 1)
		{
			instance.timer = VSTGUI::makeShared<VSTGUI::CVSTGUITimer> (
				[] (VSTGUI::CVSTGUITimer*) { gUpdateHandlerInit.get ()->triggerDeferedUpdates (); },
				1000 / 30);
		}
	}

	static void stop ()
	{
		auto& instance = get ();
		if (--instance.users == 0)
		{
			instance.timer = nullptr;
		}
	}

protected:
	static IdleUpdateHandler& get ()
	{
		static IdleUpdateHandler gInstance;
		return gInstance;
	}

	VSTGUI::SharedPointer<VSTGUI::CVSTGUITimer> timer;
	std::atomic<uint32_t> users {0};
};

} // namespace Steinberg
/// @endcond ignore

namespace VSTGUI {

//-----------------------------------------------------------------------------
class ParameterChangeListener : public Steinberg::FObject,
								public ViewListenerAdapter
{
public:
	ParameterChangeListener (Steinberg::Vst::EditController* editController, Steinberg::Vst::Parameter* parameter, CControl* control)
	: editController (editController)
	, parameter (parameter)
	{
		if (parameter)
		{
			parameter->addRef ();
			parameter->addDependent (this);
		}
		addControl (control);
		if (parameter)
			parameter->changed ();
	}

	~ParameterChangeListener () override
	{
		for (const auto& c : controls)
			c->unregisterViewListener (this);
		if (parameter)
		{
			parameter->removeDependent (this);
			parameter->release ();
		}
	}

	void addControl (CControl* control)
	{
		if (containsControl (control))
			return;
		control->registerViewListener (this);
		controls.push_back (control);
		Steinberg::Vst::ParamValue value = 0.;
		if (parameter)
		{
			value = editController->getParamNormalized (getParameterID ());
		}
		else
		{
			if (auto ctrl = controls.front ())
				value = ctrl->getValueNormalized ();
		}
		auto* display = dynamic_cast<CParamDisplay*> (control);
		if (display)
		{
			display->setValueToStringFunction ([this] (float value, char utf8String[256], auto&) {
				return convertValueToString (value, utf8String);
			});
		}
		if (parameter)
			parameter->deferUpdate ();
		else
			updateControlValue (value);
	}

	void removeControl (CControl* control)
	{
		for (const auto& c : controls)
		{
			if (c == control)
			{
				control->unregisterViewListener (this);
				controls.remove (control);
				return;
			}
		}
	}

	bool containsControl (CControl* control)
	{
		return std::find (controls.begin (), controls.end (), control) != controls.end ();
	}

	void PLUGIN_API update (FUnknown* changedUnknown, Steinberg::int32 message) override
	{
		if (message == IDependent::kChanged && parameter)
		{
			updateControlValue (editController->getParamNormalized (getParameterID ()));
		}
	}

	Steinberg::Vst::ParamID getParameterID ()
	{
		if (parameter)
			return parameter->getInfo ().id;
		CControl* control = controls.front ();
		if (control)
			return static_cast<Steinberg::Vst::ParamID> (control->getTag ());
		return 0xFFFFFFFF;
	}

	void beginEdit ()
	{
		if (parameter)
			editController->beginEdit (getParameterID ());
	}

	void endEdit ()
	{
		if (parameter)
			editController->endEdit (getParameterID ());

		// fix textual representation.
		// It can happen that a parameter edit does not change the normalized value but the textual
		// representation shows a wrong text because the text is only translated when the
		// normalized value changes.
		Steinberg::Vst::String128 str {};
		for (const auto& c : controls)
		{
			if (auto label = dynamic_cast<CTextLabel*> (c))
			{
				if (str[0] == 0)
				{
					editController->getParamStringByValue (
					    getParameterID (), editController->getParamNormalized (getParameterID ()),
					    str);
				}
				Steinberg::String s (str);
				s.toMultiByte (Steinberg::kCP_Utf8);
				if (label->getText () != s.text8 ())
					label->setText (s.text8 ());
			}
		}
	}

	void performEdit (Steinberg::Vst::ParamValue value)
	{
		if (parameter)
		{
			auto id = getParameterID ();
			if (editController->setParamNormalized (id, value) == Steinberg::kResultTrue)
				editController->performEdit (id, editController->getParamNormalized (id));
		}
		else
		{
			updateControlValue (value);
		}
	}
	Steinberg::Vst::Parameter* getParameter () const { return parameter; }

protected:
	void viewWillDelete (CView& view) override
	{
		if (auto control = dynamic_cast<CControl*> (&view))
			removeControl (control);
	}

	bool convertValueToString (float value, char utf8String[256])
	{
		if (parameter)
		{
			Steinberg::Vst::String128 utf16Str;
			if (parameter && parameter->getInfo ().stepCount)
			{
				// convert back to normalized value
				value = (float)editController->plainParamToNormalized (getParameterID (), (Steinberg::Vst::ParamValue)value);
			}
			editController->getParamStringByValue (getParameterID (), value, utf16Str);
			Steinberg::String utf8Str (utf16Str);
			utf8Str.toMultiByte (Steinberg::kCP_Utf8);
			utf8Str.copyTo8 (utf8String, 0, 256);
			return true;
		}
		return false;
	}

	void updateControlValue (Steinberg::Vst::ParamValue value)
	{
		bool mouseEnabled = true;
		bool isStepCount = false;
		Steinberg::Vst::ParamValue defaultValue = 0.5;
		float minValue = 0.f;
		float maxValue = 1.f;
		if (parameter)
		{
			defaultValue = parameter->getInfo ().defaultNormalizedValue;
			if (parameter->getInfo ().flags & Steinberg::Vst::ParameterInfo::kIsReadOnly)
				mouseEnabled = false;
			if (parameter->getInfo ().stepCount)
			{
				isStepCount = true;
				value = parameter->toPlain (value);
				defaultValue = parameter->toPlain (defaultValue);
				minValue = (float)parameter->toPlain ((Steinberg::Vst::ParamValue)minValue);
				maxValue = (float)parameter->toPlain ((Steinberg::Vst::ParamValue)maxValue);
			}
		}
		for (const auto& c : controls)
		{
			c->setMouseEnabled (mouseEnabled);
			if (parameter)
			{
				c->setDefaultValue ((float)defaultValue);
				c->setMin (minValue);
				c->setMax (maxValue);
			}
			auto* label = dynamic_cast<CTextLabel*>(c);
			if (label)
			{
				Steinberg::Vst::ParamValue normValue = value;
				if (isStepCount)
				{
					normValue = parameter->toNormalized (value);
				}
				Steinberg::Vst::String128 utf16Str;
				if (editController->getParamStringByValue (getParameterID (), normValue, utf16Str) != Steinberg::kResultTrue)
					continue;
				Steinberg::String utf8Str (utf16Str);
				utf8Str.toMultiByte (Steinberg::kCP_Utf8);
				label->setText (utf8Str.text8 ());
			}
			else
			{
				if (isStepCount)
				{
					c->setMin (minValue);
					c->setMax (maxValue);

					auto getParamStringByIndex = [&] (int32_t i) {
						Steinberg::Vst::String128 utf16Str;
						editController->getParamStringByValue (
							getParameterID (),
							(Steinberg::Vst::ParamValue)i /
								(Steinberg::Vst::ParamValue)parameter->getInfo ().stepCount,
							utf16Str);
						Steinberg::String utf8Str (utf16Str);
						utf8Str.toMultiByte (Steinberg::kCP_Utf8);
						return utf8Str;
					};

					if (auto optMenu = dynamic_cast<COptionMenu*> (c))
					{
						optMenu->removeAllEntry ();
						for (Steinberg::int32 i = 0; i <= parameter->getInfo ().stepCount; i++)
							optMenu->addEntry (getParamStringByIndex (i).text8 ());
						c->setValue ((float)value - minValue);
					}
					else if (auto segmentButton = dynamic_cast<CSegmentButton*> (c))
					{
						segmentButton->removeAllSegments ();
						for (Steinberg::int32 i = 0; i <= parameter->getInfo ().stepCount; i++)
						{
							CSegmentButton::Segment segment;
							segment.name = getParamStringByIndex (i).text8 ();
							segmentButton->addSegment (std::move (segment));
						}
						c->setValue ((float)value);
					}
					else
					{
						c->setValue ((float)value);
					}
				}
				else
					c->setValueNormalized ((float)value);
				c->valueChanged ();
			}
			c->invalid ();
		}
	}
	Steinberg::Vst::EditController* editController;
	Steinberg::Vst::Parameter* parameter;

	using ControlList = std::list<CControl*>;
	ControlList controls;
};

namespace VST3EditorInternal {

//-----------------------------------------------------------------------------
static bool parseSize (const std::string& str, CPoint& point)
{
	size_t sep = str.find (',', 0);
	if (sep != std::string::npos)
	{
		point.x = strtol (str.c_str (), nullptr, 10);
		point.y = strtol (str.c_str () + sep+1, nullptr, 10);
		return true;
	}
	return false;
}

#if VST3_SUPPORTS_CONTEXTMENU

//-----------------------------------------------------------------------------
class ContextMenuTarget : public Steinberg::FObject,
						  public Steinberg::Vst::IContextMenuTarget
{
public:
	ContextMenuTarget (const SharedPointer<CCommandMenuItem>& item) : item (item) {}

	Steinberg::tresult PLUGIN_API executeMenuItem (Steinberg::int32 tag) override
	{
		item->execute ();
		return Steinberg::kResultTrue;
	}

	OBJ_METHODS (ContextMenuTarget, Steinberg::FObject)
	FUNKNOWN_METHODS (Steinberg::Vst::IContextMenuTarget, Steinberg::FObject)
protected:
	SharedPointer<CCommandMenuItem> item;
};

//-----------------------------------------------------------------------------
static void addCOptionMenuEntriesToIContextMenu (
	VST3Editor* editor, const SharedPointer<COptionMenu>& menu,
	const Steinberg::IPtr<Steinberg::Vst::IContextMenu>& contextMenu)
{
	for (auto it = menu->getItemList ().begin (), end = menu->getItemList ().end (); it != end;
		 ++it)
	{
		auto commandItem = (*it).cast<CCommandMenuItem> ();
		if (commandItem)
			commandItem->validate ();

		Steinberg::Vst::IContextMenu::Item item = {};
		Steinberg::String title ((*it)->getTitle ());
		title.toWideString (Steinberg::kCP_Utf8);
		title.copyTo16 (item.name, 0, 128);
		if ((*it)->getSubmenu ())
		{
			item.flags = Steinberg::Vst::IContextMenu::Item::kIsGroupStart;
			contextMenu->addItem (item, nullptr);
			addCOptionMenuEntriesToIContextMenu (editor, (*it)->getSubmenu (), contextMenu);
			item.flags = Steinberg::Vst::IContextMenu::Item::kIsGroupEnd;
			contextMenu->addItem (item, nullptr);
		}
		else if ((*it)->isSeparator ())
		{
			item.flags = Steinberg::Vst::IContextMenu::Item::kIsSeparator;
			contextMenu->addItem (item, nullptr);
		}
		else
		{
			if (commandItem)
			{
				if ((*it)->isChecked ())
					item.flags |= Steinberg::Vst::IContextMenu::Item::kIsChecked;
				if ((*it)->isEnabled () == false)
					item.flags |= Steinberg::Vst::IContextMenu::Item::kIsDisabled;
				auto* target = new ContextMenuTarget (commandItem);
				contextMenu->addItem (item, target);
				target->release ();
			}
		}
	}
}

#endif

} // namespace VST3EditorInternal

//------------------------------------------------------------------------
struct VST3Editor::Controller : ControllerAdapter,
								CommandMenuItemTargetAdapter,
								IMouseObserver
{
	Controller (VST3Editor* editor) : editor (editor) {}

	SharedPointer<CView> createView (const UIAttributes& attributes,
									 const IUIDescription& description) override;
	SharedPointer<CView> verifyView (const SharedPointer<CView>& view,
									 const UIAttributes& attributes,
									 const IUIDescription& description) override;
	SharedPointer<IController> createSubController (UTF8StringPtr name,
													const IUIDescription& description) override;

	void valueChanged (CControl& control) override;
	void controlBeginEdit (CControl& control) override;
	void controlEndEdit (CControl& control) override;
	void controlTagWillChange (CControl& control) override;
	void controlTagDidChange (CControl& control) override;

	// CommandMenuItemTargetAdapter
	bool validateCommandMenuItem (CCommandMenuItem& item) override;
	bool onCommandMenuItemSelected (CCommandMenuItem& item) override;

	// IMouseObserver
	void onMouseEntered (CView& view, CFrame& frame) override {}
	void onMouseExited (CView& view, CFrame& frame) override {}
	void onMouseEvent (MouseEvent& event, CFrame& frame) override;

	VST3Editor* editor {nullptr};
};

//------------------------------------------------------------------------
struct VST3Editor::Impl
{
	struct KeyboardHook;
	KeyboardHook* keyboardHook {nullptr};
	SharedPointer<UIDescription> description;
	IVST3EditorDelegate* delegate {nullptr};
	IController* originalController {nullptr};
	struct EnterEditModeController;
	SharedPointer<EnterEditModeController> openUIEditorController;
	SharedPointer<Controller> controller;
	using ParameterChangeListenerMap = std::map<int32_t, ParameterChangeListener*>;
	ParameterChangeListenerMap paramChangeListeners;
	std::string viewName;
	std::string xmlFile;
	bool tooltipsEnabled {true};
	bool doCreateView {false};
	bool editingEnabled {false};

	double contentScaleFactor {1.};
	double zoomFactor {1.};
	std::vector<double> allowedZoomFactors;

	CPoint minSize;
	CPoint maxSize;
	CRect nonEditRect;

	Optional<CPoint> sizeRequest;
};

//-----------------------------------------------------------------------------
void VST3Editor::Controller::valueChanged (CControl& control)
{
	using namespace Steinberg;
	if (!control.isEditing ())
		return;

	auto pcl = editor->getParameterChangeListener (control.getTag ());
	if (pcl)
	{
		auto paramID = pcl->getParameterID ();
		auto normalizedValue = static_cast<Vst::ParamValue> (control.getValueNormalized ());
		auto* textEdit = dynamic_cast<CTextEdit*> (&control);
		if (textEdit && pcl->getParameter ())
		{
			Steinberg::String str (textEdit->getText ());
			str.toWideString (kCP_Utf8);
			if (editor->getController ()->getParamValueByString (
					paramID, const_cast<Vst::TChar*> (str.text16 ()), normalizedValue) !=
				kResultTrue)
			{
				pcl->update (nullptr, kChanged);
				return;
			}
		}
		pcl->performEdit (normalizedValue);
	}
}

//-----------------------------------------------------------------------------
void VST3Editor::Controller::controlBeginEdit (CControl& control)
{
	auto pcl = editor->getParameterChangeListener (control.getTag ());
	if (pcl)
	{
		pcl->beginEdit ();
	}
}

//-----------------------------------------------------------------------------
void VST3Editor::Controller::controlEndEdit (CControl& control)
{
	auto pcl = editor->getParameterChangeListener (control.getTag ());
	if (pcl)
	{
		pcl->endEdit ();
	}
}

//-----------------------------------------------------------------------------
void VST3Editor::Controller::controlTagWillChange (CControl& control)
{
	if (control.getTag () != -1 && control.getListener () == this)
	{
		auto pcl = editor->getParameterChangeListener (control.getTag ());
		if (pcl)
		{
			pcl->removeControl (&control);
		}
	}
}

//-----------------------------------------------------------------------------
void VST3Editor::Controller::controlTagDidChange (CControl& control)
{
	if (control.getTag () != -1 && control.getListener () == this)
	{
		auto pcl = editor->getParameterChangeListener (control.getTag ());
		if (pcl)
		{
			pcl->addControl (&control);
		}
		else
		{
			auto editController = editor->getController ();
			if (editController)
			{
				Steinberg::Vst::Parameter* parameter = editController->getParameterObject (
					static_cast<Steinberg::Vst::ParamID> (control.getTag ()));
				editor->pImpl->paramChangeListeners.insert (std::make_pair (
					control.getTag (),
					new ParameterChangeListener (editController, parameter, &control)));
			}
		}
	}
}

//-----------------------------------------------------------------------------
SharedPointer<IController> VST3Editor::Controller::createSubController (UTF8StringPtr name,
																		const IUIDescription& desc)
{
	return editor->pImpl->delegate
			   ? editor->pImpl->delegate->createSubController (name, desc, *editor)
			   : nullptr;
}

//-----------------------------------------------------------------------------
SharedPointer<CView> VST3Editor::Controller::createView (const UIAttributes& attrs,
														 const IUIDescription& desc)
{
	if (editor->pImpl->delegate)
	{
		auto customViewName = attrs.getAttributeValue (IUIDescription::kCustomViewName);
		if (customViewName)
		{
			auto view = editor->pImpl->delegate->createCustomView (customViewName->c_str (), attrs,
																   desc, *editor);
			return view;
		}
	}
	return {};
}

//-----------------------------------------------------------------------------
SharedPointer<CView> VST3Editor::Controller::verifyView (const SharedPointer<CView>& view,
														 const UIAttributes& attributes,
														 const IUIDescription& desc)
{
	SharedPointer<CView> result = view;
	if (editor->pImpl->delegate)
		result = editor->pImpl->delegate->verifyView (result, attributes, desc, *editor);
	auto control = result.cast<CControl> ();
	if (control && control->getTag () != -1 && control->getListener () == this)
	{
		auto pcl = editor->getParameterChangeListener (control->getTag ());
		if (pcl)
		{
			pcl->addControl (control.get ());
		}
		else
		{
			auto editController = editor->getController ();
			if (editController)
			{
				Steinberg::Vst::Parameter* parameter = editController->getParameterObject (
					static_cast<Steinberg::Vst::ParamID> (control->getTag ()));
				editor->pImpl->paramChangeListeners.insert (std::make_pair (
					control->getTag (),
					new ParameterChangeListener (editController, parameter, control.get ())));
			}
		}
	}
	return result;
}

//------------------------------------------------------------------------
bool VST3Editor::Controller::validateCommandMenuItem (CCommandMenuItem& item)
{
#if VSTGUI_LIVE_EDITING
	if (item.getCommandCategory () == "File")
	{
		if (item.getCommandName () == "Save")
		{
			bool enable = false;
			auto attributes = editor->pImpl->description->getCustomAttributes ("VST3Editor", true);
			if (attributes)
			{
				const std::string* filePath = attributes->getAttributeValue ("Path");
				if (filePath)
				{
					enable = true;
				}
			}
			item.setEnabled (enable);
			return true;
		}
	}
#endif
	return false;
}

//------------------------------------------------------------------------
bool VST3Editor::Controller::onCommandMenuItemSelected (CCommandMenuItem& item)
{
	auto& cmdCategory = item.getCommandCategory ();
#if VSTGUI_LIVE_EDITING
	auto& cmdName = item.getCommandName ();
	if (cmdCategory == "Edit")
	{
		if (cmdName == "Sync Parameter Tags")
		{
			editor->syncParameterTags ();
			return true;
		}
	}
	else if (cmdCategory == "File")
	{
		if (cmdName == "Open UIDescription Editor")
		{
			editor->pImpl->editingEnabled = true;
			editor->requestRecreateView ();
			return true;
		}
		else if (cmdName == "Close UIDescription Editor")
		{
			editor->pImpl->editingEnabled = false;
			editor->requestRecreateView ();
			return true;
		}
		else if (cmdName == "Save")
		{
			editor->save (false);
			item.setChecked (false);
			return true;
		}
		else if (cmdName == "Save As")
		{
			editor->save (true);
			item.setChecked (false);
			return true;
		}
		else if (cmdName == "Save Editor Screenshot")
		{
			editor->saveScreenshot ();
			return true;
		}
		else if (cmdName == "Show Editor Button")
		{
			auto state = editor->enableShowEditButton ();
			editor->enableShowEditButton (!state);
			if (!editor->pImpl->editingEnabled)
				editor->showEditButton (!state);
			return true;
		}
	}
	else
#endif
		if (cmdCategory == "Zoom")
	{
		size_t index = static_cast<size_t> (item.getTag ());
		if (index < editor->pImpl->allowedZoomFactors.size ())
		{
			editor->setZoomFactor (editor->pImpl->allowedZoomFactors[index]);
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void VST3Editor::Controller::onMouseEvent (MouseEvent& event, CFrame& frame)
{
	if (event.type != EventType::MouseDown)
		return;

	if (event.buttonState.isRight ())
	{
		SharedPointer<COptionMenu> controllerMenu =
			(editor->pImpl->delegate && editor->pImpl->editingEnabled == false)
				? editor->pImpl->delegate->createContextMenu (event.mousePosition, *editor)
				: nullptr;
		if (editor->pImpl->allowedZoomFactors.empty () == false &&
			editor->pImpl->editingEnabled == false)
		{
			if (controllerMenu == nullptr)
				controllerMenu = VSTGUI::makeShared<COptionMenu> ();
			else
				controllerMenu->addSeparator ();
			auto zoomMenu = makeShared<COptionMenu> ();
			zoomMenu->setStyle (COptionMenu::kMultipleCheckStyle);
			char zoomFactorString[128];
			int32_t zoomFactorTag = 0;
			for (auto it = editor->pImpl->allowedZoomFactors.begin (),
					  end = editor->pImpl->allowedZoomFactors.end ();
				 it != end; ++it, ++zoomFactorTag)
			{
				snprintf (zoomFactorString, std::size (zoomFactorString), "%d%%",
						  static_cast<int> ((*it) * 100));
				auto item = zoomMenu->addEntry (VSTGUI::makeShared<CCommandMenuItem> (
					CCommandMenuItem::Desc {zoomFactorString, zoomFactorTag,
											editor->pImpl->controller, "Zoom", zoomFactorString}));
				if (editor->getZoomFactor () == *it)
					item->setChecked (true);
			}
			auto item = controllerMenu->addEntry ("UI Zoom");
			item->setSubmenu (zoomMenu);
		}
#if VSTGUI_LIVE_EDITING
		if (editor->pImpl->editingEnabled == false)
		{
			if (controllerMenu == nullptr)
				controllerMenu = VSTGUI::makeShared<COptionMenu> ();
			else
				controllerMenu->addSeparator ();
			auto item = controllerMenu->addEntry (VSTGUI::makeShared<CCommandMenuItem> (
				CCommandMenuItem::Desc {"Open UIDescription Editor", editor->pImpl->controller,
										"File", "Open UIDescription Editor"}));
			item->setKey ("e", kControl);
			item = controllerMenu->addEntry (VSTGUI::makeShared<CCommandMenuItem> (
				CCommandMenuItem::Desc {"Show 'Open UI Editor' Button", editor->pImpl->controller,
										"File", "Show Editor Button"}));
			if (editor->enableShowEditButton ())
				item->setChecked ();
			item = controllerMenu->addEntry (VSTGUI::makeShared<CCommandMenuItem> (
				CCommandMenuItem::Desc {"Save Editor Screenshot", editor->pImpl->controller, "File",
										"Save Editor Screenshot"}));
		}
#endif
		CViewContainer::ViewList views;
		auto nonScaledPos = event.mousePosition;
		frame.getTransform ().transform (nonScaledPos);
		if (frame.getViewsAt (nonScaledPos, views,
							  GetViewOptions ().deep ().includeViewContainer ()))
		{
			auto createOrPrepareMenu = [&] () {
				if (controllerMenu == nullptr)
					controllerMenu = VSTGUI::makeShared<COptionMenu> ();
				else
					controllerMenu->addSeparator ();
			};
			for (const auto& view : views)
			{
				auto viewController = getViewController (*view);
				if (!viewController)
					continue;
				if (auto ctrler = viewController.cast<IContextMenuController2> ())
				{
					createOrPrepareMenu ();
					ctrler->appendContextMenuItems (*controllerMenu, *view,
													view->translateToLocal (nonScaledPos));
				}
				else if (auto contextMenuController =
							 viewController.cast<IContextMenuController> ())
				{
					createOrPrepareMenu ();
					contextMenuController->appendContextMenuItems (
						*controllerMenu, view->translateToLocal (nonScaledPos));
				}
			}
		}
#if VST3_SUPPORTS_CONTEXTMENU
		Steinberg::FUnknownPtr<Steinberg::Vst::IComponentHandler3> handler (
			editor->getController ()->getComponentHandler ());
		Steinberg::Vst::ParamID paramID;
		if (handler)
		{
			CPoint where2 (event.mousePosition);
			frame.getTransform ().transform (where2);
			bool paramFound =
				editor->findParameter ((Steinberg::int32)where2.x, (Steinberg::int32)where2.y,
									   paramID) == Steinberg::kResultTrue;
			auto contextMenu = Steinberg::owned (
				handler->createContextMenu (editor, paramFound ? &paramID : nullptr));
			if (contextMenu)
			{
				if (controllerMenu)
					VST3EditorInternal::addCOptionMenuEntriesToIContextMenu (editor, controllerMenu,
																			 contextMenu);
				frame.doAfterEventProcessing ([contextMenu, where2] () {
					contextMenu->popup (static_cast<Steinberg::UCoord> (where2.x),
										static_cast<Steinberg::UCoord> (where2.y));
				});
				event.consumed = true;
			}
		}
		if (!event.consumed)
		{
#endif
			if (controllerMenu && controllerMenu->getNbEntries () > 0)
			{
				frame.doAfterEventProcessing ([controllerMenu, blockFrame = shared (&frame),
											   mousePosition = event.mousePosition] () {
					controllerMenu->setStyle (COptionMenu::kPopupStyle |
											  COptionMenu::kMultipleCheckStyle);
					controllerMenu->popup (*blockFrame, mousePosition);
				});
				event.consumed = true;
			}
		}
	}
}

//-----------------------------------------------------------------------------
/*! @class VST3Editor
The VST3Editor class represents the view for a VST3 plug-in. It automatically binds the VST3 parameters to VSTGUI control tags and it includes an inline UI editor for rapid development.
@section setup Setup
Add the following code to your Steinberg::Vst::EditController class:
@code
IPlugView* PLUGIN_API MyEditController::createView (FIDString name)
{
	if (strcmp (name, ViewType::kEditor) == 0)
	{
		return new VST3Editor (this, "view", "myEditor.uidesc");
	}
	return 0;
}
@endcode
To activate the inline editor you need to define the preprocessor definition "VSTGUI_LIVE_EDITING=1".
Rebuild your plug-in, start your preferred host, instantiate your plug-in, open the context menu inside your editor and choose "Enable Editing".
Now you can define tags, colors, fonts, bitmaps and add views to your editor.

See @ref page_uidescription_editor @n
*/
//-----------------------------------------------------------------------------
VST3Editor::VST3Editor (Steinberg::Vst::EditController* controller, UTF8StringPtr _viewName,
						UTF8StringPtr _xmlFile)
: VSTGUIEditor (controller)
{
	pImpl = std::make_unique<Impl> ();
	pImpl->controller = makeShared<Controller> (this);
	pImpl->delegate = dynamic_cast<IVST3EditorDelegate*> (controller);

	pImpl->description = UIDescription::make (_xmlFile);
	pImpl->viewName = _viewName;
	pImpl->xmlFile = _xmlFile;
	init ();
}

//-----------------------------------------------------------------------------
VST3Editor::VST3Editor (const SharedPointer<UIDescription>& desc,
						Steinberg::Vst::EditController* controller, UTF8StringPtr _viewName,
						UTF8StringPtr _xmlFile)
: VSTGUIEditor (controller)
{
	pImpl = std::make_unique<Impl> ();
	pImpl->controller = makeShared<Controller> (this);
	pImpl->delegate = dynamic_cast<IVST3EditorDelegate*> (controller);

	pImpl->description = desc;
	pImpl->viewName = _viewName;
	if (_xmlFile)
		pImpl->xmlFile = _xmlFile;
	init ();
}

//-----------------------------------------------------------------------------
VST3Editor::~VST3Editor () { pImpl->controller->editor = nullptr; }

//-----------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API VST3Editor::queryInterface (const Steinberg::TUID iid, void** obj)
{
#ifdef VST3_CONTENT_SCALE_SUPPORT
	QUERY_INTERFACE(iid, obj, Steinberg::IPlugViewContentScaleSupport::iid, Steinberg::IPlugViewContentScaleSupport)
#endif
	QUERY_INTERFACE(iid, obj, Steinberg::Vst::IParameterFinder::iid, Steinberg::Vst::IParameterFinder)
	return VSTGUIEditor::queryInterface (iid, obj);
}

//-----------------------------------------------------------------------------
void VST3Editor::init ()
{
	setIdleRate (300);
	if (pImpl->description->parse ())
	{
		// get sizes
		auto attr = pImpl->description->getViewAttributes (pImpl->viewName.c_str ());
		if (attr)
		{
			const std::string* sizeStr = attr->getAttributeValue ("size");
			const std::string* minSizeStr = attr->getAttributeValue ("minSize");
			const std::string* maxSizeStr = attr->getAttributeValue ("maxSize");
			if (sizeStr)
			{
				CPoint p;
				if (VST3EditorInternal::parseSize (*sizeStr, p))
				{
					rect.right = (Steinberg::int32)p.x;
					rect.bottom = (Steinberg::int32)p.y;
					pImpl->minSize = p;
					pImpl->maxSize = p;
				}
			}
			if (minSizeStr)
				VST3EditorInternal::parseSize (*minSizeStr, pImpl->minSize);
			if (maxSizeStr)
				VST3EditorInternal::parseSize (*maxSizeStr, pImpl->maxSize);
		}
		#if DEBUG
		else
		{
			auto debugAttr = VSTGUI::makeShared<UIAttributes> ();
			debugAttr->setAttribute (UIViewCreator::kAttrClass, "CViewContainer");
			debugAttr->setAttribute ("size", "300, 300");
			pImpl->description->addNewTemplate (pImpl->viewName.c_str (), debugAttr);
			rect.right = 300;
			rect.bottom = 300;
			pImpl->minSize (rect.right, rect.bottom);
			pImpl->maxSize (rect.right, rect.bottom);
		}
		#endif
	}
	#if DEBUG
	else
	{
		auto attr = VSTGUI::makeShared<UIAttributes> ();
		attr->setAttribute (UIViewCreator::kAttrClass, "CViewContainer");
		attr->setAttribute ("size", "300, 300");
		pImpl->description->addNewTemplate (pImpl->viewName.c_str (), attr);
		rect.right = 300;
		rect.bottom = 300;
		pImpl->minSize (rect.right, rect.bottom);
		pImpl->maxSize (rect.right, rect.bottom);
	}
	#endif
}

//-----------------------------------------------------------------------------
bool VST3Editor::exchangeView (UTF8StringPtr newViewName)
{
	if (pImpl->viewName == newViewName)
		return true;

	auto attr = pImpl->description->getViewAttributes (newViewName);
	if (attr)
	{
		pImpl->viewName = newViewName;
		auto minSizeStr = attr->getAttributeValue ("minSize");
		auto maxSizeStr = attr->getAttributeValue ("maxSize");
		if (minSizeStr)
			VST3EditorInternal::parseSize (*minSizeStr, pImpl->minSize);
		if (maxSizeStr)
			VST3EditorInternal::parseSize (*maxSizeStr, pImpl->maxSize);
		requestRecreateView ();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void VST3Editor::enableTooltips (bool state)
{
	pImpl->tooltipsEnabled = state;
	if (getFrame ())
		getFrame ()->enableTooltips (state);
}

//-----------------------------------------------------------------------------
bool VST3Editor::setEditorSizeConstrains (const CPoint& newMinimumSize, const CPoint& newMaximumSize)
{
	if (newMinimumSize.x <= newMaximumSize.x && newMinimumSize.y <= newMaximumSize.y)
	{
		pImpl->minSize = newMinimumSize;
		pImpl->maxSize = newMaximumSize;
		if (frame)
		{
			CRect currentSize, newSize;
			getFrame ()->getSize (currentSize);
			newSize = currentSize;
			CCoord width = currentSize.getWidth ();
			CCoord height = currentSize.getHeight ();
			double scaleFactor = getAbsScaleFactor ();
			if (width > pImpl->maxSize.x * scaleFactor)
				newSize.setWidth (pImpl->maxSize.x * scaleFactor);
			else if (width < pImpl->minSize.x * scaleFactor)
				newSize.setWidth (pImpl->minSize.x * scaleFactor);
			if (height > pImpl->maxSize.y * scaleFactor)
				newSize.setHeight (pImpl->maxSize.y * scaleFactor);
			else if (height < pImpl->minSize.y * scaleFactor)
				newSize.setHeight (pImpl->minSize.y * scaleFactor);
			if (newSize != currentSize)
				requestResize (CPoint (newSize.getWidth (), newSize.getHeight ()));
		}

		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
double VST3Editor::getAbsScaleFactor () const
{
	return getZoomFactor () * getContentScaleFactor ();
}

//-----------------------------------------------------------------------------
double VST3Editor::getContentScaleFactor () const { return pImpl->contentScaleFactor; }

//-----------------------------------------------------------------------------
double VST3Editor::getZoomFactor () const { return pImpl->zoomFactor; }

//-----------------------------------------------------------------------------
void VST3Editor::setAllowedZoomFactors (std::vector<double> zoomFactors)
{
	pImpl->allowedZoomFactors = zoomFactors;
}

#ifdef VST3_CONTENT_SCALE_SUPPORT
//-----------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API VST3Editor::setContentScaleFactor (ScaleFactor factor)
{
	pImpl->contentScaleFactor = factor;
	if (getFrame ())
	{
		getFrame ()->setZoom (getAbsScaleFactor ());
	}
	return Steinberg::kResultOk;
}
#endif

//-----------------------------------------------------------------------------
void VST3Editor::setZoomFactor (double factor)
{
	if (getZoomFactor () == factor)
		return;

	pImpl->zoomFactor = factor;

	if (getFrame () == nullptr)
		return;

	getFrame ()->setZoom (getAbsScaleFactor ());

	if (pImpl->delegate)
		pImpl->delegate->onZoomChanged (*this, pImpl->zoomFactor);
}

//-----------------------------------------------------------------------------
bool VST3Editor::beforeSizeChange (const CRect& newSize, const CRect& oldSize)
{
	if (pImpl->sizeRequest)
		return true;
	pImpl->sizeRequest = {newSize.getSize ()};
	bool result = requestResize (*pImpl->sizeRequest);
	pImpl->sizeRequest = {};
	return result;
}

//-----------------------------------------------------------------------------
bool VST3Editor::requestResize (const CPoint& newSize)
{
	if (!plugFrame)
		return false;

	Steinberg::ViewRect vr;
	vr.right = static_cast<Steinberg::int32> (std::floor (newSize.x));
	vr.bottom = static_cast<Steinberg::int32> (std::floor (newSize.y));
	return plugFrame->resizeView (this, &vr) == Steinberg::kResultTrue ? true : false;

}

//-----------------------------------------------------------------------------
void VST3Editor::getEditorSizeConstrains (CPoint& minimumSize, CPoint& maximumSize) const
{
	minimumSize = pImpl->minSize;
	maximumSize = pImpl->maxSize;
}

//-----------------------------------------------------------------------------
ParameterChangeListener* VST3Editor::getParameterChangeListener (int32_t tag) const
{
	if (tag != -1)
	{
		auto it = pImpl->paramChangeListeners.find (tag);
		if (it != pImpl->paramChangeListeners.end ())
		{
			return it->second;
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
void VST3Editor::beginEdit (int32_t index)
{
	// we don't assume that every control tag is a parameter tag handled by this editor
	// as sub classes could build custom CControlListeners for controls
}

//-----------------------------------------------------------------------------
void VST3Editor::endEdit (int32_t index)
{
	// see above
}

//-----------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API VST3Editor::findParameter (Steinberg::int32 xPos, Steinberg::int32 yPos, Steinberg::Vst::ParamID& resultTag)
{
	std::list<SharedPointer<CView> > views;
	if (frame && getFrame ()->getViewsAt (CPoint (xPos, yPos), views))
	{
		SharedPointer<CControl> control;
		std::list<SharedPointer<CView> >::const_iterator it = views.begin ();
		while (it != views.end ())
		{
			control = (*it).cast<CControl>();
			if (control)
			{
				if (control->getMouseEnabled () && control->getTag () != -1)
					break;
				control = nullptr;
				if ((*it)->getTransparency () == false)
					break;
			}
			it++;
		}
		if (control)
		{
			ParameterChangeListener* pcl = getParameterChangeListener (control->getTag ());
			if (pcl && pcl->containsControl (control.get ()) && pcl->getParameter ())
			{
				if (pImpl->delegate && pImpl->delegate->isPrivateParameter (pcl->getParameterID ()))
					return Steinberg::kResultFalse;
				resultTag = pcl->getParameterID ();
				return Steinberg::kResultTrue;
			}
		}
		Steinberg::Vst::ParamID pid;
		if (pImpl->delegate && pImpl->delegate->findParameter (CPoint (xPos, yPos), pid, *this) &&
			!pImpl->delegate->isPrivateParameter (pid))
		{
			resultTag = pid;
			return Steinberg::kResultTrue;
		}
	}
	return Steinberg::kResultFalse;
}

//-----------------------------------------------------------------------------
void VST3Editor::recreateView ()
{
	pImpl->doCreateView = false;
	enableEditing (pImpl->editingEnabled);
}

//------------------------------------------------------------------------
void VST3Editor::requestRecreateView ()
{
	if (pImpl->doCreateView || !frame)
		return;
	pImpl->doCreateView = true;
	auto task = [Self = Steinberg::IPtr<VST3Editor> (this)] () {
		if (Self->frame)
			Self->recreateView ();
	};
	if (frame->inEventProcessing ())
	{
		frame->doAfterEventProcessing (std::move (task));
	}
	else
	{
		task ();
	}
}

//-----------------------------------------------------------------------------
bool VST3Editor::inEditMode () const { return pImpl->editingEnabled; }

//-----------------------------------------------------------------------------
const std::string& VST3Editor::getCurrentTemplateName () const { return pImpl->viewName; }

#if LINUX
// Map Steinberg Vst Interface to VSTGUI Interface
class RunLoop : public IRunLoop,
				public AtomicReferenceCounted
{
public:
	struct EventHandler : Steinberg::Linux::IEventHandler, public Steinberg::FObject
	{
		VSTGUI::IEventHandler* handler {nullptr};

		void PLUGIN_API onFDIsSet (Steinberg::Linux::FileDescriptor) override
		{
			if (handler)
				handler->onEvent ();
		}
		DELEGATE_REFCOUNT (Steinberg::FObject)
		DEFINE_INTERFACES
			DEF_INTERFACE (Steinberg::Linux::IEventHandler)
		END_DEFINE_INTERFACES (Steinberg::FObject)
	};
	struct TimerHandler : Steinberg::Linux::ITimerHandler, public Steinberg::FObject
	{
		VSTGUI::ITimerHandler* handler {nullptr};

		void PLUGIN_API onTimer () final
		{
			if (handler)
				handler->onTimer ();
		}
		DELEGATE_REFCOUNT (Steinberg::FObject)
		DEFINE_INTERFACES
			DEF_INTERFACE (Steinberg::Linux::ITimerHandler)
		END_DEFINE_INTERFACES (Steinberg::FObject)
	};

	bool registerEventHandler (int fd, VSTGUI::IEventHandler* handler) final
	{
		if(!runLoop)
			return false;

		auto smtgHandler = Steinberg::owned (new EventHandler ());
		smtgHandler->handler = handler;
		if (runLoop->registerEventHandler (smtgHandler, fd) == Steinberg::kResultTrue)
		{
			eventHandlers.push_back (smtgHandler);
			return true;
		}
		return false;
	}
	bool unregisterEventHandler (VSTGUI::IEventHandler* handler) final
	{
		if(!runLoop)
			return false;

		for (auto it = eventHandlers.begin (), end = eventHandlers.end (); it != end; ++it)
		{
			if ((*it)->handler == handler)
			{
				runLoop->unregisterEventHandler ((*it));
				eventHandlers.erase (it);
				return true;
			}
		}
		return false;
	}
	bool registerTimer (uint64_t interval, VSTGUI::ITimerHandler* handler) final
	{
		if(!runLoop)
			return false;

		auto smtgHandler = Steinberg::owned (new TimerHandler ());
		smtgHandler->handler = handler;
		if (runLoop->registerTimer (smtgHandler, interval) == Steinberg::kResultTrue)
		{
			timerHandlers.push_back (smtgHandler);
			return true;
		}
		return false;
	}
	bool unregisterTimer (VSTGUI::ITimerHandler* handler) final
	{
		if(!runLoop)
			return false;

		for (auto it = timerHandlers.begin (), end = timerHandlers.end (); it != end; ++it)
		{
			if ((*it)->handler == handler)
			{
				runLoop->unregisterTimer ((*it));
				timerHandlers.erase (it);
				return true;
			}
		}
		return false;
	}

	RunLoop (Steinberg::FUnknown* runLoop) : runLoop (runLoop) {}
private:
	using EventHandlers = std::vector<Steinberg::IPtr<EventHandler>>;
	using TimerHandlers = std::vector<Steinberg::IPtr<TimerHandler>>;
	EventHandlers eventHandlers;
	TimerHandlers timerHandlers;
	Steinberg::FUnknownPtr<Steinberg::Linux::IRunLoop> runLoop;
};

//-----------------------------------------------------------------------------
class WaylandHost : public Wayland::IWaylandHost,
					public AtomicReferenceCounted
{
public:
	virtual wl_display* openWaylandConnection () final
	{
		if (!waylandHost)
			return nullptr;

		return waylandHost->openWaylandConnection ();
	}

	virtual bool closeWaylandConnection (wl_display* display) final
	{
		if (!waylandHost)
			return false;

		return waylandHost->closeWaylandConnection (display) == Steinberg::kResultOk;
	}

	WaylandHost (Steinberg::FUnknown* waylandHost) : waylandHost (waylandHost) {}

private:
	Steinberg::FUnknownPtr<Steinberg::IWaylandHost> waylandHost;
};

//-----------------------------------------------------------------------------
class WaylandFrame : public Wayland::IWaylandFrame,
					 public AtomicReferenceCounted
{
public:
	wl_surface* getWaylandSurface (wl_display* display) final
	{
		if (!waylandFrame)
			return nullptr;

		return waylandFrame->getWaylandSurface (display);
	}

	xdg_surface* getParentSurface (CRect& parentSize, wl_display* display) final
	{
		if (!waylandFrame)
			return nullptr;

		Steinberg::ViewRect viewRect;
		xdg_surface* surface = waylandFrame->getParentSurface (viewRect, display);
		parentSize = CRect (viewRect.left, viewRect.top, viewRect.right, viewRect.bottom);
		return surface;
	}

	xdg_toplevel* getParentToplevel (wl_display* display) final
	{
		if (!waylandFrame)
			return nullptr;

		return waylandFrame->getParentToplevel (display);
	}

	WaylandFrame (Steinberg::FUnknown* waylandFrame) : waylandFrame (waylandFrame) {}

private:
	Steinberg::FUnknownPtr<Steinberg::IWaylandFrame> waylandFrame;
};
#endif

#define kFrameEnableFocusDrawingAttr "frame-enable-focus-drawing"
#define kFrameFocusColorAttr "frame-focus-color"
#define kFrameFocusWidthAttr "frame-focus-width"

#if VSTGUI_LIVE_EDITING
// keyboard hook
struct VST3Editor::Impl::KeyboardHook : public IKeyboardHook
{
public:
	using Func = std::function<void (KeyboardEvent& event, CFrame& frame)>;

	KeyboardHook (Func&& keyDown, Func&& keyUp)
	: onKeyDownFunc (std::move (keyDown)), onKeyUpFunc (std::move (keyUp))
	{
	}

private:
	void onKeyboardEvent (KeyboardEvent& event, CFrame& frame) override
	{
		if (event.type == EventType::KeyDown)
		{
			onKeyDownFunc (event, frame);
		}
		else if (event.type == EventType::KeyUp)
		{
			onKeyUpFunc (event, frame);
		}
	}

	Func onKeyDownFunc;
	Func onKeyUpFunc;
};
#else
struct VST3Editor::KeyboardHook {};
#endif

//-----------------------------------------------------------------------------
bool PLUGIN_API VST3Editor::open (void* parent, const PlatformType& type)
{
	frame = makeShared<CFrame> (CRect (0, 0, 0, 0), this);
	getFrame ()->setTransparency (true);
	getFrame ()->registerMouseObserver (pImpl->controller.get ());
#if VSTGUI_LIVE_EDITING
	pImpl->keyboardHook = new Impl::KeyboardHook (
		[this] (KeyboardEvent& event, CFrame& frame) {
			if (event.modifiers.is (ModifierKey::Control) && frame.getModalView () == nullptr)
			{
			    if (event.character == 'e')
			    {
					pImpl->editingEnabled = !pImpl->editingEnabled;
					requestRecreateView ();
					event.consumed = true;
			    }
		    }
		},
		[] (KeyboardEvent&, CFrame&) {});
	getFrame ()->registerKeyboardHook (pImpl->keyboardHook);
#endif
	getFrame ()->enableTooltips (pImpl->tooltipsEnabled);

	if (!enableEditing (false))
	{
		frame.reset ();
		return false;
	}

	IPlatformFrameConfig* config = nullptr;
#if LINUX
	if (type == PlatformType::kWaylandSurfaceID)
	{
		if (parent != nullptr)
			return false;

		Wayland::FrameConfig* waylandConfig = new Wayland::FrameConfig;
		waylandConfig->runLoop = owned (new RunLoop (plugFrame));
		waylandConfig->waylandHost = owned (new WaylandHost (plugFrame));
		waylandConfig->waylandFrame = owned (new WaylandFrame (plugFrame));
		config = waylandConfig;
		parent = config;
	}
	else
	{
		X11::FrameConfig* x11config = new X11::FrameConfig;
		x11config->runLoop = owned (new RunLoop (plugFrame));
		config = x11config;
	}
#endif

	getFrame ()->open (parent, type, config);

	delete config;
	config = nullptr;

	if (pImpl->delegate)
		pImpl->delegate->didOpen (*this);

	Steinberg::IdleUpdateHandler::start ();

	return true;
}

//-----------------------------------------------------------------------------
void PLUGIN_API VST3Editor::close ()
{
	Steinberg::IdleUpdateHandler::stop ();

	if (pImpl->delegate)
		pImpl->delegate->willClose (*this);

	for (auto it = pImpl->paramChangeListeners.begin (), end = pImpl->paramChangeListeners.end ();
		 it != end; ++it)
		it->second->release ();

	pImpl->paramChangeListeners.clear ();
	if (frame)
	{
#if LINUX
		wl_display* display {};
		Steinberg::FUnknownPtr<Steinberg::IWaylandHost> host (controller->getHostContext ());
		if (host)
		{
			if (!frame->getAttribute<wl_display*> ('WlDi', display))
			{
				display = nullptr;
			}
		}

#endif // LINUX
#if VSTGUI_LIVE_EDITING
		if (pImpl->keyboardHook)
		{
			getFrame ()->unregisterKeyboardHook (pImpl->keyboardHook);
			delete pImpl->keyboardHook;
		}
		pImpl->keyboardHook = nullptr;
		pImpl->openUIEditorController = nullptr;
#endif
		getFrame ()->unregisterMouseObserver (pImpl->controller.get ());
		getFrame ()->removeAll ();
		int32_t refCount = getFrame ()->getNbReference ();
		if (refCount == 1)
		{
			getFrame ()->close ();
			frame = nullptr;
		}
		else
		{
			frame.reset ();
		}
#if LINUX
		if (host && display)
			host->closeWaylandConnection (display);
#endif
	}
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API VST3Editor::onSize (Steinberg::ViewRect* newSize)
{
	if (pImpl->sizeRequest)
	{
		auto width = static_cast<int32_t> (std::floor (pImpl->sizeRequest->x));
		auto height = static_cast<int32_t> (std::floor (pImpl->sizeRequest->y));
		if (width == newSize->getWidth () && height == newSize->getHeight ())
		{
			VSTGUIEditor::onSize (newSize);
			return Steinberg::kResultTrue;
		}
		return Steinberg::kResultFalse;
	}
	if (getFrame ())
	{
		CRect frameSize;
		getFrame ()->getSize (frameSize);
		auto width = static_cast<int32_t> (std::floor (frameSize.getWidth ()));
		auto height = static_cast<int32_t> (std::floor (frameSize.getHeight ()));
		if (frameSize.left == newSize->left && frameSize.top == newSize->top &&
			width == newSize->getWidth () && height == newSize->getHeight ())
		{
			VSTGUIEditor::onSize (newSize);
			return Steinberg::kResultTrue;
		}
	}
	pImpl->sizeRequest = {CPoint (newSize->getWidth (), newSize->getHeight ())};
	auto result = VSTGUIEditor::onSize (newSize);
	pImpl->sizeRequest = {};
	return result;
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API VST3Editor::canResize ()
{
	// always return true as this can change dynamicaly
	return Steinberg::kResultTrue;
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API VST3Editor::checkSizeConstraint (Steinberg::ViewRect* rect)
{
#if VSTGUI_LIVE_EDITING
	if (pImpl->editingEnabled)
		return Steinberg::kResultTrue;
#endif
	double scaleFactor = getAbsScaleFactor ();
	CCoord width = rect->right - rect->left;
	CCoord height = rect->bottom - rect->top;
	if (width < pImpl->minSize.x * scaleFactor)
		width = pImpl->minSize.x * scaleFactor;
	else if (width > pImpl->maxSize.x * scaleFactor)
		width = pImpl->maxSize.x * scaleFactor;
	if (height < pImpl->minSize.y * scaleFactor)
		height = pImpl->minSize.y * scaleFactor;
	else if (height > pImpl->maxSize.y * scaleFactor)
		height = pImpl->maxSize.y * scaleFactor;
	if (width != rect->getWidth () || height != rect->getHeight ())
	{
		rect->right = static_cast<int32_t> (std::floor (width + rect->left));
		rect->bottom = static_cast<int32_t> (std::floor (height + rect->top));
	}
	return Steinberg::kResultTrue;
}

namespace VST3EditorInternal {
//------------------------------------------------------------------------
static int32_t getUIDescriptionSaveOptions (CFrame* frame)
{
	int32_t flags = 0;
#if VSTGUI_LIVE_EDITING
	auto editController = getViewController (*frame->getView (0)).cast<UIEditController> ();
	if (editController)
	{
		auto attributes = editController->getSettings ();
		bool val;
		if (attributes->getBooleanAttribute (UIEditController::kEncodeBitmapsSettingsKey, val) && val == true)
		{
#if ((VSTGUI_VERSION_MAJOR == 4 && VSTGUI_VERSION_MINOR > 9) || (VSTGUI_VERSION_MAJOR > 4))
			flags |= UIDescription::kWriteImagesIntoUIDescFile;
#else
			flags |= UIDescription::kWriteImagesIntoXMLFile;
#endif
		}
		if (attributes->getBooleanAttribute (UIEditController::kWriteWindowsRCFileSettingsKey, val) && val == true)
		{
			flags |= UIDescription::kWriteWindowsResourceFile;
		}
	}
#endif
	return flags;
}
} // namespace VST3EditorInternal

//------------------------------------------------------------------------
void VST3Editor::save (bool saveAs)
{
	auto attributes = pImpl->description->getCustomAttributes ("VST3Editor", true);
	vstgui_assert(attributes);
	if (!attributes)
		return;
	std::string savePath;
	if (saveAs)
	{
		auto fileSelector =
			CNewFileSelector::create (frame.get (), CNewFileSelector::kSelectSaveFile);
		if (fileSelector == nullptr)
			return;
		fileSelector->setTitle ("Save UIDescription File");
		fileSelector->setDefaultExtension (CFileExtension ("VSTGUI UI Description", "uidesc"));
		const std::string* prevFilePath = attributes->getAttributeValue ("Path");
		if (prevFilePath)
			fileSelector->setInitialDirectory (prevFilePath->c_str ());
		else if (!pImpl->xmlFile.empty ())
		{
			if (pImpl->xmlFile[0] == '/')
				fileSelector->setInitialDirectory (pImpl->xmlFile.c_str ());
			else
				fileSelector->setDefaultSaveName (pImpl->xmlFile.c_str ());
		}
		if (fileSelector->runModal ())
		{
			UTF8StringPtr filePath = fileSelector->getSelectedFile (0);
			if (filePath)
			{
				attributes->setAttribute ("Path", filePath);
				savePath = filePath;
			}
		}
	}
	else
	{
		const std::string* filePath = attributes ? attributes->getAttributeValue ("Path") : nullptr;
		if (filePath)
			savePath = *filePath;
	}
	if (savePath.empty ())
		return;

	// filter out attributes we will always override with the values from the parameters
	auto filter = [] (CView& view, const std::string& name) -> bool {
		if (auto control = dynamic_cast<CControl*> (&view))
		{
			if (control->getTag () != -1)
			{
				if (name == UIViewCreator::kAttrMinValue)
					return false;
				if (name == UIViewCreator::kAttrMaxValue)
					return false;
				if (name == UIViewCreator::kAttrDefaultValue)
					return false;
				if (name == UIViewCreator::kAttrMouseEnabled)
					return false;
				if (name == UIViewCreator::kAttrTitle && dynamic_cast<CTextLabel*> (control))
					return false;
			}
		}
		return true;
	};

	if (pImpl->description->save (savePath.c_str (),
								  VST3EditorInternal::getUIDescriptionSaveOptions (frame.get ()),
								  filter))
		pImpl->description->setFilePath (savePath.c_str ());
}

//------------------------------------------------------------------------
void VST3Editor::syncParameterTags ()
{
#if VSTGUI_LIVE_EDITING
	auto view = getFrame ()->getView (0);
	if (view)
	{
		auto controller = getViewController (*view);
		IActionPerformer* actionPerformer =
			controller ? dynamic_cast<IActionPerformer*> (controller.get ()) : nullptr;
		if (actionPerformer)
		{
			Steinberg::Vst::EditController* editController = getController ();

			actionPerformer->beginGroupAction ("Sync Parameter Tags");

			std::map<Steinberg::Vst::UnitID, Steinberg::Vst::UnitInfo> units;
			Steinberg::FUnknownPtr<Steinberg::Vst::IUnitInfo> ec2 (editController->unknownCast ());
			if (ec2)
			{
				Steinberg::int32 unitCount = ec2->getUnitCount ();
				Steinberg::Vst::UnitInfo info;
				for (int32_t i = 0; i < unitCount; i++)
				{
					ec2->getUnitInfo (i, info);
					units.insert (std::pair<Steinberg::Vst::UnitID, Steinberg::Vst::UnitInfo> (info.id, info));
				}
			}

			int32_t paramCount = editController->getParameterCount ();
			for (int32_t i = 0; i < paramCount; i++)
			{
				Steinberg::Vst::ParameterInfo info;
				if (editController->getParameterInfo (i, info) == Steinberg::kResultTrue)
				{
					Steinberg::String paramTitle (info.title);
					if (info.unitId != Steinberg::Vst::kRootUnitId)
					{
						std::map<Steinberg::Vst::UnitID, Steinberg::Vst::UnitInfo>::const_iterator it = units.find (info.unitId);
						if (it != units.end ())
						{
							paramTitle.insertAt (0, "::");
							paramTitle.insertAt (0, it->second.name);
						}
					}
					else if (!units.empty ())
					{
						paramTitle.insertAt (0, "::");
						paramTitle.insertAt (0, "Root");
					}
					paramTitle.toMultiByte (Steinberg::kCP_Utf8);
					paramTitle.removeChars (' ');
					Steinberg::String paramIDStr;
					paramIDStr.printInt64 (info.id);
					if (int32_t tag = pImpl->description->getTagForName (paramTitle) != -1)
					{
						if (tag != info.id)
							actionPerformer->performTagChange (paramTitle, paramIDStr);
					}
					else if (UTF8StringPtr tagName = pImpl->description->lookupControlTagName (
								 static_cast<int32_t> (info.id)))
					{
						actionPerformer->performTagNameChange (tagName, paramTitle);
					}
					else
					{
						actionPerformer->performTagChange (paramTitle, paramIDStr);
					}
				}
			}
			actionPerformer->finishGroupAction ();
		}
	}
#endif
}

//------------------------------------------------------------------------
void VST3Editor::saveScreenshot ()
{
	if (auto fileSelector =
			CNewFileSelector::create (getFrame (), CNewFileSelector::kSelectDirectory))
	{
		fileSelector->setTitle ("Select Directory where to save the screenshots");
		fileSelector->run ([this] (CNewFileSelector& fs) {
			if (fs.getNumSelectedFiles () != 1)
				return;

			auto makeScreenshot = [] (CFrame* frame) -> SharedPointer<CBitmap> {
				auto size = frame->getViewSize ().getSize ();
				if (auto offscreen = COffscreenContext::create (size, 1.))
				{
					offscreen->beginDraw ();
					frame->draw (*offscreen);
					offscreen->endDraw ();
					return offscreen->getBitmap ();
				}
				return nullptr;
			};

			showEditButton (false);

			auto origZoom = getFrame ()->getZoom ();
			getFrame ()->setZoom (1.);
			auto bitmap1 = makeScreenshot (getFrame ());
			getFrame ()->setZoom (2.);
			auto bitmap2 = makeScreenshot (getFrame ());
			getFrame ()->setZoom (origZoom);

			auto folderPath = std::string (fs.getSelectedFile (0));
			auto uidStr = std::string ("XXXXXXXX");
			if (bitmap1)
			{
				auto data = getPlatformFactory ().createBitmapMemoryPNGRepresentation (
					bitmap1->getPlatformBitmap ());
				if (!data.empty ())
				{
					auto filename = folderPath + "/" + uidStr + "_snapshot.png";
					CFileStream stream;
					if (stream.open (filename.data (), CFileStream::kWriteMode |
														   CFileStream::kTruncateMode |
														   CFileStream::kBinaryMode))
					{
						stream.writeRaw (data.data (), static_cast<uint32_t> (data.size ()));
					}
				}
			}
			if (bitmap2)
			{
				auto filename = folderPath + "/" + uidStr + "_snapshot_2.0x.png";
				auto data = getPlatformFactory ().createBitmapMemoryPNGRepresentation (
					bitmap2->getPlatformBitmap ());
				if (!data.empty ())
				{
					CFileStream stream;
					if (stream.open (filename.data (), CFileStream::kWriteMode |
														   CFileStream::kTruncateMode |
														   CFileStream::kBinaryMode))
					{
						stream.writeRaw (data.data (), static_cast<uint32_t> (data.size ()));
					}
				}
			}
			if (enableShowEditButton ())
				showEditButton (true);
		});
	}
}

//------------------------------------------------------------------------
bool VST3Editor::enableShowEditButton () const
{
	bool addShowEditorButton = true;
	if (auto attributes = pImpl->description->getCustomAttributes ("VST3Editor", true))
	{
		attributes->getBooleanAttribute ("Show Editor Button", addShowEditorButton);
	}
	return addShowEditorButton;
}

//------------------------------------------------------------------------
void VST3Editor::enableShowEditButton (bool state)
{
	if (auto attributes = pImpl->description->getCustomAttributes ("VST3Editor", true))
		attributes->setBooleanAttribute ("Show Editor Button", state);
}

#if VSTGUI_LIVE_EDITING
//------------------------------------------------------------------------
struct VST3Editor::Impl::EnterEditModeController : ViewListenerAdapter,
												   ViewEventListenerAdapter,
												   ControlListenerAdapter
{
	using EnterEditModeFunc = std::function<void ()>;

	static constexpr const auto strFull = "Open UI Editor";
	static constexpr const auto strMinimized = "e";

	EnterEditModeController (CFrame* frame, EnterEditModeFunc&& func)
	: enterEditMode (std::move (func))
	{
		auto buttonPtr = makeShared<CTextButton> (CRect {0, 0, 120, 20});
		buttonPtr->setTitle (strFull);
		buttonPtr->setRoundRadius (2.);
		buttonPtr->setFrameWidth (-1);
		buttonPtr->registerViewListener (this);
		buttonPtr->registerViewEventListener (this);
		buttonPtr->registerControlListener (this);
		frame->addSubview (buttonPtr);
		button = buttonPtr.get ();
	}
	~EnterEditModeController () noexcept override
	{
		if (button)
		{
			unregisterButtonListeners ();
			if (auto parent = button->getParentView ())
				parent->removeSubview (shared (button));
		}
	}

	void unregisterButtonListeners ()
	{
		button->unregisterViewEventListener (this);
		button->unregisterViewListener (this);
		button->unregisterControlListener (this);
	}

	void valueChanged (CControl& c) override
	{
		if (c.getValue () == 1.)
		{
			enterEditMode ();
		}
	}
	void viewAttached (CView& view) override
	{
		view.addAnimation ("SizeAnim", makeShared<Animation::AlphaValueAnimation> (1.f),
						   makeShared<Animation::LinearTimingFunction> (1000),
						   [&] (auto&&, auto&&, auto&&) { close (); });
	}

	void viewWillDelete (CView& view) override
	{
		if (button == nullptr)
			return;
		unregisterButtonListeners ();
		button = nullptr;
	}
	void viewOnEvent (CView& view, Event& event) override
	{
		if (event.type == EventType::MouseEnter)
		{
			open ();
		}
		else if (event.type == EventType::MouseExit)
		{
			close ();
		}
	}

	SharedPointer<Animation::ITimingFunction> createDefAnimTimingFunc () const
	{
		using namespace Animation;

		static const constexpr auto AnimationTime = 150;
		return makeShared<CubicBezierTimingFunction> (
			CubicBezierTimingFunction::easyInOut (AnimationTime));
	}

	void open ()
	{
		button->addAnimation ("SizeAnim",
							  makeShared<Animation::ViewSizeAnimation> (CRect {0, 0, 120, 20}),
							  createDefAnimTimingFunc (),
							  [&] (auto&&, auto&&, auto&&) { button->setTitle (strFull); });
		button->addAnimation ("AlphaValue", makeShared<Animation::AlphaValueAnimation> (1.f),
							  createDefAnimTimingFunc ());
	}
	void close ()
	{
		button->addAnimation ("SizeAnim",
							  makeShared<Animation::ViewSizeAnimation> (CRect {0, 0, 10, 20}),
							  createDefAnimTimingFunc (),
							  [&] (auto&&, auto&&, auto&&) { button->setTitle (strMinimized); });
		button->addAnimation ("AlphaValue", makeShared<Animation::AlphaValueAnimation> (0.3f),
							  createDefAnimTimingFunc ());
	}

	EnterEditModeFunc enterEditMode;
	CTextButton* button {nullptr};
};

#endif

//------------------------------------------------------------------------
void VST3Editor::showEditButton (bool state)
{
#if VSTGUI_LIVE_EDITING
	if ((state && pImpl->openUIEditorController) ||
		(!state && pImpl->openUIEditorController == nullptr))
		return;
	if (state)
	{
		pImpl->openUIEditorController =
			makeShared<Impl::EnterEditModeController> (getFrame (), [this] () {
				pImpl->editingEnabled = true;
				requestRecreateView ();
			});
	}
	else
	{
		pImpl->openUIEditorController.reset ();
	}
#endif
}

//------------------------------------------------------------------------
bool VST3Editor::enableEditing (bool state)
{
	if (getFrame ())
	{
		getFrame ()->removeAll ();

	#if VSTGUI_LIVE_EDITING
		pImpl->openUIEditorController = nullptr;
		if (state)
		{
			pImpl->editingEnabled = true;
			// update uiDesc file path to absolute if possible
			if (auto attributes = pImpl->description->getCustomAttributes ("VST3Editor", true))
			{
				const std::string* filePath = attributes->getAttributeValue ("Path");
				if (filePath)
				{
					CFileStream s;
					if (!s.open (filePath->c_str (), CFileStream::kReadMode))
					{
						attributes->removeAttribute ("Path");
					}
					else
						pImpl->description->setFilePath (filePath->c_str ());
				}
			}

			getFrame ()->setTransform (CGraphicsTransform ());
			pImpl->nonEditRect = getFrame ()->getViewSize ();
			pImpl->description->setController (pImpl->controller);
			auto editController = UIEditController::make (pImpl->description);
			if (auto view = editController->createEditView ())
			{
				CCoord width = view->getWidth ();
				CCoord height = view->getHeight ();

				getFrame ()->setSize (width, height);
				getFrame ()->addSubview (view);
				getFrame ()->setZoom (getContentScaleFactor ());

				getFrame ()->enableTooltips (true);
				CColor focusColor = kBlueCColor;
				editController->getEditorDescription ()->getColor ("focus", focusColor);
				getFrame ()->setFocusColor (focusColor);
				getFrame ()->setFocusDrawingEnabled (true);
				getFrame ()->setFocusWidth (1);

				if (auto fileMenu = editController->getMenuController ()->getFileMenu ())
				{
					auto item =
						fileMenu->addEntry (makeShared<CCommandMenuItem> (CCommandMenuItem::Desc {
												"Save", pImpl->controller, "File", "Save"}),
											0);
					item->setKey ("s", kControl);
					item =
						fileMenu->addEntry (makeShared<CCommandMenuItem> (CCommandMenuItem::Desc {
												"Save As..", pImpl->controller, "File", "Save As"}),
											1);
					item->setKey ("s", kShift | kControl);
					item = fileMenu->addEntry (makeShared<CCommandMenuItem> (
						CCommandMenuItem::Desc {"Close Editor", pImpl->controller, "File",
												"Close UIDescription Editor"}));
					item->setKey ("e", kControl);
				}
				if (auto editMenu = editController->getMenuController ()->getEditMenu ())
				{
					editMenu->addSeparator ();
					editMenu->addEntry (makeShared<CCommandMenuItem> (CCommandMenuItem::Desc {
						"Sync Parameter Tags", pImpl->controller, "Edit", "Sync Parameter Tags"}));
				}
				return true;
			}
		}
		else
	#endif
		{
			pImpl->editingEnabled = false;
			auto view =
				pImpl->description->createView (pImpl->viewName.c_str (), pImpl->controller);
			if (view)
			{
				double scaleFactor = getAbsScaleFactor ();
				CCoord width = view->getWidth () * scaleFactor;
				CCoord height = view->getHeight () * scaleFactor;

				if (canResize () == Steinberg::kResultTrue && pImpl->nonEditRect.isEmpty ())
				{
					Steinberg::ViewRect tmp;
					if (getRect ().getWidth () != width)
						tmp.right = getRect ().getWidth ();
					if (getRect ().getHeight () != height)
						tmp.bottom = getRect ().getHeight ();
					if (tmp.getWidth () || tmp.getHeight ())
					{
						if (tmp.getWidth () == 0)
							tmp.right = width;
						if (tmp.getHeight () == 0)
							tmp.bottom = width;
						checkSizeConstraint (&tmp);
						pImpl->nonEditRect.setWidth (tmp.getWidth ());
						pImpl->nonEditRect.setHeight (tmp.getHeight ());
					}
				}

				getFrame ()->setSize (width, height);
				getFrame ()->addSubview (view);
				getFrame ()->setTransform (CGraphicsTransform ().scale (scaleFactor, scaleFactor));
				getFrame ()->invalid ();
				if (pImpl->nonEditRect.isEmpty () == false)
				{
					Steinberg::ViewRect tmpRect = rect;
					tmpRect.right = tmpRect.left + (Steinberg::int32)pImpl->nonEditRect.getWidth ();
					tmpRect.bottom =
						tmpRect.top + (Steinberg::int32)pImpl->nonEditRect.getHeight ();
					plugFrame->resizeView (this, &tmpRect);
				}
				else
				{
					rect.right = rect.left + width;
					rect.bottom = rect.top + height;
					requestResize ({width, height});
				}

				getFrame ()->setFocusDrawingEnabled (false);

				// focus drawing support
				auto attributes = pImpl->description->getCustomAttributes ("FocusDrawing", true);

				// map old one
				auto oldAttributes = pImpl->description->getCustomAttributes ("VST3Editor");
				if (oldAttributes)
				{
					const std::string* attr = oldAttributes->getAttributeValue (kFrameEnableFocusDrawingAttr);
					if (attr)
					{
						if (*attr == "true")
						{
							attributes->setAttribute ("enabled", "true");
							attr = oldAttributes->getAttributeValue (kFrameFocusColorAttr);
							if (attr)
							{
								attributes->setAttribute ("color", *attr);
							}
							attr = oldAttributes->getAttributeValue (kFrameFocusWidthAttr);
							if (attr)
							{
								attributes->setAttribute ("width", *attr);
							}
						}
						oldAttributes->removeAttribute (kFrameFocusColorAttr);
						oldAttributes->removeAttribute (kFrameFocusWidthAttr);
						oldAttributes->removeAttribute (kFrameEnableFocusDrawingAttr);
					}
				}
				// new one
				const std::string* attr = attributes->getAttributeValue ("enabled");
				if (attr && *attr == "true")
				{
					getFrame ()->setFocusDrawingEnabled (true);
					attr = attributes->getAttributeValue ("color");
					if (attr)
					{
						CColor focusColor;
						if (pImpl->description->getColor (attr->c_str (), focusColor))
							getFrame ()->setFocusColor (focusColor);
					}
					attr = attributes->getAttributeValue ("width");
					if (attr)
					{
						double focusWidth = UTF8StringView (attr->c_str ()).toDouble ();
						getFrame ()->setFocusWidth (focusWidth);
					}
				}
#if VSTGUI_LIVE_EDITING
				if (enableShowEditButton ())
					showEditButton (true);
#endif
				return true;
			}
		}
	}
	return false;
}

//------------------------------------------------------------------------
void VST3Editor::setDelegate (IVST3EditorDelegate* inDelegate) { pImpl->delegate = inDelegate; }

//------------------------------------------------------------------------
IVST3EditorDelegate* VST3Editor::getDelegate () const { return pImpl->delegate; }

//------------------------------------------------------------------------
SharedPointer<UIDescription> VST3Editor::getUIDescription () const { return pImpl->description; }

//------------------------------------------------------------------------
//--- AspectRatioVST3Editor
//------------------------------------------------------------------------
void AspectRatioVST3Editor::setMinZoomFactor (double factor) { minZoomFactor = factor; }

//------------------------------------------------------------------------
double AspectRatioVST3Editor::getMinZoomFactor () const { return minZoomFactor; }

//------------------------------------------------------------------------
bool PLUGIN_API AspectRatioVST3Editor::open (void* parent, const PlatformType& type)
{
	calcZoomFactor = getZoomFactor ();
	if (VST3Editor::open (parent, type) && getFrame ())
	{
		initialSize = getFrame ()->getViewSize ().getSize ();
		initialSize /= calcZoomFactor;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API AspectRatioVST3Editor::onSize (Steinberg::ViewRect* newSize)
{
	if (newSize == nullptr)
		return Steinberg::kInvalidArgument;

	if (!canCalculateAspectRatio ())
		return VST3Editor::onSize (newSize);

	setZoomFactor (calcZoomFactor);
	return Steinberg::kResultTrue;
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API AspectRatioVST3Editor::checkSizeConstraint (Steinberg::ViewRect* rect)
{
	if (rect == nullptr)
		return Steinberg::kInvalidArgument;

	if (!canCalculateAspectRatio ())
		return VST3Editor::checkSizeConstraint (rect);

	const CPoint size (rect->getWidth (), rect->getHeight ());
	auto diff = size - initialSize;
	auto anchor = initialSize.x >= initialSize.y ? initialSize.x : initialSize.y;
	auto sizeAnchor = initialSize.x >= initialSize.y ? size.x : size.y;

	auto factor = sizeAnchor / anchor;
	if (factor < minZoomFactor * getContentScaleFactor ())
		factor = minZoomFactor * getContentScaleFactor ();
	factor = std::round (factor * anchor) / anchor;
	auto newSize = initialSize * factor;
	calcZoomFactor = factor / getContentScaleFactor ();
	if (newSize != size)
	{
		rect->right = rect->left + newSize.x;
		rect->bottom = rect->top + newSize.y;
	}
	return Steinberg::kResultTrue;
}

#ifdef VST3_CONTENT_SCALE_SUPPORT
//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API AspectRatioVST3Editor::setContentScaleFactor (ScaleFactor factor)
{
	auto res = VST3Editor::setContentScaleFactor (factor);
	if (res == Steinberg::kResultTrue && canCalculateAspectRatio ())
		setZoomFactor (calcZoomFactor);
	return res;
}
#endif

//------------------------------------------------------------------------
bool AspectRatioVST3Editor::canCalculateAspectRatio () const
{
	auto f = getFrame ();
	if (inEditMode () || f == nullptr || f->hasChildren () == false)
		return false;
	return true;
}

//------------------------------------------------------------------------
} // VSTGUI
