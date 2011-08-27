#include "uitemplatecontroller.h"

#if VSTGUI_LIVE_EDITING

#include "../uiviewfactory.h"
#include "../../lib/controls/ctextedit.h"
#include "uieditcontroller.h"
#include "uiselection.h"
#include "uiundomanager.h"
#include "uiactions.h"
#include <assert.h>
/*
	TODO: view z index editing via row drag and drop
*/

#ifdef verify
	#undef verify
#endif

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UINavigationDataSource : public GenericStringListDataBrowserSource
{
public:
	UINavigationDataSource (IGenericStringListDataBrowserSourceSelectionChanged* delegate)
	: GenericStringListDataBrowserSource (0, delegate) {}

	int32_t dbOnKeyDown (const VstKeyCode& key, CDataBrowser* browser)
	{
		if (dynamic_cast<CTextEdit*> (browser->getFrame ()->getFocusView ()))
			return -1;
		if (key.virt == VKEY_LEFT)
		{
			CViewContainer* parent = dynamic_cast<CViewContainer*>(browser->getParentView ());
			if (parent)
			{
				if (parent->advanceNextFocusView (browser, true))
				{
					return 1;
				}
			}
		}
		else if (key.virt == VKEY_RIGHT)
		{
			CViewContainer* parent = dynamic_cast<CViewContainer*>(browser->getParentView ());
			if (parent)
			{
				if (parent->advanceNextFocusView (browser, false))
				{
					CView* focusView = dynamic_cast<CView*> (browser->getFrame()->getFocusView ());
					if (focusView)
					{
						CViewContainer* parent = dynamic_cast<CViewContainer*>(focusView->getParentView ());
						while (parent != browser->getFrame ())
						{
							parent = dynamic_cast<CViewContainer*>(parent->getParentView ());
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
};

//----------------------------------------------------------------------------------------------------
class UITemplatesDataSource : public UINavigationDataSource
{
public:
	UITemplatesDataSource (IGenericStringListDataBrowserSourceSelectionChanged* delegate, UIDescription* description, const std::string* templateName);
	
	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser);
	void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText, CDataBrowser* browser);
	void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* textEditControl, CDataBrowser* browser);
	void dbAttached (CDataBrowser* browser);
protected:
	SharedPointer<UIDescription> description;
	std::string firstSelectedTemplateName;
};

//----------------------------------------------------------------------------------------------------
class UIViewListDataSource : public UINavigationDataSource
{
public:
	UIViewListDataSource (CViewContainer* view, UIViewFactory* viewFactory, UISelection* selection, UIUndoManager* undoManager ,IGenericStringListDataBrowserSourceSelectionChanged* delegate);
	~UIViewListDataSource ();

	CViewContainer* getView () const { return view; }
	CView* getSubview (int32_t index);
	
	bool update (CViewContainer* vc);
	void remove ();
protected:
	void verify ();
	CMessageResult notify (CBaseObject* sender, IdStringPtr message);

	CCoord calculateSubViewWidth (CViewContainer* view);
	void dbSelectionChanged (CDataBrowser* browser);
	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser);
	int32_t dbOnKeyDown (const VstKeyCode& key, CDataBrowser* browser);

	CViewContainer* view;
	SharedPointer<UIViewFactory> viewFactory;
	UIViewListDataSource* next;
	SharedPointer<UISelection> selection;
	SharedPointer<UIUndoManager> undoManager;
	CView* selectedView;
	std::vector<std::string> names;
	std::vector<CView*> subviews;
	bool inUpdate;
};

//----------------------------------------------------------------------------------------------------
IdStringPtr UITemplateController::kMsgTemplateChanged = "UITemplateControllerTemplateChanged";

//----------------------------------------------------------------------------------------------------
UITemplateController::UITemplateController (IController* baseController, UIDescription* description, UISelection* selection, UIUndoManager* undoManager)
: DelegationController (baseController)
, editDescription (description)
, selection (selection)
, undoManager (undoManager)
, templateView (0)
, templateDataBrowser (0)
, mainViewDataSource (0)
, selectedTemplateName (0)
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
void UITemplateController::dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source)
{
	if (source->getStringList () == &templateNames)
	{
		if (selectedRow == CDataBrowser::kNoSelection)
			selectedTemplateName = 0;
		else
			selectedTemplateName = &templateNames[selectedRow];
		changed (kMsgTemplateChanged);
		return;
	}
}

//----------------------------------------------------------------------------------------------------
CMessageResult UITemplateController::notify (CBaseObject* sender, IdStringPtr message)
{
	if (templateDataBrowser && message == UIDescription::kMessageTemplateChanged)
	{
		GenericStringListDataBrowserSource* dataSource = dynamic_cast<GenericStringListDataBrowserSource*>(templateDataBrowser->getDataSource ());
		if (dataSource)
		{
			int32_t rowToSelect = templateDataBrowser->getSelectedRow ();
			int32_t index = 0;
			templateNames.clear ();
			dataSource->setStringList (&templateNames);
			std::list<const std::string*> tmp;
			editDescription->collectTemplateViewNames (tmp);
			tmp.sort (UIEditController::std__stringCompare);
			for (std::list<const std::string*>::const_iterator it = tmp.begin (); it != tmp.end (); it++, index++)
			{
				templateNames.push_back ((*it)->c_str ());
				if (selectedTemplateName && *(*it) == *selectedTemplateName)
					rowToSelect = index;
			}
			dataSource->setStringList (&templateNames);
			templateDataBrowser->setSelectedRow (rowToSelect, true);
		}
		return kMessageNotified;
	}
	else if (message == UIDescription::kMessageBeforeSave)
	{
		UIAttributes* attr = editDescription->getCustomAttributes ("UITemplateController", true);
		if (attr)
		{
			attr->setAttribute ("SelectedTemplate", selectedTemplateName ? selectedTemplateName->c_str () : "");
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
			mainViewDataSource = 0;
		}
		if (templateView && templateDataBrowser)
		{
			CViewContainer* parentView = reinterpret_cast<CViewContainer*>(templateDataBrowser->getParentView ());
			if (parentView)
			{
				UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*> (editDescription->getViewFactory ());
				mainViewDataSource = new UIViewListDataSource (templateView, viewFactory, selection, undoManager, this);
				UIEditController::setupDataSource (mainViewDataSource);
				CRect r (templateDataBrowser->getViewSize ());
				r.offset (r.getWidth (), 0);
				CDataBrowser* browser = new CDataBrowser (r, 0, mainViewDataSource);
				setupDataBrowser (templateDataBrowser, browser);
				parentView->addView (browser);
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------
CView* UITemplateController::createView (const UIAttributes& attributes, IUIDescription* description)
{
	const std::string* name = attributes.getAttributeValue ("custom-view-name");
	if (name)
	{
		if (*name == "TemplateBrowser")
		{
			assert (templateDataBrowser == 0);
			std::list<const std::string*> tmp;
			editDescription->collectTemplateViewNames (tmp);
			tmp.sort (UIEditController::std__stringCompare);
			for (std::list<const std::string*>::const_iterator it = tmp.begin (); it != tmp.end (); it++)
				templateNames.push_back ((*it)->c_str ());
			
			UIAttributes* attr = editDescription->getCustomAttributes ("UITemplateController", true);
			const std::string* templateName = attr ? attr->getAttributeValue ("SelectedTemplate") : 0;
			UITemplatesDataSource* dataSource = new UITemplatesDataSource (this, editDescription, templateName);
			dataSource->setStringList (&templateNames);
			UIEditController::setupDataSource (dataSource);
			templateDataBrowser = new CDataBrowser (CRect (0, 0, 0, 0), 0, dataSource, CDataBrowser::kDrawRowLines|CScrollView::kHorizontalScrollbar | CScrollView::kVerticalScrollbar);
			dataSource->forget ();
			return templateDataBrowser;
		}
	}
	return DelegationController::createView (attributes, description);
}

//----------------------------------------------------------------------------------------------------
CView* UITemplateController::verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
{
	return DelegationController::verifyView (view, attributes, description);
}

//----------------------------------------------------------------------------------------------------
IController* UITemplateController::createSubController (UTF8StringPtr name, IUIDescription* description)
{
	return DelegationController::createSubController (name, description);
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIViewListDataSource::UIViewListDataSource (CViewContainer* view, UIViewFactory* viewFactory, UISelection* selection, UIUndoManager* undoManager, IGenericStringListDataBrowserSourceSelectionChanged* delegate)
: UINavigationDataSource (delegate)
, view (view)
, viewFactory (viewFactory)
, next (0)
, selection (selection)
, undoManager (undoManager)
, selectedView (0)
, inUpdate (false)
{
	update (view);
	selection->addDependency (this);
}

//----------------------------------------------------------------------------------------------------
UIViewListDataSource::~UIViewListDataSource ()
{
	selection->removeDependency (this);
}

//----------------------------------------------------------------------------------------------------
CView* UIViewListDataSource::getSubview (int32_t index)
{
	if (index >= 0 && index < (int32_t)subviews.size ())
		return subviews[index];
	return 0;
}

//----------------------------------------------------------------------------------------------------
bool UIViewListDataSource::update (CViewContainer* vc)
{
	inUpdate = true;
	names.clear ();
	setStringList (&names);
	subviews.clear ();
	ViewIterator it (vc);
	while (*it)
	{
		CView* subview = *it;
		IdStringPtr viewName = viewFactory->getViewName (subview);
		if (viewName)
		{
			names.push_back (viewName);
			subviews.push_back (subview);
		}
		it++;
	}
	if (names.empty () && view->getNbViews () > 0)
	{
		ViewIterator it (view);
		while (*it)
		{
			CViewContainer* subview = dynamic_cast<CViewContainer*>(*it);
			if (subview)
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
		next = 0;
	}
	GenericStringListDataBrowserSource::dbSelectionChanged (browser);
	CViewContainer* container = dynamic_cast<CViewContainer*>(subview);
	if (container)
	{
		UIViewListDataSource* dataSource = new UIViewListDataSource (container, viewFactory, selection, undoManager, delegate);
		UIEditController::setupDataSource (dataSource);
		CRect r (browser->getViewSize ());
		r.offset (r.getWidth (), 0);
		CDataBrowser* newDataBrowser = new CDataBrowser (r, 0, dataSource);
		UITemplateController::setupDataBrowser (browser, newDataBrowser);
		CViewContainer* parentView = reinterpret_cast<CViewContainer*>(browser->getParentView ());
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
		next = 0;
	}
	if (dataBrowser)
	{
		CViewContainer* parentView = reinterpret_cast<CViewContainer*>(dataBrowser->getParentView ());
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
	if (message == UISelection::kMsgSelectionChanged)
	{
		update (view);
		if (selectedView)
		{
			int32_t index = 0;
			for (std::vector<CView*>::const_iterator it = subviews.begin(); it != subviews.end (); it++, index++)
			{
				if (*it == selectedView)
				{
					dataBrowser->setSelectedRow (index, true);
					return kMessageNotified;
				}
			}
			selectedView = 0;
			if (next)
			{
				next->remove ();
				next = 0;
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
				case VKEY_UP:
				{
					if (key.modifier == MODIFIER_CONTROL && row > 0)
					{
						undoManager->pushAndPerform (new HierarchyMoveViewOperation (subview, selection, true));
						//browser->setSelectedRow (row-1, true);
						return 1;
					}
					break;
				}
				case VKEY_DOWN:
				{
					if (key.modifier == MODIFIER_CONTROL && row < view->getNbViews ())
					{
						undoManager->pushAndPerform (new HierarchyMoveViewOperation (subview, selection, false));
						//browser->setSelectedRow (row+1, true);
						return 1;
					}
					break;
				}
			}
		}
	}
	return UINavigationDataSource::dbOnKeyDown (key, browser);
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UITemplatesDataSource::UITemplatesDataSource (IGenericStringListDataBrowserSourceSelectionChanged* delegate, UIDescription* description, const std::string* templateName)
: UINavigationDataSource (delegate)
, description (description)
{
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
			browser->beginTextEdit (row, column, getStringList ()->at (row).c_str ());
		}
		else
		{
			delegate->dbSelectionChanged (browser->getSelectedRow (), this);
		}
	}
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//----------------------------------------------------------------------------------------------------
void UITemplatesDataSource::dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText, CDataBrowser* browser)
{
	browser->setSelectedRow (CDataBrowser::kNoSelection);
	description->changeTemplateName (getStringList ()->at (row).c_str (), newText);
	browser->setSelectedRow (row, true);
}

//----------------------------------------------------------------------------------------------------
void UITemplatesDataSource::dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* textEditControl, CDataBrowser* browser)
{
	textEditControl->setBackColor (kWhiteCColor);
	textEditControl->setFontColor (fontColor);
	textEditControl->setFont (drawFont);
	textEditControl->setHoriAlign (kLeftText);
}

//----------------------------------------------------------------------------------------------------
void UITemplatesDataSource::dbAttached (CDataBrowser* browser)
{
	UINavigationDataSource::dbAttached (browser);
	if (getStringList ())
	{
		if (firstSelectedTemplateName.empty ())
		{
			browser->setSelectedRow (0);
		}
		else
		{
			int32_t index = 0;
			for (std::vector<std::string>::const_iterator it = getStringList ()->begin (); it != getStringList ()->end (); it++, index++)
			{
				if (getStringList()->at (index) == firstSelectedTemplateName)
				{
					browser->setSelectedRow (index);
					break;
				}
			}
		}
	}
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
