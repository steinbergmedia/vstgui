// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uicolorscontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uicolor.h"
#include "uicolorchoosercontroller.h"
#include "uieditcontroller.h"
#include "uibasedatasource.h"
#include "../../lib/cdropsource.h"
#include "../../lib/coffscreencontext.h"
#include "../../lib/dragging.h"
#include "../../lib/optional.h"
#include "../../lib/idatapackage.h"
#include "../../lib/controls/csearchtextedit.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIColorsDataSource : public UIBaseDataSource, public UIColorListenerAdapter
{
public:
	UIColorsDataSource (UIDescription* description, IActionPerformer* actionPerformer, UIColor* color);
	~UIColorsDataSource () override;
	
protected:
	void onUIDescColorChanged (UIDescription* desc) override;
	void update () override;
	void getNames (std::list<const std::string*>& names) override;
	bool addItem (UTF8StringPtr name) override;
	bool removeItem (UTF8StringPtr name) override;
	bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) override;
	UTF8StringPtr getDefaultsName () override { return "UIColorsDataSource"; }

	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser) override;
	void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* control, CDataBrowser* browser) override;
	void dbSelectionChanged (CDataBrowser* browser) override;

	void dbOnDragEnterBrowser (IDataPackage* drag, CDataBrowser* browser) override;
	void dbOnDragExitBrowser (IDataPackage* drag, CDataBrowser* browser) override;
	DragOperation dbOnDragEnterCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser) override;
	DragOperation dbOnDragMoveInCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser) override;
	void dbOnDragExitCell (int32_t row, int32_t column, IDataPackage* drag, CDataBrowser* browser) override;
	bool dbOnDropInCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser) override;
	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) override;
	CMouseEventResult dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) override;

	CCoord getColorIconWith ();

	void uiColorChanged (UIColor* c) override;
	void uiColorBeginEditing (UIColor* c) override;
	void uiColorEndEditing (UIColor* c) override;

	SharedPointer<UIColor> color;
	bool editing;
	Optional<CColor> dragColor;
	int32_t dragRow;
	DragStartMouseObserver dragStartMouseObserver;
	bool allowDrag {false};
};

//----------------------------------------------------------------------------------------------------
UIColorsDataSource::UIColorsDataSource (UIDescription* description, IActionPerformer* actionPerformer, UIColor* color)
: UIBaseDataSource (description, actionPerformer)
, color (color)
, editing (false)
, dragRow (-1)
{
	color->registerListener (this);
}

//----------------------------------------------------------------------------------------------------
UIColorsDataSource::~UIColorsDataSource ()
{
	color->unregisterListener (this);
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::onUIDescColorChanged (UIDescription* desc)
{
	onUIDescriptionUpdate ();
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::uiColorChanged (UIColor* c)
{
	if (editing)
	{
		int32_t selectedRow = dataBrowser->getSelectedRow ();
		if (selectedRow != CDataBrowser::kNoSelection)
		{
			actionPerformer->performLiveColorChange (
			    names.at (static_cast<uint32_t> (selectedRow)).data (), color->base ());
			dataBrowser->setSelectedRow (selectedRow);
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::uiColorBeginEditing (UIColor* c)
{
	int32_t selectedRow = dataBrowser->getSelectedRow ();
	if (selectedRow != CDataBrowser::kNoSelection)
	{
		actionPerformer->beginLiveColorChange (
		    names.at (static_cast<uint32_t> (selectedRow)).data ());
		editing = true;
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::uiColorEndEditing (UIColor* c)
{
	int32_t selectedRow = dataBrowser->getSelectedRow ();
	if (selectedRow != CDataBrowser::kNoSelection)
	{
		actionPerformer->endLiveColorChange (
		    names.at (static_cast<uint32_t> (selectedRow)).data ());
		editing = false;
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::getNames (std::list<const std::string*>& names)
{
	description->collectColorNames (names);
}

//----------------------------------------------------------------------------------------------------
bool UIColorsDataSource::addItem (UTF8StringPtr name)
{
	actionPerformer->performColorChange (name, kWhiteCColor);
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIColorsDataSource::removeItem (UTF8StringPtr name)
{
	actionPerformer->performColorChange (name, kWhiteCColor, true);
	return true;
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::update ()
{
	UIBaseDataSource::update ();
	if (dataBrowser)
	{
		dbSelectionChanged (dataBrowser);
		dataBrowser->invalid ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::dbSelectionChanged (CDataBrowser* browser)
{
	int32_t selectedRow = dataBrowser->getSelectedRow ();
	if (selectedRow != CDataBrowser::kNoSelection)
	{
		CColor c;
		if (description->getColor (names.at (static_cast<uint32_t> (selectedRow)).data (), c))
		{
			if (c != color->base ())
			{
				*color = c;
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------
CCoord UIColorsDataSource::getColorIconWith ()
{
	return dataBrowser ? dbGetRowHeight (dataBrowser) : 0.;
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser)
{
	GenericStringListDataBrowserSource::drawRowBackground (context, size, row, flags, browser);
	CRect r (size);
	r.right -= getColorIconWith ();
	GenericStringListDataBrowserSource::drawRowString (context, r, row, flags, browser);
	CColor cellColor;
	if (description->getColor (names.at (static_cast<uint32_t> (row)).data (), cellColor))
	{
		context->setFillColor (cellColor);
		context->setFrameColor (dragRow == row ? kRedCColor : kBlackCColor);
		context->setLineWidth (context->getHairlineSize ());
		context->setLineStyle (kLineSolid);
		context->setDrawMode (kAliasing);
		r = size;
		r.left = r.right - getColorIconWith ();
		r.inset (2, 2);
		context->drawRect (r, kDrawFilledAndStroked);
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* control, CDataBrowser* browser)
{
	UIBaseDataSource::dbCellSetupTextEdit(row, column, control, browser);
	CRect r (control->getViewSize ());
	r.right -= getColorIconWith ();
	control->setViewSize (r);
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIColorsDataSource::dbOnMouseDown (const CPoint& where,
                                                     const CButtonState& buttons, int32_t row,
                                                     int32_t column, CDataBrowser* browser)
{
	auto r = browser->getCellBounds ({row, column});
	r.left = r.right - getColorIconWith ();
	r.inset (2, 2);
	if (r.pointInside (where))
	{
		allowDrag = true;
		dragStartMouseObserver.init (where);
	}
	UIBaseDataSource::dbOnMouseDown (where, buttons, row, column, browser);
	return kMouseEventHandled;
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIColorsDataSource::dbOnMouseMoved (const CPoint& where,
                                                      const CButtonState& buttons, int32_t row,
                                                      int32_t column, CDataBrowser* browser)
{
	auto r = browser->getCellBounds ({row, column});
	r.left = r.right - getColorIconWith ();
	r.inset (2, 2);
	if (allowDrag)
	{
		if (buttons.isLeftButton ())
		{
			if (dragStartMouseObserver.shouldStartDrag (where))
			{
				CColor cellColor;
				if (description->getColor (names.at (static_cast<uint32_t> (row)).data (), cellColor))
				{
					auto colorStr = cellColor.toString ();
					auto dropSource = CDropSource::create (
						colorStr.data (), static_cast<uint32_t> (colorStr.length () + 1),
						CDropSource::kText);
					SharedPointer<CBitmap> dragBitmap;
					if (auto offscreen = COffscreenContext::create (r.getSize ()))
					{
						offscreen->beginDraw ();
						offscreen->setFillColor (cellColor);
						offscreen->drawRect (CRect (0, 0, r.getWidth(), r.getHeight()), kDrawFilled);
						offscreen->endDraw();
						dragBitmap = offscreen->getBitmap ();
					}
					
					auto df = makeOwned<DragCallbackFunctions> ();
					df->endedFunc = [browser] (IDraggingSession*, CPoint, DragOperation) {
						browser->getFrame ()->setCursor (kCursorDefault);
					};
					browser->doDrag (DragDescription (dropSource, -r.getSize () / 2., dragBitmap), df);
				}
			}
			return kMouseEventHandled;
		}
	}
	if (r.pointInside (where))
		browser->getFrame ()->setCursor (kCursorHand);
	else
		browser->getFrame ()->setCursor (kCursorDefault);
	return UIBaseDataSource::dbOnMouseMoved (where, buttons, row, column, browser);
}

//----------------------------------------------------------------------------------------------------
bool UIColorsDataSource::performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	actionPerformer->performColorNameChange (oldName, newName);
	return true;
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::dbOnDragEnterBrowser (IDataPackage* drag, CDataBrowser* browser)
{
	IDataPackage::Type type;
	const void* item;
	if (drag->getData (0, item, type) > 0 && type == IDataPackage::kText)
	{
		if (CColor::isColorRepresentation (static_cast<UTF8StringPtr> (item)))
		{
			CColor c;
			c.fromString (static_cast<UTF8StringPtr> (item));
			dragColor = Optional<CColor> (c);
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::dbOnDragExitBrowser (IDataPackage* drag, CDataBrowser* browser)
{
	dragColor = {};
	dragRow = -1;
}

//----------------------------------------------------------------------------------------------------
DragOperation UIColorsDataSource::dbOnDragEnterCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser)
{
	if (dragColor && row >= 0)
	{
		CColor cellColor;
		if (description->getColor (names.at (static_cast<uint32_t> (row)).data (), cellColor) &&
		    cellColor != *dragColor)
		{
			dragRow = row;
			browser->invalidateRow (dragRow);
			return DragOperation::Copy;
		}
		else
			dragRow = -1;
	}
	return DragOperation::None;
}

//----------------------------------------------------------------------------------------------------
DragOperation UIColorsDataSource::dbOnDragMoveInCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser)
{
	return (dragColor && (dragRow >= 0 || row == -1)) ? DragOperation::Copy : DragOperation::None;
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::dbOnDragExitCell (int32_t row, int32_t column, IDataPackage* drag, CDataBrowser* browser)
{
	if (dragColor)
	{
		if (dragRow >= 0)
			browser->invalidateRow (dragRow);
		dragRow = -1;
	}
}

//----------------------------------------------------------------------------------------------------
bool UIColorsDataSource::dbOnDropInCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser)
{
	if (dragColor)
	{
		if (row >= 0)
		{
			CColor cellColor;
			if (description->getColor (names.at (static_cast<uint32_t> (row)).data (), cellColor))
			{
				if (cellColor != *dragColor)
				{
					actionPerformer->performColorChange (names[static_cast<uint32_t> (row)].data (), *dragColor);
					selectName (names[static_cast<uint32_t> (row)].data ());
				}
			}
		}
		else
		{
			std::string newName (filterString.empty () ? "New" : filterString);
			if (createUniqueName (newName))
			{
				actionPerformer->performColorChange (newName.data (), *dragColor);
				selectName (newName.data ());
			}
		}
		dragColor = {};
		dragRow = -1;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIColorsController::UIColorsController (IController* baseController, UIDescription* description, IActionPerformer* actionPerformer)
: DelegationController (baseController)
, editDescription (description)
, actionPerformer (actionPerformer)
, dataSource (nullptr)
, color (makeOwned<UIColor> ())
{
	dataSource = new UIColorsDataSource (editDescription, actionPerformer, color);
	UIEditController::setupDataSource (dataSource);
}

//----------------------------------------------------------------------------------------------------
UIColorsController::~UIColorsController ()
{
	dataSource->forget ();
}

//----------------------------------------------------------------------------------------------------
CView* UIColorsController::createView (const UIAttributes& attributes, const IUIDescription* description)
{
	const std::string* name = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (name)
	{
		if (*name == "ColorsBrowser")
		{
			CDataBrowser* dataBrowser = new CDataBrowser (CRect (0, 0, 0, 0), dataSource, CDataBrowser::kDrawRowLines|CScrollView::kHorizontalScrollbar|CScrollView::kVerticalScrollbar);
			return dataBrowser;
		}
	}
	return DelegationController::createView (attributes, description);
}

//----------------------------------------------------------------------------------------------------
CView* UIColorsController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
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
IControlListener* UIColorsController::getControlListener (UTF8StringPtr name)
{
	return this;
}

//----------------------------------------------------------------------------------------------------
void UIColorsController::valueChanged (CControl* pControl)
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
	}
}

//----------------------------------------------------------------------------------------------------
IController* UIColorsController::createSubController (IdStringPtr name, const IUIDescription* description)
{
	if (std::strcmp (name, "ColorChooserController") == 0)
		return new UIColorChooserController (this, color);
	return controller->createSubController (name, description);
}

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
