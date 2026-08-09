// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uiviewcreatecontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uieditcontroller.h"
#include "uibasedatasource.h"
#include "../cstream.h"
#include "../../lib/cdropsource.h"
#include "../../lib/dragging.h"
#include "../../lib/controls/coptionmenu.h"
#include "../../lib/controls/csearchtextedit.h"
#include "../detail/uiviewcreatorattributes.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIViewCreatorDataSource : public UIBaseDataSource
{
public:
	UIViewCreatorDataSource (const IViewFactory& factory, const SPtr<UIDescription>& description);

	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row,
									 int32_t column, CDataBrowser& browser) override;
	CMouseEventResult dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row,
									  int32_t column, CDataBrowser& browser) override;

	void getNames (std::list<const std::string*>& names) override;
	bool addItem (UTF8StringPtr name) override { return false; }
	bool removeItem (UTF8StringPtr name) override { return false; }
	bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) override { return false; }
	UTF8StringPtr getDefaultsName () override { return "UIViewCreatorDataSource"; }

	void addViewToCurrentEditView (int32_t row);
protected:
	SPtr<UISelection> createSelection (int32_t row);
	IViewFactoryEditingSupport::ViewAndDisplayNameList viewAndDisplayNameList;
	const IViewFactory& factory;
	DragStartMouseObserver dragStartMouseObserver;
};

//----------------------------------------------------------------------------------------------------
UIViewCreatorController::UIViewCreatorController (const SPtr<IController>& baseController,
												  const SPtr<UIDescription>& description)
: DelegationController (baseController), description (description)
{
}

//----------------------------------------------------------------------------------------------------
UIViewCreatorController::~UIViewCreatorController () {}

//----------------------------------------------------------------------------------------------------
SPtr<CView> UIViewCreatorController::createView (const UIAttributes& attributes,
												 const IUIDescription& _description)
{
	const std::string* name = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (name)
	{
		if (*name == "ViewDataBrowser")
		{
			vstgui_assert (dataBrowser == nullptr);
			dataSource =
				makeShared<UIViewCreatorDataSource> (description->getViewFactory (), description);
			UIEditController::setupDataSource (dataSource);
			dataBrowser = makeShared<CDataBrowser> (CRect (0, 0, 0, 0), dataSource,
													CDataBrowser::kDrawRowLines |
														CScrollView::kHorizontalScrollbar |
														CScrollView::kVerticalScrollbar);
			return dataBrowser;
		}
	}
	return DelegationController::createView (attributes, _description);
}

//----------------------------------------------------------------------------------------------------
SPtr<CView> UIViewCreatorController::verifyView (const SPtr<CView>& view,
												 const UIAttributes& attributes,
												 const IUIDescription& desc)
{
	auto searchField = view.cast<CSearchTextEdit> ();
	if (dataSource && searchField && searchField->getTag () == kSearchFieldTag)
	{
		dataSource->setSearchFieldControl (searchField);
	}
	return DelegationController::verifyView (view, attributes, desc);
}

//----------------------------------------------------------------------------------------------------
IControlListener* UIViewCreatorController::getControlListener (UTF8StringPtr name)
{
	if (std::strcmp (name, "viewcreator.search") == 0)
		return dataSource.get ();
	return this;
}

//----------------------------------------------------------------------------------------------------
void UIViewCreatorController::valueChanged (CControl& control) {}

//----------------------------------------------------------------------------------------------------
void UIViewCreatorController::appendContextMenuItems (COptionMenu& contextMenu, const CPoint& where)
{
	auto cell = dataBrowser->getCellAt (where);
	if (!cell.isValid ())
		return;
	const auto& viewName = dataSource->getStringList ()->at (static_cast<uint32_t> (cell.row));
	UTF8String menuEntryName = "Insert '" + viewName + "'";
	auto item = makeShared<CCommandMenuItem> (menuEntryName);
	item->setActions ([&, cell] (auto&& item) { dataSource->addViewToCurrentEditView (cell.row); });
	contextMenu.addEntry (item);
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIViewCreatorDataSource::UIViewCreatorDataSource (const IViewFactory& factory,
												  const SPtr<UIDescription>& description)
: UIBaseDataSource (description, {}, nullptr), factory (factory)
{
}

//----------------------------------------------------------------------------------------------------
void UIViewCreatorDataSource::getNames (std::list<const std::string*>& names)
{
	if (const auto* vfEditingSupport = dynamic_cast<const IViewFactoryEditingSupport*> (&factory))
	{
		viewAndDisplayNameList = vfEditingSupport->collectRegisteredViewAndDisplayNames ();
		for (const auto& e : viewAndDisplayNameList)
		{
			names.emplace_back (&e.second);
		}
		names.sort ([] (const auto& lhs, const auto& rhs) { return *lhs < *rhs; });
	}
}

//----------------------------------------------------------------------------------------------------
void UIViewCreatorDataSource::addViewToCurrentEditView (int32_t row)
{
	if (auto dataBrowser = dbPtr.lock ())
	{
		auto controller =
			getViewController (*dataBrowser.get (), true).cast<UIViewCreatorController> ();
		if (controller)
		{
			if (auto editController = controller->getBaseController ().cast<UIEditController> ())
			{
				SPtr<UISelection> selection = createSelection (row);
				editController->addSelectionToCurrentView (selection);
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------
SPtr<UISelection> createSelectionFromViewName (const std::string& viewName,
											   const IViewFactory& factory,
											   const UIDescription& description,
											   const SPtr<UIAttributes>& optionalAttributes)
{
	SPtr<UISelection> selection;
	UIAttributes viewAttr;
	viewAttr.setAttribute (UIViewCreator::kAttrClass, viewName);
	if (optionalAttributes)
	{
		for (auto& a : *optionalAttributes.get ())
			viewAttr.setAttribute (a.first, a.second);
	}
	auto view = factory.createView (viewAttr, description);
	if (view)
	{
		if (view->getViewSize ().isEmpty ())
		{
			CRect size (CPoint (0, 0), CPoint (20, 20));
			view->setViewSize (size);
			view->setMouseableArea (size);
		}
		selection = makeShared<UISelection> ();
		selection->add (view);
	}
	return selection;
}

//----------------------------------------------------------------------------------------------------
SPtr<UISelection> UIViewCreatorDataSource::createSelection (int32_t row)
{
	SPtr<UISelection> selection;
	auto viewDisplayName = getStringList ()->at (static_cast<uint32_t> (row)).getString ();
	auto it = std::find_if (viewAndDisplayNameList.begin (), viewAndDisplayNameList.end (),
	                        [&] (const auto& entry) { return entry.second == viewDisplayName; });
	if (it == viewAndDisplayNameList.end ())
		return nullptr;
	return createSelectionFromViewName (*it->first, factory, *description.get (), nullptr);
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIViewCreatorDataSource::dbOnMouseDown (const CPoint& where,
														  const CButtonState& buttons, int32_t row,
														  int32_t column, CDataBrowser& browser)
{
	if (buttons.isLeftButton ())
	{
		dragStartMouseObserver.init (where);
		if (!buttons.isDoubleClick ())
			return kMouseEventHandled;
		addViewToCurrentEditView (row);
	}
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIViewCreatorDataSource::dbOnMouseMoved (const CPoint& where,
														   const CButtonState& buttons, int32_t row,
														   int32_t column, CDataBrowser& browser)
{
	if (buttons.isLeftButton () && row != -1 && column != -1)
	{
		if (dragStartMouseObserver.shouldStartDrag (where))
		{
			auto selRow = browser.getSelection ().front ();
			SPtr<UISelection> selection = createSelection (selRow);
			CMemoryStream stream (1024, 1024, false);
			if (selection->store (stream, description))
			{
				stream.end ();
				auto dropSource = CDropSource::create (stream.getBuffer (),
				                                       static_cast<uint32_t> (stream.tell ()),
				                                       CDropSource::kText);
				auto bitmap = createBitmapFromSelection (*selection.get (),
														 browser.getFrame ()->getScaleFactor ());
				browser.doDrag (DragDescription (dropSource, CPoint (), bitmap));
			}
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
