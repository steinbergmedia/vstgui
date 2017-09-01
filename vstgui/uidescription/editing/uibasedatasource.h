// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uibasedatasource__
#define __uibasedatasource__

#include "../iuidescription.h"

#if VSTGUI_LIVE_EDITING

#include "../uiattributes.h"
#include "../../lib/cdatabrowser.h"
#include "../../lib/controls/csearchtextedit.h"
#include "../../lib/controls/cscrollbar.h"
#include <sstream>
#include <algorithm>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIBaseDataSource : public GenericStringListDataBrowserSource, public IControlListener
{
public:
	using StringVector = GenericStringListDataBrowserSource::StringVector;

	UIBaseDataSource (UIDescription* description, IActionPerformer* actionPerformer, IdStringPtr descriptionMessage, IGenericStringListDataBrowserSourceSelectionChanged* delegate = nullptr)
	: GenericStringListDataBrowserSource (0, delegate) , description (description), actionPerformer (actionPerformer), descriptionMessage (descriptionMessage)
	{
		description->addDependency (this);
		textInset.x = 4;
	}
	
	~UIBaseDataSource () override
	{
		description->removeDependency (this);
	}

	void setSearchFieldControl (CSearchTextEdit* searchControl)
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
				addItem (newName.data ());
				int32_t row = selectName (newName.data ());
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
				removeItem (names.at (static_cast<uint32_t> (selectedRow)).data ());
				dbSelectionChanged (dataBrowser);
				dataBrowser->setSelectedRow (selectedRow);
				return true;
			}
		}
		return false;
	}

	virtual void setFilter (const UTF8String& filter)
	{
		if (filterString != filter)
		{
			filterString = filter;
			int32_t selectedRow = dataBrowser ? dataBrowser->getSelectedRow () : CDataBrowser::kNoSelection;
			std::string selectedName;
			if (selectedRow != CDataBrowser::kNoSelection)
				selectedName = names.at (static_cast<uint32_t> (selectedRow));
			update ();
			if (selectedRow != CDataBrowser::kNoSelection)
				selectName (selectedName.data ());
		}
	}

	virtual int32_t selectName (UTF8StringPtr name)
	{
		int32_t index = 0;
		for (auto& it : names)
		{
			if (it == name)
			{
				dataBrowser->setSelectedRow (index, true);
				if (delegate)
					delegate->dbSelectionChanged (index, this);
				return index;
			}
			++index;
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

		std::string filter = filterString.getString ();
		std::transform (filter.begin (), filter.end (), filter.begin (), ::tolower);

		for (auto& name : tmpNames)
		{
			if (!filter.empty ())
			{
				std::string tmp (*name);
				std::transform (tmp.begin (), tmp.end (), tmp.begin (), ::tolower);
				if (tmp.find (filter) == std::string::npos)
					continue;
			}
			if (name->find ("~ ") == 0)
				continue; // don't show static items
			names.emplace_back (UTF8String (*name));
		}
		bool vsbIsVisible = false;
		if (dataBrowser)
		{
			if (auto vsb = dataBrowser->getVerticalScrollbar ())
				vsbIsVisible = vsb->isVisible ();
		}
		setStringList (&names);
		if (dataBrowser)
		{
			if (auto vsb = dataBrowser->getVerticalScrollbar ())
			{
				if (vsb->isVisible() != vsbIsVisible)
					dataBrowser->recalculateLayout ();
			}
		}
	}
	
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override
	{
		if (message == descriptionMessage)
		{
			int32_t selectedRow = dataBrowser ? dataBrowser->getSelectedRow () : CDataBrowser::kNoSelection;
			std::string selectedName;
			if (selectedRow != CDataBrowser::kNoSelection)
				selectedName = names.at (static_cast<uint32_t> (selectedRow));
			update ();
			if (selectedRow != CDataBrowser::kNoSelection)
				selectName (selectedName.data ());
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
				attributes->setAttribute ("FilterString", filterString.getString ());
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
					setFilter (str->data ());
				if (dataBrowser)
				{
					int32_t selectedRow;
					if (attributes->getIntegerAttribute("SelectedRow", selectedRow))
						dataBrowser->setSelectedRow (selectedRow, true);
				}
			}
		}
	}

	void dbAttached (CDataBrowser* browser) override
	{
		GenericStringListDataBrowserSource::dbAttached (browser);
		update ();
		loadDefaults ();
		if (searchField)
			searchField->setText (filterString);
	}

	void dbRemoved (CDataBrowser* browser) override
	{
		saveDefaults ();
		GenericStringListDataBrowserSource::dbRemoved (browser);
	}

	void valueChanged (CControl* control) override
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
		for (auto& it : names)
		{
			if (it == str.str ())
				return createUniqueName (name, count+1);
		}
		name = str.str ();
		return true;
	}

	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) override
	{
		if (buttons.isLeftButton () && buttons.isDoubleClick ())
		{
			browser->beginTextEdit (CDataBrowser::Cell (row, column), names.at (static_cast<uint32_t> (row)).data ());
		}
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}

	void dbCellTextChanged (int32_t _row, int32_t column, UTF8StringPtr newText, CDataBrowser* browser) override
	{
		textEditControl = nullptr;
		if (_row < 0 || _row >= static_cast<int32_t> (names.size ()))
			return;
		auto row = static_cast<size_t> (_row);
		for (auto& name : names)
		{
			if (name == newText)
				return;
		}
		auto& currentName = names.at (row);
		if (performNameChange (currentName.data (), newText))
		{
			if (selectName (newText) == -1 && row < names.size ())
				selectName (names.at (row).data ());
		}
	}

	void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* control, CDataBrowser* browser) override
	{
		textEditControl = control;
		textEditControl->setBackColor (kWhiteCColor);
		textEditControl->setFontColor (fontColor);
		textEditControl->setFont (drawFont);
		textEditControl->setHoriAlign (textAlignment);
		textEditControl->setTextInset (textInset);
	}

	SharedPointer<UIDescription> description;
	SharedPointer<CSearchTextEdit> searchField;
	SharedPointer<CTextEdit> textEditControl;
	IActionPerformer* actionPerformer;
	IdStringPtr descriptionMessage;

	StringVector names;
	UTF8String filterString;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uibasedatasource__
