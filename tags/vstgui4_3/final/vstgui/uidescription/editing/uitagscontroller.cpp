#include "uitagscontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uieditcontroller.h"
#include "uisearchtextfield.h"
#include "uibasedatasource.h"
#include <sstream>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UITagsDataSource : public UIBaseDataSource
{
public:
	UITagsDataSource (UIDescription* description, IActionPerformer* actionPerformer);
	~UITagsDataSource ();

protected:
	void getNames (std::list<const std::string*>& names) VSTGUI_OVERRIDE_VMETHOD;
	bool addItem (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD;
	bool removeItem (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD;
	bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) VSTGUI_OVERRIDE_VMETHOD;
	UTF8StringPtr getDefaultsName () VSTGUI_OVERRIDE_VMETHOD;

	void update () VSTGUI_OVERRIDE_VMETHOD;
	
	int32_t dbGetNumColumns (CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD { return 2; }
	CCoord dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;
	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;
	void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;
	void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* textEditControl, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;

	StringVector tags;
};

//----------------------------------------------------------------------------------------------------
UITagsDataSource::UITagsDataSource (UIDescription* description, IActionPerformer* actionPerformer)
: UIBaseDataSource (description, actionPerformer, UIDescription::kMessageTagChanged)
{
}

//----------------------------------------------------------------------------------------------------
UITagsDataSource::~UITagsDataSource ()
{
}

//----------------------------------------------------------------------------------------------------
void UITagsDataSource::getNames (std::list<const std::string*>& names)
{
	description->collectControlTagNames (names);
}

//----------------------------------------------------------------------------------------------------
bool UITagsDataSource::addItem (UTF8StringPtr name)
{
	actionPerformer->performTagChange (name, "-2");
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UITagsDataSource::removeItem (UTF8StringPtr name)
{
	actionPerformer->performTagChange (name, 0, true);
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UITagsDataSource::performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	actionPerformer->performTagNameChange (oldName, newName);
	return true;
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr UITagsDataSource::getDefaultsName ()
{
	return "UITagsDataSource";
}

//----------------------------------------------------------------------------------------------------
void UITagsDataSource::update ()
{
	UIBaseDataSource::update ();
	tags.clear ();
	for (StringVector::const_iterator it = names.begin (); it != names.end (); it++)
	{
		std::string tagString;
		description->getControlTagString ((*it).c_str (), tagString);
		tags.push_back (tagString);
	}
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UITagsDataSource::dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
{
	if (buttons.isLeftButton () && buttons.isDoubleClick ())
	{
		UTF8StringPtr value = column == 0 ? names.at (static_cast<uint32_t> (row)).c_str () : tags.at (static_cast<uint32_t> (row)).c_str ();
		browser->beginTextEdit (CDataBrowser::Cell (row, column), value);
	}
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//----------------------------------------------------------------------------------------------------
void UITagsDataSource::dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText, CDataBrowser* browser)
{
	if (column == 0)
	{
		UIBaseDataSource::dbCellTextChanged (row, column, newText, browser);
	}
	else
	{
		if (tags.at (static_cast<uint32_t> (row)) != newText)
		{
			actionPerformer->performTagChange (names.at (static_cast<uint32_t> (row)).c_str (), newText);
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UITagsDataSource::dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* textEditControl, CDataBrowser* browser)
{
	UIBaseDataSource::dbCellSetupTextEdit (row, column, textEditControl, browser);
	if (column == 1)
		textEditControl->setHoriAlign (kRightText);
}

//----------------------------------------------------------------------------------------------------
CCoord UITagsDataSource::dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser)
{
	CCoord width = browser->getWidth () - ((browser->getActiveScrollbars () & CScrollView::kVerticalScrollbar) == 0 ? 0 : browser->getScrollbarWidth () + ((browser->getStyle () & CScrollView::kDontDrawFrame) ? 0 : 2));
	if (browser->getStyle () & CDataBrowser::kDrawColumnLines)
		width -= 1;
	if (index == 0)
		return width * 0.5;
	return std::floor (width - width * 0.5 + 0.5);
}

//----------------------------------------------------------------------------------------------------
void UITagsDataSource::dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser)
{
	if (column == 1)
	{
		stringList = &tags;
		column = 0;
		textAlignment = kRightText;
	}
	UIBaseDataSource::dbDrawCell (context, size, row, column, flags, browser);
	stringList = &names;
	textAlignment = kLeftText;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UITagsController::UITagsController (IController* baseController, UIDescription* description, IActionPerformer* actionPerformer)
: DelegationController (baseController)
, editDescription (description)
, actionPerformer (actionPerformer)
, dataSource (0)
{
}

//----------------------------------------------------------------------------------------------------
UITagsController::~UITagsController ()
{
	if (dataSource)
		dataSource->forget ();
}

//----------------------------------------------------------------------------------------------------
CView* UITagsController::createView (const UIAttributes& attributes, const IUIDescription* description)
{
	const std::string* name = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (name)
	{
		if (*name == "TagsBrowser")
		{
			dataSource = new UITagsDataSource (editDescription, actionPerformer);
			UIEditController::setupDataSource (dataSource);
			return new CDataBrowser (CRect (0, 0, 0, 0), dataSource, CDataBrowser::kDrawColumnLines|CDataBrowser::kDrawRowLines|CScrollView::kHorizontalScrollbar | CScrollView::kVerticalScrollbar);
		}
	}
	return DelegationController::createView (attributes, description);
}

//----------------------------------------------------------------------------------------------------
CView* UITagsController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	UISearchTextField* searchField = dynamic_cast<UISearchTextField*>(view);
	if (searchField && searchField->getTag () == kSearchTag)
	{
		dataSource->setSearchFieldControl (searchField);
		return searchField;
	}
	return controller->verifyView (view, attributes, description);
}

//----------------------------------------------------------------------------------------------------
IControlListener* UITagsController::getControlListener (UTF8StringPtr name)
{
	return this;
}

//----------------------------------------------------------------------------------------------------
void UITagsController::valueChanged (CControl* pControl)
{
	switch (pControl->getTag ())
	{
		case kAddTag:
		{
			if (dataSource && pControl->getValue () == pControl->getMax ())
			{
				dataSource->add ();
			}
			break;
		}
		case kRemoveTag:
		{
			if (dataSource && pControl->getValue () == pControl->getMax ())
			{
				dataSource->remove ();
			}
			break;
		}
	}
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
