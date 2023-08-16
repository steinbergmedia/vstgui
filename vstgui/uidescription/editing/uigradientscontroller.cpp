// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
#include "../../lib/cdrawcontext.h"
#include "../../lib/events.h"
#include <algorithm>

namespace VSTGUI {

//------------------------------------------------------------------------
struct IUIColorStopEditViewListener
{
	virtual ~IUIColorStopEditViewListener () noexcept = default;
	virtual void onChange () = 0;
};

//------------------------------------------------------------------------
class UIColorStopEditView
: public CView,
  protected ListenerProvider<UIColorStopEditView, IUIColorStopEditViewListener>,
  public IFocusDrawing,
  public UIColorListenerAdapter
{
public:
	UIColorStopEditView (UIColor* editColor);
	~UIColorStopEditView () override;

	void setGradient (CGradient* gradient);
	
	void setCurrentStartOffset (double startOffset);
	double getSelectedColorStart () const { return editStartOffset; }
	const GradientColorStopMap& getColorStopMap () const { return colorStopMap; }

	using ListenerProvider<UIColorStopEditView, IUIColorStopEditViewListener>::registerListener;
	using ListenerProvider<UIColorStopEditView, IUIColorStopEditViewListener>::unregisterListener;
private:
	void draw (CDrawContext* context) override;
	bool drawFocusOnTop () override;
	bool getFocusPath (CGraphicsPath& outPath) override;
	void onKeyboardEvent (KeyboardEvent& event) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	void uiColorChanged (UIColor* c) override;

	double gradientStartPosFromMousePos (const CPoint& where) const;

	void addColorStop (double startOffset);
	void removeColorStop (double startOffset);
	void selectNextColorStop ();
	void selectPrevColorStop ();

	SharedPointer<UIColor> editColor;
	SharedPointer<CGradient> gradient;
	GradientColorStopMap colorStopMap;
	double editStartOffset;
	CCoord stopWidth;
	
	double mouseDownStartPosOffset;
};

//----------------------------------------------------------------------------------------------------
UIColorStopEditView::UIColorStopEditView (UIColor* editColor)
: CView (CRect (0, 0, 0, 0))
, editColor (editColor)
, editStartOffset (-1.)
, mouseDownStartPosOffset (0.)
{
	editColor->registerListener (this);
	setWantsFocus (true);
	stopWidth = 12.;
}

//----------------------------------------------------------------------------------------------------
UIColorStopEditView::~UIColorStopEditView ()
{
	editColor->unregisterListener (this);
}

//----------------------------------------------------------------------------------------------------
void UIColorStopEditView::selectNextColorStop ()
{
	auto pos = colorStopMap.find (getSelectedColorStart ());
	pos++;
	if (pos == colorStopMap.end ())
	{
		pos = colorStopMap.begin ();
	}
	editStartOffset = pos->first;
	*editColor = pos->second;
	forEachListener ([] (IUIColorStopEditViewListener* l) { l->onChange (); });
	invalid ();
}

//----------------------------------------------------------------------------------------------------
void UIColorStopEditView::selectPrevColorStop ()
{
	auto pos = colorStopMap.find (getSelectedColorStart ());
	if (pos == colorStopMap.begin ())
		pos = colorStopMap.end ();
	pos--;
	editStartOffset = pos->first;
	*editColor = pos->second;
	forEachListener ([] (IUIColorStopEditViewListener* l) { l->onChange (); });
	invalid ();
}

//----------------------------------------------------------------------------------------------------
void UIColorStopEditView::setCurrentStartOffset (double startOffset)
{
	if (startOffset < 0.)
		startOffset = 0.;
	else if (startOffset > 1.)
		startOffset = 1.;
	auto pos = colorStopMap.find (getSelectedColorStart ());
	if (pos != colorStopMap.end () && pos->first != startOffset)
	{
		CColor color = pos->second;
		colorStopMap.erase (pos);
		colorStopMap.emplace (startOffset, color);
		editStartOffset = startOffset;
		forEachListener ([] (IUIColorStopEditViewListener* l) { l->onChange (); });
		invalid ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorStopEditView::addColorStop (double startOffset)
{
	colorStopMap.emplace (startOffset, editColor->base ());
	editStartOffset = startOffset;
	forEachListener ([] (IUIColorStopEditViewListener* l) { l->onChange (); });
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
	forEachListener ([] (IUIColorStopEditViewListener* l) { l->onChange (); });
	invalid ();
}

//----------------------------------------------------------------------------------------------------
void UIColorStopEditView::onKeyboardEvent (KeyboardEvent& event)
{
	if (event.type != EventType::KeyDown)
		return;
	switch (event.virt)
	{
		case VirtualKey::Left:
		{
			if (event.modifiers.empty ())
			{
				selectPrevColorStop ();
				event.consumed = true;
			}
			else if (event.modifiers.is (ModifierKey::Alt))
			{
				setCurrentStartOffset (editStartOffset - 0.001);
				event.consumed = true;
			}
			break;
		}
		case VirtualKey::Right:
		{
			if (event.modifiers.empty ())
			{
				selectNextColorStop ();
				event.consumed = true;
			}
			else if (event.modifiers.is(ModifierKey::Alt))
			{
				setCurrentStartOffset (editStartOffset + 0.001);
				event.consumed = true;
			}
			break;
		}
		case VirtualKey::Back:
		{
			if (event.modifiers.empty ())
			{
				removeColorStop (getSelectedColorStart ());
				event.consumed = true;
			}
			break;
		}
		default:
			break;
	}
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
		getFrame ()->setFocusView (this);
		double pos = gradientStartPosFromMousePos (where);
		double range = (stopWidth / getWidth ()) / 2.;
		for (auto& colorStop : colorStopMap)
		{
			if (pos >= colorStop.first - range && pos <= colorStop.first + range)
			{
				if (buttons.getModifierState () == kAlt)
				{
					removeColorStop (colorStop.first);
					return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
				}
				if (editStartOffset != colorStop.first)
				{
					editStartOffset = colorStop.first;
					*editColor = colorStop.second;
					forEachListener ([] (IUIColorStopEditViewListener* l) { l->onChange (); });
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
void UIColorStopEditView::uiColorChanged (UIColor* c)
{
	invalid ();
}

//----------------------------------------------------------------------------------------------------
void UIColorStopEditView::setGradient (CGradient* inGradient)
{
	colorStopMap = inGradient->getColorStops ();
	auto it = colorStopMap.find (editStartOffset);
	if (it == colorStopMap.end ())
		editStartOffset = colorStopMap.begin ()->first;
	gradient = inGradient;
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
	for (auto& colorStop : colorStopMap)
	{
		if (colorStop.first == getSelectedColorStart ())
		{
			selectedColor = colorStop.second;
		}
		else
		{
			CGraphicsTransform offset;
			offset.translate (colorStop.first * width, getHeight () / 4.);
			if (colorStop.second.getLuma () < 127)
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
class UIGradientEditorController : public NonAtomicReferenceCounted,
                                   public IDialogController,
                                   public UIColorListenerAdapter,
                                   public IUIColorStopEditViewListener,
                                   public IController
{
public:
	UIGradientEditorController (const std::string& gradientName, CGradient* gradient, UIDescription* description, IActionPerformer* actionPerformer);
	~UIGradientEditorController () override;

	void valueChanged (CControl* pControl) override;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override;
	IController* createSubController (UTF8StringPtr name, const IUIDescription* description) override;
	void onDialogButton1Clicked (UIDialogController*) override;
	void onDialogButton2Clicked (UIDialogController*) override;
	void onDialogShow (UIDialogController*) override;
protected:
	enum {
		kApplyTag = 1,
		kPositionTag = 2,
	};
	void uiColorChanged (UIColor* c) override;
	void onChange () override;
	void apply ();
	void updatePositionEdit ();
	
	SharedPointer<UIDescription> editDescription;
	SharedPointer<UIColorStopEditView> colorStopEditView;
	SharedPointer<CGradient> gradient;
	SharedPointer<UIColor> editColor;
	CTextEdit* positionEdit {nullptr};
	IActionPerformer* actionPerformer;
	std::string gradientName;
};

//----------------------------------------------------------------------------------------------------
UIGradientEditorController::UIGradientEditorController (const std::string& gradientName, CGradient* gradient, UIDescription* description, IActionPerformer* actionPerformer)
: editDescription(description)
, gradient (gradient)
, editColor (makeOwned<UIColor> ())
, actionPerformer(actionPerformer)
, gradientName (gradientName)
{
	*editColor = gradient->getColorStops ().begin ()->second;
	editColor->registerListener (this);
}

//----------------------------------------------------------------------------------------------------
UIGradientEditorController::~UIGradientEditorController ()
{
	colorStopEditView->unregisterListener (this);
	editColor->unregisterListener (this);
}

//----------------------------------------------------------------------------------------------------
void UIGradientEditorController::apply ()
{
	CGradient* g = editDescription->getGradient (gradientName.data ());
	if (g->getColorStops () != gradient->getColorStops ())
		actionPerformer->performGradientChange (gradientName.data (), gradient);
}

//----------------------------------------------------------------------------------------------------
void UIGradientEditorController::updatePositionEdit ()
{
	if (positionEdit && colorStopEditView)
		positionEdit->setValue (static_cast<float> (colorStopEditView->getSelectedColorStart ()));
}

//----------------------------------------------------------------------------------------------------
void UIGradientEditorController::uiColorChanged (UIColor* c)
{
	auto colorStopMap = gradient->getColorStops (); // create a copy
	auto it = colorStopMap.find (colorStopEditView->getSelectedColorStart ());
	if (it != colorStopMap.end () && it->second != editColor->base ())
	{
		it->second = editColor->base ();
		gradient = CGradient::create (colorStopMap);
		colorStopEditView->setGradient (gradient);
		updatePositionEdit ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIGradientEditorController::onChange ()
{
	gradient = CGradient::create (colorStopEditView->getColorStopMap ());
	colorStopEditView->setGradient (gradient);
	updatePositionEdit ();
}

//------------------------------------------------------------------------
void UIGradientEditorController::onDialogButton1Clicked (UIDialogController*)
{
	apply ();
}

//------------------------------------------------------------------------
void UIGradientEditorController::onDialogButton2Clicked (UIDialogController*)
{
}

//------------------------------------------------------------------------
void UIGradientEditorController::onDialogShow (UIDialogController*)
{
}

//----------------------------------------------------------------------------------------------------
IController* UIGradientEditorController::createSubController (UTF8StringPtr name, const IUIDescription* description)
{
	if (UTF8StringView (name) == "ColorChooserController")
	{
		return new UIColorChooserController (this, editColor);
	}
	return nullptr;
}

//----------------------------------------------------------------------------------------------------
void UIGradientEditorController::valueChanged (CControl* pControl)
{
	switch (pControl->getTag ())
	{
		case kApplyTag:
		{
			if (pControl->getValue () > 0.f)
				apply ();
			break;
		}
		case kPositionTag:
		{
			colorStopEditView->setCurrentStartOffset (pControl->getValue ());
			break;
		}
	}
}

//----------------------------------------------------------------------------------------------------
CView* UIGradientEditorController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	if (auto control = dynamic_cast<CTextEdit*>(view))
	{
		if (control->getTag () == kPositionTag)
		{
			control->setStringToValueFunction ([] (UTF8StringPtr txt, float& result, CTextEdit*) {
				UTF8StringView t (txt);
				result = t.toFloat ();
				return true;
			});
			positionEdit = control;
			updatePositionEdit ();
		}
	}
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
			colorStopEditView->registerListener (this);
			return colorStopEditView;
		}
	}
	return nullptr;
}

//----------------------------------------------------------------------------------------------------
class UIGradientsDataSource : public UIBaseDataSource
{
public:
	UIGradientsDataSource (UIDescription* description, IActionPerformer* actionPerformer, GenericStringListDataBrowserSourceSelectionChanged* delegate);
	~UIGradientsDataSource () override = default;

	CGradient* getSelectedGradient ();
	std::string getSelectedGradientName ();
	
protected:
	void onUIDescGradientChanged (UIDescription* desc) override;
	void update () override;
	void getNames (std::list<const std::string*>& names) override;
	bool addItem (UTF8StringPtr name) override;
	bool removeItem (UTF8StringPtr name) override;
	bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) override;
	UTF8StringPtr getDefaultsName () override { return "UIGradientsDataSource"; }

	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser) override;
	void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* control, CDataBrowser* browser) override;
	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser) override;

	CCoord getGradientIconWidth ();
};

//----------------------------------------------------------------------------------------------------
UIGradientsDataSource::UIGradientsDataSource (
    UIDescription* description, IActionPerformer* actionPerformer,
    GenericStringListDataBrowserSourceSelectionChanged* delegate)
: UIBaseDataSource (description, actionPerformer, delegate)
{
}

//----------------------------------------------------------------------------------------------------
void UIGradientsDataSource::onUIDescGradientChanged (UIDescription* desc)
{
	onUIDescriptionUpdate ();
}

//----------------------------------------------------------------------------------------------------
CGradient* UIGradientsDataSource::getSelectedGradient ()
{
	int32_t selectedRow = dataBrowser ? dataBrowser->getSelectedRow() : CDataBrowser::kNoSelection;
	if (selectedRow != CDataBrowser::kNoSelection && selectedRow < (int32_t)names.size ())
		return description->getGradient (names.at (static_cast<uint32_t> (selectedRow)).data ());
	return nullptr;
}

//------------------------------------------------------------------------
std::string UIGradientsDataSource::getSelectedGradientName ()
{
	int32_t selectedRow = dataBrowser ? dataBrowser->getSelectedRow() : CDataBrowser::kNoSelection;
	if (selectedRow != CDataBrowser::kNoSelection && selectedRow < (int32_t)names.size ())
		return names[static_cast<uint32_t> (selectedRow)].getString ();
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
	actionPerformer->performGradientChange (name, nullptr, true);
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
CCoord UIGradientsDataSource::getGradientIconWidth ()
{
	return dataBrowser ? dbGetRowHeight (dataBrowser) * 2. : 0.;
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIGradientsDataSource::dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
{
	if (buttons.isDoubleClick () && row >= 0 && row < static_cast<int32_t> (names.size ()))
	{
		auto r = browser->getCellBounds ({row, column});
		r.left = r.right - getGradientIconWidth ();
		if (r.pointInside (where))
		{
			delegate->dbRowDoubleClick (row, this);
			return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		}
	}
	return UIBaseDataSource::dbOnMouseDown (where, buttons, row, column, browser);
}

//----------------------------------------------------------------------------------------------------
void UIGradientsDataSource::dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser)
{
	GenericStringListDataBrowserSource::drawRowBackground (context, size, row, flags, browser);
	CRect r (size);
	r.right -= getGradientIconWidth ();
	GenericStringListDataBrowserSource::drawRowString (context, r, row, flags, browser);
	CGradient* gradient = nullptr;
	if ((gradient = description->getGradient (names.at (static_cast<uint32_t> (row)).data ())))
	{
		context->setFrameColor (kBlackCColor);
		context->setLineWidth (context->getHairlineSize ());
		context->setLineStyle (kLineSolid);
		context->setDrawMode (kAliasing);
		r = size;
		r.left = r.right - (getGradientIconWidth ());
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
	r.right -= getGradientIconWidth ();
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
, dataSource (nullptr)
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
		auto searchField = dynamic_cast<CSearchTextEdit*> (control);
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
void UIGradientsController::dbRowDoubleClick (int32_t row, GenericStringListDataBrowserSource* source)
{
	showEditDialog ();
}

//----------------------------------------------------------------------------------------------------
void UIGradientsController::showEditDialog ()
{
	UIDialogController* dc = new UIDialogController (this, editButton->getFrame ());
	auto fsController = makeOwned<UIGradientEditorController> (
	    dataSource->getSelectedGradientName (), dataSource->getSelectedGradient (), editDescription,
	    actionPerformer);
	dc->run ("gradient.editor", "Gradient Editor", "OK", "Cancel", fsController,
	         UIEditController::getEditorDescription ());
}

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
