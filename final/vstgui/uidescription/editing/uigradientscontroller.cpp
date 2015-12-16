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

#include "uigradientscontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uibasedatasource.h"
#include "iaction.h"
#include "uieditcontroller.h"
#include "uidialogcontroller.h"
#include "uicolorchoosercontroller.h"
#include "uicolor.h"
#include "../../lib/cgradientview.h"
#include "../../lib/ifocusdrawing.h"
#include "../../lib/cgraphicspath.h"
#include <algorithm>

namespace VSTGUI {

//------------------------------------------------------------------------
class UIColorStopEditView : public CView, public IDependency, public IFocusDrawing
{
public:
	UIColorStopEditView (UIColor* editColor);
	~UIColorStopEditView ();

	void setGradient (CGradient* gradient);
	
	double getSelectedColorStart () const { return editStartOffset; }
	const CGradient::ColorStopMap& getColorStopMap () const { return colorStopMap; }

	static IdStringPtr kChanged;
private:
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;
	void draw (CDrawContext* context) VSTGUI_OVERRIDE_VMETHOD;
	bool drawFocusOnTop () VSTGUI_OVERRIDE_VMETHOD;
	bool getFocusPath (CGraphicsPath& outPath) VSTGUI_OVERRIDE_VMETHOD;
	int32_t onKeyDown (VstKeyCode& keyCode) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;

	double gradientStartPosFromMousePos (const CPoint& where) const;

	void addColorStop (double startOffset);
	void removeColorStop (double startOffset);
	void setCurrentStartOffset (double startOffset);
	void selectNextColorStop ();
	void selectPrevColorStop ();

	SharedPointer<UIColor> editColor;
	SharedPointer<CGradient> gradient;
	CGradient::ColorStopMap colorStopMap;
	double editStartOffset;
	CCoord stopWidth;
	
	double mouseDownStartPosOffset;
};

IdStringPtr UIColorStopEditView::kChanged = "UIColorStopEditView::kChanged";
//----------------------------------------------------------------------------------------------------
UIColorStopEditView::UIColorStopEditView (UIColor* editColor)
: CView (CRect (0, 0, 0, 0))
, editColor (editColor)
, editStartOffset (-1.)
, mouseDownStartPosOffset (0.)
{
	editColor->addDependency (this);
	setWantsFocus (true);
	stopWidth = 12.;
}

//----------------------------------------------------------------------------------------------------
UIColorStopEditView::~UIColorStopEditView ()
{
	editColor->removeDependency (this);
}

//----------------------------------------------------------------------------------------------------
void UIColorStopEditView::selectNextColorStop ()
{
	CGradient::ColorStopMap::const_iterator pos = colorStopMap.find (getSelectedColorStart ());
	pos++;
	if (pos == colorStopMap.end ())
	{
		pos = colorStopMap.begin ();
	}
	editStartOffset = pos->first;
	*editColor = pos->second;
	invalid ();
}

//----------------------------------------------------------------------------------------------------
void UIColorStopEditView::selectPrevColorStop ()
{
	CGradient::ColorStopMap::const_iterator pos = colorStopMap.find (getSelectedColorStart ());
	if (pos == colorStopMap.begin ())
		pos = colorStopMap.end ();
	pos--;
	editStartOffset = pos->first;
	*editColor = pos->second;
	invalid ();
}

//----------------------------------------------------------------------------------------------------
void UIColorStopEditView::setCurrentStartOffset (double startOffset)
{
	if (startOffset < 0.)
		startOffset = 0.;
	else if (startOffset > 1.)
		startOffset = 1.;
	CGradient::ColorStopMap::iterator pos = colorStopMap.find (getSelectedColorStart ());
	if (pos != colorStopMap.end () && pos->first != startOffset)
	{
		CColor color = pos->second;
		colorStopMap.erase (pos);
		colorStopMap.insert (std::make_pair (startOffset, color));
		editStartOffset = startOffset;
		changed (kChanged);
		invalid ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorStopEditView::addColorStop (double startOffset)
{
	colorStopMap.insert (std::make_pair (startOffset, editColor->base ()));
	editStartOffset = startOffset;
	changed (kChanged);
	invalid ();
}

//----------------------------------------------------------------------------------------------------
void UIColorStopEditView::removeColorStop (double startOffset)
{
	if (colorStopMap.size () < 3)
		return;
	if (getSelectedColorStart () == startOffset)
		selectNextColorStop ();
	colorStopMap.erase (startOffset);
	changed (kChanged);
	invalid ();
}

//----------------------------------------------------------------------------------------------------
int32_t UIColorStopEditView::onKeyDown (VstKeyCode& keyCode)
{
	switch (keyCode.virt)
	{
		case VKEY_LEFT:
		{
			if (keyCode.modifier == 0)
			{
				selectPrevColorStop ();
				return 1;
			}
			else if (keyCode.modifier == MODIFIER_ALTERNATE)
			{
				setCurrentStartOffset (editStartOffset-0.001);
			}
			break;
		}
		case VKEY_RIGHT:
		{
			if (keyCode.modifier == 0)
			{
				selectNextColorStop ();
				return 1;
			}
			else if (keyCode.modifier == MODIFIER_ALTERNATE)
			{
				setCurrentStartOffset (editStartOffset+0.001);
			}
			break;
		}
		case VKEY_BACK:
		{
			if (keyCode.modifier == 0)
			{
				removeColorStop (getSelectedColorStart ());
				return 1;
			}
			break;
		}
	}
	return CView::onKeyDown (keyCode);
}

//----------------------------------------------------------------------------------------------------
double UIColorStopEditView::gradientStartPosFromMousePos (const CPoint& where) const
{
	return (where.x - (getViewSize ().left + (stopWidth / 2.))) / (getWidth () - stopWidth);
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIColorStopEditView::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isDoubleClick ())
	{
		double pos = gradientStartPosFromMousePos (where);
		if (pos >= 0. && pos <= 1.)
		{
			addColorStop (pos);
		}
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	else if (buttons.isLeftButton ())
	{
		double pos = gradientStartPosFromMousePos (where);
		double range = (stopWidth / getWidth ()) / 2.;
		for (CGradient::ColorStopMap::const_iterator it = colorStopMap.begin (), end = colorStopMap.end (); it != end; ++it)
		{
			if (pos >= it->first - range && pos <= it->first + range)
			{
				if (buttons.getModifierState () == kAlt)
				{
					removeColorStop (it->first);
					return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
				}
				if (editStartOffset != it->first)
				{
					editStartOffset = it->first;
					*editColor = it->second;
				}
				mouseDownStartPosOffset = pos - editStartOffset;
				return kMouseEventHandled;
			}
		}
	}
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIColorStopEditView::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	return kMouseEventHandled;
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIColorStopEditView::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		double pos = gradientStartPosFromMousePos (where) - mouseDownStartPosOffset;
		setCurrentStartOffset (pos);
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIColorStopEditView::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == UIColor::kMsgChanged || message == UIColor::kMsgEditChange)
	{
		invalid ();
		return kMessageNotified;
	}
	return CView::notify (sender, message);
}

//----------------------------------------------------------------------------------------------------
void UIColorStopEditView::setGradient (CGradient* gradient)
{
	colorStopMap = gradient->getColorStops ();
	CGradient::ColorStopMap::const_iterator it = colorStopMap.find (editStartOffset);
	if (it == colorStopMap.end ())
		editStartOffset = colorStopMap.begin ()->first;
	this->gradient = gradient;
}

//----------------------------------------------------------------------------------------------------
void UIColorStopEditView::draw (CDrawContext* context)
{
	CDrawContext::Transform t (*context, CGraphicsTransform ().translate (getViewSize ().left, getViewSize ().top));

	context->setDrawMode (kAliasing);

	SharedPointer<CGraphicsPath> gradientPath = owned (context->createGraphicsPath ());
	gradientPath->addRect (CRect (stopWidth / 2., 0., getWidth () - stopWidth / 2., getHeight ()));
	context->fillLinearGradient (gradientPath, *gradient, CPoint (stopWidth / 2., 0), CPoint (getWidth () - stopWidth / 2, 0));

	CCoord width = getWidth () - stopWidth;
	CCoord height = (getHeight () / 2.);
	SharedPointer<CGraphicsPath> path = owned (context->createGraphicsPath ());
	path->beginSubpath (CPoint (stopWidth / 2., 0));
	path->addLine (CPoint (0, height));
	path->addLine (CPoint (stopWidth, height));
	path->closeSubpath ();

	context->setFrameColor (kBlackCColor);
	context->setLineWidth (1.1);
	context->setLineStyle (kLineSolid);
	context->setDrawMode (kAntiAliasing);

	CColor selectedColor;

	context->setGlobalAlpha (0.5f);
	for (CGradient::ColorStopMap::const_iterator it = colorStopMap.begin (), end = colorStopMap.end (); it != end; it++)
	{
		if (it->first == getSelectedColorStart ())
		{
			selectedColor = it->second;
		}
		else
		{
			CGraphicsTransform offset;
			offset.translate (it->first * width, getHeight () / 4.);
			if (it->second.getLuma () < 127)
				context->setFrameColor (kWhiteCColor);
			else
				context->setFrameColor (kBlackCColor);
			context->drawGraphicsPath (path, CDrawContext::kPathStroked, &offset);
		}
	}
	
	context->setGlobalAlpha (1.f);
	if (getSelectedColorStart () >= 0.)
	{
		CGraphicsTransform offset;
		offset.translate (getSelectedColorStart () * width, getHeight() / 4.);
		if (selectedColor.getLuma () < 127)
			context->setFrameColor (kWhiteCColor);
		else
			context->setFrameColor (kBlackCColor);
		context->setFillColor (selectedColor);
		context->drawGraphicsPath (path, CDrawContext::kPathFilled, &offset);
		context->drawGraphicsPath (path, CDrawContext::kPathStroked, &offset);
	}
}

//----------------------------------------------------------------------------------------------------
bool UIColorStopEditView::drawFocusOnTop ()
{
	return false;
}

//----------------------------------------------------------------------------------------------------
bool UIColorStopEditView::getFocusPath (CGraphicsPath& outPath)
{
	CRect r (getViewSize ());
	r.inset (stopWidth / 2 - 1, -1);
	outPath.addRect (r);
	r.inset (2, 2);
	outPath.addRect (r);
	return true;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class UIGradientEditorController : public CBaseObject, public IController
{
public:
	UIGradientEditorController (const std::string& gradientName, CGradient* gradient, UIDescription* description, IActionPerformer* actionPerformer);
	~UIGradientEditorController ();

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;
	void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	IController* createSubController (UTF8StringPtr name, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
protected:
	void apply ();
	
	IActionPerformer* actionPerformer;
	SharedPointer<UIDescription> editDescription;
	SharedPointer<UIColorStopEditView> colorStopEditView;
	SharedPointer<CGradient> gradient;
	SharedPointer<UIColor> editColor;
	std::string gradientName;
};

//----------------------------------------------------------------------------------------------------
UIGradientEditorController::UIGradientEditorController (const std::string& gradientName, CGradient* gradient, UIDescription* description, IActionPerformer* actionPerformer)
: editDescription(description)
, actionPerformer(actionPerformer)
, gradient (gradient)
, editColor (owned (new UIColor ()))
, gradientName (gradientName)
{
	*editColor = gradient->getColorStops ().begin ()->second;
	editColor->addDependency (this);
}

//----------------------------------------------------------------------------------------------------
UIGradientEditorController::~UIGradientEditorController ()
{
	colorStopEditView->removeDependency (this);
	editColor->removeDependency (this);
}

//----------------------------------------------------------------------------------------------------
void UIGradientEditorController::apply ()
{
	CGradient* g = editDescription->getGradient (gradientName.c_str ());
	if (g->getColorStops () != gradient->getColorStops ())
		actionPerformer->performGradientChange (gradientName.c_str (), gradient);
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIGradientEditorController::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == UIColor::kMsgChanged || message == UIColor::kMsgEditChange)
	{
		CGradient::ColorStopMap colorStopMap = gradient->getColorStops ();
		CGradient::ColorStopMap::iterator it = colorStopMap.find (colorStopEditView->getSelectedColorStart ());
		if (it != colorStopMap.end () && it->second != editColor->base ())
		{
			it->second = editColor->base ();
			gradient = CGradient::create (colorStopMap);
			colorStopEditView->setGradient (gradient);
		}
		return kMessageNotified;
	}
	else if (message == UIColorStopEditView::kChanged)
	{
		gradient = CGradient::create (colorStopEditView->getColorStopMap ());
		colorStopEditView->setGradient (gradient);
		return kMessageNotified;
	}
	else if (message == UIDialogController::kMsgDialogButton1Clicked)
	{
		apply ();
		return kMessageNotified;
	}
	return kMessageUnknown;
}

//----------------------------------------------------------------------------------------------------
IController* UIGradientEditorController::createSubController (UTF8StringPtr name, const IUIDescription* description)
{
	if (UTF8StringView (name) == "ColorChooserController")
	{
		return new UIColorChooserController (this, editColor);
	}
	return 0;
}

//----------------------------------------------------------------------------------------------------
void UIGradientEditorController::valueChanged (CControl* pControl)
{
	if (pControl->getTag () == 1 && pControl->getValue () > 0.f)
	{
		apply ();
	}
}

//----------------------------------------------------------------------------------------------------
CView* UIGradientEditorController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	return view;
}

//----------------------------------------------------------------------------------------------------
CView* UIGradientEditorController::createView (const UIAttributes& attributes, const IUIDescription* description)
{
	const std::string* name = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (name)
	{
		if (*name == "ColorStopEditView")
		{
			colorStopEditView = new UIColorStopEditView (editColor);
			colorStopEditView->setGradient (gradient);
			colorStopEditView->addDependency (this);
			return colorStopEditView;
		}
	}
	return 0;
}

//----------------------------------------------------------------------------------------------------
class UIGradientsDataSource : public UIBaseDataSource
{
public:
	UIGradientsDataSource (UIDescription* description, IActionPerformer* actionPerformer, IGenericStringListDataBrowserSourceSelectionChanged* delegate);
	~UIGradientsDataSource ();

	CGradient* getSelectedGradient ();
	std::string getSelectedGradientName ();
	
protected:
	virtual void update () VSTGUI_OVERRIDE_VMETHOD;
	virtual void getNames (std::list<const std::string*>& names) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool addItem (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool removeItem (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) VSTGUI_OVERRIDE_VMETHOD;
	virtual UTF8StringPtr getDefaultsName () VSTGUI_OVERRIDE_VMETHOD { return "UIGradientsDataSource"; }

	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;
	void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* control, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;
};

//----------------------------------------------------------------------------------------------------
UIGradientsDataSource::UIGradientsDataSource (UIDescription* description, IActionPerformer* actionPerformer, IGenericStringListDataBrowserSourceSelectionChanged* delegate)
: UIBaseDataSource (description, actionPerformer, UIDescription::kMessageGradientChanged, delegate)
{
}

//----------------------------------------------------------------------------------------------------
UIGradientsDataSource::~UIGradientsDataSource ()
{
}

//----------------------------------------------------------------------------------------------------
CGradient* UIGradientsDataSource::getSelectedGradient ()
{
	int32_t selectedRow = dataBrowser ? dataBrowser->getSelectedRow() : CDataBrowser::kNoSelection;
	if (selectedRow != CDataBrowser::kNoSelection && selectedRow < (int32_t)names.size ())
		return description->getGradient (names.at (static_cast<uint32_t> (selectedRow)).c_str ());
	return 0;
}

//------------------------------------------------------------------------
std::string UIGradientsDataSource::getSelectedGradientName ()
{
	int32_t selectedRow = dataBrowser ? dataBrowser->getSelectedRow() : CDataBrowser::kNoSelection;
	if (selectedRow != CDataBrowser::kNoSelection && selectedRow < (int32_t)names.size ())
		return names[static_cast<uint32_t> (selectedRow)];
	return "";
}

//----------------------------------------------------------------------------------------------------
void UIGradientsDataSource::getNames (std::list<const std::string*>& names)
{
	description->collectGradientNames (names);
}

//----------------------------------------------------------------------------------------------------
bool UIGradientsDataSource::addItem (UTF8StringPtr name)
{
	actionPerformer->performGradientChange (name, CGradient::create (0, 1, kWhiteCColor, kBlackCColor));
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIGradientsDataSource::removeItem (UTF8StringPtr name)
{
	actionPerformer->performGradientChange (name, 0, true);
	return true;
}

//----------------------------------------------------------------------------------------------------
void UIGradientsDataSource::update ()
{
	UIBaseDataSource::update ();
	if (dataBrowser)
	{
		dbSelectionChanged (dataBrowser);
		dataBrowser->invalid ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIGradientsDataSource::dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser)
{
	GenericStringListDataBrowserSource::dbDrawCell (context, size, row, column, flags, browser);
	CGradient* gradient = 0;
	if ((gradient = description->getGradient (names.at (static_cast<uint32_t> (row)).c_str ())))
	{
		context->setFrameColor (kBlackCColor);
		context->setLineWidth (1);
		context->setLineStyle (kLineSolid);
		context->setDrawMode (kAliasing);
		CRect r (size);
		r.left = r.right - (r.getHeight () * 2.);
		r.offset (-0.5, -0.5);
		r.inset (3, 2);
		SharedPointer<CGraphicsPath> path = owned (context->createGraphicsPath ());
		path->addRect (r);
		path->closeSubpath ();
		context->fillLinearGradient (path, *gradient, r.getTopLeft (), r.getTopRight ());
		context->drawGraphicsPath (path, CDrawContext::kPathStroked);
	}
}

//----------------------------------------------------------------------------------------------------
void UIGradientsDataSource::dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* control, CDataBrowser* browser)
{
	UIBaseDataSource::dbCellSetupTextEdit(row, column, control, browser);
	CRect r (control->getViewSize ());
	r.right -= r.getHeight () * 2.;
	control->setViewSize (r);
}


//----------------------------------------------------------------------------------------------------
bool UIGradientsDataSource::performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	actionPerformer->performGradientNameChange (oldName, newName);
	return true;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIGradientsController::UIGradientsController (IController* baseController, UIDescription* description, IActionPerformer* actionPerformer)
: DelegationController (baseController)
, editDescription (description)
, actionPerformer (actionPerformer)
, dataSource (0)
{
	dataSource = new UIGradientsDataSource (editDescription, actionPerformer, this);
	UIEditController::setupDataSource (dataSource);
}

//----------------------------------------------------------------------------------------------------
UIGradientsController::~UIGradientsController ()
{
	dataSource->forget ();
}

//----------------------------------------------------------------------------------------------------
CView* UIGradientsController::createView (const UIAttributes& attributes, const IUIDescription* description)
{
	const std::string* name = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (name)
	{
		if (*name == "GradientsBrowser")
		{
			CDataBrowser* dataBrowser = new CDataBrowser (CRect (0, 0, 0, 0), dataSource, CDataBrowser::kDrawRowLines|CScrollView::kHorizontalScrollbar|CScrollView::kVerticalScrollbar);
			return dataBrowser;
		}
	}
	return DelegationController::createView (attributes, description);
}

//----------------------------------------------------------------------------------------------------
CView* UIGradientsController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	CControl* control = dynamic_cast<CControl*> (view);
	if (control)
	{
		UISearchTextField* searchField = dynamic_cast<UISearchTextField*> (control);
		if (searchField && searchField->getTag () == kSearchTag)
		{
			dataSource->setSearchFieldControl (searchField);
			return searchField;
		}
		else if (control->getTag () == kEditTag)
		{
			editButton = control;
			editButton->setMouseEnabled (false);
		}
	}
	return controller->verifyView (view, attributes, description);
}

//----------------------------------------------------------------------------------------------------
IControlListener* UIGradientsController::getControlListener (UTF8StringPtr name)
{
	return this;
}

//----------------------------------------------------------------------------------------------------
void UIGradientsController::valueChanged (CControl* pControl)
{
	switch (pControl->getTag ())
	{
		case kAddTag:
		{
			if (pControl->getValue () == pControl->getMax ())
				dataSource->add ();
			break;
		}
		case kRemoveTag:
		{
			if (pControl->getValue () == pControl->getMax ())
				dataSource->remove ();
			break;
		}
		case kEditTag:
		{
			if (pControl->getValue () == pControl->getMax ())
				showEditDialog ();
			break;
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIGradientsController::dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source)
{
	if (dataSource)
	{
		CGradient* gradient = dataSource->getSelectedGradient ();
		if (editButton)
			editButton->setMouseEnabled (gradient ? true : false);
	}
}

//----------------------------------------------------------------------------------------------------
void UIGradientsController::showEditDialog ()
{
	UIDialogController* dc = new UIDialogController (this, editButton->getFrame ());
	UIGradientEditorController* fsController = new UIGradientEditorController (dataSource->getSelectedGradientName (), dataSource->getSelectedGradient (), editDescription, actionPerformer);
	dc->run ("gradient.editor", "Gradient Editor", "OK", "Cancel", fsController, &UIEditController::getEditorDescription ());
}

} // namespace

#endif // VSTGUI_LIVE_EDITING