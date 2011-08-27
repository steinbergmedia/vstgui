#include "uicolorscontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uieditcontroller.h"
#include "uisearchtextfield.h"
#include "uibasedatasource.h"
#include "../../lib/controls/ccolorchooser.h"
#include <sstream>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIColorsDataSource : public UIBaseDataSource, public IColorChooserDelegate
{
public:
	UIColorsDataSource (UIDescription* description, IActionPerformer* actionPerformer);
	
	void setColorChooser (CColorChooser* chooser) { colorChooser = chooser; }
protected:
	virtual void getNames (std::list<const std::string*>& names);
	virtual bool addItem (UTF8StringPtr name);
	virtual bool removeItem (UTF8StringPtr name);
	virtual bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName);
	virtual UTF8StringPtr getDefaultsName () { return "UIColorsDataSource"; }

	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser);
	void dbSelectionChanged (CDataBrowser* browser);

	void colorChanged (CColorChooser* chooser, const CColor& color);

	SharedPointer<CColorChooser> colorChooser;
};

//----------------------------------------------------------------------------------------------------
UIColorsDataSource::UIColorsDataSource (UIDescription* description, IActionPerformer* actionPerformer)
: UIBaseDataSource (description, actionPerformer, UIDescription::kMessageColorChanged)
{
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
void UIColorsDataSource::dbSelectionChanged (CDataBrowser* browser)
{
	int32_t selectedRow = dataBrowser->getSelectedRow ();
	if (colorChooser && selectedRow != CDataBrowser::kNoSelection)
	{
		CColor color;
		if (description->getColor(names.at (selectedRow).c_str (), color))
		{
			colorChooser->setColor (color);
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::colorChanged (CColorChooser* chooser, const CColor& color)
{
	int32_t selectedRow = dataBrowser->getSelectedRow ();
	if (selectedRow != CDataBrowser::kNoSelection)
	{
		actionPerformer->performColorChange (names.at (selectedRow).c_str (), color);
		dataBrowser->setSelectedRow (selectedRow);
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser)
{
	CColor color;
	if (description->getColor(names.at (row).c_str (), color))
	{
		context->setFillColor (color);
		CRect r (size);
		r.left = r.right - r.getHeight ();
		context->drawRect (r, kDrawFilled);
	}
	GenericStringListDataBrowserSource::dbDrawCell (context, size, row, column, flags, browser);
}

//----------------------------------------------------------------------------------------------------
bool UIColorsDataSource::performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	actionPerformer->performColorNameChange (oldName, newName);
	return true;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIColorsController::UIColorsController (IController* baseController, UIDescription* description, IActionPerformer* actionPerformer)
: DelegationController (baseController)
, editDescription (description)
, actionPerformer (actionPerformer)
, dataSource (0)
{
	dataSource = new UIColorsDataSource (editDescription, actionPerformer);
	UIEditController::setupDataSource (dataSource);
}

//----------------------------------------------------------------------------------------------------
UIColorsController::~UIColorsController ()
{
	dataSource->forget ();
}

//----------------------------------------------------------------------------------------------------
CView* UIColorsController::createView (const UIAttributes& attributes, IUIDescription* description)
{
	const std::string* name = attributes.getAttributeValue ("custom-view-name");
	if (name)
	{
		if (*name == "ColorsBrowser")
		{
			CDataBrowser* dataBrowser = new CDataBrowser (CRect (0, 0, 0, 0), 0, dataSource, CDataBrowser::kDrawRowLines|CScrollView::kHorizontalScrollbar | CScrollView::kVerticalScrollbar);
			return dataBrowser;
		}
		else if (*name == "ColorChooser")
		{
			CColorChooserUISettings ui;
			ui.font = description->getFont("colorchooser.font");
			ui.fontColor = kBlackCColor;
			ui.margin = CPoint (2, 2);
			ui.checkerBoardBack = true;
			ui.checkerBoardColor1 = kWhiteCColor;
			ui.checkerBoardColor2 = kGreyCColor;
			CColorChooser* colorChooser = new CColorChooser (dataSource, kWhiteCColor, ui);
			dataSource->setColorChooser (colorChooser);
			return colorChooser;
		}
	}
	return DelegationController::createView (attributes, description);
}

//----------------------------------------------------------------------------------------------------
CView* UIColorsController::verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
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
CControlListener* UIColorsController::getControlListener (UTF8StringPtr name)
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


} // namespace

#endif // VSTGUI_LIVE_EDITING
