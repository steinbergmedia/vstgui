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

#ifndef __uibasedatasource__
#define __uibasedatasource__

#include "../iuidescription.h"

#if VSTGUI_LIVE_EDITING

#include "../uiattributes.h"
#include "../../lib/cdatabrowser.h"
#include "uisearchtextfield.h"
#include <sstream>
#include <algorithm>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIBaseDataSource : public GenericStringListDataBrowserSource, public IControlListener
{
public:
	typedef std::vector<std::string> StringVector;

	UIBaseDataSource (UIDescription* description, IActionPerformer* actionPerformer, IdStringPtr descriptionMessage, IGenericStringListDataBrowserSourceSelectionChanged* delegate = 0)
	: GenericStringListDataBrowserSource (0, delegate) , description (description), actionPerformer (actionPerformer), descriptionMessage (descriptionMessage)
	{
		description->addDependency (this);
		textInset.x = 4;
	}
	
	~UIBaseDataSource ()
	{
		description->removeDependency (this);
	}

	void setSearchFieldControl (UISearchTextField* searchControl)
	{
		searchField = searchControl;
		searchField->setListener (this);
	}
	
	virtual bool add ()
	{
		if (dataBrowser && actionPerformer)
		{
			std::string newName (filterString.empty () ? "New" : filterString);
			if (createUniqueName (newName))
			{
				addItem (newName.c_str ());
				int32_t row = selectName (newName.c_str ());
				if (row != -1)
				{
					dbOnMouseDown (CPoint (0, 0), CButtonState (kLButton|kDoubleClick), row, 0, dataBrowser);
					return true;
				}
			}
		}
		return false;
	}

	virtual bool remove ()
	{
		if (dataBrowser && actionPerformer)
		{
			int32_t selectedRow = dataBrowser->getSelectedRow ();
			if (selectedRow != CDataBrowser::kNoSelection)
			{
				removeItem (names.at (static_cast<uint32_t> (selectedRow)).c_str ());
				dbSelectionChanged (dataBrowser);
				dataBrowser->setSelectedRow (selectedRow);
				return true;
			}
		}
		return false;
	}

	virtual void setFilter (UTF8StringPtr filter)
	{
		if (filterString != filter)
		{
			filterString = filter ? filter : "";
			int32_t selectedRow = dataBrowser ? dataBrowser->getSelectedRow () : CDataBrowser::kNoSelection;
			std::string selectedName;
			if (selectedRow != CDataBrowser::kNoSelection)
				selectedName = names.at (static_cast<uint32_t> (selectedRow));
			update ();
			if (selectedRow != CDataBrowser::kNoSelection)
				selectName (selectedName.c_str ());
		}
	}

	virtual int32_t selectName (UTF8StringPtr name)
	{
		int32_t index = 0;
		for (StringVector::const_iterator it = names.begin (); it != names.end (); it++, index++)
		{
			if (*it == name)
			{
				dataBrowser->setSelectedRow (index, true);
				if (delegate)
					delegate->dbSelectionChanged (index, this);
				return index;
			}
		}
		return -1;
	}
protected:
	virtual void getNames (std::list<const std::string*>& names) = 0;
	virtual bool addItem (UTF8StringPtr name) = 0;
	virtual bool removeItem (UTF8StringPtr name) = 0;
	virtual bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) = 0;
	virtual UTF8StringPtr getDefaultsName () = 0;

	virtual void update ()
	{
		if (textEditControl)
			textEditControl->looseFocus ();
		names.clear ();
		std::list<const std::string*> tmpNames;
		getNames (tmpNames);

		std::string filter = filterString;
		std::transform (filter.begin (), filter.end (), filter.begin (), ::tolower);

		for (std::list<const std::string*>::const_iterator it = tmpNames.begin (); it != tmpNames.end (); it++)
		{
			if (!filter.empty ())
			{
				std::string tmp (*(*it));
				std::transform (tmp.begin (), tmp.end (), tmp.begin (), ::tolower);
				if (tmp.find (filter) == std::string::npos)
					continue;
			}
			if ((*it)->find ("~ ") == 0)
				continue; // don't show static items
			names.push_back (*(*it));
		}
		setStringList (&names);
	}
	
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD
	{
		if (message == descriptionMessage)
		{
			int32_t selectedRow = dataBrowser ? dataBrowser->getSelectedRow () : CDataBrowser::kNoSelection;
			std::string selectedName;
			if (selectedRow != CDataBrowser::kNoSelection)
				selectedName = names.at (static_cast<uint32_t> (selectedRow));
			update ();
			if (selectedRow != CDataBrowser::kNoSelection)
				selectName (selectedName.c_str ());
			return kMessageNotified;
		}
		else if (message == UIDescription::kMessageBeforeSave)
		{
			saveDefaults ();
			return kMessageNotified;
		}
		return kMessageUnknown;
	}

	virtual void saveDefaults ()
	{
		UTF8StringPtr name = getDefaultsName ();
		if (name)
		{
			UIAttributes* attributes = description->getCustomAttributes (name, true);
			if (attributes)
			{
				attributes->setAttribute ("FilterString", filterString);
				if (dataBrowser)
				{
					int32_t selectedRow = dataBrowser->getSelectedRow ();
					attributes->setIntegerAttribute ("SelectedRow", selectedRow);
				}
			}
		}
	}

	virtual void loadDefaults ()
	{
		UTF8StringPtr name = getDefaultsName ();
		if (name)
		{
			UIAttributes* attributes = description->getCustomAttributes (name, true);
			if (attributes)
			{
				const std::string* str = attributes->getAttributeValue ("FilterString");
				if (str)
					setFilter (str->c_str ());
				if (dataBrowser)
				{
					int32_t selectedRow;
					if (attributes->getIntegerAttribute("SelectedRow", selectedRow))
						dataBrowser->setSelectedRow (selectedRow, true);
				}
			}
		}
	}

	void dbAttached (CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD
	{
		GenericStringListDataBrowserSource::dbAttached (browser);
		update ();
		loadDefaults ();
		if (searchField)
			searchField->setText (filterString.c_str ());
	}

	void dbRemoved (CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD
	{
		saveDefaults ();
		GenericStringListDataBrowserSource::dbRemoved (browser);
	}

	void valueChanged (CControl* control) VSTGUI_OVERRIDE_VMETHOD
	{
		CTextEdit* edit = dynamic_cast<CTextEdit*>(control);
		if (edit)
			setFilter (edit->getText ());
	}

	bool createUniqueName (std::string& name, int32_t count = 0)
	{
		std::stringstream str;
		str << name;
		if (count)
		{
			str << ' ';
			str << count;
		}
		for (StringVector::const_iterator it = names.begin (); it != names.end (); it++)
		{
			if (*it == str.str ())
				return createUniqueName (name, count+1);
		}
		name = str.str ();
		return true;
	}

	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD
	{
		if (buttons.isLeftButton () && buttons.isDoubleClick ())
		{
			browser->beginTextEdit (CDataBrowser::Cell (row, column), names.at (static_cast<uint32_t> (row)).c_str ());
		}
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}

	void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD
	{
		if (row < (int32_t)names.size () && names.at (static_cast<uint32_t> (row)) != newText)
		{
			if (performNameChange (names.at (static_cast<uint32_t> (row)).c_str (), newText))
			{
				if (selectName (newText) == -1 && row < (int32_t)names.size ())
					selectName (names.at (static_cast<uint32_t> (row)).c_str ());
			}
		}
		textEditControl = 0;
	}

	void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* control, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD
	{
		textEditControl = control;
		textEditControl->setBackColor (kWhiteCColor);
		textEditControl->setFontColor (fontColor);
		textEditControl->setFont (drawFont);
		textEditControl->setHoriAlign (textAlignment);
		textEditControl->setTextInset (textInset);
	}

	SharedPointer<UIDescription> description;
	SharedPointer<UISearchTextField> searchField;
	SharedPointer<CTextEdit> textEditControl;
	IActionPerformer* actionPerformer;
	IdStringPtr descriptionMessage;

	StringVector names;
	std::string filterString;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uibasedatasource__
