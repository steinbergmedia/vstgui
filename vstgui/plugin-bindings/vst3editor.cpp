// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "vst3editor.h"
#include "../vstgui.h"
#include "../lib/cvstguitimer.h"
#include "../lib/vstkeycode.h"
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
#include "pluginterfaces/gui/iplugview.h"
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
			instance.timer = VSTGUI::makeOwned<VSTGUI::CVSTGUITimer> (
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
class ParameterChangeListener : public Steinberg::FObject
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
		if (parameter)
		{
			parameter->removeDependent (this);
			parameter->release ();
		}
		for (const auto& c : controls)
			c->forget ();
	}

	void addControl (CControl* control)
	{
		if (containsControl (control))
			return;
		control->remember ();
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
			display->setValueToStringFunction([this](float value, char utf8String[256], CParamDisplay* display) {
				return convertValueToString (value, utf8String);
			});

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
				controls.remove (control);
				control->forget ();
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
					auto* optMenu = dynamic_cast<COptionMenu*>(c);
					if (optMenu)
					{
						optMenu->removeAllEntry ();
						for (Steinberg::int32 i = 0; i <= parameter->getInfo ().stepCount; i++)
						{
							Steinberg::Vst::String128 utf16Str;
							editController->getParamStringByValue (getParameterID (), (Steinberg::Vst::ParamValue)i / (Steinberg::Vst::ParamValue)parameter->getInfo ().stepCount, utf16Str);
							Steinberg::String utf8Str (utf16Str);
							utf8Str.toMultiByte (Steinberg::kCP_Utf8);
							optMenu->addEntry (utf8Str.text8 ());
						}
						c->setValue ((float)value - minValue);
					}
					else
						c->setValue ((float)value);
				}
				else
					c->setValueNormalized ((float)value);
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

//-----------------------------------------------------------------------------
static void releaseSubController (IController* subController)
{
	if (auto ref = dynamic_cast<IReference*> (subController))
		ref->forget();
	else if (auto fobj = dynamic_cast<Steinberg::FObject*> (subController))
		fobj->release ();
	else
		delete subController;
}

} // namespace VST3EditorInternal

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
Rebuild your plug-in, start your prefered host, instanciate your plug-in, open the context menu inside your editor and choose "Enable Editing".
Now you can define tags, colors, fonts, bitmaps and add views to your editor.

See @ref page_uidescription_editor @n
*/
//-----------------------------------------------------------------------------
VST3Editor::VST3Editor (Steinberg::Vst::EditController* controller, UTF8StringPtr _viewName, UTF8StringPtr _xmlFile)
: VSTGUIEditor (controller)
, delegate (dynamic_cast<VST3EditorDelegate*> (controller))
{
	description = new UIDescription (_xmlFile);
	viewName = _viewName;
	xmlFile = _xmlFile;
	init ();
}

//-----------------------------------------------------------------------------
VST3Editor::VST3Editor (UIDescription* desc, Steinberg::Vst::EditController* controller, UTF8StringPtr _viewName, UTF8StringPtr _xmlFile)
: VSTGUIEditor (controller)
, delegate (dynamic_cast<VST3EditorDelegate*> (controller))
{
	description = desc;
	description->remember ();
	viewName = _viewName;
	if (_xmlFile)
		xmlFile = _xmlFile;
	init ();
}

//-----------------------------------------------------------------------------
VST3Editor::~VST3Editor ()
{
	description->forget ();
}

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
	// we will always call CView::setDirty() on the main thread
	VSTGUI::CView::kDirtyCallAlwaysOnMainThread = true;

	setIdleRate (300);
	if (description->parse ())
	{
		// get sizes
		const auto* attr = description->getViewAttributes (viewName.c_str ());
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
					minSize = p;
					maxSize = p;
				}
			}
			if (minSizeStr)
				VST3EditorInternal::parseSize (*minSizeStr, minSize);
			if (maxSizeStr)
				VST3EditorInternal::parseSize (*maxSizeStr, maxSize);
		}
		#if DEBUG
		else
		{
			auto* debugAttr = new UIAttributes ();
			debugAttr->setAttribute (UIViewCreator::kAttrClass, "CViewContainer");
			debugAttr->setAttribute ("size", "300, 300");
			description->addNewTemplate (viewName.c_str (), debugAttr);
			rect.right = 300;
			rect.bottom = 300;
			minSize (rect.right, rect.bottom);
			maxSize (rect.right, rect.bottom);
		}
		#endif
	}
	#if DEBUG
	else
	{
		auto* attr = new UIAttributes ();
		attr->setAttribute (UIViewCreator::kAttrClass, "CViewContainer");
		attr->setAttribute ("size", "300, 300");
		description->addNewTemplate (viewName.c_str (), attr);
		rect.right = 300;
		rect.bottom = 300;
		minSize (rect.right, rect.bottom);
		maxSize (rect.right, rect.bottom);
	}
	#endif
}

//-----------------------------------------------------------------------------
bool VST3Editor::exchangeView (UTF8StringPtr newViewName)
{
	const UIAttributes* attr = description->getViewAttributes (newViewName);
	if (attr)
	{
		viewName = newViewName;
		doCreateView = true;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void VST3Editor::enableTooltips (bool state)
{
	tooltipsEnabled = state;
	if (getFrame ())
		getFrame ()->enableTooltips (state);
}

//-----------------------------------------------------------------------------
bool VST3Editor::setEditorSizeConstrains (const CPoint& newMinimumSize, const CPoint& newMaximumSize)
{
	if (newMinimumSize.x <= newMaximumSize.x && newMinimumSize.y <= newMaximumSize.y)
	{
		minSize = newMinimumSize;
		maxSize = newMaximumSize;
		if (frame)
		{
			CRect currentSize, newSize;
			getFrame ()->getSize (currentSize);
			newSize = currentSize;
			CCoord width = currentSize.getWidth ();
			CCoord height = currentSize.getHeight ();
			double scaleFactor = getAbsScaleFactor ();
			if (width > maxSize.x * scaleFactor)
				currentSize.setWidth (maxSize.x * scaleFactor);
			else if (width < minSize.x * scaleFactor)
				currentSize.setWidth (minSize.x * scaleFactor);
			if (height > maxSize.y * scaleFactor)
				currentSize.setHeight (maxSize.y * scaleFactor);
			else if (height < minSize.y * scaleFactor)
				currentSize.setHeight (minSize.y * scaleFactor);
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
	return zoomFactor * contentScaleFactor;
}

#ifdef VST3_CONTENT_SCALE_SUPPORT
//-----------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API VST3Editor::setContentScaleFactor (ScaleFactor factor)
{
	contentScaleFactor = factor;
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
	if (zoomFactor == factor)
		return;

	zoomFactor = factor;

	if (getFrame () == nullptr)
		return;

	getFrame ()->setZoom (getAbsScaleFactor ());
}

//-----------------------------------------------------------------------------
bool VST3Editor::beforeSizeChange (const CRect& newSize, const CRect& oldSize)
{
	if (requestResizeGuard)
		return true;
	requestResizeGuard = true;
	bool result = requestResize (newSize.getSize ());
	requestResizeGuard = false;
	return result;
}

//-----------------------------------------------------------------------------
bool VST3Editor::requestResize (const CPoint& newSize)
{
	if (!plugFrame)
		return false;
	CCoord width = newSize.x;
	CCoord height = newSize.y;
	double scaleFactor = getAbsScaleFactor ();
	if (editingEnabled || (width >= std::round (minSize.x * scaleFactor) && width <= std::round (maxSize.x * scaleFactor)
                        && height >= std::round (minSize.y * scaleFactor) && height <= std::round (maxSize.y * scaleFactor)))
	{
		Steinberg::ViewRect vr;
		vr.right = static_cast<Steinberg::int32> (width);
		vr.bottom = static_cast<Steinberg::int32> (height);
		return plugFrame->resizeView (this, &vr) == Steinberg::kResultTrue ? true : false;
	}
	return false;
}

//-----------------------------------------------------------------------------
void VST3Editor::getEditorSizeConstrains (CPoint& minimumSize, CPoint& maximumSize) const
{
	minimumSize = minSize;
	maximumSize = maxSize;
}

//-----------------------------------------------------------------------------
ParameterChangeListener* VST3Editor::getParameterChangeListener (int32_t tag) const
{
	if (tag != -1)
	{
		auto it = paramChangeListeners.find (tag);
		if (it != paramChangeListeners.end ())
		{
			return it->second;
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
void VST3Editor::valueChanged (CControl* pControl)
{
	using namespace Steinberg;

	ParameterChangeListener* pcl = getParameterChangeListener (pControl->getTag ());
	if (pcl)
	{
		auto paramID = pcl->getParameterID ();
		auto normalizedValue = static_cast<Vst::ParamValue> (pControl->getValueNormalized ());
		auto* textEdit = dynamic_cast<CTextEdit*> (pControl);
		if (textEdit && pcl->getParameter ())
		{
			Steinberg::String str (textEdit->getText ());
			str.toWideString (kCP_Utf8);
			if (getController ()->getParamValueByString (paramID,
			                                             const_cast<Vst::TChar*> (str.text16 ()),
			                                             normalizedValue) != kResultTrue)
			{
				pcl->update (nullptr, kChanged);
				return;
			}
		}
		pcl->performEdit (normalizedValue);
	}
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
void VST3Editor::controlBeginEdit (CControl* pControl)
{
	ParameterChangeListener* pcl = getParameterChangeListener (pControl->getTag ());
	if (pcl)
	{
		pcl->beginEdit ();
	}
}

//-----------------------------------------------------------------------------
void VST3Editor::controlEndEdit (CControl* pControl)
{
	ParameterChangeListener* pcl = getParameterChangeListener (pControl->getTag ());
	if (pcl)
	{
		pcl->endEdit ();
	}
}

//-----------------------------------------------------------------------------
void VST3Editor::controlTagWillChange (CControl* pControl)
{
	if (pControl->getTag () != -1 && pControl->getListener () == this)
	{
		ParameterChangeListener* pcl = getParameterChangeListener (pControl->getTag ());
		if (pcl)
		{
			pcl->removeControl (pControl);
		}
	}
}

//-----------------------------------------------------------------------------
void VST3Editor::controlTagDidChange (CControl* pControl)
{
	if (pControl->getTag () != -1 && pControl->getListener () == this)
	{
		ParameterChangeListener* pcl = getParameterChangeListener (pControl->getTag ());
		if (pcl)
		{
			pcl->addControl (pControl);
		}
		else
		{
			Steinberg::Vst::EditController* editController = getController ();
			if (editController)
			{
				Steinberg::Vst::Parameter* parameter = editController->getParameterObject (static_cast<Steinberg::Vst::ParamID> (pControl->getTag ()));
				paramChangeListeners.insert (std::make_pair (pControl->getTag (), new ParameterChangeListener (editController, parameter, pControl)));
			}
		}
	}
}

//-----------------------------------------------------------------------------
void VST3Editor::onViewAdded (CFrame* frame, CView* view)
{
}

//-----------------------------------------------------------------------------
void VST3Editor::onViewRemoved (CFrame* frame, CView* view)
{
	auto* control = dynamic_cast<CControl*> (view);
	if (control && control->getTag () != -1)
	{
		ParameterChangeListener* pcl = getParameterChangeListener (control->getTag ());
		if (pcl)
		{
			pcl->removeControl (control);
		}
	}
	// TODO: Currently when in Edit Mode in UIEditor, subcontrollers will be released, even tho the view may be added again later on.
	IController* controller = getViewController (view);
	if (controller)
	{
		VST3EditorInternal::releaseSubController (controller);
		view->removeAttribute (kCViewControllerAttribute);
	}
}

#if VST3_SUPPORTS_CONTEXTMENU
/// @cond ignore
namespace VST3EditorInternal {
//-----------------------------------------------------------------------------
class ContextMenuTarget : public Steinberg::FObject, public Steinberg::Vst::IContextMenuTarget
{
public:
	ContextMenuTarget (CCommandMenuItem* item) : item (item)
	{
		item->remember ();
	}
	~ContextMenuTarget () override
	{
		item->forget ();
	}

	Steinberg::tresult PLUGIN_API executeMenuItem (Steinberg::int32 tag) override
	{
		item->execute ();
		return Steinberg::kResultTrue;
	}
	
	OBJ_METHODS(ContextMenuTarget, Steinberg::FObject)
	FUNKNOWN_METHODS(Steinberg::Vst::IContextMenuTarget, Steinberg::FObject)
protected:
	CCommandMenuItem* item;
};

//-----------------------------------------------------------------------------
static void addCOptionMenuEntriesToIContextMenu (VST3Editor* editor, COptionMenu* menu, Steinberg::Vst::IContextMenu* contextMenu)
{
	for (CConstMenuItemIterator it = menu->getItems ()->begin (), end = menu->getItems ()->end (); it != end; ++it)
	{
		auto* commandItem = (*it).cast<CCommandMenuItem>();
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

} // namespace
/// @endcond ignore
#endif

//-----------------------------------------------------------------------------
CMouseEventResult VST3Editor::onMouseDown (CFrame* frame, const CPoint& where, const CButtonState& buttons)
{
	CMouseEventResult result = kMouseEventNotHandled;
	if (buttons.isRightButton ())
	{
		COptionMenu* controllerMenu = (delegate && editingEnabled == false) ? delegate->createContextMenu (where, this) : nullptr;
		if (allowedZoomFactors.empty () == false && editingEnabled == false)
		{
			if (controllerMenu == nullptr)
				controllerMenu = new COptionMenu ();
			else
				controllerMenu->addSeparator ();
			auto* zoomMenu = new COptionMenu ();
			zoomMenu->setStyle (COptionMenu::kMultipleCheckStyle);
			char zoomFactorString[128];
			int32_t zoomFactorTag = 0;
			for (std::vector<double>::const_iterator it = allowedZoomFactors.begin (), end = allowedZoomFactors.end (); it != end; ++it, ++zoomFactorTag)
			{
				sprintf (zoomFactorString, "%d%%", static_cast<int>((*it) * 100));
				CMenuItem* item = zoomMenu->addEntry (new CCommandMenuItem (
				    {zoomFactorString, zoomFactorTag, this, "Zoom", zoomFactorString}));
				if (zoomFactor == *it)
					item->setChecked (true);
			}
			CMenuItem* item = controllerMenu->addEntry ("UI Zoom");
			item->setSubmenu (zoomMenu);
		}
	#if VSTGUI_LIVE_EDITING
		if (editingEnabled == false)
		{
			if (controllerMenu == nullptr)
				controllerMenu = new COptionMenu ();
			else
				controllerMenu->addSeparator ();
			CMenuItem* item = controllerMenu->addEntry (new CCommandMenuItem (
			    {"Open UIDescription Editor", this, "File", "Open UIDescription Editor"}));
			item->setKey ("e", kControl);
		}
	#endif
		CViewContainer::ViewList views;
		if (editingEnabled == false && getFrame ()->getViewsAt (where, views, GetViewOptions (GetViewOptions::kDeep|GetViewOptions::kIncludeViewContainer)))
		{
			for (const auto& view : views)
			{
				auto* contextMenuController = dynamic_cast<IContextMenuController*> (getViewController (view));
				if (contextMenuController == nullptr)
					continue;
				if (controllerMenu == nullptr)
					controllerMenu = new COptionMenu ();
				else
					controllerMenu->addSeparator ();
				CPoint p (where);
				view->frameToLocal (p);
				contextMenuController->appendContextMenuItems (*controllerMenu, p);
			}
		}
	#if VST3_SUPPORTS_CONTEXTMENU
		Steinberg::FUnknownPtr<Steinberg::Vst::IComponentHandler3> handler (getController ()->getComponentHandler ());
		Steinberg::Vst::ParamID paramID;
		if (handler)
		{
			CPoint where2 (where);
			getFrame ()->getTransform ().transform (where2);
			bool paramFound = findParameter ((Steinberg::int32)where2.x, (Steinberg::int32)where2.y, paramID) == Steinberg::kResultTrue;
			Steinberg::Vst::IContextMenu* contextMenu = handler->createContextMenu (this, paramFound ? &paramID : nullptr);
			if (contextMenu)
			{
				if (controllerMenu)
					VST3EditorInternal::addCOptionMenuEntriesToIContextMenu (this, controllerMenu,
					                                                         contextMenu);
				getFrame ()->doAfterEventProcessing ([=] () {
					contextMenu->popup (static_cast<Steinberg::UCoord> (where2.x),
					                    static_cast<Steinberg::UCoord> (where2.y));
					contextMenu->release ();
				});
				result = kMouseEventHandled;
			}
		}
		if (result == kMouseEventNotHandled)
		{
	#endif
			if (controllerMenu)
			{
				controllerMenu->remember ();
				SharedPointer<CFrame> blockFrame = getFrame ();
				getFrame ()->doAfterEventProcessing ([=] () {
					controllerMenu->setStyle (COptionMenu::kPopupStyle |
					                          COptionMenu::kMultipleCheckStyle);
					controllerMenu->popup (blockFrame, where);
					controllerMenu->forget ();
				});
				result = kMouseEventHandled;
			}
		}
		if (controllerMenu)
			controllerMenu->forget ();
	}
	return result;
}

//-----------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API VST3Editor::findParameter (Steinberg::int32 xPos, Steinberg::int32 yPos, Steinberg::Vst::ParamID& resultTag)
{
	std::list<SharedPointer<CView> > views;
	if (frame && getFrame ()->getViewsAt (CPoint (xPos, yPos), views))
	{
		CControl* control = nullptr;
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
			if (pcl && pcl->containsControl (control) && pcl->getParameter ())
			{
				if (delegate && delegate->isPrivateParameter (pcl->getParameterID ()))
					return Steinberg::kResultFalse;
				resultTag = pcl->getParameterID ();
				return Steinberg::kResultTrue;
			}
		}
		Steinberg::Vst::ParamID pid;
		if (delegate && delegate->findParameter (CPoint (xPos, yPos), pid, this) && !delegate->isPrivateParameter (pid))
		{
			resultTag = pid;
			return Steinberg::kResultTrue;
		}
	}
	return Steinberg::kResultFalse;
}

//-----------------------------------------------------------------------------
IController* VST3Editor::createSubController (UTF8StringPtr name, const IUIDescription* desc)
{
	return delegate ? delegate->createSubController (name, desc, this) : nullptr;
}

//-----------------------------------------------------------------------------
CView* VST3Editor::createView (const UIAttributes& attrs, const IUIDescription* desc)
{
	if (delegate)
	{
		const std::string* customViewName = attrs.getAttributeValue (IUIDescription::kCustomViewName);
		if (customViewName)
		{
			CView* view = delegate->createCustomView (customViewName->c_str (), attrs, desc, this);
			return view;
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
CView* VST3Editor::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* desc)
{
	if (delegate)
		view = delegate->verifyView (view, attributes, desc, this);
	auto* control = dynamic_cast<CControl*> (view);
	if (control && control->getTag () != -1 && control->getListener () == this)
	{
		ParameterChangeListener* pcl = getParameterChangeListener (control->getTag ());
		if (pcl)
		{
			pcl->addControl (control);
		}
		else
		{
			Steinberg::Vst::EditController* editController = getController ();
			if (editController)
			{
				Steinberg::Vst::Parameter* parameter = editController->getParameterObject (static_cast<Steinberg::Vst::ParamID> (control->getTag ()));
				paramChangeListeners.insert (std::make_pair (control->getTag (), new ParameterChangeListener (editController, parameter, control)));
			}
		}
	}
	return view;
}

//-----------------------------------------------------------------------------
void VST3Editor::recreateView ()
{
	doCreateView = false;
	enableEditing (editingEnabled);
}

#if LINUX
// Map Steinberg Vst Interface to VSTGUI Interface
class RunLoop : public X11::IRunLoop, public AtomicReferenceCounted
{
public:
	struct EventHandler : Steinberg::Linux::IEventHandler, public Steinberg::FObject
	{
		X11::IEventHandler* handler {nullptr};

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
		X11::ITimerHandler* handler {nullptr};

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

	bool registerEventHandler (int fd, X11::IEventHandler* handler) final
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
	bool unregisterEventHandler (X11::IEventHandler* handler) final
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
	bool registerTimer (uint64_t interval, X11::ITimerHandler* handler) final
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
	bool unregisterTimer (X11::ITimerHandler* handler) final
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
#endif

#define kFrameEnableFocusDrawingAttr "frame-enable-focus-drawing"
#define kFrameFocusColorAttr "frame-focus-color"
#define kFrameFocusWidthAttr "frame-focus-width"

#if VSTGUI_LIVE_EDITING
// keyboard hook
struct VST3Editor::KeyboardHook : public IKeyboardHook
{
public:
	using Func = std::function<int32_t (const VstKeyCode& code, CFrame* frame)>;

	KeyboardHook (Func&& keyDown, Func&& keyUp)
	: onKeyDownFunc (std::move (keyDown)), onKeyUpFunc (std::move (keyUp))
	{
	}

private:
	int32_t onKeyDown (const VstKeyCode& code, CFrame* frame) override
	{
		return onKeyDownFunc (code, frame);
	}
	int32_t onKeyUp (const VstKeyCode& code, CFrame* frame) override
	{
		return onKeyUpFunc (code, frame);
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
	frame = new CFrame (CRect (0, 0, 0, 0), this);
	getFrame ()->setViewAddedRemovedObserver (this);
	getFrame ()->setTransparency (true);
	getFrame ()->registerMouseObserver (this);
#if VSTGUI_LIVE_EDITING
	// will delete itself when the frame will be destroyed
	keyboardHook = new KeyboardHook (
	    [this] (const VstKeyCode& code, CFrame* frame) {
		    if (code.modifier == MODIFIER_CONTROL && frame->getModalView () == nullptr)
		    {
			    if (code.character == 'e')
			    {
				    enableEditing (!editingEnabled);
				    return 1;
			    }
		    }
		    return -1;
	    },
	    [] (const VstKeyCode&, CFrame*) { return -1; });
	getFrame ()->registerKeyboardHook (keyboardHook);
#endif
	getFrame ()->enableTooltips (tooltipsEnabled);

	if (!enableEditing (false))
	{
		getFrame ()->forget ();
		return false;
	}

	IPlatformFrameConfig* config = nullptr;
#if LINUX
	X11::FrameConfig x11config;
	x11config.runLoop = owned (new RunLoop (plugFrame));
	config = &x11config;
#endif

	getFrame ()->open (parent, type, config);

	if (delegate)
		delegate->didOpen (this);

	Steinberg::IdleUpdateHandler::start ();

	return true;
}

//-----------------------------------------------------------------------------
void PLUGIN_API VST3Editor::close ()
{
	Steinberg::IdleUpdateHandler::stop ();

	if (delegate)
		delegate->willClose (this);

	for (ParameterChangeListenerMap::const_iterator it = paramChangeListeners.begin (), end = paramChangeListeners.end (); it != end; ++it)
		it->second->release ();

	paramChangeListeners.clear ();
	if (frame)
	{

#if VSTGUI_LIVE_EDITING
		if (keyboardHook)
		{
			getFrame ()->unregisterKeyboardHook (keyboardHook);
			delete keyboardHook;
		}
		keyboardHook = nullptr;
#endif
		getFrame ()->unregisterMouseObserver (this);
		getFrame ()->removeAll (true);
		int32_t refCount = getFrame ()->getNbReference ();
		if (refCount == 1)
		{
			getFrame ()->close ();
			frame = nullptr;
		}
		else
		{
			getFrame ()->forget ();
		}
	}
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API VST3Editor::onSize (Steinberg::ViewRect* newSize)
{
	if (getFrame ())
	{
		CRect r (newSize->left, newSize->top, newSize->right, newSize->bottom);
		CRect currentSize;
		getFrame ()->getSize (currentSize);
		if (r == currentSize)
			return Steinberg::kResultTrue;
	}
	return VSTGUIEditor::onSize (newSize);
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
	if (editingEnabled)
		return Steinberg::kResultTrue;
#endif
	double scaleFactor = getAbsScaleFactor ();
	CCoord width = rect->right - rect->left;
	CCoord height = rect->bottom - rect->top;
	if (width < minSize.x * scaleFactor)
		width = minSize.x * scaleFactor;
	else if (width > maxSize.x * scaleFactor)
		width = maxSize.x * scaleFactor;
	if (height < minSize.y * scaleFactor)
		height = minSize.y * scaleFactor;
	else if (height > maxSize.y * scaleFactor)
		height = maxSize.y * scaleFactor;
	if (width != rect->right - rect->left || height != rect->bottom - rect->top)
	{
		rect->right = (Steinberg::int32)width + rect->left;
		rect->bottom = (Steinberg::int32)height + rect->top;
	}
	return Steinberg::kResultTrue;
}

//------------------------------------------------------------------------
bool VST3Editor::validateCommandMenuItem (CCommandMenuItem* item)
{
#if VSTGUI_LIVE_EDITING
	if (item->getCommandCategory () == "File")
	{
		if (item->getCommandName () == "Save")
		{
			bool enable = false;
			UIAttributes* attributes = description->getCustomAttributes ("VST3Editor", true);
			if (attributes)
			{
				const std::string* filePath = attributes->getAttributeValue ("Path");
				if (filePath)
				{
					enable = true;
				}
			}
			item->setEnabled (enable);
			return true;
		}
	}
#endif
	return false;
}

//------------------------------------------------------------------------
bool VST3Editor::onCommandMenuItemSelected (CCommandMenuItem* item)
{
	auto& cmdCategory = item->getCommandCategory ();
#if VSTGUI_LIVE_EDITING
	auto& cmdName = item->getCommandName ();
	if (cmdCategory == "Edit")
	{
		if (cmdName == "Sync Parameter Tags")
		{
			syncParameterTags ();
			return true;
		}
	}
	else if (cmdCategory == "File")
	{
		if (cmdName == "Open UIDescription Editor")
		{
			editingEnabled = true;
			doCreateView = true;
			return true;
		}
		else if (cmdName == "Close UIDescription Editor")
		{
			editingEnabled = false;
			doCreateView = true;
			return true;
		}
		else if (cmdName == "Save")
		{
			save (false);
			item->setChecked (false);
			return true;
		}
		else if (cmdName == "Save As")
		{
			save (true);
			item->setChecked (false);
			return true;
		}
	}
	else
#endif
	    if (cmdCategory == "Zoom")
	{
		size_t index = static_cast<size_t> (item->getTag ());
		if (index < allowedZoomFactors.size ())
		{
			setZoomFactor (allowedZoomFactors[index]);
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
CMessageResult VST3Editor::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == CVSTGUITimer::kMsgTimer)
	{
		if (doCreateView)
			recreateView ();
 	}
 	return VSTGUIEditor::notify (sender, message);
}

namespace VST3EditorInternal {
//------------------------------------------------------------------------
static int32_t getUIDescriptionSaveOptions (CFrame* frame)
{
	int32_t flags = 0;
#if VSTGUI_LIVE_EDITING
	auto* editController = dynamic_cast<UIEditController*> (getViewController (frame->getView (0)));
	if (editController)
	{
		UIAttributes* attributes = editController->getSettings ();
		bool val;
		if (attributes->getBooleanAttribute (UIEditController::kEncodeBitmapsSettingsKey, val) && val == true)
		{
			flags |= UIDescription::kWriteImagesIntoXMLFile;
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
	UIAttributes* attributes = description->getCustomAttributes ("VST3Editor", true);
	vstgui_assert(attributes);
	std::string savePath;
	if (saveAs)
	{
		CNewFileSelector* fileSelector = CNewFileSelector::create (frame, CNewFileSelector::kSelectSaveFile);
		if (fileSelector == nullptr)
			return;
		fileSelector->setTitle ("Save UIDescription File");
		fileSelector->setDefaultExtension (CFileExtension ("VSTGUI UI Description", "uidesc"));
		const std::string* prevFilePath = attributes->getAttributeValue ("Path");
		if (prevFilePath)
			fileSelector->setInitialDirectory (prevFilePath->c_str ());
		else if (!xmlFile.empty ())
		{
			if (xmlFile[0] == '/')
				fileSelector->setInitialDirectory (xmlFile.c_str ());
			else
				fileSelector->setDefaultSaveName (xmlFile.c_str ());
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
		fileSelector->forget ();
	}
	else
	{
		const std::string* filePath = attributes->getAttributeValue ("Path");
		if (filePath)
			savePath = *filePath;
	}
	if (savePath.empty ())
		return;
	if (description->save (savePath.c_str (), VST3EditorInternal::getUIDescriptionSaveOptions (frame)))
		description->setFilePath (savePath.c_str ());
}

//------------------------------------------------------------------------
void VST3Editor::syncParameterTags ()
{
#if VSTGUI_LIVE_EDITING
	CView* view = getFrame ()->getView (0);
	if (view)
	{
		IController* controller = getViewController (view);
		IActionPerformer* actionPerformer = controller ? dynamic_cast<IActionPerformer*>(controller) : nullptr;
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
					if (int32_t tag = description->getTagForName (paramTitle) != -1)
					{
						if (tag != info.id)
							actionPerformer->performTagChange (paramTitle, paramIDStr);
					}
					else if (UTF8StringPtr tagName = description->lookupControlTagName (static_cast<int32_t> (info.id)))
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
bool VST3Editor::enableEditing (bool state)
{
	if (getFrame ())
	{
		getFrame ()->removeAll ();

	#if VSTGUI_LIVE_EDITING
		if (state)
		{
			// update uiDesc file path to absolute if possible
			if (UIAttributes* attributes = description->getCustomAttributes ("VST3Editor", true))
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
						description->setFilePath (filePath->c_str ());
				}
			}
			
			getFrame ()->setTransform (CGraphicsTransform ());
			nonEditRect = getFrame ()->getViewSize ();
			description->setController (this);
			auto* editController = new UIEditController (description);
			CView* view = editController->createEditView ();
			if (view)
			{
				editingEnabled = true;
				CCoord width = view->getWidth ();
				CCoord height = view->getHeight ();

				getFrame ()->setSize (width, height);
				getFrame ()->addView (view);
				getFrame()->setZoom (contentScaleFactor);

				getFrame ()->enableTooltips (true);
				CColor focusColor = kBlueCColor;
				editController->getEditorDescription ()->getColor ("focus", focusColor);
				getFrame ()->setFocusColor (focusColor);
				getFrame ()->setFocusDrawingEnabled (true);
				getFrame ()->setFocusWidth (1);
				
				COptionMenu* fileMenu = editController->getMenuController ()->getFileMenu ();
				if (fileMenu)
				{
					CMenuItem* item = fileMenu->addEntry (
					    new CCommandMenuItem ({"Save", this, "File", "Save"}), 0);
					item->setKey ("s", kControl);
					item = fileMenu->addEntry (
					    new CCommandMenuItem ({"Save As..", this, "File", "Save As"}), 1);
					item->setKey ("s", kShift | kControl);
					item = fileMenu->addEntry (new CCommandMenuItem (
					    {"Close Editor", this, "File", "Close UIDescription Editor"}));
					item->setKey ("e", kControl);
				}
				COptionMenu* editMenu = editController->getMenuController ()->getEditMenu ();
				if (editMenu)
				{
					editMenu->addSeparator ();
					editMenu->addEntry (new CCommandMenuItem (
					    {"Sync Parameter Tags", this, "Edit", "Sync Parameter Tags"}));
				}
				return true;
			}
			editController->forget ();
		}
		else
	#endif
		{
			editingEnabled = false;
			CView* view = description->createView (viewName.c_str (), this);
			if (view)
			{
				double scaleFactor = getAbsScaleFactor ();
				CCoord width = view->getWidth () * scaleFactor;
				CCoord height = view->getHeight () * scaleFactor;

				if (canResize () == Steinberg::kResultTrue)
				{
					Steinberg::ViewRect tmp;
					if (getRect ().getWidth () != width)
						tmp.right = getRect ().getWidth ();
					if (getRect ().getHeight () != height)
						tmp.bottom = getRect ().getHeight ();
					if (tmp.getWidth () && tmp.getHeight ())
					{
						checkSizeConstraint (&tmp);
						nonEditRect.setWidth (tmp.getWidth ());
						nonEditRect.setHeight (tmp.getHeight ());
					}
				}

				getFrame ()->setSize (width, height);
				getFrame ()->addView (view);
				getFrame ()->setTransform (CGraphicsTransform ().scale (scaleFactor, scaleFactor));
				getFrame ()->invalid ();
				if (nonEditRect.isEmpty () == false)
				{
					rect.right = rect.left + (Steinberg::int32)nonEditRect.getWidth ();
					rect.bottom = rect.top + (Steinberg::int32)nonEditRect.getHeight ();
					plugFrame->resizeView (this, &rect);
				}
				else
				{
					checkSizeConstraint (&rect);
					onSize (&rect);
					requestResize (CPoint (rect.getWidth (), rect.getHeight ()));
				}

				getFrame ()->setFocusDrawingEnabled (false);

				// focus drawing support
				UIAttributes* attributes = description->getCustomAttributes ("FocusDrawing", true);

				// map old one
				UIAttributes* oldAttributes = description->getCustomAttributes ("VST3Editor");
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
						if (description->getColor (attr->c_str (), focusColor))
							getFrame ()->setFocusColor (focusColor);
					}
					attr = attributes->getAttributeValue ("width");
					if (attr)
					{
						double focusWidth = UTF8StringView (attr->c_str ()).toDouble ();
						getFrame ()->setFocusWidth (focusWidth);
					}
				}
				return true;
			}
		}
	}
	return false;
}

} // namespace
