// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uigridcontroller.h"

#if VSTGUI_LIVE_EDITING

#include "../../lib/algorithm.h"
#include "../../lib/controls/coptionmenu.h"
#include "../../lib/controls/cstringlist.h"
#include "../../lib/platform/platformfactory.h"
#include "../uiattributes.h"
#include "uieditcontroller.h"
#include <array>
#include <sstream>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
UIGridController::UIGridController (IController* baseController, UIDescription* description)
: DelegationController (baseController), editDescription (description)
{
	auto attributes = editDescription->getCustomAttributes ("UIGridController", true);
	if (attributes)
		attributes->getPointAttribute ("Size", size);
	loadDefGrids ();
	if (defGrids.empty ())
	{
		defGrids = {{1., 1.}, {5., 5.}, {10., 10.}, {12., 12.}, {15., 15.}};
		auto it = std::find (defGrids.begin (), defGrids.end (), getSize ());
		if (it == defGrids.end ())
			defGrids.emplace_back (getSize ());
		saveDefGrids ();
	}
}

//----------------------------------------------------------------------------------------------------
UIGridController::~UIGridController ()
{
	saveDefGrids ();
}

//----------------------------------------------------------------------------------------------------
void UIGridController::setSize (const CPoint& p)
{
	UIGrid::setSize (p);
	if (auto attributes = editDescription->getCustomAttributes ("UIGridController", true))
		attributes->setPointAttribute ("Size", size);
}

//----------------------------------------------------------------------------------------------------
void UIGridController::valueChanged (CControl* control)
{
	switch (control->getTag ())
	{
		case kGridAddTag:
		{
			if (control->getValue () == control->getMin ())
				break;
			defGrids.push_back ({2., 2.});
			gridList->setMax (static_cast<float> (defGrids.size () - 1));
			gridList->setValue (gridList->getMax ());
			gridList->valueChanged ();
			break;
		}
		case kGridRemoveTag:
		{
			if (control->getValue () == control->getMin ())
				break;
			auto index = gridList->getIntValue ();
			if (index > 0)
			{
				auto it = defGrids.begin ();
				std::advance (it, index);
				defGrids.erase (it);
				gridList->setMax (static_cast<float> (defGrids.size () - 1));
			}
			break;
		}
		case kGridXTag:
		{
			auto index = gridList->getIntValue ();
			if (index > 0)
			{
				defGrids[index].x = control->getValue ();
				gridList->invalidRow (index);
			}
			break;
		}
		case kGridYTag:
		{
			auto index = gridList->getIntValue ();
			if (index > 0)
			{
				defGrids[index].y = control->getValue ();
				gridList->invalidRow (index);
			}
			break;
		}
		case kGridListTag:
		{
			auto index = gridList->getIntValue ();
			if (gridXEdit)
				gridXEdit->setValue (static_cast<float> (defGrids[index].x));
			if (gridYEdit)
				gridYEdit->setValue (static_cast<float> (defGrids[index].y));
			break;
		}
	}
}

//----------------------------------------------------------------------------------------------------
CView* UIGridController::verifyView (CView* view, const UIAttributes& attributes,
                                     const IUIDescription* description)
{
	if (auto menu = dynamic_cast<COptionMenu*> (view))
	{
		if (menu->getTag () == kGridMenuTag)
		{
			gridMenu = menu;
			setupMenu ();
		}
	}
	else if (auto listControl = dynamic_cast<CListControl*> (view))
	{
		if (listControl->getTag () == kGridListTag)
		{
			if (auto drawer = dynamic_cast<StringListControlDrawer*> (listControl->getDrawer ()))
			{
				drawer->setStringProvider ([this] (int32_t row) {
					return getPlatformFactory ().createString (
					    pointToDisplayString (defGrids[row]));
				});
			}
			gridList = listControl;
			gridList->setMax (static_cast<float> (defGrids.size () - 1));
		}
	}
	else if (auto textEdit = dynamic_cast<CTextEdit*> (view))
	{
		if (textEdit->getTag () == kGridXTag)
		{
			gridXEdit = textEdit;
			setupTextEdit (textEdit);
		}
		else if (textEdit->getTag () == kGridYTag)
		{
			gridYEdit = textEdit;
			setupTextEdit (textEdit);
		}
	}
	return DelegationController::verifyView (view, attributes, description);
}

//----------------------------------------------------------------------------------------------------
IControlListener* UIGridController::getControlListener (UTF8StringPtr name)
{
	return this;
}

//----------------------------------------------------------------------------------------------------
void UIGridController::onDialogButton1Clicked (UIDialogController*)
{
	gridList = nullptr;
	gridXEdit = nullptr;
	gridYEdit = nullptr;
	setupMenu ();
	saveDefGrids ();
}

//----------------------------------------------------------------------------------------------------
void UIGridController::onDialogButton2Clicked (UIDialogController*)
{
}

//----------------------------------------------------------------------------------------------------
void UIGridController::onDialogShow (UIDialogController*)
{
	if (gridList)
		gridList->valueChanged ();
}

//----------------------------------------------------------------------------------------------------
void UIGridController::loadDefGrids ()
{
	auto attributes = editDescription->getCustomAttributes ("UIGridController", true);
	if (!attributes)
		return;
	UIAttributes::StringArray strings;
	if (!attributes->getStringArrayAttribute ("Grids", strings))
		return;
	for (auto& str : strings)
	{
		auto pos = str.find_first_of ('x');
		if (pos == std::string::npos)
			continue;
		str[pos] = ',';
		CPoint p;
		if (UIAttributes::stringToPoint (str, p))
			defGrids.emplace_back (p);
	}
}

//----------------------------------------------------------------------------------------------------
void UIGridController::saveDefGrids ()
{
	if (defGrids.empty ())
		return;
	auto attributes = editDescription->getCustomAttributes ("UIGridController", true);
	if (!attributes)
		return;
	UIAttributes::StringArray strings;
	for (auto p : defGrids)
	{
		std::string str = UIAttributes::pointToString (p);
		auto pos = str.find_first_of (',');
		if (pos == std::string::npos)
			continue;
		str[pos] = 'x';
		strings.emplace_back (str);
	}
	attributes->setStringArrayAttribute ("Grids", strings);
}

//----------------------------------------------------------------------------------------------------
UTF8String UIGridController::pointToDisplayString (const CPoint& p) const
{
	if (p == CPoint (1., 1.))
		return "Off";
	auto str = toString (static_cast<int32_t> (p.x));
	str += "x";
	str += toString (static_cast<int32_t> (p.y));
	return str;
}

//----------------------------------------------------------------------------------------------------
void UIGridController::setupMenu ()
{
	std::sort (defGrids.begin (), defGrids.end (),
	           [] (const CPoint& lhs, const CPoint& rhs) { return lhs.x * lhs.y < rhs.x * rhs.y; });
	gridMenu->removeAllEntry ();
	for (auto& p : defGrids)
	{
		auto item = new CCommandMenuItem (CCommandMenuItem::Desc {pointToDisplayString (p)});
		gridMenu->addEntry (item);
		item->setActions ([this, p] (CCommandMenuItem*) { setSize (p); });
	}
	gridMenu->addSeparator ();
	auto item = new CCommandMenuItem (CCommandMenuItem::Desc {"Setup..."});
	gridMenu->addEntry (item);
	item->setActions ([this] (CCommandMenuItem*) {
		syncMenuValueAndSize ();
		auto dc = new UIDialogController (this, gridMenu->getFrame ());
		dc->run ("grid.dialog", "Grid Setup", "Close", nullptr, this,
		         UIEditController::getEditorDescription ());
	});
	syncMenuValueAndSize ();
}

//----------------------------------------------------------------------------------------------------
void UIGridController::syncMenuValueAndSize ()
{
	if (!gridMenu)
		return;

	auto index = indexOf (defGrids.begin (), defGrids.end (), getSize ());
	if (!index)
	{
		gridMenu->setValue (0.f);
		setSize (defGrids[0]);
		return;
	}
	gridMenu->setValue (static_cast<float> (*index));
}

//----------------------------------------------------------------------------------------------------
void UIGridController::setupTextEdit (CTextEdit* te) const
{
	te->setPrecision (0);
	te->setStringToValueFunction ([] (UTF8StringPtr txt, float& result, CTextEdit* textEdit) {
		if (auto value = UTF8StringView (txt).toNumber<float> ())
		{
			result = *value;
			return true;
		}
		return false;
	});
}

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
