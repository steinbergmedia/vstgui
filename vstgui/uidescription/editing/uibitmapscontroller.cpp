// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uibitmapscontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uibasedatasource.h"
#include "uieditcontroller.h"
#include "uidialogcontroller.h"
#include "uiviewcreatecontroller.h"
#include "../cstream.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../../lib/cbitmap.h"
#include "../../lib/cbitmapfilter.h"
#include "../../lib/cdropsource.h"
#include "../../lib/cfileselector.h"
#include "../../lib/idatapackage.h"
#include "../../lib/dragging.h"
#include "../../lib/cvstguitimer.h"
#include "../../lib/controls/ccolorchooser.h"
#include "../../lib/controls/ctextedit.h"
#include "../../lib/controls/csearchtextedit.h"
#include "../../lib/platform/iplatformbitmap.h"
#include "../../lib/platform/platformfactory.h"

namespace VSTGUI {
//----------------------------------------------------------------------------------------------------
class UIBitmapView : public CView
{
public:
	UIBitmapView (CBitmap* bitmap = nullptr)
	: CView (CRect (0, 0, 0, 0))
	, zoom (1.)
	{
		setBackground (bitmap);
	}
	
	void draw (CDrawContext* context) override
	{
		if (CBitmap* bitmap = getBackground ())
		{
			CGraphicsTransform matrix;
			matrix.scale (zoom, zoom);
			CDrawContext::Transform transform (*context, matrix);
			CRect r (getViewSize ());
			matrix.inverse ().transform (r);
			bitmap->CBitmap::draw (context, r);
			auto* nptBitmap = dynamic_cast<CNinePartTiledBitmap*>(bitmap);
			if (nptBitmap)
			{
				static const CCoord kDefaultOnOffDashLength2[] = {2, 2};
				const CLineStyle kLineOnOffDash2 (CLineStyle::kLineCapButt, CLineStyle::kLineJoinMiter, 0, 2, kDefaultOnOffDashLength2);

				const CNinePartTiledDescription& offsets = nptBitmap->getPartOffsets ();

				CRect r2;
				r2.setWidth (nptBitmap->getWidth());
				r2.setHeight (nptBitmap->getHeight());
				CPoint p = getViewSize ().getTopLeft ();
				matrix.inverse ().transform (p);
				r2.offset (p.x, p.y);

				context->setDrawMode (kAntiAliasing);
				context->setFrameColor (kBlueCColor);
				context->setLineWidth (1);
				context->setLineStyle (kLineSolid);
				
				context->drawLine (CPoint (r2.left, r2.top + offsets.top), CPoint (r2.right, r2.top + offsets.top));
				context->drawLine (CPoint (r2.left, r2.bottom - offsets.bottom), CPoint (r2.right, r2.bottom - offsets.bottom));
				context->drawLine (CPoint (r2.left + offsets.left, r2.top), CPoint (r2.left + offsets.left, r2.bottom));
				context->drawLine (CPoint (r2.right - offsets.right, r2.top), CPoint (r2.right - offsets.right, r2.bottom));

				context->setFrameColor (kRedCColor);
				context->setLineWidth (1);
				context->setLineStyle (kLineOnOffDash2);

				context->drawLine (CPoint (r2.left, r2.top + offsets.top), CPoint (r2.right, r2.top + offsets.top));
				context->drawLine (CPoint (r2.left, r2.bottom - offsets.bottom), CPoint (r2.right, r2.bottom - offsets.bottom));
				context->drawLine (CPoint (r2.left + offsets.left, r2.top), CPoint (r2.left + offsets.left, r2.bottom));
				context->drawLine (CPoint (r2.right - offsets.right, r2.top), CPoint (r2.right - offsets.right, r2.bottom));
			}
		}
	}

	void setZoom (CCoord factor)
	{
		if (factor <= 0. || factor == zoom)
			return;
		zoom = factor;
		updateSize ();
	}

	void updateSize ()
	{
		if (CBitmap* bitmap = getBackground ())
		{
			CCoord width = bitmap ? bitmap->getWidth () : 0;
			CCoord height = bitmap ? bitmap->getHeight () : 0;
			CRect r (getViewSize ());
			
			CGraphicsTransform ().scale (zoom, zoom).transform (width, height);
			width = std::floor (width + 0.5);
			height = std::floor (height + 0.5);
			r.setWidth (width);
			r.setHeight (height);
			
			if (getViewSize () != r)
			{
				setViewSize (r);
				setMouseableArea (r);
			}
		}
	}

	void setBackground (CBitmap *background) override
	{
		auto platformBitmap = background ? background->getPlatformBitmap () : nullptr;
		if (platformBitmap && platformBitmap->getScaleFactor () != 1.)
		{
			// get rid of the scale factor
			auto buffer = getPlatformFactory ().createBitmapMemoryPNGRepresentation (platformBitmap);
			if (!buffer.empty ())
			{
				auto newPlatformBitmap = getPlatformFactory ().createBitmapFromMemory (buffer.data (), static_cast<uint32_t> (buffer.size ()));
				CView::setBackground (makeOwned<CBitmap> (newPlatformBitmap));
			}
		}
		else
			CView::setBackground (background);
		updateSize ();
	}

protected:
	CCoord zoom;
};

//----------------------------------------------------------------------------------------------------
class UIBitmapsDataSource : public UIBaseDataSource
{
public:
	UIBitmapsDataSource (UIDescription* description, IActionPerformer* actionPerformer, GenericStringListDataBrowserSourceSelectionChanged* delegate);
	
	CBitmap* getSelectedBitmap ();
	UTF8StringPtr getSelectedBitmapName ();

	bool add () override;
protected:
	void onUIDescBitmapChanged (UIDescription* desc) override;
	void getNames (std::list<const std::string*>& names) override;
	bool addItem (UTF8StringPtr name) override;
	bool removeItem (UTF8StringPtr name) override;
	bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) override;
	UTF8StringPtr getDefaultsName () override { return "UIBitmapsDataSource"; }

	bool addBitmap (UTF8StringPtr path, std::string& outName);

	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser) override;
	void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* control, CDataBrowser* browser) override;
	void dbOnDragEnterBrowser (IDataPackage* drag, CDataBrowser* browser) override;
	void dbOnDragExitBrowser (IDataPackage* drag, CDataBrowser* browser) override;
	DragOperation dbOnDragEnterCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser) override;
	DragOperation dbOnDragMoveInCell (int32_t row, int32_t column, const CPoint& where,
	                                  IDataPackage* drag, CDataBrowser* browser) override;
	void dbOnDragExitCell (int32_t row, int32_t column, IDataPackage* drag, CDataBrowser* browser) override;
	bool dbOnDropInCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser) override;

	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) override;
	CMouseEventResult dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) override;

	SharedPointer<CColorChooser> colorChooser;
	DragStartMouseObserver dragStartMouseObserver;
	bool dragContainsBitmaps;
};

//----------------------------------------------------------------------------------------------------
UIBitmapsDataSource::UIBitmapsDataSource (UIDescription* description, IActionPerformer* actionPerformer, GenericStringListDataBrowserSourceSelectionChanged* delegate)
: UIBaseDataSource (description, actionPerformer, delegate)
, dragContainsBitmaps (false)
{
}

//----------------------------------------------------------------------------------------------------
void UIBitmapsDataSource::onUIDescBitmapChanged (UIDescription* desc)
{
	onUIDescriptionUpdate ();
}

//----------------------------------------------------------------------------------------------------
void UIBitmapsDataSource::dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser)
{
	auto drawWidth = size.getHeight ();
	GenericStringListDataBrowserSource::drawRowBackground (context, size, row, flags, browser);
	CRect r (size);
	r.right -= drawWidth;
	GenericStringListDataBrowserSource::drawRowString (context, r, row, flags, browser);
	if (auto bitmap = description->getBitmap (names.at (static_cast<uint32_t> (row)).data ()))
	{
		r = size;
		r.left = r.right - drawWidth;
		r.inset (2, 2);
		auto bitmapSize = bitmap->getSize ();
		auto scaleX = r.getWidth () / bitmapSize.x;
		auto scaleY = r.getHeight () / bitmapSize.y;
		CGraphicsTransform matrix;
		matrix.scale (scaleX, scaleY);
		CDrawContext::Transform t (*context, matrix);
		matrix.inverse ().transform (r);
		bitmap->CBitmap::draw (context, r);
	}
}

//----------------------------------------------------------------------------------------------------
void UIBitmapsDataSource::dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* control, CDataBrowser* browser)
{
	UIBaseDataSource::dbCellSetupTextEdit (row, column, control, browser);
	CRect r (control->getViewSize ());
	auto drawWidth = r.getHeight ();
	r.right -= drawWidth;
	control->setViewSize (r);
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIBitmapsDataSource::dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
{
	if (buttons.isDoubleClick () && row >= 0 && row < static_cast<int32_t> (names.size ()))
	{
		auto r = browser->getCellBounds ({row, column});
		auto drawWidth = r.getHeight ();
		r.left = r.right - drawWidth;
		if (r.pointInside (where))
		{
			delegate->dbRowDoubleClick (row, this);
			return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		}
	}
	dragStartMouseObserver.init (where);
	UIBaseDataSource::dbOnMouseDown (where, buttons, row, column, browser);
	return kMouseEventHandled;
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIBitmapsDataSource::dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
{
	if (buttons.isLeftButton () && dragStartMouseObserver.shouldStartDrag (where))
	{
		if (auto bitmap = getSelectedBitmap ())
		{
			UIAttributes attr;
			attr.setAttribute (UIViewCreator::kAttrBitmap, getSelectedBitmapName ());
			attr.setPointAttribute (UIViewCreator::kAttrSize, bitmap->getSize ());
			const auto factory =
			    dynamic_cast<const UIViewFactory*> (description->getViewFactory ());
			if (auto selection = createSelectionFromViewName (UIViewCreator::kCView, factory,
			                                                  description, &attr))
			{
				CMemoryStream stream (1024, 1024, false);
				if (selection->store (stream, description))
				{
					stream.end ();
					auto dropSource = CDropSource::create (stream.getBuffer (),
					                                       static_cast<uint32_t> (stream.tell ()),
					                                       CDropSource::kText);
					browser->doDrag (DragDescription (dropSource, {}, bitmap));
					return kMouseMoveEventHandledButDontNeedMoreEvents;
				}
			}
		}
	}
	return kMouseEventHandled;
}

//----------------------------------------------------------------------------------------------------
void UIBitmapsDataSource::dbOnDragEnterBrowser (IDataPackage* drag, CDataBrowser* browser)
{
	uint32_t index = 0;
	IDataPackage::Type type;
	const void* item = nullptr;
	while (drag->getData (index, item, type) > 0)
	{
		if (type == IDataPackage::kFilePath)
		{
			const char* ext = strrchr (static_cast<const char*> (item), '.');
			if (ext)
			{
				std::string extStr (ext);
				std::transform (extStr.begin (), extStr.end (), extStr.begin (), ::tolower);
				if (extStr == ".png" || extStr == ".bmp" || extStr == ".jpg" || extStr == ".jpeg")
				{
					dragContainsBitmaps = true;
					break;
				}
			}
		}
		index++;
	}
	if (dragContainsBitmaps)
		browser->getFrame ()->setCursor (kCursorCopy);
}

//----------------------------------------------------------------------------------------------------
void UIBitmapsDataSource::dbOnDragExitBrowser (IDataPackage* drag, CDataBrowser* browser)
{
	if (dragContainsBitmaps)
		browser->getFrame ()->setCursor (kCursorNotAllowed);
}

//----------------------------------------------------------------------------------------------------
DragOperation UIBitmapsDataSource::dbOnDragEnterCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser)
{
	return dragContainsBitmaps ? DragOperation::Copy : DragOperation::None;
}

//----------------------------------------------------------------------------------------------------
DragOperation UIBitmapsDataSource::dbOnDragMoveInCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser)
{
	return dragContainsBitmaps ? DragOperation::Copy : DragOperation::None;
}

//----------------------------------------------------------------------------------------------------
void UIBitmapsDataSource::dbOnDragExitCell (int32_t row, int32_t column, IDataPackage* drag, CDataBrowser* browser)
{
}

//----------------------------------------------------------------------------------------------------
bool UIBitmapsDataSource::dbOnDropInCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser)
{
	if (!dragContainsBitmaps)
		return false;

	bool didBeganGroupAction = false;
	uint32_t index = 0;
	IDataPackage::Type type;
	const void* item = nullptr;
	UTF8String firstNewBitmapName;
	while (drag->getData (index++, item, type) > 0)
	{
		if (type == IDataPackage::kFilePath)
		{
			auto path = static_cast<UTF8StringPtr> (item);
			const char* ext = strrchr (path, '.');
			if (ext)
			{
				std::string extStr (ext);
				std::transform (extStr.begin (), extStr.end (), extStr.begin (), ::tolower);
				if (extStr == ".png" || extStr == ".bmp" || extStr == ".jpg" || extStr == ".jpeg")
				{
					if (!didBeganGroupAction)
					{
						actionPerformer->beginGroupAction ("Add Bitmaps");
						didBeganGroupAction = true;
					}
					std::string name;
					addBitmap (path, name);
					if (firstNewBitmapName.empty ())
						firstNewBitmapName = name;
				}
			}
		}
	}
	if (didBeganGroupAction)
	{
		actionPerformer->finishGroupAction ();
		vstgui_assert (!firstNewBitmapName.empty ());
		selectName (firstNewBitmapName);
	}
	dragContainsBitmaps = false;
	return true;
}

//----------------------------------------------------------------------------------------------------
void UIBitmapsDataSource::getNames (std::list<const std::string*>& names)
{
	description->collectBitmapNames (names);
}

//----------------------------------------------------------------------------------------------------
bool UIBitmapsDataSource::addItem (UTF8StringPtr name)
{
	actionPerformer->performBitmapChange (name, nullptr);
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIBitmapsDataSource::removeItem (UTF8StringPtr name)
{
	actionPerformer->performBitmapChange (name, nullptr, true);
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
		return description->getBitmap (names.at (static_cast<uint32_t> (selectedRow)).data ());
	return nullptr;
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr UIBitmapsDataSource::getSelectedBitmapName ()
{
	int32_t selectedRow = dataBrowser ? dataBrowser->getSelectedRow() : CDataBrowser::kNoSelection;
	if (selectedRow != CDataBrowser::kNoSelection && selectedRow < (int32_t)names.size ())
		return names.at (static_cast<uint32_t> (selectedRow)).data ();
	return nullptr;
}

//----------------------------------------------------------------------------------------------------
bool UIBitmapsDataSource::addBitmap (UTF8StringPtr path, std::string& outName)
{
	bool result = false;
	outName = path;
	unixfyPath (outName);
	size_t index = outName.find_last_of (unixPathSeparator);
	outName.erase (0, index+1);
	index = outName.find_last_of ('.');
	if (index == std::string::npos)
		return false;

	outName.erase (index);
	if (createUniqueName (outName))
	{
		std::string pathStr (path);
		UTF8StringPtr descPath = description->getFilePath ();
		if (descPath && descPath[0] != 0)
		{
			std::string descPathStr (descPath);
			unixfyPath (descPathStr);
			if (removeLastPathComponent (descPathStr))
			{
				if (pathStr.find (descPathStr) == 0)
				{
					pathStr.erase (0, descPathStr.length () + 1);
				}
			}
		}
		actionPerformer->performBitmapChange (outName.data (), pathStr.data ());
		result = true;
	}
	return result;
}

//----------------------------------------------------------------------------------------------------
bool UIBitmapsDataSource::add ()
{
	bool result = false;
	auto fs = owned (CNewFileSelector::create (dataBrowser->getFrame ()));
	if (fs)
	{
		fs->addFileExtension (CFileExtension ("PNG", "PNG", "image/png"));
		fs->setAllowMultiFileSelection (true);
		if (fs->runModal ())
		{
			auto numFiles = static_cast<uint32_t> (fs->getNumSelectedFiles ());
			if (numFiles > 1)
				actionPerformer->beginGroupAction ("Add Bitmaps");
			for (uint32_t i = 0; i < numFiles; i++)
			{
				UTF8StringPtr path = fs->getSelectedFile (i);
				if (path)
				{
					std::string newName;
					if (addBitmap (path, newName) && i == numFiles - 1)
					{
						int32_t row = selectName (newName.data ());
						if (row != -1)
							dbOnMouseDown (CPoint (0, 0), CButtonState (kLButton|kDoubleClick), row, 0, dataBrowser);
						result = true;
					}
				}
			}
			if (numFiles > 1)
				actionPerformer->finishGroupAction ();
		}
	}
	return result;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class UIBitmapSettingsController : public NonAtomicReferenceCounted,
                                   public IDialogController,
                                   public IController
{
public:
	UIBitmapSettingsController (CBitmap* bitmap, const std::string& bitmapName, UIDescription* description, IActionPerformer* actionPerformer);
	~UIBitmapSettingsController () noexcept override = default;

	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override;
	void valueChanged (CControl* pControl) override;
	void controlBeginEdit (CControl* pControl) override;
	void controlEndEdit (CControl* pControl) override;

	void onDialogButton1Clicked (UIDialogController*) override;
	void onDialogButton2Clicked (UIDialogController*) override;
	void onDialogShow (UIDialogController*) override;
protected:
	void updateNinePartTiledControls ();
	static bool stringToValue (UTF8StringPtr txt, float& result, CTextEdit::StringToValueUserData* userData);
	static bool valueToString (float value, char utf8String[256], CParamDisplay::ValueToStringUserData* userData);

	SharedPointer<CBitmap> bitmap;
	SharedPointer<UIDescription> editDescription;
	SharedPointer<UIBitmapView> bitmapView;
	IActionPerformer* actionPerformer;
	std::string bitmapName;
	CRect origOffsets;

	enum {
		kBitmapPathTag,
		kBitmapWidthTag,
		kBitmapHeightTag,
		kNinePartTiledTag,
		kNinePartTiledLeftTag,
		kNinePartTiledTopTag,
		kNinePartTiledRightTag,
		kNinePartTiledBottomTag,
		kZoomTag,
		kZoomTextTag,
		kNumTags
	};
	CControl* controls[kNumTags];
};

//----------------------------------------------------------------------------------------------------
UIBitmapSettingsController::UIBitmapSettingsController (CBitmap* bitmap, const std::string& bitmapName, UIDescription* description, IActionPerformer* actionPerformer)
: bitmap (bitmap)
, editDescription (description)
, actionPerformer (actionPerformer)
, bitmapName (bitmapName)
, origOffsets (10, 10, 10, 10)
{
	for (auto& control : controls)
		control = nullptr;
}

//----------------------------------------------------------------------------------------------------
void UIBitmapSettingsController::updateNinePartTiledControls ()
{
	auto* nptb = bitmap.cast<CNinePartTiledBitmap> ();
	if (nptb)
	{
		controls[kNinePartTiledTag]->setValueNormalized (1);
		const CNinePartTiledDescription& offsets = nptb->getPartOffsets ();
		controls[kNinePartTiledLeftTag]->setValue ((float)offsets.left);
		controls[kNinePartTiledTopTag]->setValue ((float)offsets.top);
		controls[kNinePartTiledRightTag]->setValue ((float)offsets.right);
		controls[kNinePartTiledBottomTag]->setValue ((float)offsets.bottom);
	}
	else
	{
		controls[kNinePartTiledTag]->setValueNormalized (0.);
		for (int32_t i = kNinePartTiledLeftTag; i <= kNinePartTiledBottomTag; i++)
		{
			auto* label = dynamic_cast<CTextLabel*>(controls[i]);
			if (label)
				label->setText ("");
		}
	}
	for (int32_t i = kNinePartTiledLeftTag; i <= kNinePartTiledBottomTag; i++)
	{
		controls[i]->setMouseEnabled (nptb ? true : false);
	}
}

//----------------------------------------------------------------------------------------------------
void UIBitmapSettingsController::valueChanged (CControl* control)
{
	switch (control->getTag ())
	{
		case kBitmapPathTag:
		{
			auto* edit = dynamic_cast<CTextEdit*>(control);
			if (edit)
			{
				actionPerformer->performBitmapChange (bitmapName.data (), edit->getText ());
				bitmap = editDescription->getBitmap (bitmapName.data ());
				bitmapView->setBackground (bitmap);
				updateNinePartTiledControls ();
			}
			break;
		}
		case kNinePartTiledTag:
		{
			auto* nptb = bitmap.cast<CNinePartTiledBitmap> ();
			if (nptb)
			{
				origOffsets.left = nptb->getPartOffsets ().left;
				origOffsets.top = nptb->getPartOffsets ().top;
				origOffsets.right = nptb->getPartOffsets ().right;
				origOffsets.bottom = nptb->getPartOffsets ().bottom;
			}
			bool checked = control->getValue () == control->getMax ();
			actionPerformer->performBitmapNinePartTiledChange (bitmapName.data (), checked ? &origOffsets : nullptr);
			bitmap = editDescription->getBitmap (bitmapName.data ());
			bitmapView->setBackground (bitmap);
			updateNinePartTiledControls ();
			break;
		}
		case kNinePartTiledLeftTag:
		case kNinePartTiledTopTag:
		case kNinePartTiledRightTag:
		case kNinePartTiledBottomTag:
		{
			CRect r;
			r.left = controls[kNinePartTiledLeftTag]->getValue ();
			r.top = controls[kNinePartTiledTopTag]->getValue ();
			r.right = controls[kNinePartTiledRightTag]->getValue ();
			r.bottom = controls[kNinePartTiledBottomTag]->getValue ();
			actionPerformer->performBitmapNinePartTiledChange (bitmapName.data (), &r);
			bitmap = editDescription->getBitmap (bitmapName.data ());
			bitmapView->setBackground (bitmap);
			updateNinePartTiledControls ();
			SharedPointer<CTextEdit> textEdit = SharedPointer<CControl> (control).cast<CTextEdit> ();
			if (textEdit && textEdit->bWasReturnPressed)
			{
				textEdit->getFrame ()->doAfterEventProcessing ([=] () {
					textEdit->takeFocus ();
				});
			}
			break;
		}
		case kZoomTag:
		{
			CCoord zoom = floor (control->getValue () / 10. + 0.5);
			control->setValue ((float)zoom * 10.f);
			controls[kZoomTextTag]->setValue ((float)zoom * 10.f);
			bitmapView->setZoom (zoom / 10.);
			controls[kZoomTextTag]->invalid ();
			control->invalid ();
			break;
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIBitmapSettingsController::controlBeginEdit (CControl* control)
{
}

//----------------------------------------------------------------------------------------------------
void UIBitmapSettingsController::controlEndEdit (CControl* control)
{
}

//----------------------------------------------------------------------------------------------------
void UIBitmapSettingsController::onDialogButton1Clicked (UIDialogController*)
{
}

//----------------------------------------------------------------------------------------------------
void UIBitmapSettingsController::onDialogButton2Clicked (UIDialogController*)
{
}

//----------------------------------------------------------------------------------------------------
void UIBitmapSettingsController::onDialogShow (UIDialogController*)
{
	bitmapView->setBackground (bitmap);
	updateNinePartTiledControls ();
}

//----------------------------------------------------------------------------------------------------
CView* UIBitmapSettingsController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	auto* control = dynamic_cast<CControl*>(view);
	if (control && control->getTag () >= 0 && control->getTag () < kNumTags)
	{
		controls[control->getTag ()] = control;
		switch (control->getTag ())
		{
			case kBitmapPathTag:
			{
				auto* bitmapPathEdit = dynamic_cast<CTextEdit*> (control);
				if (bitmapPathEdit)
					bitmapPathEdit->setText (bitmap->getResourceDescription ().u.name);
				break;
			}
			case kBitmapWidthTag:
			{
				auto* label = dynamic_cast<CTextLabel*>(control);
				if (label)
				{
					float width = bitmap->getPlatformBitmap () ? (float)bitmap->getPlatformBitmap ()->getSize ().x : 0.f;
					label->setPrecision (0);
					label->setMax (width);
					label->setValue (width);
					label->sizeToFit ();
				}
				break;
			}
			case kBitmapHeightTag:
			{
				auto* label = dynamic_cast<CTextLabel*>(control);
				if (label)
				{
					float height = bitmap->getPlatformBitmap () ? (float)bitmap->getPlatformBitmap ()->getSize ().y : 0.f;
					label->setPrecision (0);
					label->setMax (height);
					label->setValue (height);
					label->sizeToFit ();
					if (controls[kBitmapWidthTag])
					{
						CRect r = control->getViewSize ();
						if (controls[kBitmapWidthTag]->getWidth () > r.getWidth ())
						{
							r.setWidth (controls[kBitmapWidthTag]->getWidth ());
							control->setViewSize (r);
						}
						else
						{
							r = controls[kBitmapWidthTag]->getViewSize ();
							r.setWidth (control->getWidth ());
							controls[kBitmapWidthTag]->setViewSize (r);
						}
					}
				}
				break;
			}
			case kNinePartTiledLeftTag:
			case kNinePartTiledRightTag:
			{
				auto* textEdit = dynamic_cast<CTextEdit*>(control);
				if (textEdit)
				{
					textEdit->setPrecision (0);
					textEdit->setStringToValueFunction (stringToValue);
				}
				control->setMax ((float)bitmap->getWidth ());
				break;
			}
			case kNinePartTiledTopTag:
			case kNinePartTiledBottomTag:
			{
				auto* textEdit = dynamic_cast<CTextEdit*>(control);
				if (textEdit)
				{
					textEdit->setPrecision (0);
					textEdit->setStringToValueFunction (stringToValue);
				}
				control->setMax ((float)bitmap->getHeight ());
				break;
			}
			case kZoomTag:
			{
				control->setValue (100);
				break;
			}
			case kZoomTextTag:
			{
				auto* label = dynamic_cast<CTextLabel*>(control);
				if (label)
				{
					label->setValueToStringFunction (valueToString);
				}
				control->setValue (100);
			}
		}
	}
	return view;
}

//----------------------------------------------------------------------------------------------------
CView* UIBitmapSettingsController::createView (const UIAttributes& attributes, const IUIDescription* description)
{
	const std::string* name = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (name)
	{
		if (*name == "BitmapView")
		{
			bitmapView = new UIBitmapView ();
			return bitmapView;
		}
	}
	return nullptr;
}

//----------------------------------------------------------------------------------------------------
bool UIBitmapSettingsController::valueToString (float value, char utf8String[256], CParamDisplay::ValueToStringUserData* userData)
{
	auto intValue = (int32_t)value;
	std::stringstream str;
	str << intValue;
	str << "%";
	std::strcpy (utf8String, str.str ().data ());
	return true;
}


//----------------------------------------------------------------------------------------------------
bool UIBitmapSettingsController::stringToValue (UTF8StringPtr txt, float& result, CTextEdit::StringToValueUserData* userData)
{
	int32_t value = txt ? (int32_t)strtol (txt, nullptr, 10) : 0;
	result = (float)value;
	return true;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIBitmapsController::UIBitmapsController (IController* baseController, UIDescription* description, IActionPerformer* actionPerformer)
: DelegationController (baseController)
, editDescription (description)
, actionPerformer (actionPerformer)
, dataSource (nullptr)
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
void UIBitmapsController::showSettingsDialog ()
{
	auto* dc = new UIDialogController (this, bitmapPathEdit->getFrame ());
	auto fsController = makeOwned<UIBitmapSettingsController> (dataSource->getSelectedBitmap (),
	                                                           dataSource->getSelectedBitmapName (),
	                                                           editDescription, actionPerformer);
	dc->run ("bitmap.settings", "Bitmap Settings", "Close", nullptr, fsController,
	         UIEditController::getEditorDescription ());
}

//----------------------------------------------------------------------------------------------------
CView* UIBitmapsController::createView (const UIAttributes& attributes, const IUIDescription* description)
{
	const std::string* name = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (name)
	{
		if (*name == "BitmapsBrowser")
		{
			CDataBrowser* dataBrowser = new CDataBrowser (CRect (0, 0, 0, 0), dataSource, CDataBrowser::kDrawRowLines|CScrollView::kHorizontalScrollbar | CScrollView::kVerticalScrollbar);
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
CView* UIBitmapsController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	auto searchField = dynamic_cast<CSearchTextEdit*>(view);
	if (searchField && searchField->getTag () == kSearchTag)
	{
		dataSource->setSearchFieldControl (searchField);
		return searchField;
	}
	auto* textEdit = dynamic_cast<CTextEdit*>(view);
	if (textEdit)
	{
		switch (textEdit->getTag ())
		{
			case kBitmapPathTag:
			{
				bitmapPathEdit = textEdit;
				textEdit->setMouseEnabled (false);
				break;
			}
		}
	}
	else
	{
		auto* control = dynamic_cast<CControl*> (view);
		if (control)
		{
			if (control->getTag () == kSettingsTag)
			{
				settingButton = control;
				settingButton->setMouseEnabled (false);
			}
		}
	}
	return DelegationController::verifyView (view, attributes, description);
}

//----------------------------------------------------------------------------------------------------
IControlListener* UIBitmapsController::getControlListener (UTF8StringPtr name)
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
			UTF8StringPtr bitmapName = dataSource->getSelectedBitmapName ();
			if (bitmapName)
			{
				auto* edit = dynamic_cast<CTextEdit*>(pControl);
				if (edit)
					actionPerformer->performBitmapChange (bitmapName, edit->getText ());
			}
			break;
		}
		case kSettingsTag:
		{
			if (pControl->getValue () == pControl->getMax ())
			{
				showSettingsDialog ();
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
		UTF8StringPtr selectedBitmapName = dataSource->getSelectedBitmapName ();
		if (bitmapView)
		{
			bitmapView->setBackground (bitmap);
			if (bitmapView->getParentView ())
				bitmapView->getParentView ()->invalid ();
		}
		if (bitmapPathEdit)
		{
			bitmapPathEdit->setText (bitmap ? bitmap->getResourceDescription ().u.name : nullptr);
			bitmapPathEdit->setMouseEnabled (selectedBitmapName ? true : false);
		}
		if (settingButton)
		{
			settingButton->setMouseEnabled (selectedBitmapName ? true : false);
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIBitmapsController::dbRowDoubleClick (int32_t row, GenericStringListDataBrowserSource* source)
{
	showSettingsDialog ();
}

//----------------------------------------------------------------------------------------------------
bool UIBitmapsController::valueToString (float value, char utf8String[256], void* userData)
{
	auto intValue = (int32_t)value;
	std::stringstream str;
	str << intValue;
	std::strcpy (utf8String, str.str ().data ());
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIBitmapsController::stringToValue (UTF8StringPtr txt, float& result, void* userData)
{
	int32_t value = txt ? (int32_t)strtol (txt, nullptr, 10) : 0;
	result = (float)value;
	return true;
}

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
