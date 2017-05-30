// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uitagscontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uieditcontroller.h"
#include "uibasedatasource.h"
#include "../../lib/controls/csearchtextedit.h"
#include <sstream>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UITagsDataSource : public UIBaseDataSource
{
public:
	UITagsDataSource (UIDescription* description, IActionPerformer* actionPerformer);
	~UITagsDataSource () override = default;

protected:
	void getNames (std::list<const std::string*>& names) override;
	bool addItem (UTF8StringPtr name) override;
	bool removeItem (UTF8StringPtr name) override;
	bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) override;
	UTF8StringPtr getDefaultsName () override;

	void update () override;
	
	int32_t dbGetNumColumns (CDataBrowser* browser) override { return 2; }
	CCoord dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser) override;
	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser) override;
	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) override;
	void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText, CDataBrowser* browser) override;
	void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* textEditControl, CDataBrowser* browser) override;

	StringVector tags;
};

//----------------------------------------------------------------------------------------------------
UITagsDataSource::UITagsDataSource (UIDescription* description, IActionPerformer* actionPerformer)
: UIBaseDataSource (description, actionPerformer, UIDescription::kMessageTagChanged)
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
	actionPerformer->performTagChange (name, nullptr, true);
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
	for (auto& name : names)
	{
		std::string tagString;
		description->getControlTagString (name.data (), tagString);
		tags.emplace_back (std::move (tagString));
	}
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UITagsDataSource::dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
{
	if (buttons.isLeftButton () && buttons.isDoubleClick ())
	{
		UTF8StringPtr value = column == 0 ? names.at (static_cast<uint32_t> (row)).data () : tags.at (static_cast<uint32_t> (row)).data ();
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
			actionPerformer->performTagChange (names.at (static_cast<uint32_t> (row)).data (), newText);
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
, dataSource (nullptr)
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
	auto searchField = dynamic_cast<CSearchTextEdit*>(view);
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
