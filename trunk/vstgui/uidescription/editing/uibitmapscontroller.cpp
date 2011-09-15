#include "uibitmapscontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uibasedatasource.h"
#include "uisearchtextfield.h"
#include "uieditcontroller.h"
#include "../../lib/controls/ccolorchooser.h"
#include "../../lib/cbitmapfilter.h"

namespace VSTGUI {
//----------------------------------------------------------------------------------------------------
class UIBitmapView : public CView
{
public:
	UIBitmapView () : CView (CRect (0, 0, 0, 0)) {}
	
	void draw (CDrawContext* context)
	{
		CView::draw (context);
		if (getBackground ())
		{
			CNinePartTiledBitmap* bitmap = dynamic_cast<CNinePartTiledBitmap*>(getBackground ());
			if (bitmap)
			{
				const CNinePartTiledBitmap::PartOffsets offsets = bitmap->getPartOffsets ();
				CRect r (getViewSize ());
				context->setDrawMode (kAliasing);
				context->setFrameColor (kRedCColor);
				context->setLineWidth (1);
				context->setLineStyle (kLineOnOffDash);
				context->moveTo (CPoint (r.left, r.top + offsets.top));
				context->lineTo (CPoint (r.right, r.top + offsets.top));
				context->moveTo (CPoint (r.left, r.bottom - offsets.bottom));
				context->lineTo (CPoint (r.right, r.bottom - offsets.bottom));
				context->moveTo (CPoint (r.left + offsets.left, r.top));
				context->lineTo (CPoint (r.left + offsets.left, r.bottom));
				context->moveTo (CPoint (r.right - offsets.right, r.top));
				context->lineTo (CPoint (r.right - offsets.right, r.bottom));
			}
		}
	}

	void setBackground (CBitmap *background)
	{
		CView::setBackground (background);
		CCoord width = background ? background->getWidth () : 0;
		CCoord height = background ? background->getHeight () : 0;
		CRect r (getViewSize ());
		r.setWidth (width);
		r.setHeight(height);
		if (getViewSize () != r)
		{
			setViewSize (r);
			setMouseableArea (r);
		}
	}
};

//----------------------------------------------------------------------------------------------------
class UIBitmapsDataSource : public UIBaseDataSource
{
public:
	UIBitmapsDataSource (UIDescription* description, IActionPerformer* actionPerformer, IGenericStringListDataBrowserSourceSelectionChanged* delegate);
	
	CBitmap* getSelectedBitmap ();
	UTF8StringPtr getSelectedBitmapName ();
protected:
	virtual void getNames (std::list<const std::string*>& names);
	virtual bool addItem (UTF8StringPtr name);
	virtual bool removeItem (UTF8StringPtr name);
	virtual bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName);
	virtual UTF8StringPtr getDefaultsName () { return "UIBitmapsDataSource"; }

	SharedPointer<CColorChooser> colorChooser;
};

//----------------------------------------------------------------------------------------------------
UIBitmapsDataSource::UIBitmapsDataSource (UIDescription* description, IActionPerformer* actionPerformer, IGenericStringListDataBrowserSourceSelectionChanged* delegate)
: UIBaseDataSource (description, actionPerformer, UIDescription::kMessageBitmapChanged, delegate)
{
}

//----------------------------------------------------------------------------------------------------
void UIBitmapsDataSource::getNames (std::list<const std::string*>& names)
{
	description->collectBitmapNames (names);
}

//----------------------------------------------------------------------------------------------------
bool UIBitmapsDataSource::addItem (UTF8StringPtr name)
{
	actionPerformer->performBitmapChange (name, 0);
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIBitmapsDataSource::removeItem (UTF8StringPtr name)
{
	actionPerformer->performBitmapChange (name, 0, true);
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIBitmapsDataSource::performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	actionPerformer->performBitmapNameChange (oldName, newName);
	return true;
}

//----------------------------------------------------------------------------------------------------
CBitmap* UIBitmapsDataSource::getSelectedBitmap ()
{
	int32_t selectedRow = dataBrowser ? dataBrowser->getSelectedRow() : CDataBrowser::kNoSelection;
	if (selectedRow != CDataBrowser::kNoSelection && selectedRow < (int32_t)names.size ())
		return description->getBitmap (names.at (selectedRow).c_str ());
	return 0;
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr UIBitmapsDataSource::getSelectedBitmapName ()
{
	int32_t selectedRow = dataBrowser ? dataBrowser->getSelectedRow() : CDataBrowser::kNoSelection;
	if (selectedRow != CDataBrowser::kNoSelection && selectedRow < (int32_t)names.size ())
		return names.at (selectedRow).c_str ();
	return 0;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIBitmapsController::UIBitmapsController (IController* baseController, UIDescription* description, IActionPerformer* actionPerformer)
: DelegationController (baseController)
, editDescription (description)
, actionPerformer (actionPerformer)
, dataSource (0)
{
	dataSource = new UIBitmapsDataSource (editDescription, actionPerformer, this);
	UIEditController::setupDataSource (dataSource);
}

//----------------------------------------------------------------------------------------------------
UIBitmapsController::~UIBitmapsController ()
{
	dataSource->forget ();
}

//----------------------------------------------------------------------------------------------------
CView* UIBitmapsController::createView (const UIAttributes& attributes, IUIDescription* description)
{
	const std::string* name = attributes.getAttributeValue ("custom-view-name");
	if (name)
	{
		if (*name == "BitmapsBrowser")
		{
			CDataBrowser* dataBrowser = new CDataBrowser (CRect (0, 0, 0, 0), 0, dataSource, CDataBrowser::kDrawRowLines|CScrollView::kHorizontalScrollbar | CScrollView::kVerticalScrollbar);
			return dataBrowser;
		}
		else if (*name == "BitmapView")
		{
			bitmapView = new UIBitmapView ();
			return bitmapView;
		}
	}
	return DelegationController::createView (attributes, description);
}

//----------------------------------------------------------------------------------------------------
CView* UIBitmapsController::verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
{
	UISearchTextField* searchField = dynamic_cast<UISearchTextField*>(view);
	if (searchField && searchField->getTag () == kSearchTag)
	{
		dataSource->setSearchFieldControl (searchField);
		return searchField;
	}
	CTextEdit* textEdit = dynamic_cast<CTextEdit*>(view);
	if (textEdit)
	{
		switch (textEdit->getTag ())
		{
			case kBitmapPathTag: bitmapPathEdit = textEdit; textEdit->setMouseEnabled (false); break;
			case kNinePartTiledLeftTag: ninePartRectEdit[0] = textEdit; textEdit->setValueToStringProc (valueToString); textEdit->setStringToValueProc (stringToValue); textEdit->setMouseEnabled (false); break;
			case kNinePartTiledTopTag: ninePartRectEdit[1] = textEdit; textEdit->setValueToStringProc (valueToString); textEdit->setStringToValueProc (stringToValue); textEdit->setMouseEnabled (false); break;
			case kNinePartTiledRightTag: ninePartRectEdit[2] = textEdit; textEdit->setValueToStringProc (valueToString); textEdit->setStringToValueProc (stringToValue); textEdit->setMouseEnabled (false); break;
			case kNinePartTiledBottomTag: ninePartRectEdit[3] = textEdit; textEdit->setValueToStringProc (valueToString); textEdit->setStringToValueProc (stringToValue); textEdit->setMouseEnabled (false); break;
		}
	}
	else
	{
		CControl* control = dynamic_cast<CControl*>(view);
		if (control && control->getTag () == kNinePartTiledTag)
		{
			ninePartTiled = control;
			ninePartTiled->setMouseEnabled (false);
		}
	}
	return DelegationController::verifyView (view, attributes, description);
}

//----------------------------------------------------------------------------------------------------
CControlListener* UIBitmapsController::getControlListener (UTF8StringPtr name)
{
	return this;
}

//----------------------------------------------------------------------------------------------------
void UIBitmapsController::valueChanged (CControl* pControl)
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
		case kBitmapPathTag:
		{
			CBitmap* bitmap = dataSource->getSelectedBitmap ();
			if (bitmap)
			{
				CTextEdit* edit = dynamic_cast<CTextEdit*>(pControl);
				if (edit)
					actionPerformer->performBitmapChange (dataSource->getSelectedBitmapName (), edit->getText ());
			}
			break;
		}
		case kNinePartTiledTag:
		{
			CBitmap* bitmap = dataSource->getSelectedBitmap ();
			if (bitmap)
			{
				bool checked = pControl->getValue () == pControl->getMax ();
				CRect offsets;
				actionPerformer->performBitmapNinePartTiledChange (dataSource->getSelectedBitmapName (), checked ? &offsets : 0);
			}
			break;
		}
		case kNinePartTiledLeftTag:
		case kNinePartTiledTopTag:
		case kNinePartTiledRightTag:
		case kNinePartTiledBottomTag:
		{
			CBitmap* b = dataSource->getSelectedBitmap ();
			CNinePartTiledBitmap* bitmap = b ? dynamic_cast<CNinePartTiledBitmap*>(b) : 0;
			if (bitmap)
			{
				CRect r;
				if (ninePartRectEdit[0])
					r.left = ninePartRectEdit[0]->getValue ();
				if (ninePartRectEdit[1])
					r.top = ninePartRectEdit[1]->getValue ();
				if (ninePartRectEdit[2])
					r.right = ninePartRectEdit[2]->getValue ();
				if (ninePartRectEdit[3])
					r.bottom = ninePartRectEdit[3]->getValue ();
				actionPerformer->performBitmapNinePartTiledChange (dataSource->getSelectedBitmapName (), &r);
			}
			break;
		}		
	}
}

//----------------------------------------------------------------------------------------------------
void UIBitmapsController::dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source)
{
	if (dataSource)
	{
		CBitmap* bitmap = dataSource->getSelectedBitmap ();
		if (bitmapView)
		{
//			CCoord width = bitmap ? bitmap->getWidth () : 0;
//			CCoord height = bitmap ? bitmap->getHeight () : 0;
//			CRect r (bitmapView->getViewSize ());
//			r.setWidth (width);
//			r.setHeight (height);
//			bitmapView->setViewSize (r);
			bitmapView->setBackground (bitmap);
			if (bitmapView->getParentView ())
				bitmapView->getParentView ()->invalid ();
		}
		if (bitmapPathEdit)
		{
			bitmapPathEdit->setText (bitmap ? bitmap->getResourceDescription ().u.name : 0);
			bitmapPathEdit->setMouseEnabled (bitmap ? true : false);
		}
		CNinePartTiledBitmap* nptb = bitmap ? dynamic_cast<CNinePartTiledBitmap*>(bitmap) : 0;
		if (nptb)
		{
			if (ninePartTiled)
			{
				ninePartTiled->setValue (ninePartTiled->getMax ());
				ninePartTiled->invalid ();
				ninePartTiled->setMouseEnabled (true);
			}
			const CNinePartTiledBitmap::PartOffsets offsets = nptb->getPartOffsets ();
			if (ninePartRectEdit[0])
			{
				ninePartRectEdit[0]->setValue ((float)offsets.left);
				ninePartRectEdit[0]->invalid ();
				ninePartRectEdit[0]->setMouseEnabled (true);
			}
			if (ninePartRectEdit[1])
			{
				ninePartRectEdit[1]->setValue ((float)offsets.top);
				ninePartRectEdit[1]->invalid ();
				ninePartRectEdit[1]->setMouseEnabled (true);
			}
			if (ninePartRectEdit[2])
			{
				ninePartRectEdit[2]->setValue ((float)offsets.right);
				ninePartRectEdit[2]->invalid ();
				ninePartRectEdit[2]->setMouseEnabled (true);
			}
			if (ninePartRectEdit[3])
			{
				ninePartRectEdit[3]->setValue ((float)offsets.bottom);
				ninePartRectEdit[3]->invalid ();
				ninePartRectEdit[3]->setMouseEnabled (true);
			}
		}
		else
		{
			if (ninePartTiled)
			{
				ninePartTiled->setValue (ninePartTiled->getMin ());
				ninePartTiled->invalid ();
				ninePartTiled->setMouseEnabled (bitmap ? true : false);
			}
			for (int32_t i = 0; i < 4; i++)
			{
				if (ninePartRectEdit[i])
				{
					ninePartRectEdit[i]->setText (0);
					ninePartRectEdit[i]->setMouseEnabled (false);
				}
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------
bool UIBitmapsController::valueToString (float value, char utf8String[256], void* userData)
{
	int32_t intValue = (int32_t)value;
	std::stringstream str;
	str << intValue;
	strcpy (utf8String, str.str ().c_str ());
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIBitmapsController::stringToValue (UTF8StringPtr txt, float& result, void* userData)
{
	int32_t value = txt ? (int32_t)strtol (txt, 0, 10) : 0;
	result = (float)value;
	return true;
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
