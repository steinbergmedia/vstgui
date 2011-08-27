#include "uifontscontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uieditcontroller.h"
#include "uisearchtextfield.h"
#include "uibasedatasource.h"
#include "../../lib/controls/ccolorchooser.h"
#include "../../lib/controls/coptionmenu.h"
#include <sstream>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIFontsDataSource : public UIBaseDataSource
{
public:
	UIFontsDataSource (UIDescription* description, IActionOperator* actionOperator, IGenericStringListDataBrowserSourceSelectionChanged* delegate);
	
protected:
	virtual void getNames (std::list<const std::string*>& names);
	virtual bool addItem (UTF8StringPtr name);
	virtual bool removeItem (UTF8StringPtr name);
	virtual bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName);
	virtual UTF8StringPtr getDefaultsName () { return "UIFontsDataSource"; }

	SharedPointer<CColorChooser> colorChooser;
};

//----------------------------------------------------------------------------------------------------
UIFontsDataSource::UIFontsDataSource (UIDescription* description, IActionOperator* actionOperator, IGenericStringListDataBrowserSourceSelectionChanged* delegate)
: UIBaseDataSource (description, actionOperator, UIDescription::kMessageFontChanged, delegate)
{
}

//----------------------------------------------------------------------------------------------------
void UIFontsDataSource::getNames (std::list<const std::string*>& names)
{
	description->collectFontNames (names);
}

//----------------------------------------------------------------------------------------------------
bool UIFontsDataSource::addItem (UTF8StringPtr name)
{
	actionOperator->performFontChange (name, kNormalFont);
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIFontsDataSource::removeItem (UTF8StringPtr name)
{
	actionOperator->performFontChange (name, kNormalFont, true);
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIFontsDataSource::performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	actionOperator->performFontNameChange (oldName, newName);
	return true;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIFontsController::UIFontsController (IController* baseController, UIDescription* description, IActionOperator* actionOperator)
: DelegationController (baseController)
, editDescription (description)
, actionOperator (actionOperator)
, dataSource (0)
, fontMenu (0)
, altTextEdit (0)
, sizeTextEdit (0)
, boldControl (0)
, italicControl (0)
, strikethroughControl (0)
, underlineControl (0)
{
	dataSource = new UIFontsDataSource (editDescription, actionOperator, this);
	UIEditController::setupDataSource (dataSource);
}

//----------------------------------------------------------------------------------------------------
UIFontsController::~UIFontsController ()
{
	dataSource->forget ();
}

//----------------------------------------------------------------------------------------------------
CView* UIFontsController::createView (const UIAttributes& attributes, IUIDescription* description)
{
	const std::string* name = attributes.getAttributeValue ("custom-view-name");
	if (name)
	{
		if (*name == "FontsBrowser")
		{
			CDataBrowser* dataBrowser = new CDataBrowser (CRect (0, 0, 0, 0), 0, dataSource, CDataBrowser::kDrawRowLines|CScrollView::kHorizontalScrollbar | CScrollView::kVerticalScrollbar);
			return dataBrowser;
		}
	}
	return DelegationController::createView (attributes, description);
}

//----------------------------------------------------------------------------------------------------
CView* UIFontsController::verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
{
	UISearchTextField* searchField = dynamic_cast<UISearchTextField*>(view);
	if (searchField && searchField->getTag () == kSearchTag)
	{
		dataSource->setSearchFieldControl (searchField);
		return searchField;
	}
	CControl* control = dynamic_cast<CControl*>(view);
	if (control)
	{
		switch (control->getTag ())
		{
			case kFontMainTag:
			{
				fontMenu = dynamic_cast<COptionMenu*>(control);
				std::list<std::string> fontFamilyNames;
				if (IPlatformFont::getAllPlatformFontFamilies (fontFamilyNames))
				{
					fontFamilyNames.sort ();
					for (std::list<std::string>::const_iterator it = fontFamilyNames.begin (); it != fontFamilyNames.end (); it++)
					{
						fontMenu->addEntry ((*it).c_str ());
					}
				}
				break;
			}
			case kFontAltTag:
			{
				altTextEdit = dynamic_cast<CTextEdit*>(control);
				break;
			}
			case kFontSizeTag:
			{
				sizeTextEdit = dynamic_cast<CTextEdit*>(control);
				if (sizeTextEdit)
				{
					sizeTextEdit->setValueToStringProc (valueToString);
					sizeTextEdit->setStringToValueProc (stringToValue);
				}
				break;
			}
			case kFontStyleBoldTag:
			{
				boldControl = control;
				break;
			}
			case kFontStyleItalicTag:
			{
				italicControl = control;
				break;
			}
			case kFontStyleStrikethroughTag:
			{
				strikethroughControl = control;
				break;
			}
			case kFontStyleUnderlineTag:
			{
				underlineControl = control;
				break;
			}
		}
	}
	return controller->verifyView (view, attributes, description);
}

//----------------------------------------------------------------------------------------------------
CControlListener* UIFontsController::getControlListener (UTF8StringPtr name)
{
	return this;
}

//----------------------------------------------------------------------------------------------------
void UIFontsController::valueChanged (CControl* pControl)
{
	switch (pControl->getTag ())
	{
		case kAddTag:
		{
			if (pControl->getValue () == pControl->getMax ())
			{
				dataSource->add ();
			}
			break;
		}
		case kRemoveTag:
		{
			if (pControl->getValue () == pControl->getMax ())
			{
				dataSource->remove ();
			}
			break;
		}
		case kFontMainTag:
		case kFontAltTag:
		case kFontSizeTag:
		case kFontStyleBoldTag:
		case kFontStyleItalicTag:
		case kFontStyleStrikethroughTag:
		case kFontStyleUnderlineTag:
		{
			if (fontMenu == 0 || sizeTextEdit == 0 || selectedFont.empty ())
				break;
			CMenuItem* menuItem = fontMenu->getCurrent ();
			if (menuItem)
			{
				int32_t style = 0;
				if (boldControl && boldControl->getValue () > 0)
					style |= kBoldFace;
				if (italicControl && italicControl->getValue () > 0)
					style |= kItalicFace;
				if (underlineControl && underlineControl->getValue () > 0)
					style |= kUnderlineFace;
				if (strikethroughControl && strikethroughControl->getValue () > 0)
					style |= kStrikethroughFace;
				OwningPointer<CFontDesc> font (new CFontDesc (menuItem->getTitle (), sizeTextEdit->getValue (), style));
				actionOperator->performFontChange (selectedFont.c_str (), font);
			}
			break;
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIFontsController::dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source)
{
	selectedFont = selectedRow != CDataBrowser::kNoSelection ? dataSource->getStringList ()->at (selectedRow).c_str () : "";
	CFontRef font = editDescription->getFont (selectedFont.c_str ());
	if (font)
	{
		if (fontMenu && font->getName ())
		{
			std::string fontName = font->getName ();
			CMenuItemList* items = fontMenu->getItems ();
			int32_t index = 0;
			for (CConstMenuItemIterator it = items->begin (); it != items->end (); it++, index++)
			{
				if (fontName == (*it)->getTitle ())
				{
					fontMenu->setValue ((float)index);
					break;
				}
			}
		}
		if (sizeTextEdit)
		{
			std::stringstream str;
			str << font->getSize ();
			sizeTextEdit->setText (str.str ().c_str ());
		}
		if (boldControl)
		{
			boldControl->setValue (font->getStyle () & kBoldFace ? true : false);
			boldControl->invalid ();
		}
		if (italicControl)
		{
			italicControl->setValue (font->getStyle () & kItalicFace ? true : false);
			italicControl->invalid ();
		}
		if (underlineControl)
		{
			underlineControl->setValue (font->getStyle () & kUnderlineFace ? true : false);
			underlineControl->invalid ();
		}
		if (strikethroughControl)
		{
			strikethroughControl->setValue (font->getStyle () & kStrikethroughFace ? true : false);
			strikethroughControl->invalid ();
		}
	}
}

//----------------------------------------------------------------------------------------------------
bool UIFontsController::valueToString (float value, char utf8String[256], void* userData)
{
	int32_t intValue = (int32_t)value;
	std::stringstream str;
	str << intValue;
	strcpy (utf8String, str.str ().c_str ());
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIFontsController::stringToValue (UTF8StringPtr txt, float& result, void* userData)
{
	int32_t value = txt ? (int32_t)strtol (txt, 0, 10) : 0;
	result = (float)value;
	return true;
}


} // namespace

#endif // VSTGUI_LIVE_EDITING
