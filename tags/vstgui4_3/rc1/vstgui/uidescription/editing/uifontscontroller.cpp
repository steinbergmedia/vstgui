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

#include "uifontscontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uieditcontroller.h"
#include "uisearchtextfield.h"
#include "uibasedatasource.h"
#include "../../lib/controls/ccolorchooser.h"
#include "../../lib/controls/coptionmenu.h"
#include "../../lib/platform/iplatformfont.h"
#include <sstream>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIFontsDataSource : public UIBaseDataSource
{
public:
	UIFontsDataSource (UIDescription* description, IActionPerformer* actionPerformer, IGenericStringListDataBrowserSourceSelectionChanged* delegate);
	
protected:
	void getNames (std::list<const std::string*>& names) VSTGUI_OVERRIDE_VMETHOD;
	bool addItem (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD;
	bool removeItem (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD;
	bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) VSTGUI_OVERRIDE_VMETHOD;
	UTF8StringPtr getDefaultsName () VSTGUI_OVERRIDE_VMETHOD { return "UIFontsDataSource"; }

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
, dataSource (0)
, fontMenu (0)
, altTextEdit (0)
, sizeTextEdit (0)
, boldControl (0)
, italicControl (0)
, strikethroughControl (0)
, underlineControl (0)
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
				#if VSTGUI_HAS_FUNCTIONAL
					sizeTextEdit->setValueToStringFunction (valueToString);
					sizeTextEdit->setStringToValueFunction (stringToValue);
				#else
					sizeTextEdit->setValueToStringProc (valueToString, sizeTextEdit);
					sizeTextEdit->setStringToValueProc (stringToValue);
				#endif
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
				actionPerformer->performFontChange (selectedFont.c_str (), font);
			}
			break;
		}
		case kFontAltTag:
		{
			actionPerformer->performAlternativeFontChange (selectedFont.c_str (), altTextEdit->getText ());
			break;
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIFontsController::dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source)
{
	selectedFont = selectedRow != CDataBrowser::kNoSelection ? dataSource->getStringList ()->at (static_cast<uint32_t> (selectedRow)).c_str () : "";
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
			fontMenu->setStyle (fontMenu->getStyle () & ~kNoTextStyle);
			fontMenu->setMouseEnabled (true);
		}
		if (sizeTextEdit)
		{
			sizeTextEdit->setMouseEnabled (true);
			std::stringstream str;
			str << font->getSize ();
			sizeTextEdit->setText (str.str ().c_str ());
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
			editDescription->getAlternativeFontNames (selectedFont.c_str (), alternativeFonts);
			altTextEdit->setText (alternativeFonts.c_str ());
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
			altTextEdit->setText (0);
		}
		if (sizeTextEdit)
		{
			sizeTextEdit->setMouseEnabled (false);
			sizeTextEdit->setText (0);
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
	std::strcpy (utf8String, str.str ().c_str ());
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIFontsController::stringToValue (UTF8StringPtr txt, float& result, CTextEdit::StringToValueUserData* userData)
{
	int32_t value = txt ? (int32_t)strtol (txt, 0, 10) : 0;
	result = (float)value;
	return true;
}


} // namespace

#endif // VSTGUI_LIVE_EDITING
