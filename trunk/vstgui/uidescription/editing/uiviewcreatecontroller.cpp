#include "uiviewcreatecontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uieditcontroller.h"
#include "uisearchtextfield.h"
#include "uibasedatasource.h"
#include "../uiviewfactory.h"
#include "../../lib/cdropsource.h"
#include "uiselection.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIViewCreatorDataSource : public UIBaseDataSource
{
public:
	UIViewCreatorDataSource (UIViewFactory* factory, UIDescription* description);

	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser);
	CMouseEventResult dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser);

	virtual void getNames (std::list<const std::string*>& names);
	virtual bool addItem (UTF8StringPtr name) { return false; }
	virtual bool removeItem (UTF8StringPtr name) { return false; }
	virtual bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) { return false; }
	virtual UTF8StringPtr getDefaultsName () { return "UIViewCreatorDataSource"; }
protected:
	UIViewFactory* factory;
	int32_t mouseDownRow;
};

//----------------------------------------------------------------------------------------------------
UIViewCreatorController::UIViewCreatorController (IController* baseController, UIDescription* description)
: DelegationController (baseController)
, description (description)
, dataSource (0)
{
}

//----------------------------------------------------------------------------------------------------
UIViewCreatorController::~UIViewCreatorController ()
{
	if (dataSource)
		dataSource->forget ();
}

//----------------------------------------------------------------------------------------------------
CView* UIViewCreatorController::createView (const UIAttributes& attributes, IUIDescription* _description)
{
	const std::string* name = attributes.getAttributeValue ("custom-view-name");
	if (name)
	{
		if (*name == "ViewDataBrowser")
		{
			UIViewFactory* factory = dynamic_cast<UIViewFactory*> (description->getViewFactory ());
			dataSource = new UIViewCreatorDataSource (factory, description);
			UIEditController::setupDataSource (dataSource);
			CDataBrowser* dataBrowser = new CDataBrowser (CRect (0, 0, 0, 0), dataSource, CDataBrowser::kDrawRowLines|CScrollView::kHorizontalScrollbar | CScrollView::kVerticalScrollbar);
			return dataBrowser;
		}
	}
	return DelegationController::createView (attributes, _description);
}

//----------------------------------------------------------------------------------------------------
CView* UIViewCreatorController::verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
{
	UISearchTextField* searchField = dynamic_cast<UISearchTextField*>(view);
	if (searchField && searchField->getTag () == kSearchFieldTag)
	{
		dataSource->setSearchFieldControl (searchField);
	}
	return DelegationController::verifyView (view, attributes, description);
}

//----------------------------------------------------------------------------------------------------
CControlListener* UIViewCreatorController::getControlListener (UTF8StringPtr name)
{
	if (strcmp (name, "viewcreator.search") == 0)
		return dataSource;
	return this;
}

//----------------------------------------------------------------------------------------------------
void UIViewCreatorController::valueChanged (CControl* control)
{
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIViewCreatorDataSource::UIViewCreatorDataSource (UIViewFactory* factory, UIDescription* description)
: UIBaseDataSource (description, 0, 0)
, factory (factory)
{
}

//----------------------------------------------------------------------------------------------------
void UIViewCreatorDataSource::getNames (std::list<const std::string*>& names)
{
	factory->collectRegisteredViewNames (names);
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIViewCreatorDataSource::dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
{
	if (buttons.isLeftButton ())
	{
		mouseDownRow = row;
		return kMouseEventHandled;
	}
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIViewCreatorDataSource::dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
{
	if (mouseDownRow >= 0 && buttons.isLeftButton ())
	{
		UIAttributes viewAttr;
		viewAttr.setAttribute ("class", getStringList ()->at (mouseDownRow).c_str ());
		CView* view = factory->createView (viewAttr, description);
		if (view)
		{
			if (view->getViewSize ().isEmpty ())
			{
				CRect size (CPoint (0, 0), CPoint (20, 20));
				view->setViewSize (size);
				view->setMouseableArea (size);
			}
			UISelection selection;
			selection.add (view);
			CMemoryStream stream (1024, 1024, false);
			if (selection.store (stream, factory, description))
			{
				stream.end ();
				CDropSource* dropSource = new CDropSource (stream.getBuffer (), (int32_t)stream.tell (), CDropSource::kText);
				browser->doDrag (dropSource);
				dropSource->forget ();
			}
			view->forget ();
		}
		mouseDownRow = -1;
	}
	return kMouseEventNotHandled;
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
