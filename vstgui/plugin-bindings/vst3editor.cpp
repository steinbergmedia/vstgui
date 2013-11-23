//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
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
#include "../uidescription/editing/uieditcontroller.h"
#include "../uidescription/editing/uieditmenucontroller.h"
#include "../uidescription/uiviewfactory.h"
#include "base/source/updatehandler.h"
#include "base/source/fstring.h"
#include "base/source/timer.h"
#include "pluginterfaces/base/keycodes.h"
#include <list>
#include <sstream>
#include <assert.h>
#include <algorithm>

#if defined (kVstVersionMajor) && defined (kVstVersionMinor)
#define VST3_SUPPORTS_CONTEXTMENU (kVstVersionMajor > 3 || (kVstVersionMajor == 3 && kVstVersionMinor > 1))
#if VST3_SUPPORTS_CONTEXTMENU
	#include "pluginterfaces/vst/ivstcontextmenu.h"
#endif
#else
#define VST3_SUPPORTS_CONTEXTMENU  0
#endif

/// @cond ignore
namespace Steinberg {

//-----------------------------------------------------------------------------
class IdleUpdateHandler : public FObject, public ITimerCallback
{
public:
	OBJ_METHODS (IdleUpdateHandler, FObject)
	SINGLETON (IdleUpdateHandler)
protected:
	IdleUpdateHandler () 
	{
		UpdateHandler::instance ();
		timer = Timer::create (this, 1000/30); // 30 Hz timer
		CView::kDirtyCallAlwaysOnMainThread = true; // we will always call CView::setDirty() on the main thread
	}
	~IdleUpdateHandler () { timer->release (); }
	void onTimer (Timer* timer)
	{
		UpdateHandler::instance ()->triggerDeferedUpdates ();
	}

	Steinberg::Timer* timer;
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

	~ParameterChangeListener ()
	{
		if (parameter)
		{
			parameter->removeDependent (this);
			parameter->release ();
		}
		VSTGUI_RANGE_BASED_FOR_LOOP(ControlList, controls, CControl*, c)
			c->forget ();
		VSTGUI_RANGE_BASED_FOR_LOOP_END
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
				value = control->getValueNormalized ();
		}
		CParamDisplay* display = dynamic_cast<CParamDisplay*> (control);
		if (display)
			display->setValueToStringProc (valueToString, this);

		if (parameter)
			parameter->deferUpdate ();
		else
			updateControlValue (value);
	}
	
	void removeControl (CControl* control)
	{
		VSTGUI_RANGE_BASED_FOR_LOOP(ControlList, controls, CControl*, c)
			if (c == control)
			{
				controls.remove (control);
				control->forget ();
				return;
			}
		VSTGUI_RANGE_BASED_FOR_LOOP_END
	}
	
	bool containsControl (CControl* control)
	{
		return std::find (controls.begin (), controls.end (), control) != controls.end ();
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

	static bool valueToString (float value, char utf8String[256], void* userData)
	{
		ParameterChangeListener* This = (ParameterChangeListener*)userData;
		return This->convertValueToString (value, utf8String);
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
		VSTGUI_RANGE_BASED_FOR_LOOP(ControlList, controls, CControl*, c)
			c->setMouseEnabled (mouseEnabled);
			c->setDefaultValue ((float)defaultValue);
			CTextLabel* label = dynamic_cast<CTextLabel*>(c);
			if (label)
			{
				Steinberg::Vst::ParamValue normValue = value;
				if (isStepCount)
				{
					normValue = parameter->toNormalized (value);
				}
				Steinberg::Vst::String128 utf16Str;
				editController->getParamStringByValue (getParameterID (), normValue, utf16Str);
				Steinberg::String utf8Str (utf16Str);
				utf8Str.toMultiByte (Steinberg::kCP_Utf8);
				label->setText (utf8Str);
			}
			else
			{
				if (isStepCount)
				{
					c->setMin (minValue);
					c->setMax (maxValue);
					COptionMenu* optMenu = dynamic_cast<COptionMenu*>(c);
					if (optMenu)
					{
						optMenu->removeAllEntry ();
						for (Steinberg::int32 i = 0; i <= parameter->getInfo ().stepCount; i++)
						{
							Steinberg::Vst::String128 utf16Str;
							editController->getParamStringByValue (getParameterID (), (Steinberg::Vst::ParamValue)i / (Steinberg::Vst::ParamValue)parameter->getInfo ().stepCount, utf16Str);
							Steinberg::String utf8Str (utf16Str);
							utf8Str.toMultiByte (Steinberg::kCP_Utf8);
							optMenu->addEntry (utf8Str);
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
		VSTGUI_RANGE_BASED_FOR_LOOP_END
	}
	Steinberg::Vst::EditController* editController;
	Steinberg::Vst::Parameter* parameter;
	
	typedef std::list<CControl*> ControlList;
	ControlList controls;
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
static void releaseSubController (IController* subController)
{
	CBaseObject* baseObject = dynamic_cast<CBaseObject*> (subController);
	if (baseObject)
		baseObject->forget ();
	else
	{
		Steinberg::FObject* fobj = dynamic_cast<Steinberg::FObject*> (subController);
		if (fobj)
			fobj->release ();
		else
			delete subController;
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
Rebuild your plug-in, start your prefered host, instanciate your plug-in, open the context menu inside your editor and choose "Enable Editing".
Now you can define tags, colors, fonts, bitmaps and add views to your editor.

See @ref page_vst3_inline_editing @n
*/
//-----------------------------------------------------------------------------
VST3Editor::VST3Editor (Steinberg::Vst::EditController* controller, UTF8StringPtr _viewName, UTF8StringPtr _xmlFile)
: VSTGUIEditor (controller)
, doCreateView (false)
, tooltipsEnabled (true)
, delegate (dynamic_cast<VST3EditorDelegate*> (controller))
, originalController (0)
, editingEnabled (false)
{
	description = new UIDescription (_xmlFile);
	viewName = _viewName;
	xmlFile = _xmlFile;
	init ();
}

//-----------------------------------------------------------------------------
VST3Editor::VST3Editor (UIDescription* desc, Steinberg::Vst::EditController* controller, UTF8StringPtr _viewName, UTF8StringPtr _xmlFile)
: VSTGUIEditor (controller)
, doCreateView (false)
, tooltipsEnabled (true)
, delegate (dynamic_cast<VST3EditorDelegate*> (controller))
, originalController (0)
, editingEnabled (false)
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
	setIdleRate (300);
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
	if (frame)
		frame->enableTooltips (state);
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
			frame->getSize (currentSize);
			newSize = currentSize;
			CCoord width = currentSize.getWidth ();
			CCoord height = currentSize.getHeight ();
			if (width > maxSize.x)
				currentSize.setWidth (maxSize.x);
			else if (width < minSize.x)
				currentSize.setWidth (minSize.x);
			if (height > maxSize.y)
				currentSize.setHeight (maxSize.y);
			else if (height < minSize.y)
				currentSize.setHeight (minSize.y);
			if (newSize != currentSize)
				requestResize (CPoint (newSize.getWidth (), newSize.getHeight ()));
		}
		
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool VST3Editor::requestResize (const CPoint& newSize)
{
	CCoord width = newSize.x;
	CCoord height = newSize.y;
	if (width >= minSize.x && width <= maxSize.x && height >= minSize.y && height <= maxSize.y)
	{
		Steinberg::ViewRect vr;
		vr.right = width;
		vr.bottom = height;
		return plugFrame->resizeView (this, &vr) == Steinberg::kResultTrue ? true : false;
	}
	return false;
}

//-----------------------------------------------------------------------------
void VST3Editor::getEditorSizeConstrains (CPoint& minimumSize, CPoint& maximumSize)
{
	minimumSize = minSize;
	maximumSize = maxSize;
}

//-----------------------------------------------------------------------------
ParameterChangeListener* VST3Editor::getParameterChangeListener (int32_t tag)
{
	if (tag != -1)
	{
		ParameterChangeListenerMap::iterator it = paramChangeListeners.find (tag);
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
		Steinberg::Vst::ParamValue value = pControl->getValueNormalized ();
		CTextEdit* textEdit = dynamic_cast<CTextEdit*> (pControl);
		if (textEdit && pcl->getParameter ())
		{
			Steinberg::String str (textEdit->getText ());
			str.toWideString (Steinberg::kCP_Utf8);
			if (getController ()->getParamValueByString (pcl->getParameterID (), (Steinberg::Vst::TChar*)str.text16 (), value) != Steinberg::kResultTrue)
			{
				pcl->update (0, kChanged);
				return;
			}
		}
		pcl->performEdit (value);
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
				Steinberg::Vst::Parameter* parameter = editController->getParameterObject (pControl->getTag ());
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
	CControl* control = dynamic_cast<CControl*> (view);
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
		releaseSubController (controller);
		view->removeAttribute (kCViewControllerAttribute);
	}
}

//-----------------------------------------------------------------------------
int32_t VST3Editor::onKeyDown (const VstKeyCode& code, CFrame* frame)
{
#if VSTGUI_LIVE_EDITING
	if (code.modifier == MODIFIER_CONTROL && frame->getModalView () == 0)
	{
		if (code.character == 'e')
		{
			enableEditing (!editingEnabled);
			return 1;
		}
	}
#endif
	return -1;
}

//-----------------------------------------------------------------------------
int32_t VST3Editor::onKeyUp (const VstKeyCode& code, CFrame* frame)
{
	return -1;
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
	~ContextMenuTarget ()
	{
		item->forget ();
	}

	Steinberg::tresult PLUGIN_API executeMenuItem (Steinberg::int32 tag)
	{
		if (item->getTarget ())
			item->getTarget ()->notify (item, CCommandMenuItem::kMsgMenuItemSelected);
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
	for (CConstMenuItemIterator it = menu->getItems ()->begin (); it != menu->getItems ()->end ();it++)
	{
		CCommandMenuItem* commandItem = (*it).cast<CCommandMenuItem>();
		if (commandItem && commandItem->getTarget ())
			commandItem->getTarget ()->notify (commandItem, CCommandMenuItem::kMsgMenuItemValidate);

		Steinberg::Vst::IContextMenu::Item item = {};
		Steinberg::String title ((*it)->getTitle ());
		title.toWideString (Steinberg::kCP_Utf8);
		title.copyTo16 (item.name, 0, 128);
		if ((*it)->getSubmenu ())
		{
			item.flags = Steinberg::Vst::IContextMenu::Item::kIsGroupStart;
			contextMenu->addItem (item, 0);
			addCOptionMenuEntriesToIContextMenu (editor, (*it)->getSubmenu (), contextMenu);
			item.flags = Steinberg::Vst::IContextMenu::Item::kIsGroupEnd;
			contextMenu->addItem (item, 0);
		}
		else if ((*it)->isSeparator ())
		{
			item.flags = Steinberg::Vst::IContextMenu::Item::kIsSeparator;
			contextMenu->addItem (item, 0);
		}
		else
		{
			if (commandItem)
			{
				if ((*it)->isChecked ())
					item.flags |= Steinberg::Vst::IContextMenu::Item::kIsChecked;
				if ((*it)->isEnabled () == false)
					item.flags |= Steinberg::Vst::IContextMenu::Item::kIsDisabled;
				ContextMenuTarget* target = new ContextMenuTarget (commandItem);
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
		COptionMenu* controllerMenu = (delegate && editingEnabled == false) ? delegate->createContextMenu (where, this) : 0;
	#if VSTGUI_LIVE_EDITING
		if (editingEnabled == false)
		{
			if (controllerMenu == 0)
				controllerMenu = new COptionMenu ();
			else
				controllerMenu->addSeparator ();
			CMenuItem* item = controllerMenu->addEntry (new CCommandMenuItem ("Open UIDescription Editor", this, "File", "Open UIDescription Editor"));
			item->setKey ("e", kControl);
		}
	#endif
	#if VST3_SUPPORTS_CONTEXTMENU
		Steinberg::FUnknownPtr<Steinberg::Vst::IComponentHandler3> handler (getController ()->getComponentHandler ());
		Steinberg::Vst::ParamID paramID;
		if (handler)
		{
			bool paramFound = findParameter ((Steinberg::int32)where.x, (Steinberg::int32)where.y, paramID) == Steinberg::kResultTrue;
			Steinberg::Vst::IContextMenu* contextMenu = handler->createContextMenu (this, paramFound ? &paramID : 0);
			if (contextMenu)
			{
				if (controllerMenu)
					VST3EditorInternal::addCOptionMenuEntriesToIContextMenu (this, controllerMenu, contextMenu);
				if (contextMenu->popup (where.x, where.y) == Steinberg::kResultTrue)
					result = kMouseEventHandled;
				contextMenu->release ();
			}
		}
		if (result == kMouseEventNotHandled)
	#endif
		if (controllerMenu)
		{
			controllerMenu->setStyle (kPopupStyle|kMultipleCheckStyle);
			controllerMenu->popup (frame, where);
			result = kMouseEventHandled;
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
	if (frame && frame->getViewsAt (CPoint (xPos, yPos), views))
	{
		CControl* control = 0;
		std::list<SharedPointer<CView> >::const_iterator it = views.begin ();
		while (it != views.end ())
		{
			control = (*it).cast<CControl>();
			if (control)
			{
				if (control->getMouseEnabled () && control->getTag () != -1)
					break;
				control = 0;
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
IController* VST3Editor::createSubController (UTF8StringPtr name, IUIDescription* description)
{
	return delegate ? delegate->createSubController (name, description, this) : 0;
}

//-----------------------------------------------------------------------------
CView* VST3Editor::createView (const UIAttributes& attributes, IUIDescription* description)
{
	if (delegate)
	{
		const std::string* customViewName = attributes.getAttributeValue ("custom-view-name");
		if (customViewName)
		{
			CView* view = delegate->createCustomView (customViewName->c_str (), attributes, description, this);
			return view;
		}
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
	enableEditing (editingEnabled);
}

#define kFrameEnableFocusDrawingAttr	"frame-enable-focus-drawing"
#define kFrameFocusColorAttr			"frame-focus-color"
#define kFrameFocusWidthAttr			"frame-focus-width"

//-----------------------------------------------------------------------------
bool PLUGIN_API VST3Editor::open (void* parent, const PlatformType& type)
{
	frame = new CFrame (CRect (0, 0, 0, 0), this);
	frame->setViewAddedRemovedObserver (this);
	frame->setTransparency (true);
	frame->registerMouseObserver (this);
#if VSTGUI_LIVE_EDITING
	frame->registerKeyboardHook (this);
#endif
	frame->enableTooltips (tooltipsEnabled);

	if (!enableEditing (false))
	{
		frame->forget ();
		return false;
	}

	frame->open (parent, type);

	if (delegate)
		delegate->didOpen (this);
	return true;
}

//-----------------------------------------------------------------------------
void PLUGIN_API VST3Editor::close ()
{
	if (delegate)
		delegate->willClose (this);

	ParameterChangeListenerMap::iterator it = paramChangeListeners.begin ();
	while (it != paramChangeListeners.end ())
	{
		it->second->release ();
		it++;
	}
	paramChangeListeners.clear ();
	if (frame)
	{
	#if VSTGUI_LIVE_EDITING
		frame->unregisterKeyboardHook (this);
	#endif
		frame->unregisterMouseObserver (this);
		frame->removeAll (true);
		int32_t refCount = frame->getNbReference ();
		if (refCount == 1)
		{
			frame->close ();
			frame = 0;
		}
		else
		{
			frame->forget ();
		}
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
CMessageResult VST3Editor::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == CVSTGUITimer::kMsgTimer)
	{
		if (doCreateView)
			recreateView ();
 	}
	#if VSTGUI_LIVE_EDITING
	else if (message == CCommandMenuItem::kMsgMenuItemValidate)
	{
		CCommandMenuItem* item = dynamic_cast<CCommandMenuItem*>(sender);
		if (item)
		{
			if (strcmp (item->getCommandCategory(), "File") == 0)
			{
				if (strcmp (item->getCommandName(), "Save") == 0)
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
					return kMessageNotified;
				}
			}
		}
	}
	else if (message == CCommandMenuItem::kMsgMenuItemSelected)
	{
		CCommandMenuItem* item = dynamic_cast<CCommandMenuItem*>(sender);
		if (item)
		{
			if (strcmp (item->getCommandCategory (), "Edit") == 0)
			{
				if (strcmp (item->getCommandName (), "Sync Parameter Tags") == 0)
				{
					syncParameterTags ();
					return kMessageNotified;
				}
			}
			else if (strcmp (item->getCommandCategory (), "File") == 0)
			{
				if (strcmp (item->getCommandName (), "Open UIDescription Editor") == 0)
				{
					editingEnabled = true;
					doCreateView = true;
					return kMessageNotified;
				}
				else if (strcmp (item->getCommandName (), "Close UIDescription Editor") == 0)
				{
					editingEnabled = false;
					doCreateView = true;
					return kMessageNotified;
				}
				else if (strcmp (item->getCommandName (), "Save") == 0)
				{
					UIAttributes* attributes = description->getCustomAttributes ("VST3Editor", true);
					if (attributes)
					{
						const std::string* filePath = attributes->getAttributeValue ("Path");
						if (filePath)
						{
							int32_t flags = 0;
							// check save options from UIEditController
							UIEditController* editController = dynamic_cast<UIEditController*> (getViewController (frame->getView (0)));
							if (editController)
							{
								attributes = editController->getSettings ();
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
							description->save (filePath->c_str (), flags);
						}
					}
					item->setChecked (false);
					return kMessageNotified;
				}
				else if (strcmp (item->getCommandName (), "Save As") == 0)
				{
					UIAttributes* attributes = description->getCustomAttributes ("VST3Editor", true);
					if (attributes)
					{
						CNewFileSelector* fileSelector = CNewFileSelector::create (frame, CNewFileSelector::kSelectSaveFile);
						if (fileSelector)
						{
							fileSelector->setTitle ("Save UIDescription File");
							fileSelector->setDefaultExtension (CFileExtension ("VSTGUI UI Description", "uidesc"));
							const std::string* filePath = attributes->getAttributeValue ("Path");
							if (filePath)
								fileSelector->setInitialDirectory (filePath->c_str ());
							if (fileSelector->runModal ())
							{
								UTF8StringPtr filePath = fileSelector->getSelectedFile (0);
								if (filePath)
								{
									attributes->setAttribute ("Path", filePath);
									description->save (filePath);
								}
							}
							fileSelector->forget ();
						}
					}
					item->setChecked (false);
					return kMessageNotified;
				}
			}
		}
	}
	#endif
 	return VSTGUIEditor::notify (sender, message); 
}

//------------------------------------------------------------------------
void VST3Editor::syncParameterTags ()
{
#if VSTGUI_LIVE_EDITING
	CView* view = getFrame ()->getView (0);
	if (view)
	{
		IController* controller = getViewController (view);
		IActionPerformer* actionPerformer = controller ? dynamic_cast<IActionPerformer*>(controller) : 0;
		if (actionPerformer)
		{
			UIDescription::DeferChanges dc (description);
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
					else if (units.size () > 0)
					{
						paramTitle.insertAt (0, "::");
						paramTitle.insertAt (0, "Root");
					}
					paramTitle.toMultiByte (Steinberg::kCP_Utf8);
					paramTitle.removeChars (' ');
					Steinberg::String paramIDStr;
					paramIDStr.printInt64 (info.id);
					if (UTF8StringPtr tagName = description->lookupControlTagName (info.id))
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
			nonEditRect = frame->getViewSize ();
			description->setController (this);
			UIEditController* editController = new UIEditController (description);
			CView* view = editController->createEditView ();
			if (view)
			{
				frame->setSize (view->getWidth (), view->getHeight ());
				frame->addView (view);

				rect.right = rect.left + (Steinberg::int32)view->getWidth ();
				rect.bottom = rect.top + (Steinberg::int32)view->getHeight ();
				plugFrame->resizeView (this, &rect);

				frame->enableTooltips (true);
				CColor focusColor = kBlueCColor;
				editController->getEditorDescription ().getColor ("focus", focusColor);
				frame->setFocusColor (focusColor);
				frame->setFocusDrawingEnabled (true);
				frame->setFocusWidth (1);
				
				COptionMenu* fileMenu = editController->getMenuController ()->getFileMenu ();
				if (fileMenu)
				{
					CMenuItem* item = fileMenu->addEntry (new CCommandMenuItem ("Save", this, "File", "Save"), 0);
					item->setKey ("s", kControl);
					item = fileMenu->addEntry (new CCommandMenuItem ("Save As..", this, "File", "Save As"), 1);
					item->setKey ("s", kShift|kControl);
					item = fileMenu->addEntry (new CCommandMenuItem ("Close Editor", this, "File", "Close UIDescription Editor"));
					item->setKey ("e", kControl);
				}
				COptionMenu* editMenu = editController->getMenuController ()->getEditMenu ();
				if (editMenu)
				{
					editMenu->addSeparator ();
					editMenu->addEntry (new CCommandMenuItem ("Sync Parameter Tags", this, "Edit", "Sync Parameter Tags"));
				}
				editingEnabled = true;
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
				frame->setSize (view->getWidth (), view->getHeight ());
				frame->addView (view);
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
				}

				frame->setFocusDrawingEnabled (false);

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
								attributes->setAttribute ("color", attr->c_str ());
							}
							attr = oldAttributes->getAttributeValue (kFrameFocusWidthAttr);
							if (attr)
							{
								attributes->setAttribute ("width", attr->c_str ());
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
					frame->setFocusDrawingEnabled (true);
					attr = attributes->getAttributeValue ("color");
					if (attr)
					{
						CColor focusColor;
						if (description->getColor (attr->c_str (), focusColor))
							frame->setFocusColor (focusColor);
					}
					attr = attributes->getAttributeValue ("width");
					if (attr)
					{
						double focusWidth = strtod (attr->c_str (), 0);
						frame->setFocusWidth (focusWidth);
					}
				}
				return true;
			}
		}
	}
	return false;
}

} // namespace
