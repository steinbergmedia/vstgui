//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2009, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "vst3editor.h"
#include "../lib/vstkeycode.h"
#include "ceditframe.h"
#include "base/source/updatehandler.h"
#include "base/source/fstring.h"
#include "base/source/timer.h"
#include "pluginterfaces/base/keycodes.h"
#include "dialog.h"
#include <list>
#include <sstream>

namespace Steinberg {

//-----------------------------------------------------------------------------
class IdleUpdateHandler : public FObject, public ITimerCallback
{
public:
	OBJ_METHODS (IdleUpdateHandler, FObject)
	SINGLETON (IdleUpdateHandler)
protected:
	IdleUpdateHandler () { UpdateHandler::instance (); timer = Timer::create (this, 1000/30); } // 30 Hz timer
	~IdleUpdateHandler () { timer->release (); }
	void onTimer (Timer* timer)
	{
		UpdateHandler::instance ()->triggerDeferedUpdates ();
	}

	Steinberg::Timer* timer;
};

} // namespace Steinberg

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
			value = editController->getParamNormalized (getParameterID ());
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
				editController->getParamStringByValue (getParameterID (), (Steinberg::Vst::ParamValue)i / (Steinberg::Vst::ParamValue)parameter->getInfo ().stepCount, utf16Str);
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
			updateControlValue (editController->getParamNormalized (getParameterID ()));
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
			editController->getParamStringByValue (getParameterID (), value, utf16Str);
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
					optMenu->setValue (editController->normalizedParamToPlain (getParameterID (), value));
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
, doCreateView (false)
, tooltipSupport (0)
, tooltipsEnabled (true)
{
	description = new UIDescription (_xmlFile);
	viewName = _viewName;
	xmlFile = _xmlFile;
	init ();
}

//-----------------------------------------------------------------------------
VST3Editor::VST3Editor (UIDescription* desc, void* controller, const char* _viewName, const char* _xmlFile, bool debugMode)
: VSTGUIEditor (controller)
, debugMode (debugMode)
, doCreateView (false)
, tooltipSupport (0)
, tooltipsEnabled (true)
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
	QUERY_INTERFACE(iid, obj, Steinberg::Vst::IParameterFinder::iid, Steinberg::Vst::IParameterFinder)
	return VSTGUIEditor::queryInterface (iid, obj);
}

//-----------------------------------------------------------------------------
void VST3Editor::init ()
{
	Steinberg::IdleUpdateHandler::instance ();
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
			minSize (rect.right, rect.bottom);
			maxSize (rect.right, rect.bottom);
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
		minSize (rect.right, rect.bottom);
		maxSize (rect.right, rect.bottom);
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
			if (getController ()->getParamValueByString (pcl->getParameterID (), (Steinberg::Vst::TChar*)str.text16 (), value) != Steinberg::kResultTrue)
			{
				pcl->changed ();
				return;
			}
		}
		COptionMenu* optMenu = dynamic_cast<COptionMenu*> (pControl);
		if (optMenu && pcl->getParameter ())
		{
			//need to convert to normalized from plain
			value = getController ()->plainParamToNormalized (pcl->getParameterID (), value);
		}
		pcl->performEdit (value);
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
Steinberg::tresult PLUGIN_API VST3Editor::findParameter (Steinberg::int32 xPos, Steinberg::int32 yPos, Steinberg::Vst::ParamID& resultTag)
{
	CView* view = frame->getViewAt (CPoint (xPos, yPos), true);
	if (view)
	{
		CControl* control = dynamic_cast<CControl*> (view);
		if (control && control->getTag () != -1)
		{
			ParameterChangeListener* pcl = getParameterChangeListener (control->getTag ());
			if (pcl)
			{
				resultTag = pcl->getParameterID ();
				return Steinberg::kResultTrue;
			}
		}
		VST3EditorDelegate* delegate = dynamic_cast<VST3EditorDelegate*> (getController ());
		if (delegate)
		{
			return (delegate->findParameter (CPoint (xPos, yPos), resultTag, this) ? Steinberg::kResultTrue : Steinberg::kResultFalse);
		}
	}
	return Steinberg::kResultFalse;
}

//-----------------------------------------------------------------------------
CView* VST3Editor::createView (const UIAttributes& attributes, IUIDescription* description)
{
	const std::string* customViewName = attributes.getAttributeValue ("custom-view-name");
	if (customViewName)
	{
		VST3EditorDelegate* delegate = dynamic_cast<VST3EditorDelegate*> (getController ());
		if (delegate)
			return delegate->createCustomView (customViewName->c_str (), attributes, description);
	}
	return 0;
}

//-----------------------------------------------------------------------------
CView* VST3Editor::verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
{
	CControl* control = dynamic_cast<CControl*> (view);
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
				Steinberg::Vst::Parameter* parameter = editController->getParameterObject (control->getTag ());
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
	CView* view = description->createView (viewName.c_str (), this);
	if (view)
	{
		#if VSTGUI_LIVE_EDITING
		if (debugMode)
		{
			frame = new CEditFrame (view->getViewSize (), parent, this, CEditFrame::kNoEditMode, 0, description, viewName.c_str ());
		}
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
				menu->addEntry (new CMenuItem ("Template Settings..."));
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
			if (item->getTitle () == std::string ("Template Settings..."))
			{
				runTemplateSettingsDialog ();
			}
			else
			{
				long index = item->getTag ();
				if (index >= 10000)
				{
					runNewTemplateDialog (item->getTitle ());
				}
				else
				{
					exchangeView (item->getTitle ());
				}
			}
		}
		return kMessageNotified;
	}
	else if (message == CEditFrame::kMsgEditEnding)
	{
		exchangeView (viewName.c_str ());
		return kMessageNotified;
	}
	else if (message == kMsgViewSizeChanged)
	{
		if (plugFrame)
		{
			rect.right = rect.left + (Steinberg::int32)frame->getWidth ();
			rect.bottom = rect.top + (Steinberg::int32)frame->getHeight ();
			plugFrame->resizeView (this, &rect);
		}
	}
	#endif
 	return VSTGUIEditor::notify (sender, message); 
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API VST3Editor::onWheel (float distance)
{
	if (frame)
	{
		CPoint where;
		frame->getCurrentMouseLocation (where);
		if (frame->onWheel (where, distance, frame->getCurrentMouseButtons ()))
			return Steinberg::kResultTrue;
	}
	return Steinberg::kResultFalse;
}

//------------------------------------------------------------------------
static bool translateKeyMessage (VstKeyCode& keyCode, Steinberg::char16 key, Steinberg::int16 keyMsg, Steinberg::int16 modifiers)
{
	keyCode.character = 0;
	keyCode.virt = keyMsg;
	keyCode.modifier = 0;
	if (key == 0)
		key = Steinberg::VirtualKeyCodeToChar (keyMsg);
	if (key)
	{
		Steinberg::String keyStr (STR(" "));
		keyStr.setChar16 (0, key);
		keyStr.toMultiByte (Steinberg::kCP_Utf8);
		if (keyStr.length () == 1)
			keyCode.character = keyStr.getChar8 (0);
	}
	if (modifiers)
	{
		if (modifiers & Steinberg::kShiftKey)
			keyCode.modifier |= MODIFIER_SHIFT;
		if (modifiers & Steinberg::kAlternateKey)
			keyCode.modifier |= MODIFIER_ALTERNATE;
		if (modifiers & Steinberg::kCommandKey)
			keyCode.modifier |= MODIFIER_CONTROL;
		if (modifiers & Steinberg::kControlKey)
			keyCode.modifier |= MODIFIER_COMMAND;
	}
	return true;
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API VST3Editor::onKeyDown (Steinberg::char16 key, Steinberg::int16 keyMsg, Steinberg::int16 modifiers)
{
	if (frame)
	{
		VstKeyCode keyCode = {0};
		if (translateKeyMessage (keyCode, key, keyMsg, modifiers))
		{
			long result = frame->onKeyDown (keyCode);
			if (result == 1)
				return Steinberg::kResultTrue;
		}
	}
	return Steinberg::kResultFalse;
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API VST3Editor::onKeyUp (Steinberg::char16 key, Steinberg::int16 keyMsg, Steinberg::int16 modifiers)
{
	if (frame)
	{
		VstKeyCode keyCode = {0};
		if (translateKeyMessage (keyCode, key, keyMsg, modifiers))
		{
			long result = frame->onKeyUp (keyCode);
			if (result == 1)
				return Steinberg::kResultTrue;
		}
	}
	return Steinberg::kResultFalse;
}

#if VSTGUI_LIVE_EDITING
static const CCoord kMargin = 10;
static const CCoord kControlHeight = 22;
//------------------------------------------------------------------------
void VST3Editor::runNewTemplateDialog (const char* baseViewName)
{
	CRect size (0, 0, 300+kMargin, kControlHeight*3+kMargin*2);
	CViewContainer* c = new CViewContainer (size, 0);
	c->setTransparency (true);
	size (0, 0, 80, kControlHeight);
	CTextLabel* label = new CTextLabel (size, "Name:");
	label->setStyle (kNoFrame);
	label->setBackColor (kWhiteCColor);
	label->setFontColor (kBlackCColor);
	label->setTextInset (CPoint (5, 0));
	label->setHoriAlign (kRightText);
	c->addView (label);
	size.offset (0, kMargin+kControlHeight);
	label = new CTextLabel (*label);
	label->setViewSize (size);
	label->setText ("Width:");
	c->addView (label);
	size.offset (0, kMargin+kControlHeight);
	label = new CTextLabel (*label);
	label->setViewSize (size);
	label->setText ("Height:");
	c->addView (label);
	size.top = 0;
	size.bottom = kControlHeight;
	size.offset (80+kMargin, 0);
	size.setWidth (220);
	CColor lightGrey = MakeCColor (200, 200, 200, 255);
	CColor frameColor = MakeCColor (100, 100, 100, 100);
	CTextEdit* nameTextEdit = new CTextEdit (size, 0, -1);
	nameTextEdit->setBackColor (lightGrey);
	nameTextEdit->setFrameColor (frameColor);
	nameTextEdit->setFontColor (kBlackCColor);
	nameTextEdit->setText ("TemplateName");
	c->addView (nameTextEdit);
	size.offset (0, kMargin+kControlHeight);
	CTextEdit* widthTextEdit = new CTextEdit (size, 0, -1);
	widthTextEdit->setBackColor (lightGrey);
	widthTextEdit->setFrameColor (frameColor);
	widthTextEdit->setFontColor (kBlackCColor);
	widthTextEdit->setText ("300");
	c->addView (widthTextEdit);
	size.offset (0, kMargin+kControlHeight);
	CTextEdit* heightTextEdit = new CTextEdit (size, 0, -1);
	heightTextEdit->setBackColor (lightGrey);
	heightTextEdit->setFrameColor (frameColor);
	heightTextEdit->setFontColor (kBlackCColor);
	heightTextEdit->setText ("300");
	c->addView (heightTextEdit);
	CPoint p (-1, -1);
	if (Dialog::runViewModal (p, c, Dialog::kOkCancelButtons, "Create new template"))
	{
		long width = strtol (widthTextEdit->getText (), 0, 10);
		long height = strtol (heightTextEdit->getText (), 0, 10);
		if (width < 20)
			widthTextEdit->setText ("100");
		if (height < 20)
			heightTextEdit->setText ("50");
		std::string sizeAttr (widthTextEdit->getText ());
		sizeAttr += ", ";
		sizeAttr += heightTextEdit->getText ();
		UIAttributes* attr = new UIAttributes ();
		attr->setAttribute ("class", baseViewName);
		attr->setAttribute ("size", sizeAttr.c_str ());
		description->addNewTemplate (nameTextEdit->getText (), attr);
		exchangeView (nameTextEdit->getText ());
	}
	c->forget ();
}

//------------------------------------------------------------------------
void VST3Editor::runTemplateSettingsDialog ()
{
	CRect size (0, 0, 300+kMargin, kControlHeight*4+kMargin*3);
	CViewContainer* c = new CViewContainer (size, 0);
	c->setTransparency (true);
	size (0, 0, 80, kControlHeight);
	CTextLabel* label = new CTextLabel (size, "Min Width:");
	label->setStyle (kNoFrame);
	label->setBackColor (kWhiteCColor);
	label->setFontColor (kBlackCColor);
	label->setTextInset (CPoint (5, 0));
	label->setHoriAlign (kRightText);
	c->addView (label);
	size.offset (0, kMargin+kControlHeight);
	label = new CTextLabel (*label);
	label->setViewSize (size);
	label->setText ("Min Height:");
	c->addView (label);
	size.offset (0, kMargin+kControlHeight);
	label = new CTextLabel (*label);
	label->setViewSize (size);
	label->setText ("Max Width:");
	c->addView (label);
	size.offset (0, kMargin+kControlHeight);
	label = new CTextLabel (*label);
	label->setViewSize (size);
	label->setText ("Max Height:");
	c->addView (label);
	size.top = 0;
	size.bottom = kControlHeight;
	size.offset (80+kMargin, 0);
	size.setWidth (220);
	CColor lightGrey = MakeCColor (200, 200, 200, 255);
	CColor frameColor = MakeCColor (100, 100, 100, 100);
	CTextEdit* minWidthTextEdit = new CTextEdit (size, 0, -1);
	minWidthTextEdit->setBackColor (lightGrey);
	minWidthTextEdit->setFrameColor (frameColor);
	minWidthTextEdit->setFontColor (kBlackCColor);
	std::stringstream str;
	str << minSize.x;
	minWidthTextEdit->setText (str.str ().c_str ());
	c->addView (minWidthTextEdit);
	size.offset (0, kMargin+kControlHeight);
	CTextEdit* minHeightTextEdit = new CTextEdit (size, 0, -1);
	minHeightTextEdit->setBackColor (lightGrey);
	minHeightTextEdit->setFrameColor (frameColor);
	minHeightTextEdit->setFontColor (kBlackCColor);
	str.str ("");
	str << minSize.y;
	minHeightTextEdit->setText (str.str ().c_str ());
	c->addView (minHeightTextEdit);
	size.offset (0, kMargin+kControlHeight);
	CTextEdit* maxWidthTextEdit = new CTextEdit (size, 0, -1);
	maxWidthTextEdit->setBackColor (lightGrey);
	maxWidthTextEdit->setFrameColor (frameColor);
	maxWidthTextEdit->setFontColor (kBlackCColor);
	str.str ("");
	str << maxSize.x;
	maxWidthTextEdit->setText (str.str ().c_str ());
	c->addView (maxWidthTextEdit);
	size.offset (0, kMargin+kControlHeight);
	CTextEdit* maxHeightTextEdit = new CTextEdit (size, 0, -1);
	maxHeightTextEdit->setBackColor (lightGrey);
	maxHeightTextEdit->setFrameColor (frameColor);
	maxHeightTextEdit->setFontColor (kBlackCColor);
	str.str ("");
	str << maxSize.y;
	maxHeightTextEdit->setText (str.str ().c_str ());
	c->addView (maxHeightTextEdit);
	CPoint p (-1, -1);
	if (Dialog::runViewModal (p, c, Dialog::kOkCancelButtons, "Template Settings"))
	{
		UIAttributes* attr = const_cast<UIAttributes*> (description->getViewAttributes (viewName.c_str ()));
		if (attr)
		{
			std::string temp (minWidthTextEdit->getText ());
			temp += ", ";
			temp += minHeightTextEdit->getText ();
			attr->setAttribute ("minSize", temp.c_str ());
			temp = maxWidthTextEdit->getText ();
			temp += ", ";
			temp += maxHeightTextEdit->getText ();
			attr->setAttribute ("maxSize", temp.c_str ());
			recreateView ();
		}
	}
	c->forget ();
}

#endif // VSTGUI_LIVE_EDITING

END_NAMESPACE_VSTGUI
