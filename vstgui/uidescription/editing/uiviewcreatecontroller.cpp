//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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
