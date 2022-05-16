// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uitemplatecontroller.h"

#if VSTGUI_LIVE_EDITING

#include "../uiviewfactory.h"
#include "../uiattributes.h"
#include "../viewcreator/viewcreator.h"
#include "../../lib/controls/ctextedit.h"
#include "../../lib/controls/coptionmenu.h"
#include "../../lib/controls/cscrollbar.h"
#include "../../lib/algorithm.h"
#include "../../lib/cgraphicspath.h"
#include "../../lib/cdropsource.h"
#include "../../lib/coffscreencontext.h"
#include "../../lib/dragging.h"
#include "../../lib/malloc.h"
#include "uieditcontroller.h"
#include "uiselection.h"
#include "uiundomanager.h"
#include "uiactions.h"
#include "uieditmenucontroller.h"
#include <cassert>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UINavigationDataSource : public GenericStringListDataBrowserSource
{
public:
	UINavigationDataSource (GenericStringListDataBrowserSourceSelectionChanged* delegate)
	: GenericStringListDataBrowserSource (nullptr, delegate) { textInset.x = 4.; }

	void dbOnKeyboardEvent (KeyboardEvent& event, CDataBrowser* browser) override
	{
		if (event.type == EventType::KeyDown &&
		    dynamic_cast<CTextEdit*> (browser->getFrame ()->getFocusView ()) == nullptr)
		{
			if (event.virt == VirtualKey::Left)
			{
				if (auto parent = browser->getParentView ()->asViewContainer ())
				{
					if (parent->advanceNextFocusView (browser, true))
					{
						browser->unselectAll ();
						event.consumed = true;
						return;
					}
				}
			}
			else if (event.virt == VirtualKey::Right)
			{
				if (auto parent = browser->getParentView ()->asViewContainer ())
				{
					if (parent->advanceNextFocusView (browser, false))
					{
						if (auto* focusView = browser->getFrame ()->getFocusView ())
						{
							auto* focusBrowser = dynamic_cast<CDataBrowser*>(focusView);
							parent = focusView->getParentView ()->asViewContainer ();
							while (!focusBrowser && parent != browser->getFrame ())
							{
								if (parent->getParentView () == nullptr)
									break;
								parent = parent->getParentView ()->asViewContainer ();
								focusBrowser = dynamic_cast<CDataBrowser*> (parent);
							}
							if (focusBrowser)
							{
								if (focusBrowser->getSelectedRow () == CDataBrowser::kNoSelection)
									focusBrowser->setSelectedRow (0);
							}
						}
						event.consumed = true;
						return;
					}
				}
			}
		}
		GenericStringListDataBrowserSource::dbOnKeyboardEvent (event, browser);
	}
	virtual const UTF8String& getHeaderTitle () const { return headerTitle; }
	void dbDrawHeader (CDrawContext* context, const CRect& size, int32_t column, int32_t flags, CDataBrowser* browser) override
	{
		context->setDrawMode (kAliasing);
		if (!headerGradient)
		{
			headerGradient = UIEditController::getEditorDescription ()->getGradient ("shading.light");
			UIEditController::getEditorDescription ()->getColor ("shading.light.frame", headerLineColor);
		}
		if (headerGradient)
		{
			if (auto path = owned (context->createGraphicsPath ()))
			{
				path->addRect (size);
				context->fillLinearGradient (path, *headerGradient, CPoint (size.left, size.top), CPoint (size.left, size.bottom));
			}
		}
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
		auto hairlineSize = context->getHairlineSize ();
		context->setLineWidth (hairlineSize);
		context->setFrameColor (headerLineColor);
		context->drawLine (CPoint (size.right-hairlineSize, size.top), CPoint (size.right-hairlineSize, size.bottom));
		context->drawLine (CPoint (size.left, size.bottom), CPoint (size.right-hairlineSize, size.bottom));
	}

	static void drawTriangle (CDrawContext* context, const CRect& size)
	{
		if (auto path = owned (context->createGraphicsPath ()))
		{
			CRect r (size);
			r.left = r.right - r.getHeight ();
			r.inset (4, 4);
			path->beginSubpath (r.getTopLeft ());
			path->addLine (r.getBottomLeft ());
			path->addLine (r.right, r.top + r.getHeight () / 2.);
			path->closeSubpath ();
			context->setFillColor (CColor (0, 0, 0, 30));
			context->drawGraphicsPath (path);
		}
	}

protected:
	mutable UTF8String headerTitle;
	CColor headerLineColor {kBlackCColor};
	SharedPointer<CFontDesc> headerFont;
	SharedPointer<CGradient> headerGradient;
};

//----------------------------------------------------------------------------------------------------
class UITemplatesDataSource : public UINavigationDataSource
{
public:
	UITemplatesDataSource (GenericStringListDataBrowserSourceSelectionChanged* delegate, UIDescription* description, IActionPerformer* actionPerformer, const std::string* templateName);
	
	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) override;
	void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText, CDataBrowser* browser) override;
	void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* textEditControl, CDataBrowser* browser) override;
	void dbAttached (CDataBrowser* browser) override;
	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser) override;
protected:
	SharedPointer<UIDescription> description;
	IActionPerformer* actionPerformer;
	std::string firstSelectedTemplateName;
};

//----------------------------------------------------------------------------------------------------
class UIViewListDataSource : public UINavigationDataSource, public IUIUndoManagerListener
{
public:
	UIViewListDataSource (CViewContainer* view, const IViewFactory* viewFactory, UISelection* selection, UIUndoManager* undoManager ,GenericStringListDataBrowserSourceSelectionChanged* delegate);
	~UIViewListDataSource () override;

	CViewContainer* getView () const { return view; }
	CView* getSubview (int32_t index);
	bool setSelectedView (CView* view, bool makeRowVisible = false);
	UIViewListDataSource* getNext () const { return next; }

	bool update (CViewContainer* vc);
	void remove ();
protected:
	UTF8String getViewDisplayString (CView* v) const
	{
		uint32_t outSize = 0;
		if (v->getAttributeSize (UIViewCreator::ViewCreator::labelAttrID, outSize))
		{
			Buffer<char> buffer (outSize);
			if (v->getAttribute (UIViewCreator::ViewCreator::labelAttrID, outSize, buffer.data (),
			                     outSize))
			{
				return buffer.data ();
			}
		}
		return viewFactory->getViewDisplayName (v);
	}
	const UTF8String& getHeaderTitle () const override
	{
		headerTitle = "";
		if (view)
		{
			headerTitle = getViewDisplayString (view);
			if (headerTitle.empty () && view->getParentView ())
				headerTitle = getViewDisplayString (view->getParentView ());
		}
		return headerTitle;
	}
	
	CCoord calculateSubViewWidth (CViewContainer* view) const;
	void dbSelectionChanged (CDataBrowser* browser) override;
	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row,
	                                 int32_t column, CDataBrowser* browser) override;
	CMouseEventResult dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row,
	                                  int32_t column, CDataBrowser* browser) override;
	void dbOnKeyboardEvent (KeyboardEvent& event, CDataBrowser* browser) override;
	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser) override;
	DragOperation dbOnDragEnterCell (int32_t row, int32_t column, const CPoint& where,
	                                 IDataPackage* drag, CDataBrowser* browser) override;
	DragOperation dbOnDragMoveInCell (int32_t row, int32_t column, const CPoint& where,
	                                  IDataPackage* drag, CDataBrowser* browser) override;
	void dbOnDragExitCell (int32_t row, int32_t column, IDataPackage* drag,
	                       CDataBrowser* browser) override;
	bool dbOnDropInCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag,
	                     CDataBrowser* browser) override;

	// IUIUndoManagerListener
	void onUndoManagerChange () override;

	CViewContainer* view;
	const UIViewFactory* viewFactory;
	UIViewListDataSource* next;
	SharedPointer<UISelection> selection;
	SharedPointer<UIUndoManager> undoManager;
	CView* selectedView;
	StringVector names;
	std::vector<CView*> subviews;
	bool inUpdate;
	DragStartMouseObserver dragStartMouseObserver;
	int32_t dragRow {-1};
	int32_t dragDestinationRow {-1};
};

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
	editDescription->registerListener (this);
}

//----------------------------------------------------------------------------------------------------
UITemplateController::~UITemplateController ()
{
	if (templateDataBrowser)
		templateDataBrowser->unregisterViewListener (this);
	if (mainViewDataSource)
		mainViewDataSource->forget ();
	editDescription->unregisterListener (this);
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
		if (name == nullptr)
		{
			templateDataBrowser->unselectAll ();
			return;
		}
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
			auto attr = editDescription->getCustomAttributes ("UITemplateController", true);
			if (attr)
			{
				attr->setAttribute ("SelectedTemplate", selectedTemplateName ? selectedTemplateName->getString () : "");
			}

			forEachListener ([] (IUITemplateControllerListener* l) { l->onTemplateSelectionChanged ();});
		}
		else if (templateView)
		{
			selection->setExclusive (templateView);
		}
		else
			selection->clear ();
		return;
	}
}

//----------------------------------------------------------------------------------------------------
void UITemplateController::onUIDescTemplateChanged (UIDescription* desc)
{
	if (!templateDataBrowser)
		return;
	auto dataSource = dynamic_cast<GenericStringListDataBrowserSource*>(templateDataBrowser->getDelegate ());
	if (dataSource)
	{
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

//------------------------------------------------------------------------
void UITemplateController::navigateTo (CView* view)
{
	std::list<CView*> parents;
	CView* v = view;
	while (auto parent = v->getParentView ())
	{
		if (parent == parent->getFrame ())
			return; // view is not a child of the templateView
		if (parent == templateView)
			break;
		if (UIViewFactory::getViewName (parent) == nullptr)
		{
			v = parent;
			continue;
		}
		parents.emplace_front (parent);
		v = parent;
	}
	UIViewListDataSource* dataSource = mainViewDataSource;
	for (auto parent : parents)
	{
		dataSource->setSelectedView (parent, true);
		dataSource = dataSource->getNext ();
		if (dataSource == nullptr)
			break;
	}
	if (dataSource)
		dataSource->setSelectedView (view, true);
}

//----------------------------------------------------------------------------------------------------
void UITemplateController::viewWillDelete (CView* view)
{
	if (view == templateDataBrowser)
	{
		templateDataBrowser->unregisterViewListener (this);
		templateDataBrowser = nullptr;
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
			for (auto& name2 : tmp)
				templateNames.emplace_back (*name2);
			
			auto attr = editDescription->getCustomAttributes ("UITemplateController", true);
			const std::string* templateName = attr ? attr->getAttributeValue ("SelectedTemplate") : nullptr;
			UITemplatesDataSource* dataSource = new UITemplatesDataSource (this, editDescription, actionPerformer, templateName);
			dataSource->setStringList (&templateNames);
			UIEditController::setupDataSource (dataSource);
			templateDataBrowser = new CDataBrowser (CRect (0, 0, 0, 0), dataSource, CDataBrowser::kDrawRowLines|CScrollView::kAutoHideScrollbars|CScrollView::kHorizontalScrollbar|CScrollView::kVerticalScrollbar|CDataBrowser::kDrawHeader);
			dataSource->forget ();
			templateDataBrowser->registerViewListener (this);
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
UIViewListDataSource::UIViewListDataSource (CViewContainer* view, const IViewFactory* viewFactory, UISelection* selection, UIUndoManager* undoManager, GenericStringListDataBrowserSourceSelectionChanged* delegate)
: UINavigationDataSource (delegate)
, view (view)
, viewFactory (dynamic_cast<const UIViewFactory*> (viewFactory))
, next (nullptr)
, selection (selection)
, undoManager (undoManager)
, selectedView (nullptr)
, inUpdate (false)
{
	update (view);
	undoManager->registerListener (this);
}

//----------------------------------------------------------------------------------------------------
UIViewListDataSource::~UIViewListDataSource ()
{
	undoManager->unregisterListener (this);
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
	vc->forEachChild ([&] (CView* subview) {
		auto viewName = getViewDisplayString (subview);
		if (!viewName.empty ())
		{
			names.emplace_back (std::move (viewName));
			subviews.emplace_back (subview);
		}
	});
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
			++it;
		}
		inUpdate = false;
		return false;
	}
	setStringList (&names);
	inUpdate = false;
	return true;
}

//----------------------------------------------------------------------------------------------------
CCoord UIViewListDataSource::calculateSubViewWidth (CViewContainer* inView) const
{
	CCoord result = 0;
	
	inView->forEachChild ([&result] (CView* subView) {
		result += subView->getViewSize ().getWidth ();
	});
	return result;
}

//----------------------------------------------------------------------------------------------------
bool UIViewListDataSource::setSelectedView (CView* newView, bool makeRowVisible)
{
	auto index = indexOf (subviews.begin (), subviews.end (), newView);
	if (!index)
		return false;

	selectedView = newView;
	dataBrowser->selectRow (*index);
	if (makeRowVisible)
		dataBrowser->makeRowVisible (dataBrowser->getSelectedRow ());

	if (next)
	{
		next->remove ();
		next = nullptr;
	}
	if (auto container = selectedView ? selectedView->asViewContainer () : nullptr)
	{
		UIViewListDataSource* dataSource = new UIViewListDataSource (container, viewFactory, selection, undoManager, delegate);
		UIEditController::setupDataSource (dataSource);
		CRect r (dataBrowser->getViewSize ());
		r.offset (r.getWidth (), 0);
		CDataBrowser* newDataBrowser = new CDataBrowser (r, dataSource);
		UITemplateController::setupDataBrowser (dataBrowser, newDataBrowser);
		CViewContainer* parentView = static_cast<CViewContainer*>(dataBrowser->getParentView ());
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
	return true;
}

//----------------------------------------------------------------------------------------------------
void UIViewListDataSource::dbSelectionChanged (CDataBrowser* browser)
{
	CView* subview = getSubview (browser->getSelectedRow ());
	if (subview == selectedView || inUpdate)
		return;
	setSelectedView (subview);
	GenericStringListDataBrowserSource::dbSelectionChanged (dataBrowser);
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
	if (buttons.isLeftButton ())
	{
		if (buttons.isDoubleClick ())
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
		dragStartMouseObserver.init (where);
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult UIViewListDataSource::dbOnMouseMoved (const CPoint& where,
                                                        const CButtonState& buttons, int32_t row,
                                                        int32_t column, CDataBrowser* browser)
{
	if (row >= 0 && buttons.isLeftButton () && dragStartMouseObserver.shouldStartDrag (where))
	{
		row = browser->getSelectedRow ();
		dragRow = row;

		auto cellBounds = browser->getCellBounds ({row, column});
		auto offscreen = COffscreenContext::create (cellBounds.getSize (),
		                                            browser->getFrame ()->getScaleFactor ());
		auto offscreenSize = cellBounds;
		offscreenSize.originize ();
		offscreen->beginDraw ();
		dbDrawCell (offscreen, offscreenSize, row, column, 0, browser);
		offscreen->endDraw ();
		
		auto startPos = dragStartMouseObserver.getInitPosition ();
		DragDescription dd (CDropSource::create (&row, sizeof (int32_t), IDataPackage::kBinary),
		                    {cellBounds.left - startPos.x, cellBounds.top - startPos.y},
		                    shared (offscreen->getBitmap ()));
		auto callbackFunc = makeOwned<DragCallbackFunctions> ();
		auto Self = shared (this);
		callbackFunc->endedFunc = [Self] (IDraggingSession*, CPoint, DragOperation) {
			Self->dragRow = -1;
			Self->dragDestinationRow = -1;
		};

		browser->doDrag (dd, callbackFunc);
		return kMouseMoveEventHandledButDontNeedMoreEvents;
	}
	return kMouseEventHandled;
}

//----------------------------------------------------------------------------------------------------
DragOperation UIViewListDataSource::dbOnDragEnterCell (int32_t row, int32_t column,
                                                       const CPoint& where, IDataPackage* drag,
                                                       CDataBrowser* browser)
{
	if (dragRow >= 0)
	{
		if (dragDestinationRow >= 0)
			browser->invalidateRow (dragDestinationRow);
		if (dragRow == row)
		{
			dragDestinationRow = -1;
		}
		else
		{
			dragDestinationRow = row;
			browser->invalidateRow (dragDestinationRow);
		}
		return DragOperation::Move;
	}
	return DragOperation::None;
}

//----------------------------------------------------------------------------------------------------
DragOperation UIViewListDataSource::dbOnDragMoveInCell (int32_t row, int32_t column,
                                                        const CPoint& where, IDataPackage* drag,
                                                        CDataBrowser* browser)
{
	if (dragRow >= 0)
		return DragOperation::Move;
	return DragOperation::None;
}

//----------------------------------------------------------------------------------------------------
void UIViewListDataSource::dbOnDragExitCell (int32_t row, int32_t column, IDataPackage* drag,
                                             CDataBrowser* browser)
{
	if (dragDestinationRow >= 0)
	{
		browser->invalidateRow (dragDestinationRow);
		dragDestinationRow = -1;
	}
}

//----------------------------------------------------------------------------------------------------
bool UIViewListDataSource::dbOnDropInCell (int32_t row, int32_t column, const CPoint& where,
                                           IDataPackage* drag, CDataBrowser* browser)
{
	bool result = false;
	if (row != dragRow && dragDestinationRow != -1 && row != -1)
	{
		int32_t dir = dragDestinationRow - dragRow;
		undoManager->pushAndPerform (new HierarchyMoveViewOperation (subviews[dragRow], selection, dir));
		result = true;
	}
	dragRow = dragDestinationRow = -1;
	browser->invalidateRow (row);
	return result;
}

//----------------------------------------------------------------------------------------------------
void UIViewListDataSource::onUndoManagerChange ()
{
	update (view);
	if (selectedView)
	{
		if (dataBrowser)
		{
			if (auto index = indexOf (subviews.begin (), subviews.end (), selectedView))
			{
				dataBrowser->setSelectedRow (*index, true);
				return;
			}
		}
		selectedView = nullptr;
		if (next)
		{
			next->remove ();
			next = nullptr;
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIViewListDataSource::dbOnKeyboardEvent (KeyboardEvent& event, CDataBrowser* browser)
{
	if (event.type == EventType::KeyDown)
	{
		if (event.virt == VirtualKey::Return)
		{
			int32_t row = browser->getSelectedRow ();
			CView* subview = getSubview (row);
			if (subview)
			{
				selection->setExclusive (subview);
				event.consumed = true;
				return;
			}
		}
	}
	UINavigationDataSource::dbOnKeyboardEvent (event, browser);
}

//----------------------------------------------------------------------------------------------------
void UIViewListDataSource::dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser)
{
	drawRowBackground (context, size, row, flags, browser);
	auto subview = getSubview (row);
	if (subview && subview->asViewContainer ())
		drawTriangle (context, size);
	drawRowString (context, size, row, flags, browser);
	if (dragDestinationRow == row)
	{
		CColor color = kRedCColor;
		UIEditController::getEditorDescription ()->getColor ("db.drag.indicator", color);
		context->setFrameColor (color);
		context->setLineWidth (1.);
		auto r = size;
		r.top += 1.;
		r.bottom -= 2.;
		if (dragDestinationRow < dragRow)
			context->drawLine (r.getTopLeft (), r.getTopRight ());
		else
			context->drawLine (r.getBottomLeft (), r.getBottomRight ());
	}
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UITemplatesDataSource::UITemplatesDataSource (GenericStringListDataBrowserSourceSelectionChanged* delegate, UIDescription* description, IActionPerformer* actionPerformer, const std::string* templateName)
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
void UITemplatesDataSource::dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser)
{
	drawRowBackground (context, size, row, flags, browser);
	drawTriangle (context, size);
	drawRowString (context, size, row, flags, browser);
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

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
