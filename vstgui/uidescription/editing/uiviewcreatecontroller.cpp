// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uiviewcreatecontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uieditcontroller.h"
#include "uibasedatasource.h"
#include "../uiviewfactory.h"
#include "../../lib/cdropsource.h"
#include "../../lib/controls/coptionmenu.h"
#include "../../lib/controls/csearchtextedit.h"
#include "uiselection.h"
#include "../detail/uiviewcreatorattributes.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIViewCreatorDataSource : public UIBaseDataSource
{
public:
	UIViewCreatorDataSource (const UIViewFactory* factory, UIDescription* description);

	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) override;
	CMouseEventResult dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) override;

	void getNames (std::list<const std::string*>& names) override;
	bool addItem (UTF8StringPtr name) override { return false; }
	bool removeItem (UTF8StringPtr name) override { return false; }
	bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) override { return false; }
	UTF8StringPtr getDefaultsName () override { return "UIViewCreatorDataSource"; }

	void addViewToCurrentEditView (int32_t row);
protected:
	SharedPointer<UISelection> createSelection (int32_t row);
	UIViewFactory::ViewAndDisplayNameList viewAndDisplayNameList;
	const UIViewFactory* factory;
};

//----------------------------------------------------------------------------------------------------
UIViewCreatorController::UIViewCreatorController (IController* baseController, UIDescription* description)
: DelegationController (baseController)
, description (description)
{
}

//----------------------------------------------------------------------------------------------------
UIViewCreatorController::~UIViewCreatorController ()
{
	if (dataSource)
		dataSource->forget ();
}

//----------------------------------------------------------------------------------------------------
CView* UIViewCreatorController::createView (const UIAttributes& attributes, const IUIDescription* _description)
{
	const std::string* name = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (name)
	{
		if (*name == "ViewDataBrowser")
		{
			vstgui_assert (dataBrowser == nullptr);
			const auto factory = dynamic_cast<const UIViewFactory*> (description->getViewFactory ());
			dataSource = new UIViewCreatorDataSource (factory, description);
			UIEditController::setupDataSource (dataSource);
			dataBrowser = new CDataBrowser (CRect (0, 0, 0, 0), dataSource, CDataBrowser::kDrawRowLines|CScrollView::kHorizontalScrollbar | CScrollView::kVerticalScrollbar);
			return dataBrowser;
		}
	}
	return DelegationController::createView (attributes, _description);
}

//----------------------------------------------------------------------------------------------------
CView* UIViewCreatorController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	auto searchField = dynamic_cast<CSearchTextEdit*>(view);
	if (searchField && searchField->getTag () == kSearchFieldTag)
	{
		dataSource->setSearchFieldControl (searchField);
	}
	return DelegationController::verifyView (view, attributes, description);
}

//----------------------------------------------------------------------------------------------------
IControlListener* UIViewCreatorController::getControlListener (UTF8StringPtr name)
{
	if (std::strcmp (name, "viewcreator.search") == 0)
		return dataSource;
	return this;
}

//----------------------------------------------------------------------------------------------------
void UIViewCreatorController::valueChanged (CControl* control)
{
}

//----------------------------------------------------------------------------------------------------
void UIViewCreatorController::appendContextMenuItems (COptionMenu& contextMenu, const CPoint& where)
{
	auto cell = dataBrowser->getCellAt (where);
	if (!cell.isValid ())
		return;
	const auto& viewName = dataSource->getStringList ()->at (static_cast<uint32_t> (cell.row));
	UTF8String menuEntryName = "Insert '" + viewName + "'";
	auto item = new CCommandMenuItem (menuEntryName);
	item->setActions ([&, cell] (CCommandMenuItem* item) {
		dataSource->addViewToCurrentEditView (cell.row);
	});
	contextMenu.addEntry (item);
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIViewCreatorDataSource::UIViewCreatorDataSource (const UIViewFactory* factory, UIDescription* description)
: UIBaseDataSource (description, nullptr, nullptr)
, factory (factory)
{
}

//----------------------------------------------------------------------------------------------------
void UIViewCreatorDataSource::getNames (std::list<const std::string*>& names)
{
	viewAndDisplayNameList = factory->collectRegisteredViewAndDisplayNames ();
	for (const auto& e : viewAndDisplayNameList)
	{
		names.emplace_back (&e.second);
	}
	names.sort ([] (const auto& lhs, const auto& rhs) { return *lhs < *rhs; });

}

//----------------------------------------------------------------------------------------------------
void UIViewCreatorDataSource::addViewToCurrentEditView (int32_t row)
{
	UIViewCreatorController* controller = dynamic_cast<UIViewCreatorController*> (getViewController (dataBrowser, true));
	if (controller)
	{
		if (UIEditController* editController = dynamic_cast<UIEditController*> (controller->getBaseController ()))
		{
			SharedPointer<UISelection> selection = createSelection (row);
			editController->addSelectionToCurrentView (selection);
		}
	}
}

//----------------------------------------------------------------------------------------------------
SharedPointer<UISelection> UIViewCreatorDataSource::createSelection (int32_t row)
{
	SharedPointer<UISelection> selection;
	auto viewDisplayName = getStringList ()->at (static_cast<uint32_t> (row)).getString ();
	auto it = std::find_if (viewAndDisplayNameList.begin (), viewAndDisplayNameList.end (), [&] (const auto& entry) {
		return entry.second == viewDisplayName;
	});
	if (it == viewAndDisplayNameList.end ())
		return nullptr;
	UIAttributes viewAttr;
	viewAttr.setAttribute (UIViewCreator::kAttrClass, *it->first);
	CView* view = factory->createView (viewAttr, description);
	if (view)
	{
		if (view->getViewSize ().isEmpty ())
		{
			CRect size (CPoint (0, 0), CPoint (20, 20));
			view->setViewSize (size);
			view->setMouseableArea (size);
		}
		selection = makeOwned<UISelection> ();
		selection->add (view);
		view->forget ();
	}
	return selection;
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIViewCreatorDataSource::dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
{
	if (buttons.isLeftButton ())
	{
		if (!buttons.isDoubleClick ())
			return kMouseEventHandled;
		addViewToCurrentEditView (row);
	}
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIViewCreatorDataSource::dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
{
	if (buttons.isLeftButton () && row != -1 && column != -1)
	{
		auto row = dataBrowser->getSelection().front ();
		SharedPointer<UISelection> selection = createSelection (row);
		CMemoryStream stream (1024, 1024, false);
		if (selection->store (stream, description))
		{
			stream.end ();
			auto dropSource = CDropSource::create (stream.getBuffer (), static_cast<uint32_t> (stream.tell ()), CDropSource::kText);
			browser->doDrag (dropSource);
		}
		return kMouseMoveEventHandledButDontNeedMoreEvents;
	}
	return kMouseEventNotHandled;
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
