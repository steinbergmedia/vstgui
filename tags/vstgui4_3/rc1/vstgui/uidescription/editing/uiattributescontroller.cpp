//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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

#include "uiattributescontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uiactions.h"
#include "uieditcontroller.h"
#include "uisearchtextfield.h"
#include "../uiviewfactory.h"
#include "../uiattributes.h"
#include "../../lib/controls/coptionmenu.h"
#include "../../lib/controls/cslider.h"
#include "../../lib/coffscreencontext.h"
#include "../../lib/crowcolumnview.h"
#include "../../lib/iviewlistener.h"
#include "../../lib/cstring.h"
#include "../../lib/cgraphicspath.h"
#include "../../lib/cvstguitimer.h"
#include <sstream>
#include <algorithm>
#include <cassert>

namespace VSTGUI {

namespace UIAttributeControllers {

//----------------------------------------------------------------------------------------------------
class Controller : public CBaseObject, public DelegationController
{
public:
	Controller (IController* baseController, const std::string& attrName)
	: DelegationController (baseController), attrName (attrName), differentValues (false) {}
	
	const std::string& getAttributeName () const { return attrName; }
	virtual void setValue (const std::string& value) = 0;
	
	virtual void hasDifferentValues (bool state) { differentValues = state; }
	bool hasDifferentValues () const { return differentValues; }
protected:
	IControlListener* getControlListener (UTF8StringPtr controlTagName) VSTGUI_OVERRIDE_VMETHOD { return this; }
	UIAttributesController* getAttributesController () const { return dynamic_cast<UIAttributesController*> (controller); }
	void performValueChange (UTF8StringPtr value)
	{
		hasDifferentValues (false);
		std::string valueStr = value ? value : "";
		UIAttributesController* attrController = getAttributesController ();
		if (attrController)
			attrController->performAttributeChange (attrName, valueStr);
	}
	std::string attrName;
	bool differentValues;
};

//----------------------------------------------------------------------------------------------------
class TextAlignmentController : public Controller
{
public:
	TextAlignmentController (IController* baseController, const std::string& attrName)
	: Controller (baseController, attrName) {}

	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD
	{
		CControl* control = dynamic_cast<CControl*>(view);
		if (control)
		{
			int32_t tag = control->getTag ();
			if (tag >= kLeftTag && tag <= kRightTag)
				controls[tag] = control;
		}
		return controller->verifyView (view, attributes, description);
	}

	void setValue (const std::string& value) VSTGUI_OVERRIDE_VMETHOD
	{
		if (hasDifferentValues ())
		{
			for (int32_t i = kLeftTag; i <= kRightTag; i++)
			{
				controls[i]->setValue (0.f);
				controls[i]->invalid ();
			}
		}
		else
		{
			int32_t alignment = 0;
			if (value == "center")
				alignment = 1;
			else if (value == "right")
				alignment = 2;
			for (int32_t i = kLeftTag; i <= kRightTag; i++)
			{
				controls[i]->setValue (i == alignment ? 1.f : 0.f);
				controls[i]->invalid ();
			}
		}
	}

	void valueChanged (CControl* control) VSTGUI_OVERRIDE_VMETHOD
	{
		if (control->getValue () == control->getMax ())
		{
			switch (control->getTag ())
			{
				case kLeftTag:
				{
					performValueChange ("left");
					break;
				}
				case kCenterTag:
				{
					performValueChange ("center");
					break;
				}
				case kRightTag:
				{
					performValueChange ("right");
					break;
				}
			}
		}
		else
		{
			control->setValue (control->getMax ());
			control->invalid ();
		}
	}

protected:
	enum {
		kLeftTag = 0,
		kCenterTag,
		kRightTag
	};

	CControl* controls[3];
};

//----------------------------------------------------------------------------------------------------
class AutosizeController : public Controller
{
public:
	AutosizeController (IController* baseController, UISelection* selection, const std::string& attrName)
	: Controller (baseController, attrName), selection (selection) {}

	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD
	{
		CControl* control = dynamic_cast<CControl*>(view);
		if (control)
		{
			int32_t tag = control->getTag ();
			if (tag >= kLeftTag && tag <= kColTag)
				controls[tag] = control;
			if (tag >= kRowTag && tag <= kColTag)
			{
				FOREACH_IN_SELECTION(selection, view)
					if (dynamic_cast<CViewContainer*>(view) == 0)
					{
						controls[tag]->setVisible (false);
						break;
					}
				FOREACH_IN_SELECTION_END
			}
		}
		return controller->verifyView (view, attributes, description);
	}

	void setValue (const std::string& value) VSTGUI_OVERRIDE_VMETHOD
	{
		if (hasDifferentValues ())
		{
			for (int32_t i = kLeftTag; i <= kColTag; i++)
			{
				controls[i]->setValue (0.f);
			}
		}
		else
		{
			controls[kLeftTag]->setValue (value.find ("left") == std::string::npos ? controls[kLeftTag]->getMin () : controls[kLeftTag]->getMax ());
			controls[kRightTag]->setValue (value.find ("right") == std::string::npos ? controls[kRightTag]->getMin () : controls[kRightTag]->getMax ());
			controls[kTopTag]->setValue (value.find ("top") == std::string::npos ? controls[kTopTag]->getMin () : controls[kTopTag]->getMax ());
			controls[kBottomTag]->setValue (value.find ("bottom") == std::string::npos ? controls[kBottomTag]->getMin () : controls[kBottomTag]->getMax ());
			controls[kRowTag]->setValue (value.find ("row") == std::string::npos ? controls[kRowTag]->getMin () : controls[kRowTag]->getMax ());
			controls[kColTag]->setValue (value.find ("column") == std::string::npos ? controls[kColTag]->getMin () : controls[kColTag]->getMax ());
		}
		for (int32_t i = kLeftTag; i <= kColTag; i++)
		{
			controls[i]->invalid ();
		}
	}

	void valueChanged (CControl* control) VSTGUI_OVERRIDE_VMETHOD
	{
		if (control == controls[kRowTag])
		{
			if (control->getValue () == control->getMax ())
			{
				controls[kColTag]->setValue (control->getMin ());
			}
		}
		else if (control == controls[kColTag])
		{
			if (control->getValue () == control->getMax ())
			{
				controls[kRowTag]->setValue (control->getMin ());
			}
		}
		std::string str;
		if (controls[kLeftTag]->getValue () == controls[kLeftTag]->getMax ())
			str = "left";
		if (controls[kRightTag]->getValue () == controls[kRightTag]->getMax ())
		{
			if (str.empty () == false)
				str += " ";
			str += "right";
		}
		if (controls[kTopTag]->getValue () == controls[kTopTag]->getMax ())
		{
			if (str.empty () == false)
				str += " ";
			str += "top";
		}
		if (controls[kBottomTag]->getValue () == controls[kBottomTag]->getMax ())
		{
			if (str.empty () == false)
				str += " ";
			str += "bottom";
		}
		if (controls[kRowTag]->getValue () == controls[kRowTag]->getMax ())
		{
			if (str.empty () == false)
				str += " ";
			str += "row";
		}
		if (controls[kColTag]->getValue () == controls[kColTag]->getMax ())
		{
			if (str.empty () == false)
				str += " ";
			str += "column";
		}
		performValueChange (str.c_str ());
	}


protected:
	enum {
		kLeftTag = 0,
		kTopTag,
		kRightTag,
		kBottomTag,
		kRowTag,
		kColTag
	};

	CControl* controls[6];
	SharedPointer<UISelection> selection;
};

//----------------------------------------------------------------------------------------------------
class BooleanController : public Controller
{
public:
	BooleanController (IController* baseController, const std::string& attrName)
	: Controller (baseController, attrName), control (0) {}

	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD
	{
		if (control == 0)
		{
			control = dynamic_cast<CControl*>(view);
		}
		return controller->verifyView (view, attributes, description);
	}
	void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD
	{
		if (pControl->getValue () == control->getMax ())
			performValueChange ("true");
		else
			performValueChange ("false");
	}
	void setValue (const std::string& value) VSTGUI_OVERRIDE_VMETHOD
	{
		if (hasDifferentValues())
		{
			control->setValue (control->getMin () + (control->getMax () - control->getMin ()) / 2.f);
		}
		else
		{
			if (value == "true")
				control->setValue (control->getMax ());
			else
				control->setValue (control->getMin ());
		}
		control->invalid ();
	}

protected:
	CControl* control;
};

//----------------------------------------------------------------------------------------------------
class TextController : public Controller, public IViewListenerAdapter
{
public:
	TextController (IController* baseController, const std::string& attrName)
	: Controller (baseController, attrName) {}

	~TextController ()
	{
		if (textLabel)
		{
			textLabel->unregisterViewListener (this);
			textLabel->removeDependency (this);
		}
		if (slider)
			slider->removeDependency (this);
	}
	
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD
	{
		if (textLabel == 0)
		{
			CTextLabel* edit = dynamic_cast<CTextLabel*>(view);
			if (edit)
			{
				textLabel = edit;
				originalTextColor = textLabel->getFontColor ();
				textLabel->addDependency (this);
				textLabel->registerViewListener (this);
			}
		}
		if (slider == 0)
		{
			CSlider* sliderView = dynamic_cast<CSlider*>(view);
			if (sliderView)
			{
				slider = sliderView;
				slider->addDependency (this);
			}
		}
		return controller->verifyView (view, attributes, description);
	}
	
	void controlBeginEdit (VSTGUI::CControl* pControl) VSTGUI_OVERRIDE_VMETHOD
	{
		if (pControl == slider)
		{
			std::stringstream sstream;
			sstream << slider->getValue ();
			getAttributesController ()->beginLiveAttributeChange (attrName, sstream.str ());
		}
		Controller::controlBeginEdit (pControl);
	}
	
	void controlEndEdit (VSTGUI::CControl* pControl) VSTGUI_OVERRIDE_VMETHOD
	{
		if (pControl == slider)
		{
			getAttributesController ()->endLiveAttributeChange ();
		}
		Controller::controlEndEdit (pControl);
	}
	
	void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD
	{
		if (textLabel == pControl)
		{
			textLabel->setFontColor (originalTextColor);
			performValueChange (textLabel->getText ());
		}
		else if (slider == pControl)
		{
			std::stringstream sstream;
			sstream << slider->getValue ();
			performValueChange (sstream.str ().c_str ());
		}
	}
	
	void setValue (const std::string& value) VSTGUI_OVERRIDE_VMETHOD
	{
		if (textLabel)
		{
			if (hasDifferentValues ())
			{
				CColor newColor (originalTextColor);
				newColor.alpha /= 2;
				textLabel->setFontColor (newColor);
				textLabel->setText ("Multiple Values");
			}
			else
			{
				textLabel->setText (value.c_str ());
			}
		}
		if (slider)
		{
			float floatValue;

			std::istringstream sstream (value);
			sstream.imbue (std::locale::classic ());
			sstream.precision (40);
			sstream >> floatValue;

			slider->setValue (floatValue);
			slider->invalid ();
		}
	}

	virtual void valueDisplayTruncated (UTF8StringPtr txt)
	{
		if (textLabel)
		{
			if (txt && *txt != 0)
				textLabel->setAttribute (kCViewTooltipAttribute, static_cast<uint32_t> (strlen (textLabel->getText ()) + 1), textLabel->getText ());
			else
				textLabel->removeAttribute (kCViewTooltipAttribute);
		}
		
	}

	void viewLostFocus (CView* view) VSTGUI_OVERRIDE_VMETHOD
	{
	#if VSTGUI_HAS_FUNCTIONAL
		if (view == textLabel)
		{
			SharedPointer<CTextEdit> textEdit = textLabel.cast<CTextEdit> ();
			if (textEdit && textEdit->bWasReturnPressed)
			{
				Call::later([textEdit] () {
					textEdit->takeFocus();
				});
			}
		}
	#endif
	}
	
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD
	{
		if (message == CTextLabel::kMsgTruncatedTextChanged)
		{
			UTF8StringPtr txt = textLabel->getTruncatedText ();
			valueDisplayTruncated (txt);
			return kMessageNotified;
		}
		return kMessageUnknown;
	}
protected:
	SharedPointer<CTextLabel> textLabel;
	SharedPointer<CSlider> slider;
	CColor originalTextColor;
};

//----------------------------------------------------------------------------------------------------
class MenuController : public TextController
{
public:
	MenuController (IController* baseController, const std::string& attrName, UIDescription* description, bool addNoneItem = true, bool sortItems = true)
	: TextController (baseController, attrName), description (description), addNoneItem (addNoneItem), sortItems (sortItems) {}

	~MenuController ()
	{
		if (menu)
			menu->removeDependency (this);
	}
	
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD
	{
		if (menu == 0)
		{
			menu = dynamic_cast<COptionMenu*>(view);
			if (menu)
				menu->addDependency (this);
		}
		return TextController::verifyView (view, attributes, description);
	}

	typedef std::list<const std::string*> StringPtrList;
	virtual void collectMenuItemNames (StringPtrList& names) = 0;
	virtual void validateMenuEntry (CCommandMenuItem* item) {}

	virtual void addMenuEntry (const std::string* entryName)
	{
		CCommandMenuItem* item = new CCommandMenuItem (entryName->c_str (), this);
		validateMenuEntry (item);
		menu->addEntry (item);
		if (textLabel->getText () == *entryName)
		{
			int32_t index = menu->getNbEntries () - 1;
			menu->setValue ((float)index);
			menu->setCurrent (index);
		}
	}

	void setValue (const std::string& value) VSTGUI_OVERRIDE_VMETHOD
	{
		TextController::setValue (value);
	}

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD
	{
		if (sender == menu && message == COptionMenu::kMsgBeforePopup)
		{
			menu->removeAllEntry ();
			if (addNoneItem)
				menu->addEntry (new CCommandMenuItem ("None", 100, this));
			StringPtrList names;
			collectMenuItemNames (names);
			if (sortItems)
				names.sort (UIEditController::std__stringCompare);
			if (addNoneItem && !names.empty ())
				menu->addSeparator ();
			VSTGUI_RANGE_BASED_FOR_LOOP(StringPtrList, names, const std::string*, name)
				addMenuEntry (name);
			VSTGUI_RANGE_BASED_FOR_LOOP_END
			return kMessageNotified;
		}
		else if (message == CCommandMenuItem::kMsgMenuItemSelected)
		{
			CCommandMenuItem* item = dynamic_cast<CCommandMenuItem*>(sender);
			if (item)
			{
				performValueChange (item->getTag () == 100 ? "" : item->getTitle ());
			}
			return kMessageNotified;
		}
		return TextController::notify (sender, message);
	}

	void valueDisplayTruncated (UTF8StringPtr txt) VSTGUI_OVERRIDE_VMETHOD
	{
		if (textLabel && menu)
		{
			if (txt && *txt != 0)
				menu->setAttribute (kCViewTooltipAttribute, static_cast<uint32_t> (strlen (textLabel->getText ()) + 1), textLabel->getText ());
			else
				menu->removeAttribute (kCViewTooltipAttribute);
		}
		
	}

protected:
	SharedPointer<UIDescription> description;
	SharedPointer<COptionMenu> menu;
	
	bool addNoneItem;
	bool sortItems;
};

//----------------------------------------------------------------------------------------------------
class ColorController : public MenuController
{
public:
	ColorController (IController* baseController, const std::string& attrName, UIDescription* description)
	: MenuController (baseController, attrName, description) {}

	void collectMenuItemNames (StringPtrList& names) VSTGUI_OVERRIDE_VMETHOD
	{
		description->collectColorNames (names);
	}
	
	void validateMenuEntry (CCommandMenuItem* item) VSTGUI_OVERRIDE_VMETHOD
	{
		const CCoord size = 15;
		CColor color;
		if (description->getColor (item->getTitle (), color))
		{
			COffscreenContext* context = COffscreenContext::create (menu->getFrame (), size, size);
			if (context)
			{
				context->beginDraw ();
				context->setFillColor (color);
				context->drawRect (CRect (0, 0, size, size), kDrawFilled);
				context->endDraw ();
				item->setIcon (context->getBitmap ());
				context->forget ();
			}
		}
	}
	
	void setValue (const std::string& value) VSTGUI_OVERRIDE_VMETHOD
	{
		MenuController::setValue (value);
		if (colorView)
		{
			if (hasDifferentValues ())
			{
				colorView->color = kTransparentCColor;
			}
			else
			{
				CColor color;
				if (description->getColor (value.c_str (), color))
				{
					colorView->color = color;
				}
				else
				{
					colorView->color = kTransparentCColor;
				}
			}
			colorView->invalid ();
		}
	}

	CView* createView (const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD
	{
		const std::string* attr = attributes.getAttributeValue (IUIDescription::kCustomViewName);
		if (attr && *attr == "ColorView")
		{
			colorView = new ColorView ();
			return colorView;
		}
		return 0;
	}
protected:
	class ColorView : public CView
	{
	public:
		ColorView () : CView (CRect (0, 0, 0, 0)) ,color (kTransparentCColor) {}
		void draw (CDrawContext* context) VSTGUI_OVERRIDE_VMETHOD
		{
			context->setFillColor (color);
			context->setDrawMode (kAliasing);
			context->drawRect (getViewSize (), kDrawFilled);
		}
		CColor color;
	};
	SharedPointer<ColorView> colorView;
};

//----------------------------------------------------------------------------------------------------
class GradientController : public MenuController
{
public:
	GradientController (IController* baseController, const std::string& attrName, UIDescription* description)
	: MenuController (baseController, attrName, description) {}

	void collectMenuItemNames (StringPtrList& names) VSTGUI_OVERRIDE_VMETHOD
	{
		description->collectGradientNames (names);
	}
	
	void validateMenuEntry (CCommandMenuItem* item) VSTGUI_OVERRIDE_VMETHOD
	{
		const CCoord size = 15;
		if (CGradient* gradient = description->getGradient (item->getTitle ()))
		{
			COffscreenContext* context = COffscreenContext::create (menu->getFrame (), size, size);
			if (context)
			{
				context->beginDraw ();
				SharedPointer<CGraphicsPath> path = owned (context->createGraphicsPath ());
				path->addRect (CRect (0, 0, size, size));
				context->fillLinearGradient(path, *gradient, CPoint (0, 0), CPoint (size, 0));
				context->endDraw ();
				item->setIcon (context->getBitmap ());
				context->forget ();
			}
		}
	}
	
	void setValue (const std::string& value) VSTGUI_OVERRIDE_VMETHOD
	{
		MenuController::setValue (value);
		if (gradientView)
		{
			if (hasDifferentValues ())
			{
				gradientView->gradient = 0;
			}
			else
			{
				gradientView->gradient = description->getGradient (value.c_str ());
			}
			gradientView->invalid ();
		}
	}

	CView* createView (const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD
	{
		const std::string* attr = attributes.getAttributeValue (IUIDescription::kCustomViewName);
		if (attr && *attr == "GradientView")
		{
			gradientView = new GradientView ();
			return gradientView;
		}
		return 0;
	}
protected:
	class GradientView : public CView
	{
	public:
		GradientView () : CView (CRect (0, 0, 0, 0)) {}
		void draw (CDrawContext* context) VSTGUI_OVERRIDE_VMETHOD
		{
			if (gradient == 0)
				return;
			CRect r = getViewSize ();
			SharedPointer<CGraphicsPath> path = owned (context->createGraphicsPath ());
			path->addRect (r);
			context->fillLinearGradient (path, *gradient, r.getTopLeft (), r.getTopRight ());
		}
		SharedPointer<CGradient> gradient;
	};
	SharedPointer<GradientView> gradientView;
};

//----------------------------------------------------------------------------------------------------
class TagController : public MenuController
{
public:
	TagController (IController* baseController, const std::string& attrName, UIDescription* description)
	: MenuController (baseController, attrName, description, true, false) {}

	void collectMenuItemNames (StringPtrList& names) VSTGUI_OVERRIDE_VMETHOD
	{
		description->collectControlTagNames (names);
	}
	
};

//----------------------------------------------------------------------------------------------------
class BitmapController : public MenuController
{
public:
	BitmapController (IController* baseController, const std::string& attrName, UIDescription* description)
	: MenuController (baseController, attrName, description) {}

	void collectMenuItemNames (StringPtrList& names) VSTGUI_OVERRIDE_VMETHOD
	{
		description->collectBitmapNames (names);
	}
	
};

//----------------------------------------------------------------------------------------------------
class FontController : public MenuController
{
public:
	FontController (IController* baseController, const std::string& attrName, UIDescription* description)
	: MenuController (baseController, attrName, description) {}

	void collectMenuItemNames (StringPtrList& names) VSTGUI_OVERRIDE_VMETHOD
	{
		description->collectFontNames (names);
	}
	
};

//----------------------------------------------------------------------------------------------------
class ListController : public MenuController
{
public:
	ListController (IController* baseController, const std::string& attrName, UIDescription* description, UISelection* selection)
	: MenuController (baseController, attrName, description, false, false), selection (selection) {}

	void collectMenuItemNames (StringPtrList& names) VSTGUI_OVERRIDE_VMETHOD
	{
		const UIViewFactory* viewFactory = dynamic_cast<const UIViewFactory*>(description->getViewFactory ());
		if (viewFactory)
		{
			viewFactory->getPossibleAttributeListValues (selection->first (), attrName, names);
		}
	}
	
protected:
	SharedPointer<UISelection> selection;
};


} // namespace


//----------------------------------------------------------------------------------------------------
UIAttributesController::UIAttributesController (IController* baseController, UISelection* selection, UIUndoManager* undoManager, UIDescription* description)
: DelegationController (baseController)
, selection (selection)
, undoManager (undoManager)
, editDescription (description)
, attributeView (0)
, viewNameLabel (0)
, currentAttributeName (0)
, liveAction (0)
{
	selection->addDependency (this);
	undoManager->addDependency (this);
	description->addDependency (this);
}

//----------------------------------------------------------------------------------------------------
UIAttributesController::~UIAttributesController ()
{
	selection->removeDependency (this);
	undoManager->removeDependency (this);
	editDescription->removeDependency (this);
}

//----------------------------------------------------------------------------------------------------
void UIAttributesController::beginLiveAttributeChange (const std::string& name, const std::string& currentValue)
{
	liveAction = new AttributeChangeAction (editDescription, selection, name, currentValue);
	undoManager->startGroupAction (liveAction->getName ());
	undoManager->pushAndPerform (new AttributeChangeAction (editDescription, selection, name, currentValue));
}

//----------------------------------------------------------------------------------------------------
void UIAttributesController::endLiveAttributeChange ()
{
	if (liveAction)
	{
		liveAction->undo ();
		undoManager->pushAndPerform (liveAction);
		liveAction = 0;
		undoManager->endGroupAction ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIAttributesController::performAttributeChange (const std::string& name, const std::string& value)
{
	IAction* action = new AttributeChangeAction (editDescription, selection, name, value);
	if (liveAction)
	{
		delete liveAction;
		liveAction = action;
		action->perform ();
	}
	else
		undoManager->pushAndPerform (action);
}

//----------------------------------------------------------------------------------------------------
void UIAttributesController::valueChanged (CControl* control)
{
	switch (control->getTag ())
	{
		case kSearchFieldTag:
		{
			UISearchTextField* searchField = dynamic_cast<UISearchTextField*> (control);
			if (searchField)
			{
				filterString = searchField->getText ();
				rebuildAttributesView ();
				UIAttributes* attributes = editDescription->getCustomAttributes ("UIAttributesController", true);
				if (attributes)
				{
					attributes->setAttribute("UIAttributesController", filterString);
				}
			}
			break;
		}
	}
}

//----------------------------------------------------------------------------------------------------
CView* UIAttributesController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	if (attributeView == 0)
	{
		CRowColumnView* rcv = dynamic_cast<CRowColumnView*>(view);
		if (rcv)
		{
			attributeView = rcv;
		}
	}
	if (searchField == 0)
	{
		CTextEdit* textEdit = dynamic_cast<CTextEdit*>(view);
		if (textEdit && textEdit->getTag () == kSearchFieldTag)
		{
			searchField = textEdit;
			UIAttributes* attributes = editDescription->getCustomAttributes ("UIAttributesController", true);
			if (attributes)
			{
				const std::string* searchText = attributes->getAttributeValue ("SearchString");
				if (searchText)
				{
					searchField->setText (searchText->c_str ());
				}
			}
		}
	}
	if (viewNameLabel == 0)
	{
		CTextLabel* textLabel = dynamic_cast<CTextLabel*>(view);
		if (textLabel && textLabel->getTag () == kViewNameTag)
		{
			viewNameLabel = textLabel;
			viewNameLabel->setText ("No Selection");
		}
	}
	return DelegationController::verifyView (view, attributes, description);
}

//----------------------------------------------------------------------------------------------------
IControlListener* UIAttributesController::getControlListener (UTF8StringPtr name)
{
	return this;
}

//----------------------------------------------------------------------------------------------------
IController* UIAttributesController::createSubController (IdStringPtr _name, const IUIDescription* description)
{
	UTF8StringView name (_name);
	if (currentAttributeName)
	{
		if (name == "TextController")
		{
			return new UIAttributeControllers::TextController (this, *currentAttributeName);
		}
		else if (name == "BooleanController")
		{
			return new UIAttributeControllers::BooleanController (this, *currentAttributeName);
		}
		else if (name == "ColorController")
		{
			return new UIAttributeControllers::ColorController (this, *currentAttributeName, editDescription);
		}
		else if (name == "GradientController")
		{
			return new UIAttributeControllers::GradientController (this, *currentAttributeName, editDescription);
		}
		else if (name == "TagController")
		{
			return new UIAttributeControllers::TagController (this, *currentAttributeName, editDescription);
		}
		else if (name == "BitmapController")
		{
			return new UIAttributeControllers::BitmapController (this, *currentAttributeName, editDescription);
		}
		else if (name == "FontController")
		{
			return new UIAttributeControllers::FontController (this, *currentAttributeName, editDescription);
		}
		else if (name == "ListController")
		{
			return new UIAttributeControllers::ListController (this, *currentAttributeName, editDescription, selection);
		}
		else if (name == "TextAlignmentController")
		{
			return new UIAttributeControllers::TextAlignmentController (this, *currentAttributeName);
		}
		else if (name == "AutosizeController")
		{
			return new UIAttributeControllers::AutosizeController (this, selection, *currentAttributeName);
		}
	}
	return controller->createSubController (name, description);
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIAttributesController::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == UISelection::kMsgSelectionChanged)
	{
		if (timer == 0)
		{
			timer = new CVSTGUITimer (this, 10);
			timer->start ();
		}
		return kMessageNotified;
	}
	else if (message == UISelection::kMsgSelectionViewChanged || message == UIUndoManager::kMsgChanged)
	{
		validateAttributeViews ();
		return kMessageNotified;
	}
	else if (message == UIDescription::kMessageBitmapChanged
			|| message == UIDescription::kMessageColorChanged
			|| message == UIDescription::kMessageFontChanged
			|| message == UIDescription::kMessageTagChanged
			)
	{
		validateAttributeViews ();
		return kMessageNotified;
	}
	else if (message == CVSTGUITimer::kMsgTimer)
	{
		rebuildAttributesView ();
		timer = 0;
		return kMessageNotified;
	}
	return kMessageUnknown;
}

//----------------------------------------------------------------------------------------------------
void UIAttributesController::validateAttributeViews ()
{
	const UIViewFactory* viewFactory = static_cast<const UIViewFactory*> (editDescription->getViewFactory ());

	for (UIAttributeControllerList::const_iterator it = attributeControllers.begin (); it != attributeControllers.end (); it++)
	{
		std::string attrValue;
		bool first = true;
		bool hasDifferentValues = false;
		FOREACH_IN_SELECTION (selection, view)
			std::string temp;
			viewFactory->getAttributeValue (view, (*it)->getAttributeName (), temp, editDescription);
			if (temp != attrValue && !first)
				hasDifferentValues = true;
			attrValue = temp;
			first = false;
		FOREACH_IN_SELECTION_END
		(*it)->hasDifferentValues (hasDifferentValues);
		(*it)->setValue (attrValue);
	}
}

//----------------------------------------------------------------------------------------------------
CView* UIAttributesController::createValueViewForAttributeType (const UIViewFactory* viewFactory, CView* view, const std::string& attrName, IViewCreator::AttrType attrType)
{
	switch (attrType)
	{
		case IViewCreator::kFontType:
			return UIEditController::getEditorDescription ().createView ("attributes.font", this);
		case IViewCreator::kBitmapType:
			return UIEditController::getEditorDescription ().createView ("attributes.bitmap", this);
		case IViewCreator::kTagType:
			return UIEditController::getEditorDescription ().createView ("attributes.tag", this);
		case IViewCreator::kColorType:
			return UIEditController::getEditorDescription ().createView ("attributes.color", this);
		case IViewCreator::kGradientType:
			return UIEditController::getEditorDescription ().createView ("attributes.gradient", this);
		case IViewCreator::kBooleanType:
			return UIEditController::getEditorDescription ().createView ("attributes.boolean", this);
		case IViewCreator::kListType:
			return UIEditController::getEditorDescription ().createView ("attributes.list", this);
		case IViewCreator::kFloatType:
		case IViewCreator::kIntegerType:
		{
			double minValue, maxValue;
			if (viewFactory->getAttributeValueRange (view, attrName, minValue, maxValue))
			{
				CView* view = UIEditController::getEditorDescription ().createView ("attributes.number", this);
				if (view)
				{
					CViewContainer* container = dynamic_cast<CViewContainer*>(view);
					if (container)
					{
						std::vector<CSlider*> sliders;
						if (container->getChildViewsOfType<CSlider> (sliders) == 1)
						{
							sliders[0]->setMin (static_cast<float> (minValue));
							sliders[0]->setMax (static_cast<float> (maxValue));
						}
					}
					return view;
				}
			}
		}
		default:
			break;
	}
	return UIEditController::getEditorDescription ().createView ("attributes.text", this);
}

//----------------------------------------------------------------------------------------------------
CView* UIAttributesController::createViewForAttribute (const std::string& attrName)
{
	const CCoord height = 18;
	const CCoord width = 160;
	const CCoord margin = 2;
	CViewContainer* result = new CViewContainer (CRect (0, 0, width, height+2));
	result->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeColumn);
	result->setTransparency (true);

	CCoord middle = width/2;
	CTextLabel* label = new CTextLabel (CRect (5, 1, middle - margin, height+1), attrName.c_str ());
	label->setTextTruncateMode (CTextLabel::kTruncateHead);
	label->setTransparency (true);
	label->setHoriAlign (kRightText);
	label->setFontColor (kBlackCColor);
	label->setFont (kNormalFontSmall);
	label->setAutosizeFlags (kAutosizeAll);

	result->addView (label);
	
	bool hasDifferentValues = false;

	const UIViewFactory* viewFactory = static_cast<const UIViewFactory*> (editDescription->getViewFactory ());

	std::string attrValue;
	bool first = true;
	FOREACH_IN_SELECTION(selection, view)
		std::string temp;
		viewFactory->getAttributeValue (view, attrName, temp, editDescription);
		if (temp != attrValue && !first)
			hasDifferentValues = true;
		attrValue = temp;
		first = false;
	FOREACH_IN_SELECTION_END

	CRect r (middle+margin, 1, width-5, height+1);
	CView* valueView = 0;
	
	if (attrName == "text-alignment")
	{
		valueView = UIEditController::getEditorDescription ().createView ("attributes.text.alignment", this);
	}
	else if (attrName == "autosize")
	{
		valueView = UIEditController::getEditorDescription ().createView ("attributes.view.autosize", this);
	}
	
	if (valueView == 0)
	{
		CView* firstView = selection->first ();
		IViewCreator::AttrType attrType = viewFactory->getAttributeType (firstView, attrName);
		valueView = createValueViewForAttributeType (viewFactory, firstView, attrName, attrType);
	}
	if (valueView == 0) // fallcack if attributes.text template not defined
	{
		IController* controller = new UIAttributeControllers::TextController (this, *currentAttributeName);
		CTextEdit* textEdit = new CTextEdit (r, this, -1);
		textEdit->setText (attrValue.c_str ());
		textEdit->setTransparency (true);
		textEdit->setFontColor (kBlackCColor);
		textEdit->setFont (kNormalFontSmall);
		textEdit->setListener (controller);
		valueView = textEdit;
		valueView->setAttribute (kCViewControllerAttribute, sizeof (IController*), &controller);
	}
	if (valueView)
	{
		IController* controller = getViewController (valueView, true);
		if (controller)
		{
			UIAttributeControllers::Controller* c = dynamic_cast<UIAttributeControllers::Controller*>(controller);
			if (c)
			{
				c->hasDifferentValues (hasDifferentValues);
				c->setValue (attrValue);
				attributeControllers.push_back (c);
			}
		}
		r.setHeight (valueView->getHeight ());
		valueView->setViewSize (r);
		valueView->setMouseableArea (r);
		result->addView (valueView);
		r = result->getViewSize ();
		r.setHeight (valueView->getHeight()+2);
		result->setViewSize (r);
		result->setMouseableArea (r);
	}
	return result;
}

//----------------------------------------------------------------------------------------------------
void UIAttributesController::getConsolidatedAttributeNames (StringList& attrNames, const std::string& filter)
{
	const UIViewFactory* viewFactory = dynamic_cast<const UIViewFactory*> (editDescription->getViewFactory ());
	vstgui_assert (viewFactory);
	FOREACH_IN_SELECTION(selection, view)
		StringList temp;
		if (viewFactory->getAttributeNamesForView (view, temp))
		{
			StringList toRemove;
			if (attrNames.empty ())
				attrNames = temp;
			else
			{
				StringList::const_iterator it = attrNames.begin ();
				while (it != attrNames.end ())
				{
					bool found = std::find (temp.begin (), temp.end (), *it) != temp.end ();
					if (!found)
					{
						toRemove.push_back ((*it));
						temp.remove (*it);
					}
					it++;
				}
			}
			if (!filter.empty ())
			{
				for (StringList::reverse_iterator rit = temp.rbegin (); rit != temp.rend (); rit++)
				{
					std::string lowerCaseName (*rit);
					std::transform (lowerCaseName.begin (), lowerCaseName.end (), lowerCaseName.begin (), ::tolower);
					if (lowerCaseName.find (filter) == std::string::npos)
						toRemove.push_back (*rit);
				}
			}
			for (StringList::const_iterator it = toRemove.begin (); it != toRemove.end (); it++)
			{
				attrNames.remove ((*it));
			}
		}
	FOREACH_IN_SELECTION_END
}

//----------------------------------------------------------------------------------------------------
void UIAttributesController::rebuildAttributesView ()
{
	const IViewFactory* viewFactory = editDescription->getViewFactory ();
	if (attributeView == 0 || viewFactory == 0)
		return;

	attributeView->invalid ();
	attributeView->removeAll ();
	attributeControllers.clear ();

	std::string filter (filterString);
	std::transform (filter.begin (), filter.end (), filter.begin (), ::tolower);

	if (viewNameLabel)
	{
		int32_t selectedViews = selection->total ();
		if (selectedViews > 0)
		{
			UTF8StringPtr viewname = 0;
			FOREACH_IN_SELECTION(selection, view)
				UTF8StringPtr name = viewFactory->getViewName (view);
				if (viewname != 0 && UTF8StringView (name) != viewname)
				{
					viewname = 0;
					break;
				}
				viewname = name;
			FOREACH_IN_SELECTION_END
			if (viewname != 0)
			{
				if (selectedViews == 1)
					viewNameLabel->setText (viewname);
				else
				{
					std::stringstream str;
					str << selectedViews << "x " << viewname;
					viewNameLabel->setText (str.str ().c_str ());
				}
			}
			else
			{
				std::stringstream str;
				str << selectedViews << "x different views";
				viewNameLabel->setText (str.str ().c_str ());
			}
		}
		else
		{
			viewNameLabel->setText ("No Selection");
		}
	}


	StringList attrNames;
	getConsolidatedAttributeNames (attrNames, filter);
	if (attrNames.empty ())
	{
		CRect r (attributeView->getViewSize ());
		r.setHeight (0);
		attributeView->setViewSize (r);
		attributeView->setMouseableArea (r);
	}
	else
	{
		CCoord width = attributeView->getWidth () - (attributeView->getMargin ().left + attributeView->getMargin ().right);
		StringList::const_iterator it = attrNames.begin ();
		while (it != attrNames.end ())
		{
			currentAttributeName = &(*it);
			CView* view = createViewForAttribute ((*it));
			if (view)
			{
				CRect r = view->getViewSize ();
				r.setWidth (width);
				view->setViewSize (r);
				view->setMouseableArea (r);
				attributeView->addView (view);
			}
			it++;
		}
		currentAttributeName = 0;
		attributeView->sizeToFit ();
		attributeView->setMouseableArea (attributeView->getViewSize ());
	}
	attributeView->invalid ();
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
