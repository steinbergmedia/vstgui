//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "uiviewcreatecontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uieditcontroller.h"
#include "uisearchtextfield.h"
#include "uibasedatasource.h"
#include "../uiviewfactory.h"
#include "../../lib/cdropsource.h"
#include "../../lib/controls/coptionmenu.h"
#include "uiselection.h"
#include "../detail/uiviewcreatorattributes.h"

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIViewCreatorDataSource : public UIBaseDataSource
{
public:
	UIViewCreatorDataSource (const UIViewFactory* factory, UIDescription* description);

	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;

	void getNames (std::list<const std::string*>& names) VSTGUI_OVERRIDE_VMETHOD;
	bool addItem (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD { return false; }
	bool removeItem (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD { return false; }
	bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) VSTGUI_OVERRIDE_VMETHOD { return false; }
	UTF8StringPtr getDefaultsName () VSTGUI_OVERRIDE_VMETHOD { return "UIViewCreatorDataSource"; }
protected:
	void addViewToCurrentEditView ();
	SharedPointer<UISelection> createSelection ();
	const UIViewFactory* factory;
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
CView* UIViewCreatorController::createView (const UIAttributes& attributes, const IUIDescription* _description)
{
	const std::string* name = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (name)
	{
		if (*name == "ViewDataBrowser")
		{
			const UIViewFactory* factory = dynamic_cast<const UIViewFactory*> (description->getViewFactory ());
			dataSource = new UIViewCreatorDataSource (factory, description);
			UIEditController::setupDataSource (dataSource);
			CDataBrowser* dataBrowser = new CDataBrowser (CRect (0, 0, 0, 0), dataSource, CDataBrowser::kDrawRowLines|CScrollView::kHorizontalScrollbar | CScrollView::kVerticalScrollbar);
			return dataBrowser;
		}
	}
	return DelegationController::createView (attributes, _description);
}

//----------------------------------------------------------------------------------------------------
CView* UIViewCreatorController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	UISearchTextField* searchField = dynamic_cast<UISearchTextField*>(view);
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
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIViewCreatorDataSource::UIViewCreatorDataSource (const UIViewFactory* factory, UIDescription* description)
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
void UIViewCreatorDataSource::addViewToCurrentEditView ()
{
	UIViewCreatorController* controller = dynamic_cast<UIViewCreatorController*> (getViewController (dataBrowser, true));
	if (controller)
	{
		if (UIEditController* editController = dynamic_cast<UIEditController*>(controller->getBaseController ()))
		{
			SharedPointer<UISelection> selection = createSelection ();
			editController->addSelectionToCurrentView (selection);
		}
	}
}

//----------------------------------------------------------------------------------------------------
SharedPointer<UISelection> UIViewCreatorDataSource::createSelection ()
{
	SharedPointer<UISelection> selection;
	UIAttributes viewAttr;
	viewAttr.setAttribute (UIViewCreator::kAttrClass, getStringList ()->at (static_cast<uint32_t> (mouseDownRow)));
	CView* view = factory->createView (viewAttr, description);
	if (view)
	{
		if (view->getViewSize ().isEmpty ())
		{
			CRect size (CPoint (0, 0), CPoint (20, 20));
			view->setViewSize (size);
			view->setMouseableArea (size);
		}
		selection = owned (new UISelection ());
		selection->add (view);
		view->forget ();
	}
	return selection;
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIViewCreatorDataSource::dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
{
	mouseDownRow = row;
	if (buttons.isLeftButton ())
	{
		if (!buttons.isDoubleClick ())
			return kMouseEventHandled;
		addViewToCurrentEditView ();
	}
	else if (buttons.isRightButton ())
	{
		const std::string& viewName = getStringList ()->at (static_cast<uint32_t> (mouseDownRow));
		std::string menuEntryName = "Add a new '" + viewName + "'";
		COptionMenu menu;
		menu.setStyle (kPopupStyle);
		menu.addEntry (menuEntryName.c_str ());
		CPoint menuLocation (where);
		browser->localToFrame (menuLocation);
		if (menu.popup (browser->getFrame (), menuLocation))
		{
			if (menu.getEntry (menu.getLastResult ()))
				addViewToCurrentEditView ();
		}
	}
	mouseDownRow = -1;
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIViewCreatorDataSource::dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
{
	if (mouseDownRow >= 0 && buttons.isLeftButton ())
	{
		SharedPointer<UISelection> selection = createSelection ();
		CMemoryStream stream (1024, 1024, false);
		if (selection->store (stream, description))
		{
			stream.end ();
			CDropSource* dropSource = new CDropSource (stream.getBuffer (), static_cast<uint32_t> (stream.tell ()), CDropSource::kText);
			browser->doDrag (dropSource);
			dropSource->forget ();
		}
		mouseDownRow = -1;
	}
	return kMouseEventNotHandled;
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
