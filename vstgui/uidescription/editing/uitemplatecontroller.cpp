// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uitemplatecontroller.h"

#if VSTGUI_LIVE_EDITING

#include "../uiviewfactory.h"
#include "../uiattributes.h"
#include "../../lib/controls/ctextedit.h"
#include "../../lib/controls/coptionmenu.h"
#include "../../lib/controls/cscrollbar.h"
#include "uieditcontroller.h"
#include "uiselection.h"
#include "uiundomanager.h"
#include "uiactions.h"
#include "uieditmenucontroller.h"
#include <cassert>

#ifdef verify
	#undef verify
#endif

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UINavigationDataSource : public GenericStringListDataBrowserSource
{
public:
	UINavigationDataSource (IGenericStringListDataBrowserSourceSelectionChanged* delegate)
	: GenericStringListDataBrowserSource (nullptr, delegate) { textInset.x = 4.; headerBackgroundColor = kTransparentCColor; }

	int32_t dbOnKeyDown (const VstKeyCode& key, CDataBrowser* browser) override
	{
		if (dynamic_cast<CTextEdit*> (browser->getFrame ()->getFocusView ()))
			return -1;
		if (key.virt == VKEY_LEFT)
		{
			if (auto parent = browser->getParentView ()->asViewContainer ())
			{
				if (parent->advanceNextFocusView (browser, true))
				{
					return 1;
				}
			}
		}
		else if (key.virt == VKEY_RIGHT)
		{
			if (auto parent = browser->getParentView ()->asViewContainer ())
			{
				if (parent->advanceNextFocusView (browser, false))
				{
					CView* focusView = dynamic_cast<CView*> (browser->getFrame()->getFocusView ());
					if (focusView)
					{
						CViewContainer* parent = focusView->getParentView ()->asViewContainer ();
						while (parent != browser->getFrame ())
						{
							parent = parent->getParentView ()->asViewContainer ();
							CDataBrowser* focusBrowser = dynamic_cast<CDataBrowser*>(parent);
							if (focusBrowser)
							{
								if (focusBrowser->getSelectedRow() == CDataBrowser::kNoSelection)
									focusBrowser->setSelectedRow (0);
								break;
							}
						}
					}
					return 1;
				}
			}
		}
		return GenericStringListDataBrowserSource::dbOnKeyDown (key, browser);
	}
	virtual const UTF8String& getHeaderTitle () const { return headerTitle; }
	void dbDrawHeader (CDrawContext* context, const CRect& size, int32_t column, int32_t flags, CDataBrowser* browser) override
	{
		context->setDrawMode (kAliasing);
		context->setLineWidth (1);
		if (headerBackgroundColor == kTransparentCColor)
		{
			double h,s,l;
			rowAlternateBackColor.toHSL (h, s, l);
			l /= 2.;
			headerBackgroundColor.fromHSL (h, s, l);
			headerBackgroundColor.alpha = rowAlternateBackColor.alpha;
		}
		context->setFillColor (headerBackgroundColor);
		context->drawRect (size, kDrawFilled);
		context->setFrameColor (rowlineColor);
		context->drawLine (CPoint (size.left, size.bottom-1), CPoint (size.right, size.bottom-1));
		if (!getHeaderTitle ().empty ())
		{
			if (headerFont == nullptr)
			{
				headerFont = makeOwned<CFontDesc> (*drawFont);
				headerFont->setStyle (kBoldFace);
				headerFont->setSize (headerFont->getSize ()-1);
			}
			context->setFont (headerFont);
			context->setFontColor (fontColor);
			context->drawString (getHeaderTitle ().getPlatformString (), size, kCenterText);
		}
	}
protected:
	mutable UTF8String headerTitle;
	CColor headerBackgroundColor;
	SharedPointer<CFontDesc> headerFont;
};

//----------------------------------------------------------------------------------------------------
class UITemplatesDataSource : public UINavigationDataSource
{
public:
	UITemplatesDataSource (IGenericStringListDataBrowserSourceSelectionChanged* delegate, UIDescription* description, IActionPerformer* actionPerformer, const std::string* templateName);
	
	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) override;
	void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText, CDataBrowser* browser) override;
	void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* textEditControl, CDataBrowser* browser) override;
	void dbAttached (CDataBrowser* browser) override;
protected:
	SharedPointer<UIDescription> description;
	IActionPerformer* actionPerformer;
	std::string firstSelectedTemplateName;
};

//----------------------------------------------------------------------------------------------------
class UIViewListDataSource : public UINavigationDataSource
{
public:
	UIViewListDataSource (CViewContainer* view, const IViewFactory* viewFactory, UISelection* selection, UIUndoManager* undoManager ,IGenericStringListDataBrowserSourceSelectionChanged* delegate);
	~UIViewListDataSource () override;

	CViewContainer* getView () const { return view; }
	CView* getSubview (int32_t index);
	
	bool update (CViewContainer* vc);
	void remove ();
protected:
	const UTF8String& getHeaderTitle () const override
	{
		headerTitle = "";
		if (view)
		{
			headerTitle = viewFactory->getViewName (view);
			if (headerTitle.empty () && view->getParentView ())
				headerTitle = viewFactory->getViewName (view->getParentView ());
		}
		return headerTitle;
	}
	
	void verify ();
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;

	CCoord calculateSubViewWidth (CViewContainer* view);
	void dbSelectionChanged (CDataBrowser* browser) override;
	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) override;
	int32_t dbOnKeyDown (const VstKeyCode& key, CDataBrowser* browser) override;

	CViewContainer* view;
	const IViewFactory* viewFactory;
	UIViewListDataSource* next;
	SharedPointer<UISelection> selection;
	SharedPointer<UIUndoManager> undoManager;
	CView* selectedView;
	StringVector names;
	std::vector<CView*> subviews;
	bool inUpdate;
};

//----------------------------------------------------------------------------------------------------
IdStringPtr UITemplateController::kMsgTemplateChanged = "UITemplateController::kMsgTemplateChanged";
IdStringPtr UITemplateController::kMsgTemplateNameChanged = "UITemplateController::kMsgTemplateNameChanged";

//----------------------------------------------------------------------------------------------------
UITemplateController::UITemplateController (IController* baseController, UIDescription* description, UISelection* selection, UIUndoManager* undoManager, IActionPerformer* actionPerformer)
: DelegationController (baseController)
, editDescription (description)
, selection (selection)
, undoManager (undoManager)
, actionPerformer (actionPerformer)
, templateView (nullptr)
, templateDataBrowser (nullptr)
, mainViewDataSource (nullptr)
, selectedTemplateName (nullptr)
{
	editDescription->addDependency (this);
}

//----------------------------------------------------------------------------------------------------
UITemplateController::~UITemplateController ()
{
	if (mainViewDataSource)
		mainViewDataSource->forget ();
	editDescription->removeDependency (this);
}

//----------------------------------------------------------------------------------------------------
void UITemplateController::setupDataBrowser (CDataBrowser* orignalBrowser, CDataBrowser* dataBrowser)
{
	if (orignalBrowser)
	{
		dataBrowser->setTransparency (orignalBrowser->getTransparency ());
		dataBrowser->setBackgroundColor (orignalBrowser->getBackgroundColor ());
		dataBrowser->setAutosizeFlags (orignalBrowser->getAutosizeFlags ());
		dataBrowser->setStyle (orignalBrowser->getStyle ());
		dataBrowser->setScrollbarWidth (orignalBrowser->getScrollbarWidth ());
		CScrollbar* sb1 = orignalBrowser->getHorizontalScrollbar ();
		CScrollbar* sb2 = dataBrowser->getHorizontalScrollbar ();
		if (sb1 && sb2)
		{
			sb2->setScrollerColor (sb1->getScrollerColor ());
			sb2->setBackgroundColor (sb1->getBackgroundColor ());
			sb2->setFrameColor (sb1->getFrameColor ());
		}
		sb1 = orignalBrowser->getVerticalScrollbar ();
		sb2 = dataBrowser->getVerticalScrollbar ();
		if (sb1 && sb2)
		{
			sb2->setScrollerColor (sb1->getScrollerColor ());
			sb2->setBackgroundColor (sb1->getBackgroundColor ());
			sb2->setFrameColor (sb1->getFrameColor ());
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UITemplateController::selectTemplate (UTF8StringPtr name)
{
	if (templateDataBrowser)
	{
		int32_t index = 0;
		for (auto& templName : templateNames)
		{
			if (templName == name)
			{
				templateDataBrowser->setSelectedRow (index, true);
				break;
			}
			index++;
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UITemplateController::dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source)
{
	if (source->getStringList () == &templateNames)
	{
		UTF8String* newName = nullptr;
		if (selectedRow == CDataBrowser::kNoSelection)
			newName = nullptr;
		else
			newName = &templateNames[static_cast<uint32_t> (selectedRow)];

		if ((newName == nullptr && selectedTemplateName != nullptr)
		 || (newName != nullptr && selectedTemplateName == nullptr)
		 || (newName != selectedTemplateName && *newName != *selectedTemplateName))
		{
			selectedTemplateName = newName;
			UIAttributes* attr = editDescription->getCustomAttributes ("UITemplateController", true);
			if (attr)
			{
				attr->setAttribute ("SelectedTemplate", selectedTemplateName ? selectedTemplateName->getString () : "");
			}

			changed (kMsgTemplateChanged);
		}
		else if (templateView)
		{
			selection->setExclusive (templateView);
		}
		else
			selection->empty ();
		return;
	}
}

//----------------------------------------------------------------------------------------------------
CMessageResult UITemplateController::notify (CBaseObject* sender, IdStringPtr message)
{
	if (templateDataBrowser && message == UIDescription::kMessageTemplateChanged)
	{
		GenericStringListDataBrowserSource* dataSource = dynamic_cast<GenericStringListDataBrowserSource*>(templateDataBrowser->getDelegate ());
		if (dataSource)
		{
			DeferChanges dc (this);
			int32_t rowToSelect = templateDataBrowser->getSelectedRow ();
			int32_t index = 0;
			auto selectedTemplateStr = selectedTemplateName ? *selectedTemplateName : "";
			templateNames.clear ();
			dataSource->setStringList (&templateNames);
			std::list<const std::string*> tmp;
			editDescription->collectTemplateViewNames (tmp);
			tmp.sort (UIEditController::std__stringCompare);
			for (auto& name : tmp)
			{
				templateNames.emplace_back (*name);
				if (*name == selectedTemplateStr)
					rowToSelect = index;
				++index;
			}
			if (rowToSelect < 0)
				rowToSelect = 0;
			dataSource->setStringList (&templateNames);
			templateDataBrowser->setSelectedRow (rowToSelect, true);
		}
		return kMessageNotified;
	}
	return kMessageUnknown;
}

//----------------------------------------------------------------------------------------------------
void UITemplateController::setTemplateView (CViewContainer* view)
{
	if (view != templateView && templateDataBrowser && templateDataBrowser->getParentView ())
	{
		templateView = view;
		if (mainViewDataSource)
		{
			mainViewDataSource->remove ();
			mainViewDataSource->forget ();
			mainViewDataSource = nullptr;
		}
		if (templateView && templateDataBrowser)
		{
			CViewContainer* parentView = static_cast<CViewContainer*>(templateDataBrowser->getParentView ());
			if (parentView)
			{
				const IViewFactory* viewFactory = editDescription->getViewFactory ();
				mainViewDataSource = new UIViewListDataSource (templateView, viewFactory, selection, undoManager, this);
				UIEditController::setupDataSource (mainViewDataSource);
				CRect r (templateDataBrowser->getViewSize ());
				r.offset (r.getWidth (), 0);
				CDataBrowser* browser = new CDataBrowser (r, mainViewDataSource);
				setupDataBrowser (templateDataBrowser, browser);
				parentView->addView (browser);
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------
CView* UITemplateController::createView (const UIAttributes& attributes, const IUIDescription* description)
{
	const std::string* name = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (name)
	{
		if (*name == "TemplateBrowser")
		{
			vstgui_assert (templateDataBrowser == nullptr);
			std::list<const std::string*> tmp;
			editDescription->collectTemplateViewNames (tmp);
			tmp.sort (UIEditController::std__stringCompare);
			for (auto& name : tmp)
				templateNames.emplace_back (*name);
			
			UIAttributes* attr = editDescription->getCustomAttributes ("UITemplateController", true);
			const std::string* templateName = attr ? attr->getAttributeValue ("SelectedTemplate") : nullptr;
			UITemplatesDataSource* dataSource = new UITemplatesDataSource (this, editDescription, actionPerformer, templateName);
			dataSource->setStringList (&templateNames);
			UIEditController::setupDataSource (dataSource);
			templateDataBrowser = new CDataBrowser (CRect (0, 0, 0, 0), dataSource, CDataBrowser::kDrawRowLines|CScrollView::kAutoHideScrollbars|CScrollView::kHorizontalScrollbar|CScrollView::kVerticalScrollbar|CDataBrowser::kDrawHeader);
			dataSource->forget ();
			return templateDataBrowser;
		}
	}
	return DelegationController::createView (attributes, description);
}

//----------------------------------------------------------------------------------------------------
CView* UITemplateController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	return DelegationController::verifyView (view, attributes, description);
}

//----------------------------------------------------------------------------------------------------
IController* UITemplateController::createSubController (UTF8StringPtr name, const IUIDescription* description)
{
	return DelegationController::createSubController (name, description);
}

//----------------------------------------------------------------------------------------------------
void UITemplateController::appendContextMenuItems (COptionMenu& contextMenu, CView* view, const CPoint& where)
{
	CPoint w (where);
	view->localToFrame (w);
	templateDataBrowser->frameToLocal (w);
	if (!templateDataBrowser->hitTest (w))
		return;
	auto cell = templateDataBrowser->getCellAt (w);
	if (!cell.isValid ())
		return;
	auto dataSource = dynamic_cast<UITemplatesDataSource*> (templateDataBrowser->getDelegate ());
	auto templateName = dataSource->getStringList()->at (static_cast<uint32_t> (cell.row));
	vstgui_assert (dataSource);
	auto item = new CCommandMenuItem ("Duplicate Template '" + templateName + "'");
	item->setActions ([this, cell, dataSource] (CCommandMenuItem*) {
		std::list<const std::string*> tmp;
		editDescription->collectTemplateViewNames (tmp);
		std::string newName (dataSource->getStringList ()->at (static_cast<uint32_t> (cell.row)).data ());
		UIEditMenuController::createUniqueTemplateName (tmp, newName);
		actionPerformer->performDuplicateTemplate (dataSource->getStringList ()->at (static_cast<uint32_t> (cell.row)).data (), newName.data ());
	});
	contextMenu.addEntry (item);
	item = new CCommandMenuItem ("Delete Template '" + templateName + "'");
	item->setActions ([this, cell, dataSource] (CCommandMenuItem*) {
		actionPerformer->performDeleteTemplate (dataSource->getStringList ()->at (static_cast<uint32_t> (cell.row)).data ());
	});
	contextMenu.addEntry (item);
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIViewListDataSource::UIViewListDataSource (CViewContainer* view, const IViewFactory* viewFactory, UISelection* selection, UIUndoManager* undoManager, IGenericStringListDataBrowserSourceSelectionChanged* delegate)
: UINavigationDataSource (delegate)
, view (view)
, viewFactory (viewFactory)
, next (nullptr)
, selection (selection)
, undoManager (undoManager)
, selectedView (nullptr)
, inUpdate (false)
{
	update (view);
	undoManager->addDependency (this);
}

//----------------------------------------------------------------------------------------------------
UIViewListDataSource::~UIViewListDataSource ()
{
	undoManager->removeDependency (this);
}

//----------------------------------------------------------------------------------------------------
CView* UIViewListDataSource::getSubview (int32_t index)
{
	if (index >= 0 && index < (int32_t)subviews.size ())
		return subviews[static_cast<uint32_t> (index)];
	return nullptr;
}

//----------------------------------------------------------------------------------------------------
bool UIViewListDataSource::update (CViewContainer* vc)
{
	inUpdate = true;
	names.clear ();
	subviews.clear ();
	ViewIterator it (vc);
	while (*it)
	{
		CView* subview = *it;
		IdStringPtr viewName = viewFactory->getViewName (subview);
		if (viewName)
		{
			names.emplace_back (viewName);
			subviews.emplace_back (subview);
		}
		it++;
	}
	if (names.empty () && vc->getNbViews () > 0)
	{
		ViewIterator it (vc);
		while (*it)
		{
			if (auto subview = (*it)->asViewContainer ())
			{
				if (update (subview))
				{
					view = subview;
					inUpdate = false;
					return true;
				}
			}
			it++;
		}
		inUpdate = false;
		return false;
	}
	setStringList (&names);
	inUpdate = false;
	return true;
}

//----------------------------------------------------------------------------------------------------
CCoord UIViewListDataSource::calculateSubViewWidth (CViewContainer* view)
{
	CCoord result = 0;
	
	ViewIterator it (view);
	while (*it)
	{
		result += (*it)->getViewSize ().getWidth ();
		it++;
	}
	return result;
}

//----------------------------------------------------------------------------------------------------
void UIViewListDataSource::dbSelectionChanged (CDataBrowser* browser)
{
	CView* subview = getSubview (browser->getSelectedRow ());
	if (subview == selectedView || inUpdate)
		return;
	selectedView = subview;
	if (next)
	{
		next->remove ();
		next = nullptr;
	}
	GenericStringListDataBrowserSource::dbSelectionChanged (browser);
	if (auto container = subview ? subview->asViewContainer () : nullptr)
	{
		UIViewListDataSource* dataSource = new UIViewListDataSource (container, viewFactory, selection, undoManager, delegate);
		UIEditController::setupDataSource (dataSource);
		CRect r (browser->getViewSize ());
		r.offset (r.getWidth (), 0);
		CDataBrowser* newDataBrowser = new CDataBrowser (r, dataSource);
		UITemplateController::setupDataBrowser (browser, newDataBrowser);
		CViewContainer* parentView = static_cast<CViewContainer*>(browser->getParentView ());
		parentView->addView (newDataBrowser);
		next = dataSource;
		dataSource->forget ();
		CScrollView* scrollView = dynamic_cast<CScrollView*>(parentView->getParentView ());
		if (scrollView)
		{
			CRect containerSize (scrollView->getContainerSize ());
			containerSize.right = calculateSubViewWidth (parentView);
			scrollView->setContainerSize (containerSize, true);
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIViewListDataSource::remove ()
{
	if (next)
	{
		next->remove ();
		next = nullptr;
	}
	if (dataBrowser)
	{
		CViewContainer* parentView = static_cast<CViewContainer*>(dataBrowser->getParentView ());
		CScrollView* scrollView = dynamic_cast<CScrollView*>(parentView->getParentView ());
		parentView->removeView (dataBrowser);
		if (scrollView)
		{
			CRect containerSize (scrollView->getContainerSize ());
			containerSize.right = calculateSubViewWidth (parentView);
			scrollView->setContainerSize (containerSize, true);
		}
	}
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIViewListDataSource::dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
{
	if (buttons.isLeftButton() && buttons.isDoubleClick ())
	{
		CView* subview = getSubview (row);
		if (subview)
		{
			if (buttons.getModifierState () & kControl)
			{
				if (selection->contains (subview))
					selection->remove (subview);
				else
					selection->add (subview);
			}
			else
				selection->setExclusive (subview);
		}
	}
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIViewListDataSource::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == UIUndoManager::kMsgChanged)
	{
		update (view);
		if (selectedView)
		{
			if (dataBrowser)
			{
				int32_t index = 0;
				for (std::vector<CView*>::const_iterator it = subviews.begin (); it != subviews.end (); ++it, index++)
				{
					if (*it == selectedView)
					{
						dataBrowser->setSelectedRow (index, true);
						return kMessageNotified;
					}
				}
			}
			selectedView = nullptr;
			if (next)
			{
				next->remove ();
				next = nullptr;
			}
		}
		return kMessageNotified;
	}
	return GenericStringListDataBrowserSource::notify (sender, message);
}

//----------------------------------------------------------------------------------------------------
int32_t UIViewListDataSource::dbOnKeyDown (const VstKeyCode& key, CDataBrowser* browser)
{
	if (key.virt != 0)
	{
		int32_t row = browser->getSelectedRow ();
		CView* subview = getSubview (row);
		if (subview)
		{
			switch (key.virt)
			{
				case VKEY_RETURN:
				{
					selection->setExclusive (subview);
					return 1;
				}
			}
		}
	}
	return UINavigationDataSource::dbOnKeyDown (key, browser);
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UITemplatesDataSource::UITemplatesDataSource (IGenericStringListDataBrowserSourceSelectionChanged* delegate, UIDescription* description, IActionPerformer* actionPerformer, const std::string* templateName)
: UINavigationDataSource (delegate)
, description (description)
, actionPerformer (actionPerformer)
{
	headerTitle = "Templates";
	if (templateName)
		firstSelectedTemplateName = *templateName;
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UITemplatesDataSource::dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
{
	if (buttons.isLeftButton ())
	{
		if (buttons.isDoubleClick ())
		{
			browser->beginTextEdit (CDataBrowser::Cell (row, column), getStringList ()->at (static_cast<uint32_t> (row)).data ());
			return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		}
		delegate->dbSelectionChanged (row, this);
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	else if (buttons.isRightButton ())
	{
		COptionMenu menu;
		menu.addEntry ("Duplicate");
		menu.addEntry ("Delete");
		CPoint p (where);
		browser->CView::localToFrame (p);
		if (menu.popup (browser->getFrame(), p))
		{
			switch (menu.getLastResult ())
			{
				case 0:
				{
					std::list<const std::string*> tmp;
					description->collectTemplateViewNames (tmp);
					std::string newName (getStringList ()->at (static_cast<uint32_t> (row)).data ());
					UIEditMenuController::createUniqueTemplateName (tmp, newName);
					actionPerformer->performDuplicateTemplate (getStringList ()->at (static_cast<uint32_t> (row)).data (), newName.data ());
					break;
				}
				case 1:
				{
					actionPerformer->performDeleteTemplate (getStringList ()->at (static_cast<uint32_t> (row)).data ());
					break;
				}
			}
		}
	}
	return UINavigationDataSource::dbOnMouseDown (where, buttons, row, column, browser);
}

//----------------------------------------------------------------------------------------------------
void UITemplatesDataSource::dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText, CDataBrowser* browser)
{
	auto oldName = getStringList ()->at (static_cast<uint32_t> (row));
	if (oldName != newText)
	{
		for (auto& name : *getStringList ())
		{
			if (name == newText)
				return;
		}
		actionPerformer->performTemplateNameChange (oldName.data (), newText);
	}
}

//----------------------------------------------------------------------------------------------------
void UITemplatesDataSource::dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* textEditControl, CDataBrowser* browser)
{
	textEditControl->setBackColor (kWhiteCColor);
	textEditControl->setFontColor (fontColor);
	textEditControl->setFont (drawFont);
	textEditControl->setHoriAlign (kLeftText);
	textEditControl->setTextInset (textInset);
}

//----------------------------------------------------------------------------------------------------
void UITemplatesDataSource::dbAttached (CDataBrowser* browser)
{
	UINavigationDataSource::dbAttached (browser);
	if (getStringList ())
	{
		if (firstSelectedTemplateName.empty ())
		{
			browser->setSelectedRow (0, true);
		}
		else
		{
			uint32_t index = 0;
			for (auto& name : *getStringList ())
			{
				if (name == firstSelectedTemplateName)
				{
					browser->setSelectedRow (static_cast<int32_t> (index), true);
					break;
				}
				index++;
			}
		}
	}
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
