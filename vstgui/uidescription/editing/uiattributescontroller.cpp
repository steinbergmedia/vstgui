// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uiattributescontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uiactions.h"
#include "uieditcontroller.h"
#include "../uiviewfactory.h"
#include "../uiattributes.h"
#include "../../lib/controls/coptionmenu.h"
#include "../../lib/controls/csearchtextedit.h"
#include "../../lib/controls/cslider.h"
#include "../../lib/coffscreencontext.h"
#include "../../lib/crowcolumnview.h"
#include "../../lib/iviewlistener.h"
#include "../../lib/cstring.h"
#include "../../lib/cgraphicspath.h"
#include <sstream>
#include <algorithm>
#include <cassert>

namespace VSTGUI {

namespace UIAttributeControllers {

//----------------------------------------------------------------------------------------------------
class Controller : public NonAtomicReferenceCounted, public DelegationController
{
public:
	Controller (IController* baseController, const std::string& attrName)
	: DelegationController (baseController), attrName (attrName), differentValues (false) {}
	
	const std::string& getAttributeName () const { return attrName; }
	virtual void setValue (const std::string& value) = 0;
	
	virtual void hasDifferentValues (bool state) { differentValues = state; }
	bool hasDifferentValues () const { return differentValues; }
protected:
	IControlListener* getControlListener (UTF8StringPtr controlTagName) override { return this; }
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

	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override
	{
		auto* control = dynamic_cast<CControl*>(view);
		if (control)
		{
			int32_t tag = control->getTag ();
			if (tag >= kLeftTag && tag <= kRightTag)
				controls[tag] = control;
		}
		return controller->verifyView (view, attributes, description);
	}

	void setValue (const std::string& value) override
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

	void valueChanged (CControl* control) override
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

	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override
	{
		auto* control = dynamic_cast<CControl*>(view);
		if (control)
		{
			int32_t tag = control->getTag ();
			if (tag >= kLeftTag && tag <= kColTag)
				controls[tag] = control;
			if (tag >= kRowTag && tag <= kColTag)
			{
				for (const auto& selView : *selection)
				{
					if (selView->asViewContainer () == nullptr)
					{
						controls[tag]->setVisible (false);
						break;
					}
				}
			}
		}
		return controller->verifyView (view, attributes, description);
	}

	void setValue (const std::string& value) override
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

	void valueChanged (CControl* control) override
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
	: Controller (baseController, attrName), control (nullptr) {}

	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override
	{
		if (control == nullptr)
		{
			control = dynamic_cast<CControl*>(view);
		}
		return controller->verifyView (view, attributes, description);
	}
	void valueChanged (CControl* pControl) override
	{
		if (pControl->getValue () == control->getMax ())
			performValueChange ("true");
		else
			performValueChange ("false");
	}
	void setValue (const std::string& value) override
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
class TextController : public Controller, public ViewListenerAdapter, public ITextLabelListener
{
public:
	TextController (IController* baseController, const std::string& attrName)
	: Controller (baseController, attrName) {}

	~TextController () override
	{
		if (textLabel)
		{
			textLabel->unregisterViewListener (this);
			textLabel->unregisterTextLabelListener (this);
		}
	}
	
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override
	{
		if (textLabel == nullptr)
		{
			auto* edit = dynamic_cast<CTextLabel*>(view);
			if (edit)
			{
				textLabel = edit;
				originalTextColor = textLabel->getFontColor ();
				textLabel->registerTextLabelListener (this);
				textLabel->registerViewListener (this);
			}
		}
		if (slider == nullptr)
		{
			auto* sliderView = dynamic_cast<CSlider*>(view);
			if (sliderView)
				slider = sliderView;
		}
		return controller->verifyView (view, attributes, description);
	}
	
	void controlBeginEdit (VSTGUI::CControl* pControl) override
	{
		if (pControl == slider)
		{
			getAttributesController ()->beginLiveAttributeChange (attrName, UIAttributes::doubleToString (slider->getValue ()));
		}
		Controller::controlBeginEdit (pControl);
	}
	
	void controlEndEdit (VSTGUI::CControl* pControl) override
	{
		if (pControl == slider)
		{
			getAttributesController ()->endLiveAttributeChange ();
		}
		Controller::controlEndEdit (pControl);
	}
	
	void valueChanged (CControl* pControl) override
	{
		if (textLabel == pControl)
		{
			textLabel->setFontColor (originalTextColor);
			performValueChange (textLabel->getText ());
		}
		else if (slider == pControl)
		{
			performValueChange (UIAttributes::doubleToString (slider->getValue ()).data ());
		}
	}
	
	void setValue (const std::string& value) override
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
				textLabel->setAttribute (kCViewTooltipAttribute, static_cast<uint32_t> (textLabel->getText ().length () + 1), textLabel->getText ().data ());
			else
				textLabel->removeAttribute (kCViewTooltipAttribute);
		}
		
	}

	void viewLostFocus (CView* view) override
	{
		if (view == textLabel)
		{
			SharedPointer<CTextEdit> textEdit = textLabel.cast<CTextEdit> ();
			if (textEdit && textEdit->bWasReturnPressed)
			{
				textEdit->getFrame ()->doAfterEventProcessing ([=] () {
					textEdit->takeFocus ();
				});
			}
		}
	}
	
	void onTextLabelTruncatedTextChanged (CTextLabel* label) override
	{
		UTF8StringPtr txt = label->getTruncatedText ();
		valueDisplayTruncated (txt);
	}
	
protected:
	SharedPointer<CTextLabel> textLabel;
	SharedPointer<CSlider> slider;
	CColor originalTextColor;
};

//----------------------------------------------------------------------------------------------------
class MenuController : public TextController, public OptionMenuListenerAdapter, public CommandMenuItemTargetAdapter
{
public:
	MenuController (IController* baseController, const std::string& attrName, UIDescription* description, bool addNoneItem = true, bool sortItems = true)
	: TextController (baseController, attrName), description (description), addNoneItem (addNoneItem), sortItems (sortItems) {}

	~MenuController () override
	{
		if (menu)
			menu->unregisterOptionMenuListener (this);
	}
	
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription*) override
	{
		if (menu == nullptr)
		{
			menu = dynamic_cast<COptionMenu*>(view);
			if (menu)
				menu->registerOptionMenuListener (this);
		}
		return TextController::verifyView (view, attributes, description);
	}

	using StringPtrList =std::list<const std::string*>;
	virtual void collectMenuItemNames (StringPtrList& names) = 0;
	virtual void validateMenuEntry (CCommandMenuItem* item) {}

	virtual void addMenuEntry (const std::string* entryName)
	{
		CCommandMenuItem* item = new CCommandMenuItem (CCommandMenuItem::Desc{entryName->data (), this});
		validateMenuEntry (item);
		menu->addEntry (item);
		if (textLabel->getText () == *entryName)
		{
			int32_t index = menu->getNbEntries () - 1;
			menu->setValue ((float)index);
			menu->setCurrent (index);
		}
	}

	void setValue (const std::string& value) override
	{
		TextController::setValue (value);
	}

	void onOptionMenuPrePopup (COptionMenu* optMenu) override
	{
		optMenu->removeAllEntry ();
		if (addNoneItem)
			optMenu->addEntry (new CCommandMenuItem (CCommandMenuItem::Desc{"None", 100, this}));
		StringPtrList names;
		collectMenuItemNames (names);
		if (sortItems)
			names.sort (UIEditController::std__stringCompare);
		if (addNoneItem && !names.empty ())
			optMenu->addSeparator ();
		for (const auto& name : names)
			addMenuEntry (name);
	}

	bool onCommandMenuItemSelected (CCommandMenuItem* item) override
	{
		performValueChange (item->getTag () == 100 ? UTF8String ("") : item->getTitle ());
		return true;
	}

	void valueDisplayTruncated (UTF8StringPtr txt) override
	{
		if (textLabel && menu)
		{
			if (txt && *txt != 0)
				menu->setAttribute (kCViewTooltipAttribute, static_cast<uint32_t> (textLabel->getText ().length () + 1), textLabel->getText ().data ());
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

	void collectMenuItemNames (StringPtrList& names) override
	{
		description->collectColorNames (names);
	}
	
	void validateMenuEntry (CCommandMenuItem* item) override
	{
		const CCoord size = 15;
		CColor color;
		if (description->getColor (item->getTitle (), color))
		{
			
			if (auto context = COffscreenContext::create ({size, size}))
			{
				context->beginDraw ();
				context->setFillColor (color);
				context->drawRect (CRect (0, 0, size, size), kDrawFilled);
				context->endDraw ();
				item->setIcon (context->getBitmap ());
			}
		}
	}
	
	void setValue (const std::string& value) override
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

	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override
	{
		const std::string* attr = attributes.getAttributeValue (IUIDescription::kCustomViewName);
		if (attr && *attr == "ColorView")
		{
			colorView = new ColorView ();
			return colorView;
		}
		return nullptr;
	}
protected:
	class ColorView : public CView
	{
	public:
		ColorView () : CView (CRect (0, 0, 0, 0)) ,color (kTransparentCColor) {}
		void draw (CDrawContext* context) override
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

	void collectMenuItemNames (StringPtrList& names) override
	{
		description->collectGradientNames (names);
	}
	
	void validateMenuEntry (CCommandMenuItem* item) override
	{
		const CCoord size = 15;
		if (CGradient* gradient = description->getGradient (item->getTitle ()))
		{
			if (auto context = COffscreenContext::create ({size, size}))
			{
				context->beginDraw ();
				SharedPointer<CGraphicsPath> path = owned (context->createGraphicsPath ());
				path->addRect (CRect (0, 0, size, size));
				context->fillLinearGradient(path, *gradient, CPoint (0, 0), CPoint (size, 0));
				context->endDraw ();
				item->setIcon (context->getBitmap ());
			}
		}
	}
	
	void setValue (const std::string& value) override
	{
		MenuController::setValue (value);
		if (gradientView)
		{
			if (hasDifferentValues ())
			{
				gradientView->gradient = nullptr;
			}
			else
			{
				gradientView->gradient = description->getGradient (value.c_str ());
			}
			gradientView->invalid ();
		}
	}

	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override
	{
		const std::string* attr = attributes.getAttributeValue (IUIDescription::kCustomViewName);
		if (attr && *attr == "GradientView")
		{
			gradientView = new GradientView ();
			return gradientView;
		}
		return nullptr;
	}
protected:
	class GradientView : public CView
	{
	public:
		GradientView () : CView (CRect (0, 0, 0, 0)) {}
		void draw (CDrawContext* context) override
		{
			if (gradient == nullptr)
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

	void collectMenuItemNames (StringPtrList& names) override
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

	void collectMenuItemNames (StringPtrList& names) override
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

	void collectMenuItemNames (StringPtrList& names) override
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

	void collectMenuItemNames (StringPtrList& names) override
	{
		const auto* viewFactory = dynamic_cast<const UIViewFactory*>(description->getViewFactory ());
		if (viewFactory)
		{
			viewFactory->getPossibleAttributeListValues (selection->first (), attrName, names);
		}
	}
	
protected:
	SharedPointer<UISelection> selection;
};


} // VSTGUI


//----------------------------------------------------------------------------------------------------
UIAttributesController::UIAttributesController (IController* baseController, UISelection* selection, UIUndoManager* undoManager, UIDescription* description)
: DelegationController (baseController)
, selection (selection)
, undoManager (undoManager)
, editDescription (description)
, liveAction (nullptr)
, viewNameLabel (nullptr)
, attributeView (nullptr)
, currentAttributeName (nullptr)
{
	selection->registerListener (this);
	undoManager->registerListener (this);
	description->registerListener (this);

	UIEditController::getEditorDescription ()->getColor ("control.font", attributeNameColor);
}

//----------------------------------------------------------------------------------------------------
UIAttributesController::~UIAttributesController ()
{
	if (viewNameLabel)
		viewNameLabel->unregisterViewListener (this);
	if (attributeView)
		attributeView->unregisterViewListener (this);
	selection->unregisterListener (this);
	undoManager->unregisterListener (this);
	editDescription->unregisterListener (this);
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
		liveAction = nullptr;
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
			if (auto sf = dynamic_cast<CSearchTextEdit*> (control))
			{
				filterString = sf->getText ();
				rebuildAttributesView ();
				auto attributes = editDescription->getCustomAttributes ("UIAttributesController", true);
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
	if (attributeView == nullptr)
	{
		auto* rcv = dynamic_cast<CRowColumnView*>(view);
		if (rcv)
		{
			attributeView = rcv;
			attributeView->registerViewListener (this);
		}
	}
	if (searchField == nullptr)
	{
		auto* textEdit = dynamic_cast<CTextEdit*>(view);
		if (textEdit && textEdit->getTag () == kSearchFieldTag)
		{
			searchField = textEdit;
			auto attrs = editDescription->getCustomAttributes ("UIAttributesController", true);
			if (attrs)
			{
				const std::string* searchText = attrs->getAttributeValue ("SearchString");
				if (searchText)
				{
					searchField->setText (searchText->c_str ());
				}
			}
		}
	}
	if (viewNameLabel == nullptr)
	{
		auto* textLabel = dynamic_cast<CTextLabel*>(view);
		if (textLabel && textLabel->getTag () == kViewNameTag)
		{
			viewNameLabel = textLabel;
			viewNameLabel->setText ("No Selection");
			viewNameLabel->registerViewListener (this);
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
void UIAttributesController::onUIDescTagChanged (UIDescription* desc)
{
	validateAttributeViews ();
}

//----------------------------------------------------------------------------------------------------
void UIAttributesController::onUIDescColorChanged (UIDescription* desc)
{
	validateAttributeViews ();
}

//----------------------------------------------------------------------------------------------------
void UIAttributesController::onUIDescFontChanged (UIDescription* desc)
{
	validateAttributeViews ();
}

//----------------------------------------------------------------------------------------------------
void UIAttributesController::onUIDescBitmapChanged (UIDescription* desc)
{
	validateAttributeViews ();
}

//----------------------------------------------------------------------------------------------------
void UIAttributesController::onUIDescTemplateChanged (UIDescription* desc)
{
	validateAttributeViews ();
}

//----------------------------------------------------------------------------------------------------
void UIAttributesController::onUIDescGradientChanged (UIDescription* desc)
{
	validateAttributeViews ();
}

//----------------------------------------------------------------------------------------------------
void UIAttributesController::selectionDidChange (UISelection*)
{
	if (!rebuildRequested && attributeView)
	{
		if (auto frame = attributeView->getFrame ())
		{
			if (frame->inEventProcessing ())
			{
				rebuildRequested = true;
				frame->doAfterEventProcessing ([Self = shared (this)] () {
					Self->rebuildAttributesView ();
					Self->rebuildRequested = false;
				});
			}
		}
		if (!rebuildRequested)
			rebuildAttributesView ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIAttributesController::selectionViewsDidChange (UISelection*)
{
	validateAttributeViews ();
}

//----------------------------------------------------------------------------------------------------
void UIAttributesController::onUndoManagerChange ()
{
	validateAttributeViews ();
}

//----------------------------------------------------------------------------------------------------
void UIAttributesController::validateAttributeViews ()
{
	const auto* viewFactory = static_cast<const UIViewFactory*> (editDescription->getViewFactory ());

	for (auto& controller : attributeControllers)
	{
		std::string attrValue;
		bool first = true;
		bool hasDifferentValues = false;
		for (const auto& view : *selection)
		{
			std::string temp;
			viewFactory->getAttributeValue (view, controller->getAttributeName (), temp, editDescription);
			if (temp != attrValue && !first)
				hasDifferentValues = true;
			attrValue = temp;
			first = false;
		}
		controller->hasDifferentValues (hasDifferentValues);
		controller->setValue (attrValue);
	}
}

//----------------------------------------------------------------------------------------------------
CView* UIAttributesController::createValueViewForAttributeType (const UIViewFactory* viewFactory, CView* view, const std::string& attrName, IViewCreator::AttrType attrType)
{
	auto editorDescription = UIEditController::getEditorDescription ();
	if (!editDescription)
		return nullptr;
	switch (attrType)
	{
		case IViewCreator::kFontType:
			return editorDescription->createView ("attributes.font", this);
		case IViewCreator::kBitmapType:
			return editorDescription->createView ("attributes.bitmap", this);
		case IViewCreator::kTagType:
			return editorDescription->createView ("attributes.tag", this);
		case IViewCreator::kColorType:
			return editorDescription->createView ("attributes.color", this);
		case IViewCreator::kGradientType:
			return editorDescription->createView ("attributes.gradient", this);
		case IViewCreator::kBooleanType:
			return editorDescription->createView ("attributes.boolean", this);
		case IViewCreator::kListType:
			return editorDescription->createView ("attributes.list", this);
		case IViewCreator::kFloatType:
		case IViewCreator::kIntegerType:
		{
			double minValue, maxValue;
			if (viewFactory->getAttributeValueRange (view, attrName, minValue, maxValue))
			{
				CView* valueView = editorDescription->createView ("attributes.number", this);
				if (valueView)
				{
					if (auto container = valueView->asViewContainer ())
					{
						std::vector<CSlider*> sliders;
						if (container->getChildViewsOfType<CSlider> (sliders) == 1)
						{
							sliders[0]->setMin (static_cast<float> (minValue));
							sliders[0]->setMax (static_cast<float> (maxValue));
						}
					}
					return valueView;
				}
			}
		}
		default:
			break;
	}
	return editorDescription->createView ("attributes.text", this);
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
	label->setFontColor (attributeNameColor);
	label->setFont (kNormalFontSmall);
	label->setAutosizeFlags (kAutosizeAll);

	result->addView (label);
	
	bool hasDifferentValues = false;

	const auto* viewFactory = static_cast<const UIViewFactory*> (editDescription->getViewFactory ());

	std::string attrValue;
	bool first = true;
	for (const auto& view : *selection)
	{
		std::string temp;
		viewFactory->getAttributeValue (view, attrName, temp, editDescription);
		if (temp != attrValue && !first)
			hasDifferentValues = true;
		attrValue = temp;
		first = false;
	}

	CRect r (middle+margin, 1, width-5, height+1);
	CView* valueView = nullptr;
	
	if (attrName == "text-alignment")
	{
		valueView = UIEditController::getEditorDescription ()->createView ("attributes.text.alignment", this);
	}
	else if (attrName == "autosize")
	{
		valueView = UIEditController::getEditorDescription ()->createView ("attributes.view.autosize", this);
	}
	
	if (valueView == nullptr)
	{
		CView* firstView = selection->first ();
		IViewCreator::AttrType attrType = viewFactory->getAttributeType (firstView, attrName);
		valueView = createValueViewForAttributeType (viewFactory, firstView, attrName, attrType);
	}
	if (valueView == nullptr) // fallcack if attributes.text template not defined
	{
		IController* controller = new UIAttributeControllers::TextController (this, *currentAttributeName);
		auto* textEdit = new CTextEdit (r, this, -1);
		textEdit->setText (attrValue.c_str ());
		textEdit->setTransparency (true);
		textEdit->setFontColor (kBlackCColor);
		textEdit->setFont (kNormalFontSmall);
		textEdit->setListener (controller);
		valueView = textEdit;
		valueView->setAttribute (kCViewControllerAttribute, controller);
	}
	if (valueView)
	{
		IController* controller = getViewController (valueView, true);
		if (controller)
		{
			auto* c = dynamic_cast<UIAttributeControllers::Controller*>(controller);
			if (c)
			{
				c->hasDifferentValues (hasDifferentValues);
				c->setValue (attrValue);
				attributeControllers.emplace_back (c);
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
	const auto* viewFactory = dynamic_cast<const UIViewFactory*> (editDescription->getViewFactory ());
	vstgui_assert (viewFactory);
	if (!viewFactory)
		return;
	for (const auto& view : *selection)
	{
		StringList temp;
		if (viewFactory->getAttributeNamesForView (view, temp))
		{
			StringList toRemove;
			if (attrNames.empty ())
				attrNames = temp;
			else
			{
				for (auto& attrName : attrNames)
				{
					bool found = std::find (temp.begin (), temp.end (), attrName) != temp.end ();
					if (!found)
					{
						toRemove.emplace_back (attrName);
						temp.remove (attrName);
					}
				}
			}
			if (!filter.empty ())
			{
				for (auto rit = temp.rbegin (); rit != temp.rend (); ++rit)
				{
					std::string lowerCaseName (*rit);
					std::transform (lowerCaseName.begin (), lowerCaseName.end (), lowerCaseName.begin (), ::tolower);
					if (lowerCaseName.find (filter) == std::string::npos)
						toRemove.emplace_back (*rit);
				}
			}
			for (auto& attrName : toRemove)
			{
				attrNames.remove (attrName);
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIAttributesController::rebuildAttributesView ()
{
	auto viewFactory = dynamic_cast<const UIViewFactory*> (editDescription->getViewFactory ());
	if (attributeView == nullptr || viewFactory == nullptr)
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
			UTF8StringPtr viewname = nullptr;
			for (const auto& view : *selection)
			{
				UTF8StringPtr name = viewFactory->getViewDisplayName (view);
				if (viewname != nullptr && UTF8StringView (name) != viewname)
				{
					viewname = nullptr;
					break;
				}
				viewname = name;
			}
			if (viewname != nullptr)
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
		for (const auto& name : attrNames)
		{
			currentAttributeName = &name;
			CView* view = createViewForAttribute (name);
			if (view)
			{
				CRect r = view->getViewSize ();
				r.setWidth (width);
				view->setViewSize (r);
				view->setMouseableArea (r);
				attributeView->addView (view);
			}
		}
		currentAttributeName = nullptr;
		attributeView->sizeToFit ();
		attributeView->setMouseableArea (attributeView->getViewSize ());
	}
	attributeView->invalid ();
}

//----------------------------------------------------------------------------------------------------
void UIAttributesController::viewWillDelete (CView* view)
{
	if (view == attributeView)
		attributeView = nullptr;
	else if (view == viewNameLabel)
		viewNameLabel = nullptr;

	view->unregisterViewListener (this);
}

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
