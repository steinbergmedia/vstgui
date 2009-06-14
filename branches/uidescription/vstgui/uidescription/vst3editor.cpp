
#include "vst3editor.h"
#include "ceditframe.h"
#include "base/source/updatehandler.h"
#include "base/source/fstring.h"
#include "namingdialog.h"
#include <list>

BEGIN_NAMESPACE_VSTGUI

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
	}

	~ParameterChangeListener ()
	{
		if (parameter)
		{
			parameter->removeDependent (this);
			parameter->release ();
		}
		std::list<CControl*>::iterator it = controls.begin ();
		while (it != controls.end ())
		{
			(*it)->forget ();
			it++;
		}
	}

	void addControl (CControl* control)
	{
		control->remember ();
		controls.push_back (control);
		Steinberg::Vst::ParamValue value = 0.;
		if (parameter)
		{
			value = parameter->getNormalized ();
		}
		else
		{
			CControl* control = controls.front ();
			if (control)
				value = control->getValue ();
		}
		CParamDisplay* display = dynamic_cast<CParamDisplay*> (control);
		if (display)
			display->setStringConvert (stringConvert, this);

		COptionMenu* optMenu = dynamic_cast<COptionMenu*> (control);
		if (optMenu && parameter && parameter->getInfo ().stepCount > 0)
		{
			for (Steinberg::int32 i = 0; i <= parameter->getInfo ().stepCount; i++)
			{
				Steinberg::Vst::String128 utf16Str;
				parameter->toString ((Steinberg::Vst::ParamValue)i / (Steinberg::Vst::ParamValue)parameter->getInfo ().stepCount, utf16Str);
				Steinberg::String utf8Str (utf16Str);
				utf8Str.toMultiByte (Steinberg::kCP_Utf8);
				optMenu->addEntry (utf8Str);
			}
		}

		updateControlValue (value);
	}
	
	void PLUGIN_API update (FUnknown* changedUnknown, Steinberg::int32 message)
	{
		if (message == IDependent::kChanged && parameter)
		{
			updateControlValue (parameter->getNormalized ());
		}
	}

	Steinberg::Vst::ParamID getParameterID () 
	{
		if (parameter)
			return parameter->getInfo ().id;
		CControl* control = controls.front ();
		if (control)
			return control->getTag ();
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
	}
	
	void performEdit (Steinberg::Vst::ParamValue value)
	{
		if (parameter)
		{
			editController->setParamNormalized (getParameterID (), value);
			editController->performEdit (getParameterID (), value);
		}
		else
		{
			updateControlValue (value);
		}
	}
	Steinberg::Vst::Parameter* getParameter () const { return parameter; }

protected:
	void convertValueToString (float value, char* string)
	{
		if (parameter)
		{
			Steinberg::Vst::String128 utf16Str;
			parameter->toString (value, utf16Str);
			Steinberg::String utf8Str (utf16Str);
			utf8Str.toMultiByte (Steinberg::kCP_Utf8);
			utf8Str.copyTo8 (string, 0, 256);
		}
	}

	static void stringConvert (float value, char* string, void* userDta)
	{
		ParameterChangeListener* This = (ParameterChangeListener*)userDta;
		This->convertValueToString (value, string);
	}
	
	void updateControlValue (Steinberg::Vst::ParamValue value)
	{
		std::list<CControl*>::iterator it = controls.begin ();
		while (it != controls.end ())
		{
			COptionMenu* optMenu = dynamic_cast<COptionMenu*> (*it);
			if (optMenu)
			{
				if (parameter)
					optMenu->setValue (parameter->toPlain (value));
			}
			else
				(*it)->setValue (value);
			(*it)->invalid ();
			it++;
		}
	}
	Steinberg::Vst::EditController* editController;
	Steinberg::Vst::Parameter* parameter;
	std::list<CControl*> controls;
};

//-----------------------------------------------------------------------------
static bool parseSize (const std::string& str, CPoint& point)
{
	size_t sep = str.find (',', 0);
	if (sep != std::string::npos)
	{
		point.x = strtol (str.c_str (), 0, 10);
		point.y = strtol (str.c_str () + sep+1, 0, 10);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
VST3Editor::VST3Editor (void* controller, const char* _viewName, const char* _xmlFile, bool debugMode)
: VSTGUIEditor (controller)
, debugMode (debugMode)
, reloadDescriptionTag (-1)
, doReloadDescription (false)
, doCreateView (false)
, tooltipSupport (0)
, tooltipsEnabled (true)
{
	Steinberg::UpdateHandler::instance ();
	description = new UIDescription (_xmlFile);
	viewName = _viewName;
	xmlFile = _xmlFile;
	init ();
}

//-----------------------------------------------------------------------------
VST3Editor::VST3Editor (UIDescription* desc, void* controller, const char* _viewName, const char* _xmlFile, bool debugMode)
: VSTGUIEditor (controller)
, debugMode (debugMode)
, reloadDescriptionTag (-1)
, doReloadDescription (false)
, doCreateView (false)
, tooltipSupport (0)
, tooltipsEnabled (true)
{
	Steinberg::UpdateHandler::instance ();
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
void VST3Editor::init ()
{
	if (description->parse ())
	{
		// get sizes
		const UIAttributes* attr = description->getViewAttributes (viewName.c_str ());
		if (attr)
		{
			const std::string* sizeStr = attr->getAttributeValue ("size");
			const std::string* minSizeStr = attr->getAttributeValue ("minSize");
			const std::string* maxSizeStr = attr->getAttributeValue ("maxSize");
			if (sizeStr)
			{
				CPoint p;
				if (parseSize (*sizeStr, p))
				{
					rect.right = (Steinberg::int32)p.x;
					rect.bottom = (Steinberg::int32)p.y;
					minSize = p;
					maxSize = p;
				}
			}
			if (minSizeStr)
				parseSize (*minSizeStr, minSize);
			if (maxSizeStr)
				parseSize (*maxSizeStr, maxSize);
		}
		#if DEBUG
		else
		{
			UIAttributes* attr = new UIAttributes ();
			attr->setAttribute ("class", "CViewContainer");
			attr->setAttribute ("size", "300, 300");
			description->addNewTemplate (viewName.c_str (), attr);
			rect.right = 300;
			rect.bottom = 300;
		}
		#endif
	}
	#if DEBUG
	else
	{
		UIAttributes* attr = new UIAttributes ();
		attr->setAttribute ("class", "CViewContainer");
		attr->setAttribute ("size", "300, 300");
		description->addNewTemplate (viewName.c_str (), attr);
		rect.right = 300;
		rect.bottom = 300;
	}
	#endif
}

//-----------------------------------------------------------------------------
bool VST3Editor::verify ()
{
	return description->parse ();
}

//-----------------------------------------------------------------------------
bool VST3Editor::exchangeView (const char* newViewName)
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
	if (state)
	{
		if (frame && !tooltipSupport)
			tooltipSupport = new CTooltipSupport (frame);
	}
	else
	{
		if (tooltipSupport)
		{
			tooltipSupport->forget ();
			tooltipSupport = 0;
		}
	}
}

//-----------------------------------------------------------------------------
ParameterChangeListener* VST3Editor::getParameterChangeListener (long tag)
{
	if (tag != -1)
	{
		std::map<long, ParameterChangeListener*>::iterator it = paramChangeListeners.find (tag);
		if (it != paramChangeListeners.end ())
		{
			return it->second;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
void VST3Editor::valueChanged (CControl* pControl)
{
	ParameterChangeListener* pcl = getParameterChangeListener (pControl->getTag ());
	if (pcl)
	{
		Steinberg::Vst::ParamValue value = pControl->getValue ();
		CTextEdit* textEdit = dynamic_cast<CTextEdit*> (pControl);
		if (textEdit && pcl->getParameter ())
		{
			Steinberg::String str (textEdit->getText ());
			str.toWideString (Steinberg::kCP_Utf8);
			if (getController ()->getParamValueByString (pcl->getParameterID (), (Steinberg::Vst::TChar*)str.text16 (), value) == Steinberg::kResultFalse)
			{
				pcl->changed ();
				return;
			}
		}
		COptionMenu* optMenu = dynamic_cast<COptionMenu*> (pControl);
		if (optMenu && pcl->getParameter ())
		{
			//need to convert to normalized from plain
			value = pcl->getParameter ()->toNormalized (value);
		}
		pcl->performEdit (value);
	}
	if (reloadDescriptionTag != -1 && pControl->getTag () == reloadDescriptionTag)
	{
		doReloadDescription = true;
	}
}

//-----------------------------------------------------------------------------
void VST3Editor::beginEdit (long index)
{
	// we don't assume that every control tag is a parameter tag handled by this editor
	// as sub classes could build custom CControlListeners for controls
}

//-----------------------------------------------------------------------------
void VST3Editor::endEdit (long index)
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
CView* VST3Editor::verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
{
	CControl* control = dynamic_cast<CControl*> (view);
	if (control && control->getTag () != -1 && control->getListener () == this)
	{
		if (reloadDescriptionTag != -1 && !debugMode && control->getTag () == reloadDescriptionTag)
		{
			view->forget ();
			return 0;
		}
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
				Steinberg::Vst::Parameter* parameter = editController->getParameter (control->getTag ());
				paramChangeListeners.insert (std::make_pair (control->getTag (), new ParameterChangeListener (editController, parameter, control)));
			}
		}
	}
	return view;
}

//-----------------------------------------------------------------------------
void VST3Editor::reloadDescription ()
{
	doReloadDescription = false;
	UIDescription* newDescription = new UIDescription (xmlFile.c_str ());
	if (!newDescription->parse ())
	{
		newDescription->forget ();
		return;
	}
	frame->removeAll (true);
	description->forget ();
	description = newDescription;
	#if VSTGUI_LIVE_EDITING
	dynamic_cast<CEditFrame*> (frame)->setUIDescription (description);
	#endif
	recreateView ();
}

//-----------------------------------------------------------------------------
void VST3Editor::recreateView ()
{
	doCreateView = false;
	frame->remember ();
	close ();

	CView* view = description->createView (viewName.c_str (), this);
	if (view)
	{
		if (plugFrame)
		{
			rect.right = rect.left + (Steinberg::int32)view->getWidth ();
			rect.bottom = rect.top + (Steinberg::int32)view->getHeight ();
			plugFrame->resizeView (this, &rect);
		}
		else
		{
			frame->setSize (view->getWidth (), view->getHeight ());
		}
		frame->addView (view);
		if (tooltipsEnabled)
			tooltipSupport = new CTooltipSupport (frame);
	}
	init ();
	frame->invalid ();
}

//-----------------------------------------------------------------------------
bool PLUGIN_API VST3Editor::open (void* parent)
{
	reloadDescriptionTag = description->getTagForName (kVST3EditorReloadDescriptionTagName);
	CView* view = description->createView (viewName.c_str (), this);
	if (view)
	{
		#if VSTGUI_LIVE_EDITING
		if (debugMode)
			frame = new CEditFrame (view->getViewSize (), parent, this, CEditFrame::kEditMode, 0, description, viewName.c_str ());
		else
		#endif
			frame = new CFrame (view->getViewSize (), parent, this);
		frame->setTransparency (true);
		frame->addView (view);
		CRect size (rect.left, rect.top, rect.right, rect.bottom);
		frame->setSize (size.getWidth (), size.getHeight ());
		if (tooltipsEnabled)
			tooltipSupport = new CTooltipSupport (frame);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void PLUGIN_API VST3Editor::close ()
{
	std::map<long, ParameterChangeListener*>::iterator it = paramChangeListeners.begin ();
	while (it != paramChangeListeners.end ())
	{
		it->second->release ();
		it++;
	}
	paramChangeListeners.clear ();
	if (tooltipSupport)
	{
		tooltipSupport->forget ();
		tooltipSupport = 0;
	}
	if (frame)
	{
		frame->removeAll (true);
		frame->forget ();
	}
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API VST3Editor::onSize (Steinberg::ViewRect* newSize)
{
	return VSTGUIEditor::onSize (newSize);
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API VST3Editor::canResize ()
{
	return (minSize == maxSize) ? Steinberg::kResultFalse : Steinberg::kResultTrue;
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API VST3Editor::checkSizeConstraint (Steinberg::ViewRect* rect)
{
	CCoord width = rect->right - rect->left;
	CCoord height = rect->bottom - rect->top;
	if (width < minSize.x)
		width = minSize.x;
	else if (width > maxSize.x)
		width = maxSize.x;
	if (height < minSize.y)
		height = minSize.y;
	else if (height > maxSize.y)
		height = maxSize.y;
	if (width != rect->right - rect->left || height != rect->bottom - rect->top)
	{
		rect->right = (Steinberg::int32)width + rect->left;
		rect->bottom = (Steinberg::int32)height + rect->top;
	}
	return Steinberg::kResultTrue;
}

//------------------------------------------------------------------------
CMessageResult VST3Editor::notify (CBaseObject* sender, const char* message)
{
	if (message == CVSTGUITimer::kMsgTimer)
	{
		if (doReloadDescription)
			reloadDescription ();
		if (doCreateView)
			recreateView ();
 	}
	#if VSTGUI_LIVE_EDITING
	else if (message == CEditFrame::kMsgShowOptionsMenu)
	{
		COptionMenu* menu = dynamic_cast<COptionMenu*> (sender);
		if (menu)
		{
			std::list<const std::string*> templateNames;
			description->collectTemplateViewNames (templateNames);
			if (templateNames.size () > 0)
			{
				menu->addSeparator ();
				COptionMenu* submenu = new COptionMenu ();
				long menuTag = 1000;
				std::list<const std::string*>::const_iterator it = templateNames.begin ();
				while (it != templateNames.end ())
				{
					submenu->addEntry (new CMenuItem ((*it)->c_str (), menuTag++));
					it++;
				}
				menu->addEntry (submenu, "Change Template");
				submenu->forget ();

				ViewFactory* viewFactory = dynamic_cast<ViewFactory*> (description->getViewFactory ());
				if (viewFactory)
				{
					std::list<const std::string*> viewNames;
					viewFactory->collectRegisteredViewNames (viewNames, "CViewContainer");
					if (viewNames.size () > 0)
					{
						submenu = new COptionMenu ();
						CMenuItem* item = submenu->addEntry ("Root View Type");
						item->setIsTitle (true);
						menuTag = 10000;
						std::list<const std::string*>::const_iterator it = viewNames.begin ();
						while (it != viewNames.end ())
						{
							submenu->addEntry (new CMenuItem ((*it)->c_str (), menuTag++));
							it++;
						}
						menu->addEntry (submenu, "Add New Template");
						submenu->forget ();
					}
				}
			}
		}
		return kMessageNotified;
	}
	else if (message == CEditFrame::kMsgPerformOptionsMenuAction)
	{
		CMenuItem* item = dynamic_cast<CMenuItem*> (sender);
		if (item)
		{
			long index = item->getTag ();
			if (index >= 10000)
			{
				std::string templateTitle ("NewTemplate");
				if (NamingDialog::askForName (templateTitle, "Choose a template name") && templateTitle.length () > 0)
				{
					UIAttributes* attr = new UIAttributes ();
					attr->setAttribute ("class", item->getTitle ());
					attr->setAttribute ("size", "300, 300");
					description->addNewTemplate (templateTitle.c_str (), attr);
					exchangeView (templateTitle.c_str ());
				}
			}
			else
			{
				exchangeView (item->getTitle ());
			}
		}
		return kMessageNotified;
	}
	else if (message == CEditFrame::kMsgEditEnding)
	{
		exchangeView (viewName.c_str ());
		return kMessageNotified;
	}
	#endif
 	return VSTGUIEditor::notify (sender, message); 
}


END_NAMESPACE_VSTGUI
