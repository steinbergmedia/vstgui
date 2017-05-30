// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uifontscontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uieditcontroller.h"
#include "uibasedatasource.h"
#include "../../lib/controls/ccolorchooser.h"
#include "../../lib/controls/coptionmenu.h"
#include "../../lib/controls/csearchtextedit.h"
#include "../../lib/platform/iplatformfont.h"
#include <sstream>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIFontsDataSource : public UIBaseDataSource
{
public:
	UIFontsDataSource (UIDescription* description, IActionPerformer* actionPerformer, IGenericStringListDataBrowserSourceSelectionChanged* delegate);
	
protected:
	void getNames (std::list<const std::string*>& names) override;
	bool addItem (UTF8StringPtr name) override;
	bool removeItem (UTF8StringPtr name) override;
	bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) override;
	UTF8StringPtr getDefaultsName () override { return "UIFontsDataSource"; }

	SharedPointer<CColorChooser> colorChooser;
};

//----------------------------------------------------------------------------------------------------
UIFontsDataSource::UIFontsDataSource (UIDescription* description, IActionPerformer* actionPerformer, IGenericStringListDataBrowserSourceSelectionChanged* delegate)
: UIBaseDataSource (description, actionPerformer, UIDescription::kMessageFontChanged, delegate)
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
	actionPerformer->performFontChange (name, kNormalFont);
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIFontsDataSource::removeItem (UTF8StringPtr name)
{
	actionPerformer->performFontChange (name, kNormalFont, true);
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIFontsDataSource::performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	actionPerformer->performFontNameChange (oldName, newName);
	return true;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIFontsController::UIFontsController (IController* baseController, UIDescription* description, IActionPerformer* actionPerformer)
: DelegationController (baseController)
, editDescription (description)
, actionPerformer (actionPerformer)
, dataSource (nullptr)
, fontMenu (nullptr)
, altTextEdit (nullptr)
, sizeTextEdit (nullptr)
, boldControl (nullptr)
, italicControl (nullptr)
, strikethroughControl (nullptr)
, underlineControl (nullptr)
{
	dataSource = new UIFontsDataSource (editDescription, actionPerformer, this);
	UIEditController::setupDataSource (dataSource);
}

//----------------------------------------------------------------------------------------------------
UIFontsController::~UIFontsController ()
{
	dataSource->forget ();
}

//----------------------------------------------------------------------------------------------------
CView* UIFontsController::createView (const UIAttributes& attributes, const IUIDescription* description)
{
	const std::string* name = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (name)
	{
		if (*name == "FontsBrowser")
		{
			CDataBrowser* dataBrowser = new CDataBrowser (CRect (0, 0, 0, 0), dataSource, CDataBrowser::kDrawRowLines|CScrollView::kHorizontalScrollbar | CScrollView::kVerticalScrollbar);
			return dataBrowser;
		}
	}
	return DelegationController::createView (attributes, description);
}

//----------------------------------------------------------------------------------------------------
CView* UIFontsController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	auto searchField = dynamic_cast<CSearchTextEdit*>(view);
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
					for (auto& name : fontFamilyNames)
					{
						fontMenu->addEntry (name.data ());
					}
				}
				fontMenu->setStyle (fontMenu->getStyle () | kNoTextStyle);
				fontMenu->setMouseEnabled (false);
				break;
			}
			case kFontAltTag:
			{
				altTextEdit = dynamic_cast<CTextEdit*>(control);
				control->setMouseEnabled (false);
				break;
			}
			case kFontSizeTag:
			{
				sizeTextEdit = dynamic_cast<CTextEdit*>(control);
				if (sizeTextEdit)
				{
					sizeTextEdit->setValueToStringFunction (valueToString);
					sizeTextEdit->setStringToValueFunction (stringToValue);
				}
				control->setMouseEnabled (false);
				break;
			}
			case kFontStyleBoldTag:
			{
				boldControl = control;
				control->setMouseEnabled (false);
				break;
			}
			case kFontStyleItalicTag:
			{
				italicControl = control;
				control->setMouseEnabled (false);
				break;
			}
			case kFontStyleStrikethroughTag:
			{
				strikethroughControl = control;
				control->setMouseEnabled (false);
				break;
			}
			case kFontStyleUnderlineTag:
			{
				underlineControl = control;
				control->setMouseEnabled (false);
				break;
			}
		}
	}
	return controller->verifyView (view, attributes, description);
}

//----------------------------------------------------------------------------------------------------
IControlListener* UIFontsController::getControlListener (UTF8StringPtr name)
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
		case kFontSizeTag:
		case kFontStyleBoldTag:
		case kFontStyleItalicTag:
		case kFontStyleStrikethroughTag:
		case kFontStyleUnderlineTag:
		{
			if (fontMenu == nullptr || sizeTextEdit == nullptr || selectedFont.empty ())
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
				auto font = makeOwned<CFontDesc> (menuItem->getTitle (), sizeTextEdit->getValue (), style);
				actionPerformer->performFontChange (selectedFont.data (), font);
			}
			break;
		}
		case kFontAltTag:
		{
			actionPerformer->performAlternativeFontChange (selectedFont.data (), altTextEdit->getText ());
			break;
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIFontsController::dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source)
{
	selectedFont = selectedRow != CDataBrowser::kNoSelection ? dataSource->getStringList ()->at (static_cast<uint32_t> (selectedRow)).data () : "";
	CFontRef font = editDescription->getFont (selectedFont.data ());
	if (font)
	{
		if (fontMenu && !font->getName ().empty ())
		{
			auto fontName = font->getName ();
			CMenuItemList* items = fontMenu->getItems ();
			int32_t index = 0;
			for (auto& item : *items)
			{
				if (fontName == item->getTitle ())
				{
					fontMenu->setValue ((float)index);
					break;
				}
				index++;
			}
			fontMenu->setStyle (fontMenu->getStyle () & ~kNoTextStyle);
			fontMenu->setMouseEnabled (true);
		}
		if (sizeTextEdit)
		{
			sizeTextEdit->setMouseEnabled (true);
			std::stringstream str;
			str << font->getSize ();
			sizeTextEdit->setText (str.str ().data ());
		}
		if (boldControl)
		{
			boldControl->setValue (font->getStyle () & kBoldFace ? true : false);
			boldControl->invalid ();
			boldControl->setMouseEnabled (true);
		}
		if (italicControl)
		{
			italicControl->setValue (font->getStyle () & kItalicFace ? true : false);
			italicControl->invalid ();
			italicControl->setMouseEnabled (true);
		}
		if (underlineControl)
		{
			underlineControl->setValue (font->getStyle () & kUnderlineFace ? true : false);
			underlineControl->invalid ();
			underlineControl->setMouseEnabled (true);
		}
		if (strikethroughControl)
		{
			strikethroughControl->setValue (font->getStyle () & kStrikethroughFace ? true : false);
			strikethroughControl->invalid ();
			strikethroughControl->setMouseEnabled (true);
		}
		if (altTextEdit)
		{
			std::string alternativeFonts;
			editDescription->getAlternativeFontNames (selectedFont.data (), alternativeFonts);
			altTextEdit->setText (alternativeFonts.data ());
			altTextEdit->setMouseEnabled (true);
		}
	}
	else
	{
		if (fontMenu)
		{
			fontMenu->setStyle (fontMenu->getStyle () | kNoTextStyle);
			fontMenu->setMouseEnabled (false);
		}
		if (boldControl)
			boldControl->setMouseEnabled (false);
		if (italicControl)
			italicControl->setMouseEnabled (false);
		if (underlineControl)
			underlineControl->setMouseEnabled (false);
		if (strikethroughControl)
			strikethroughControl->setMouseEnabled (false);
		if (altTextEdit)
		{
			altTextEdit->setMouseEnabled (false);
			altTextEdit->setText (nullptr);
		}
		if (sizeTextEdit)
		{
			sizeTextEdit->setMouseEnabled (false);
			sizeTextEdit->setText (nullptr);
		}
	}
}

//----------------------------------------------------------------------------------------------------
bool UIFontsController::valueToString (float value, char utf8String[256], CParamDisplay::ValueToStringUserData* userData)
{
	CTextEdit* edit = static_cast<CTextEdit*> (userData);
	if (edit && edit->getMouseEnabled () == false)
		return true;
		
	int32_t intValue = (int32_t)value;
	std::stringstream str;
	str << intValue;
	std::strcpy (utf8String, str.str ().data ());
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIFontsController::stringToValue (UTF8StringPtr txt, float& result, CTextEdit::StringToValueUserData* userData)
{
	int32_t value = txt ? (int32_t)strtol (txt, nullptr, 10) : 0;
	result = (float)value;
	return true;
}


} // namespace

#endif // VSTGUI_LIVE_EDITING
