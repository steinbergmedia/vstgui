//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins
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

#include "uiattributescontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uiactions.h"
#include "uieditcontroller.h"
#include "uisearchtextfield.h"
#include "../uiviewfactory.h"
#include "../../lib/controls/coptionmenu.h"
#include "../../lib/coffscreencontext.h"
#include "../../lib/crowcolumnview.h"
#include <algorithm>

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
	CControlListener* getControlListener (UTF8StringPtr controlTagName) { return this; }
	void performValueChange (UTF8StringPtr value)
	{
		hasDifferentValues (false);
		std::string valueStr = value ? value : "";
		UIAttributesController* attrController = dynamic_cast<UIAttributesController*> (controller);
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

	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
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

	void setValue (const std::string& value)
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

	void valueChanged (CControl* control)
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

	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
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

	void setValue (const std::string& value)
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

	void valueChanged (CControl* control)
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

	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
	{
		if (control == 0)
		{
			control = dynamic_cast<CControl*>(view);
		}
		return controller->verifyView (view, attributes, description);
	}
	void valueChanged (CControl* pControl)
	{
		if (pControl->getValue () == control->getMax ())
			performValueChange ("true");
		else
			performValueChange ("false");
	}
	void setValue (const std::string& value)
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
class TextController : public Controller
{
public:
	TextController (IController* baseController, const std::string& attrName)
	: Controller (baseController, attrName), label (0) {}

	~TextController ()
	{
		label->removeDependency (this);
	}
	
	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
	{
		if (label == 0)
		{
			CTextLabel* edit = dynamic_cast<CTextLabel*>(view);
			if (edit)
			{
				label = edit;
				originalTextColor = label->getFontColor ();
				label->addDependency (this);
			}
		}
		return controller->verifyView (view, attributes, description);
	}
	void valueChanged (CControl* pControl)
	{
		CTextLabel* edit = dynamic_cast<CTextLabel*>(pControl);
		if (edit)
		{
			label->setFontColor (originalTextColor);
			performValueChange (edit->getText ());
		}
	}
	void setValue (const std::string& value)
	{
		if (label)
		{
			if (hasDifferentValues ())
			{
				CColor newColor (originalTextColor);
				newColor.alpha /= 2;
				label->setFontColor (newColor);
				label->setText ("Multiple Values");
			}
			else
			{
				label->setText (value.c_str ());
			}
		}
	}

	virtual void valueDisplayTruncated (UTF8StringPtr txt)
	{
		if (label)
		{
			if (txt && *txt != 0)
				label->setAttribute (kCViewTooltipAttribute, (int32_t)strlen (label->getText ())+1, label->getText ());
			else
				label->removeAttribute (kCViewTooltipAttribute);
		}
		
	}

	CMessageResult notify (CBaseObject* sender, IdStringPtr message)
	{
		if (message == CTextLabel::kMsgTruncatedTextChanged)
		{
			UTF8StringPtr txt = label->getTruncatedText ();
			valueDisplayTruncated (txt);
			return kMessageNotified;
		}
		return kMessageUnknown;
	}
protected:
	SharedPointer<CTextLabel> label;
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
	
	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
	{
		if (menu == 0)
		{
			menu = dynamic_cast<COptionMenu*>(view);
			if (menu)
				menu->addDependency (this);
		}
		return TextController::verifyView (view, attributes, description);
	}

	virtual void collectMenuItemNames (std::list<const std::string*>& names) = 0;
	virtual void validateMenuEntry (CCommandMenuItem* item) {}

	virtual void addMenuEntry (const std::string* entryName)
	{
		CCommandMenuItem* item = new CCommandMenuItem (entryName->c_str (), this);
		validateMenuEntry (item);
		menu->addEntry (item);
		if (*entryName == label->getText ())
		{
			int32_t index = menu->getNbEntries () - 1;
			menu->setValue ((float)index);
			menu->setCurrent (index);
		}
	}

	void setValue (const std::string& value)
	{
		TextController::setValue (value);
	}

	CMessageResult notify (CBaseObject* sender, IdStringPtr message)
	{
		if (sender == menu && message == COptionMenu::kMsgBeforePopup)
		{
			menu->removeAllEntry ();
			if (addNoneItem)
				menu->addEntry (new CCommandMenuItem ("None", 100, this));
			std::list<const std::string*> names;
			collectMenuItemNames (names);
			if (sortItems)
				names.sort (UIEditController::std__stringCompare);
			if (addNoneItem && !names.empty ())
				menu->addSeparator ();
			for (std::list<const std::string*>::const_iterator it = names.begin (); it != names.end (); it++)
			{
				addMenuEntry (*it);
			}
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

	virtual void valueDisplayTruncated (UTF8StringPtr txt)
	{
		if (label && menu)
		{
			if (txt && *txt != 0)
				menu->setAttribute (kCViewTooltipAttribute, (int32_t)strlen (label->getText ())+1, label->getText ());
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

	virtual void collectMenuItemNames (std::list<const std::string*>& names)
	{
		description->collectColorNames (names);
	}
	
	virtual void validateMenuEntry (CCommandMenuItem* item)
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
	
	void setValue (const std::string& value)
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

	CView* createView (const UIAttributes& attributes, IUIDescription* description)
	{
		const std::string* attr = attributes.getAttributeValue ("custom-view-name");
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
		void draw (CDrawContext* context)
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
class TagController : public MenuController
{
public:
	TagController (IController* baseController, const std::string& attrName, UIDescription* description)
	: MenuController (baseController, attrName, description) {}

	virtual void collectMenuItemNames (std::list<const std::string*>& names)
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

	virtual void collectMenuItemNames (std::list<const std::string*>& names)
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

	virtual void collectMenuItemNames (std::list<const std::string*>& names)
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

	virtual void collectMenuItemNames (std::list<const std::string*>& names)
	{
		UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*>(description->getViewFactory ());
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
void UIAttributesController::performAttributeChange (const std::string& name, const std::string& value)
{
	IAction* action = new AttributeChangeAction (editDescription, selection, name, value);
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
				filterString = searchField->getText () ? searchField->getText () : "";
				rebuildAttributesView ();
				UIAttributes* attributes = editDescription->getCustomAttributes ("UIAttributesController", true);
				if (attributes)
				{
					attributes->setAttribute("UIAttributesController", filterString.c_str ());
				}
			}
			break;
		}
	}
}

//----------------------------------------------------------------------------------------------------
CView* UIAttributesController::verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
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
CControlListener* UIAttributesController::getControlListener (UTF8StringPtr name)
{
	return this;
}

//----------------------------------------------------------------------------------------------------
IController* UIAttributesController::createSubController (IdStringPtr name, IUIDescription* description)
{
	if (currentAttributeName)
	{
		if (strcmp (name, "TextController") == 0)
		{
			return new UIAttributeControllers::TextController (this, *currentAttributeName);
		}
		else if (strcmp (name, "BooleanController") == 0)
		{
			return new UIAttributeControllers::BooleanController (this, *currentAttributeName);
		}
		else if (strcmp (name, "ColorController") == 0)
		{
			return new UIAttributeControllers::ColorController (this, *currentAttributeName, editDescription);
		}
		else if (strcmp (name, "TagController") == 0)
		{
			return new UIAttributeControllers::TagController (this, *currentAttributeName, editDescription);
		}
		else if (strcmp (name, "BitmapController") == 0)
		{
			return new UIAttributeControllers::BitmapController (this, *currentAttributeName, editDescription);
		}
		else if (strcmp (name, "FontController") == 0)
		{
			return new UIAttributeControllers::FontController (this, *currentAttributeName, editDescription);
		}
		else if (strcmp (name, "ListController") == 0)
		{
			return new UIAttributeControllers::ListController (this, *currentAttributeName, editDescription, selection);
		}
		else if (strcmp (name, "TextAlignmentController") == 0)
		{
			return new UIAttributeControllers::TextAlignmentController (this, *currentAttributeName);
		}
		else if (strcmp (name, "AutosizeController") == 0)
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
	UIViewFactory* viewFactory = static_cast<UIViewFactory*> (editDescription->getViewFactory ());

	for (std::list<UIAttributeControllers::Controller*>::const_iterator it = attributeControllers.begin (); it != attributeControllers.end (); it++)
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

	UIViewFactory* viewFactory = static_cast<UIViewFactory*> (editDescription->getViewFactory ());

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
		switch (attrType)
		{
			case IViewCreator::kFontType:
			{
				valueView = UIEditController::getEditorDescription ().createView ("attributes.font", this);
				break;
			}
			case IViewCreator::kBitmapType:
			{
				valueView = UIEditController::getEditorDescription ().createView ("attributes.bitmap", this);
				break;
			}
			case IViewCreator::kTagType:
			{
				valueView = UIEditController::getEditorDescription ().createView ("attributes.tag", this);
				break;
			}
			case IViewCreator::kColorType:
			{
				valueView = UIEditController::getEditorDescription ().createView ("attributes.color", this);
				break;
			}
			case IViewCreator::kBooleanType:
			{
				valueView = UIEditController::getEditorDescription ().createView ("attributes.boolean", this);
				break;
			}
			case IViewCreator::kListType:
			{
				valueView = UIEditController::getEditorDescription ().createView ("attributes.list", this);
				break;
			}
			default:
			{
				valueView = UIEditController::getEditorDescription ().createView ("attributes.text", this);
				break;
			}
		}
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
void UIAttributesController::rebuildAttributesView ()
{
	UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*> (editDescription->getViewFactory ());
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
		if (selectedViews == 1)
		{
			CView* view = selection->first ();
			UTF8StringPtr viewname = viewFactory->getViewName (view);
			viewNameLabel->setText (viewname);
		}
		else if (selectedViews > 1)
		{
			viewNameLabel->setText ("Multiple Selection");
		}
		else
		{
			viewNameLabel->setText ("No Selection");
		}
	}
	std::list<std::string> attrNames;
	FOREACH_IN_SELECTION(selection, view)
		std::list<std::string> temp;
		if (viewFactory->getAttributeNamesForView (view, temp))
		{
			std::list<std::string> toRemove;
			if (attrNames.empty ())
				attrNames = temp;
			else
			{
				std::list<std::string>::const_iterator it = attrNames.begin ();
				while (it != attrNames.end ())
				{
					bool found = false;
					std::list<std::string>::const_iterator it2 = temp.begin ();
					while (it2 != temp.end ())
					{
						if ((*it) == (*it2))
						{
							found = true;
							break;
						}
						it2++;
					}
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
				for (std::list<std::string>::reverse_iterator rit = temp.rbegin (); rit != temp.rend (); rit++)
				{
					std::string lowerCaseName (*rit);
					std::transform (lowerCaseName.begin (), lowerCaseName.end (), lowerCaseName.begin (), ::tolower);
					if (lowerCaseName.find (filter) == std::string::npos)
						toRemove.push_back (*rit);
				}
			}
			for (std::list<std::string>::const_iterator it = toRemove.begin (); it != toRemove.end (); it++)
			{
				attrNames.remove ((*it));
			}
		}
	FOREACH_IN_SELECTION_END
	if (attrNames.empty ())
	{
		CRect r (attributeView->getViewSize ());
		r.setHeight (0);
		attributeView->setViewSize (r);
		attributeView->setMouseableArea (r);
	}
	else
	{
		CCoord width = attributeView->getWidth () - (attributeView->getMargin().left + attributeView->getMargin().right);
		std::list<std::string>::const_iterator it = attrNames.begin ();
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
