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

#include "uibitmapscontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uibasedatasource.h"
#include "uisearchtextfield.h"
#include "uieditcontroller.h"
#include "uidialogcontroller.h"
#include "../../lib/cbitmapfilter.h"
#include "../../lib/cfileselector.h"
#include "../../lib/idatapackage.h"
#include "../../lib/cvstguitimer.h"
#include "../../lib/controls/ccolorchooser.h"
#include "../../lib/controls/ctextedit.h"
#include "../../lib/platform/iplatformbitmap.h"

namespace VSTGUI {
//----------------------------------------------------------------------------------------------------
class UIBitmapView : public CView
{
public:
	UIBitmapView (CBitmap* bitmap = 0)
	: CView (CRect (0, 0, 0, 0))
	, zoom (1.)
	{
		setBackground (bitmap);
	}
	
	void draw (CDrawContext* context) VSTGUI_OVERRIDE_VMETHOD
	{
		if (CBitmap* bitmap = getBackground ())
		{
			CGraphicsTransform matrix;
			matrix.scale (zoom, zoom);
			CDrawContext::Transform transform (*context, matrix);
			CRect r (getViewSize ());
			matrix.inverse ().transform (r);
			bitmap->CBitmap::draw (context, r);
			CNinePartTiledBitmap* nptBitmap = dynamic_cast<CNinePartTiledBitmap*>(bitmap);
			if (nptBitmap)
			{
				static const CCoord kDefaultOnOffDashLength2[] = {2, 2};
				const CLineStyle kLineOnOffDash2 (CLineStyle::kLineCapButt, CLineStyle::kLineJoinMiter, 0, 2, kDefaultOnOffDashLength2);

				const CNinePartTiledDescription& offsets = nptBitmap->getPartOffsets ();

				CRect r;
				r.setWidth (nptBitmap->getWidth());
				r.setHeight (nptBitmap->getHeight());
				CPoint p = getViewSize ().getTopLeft ();
				matrix.inverse ().transform (p);
				r.offset (p.x, p.y);

				context->setDrawMode (kAntiAliasing);
				context->setFrameColor (kBlueCColor);
				context->setLineWidth (1);
				context->setLineStyle (kLineSolid);
				
				context->drawLine (CPoint (r.left, r.top + offsets.top), CPoint (r.right, r.top + offsets.top));
				context->drawLine (CPoint (r.left, r.bottom - offsets.bottom), CPoint (r.right, r.bottom - offsets.bottom));
				context->drawLine (CPoint (r.left + offsets.left, r.top), CPoint (r.left + offsets.left, r.bottom));
				context->drawLine (CPoint (r.right - offsets.right, r.top), CPoint (r.right - offsets.right, r.bottom));

				context->setFrameColor (kRedCColor);
				context->setLineWidth (1);
				context->setLineStyle (kLineOnOffDash2);

				context->drawLine (CPoint (r.left, r.top + offsets.top), CPoint (r.right, r.top + offsets.top));
				context->drawLine (CPoint (r.left, r.bottom - offsets.bottom), CPoint (r.right, r.bottom - offsets.bottom));
				context->drawLine (CPoint (r.left + offsets.left, r.top), CPoint (r.left + offsets.left, r.bottom));
				context->drawLine (CPoint (r.right - offsets.right, r.top), CPoint (r.right - offsets.right, r.bottom));
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

	void setBackground (CBitmap *background) VSTGUI_OVERRIDE_VMETHOD
	{
		IPlatformBitmap* platformBitmap = background ? background->getPlatformBitmap () : 0;
		if (platformBitmap && platformBitmap->getScaleFactor () != 1.)
		{
			// get rid of the scale factor
			void* ptr;
			uint32_t memSize;
			if (IPlatformBitmap::createMemoryPNGRepresentation (platformBitmap, &ptr, memSize))
			{
				SharedPointer<IPlatformBitmap> newPlatformBitmap = owned (IPlatformBitmap::createFromMemory (ptr, memSize));
				CView::setBackground (owned (new CBitmap (newPlatformBitmap)));
				std::free (ptr);
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
	UIBitmapsDataSource (UIDescription* description, IActionPerformer* actionPerformer, IGenericStringListDataBrowserSourceSelectionChanged* delegate);
	
	CBitmap* getSelectedBitmap ();
	UTF8StringPtr getSelectedBitmapName ();

	bool add () VSTGUI_OVERRIDE_VMETHOD;
protected:
	void getNames (std::list<const std::string*>& names) VSTGUI_OVERRIDE_VMETHOD;
	bool addItem (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD;
	bool removeItem (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD;
	bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) VSTGUI_OVERRIDE_VMETHOD;
	UTF8StringPtr getDefaultsName () VSTGUI_OVERRIDE_VMETHOD { return "UIBitmapsDataSource"; }

	bool addBitmap (UTF8StringPtr path, std::string& outName);

	void dbOnDragEnterBrowser (IDataPackage* drag, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;
	void dbOnDragExitBrowser (IDataPackage* drag, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;
	void dbOnDragEnterCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;
	void dbOnDragExitCell (int32_t row, int32_t column, IDataPackage* drag, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;
	bool dbOnDropInCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;

	SharedPointer<CColorChooser> colorChooser;
	bool dragContainsBitmaps;
};

//----------------------------------------------------------------------------------------------------
UIBitmapsDataSource::UIBitmapsDataSource (UIDescription* description, IActionPerformer* actionPerformer, IGenericStringListDataBrowserSourceSelectionChanged* delegate)
: UIBaseDataSource (description, actionPerformer, UIDescription::kMessageBitmapChanged, delegate)
, dragContainsBitmaps (false)
{
}

//----------------------------------------------------------------------------------------------------
void UIBitmapsDataSource::dbOnDragEnterBrowser (IDataPackage* drag, CDataBrowser* browser)
{
	uint32_t index = 0;
	IDataPackage::Type type;
	const void* item = 0;
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
void UIBitmapsDataSource::dbOnDragEnterCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser)
{
#if DEBUG
	DebugPrint ("enter cell: %d-%d\n", row, column);
#endif
}

//----------------------------------------------------------------------------------------------------
void UIBitmapsDataSource::dbOnDragExitCell (int32_t row, int32_t column, IDataPackage* drag, CDataBrowser* browser)
{
#if DEBUG
	DebugPrint ("exit cell: %d-%d\n", row, column);
#endif
}

//----------------------------------------------------------------------------------------------------
bool UIBitmapsDataSource::dbOnDropInCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser)
{
	if (dragContainsBitmaps)
	{
		bool didBeganGroupAction = false;
		uint32_t index = 0;
		IDataPackage::Type type;
		const void* item = 0;
		while (drag->getData (index++, item, type) > 0)
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
						if (!didBeganGroupAction)
						{
							actionPerformer->beginGroupAction ("Add Bitmaps");
							didBeganGroupAction = true;
						}
						std::string name;
						addBitmap (static_cast<UTF8StringPtr> (item), name);
					}
				}
			}
		}
		if (didBeganGroupAction)
			actionPerformer->finishGroupAction ();
		dragContainsBitmaps = false;
		return true;
	}
	return false;
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
		return description->getBitmap (names.at (static_cast<uint32_t> (selectedRow)).c_str ());
	return 0;
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr UIBitmapsDataSource::getSelectedBitmapName ()
{
	int32_t selectedRow = dataBrowser ? dataBrowser->getSelectedRow() : CDataBrowser::kNoSelection;
	if (selectedRow != CDataBrowser::kNoSelection && selectedRow < (int32_t)names.size ())
		return names.at (static_cast<uint32_t> (selectedRow)).c_str ();
	return 0;
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
		actionPerformer->performBitmapChange (outName.c_str (), pathStr.c_str ());
		result = true;
	}
	return result;
}

//----------------------------------------------------------------------------------------------------
bool UIBitmapsDataSource::add ()
{
	bool result = false;
	OwningPointer<CNewFileSelector> fs = CNewFileSelector::create (dataBrowser->getFrame ());
	if (fs)
	{
		fs->addFileExtension (CFileExtension ("PNG", "PNG", "image/png"));
		fs->setAllowMultiFileSelection (true);
		if (fs->runModal ())
		{
			uint32_t numFiles = static_cast<uint32_t> (fs->getNumSelectedFiles ());
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
						int32_t row = selectName (newName.c_str ());
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
class UIBitmapSettingsController : public CBaseObject, public IController
{
public:
	UIBitmapSettingsController (CBitmap* bitmap, const std::string& bitmapName, UIDescription* description, IActionPerformer* actionPerformer);
	~UIBitmapSettingsController ();

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
	void controlBeginEdit (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
	void controlEndEdit (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
protected:
	void updateNinePartTiledControls ();
	static bool stringToValue (UTF8StringPtr txt, float& result, CTextEdit::StringToValueUserData* userData);
	static bool valueToString (float value, char utf8String[256], CParamDisplay::ValueToStringUserData* userData);

	IActionPerformer* actionPerformer;
	SharedPointer<UIDescription> editDescription;
	SharedPointer<CBitmap> bitmap;
	SharedPointer<UIBitmapView> bitmapView;
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
, bitmapName (bitmapName)
, editDescription (description)
, actionPerformer (actionPerformer)
, origOffsets (10, 10, 10, 10)
{
	for (int32_t i = 0; i < kNumTags; i++)
		controls[i] = 0;
}

//----------------------------------------------------------------------------------------------------
UIBitmapSettingsController::~UIBitmapSettingsController ()
{
}

//----------------------------------------------------------------------------------------------------
void UIBitmapSettingsController::updateNinePartTiledControls ()
{
	CNinePartTiledBitmap* nptb = bitmap.cast<CNinePartTiledBitmap> ();
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
			CTextLabel* label = dynamic_cast<CTextLabel*>(controls[i]);
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
			CTextEdit* edit = dynamic_cast<CTextEdit*>(control);
			if (edit)
			{
				actionPerformer->performBitmapChange (bitmapName.c_str (), edit->getText ());
				bitmap = editDescription->getBitmap (bitmapName.c_str ());
				bitmapView->setBackground (bitmap);
				updateNinePartTiledControls ();
			}
			break;
		}
		case kNinePartTiledTag:
		{
			CNinePartTiledBitmap* nptb = bitmap.cast<CNinePartTiledBitmap> ();
			if (nptb)
			{
				origOffsets.left = nptb->getPartOffsets ().left;
				origOffsets.top = nptb->getPartOffsets ().top;
				origOffsets.right = nptb->getPartOffsets ().right;
				origOffsets.bottom = nptb->getPartOffsets ().bottom;
			}
			bool checked = control->getValue () == control->getMax ();
			actionPerformer->performBitmapNinePartTiledChange (bitmapName.c_str (), checked ? &origOffsets : 0);
			bitmap = editDescription->getBitmap (bitmapName.c_str ());
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
			actionPerformer->performBitmapNinePartTiledChange (bitmapName.c_str (), &r);
			bitmap = editDescription->getBitmap (bitmapName.c_str ());
			bitmapView->setBackground (bitmap);
			updateNinePartTiledControls ();
#if VSTGUI_HAS_FUNCTIONAL
			SharedPointer<CTextEdit> textEdit = SharedPointer<CControl> (control).cast<CTextEdit> ();
			if (textEdit && textEdit->bWasReturnPressed)
			{
				Call::later([textEdit] () {
					textEdit->takeFocus();
				});
			}
#endif
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
CMessageResult UIBitmapSettingsController::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == UIDialogController::kMsgDialogButton1Clicked)
	{
		return kMessageNotified;
	}
	else if (message == UIDialogController::kMsgDialogShow)
	{
		bitmapView->setBackground (bitmap);
		updateNinePartTiledControls ();
		return kMessageNotified;
	}
	return kMessageUnknown;
}

//----------------------------------------------------------------------------------------------------
CView* UIBitmapSettingsController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	CControl* control = dynamic_cast<CControl*>(view);
	if (control && control->getTag () >= 0 && control->getTag () < kNumTags)
	{
		controls[control->getTag ()] = control;
		switch (control->getTag ())
		{
			case kBitmapPathTag:
			{
				CTextEdit* bitmapPathEdit = dynamic_cast<CTextEdit*> (control);
				if (bitmapPathEdit)
					bitmapPathEdit->setText (bitmap->getResourceDescription ().u.name);
				break;
			}
			case kBitmapWidthTag:
			{
				CTextLabel* label = dynamic_cast<CTextLabel*>(control);
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
				CTextLabel* label = dynamic_cast<CTextLabel*>(control);
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
				CTextEdit* textEdit = dynamic_cast<CTextEdit*>(control);
				if (textEdit)
				{
					textEdit->setPrecision (0);
				#if VSTGUI_HAS_FUNCTIONAL
					textEdit->setStringToValueFunction (stringToValue);
				#else
					textEdit->setStringToValueProc (stringToValue);
				#endif
				}
				control->setMax ((float)bitmap->getWidth ());
				break;
			}
			case kNinePartTiledTopTag:
			case kNinePartTiledBottomTag:
			{
				CTextEdit* textEdit = dynamic_cast<CTextEdit*>(control);
				if (textEdit)
				{
					textEdit->setPrecision (0);
				#if VSTGUI_HAS_FUNCTIONAL
					textEdit->setStringToValueFunction (stringToValue);
				#else
					textEdit->setStringToValueProc (stringToValue);
				#endif
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
				CTextLabel* label = dynamic_cast<CTextLabel*>(control);
				if (label)
				{
				#if VSTGUI_HAS_FUNCTIONAL
					label->setValueToStringFunction (valueToString);
				#else
					label->setValueToStringProc (valueToString);
				#endif
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
	return 0;
}

//----------------------------------------------------------------------------------------------------
bool UIBitmapSettingsController::valueToString (float value, char utf8String[256], CParamDisplay::ValueToStringUserData* userData)
{
	int32_t intValue = (int32_t)value;
	std::stringstream str;
	str << intValue;
	str << "%";
	std::strcpy (utf8String, str.str ().c_str ());
	return true;
}


//----------------------------------------------------------------------------------------------------
bool UIBitmapSettingsController::stringToValue (UTF8StringPtr txt, float& result, CTextEdit::StringToValueUserData* userData)
{
	int32_t value = txt ? (int32_t)strtol (txt, 0, 10) : 0;
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
void UIBitmapsController::showSettingsDialog ()
{
	UIDialogController* dc = new UIDialogController (this, bitmapPathEdit->getFrame ());
	UIBitmapSettingsController* fsController = new UIBitmapSettingsController (dataSource->getSelectedBitmap (), dataSource->getSelectedBitmapName (), editDescription, actionPerformer);
	dc->run ("bitmap.settings", "Bitmap Settings", "Close", 0, fsController, &UIEditController::getEditorDescription ());
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
		CControl* control = dynamic_cast<CControl*> (view);
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
				CTextEdit* edit = dynamic_cast<CTextEdit*>(pControl);
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
			bitmapPathEdit->setText (bitmap ? bitmap->getResourceDescription ().u.name : 0);
			bitmapPathEdit->setMouseEnabled (selectedBitmapName ? true : false);
		}
		if (settingButton)
		{
			settingButton->setMouseEnabled (selectedBitmapName ? true : false);
		}
	}
}

//----------------------------------------------------------------------------------------------------
bool UIBitmapsController::valueToString (float value, char utf8String[256], void* userData)
{
	int32_t intValue = (int32_t)value;
	std::stringstream str;
	str << intValue;
	std::strcpy (utf8String, str.str ().c_str ());
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
