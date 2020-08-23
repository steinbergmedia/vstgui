// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uieditcontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uieditview.h"
#include "uiattributescontroller.h"
#include "uibitmapscontroller.h"
#include "uicolorscontroller.h"
#include "uigradientscontroller.h"
#include "uieditmenucontroller.h"
#include "uifontscontroller.h"
#include "uigridcontroller.h"
#include "uitagscontroller.h"
#include "uitemplatecontroller.h"
#include "uiviewcreatecontroller.h"
#include "uiundomanager.h"
#include "uiactions.h"
#include "uiselection.h"
#include "uidialogcontroller.h"
#include "uitemplatesettingscontroller.h"
#include "uifocussettingscontroller.h"
#include "../cstream.h"
#include "../uiattributes.h"
#include "../uicontentprovider.h"
#include "../xmlparser.h"
#include "../../lib/controls/coptionmenu.h"
#include "../../lib/controls/csegmentbutton.h"
#include "../../lib/controls/csearchtextedit.h"
#include "../../lib/animation/animations.h"
#include "../../lib/animation/timingfunctions.h"
#include "../../lib/cdropsource.h"
#include "../../lib/cgraphicspath.h"
#include "../../lib/coffscreencontext.h"
#include "../../lib/iviewlistener.h"
#include "../../lib/cvstguitimer.h"

#include <sstream>
#include <algorithm>
#include <cassert>
#include <array>

#if WINDOWS
#define snprintf _snprintf
#endif

namespace VSTGUI {

#ifdef HAVE_EDITORUIDESC_H
#include "editoruidesc.h"
#endif

//----------------------------------------------------------------------------------------------------
class UIEditControllerDescription
{
public:
	SharedPointer<UIDescription> get () const
	{
		if (uiDesc == nullptr)
		{
#ifdef HAVE_EDITORUIDESC_H
			MemoryContentProvider provider (editorUIDesc, strlen (editorUIDesc));
			SharedPointer<UIDescription> editorDesc = owned (new UIDescription (&provider));
			if (editorDesc->parse ())
			{
				uiDesc = editorDesc;
			}
#else
			std::string descPath (__FILE__);
			unixfyPath (descPath);
			if (removeLastPathComponent (descPath))
			{
				descPath += "/uidescriptioneditor.uidesc";
				auto editorDesc = makeOwned<UIDescription> (descPath.c_str ());
				if (editorDesc->parse ())
				{
					uiDesc = std::move (editorDesc);
				}
				else
				{
					vstgui_assert (false, "the __FILE__ macro is relative, so it's not possible to find the uidescriptioneditor.uidesc. You can replace the macro with the absolute filename to make this work on your devel machine");
				}
			}
#endif
		}
		return uiDesc;
	}

	void tryFree ()
	{
		if (uiDesc->getNbReference () == 1)
			uiDesc = nullptr;
	}

private:
	mutable SharedPointer<UIDescription> uiDesc;
};

static UIEditControllerDescription gUIDescription;

//----------------------------------------------------------------------------------------------------
SharedPointer<UIDescription> UIEditController::getEditorDescription ()
{
	return gUIDescription.get ();
}

//----------------------------------------------------------------------------------------------------
void UIEditController::setupDataSource (GenericStringListDataBrowserSource* source)
{
	static CColor selectionColor;
	static CColor fontColor;
	static CColor rowlineColor;
	static CColor rowBackColor;
	static CColor rowAlternateBackColor;
	static bool once = true;
	auto editorDescription = UIEditController::getEditorDescription ();
	if (once)
	{
		editorDescription->getColor ("db.selection", selectionColor);
		editorDescription->getColor ("db.font", fontColor);
		editorDescription->getColor ("db.row.line", rowlineColor);
		editorDescription->getColor ("db.row.back", rowBackColor);
		editorDescription->getColor ("db.row.alternate.back", rowAlternateBackColor);
		once = false;
	}
	CFontRef font = editorDescription->getFont ("db.font");
	source->setupUI (selectionColor, fontColor, rowlineColor, rowBackColor, rowAlternateBackColor, font);
}

//-----------------------------------------------------------------------------
bool UIEditController::std__stringCompare (const std::string* lhs, const std::string* rhs)
{
	for (std::string::const_iterator it = lhs->begin (), it2 = rhs->begin (); it != lhs->end () && it2 != rhs->end (); it++, it2++)
	{
		char c1 = static_cast<char> (tolower (*it));
		char c2 = static_cast<char> (tolower (*it2));
		if (c1 != c2)
			return c1 < c2;
	}
	return false;
}

const UTF8StringPtr UIEditController::kEncodeBitmapsSettingsKey = "EncodeBitmaps";
const UTF8StringPtr UIEditController::kWriteWindowsRCFileSettingsKey = "WriteRCFile";

//----------------------------------------------------------------------------------------------------
class UIEditControllerShadingView : public CView
{
public:
	UIEditControllerShadingView (bool horizontal, bool drawTopLine = false, bool drawBottomLine = true)
	: CView (CRect (0, 0, 0, 0))
	, horizontal (horizontal)
	, drawTopLine (drawTopLine)
	, drawBottomLine (drawBottomLine)
	{}

	void draw (CDrawContext* context) override
	{
		drawGradient (context, getViewSize (), horizontal, drawTopLine, drawBottomLine);
	}

	static void drawGradient (CDrawContext* context, const CRect& _size, bool horizontal, bool drawTopLine = true, bool drawBottomLine = true)
	{
		SharedPointer<CGraphicsPath> path = owned (context->createGraphicsPath ());
		if (path)
		{
			static CColor lineColor = kBlackCColor;
			if (lineColor == kBlackCColor)
				UIEditController::getEditorDescription ()->getColor ("shading.light.frame", lineColor);

			auto lineWidth = 1.;

			CRect size (_size);
			context->setDrawMode (kAliasing);
			context->setLineStyle (kLineSolid);
			context->setLineWidth (lineWidth);
			context->setFrameColor (lineColor);

			CGradient* shading = UIEditController::getEditorDescription ()->getGradient ("shading.light");
			if (shading)
			{
				path->addRect (size);
				if (horizontal)
				{
					context->fillLinearGradient (path, *shading, CPoint (size.left, size.top), CPoint (size.right, size.top));
					if (drawBottomLine)
						context->drawLine (CPoint (size.left, size.top), CPoint (size.left, size.bottom));
					if (drawTopLine)
						context->drawLine (CPoint (size.right-lineWidth, size.bottom), CPoint (size.right-lineWidth, size.top));
				}
				else
				{
					context->fillLinearGradient (path, *shading, CPoint (size.left, size.top), CPoint (size.left, size.bottom));
					if (drawTopLine)
						context->drawLine (CPoint (size.left, size.top), CPoint (size.right, size.top));
					if (drawBottomLine)
						context->drawLine (CPoint (size.right, size.bottom-lineWidth), CPoint (size.left, size.bottom-lineWidth));
				}
			}
		}
	}

protected:
	bool horizontal;
	bool drawTopLine;
	bool drawBottomLine;
};

//----------------------------------------------------------------------------------------------------
class UIZoomSettingController : public IController,
                                public IContextMenuController2,
                                public ViewMouseListenerAdapter,
                                public ViewListenerAdapter,
                                public NonAtomicReferenceCounted
{
public:
	UIZoomSettingController (UIEditController* editController)
	: editController (editController)
	{}

	~UIZoomSettingController () noexcept override
	{
		if (zoomValueControl)
			viewWillDelete (zoomValueControl);
	}

	void restoreSetting (const UIAttributes& attributes)
	{
		double value;
		if (attributes.getDoubleAttribute ("EditViewScale", value))
		{
			if (zoomValueControl)
			{
				zoomValueControl->setValue (static_cast<float> (value) * 100.f);
				valueChanged (zoomValueControl);
			}
		}
	}
	
	void storeSetting (UIAttributes& attributes) const
	{
		if (zoomValueControl)
			attributes.setDoubleAttribute ("EditViewScale", zoomValueControl->getValue () / 100.f);
	}

	void increaseZoom ()
	{
		if (!zoomValueControl)
			return;
		float add = 10.f;
		float current = zoomValueControl->getValue ();
		if (current >= 100.f)
			add = 50.f;
		updateZoom (current + add);
	}
	
	void decreaseZoom ()
	{
		if (!zoomValueControl)
			return;
		float sub = 10.f;
		float current = zoomValueControl->getValue ();
		if (current >= 150.f)
			sub = 50.f;
		updateZoom (current - sub);
	}
	
	void resetZoom ()
	{
		if (!zoomValueControl)
			return;
		updateZoom (100.f);
	}
	
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override
	{
		if (!zoomValueControl)
		{
			zoomValueControl = dynamic_cast<CTextEdit*> (view);
			if (zoomValueControl)
			{
				zoomValueControl->setMin (50.f);
				zoomValueControl->setMax (1000.f);
				zoomValueControl->setStringToValueFunction ([] (UTF8StringPtr txt, float& result, CTextEdit*) {
					int32_t intValue = static_cast<int32_t> (strtol (txt, nullptr, 10));
					if (intValue > 0)
					{
						result = static_cast<float> (intValue);
						return true;
					}
					
					return false;
				});
				zoomValueControl->setValueToStringFunction ([] (float value, char utf8String[256], CParamDisplay*) {
					snprintf (utf8String, 255, "%d %%", static_cast<uint32_t> (value));
					return true;
				});
				zoomValueControl->setValue (100.f);
				CFontRef font = description->getFont ("control.font");
				CColor fontColor = kWhiteCColor, frameColor = kBlackCColor, backColor = kBlackCColor;
				description->getColor ("control.font", fontColor);
				description->getColor ("control.frame", frameColor);
				description->getColor ("control.back", backColor);
				zoomValueControl->setFont (font);
				zoomValueControl->setFontColor (fontColor);
				zoomValueControl->setBackColor (backColor);
				zoomValueControl->setFrameColor (frameColor);
				zoomValueControl->setFrameWidth (-1);
				zoomValueControl->setTooltipText ("Editor Zoom");
				zoomValueControl->registerViewListener (this);
				zoomValueControl->registerViewMouseListener (this);
				zoomValueControl->setStyle (zoomValueControl->getStyle () | CTextEdit::kDoubleClickStyle);
			}
		}
		return view;
	}

	void valueChanged (CControl* pControl) override
	{
		if (pControl == zoomValueControl)
			editController->onZoomChanged (pControl->getValue () / 100.f);
	}
	
	void appendContextMenuItems (COptionMenu& contextMenu, CView* view, const CPoint& where) override
	{
		if (view == zoomValueControl)
		{
			for (auto i = 50; i <= 250; i += 25)
			{
				auto item = new CCommandMenuItem ("Zoom " + toString (i) + "%");
				item->setActions ([this, i] (CCommandMenuItem*) {
					updateZoom (static_cast<float> (i));
				});
				if (zoomValueControl->getValue () == static_cast<float> (i))
					item->setChecked (true);
				contextMenu.addEntry (item);
			}
		}
	}

	CMouseEventResult viewOnMouseDown (CView* view, CPoint pos, CButtonState buttons) override
	{
		vstgui_assert (view == zoomValueControl);
		if (buttons.isDoubleClick ())
			popupTimer = nullptr;
		else if (buttons.isLeftButton () && buttons.getModifierState () == 0)
		{
			popupTimer = makeOwned<CVSTGUITimer> ([this] (CVSTGUITimer*) {
				popupTimer = nullptr;
				auto menu = makeOwned<COptionMenu> ();
				menu->setStyle (COptionMenu::kPopupStyle | COptionMenu::kMultipleCheckStyle);
				appendContextMenuItems (*menu, zoomValueControl, CPoint ());
				menu->popup (zoomValueControl->getFrame (),
				             zoomValueControl->translateToGlobal (
				                 zoomValueControl->getViewSize ().getTopLeft (), true));
			}, 250);
		}
		return kMouseEventNotHandled;
	}

	void viewWillDelete (CView* view) override
	{
		vstgui_assert (view == zoomValueControl);
		view->unregisterViewListener (this);
		view->unregisterViewMouseListener (this);
		zoomValueControl = nullptr;
	}

private:
	void updateZoom (float newZoom)
	{
		if (zoomValueControl)
		{
			zoomValueControl->setValue (newZoom);
			valueChanged (zoomValueControl);
		}
	}
	
	UIEditController* editController{nullptr};
	CTextEdit* zoomValueControl{nullptr};
	SharedPointer<CVSTGUITimer> popupTimer;
};

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIEditController::UIEditController (UIDescription* description)
: editDescription (description)
, selection (makeOwned<UISelection> ())
, undoManager (makeOwned<UIUndoManager> ())
, gridController (makeOwned<UIGridController> (this, description))
, editView (nullptr)
, templateController (nullptr)
, dirty (false)
{
	editorDesc = getEditorDescription ();
	undoManager->registerListener (this);
	editDescription->registerListener (this);
	menuController = new UIEditMenuController (this, selection, undoManager, editDescription, this);
	onTemplatesChanged ();
}

//----------------------------------------------------------------------------------------------------
UIEditController::~UIEditController ()
{
	selection->clear ();
	if (templateController)
		templateController->unregisterListener (this);
	undoManager->unregisterListener (this);
	editDescription->unregisterListener (this);
	editorDesc = nullptr;
	templateController = nullptr;
	undoManager->clear ();
	gUIDescription.tryFree ();
}

//----------------------------------------------------------------------------------------------------
CView* UIEditController::createEditView ()
{
	if (editorDesc->parse ())
	{
		IController* controller = this;
		CView* view = editorDesc->createView ("view", controller);
		if (view)
		{
			view->setAttribute (kCViewControllerAttribute, controller);
			CRect r;
			if (getSettings ()->getRectAttribute ("EditorSize", r))
			{
				view->setViewSize (r);
				view->setMouseableArea (r);
			}
			return view;
		}
	}
	return nullptr;
}

//----------------------------------------------------------------------------------------------------
CView* UIEditController::createView (const UIAttributes& attributes, const IUIDescription* description)
{
	const std::string* name = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (name)
	{
		if (*name == "UIEditView")
		{
			vstgui_assert (editView == nullptr);
			editView = new UIEditView (CRect (0, 0, 0, 0), editDescription);
			editView->setSelection (selection);
			editView->setUndoManager (undoManager);
			editView->setGridProcessor (gridController);
			editView->setupColors (description);
			return editView;
		}
		else if (*name == "ShadingViewHorizontal")
		{
			return new UIEditControllerShadingView (true);
		}
		else if (*name == "ShadingViewVertical")
		{
			return new UIEditControllerShadingView (false);
		}
		else if (*name == "ShadingViewVerticalTopLine")
		{
			return new UIEditControllerShadingView (false, true, false);
		}
	}
	return nullptr;
}

//----------------------------------------------------------------------------------------------------
void UIEditController::onZoomChanged (double zoom)
{
	if (editView)
		editView->setScale (zoom);
	if (zoomSettingController)
		zoomSettingController->storeSetting (*getSettings ());
}

enum {
	kNotSavedTag = 666,
	kEditingTag,
	kAutosizeTag,
	kBackgroundSelectTag,
	kTabSwitchTag = 123456
};

//----------------------------------------------------------------------------------------------------
static SharedPointer<CBitmap> createColorBitmap (CPoint size, CColor color)
{
	auto bitmap = makeOwned<CBitmap> (size);
	if (auto pixelAccessor = owned (CBitmapPixelAccess::create (bitmap)))
	{
		for (auto y = 0u; y < static_cast<uint32_t> (size.y); y++)
		{
			pixelAccessor->setPosition (0, y);
			for (auto x = 0u; x < static_cast<uint32_t> (size.x); ++x)
			{
				pixelAccessor->setColor (color);
				++(*pixelAccessor);
			}
		}
	}
	
	return bitmap;
}

//----------------------------------------------------------------------------------------------------
using BackgroundColors = std::array<CColor, 4>;

static const BackgroundColors& editViewBackgroundColors ()
{
	static const BackgroundColors colors = {{
		{230, 230, 230},
		{150, 150, 150},
		kWhiteCColor,
		kBlackCColor
	}};
	return colors;
}

//----------------------------------------------------------------------------------------------------
CView* UIEditController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	if (view == editView)
	{
		editView->setBackgroundColor (editViewBackgroundColors ()[0]);
		return view;
	}
	auto* splitView = dynamic_cast<CSplitView*>(view);
	if (splitView)
	{
		splitViews.emplace_back (splitView);
		if (splitViews.size () == 1)
		{
			CFontRef font = description->getFont ("control.font");
			CColor fontColor = kWhiteCColor, frameColor = kBlackCColor, backColor = kBlackCColor;
			description->getColor ("control.font", fontColor);
			description->getColor ("control.frame", frameColor);
			description->getColor ("control.back", backColor);
			auto gradient = description->getGradient ("Default TextButton Gradient");
			auto gradientHighlighted = description->getGradient ("Default TextButton Gradient Highlighted");

			// Add Background Menu
			CRect backSelectRect (0., 0., 20. * editViewBackgroundColors ().size (), splitView->getSeparatorWidth ());
			backSelectRect.inset (2, 2);
			auto backSelectControl = new CSegmentButton (backSelectRect, this, kBackgroundSelectTag);
			backSelectControl->setGradient (gradient);
			backSelectControl->setGradientHighlighted (gradientHighlighted);
			backSelectControl->setFrameColor (frameColor);
			backSelectControl->setFrameWidth (-1.);
			backSelectControl->setRoundRadius (2.);
			auto bitmapSize = splitView->getSeparatorWidth () - 12;
			for (auto& color : editViewBackgroundColors ())
			{
				CSegmentButton::Segment segment {};
				segment.iconPosition = CDrawMethods::kIconCenterAbove;
				segment.icon = segment.iconHighlighted = createColorBitmap ({bitmapSize, bitmapSize}, color);
				backSelectControl->addSegment (std::move (segment));
			}
			backSelectControl->setTooltipText ("Editor Background Color");
			splitView->addViewToSeparator (0, backSelectControl);

			// Add Title
			CTextLabel* label = new CTextLabel (CRect (0, 0, splitView->getWidth (), splitView->getSeparatorWidth ()), "Templates | View Hierarchy");
			label->setTransparency (true);
			label->setMouseEnabled (false);
			label->setFont (font);
			label->setFontColor (kBlackCColor);
			label->setAutosizeFlags (kAutosizeAll);
			splitView->addViewToSeparator (0, label);

			// Add Scale Menu
			CRect scaleMenuRect (0, 0, 50, splitView->getSeparatorWidth ());
			scaleMenuRect.offset (splitView->getWidth ()-scaleMenuRect.getWidth (), 0);
			scaleMenuRect.inset (2, 2);
			
			zoomSettingController = new UIZoomSettingController (this); // not owned, shared with control
			auto* textEdit = new CTextEdit (scaleMenuRect, zoomSettingController, 0);
			textEdit->setAttribute (kCViewControllerAttribute, zoomSettingController);
			CView* zoomView = zoomSettingController->verifyView (textEdit, UIAttributes (), editorDesc);
			zoomView->setAutosizeFlags (kAutosizeRight|kAutosizeTop|kAutosizeBottom);
			splitView->addViewToSeparator (0, zoomView);
			zoomSettingController->restoreSetting (*getSettings ());
			
		}
	}
	auto* control = dynamic_cast<CControl*>(view);
	if (control)
	{
		switch (control->getTag ())
		{
			case kNotSavedTag:
			{
				notSavedControl = control;
				notSavedControl->setAlphaValue (dirty ? 1.f : 0.f);
				break;
			}
			case kAutosizeTag:
			{
				control->setListener (this);
				control->setValue (1.);
				break;
			}
			case kEditingTag:
			{
				enableEditingControl = control;
				enableEditingControl->setValue (1.f);
				enableEditingControl->setListener (this);
				break;
			}
			case kTabSwitchTag:
			{
				auto* button = dynamic_cast<CSegmentButton*>(control);
				if (button)
				{
					size_t numSegments = button->getSegments ().size ();
					button->setMax (static_cast<float> (numSegments));
					tabSwitchControl = button;
					int32_t value = 0;
					getSettings ()->getIntegerAttribute ("TabSwitchValue", value);
					button->setSelectedSegment (static_cast<uint32_t> (value));
					static const char* segmentBitmapNames[] = {"segment-views", "segment-tags", "segment-colors", "segment-gradients", "segment-bitmaps", "segment-fonts", nullptr};
					size_t segmentBitmapNameIndex = 0;
					for (const auto& segment : button->getSegments())
					{
						if (segmentBitmapNames[segmentBitmapNameIndex])
						{
							CBitmap* bitmap = editorDesc->getBitmap (segmentBitmapNames[segmentBitmapNameIndex++]);
							if (!bitmap)
								continue;
							segment.icon = bitmap;
							segment.iconHighlighted = bitmap;
							segment.iconPosition = CDrawMethods::kIconLeft;
						}
					}
				}
				break;
			}
		}
	}
	return view;
}

//----------------------------------------------------------------------------------------------------
IController* UIEditController::createSubController (UTF8StringPtr name, const IUIDescription* description)
{
	UTF8StringView subControllerName (name);
	if (subControllerName == "TemplatesController")
	{
		vstgui_assert (templateController == nullptr);
		templateController = new UITemplateController (this, editDescription, selection, undoManager, this);
		templateController->registerListener (this);
		return templateController;
	}
	else if (subControllerName == "MenuController")
	{
		return menuController;
	}
	else if (subControllerName == "ViewCreatorController")
	{
		return new UIViewCreatorController (this, editDescription);
	}
	else if (subControllerName == "AttributesController")
	{
		return new UIAttributesController (this, selection, undoManager, editDescription);
	}
	else if (subControllerName == "TagEditController")
	{
		return new UITagsController (this, editDescription, this);
	}
	else if (subControllerName == "ColorEditController")
	{
		return new UIColorsController (this, editDescription, this);
	}
	else if (subControllerName == "GradientEditController")
	{
		return new UIGradientsController (this, editDescription, this);
	}
	else if (subControllerName == "BitmapEditController")
	{
		return new UIBitmapsController (this, editDescription, this);
	}
	else if (subControllerName == "FontEditController")
	{
		return new UIFontsController (this, editDescription, this);
	}
	else if (subControllerName == "GridController")
	{
		gridController->remember ();
		return gridController;
	}
	return nullptr;
}

//----------------------------------------------------------------------------------------------------
void UIEditController::valueChanged (CControl* control)
{
	if (editView)
	{
		switch (control->getTag ())
		{
			case kEditingTag:
			{
				selection->clear ();
				if (auto container = editView->getEditView () ? editView->getEditView ()->asViewContainer () : nullptr)
					resetScrollViewOffsets (container);
				editView->enableEditing (control->getValue () == control->getMax () ? true : false);
				break;
			}
			case kAutosizeTag:
			{
				editView->enableAutosizing (control->getValue () == 1.f);
				break;
			}
			case kBackgroundSelectTag:
			{
				if (auto seg = dynamic_cast<CSegmentButton*> (control))
				{
					CColor color = editViewBackgroundColors ()[seg->getSelectedSegment ()];
					editView->setBackgroundColor (color);
				}
				break;
			}
			case kTabSwitchTag:
			{
				getSettings ()->setIntegerAttribute ("TabSwitchValue", static_cast<int32_t> (tabSwitchControl->getValue ()));
				break;
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------
bool UIEditController::validateCommandMenuItem (CCommandMenuItem* item)
{
	return validateMenuItem (item) == kMessageNotified;
}

//----------------------------------------------------------------------------------------------------
bool UIEditController::onCommandMenuItemSelected (CCommandMenuItem* item)
{
	return onMenuItemSelection (item) == kMessageNotified;
}

//----------------------------------------------------------------------------------------------------
void UIEditController::onUndoManagerChange ()
{
	onUndoManagerChanged ();
}

//----------------------------------------------------------------------------------------------------
void UIEditController::onTemplateSelectionChanged ()
{
	if (editView && templateController)
	{
		const auto& name = templateController->getSelectedTemplateName ();
		if ((name && *name != editTemplateName) || name == nullptr)
		{
			if (undoManager->canUndo () && !editTemplateName.empty ())
				updateTemplate (editTemplateName.c_str ());
			if (name)
			{
				for (auto& it : templates)
				{
					if (*name == it.name)
					{
						CView* view = it.view;
						editView->setEditView (view);
						templateController->setTemplateView (static_cast<CViewContainer*> (view));
						editTemplateName = *templateController->getSelectedTemplateName ();
						view->remember ();
						break;
					}
				}
			}
			else
			{
				selection->clear ();
				editView->setEditView (nullptr);
				templateController->setTemplateView (nullptr);
				editTemplateName = "";
			}
		}
		if (editView->getEditView ())
		{
			if (!(selection->first () && editView->getEditView ()->asViewContainer ()->isChild (selection->first (), true)))
				selection->setExclusive (editView->getEditView ());
		}
		else
			selection->clear ();
	}
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIEditController::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == UIEditView::kMsgAttached)
	{
		vstgui_assert (editView);
		editView->getFrame ()->registerKeyboardHook (this);
		return kMessageNotified;
	}
	else if (message == UIEditView::kMsgRemoved)
	{
		editView->getFrame ()->unregisterKeyboardHook (this);
		beforeSave ();
		splitViews.clear ();
		getEditorDescription ()->freePlatformResources ();
		return kMessageNotified;
	}
	
	return kMessageUnknown;
}

//----------------------------------------------------------------------------------------------------
void UIEditController::onUIDescTemplateChanged (UIDescription* desc)
{
	onTemplatesChanged ();
}

//----------------------------------------------------------------------------------------------------
void UIEditController::beforeUIDescSave (UIDescription* desc)
{
	beforeSave ();
}

//----------------------------------------------------------------------------------------------------
bool UIEditController::doUIDescTemplateUpdate (UIDescription* desc, UTF8StringPtr name)
{
	if (onlyTemplateToUpdateName.empty ())
		return true;
	return onlyTemplateToUpdateName == name;
}

//----------------------------------------------------------------------------------------------------
void UIEditController::beforeSave ()
{
	if (editView && editView->getEditView ())
	{
		if (undoManager->canUndo ())
		{
			if (!editTemplateName.empty ())
				updateTemplate (editTemplateName.c_str ());
			for (std::vector<Template>::const_iterator it = templates.begin (); it != templates.end (); it++)
			{
				onlyTemplateToUpdateName = it->name;
				updateTemplate (it);
			}
			onlyTemplateToUpdateName.clear ();
		}
		for (auto& splitView : splitViews)
			splitView->storeViewSizes ();
		
		getSettings ()->setIntegerAttribute ("Version", 1);
		// find the view of this controller
		auto container = editView->getParentView ()->asViewContainer ();
		while (container && container != container->getFrame ())
		{
			if (getViewController (container, false) == this)
			{
				getSettings ()->setRectAttribute ("EditorSize", container->getViewSize ());
				break;
			}
			container = container->getParentView () ? container->getParentView ()->asViewContainer () : nullptr;
		}
		undoManager->markSavePosition ();
		if (zoomSettingController)
			zoomSettingController->storeSetting (*getSettings ());
		setDirty (false);
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditController::doCopy (bool cut)
{
	if (!editTemplateName.empty ())
		updateTemplate (editTemplateName.c_str ());
	CMemoryStream stream (1024, 1024, false);
	selection->store (stream, editDescription);
	auto dataSource = CDropSource::create (stream.getBuffer (), static_cast<uint32_t> (stream.tell ()), IDataPackage::kText);
	editView->getFrame ()->setClipboard (dataSource);
	if (cut)
		undoManager->pushAndPerform (new DeleteOperation (selection));
}

//----------------------------------------------------------------------------------------------------
void UIEditController::addSelectionToCurrentView (UISelection* copySelection)
{
	if (selection->total () == 0)
		return;
	CPoint offset;
	CViewContainer* container = selection->first ()->asViewContainer ();
	if (container == nullptr)
	{
		container = selection->first ()->getParentView ()->asViewContainer ();
		offset = selection->first ()->getViewSize ().getTopLeft ();
		offset.offset (gridController->getSize ().x, gridController->getSize ().y);
	}
	IAction* action = new ViewCopyOperation (copySelection, selection, container, offset, editDescription);
	undoManager->pushAndPerform (action);
	if (!editTemplateName.empty ())
		updateTemplate (editTemplateName.c_str ());
}

//----------------------------------------------------------------------------------------------------
void UIEditController::doPaste ()
{
	if (auto clipboard = editView->getFrame ()->getClipboard ())
	{
		if (clipboard->getDataType (0) == IDataPackage::kText)
		{
			const void* data;
			IDataPackage::Type type;
			uint32_t size = clipboard->getData (0, data, type);
			if (size > 0)
			{
				CMemoryStream stream ((const int8_t*)data, size, false);
				auto* copySelection = new UISelection ();
				if (copySelection->restore (stream, editDescription))
				{
					addSelectionToCurrentView (copySelection);
				}
				copySelection->forget ();
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditController::showTemplateSettings ()
{
	if (undoManager->canUndo () && !editTemplateName.empty ())
	{
		updateTemplate (editTemplateName.c_str ());
	}
	auto dc = new UIDialogController (this, editView->getFrame ());
	auto tsController =
	    makeOwned<UITemplateSettingsController> (editTemplateName, editDescription, this);
	dc->run ("template.settings", "Template Settings", "OK", "Cancel", tsController, editorDesc);
}

//----------------------------------------------------------------------------------------------------
void UIEditController::showFocusSettings ()
{
	auto dc = new UIDialogController (this, editView->getFrame ());
	auto fsController = makeOwned<UIFocusSettingsController> (editDescription, this);
	dc->run ("focus.settings", "Focus Drawing Settings", "OK", "Cancel", fsController, editorDesc);
}

//----------------------------------------------------------------------------------------------------
static void toggleBoolAttribute (UIAttributes* attributes, UTF8StringPtr key)
{
	if (attributes)
	{
		bool val = false;
		attributes->getBooleanAttribute (key, val);
		attributes->setBooleanAttribute (key, !val);
	}
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIEditController::onMenuItemSelection (CCommandMenuItem* item)
{
	UTF8StringView cmdCategory (item->getCommandCategory ());
	UTF8StringView cmdName (item->getCommandName ());

	if (cmdCategory == "Edit")
	{
		if (cmdName == "Copy")
		{
			doCopy (false);
			return kMessageNotified;
		}
		else if (cmdName == "Cut")
		{
			doCopy (true);
			return kMessageNotified;
		}
		else if (cmdName == "Paste")
		{
			doPaste ();
			return kMessageNotified;
		}
		else if (cmdName == "Template Settings...")
		{
			showTemplateSettings ();
			return kMessageNotified;
		}
		else if (cmdName == "Focus Drawing Settings...")
		{
			showFocusSettings ();
			return kMessageNotified;
		}
	}
	else if (cmdCategory == "File")
	{
		if (cmdName == "Encode Bitmaps in XML")
		{
			toggleBoolAttribute (getSettings (), kEncodeBitmapsSettingsKey);
			return kMessageNotified;
		}
		else if (cmdName == "Write Windows RC File on Save")
		{
			toggleBoolAttribute (getSettings (), kWriteWindowsRCFileSettingsKey);
			return kMessageNotified;
		}
	}
	else if (cmdCategory == "SelectionMoveByGrid")
	{
		if (doSelectionMove (item->getCommandName (), true))
			return kMessageNotified;
	}
	else if (cmdCategory == "SelectionSizeByGrid")
	{
		if (doSelectionSize (item->getCommandName (), true))
			return kMessageNotified;
	}
	else if (cmdCategory == "SelectionMoveByPixel")
	{
		if (doSelectionMove (item->getCommandName (), false))
			return kMessageNotified;
	}
	else if (cmdCategory == "SelectionSizeByPixel")
	{
		if (doSelectionSize (item->getCommandName (), false))
			return kMessageNotified;
	}
	else if (cmdCategory == "SelectionZOrder")
	{
		bool lower = cmdName == "Lower" ? true : false;
		if (doZOrderAction (lower))
			return kMessageNotified;
		
	}
	else if (cmdCategory == "Selection")
	{
		if (cmdName == "Select All Children")
		{
			doSelectAllChildren ();
			return kMessageNotified;
		}
		if (cmdName == "Select Parent(s)")
		{
			doSelectParents ();
			return kMessageNotified;
		}
		if (cmdName == "Select View in Hierarchy Browser")
		{
			doSelectViewInHierarchyBrowser (selection->first ());
			return kMessageNotified;
		}
	}
	else if (cmdCategory == "Zoom")
	{
		if (cmdName == "Zoom In")
		{
			zoomSettingController->increaseZoom ();
			return kMessageNotified;
		}
		else if (cmdName == "Zoom Out")
		{
			zoomSettingController->decreaseZoom ();
			return kMessageNotified;
		}
		else if (cmdName == "Zoom 100%")
		{
			zoomSettingController->resetZoom ();
			return kMessageNotified;
		}
	}
	return kMessageUnknown;
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIEditController::validateMenuItem (CCommandMenuItem* item)
{
	UTF8StringView cmdCategory (item->getCommandCategory ());
	UTF8StringView cmdName (item->getCommandName ());
	
	if (cmdCategory == "Edit")
	{
		if (cmdName == "Template Settings...")
		{
			item->setEnabled (editTemplateName.empty () ? false : true);
			return kMessageNotified;
		}
		else if (cmdName == "Copy" || cmdName == "Cut")
		{
			if (editView && selection->first () && selection->contains (editView->getEditView ()) == false)
				item->setEnabled (true);
			else
				item->setEnabled (false);
			return kMessageNotified;
		}
		else if (cmdName == "Paste")
		{
			item->setEnabled (false);
			if (editView && selection->first ())
			{
				if (auto clipboard = editView->getFrame ()->getClipboard ())
				{
					if (clipboard->getDataType (0) == IDataPackage::kText)
						item->setEnabled (true);
				}
			}
			return kMessageNotified;
		}
	}
	else if (cmdCategory == "File")
	{
		if (cmdName == "Encode Bitmaps in XML")
		{
			auto attr = getSettings ();
			bool encodeBitmaps = false;
			if (attr && attr->getBooleanAttribute (kEncodeBitmapsSettingsKey, encodeBitmaps))
			{
				item->setChecked (encodeBitmaps);
			}
			return kMessageNotified;
		}
		else if (cmdName == "Write Windows RC File on Save")
		{
			auto attr = getSettings ();
			bool encodeBitmaps = false;
			if (attr && attr->getBooleanAttribute (kWriteWindowsRCFileSettingsKey, encodeBitmaps))
			{
				item->setChecked (encodeBitmaps);
			}
			return kMessageNotified;
		}
	}
	else if (cmdCategory == "SelectionMoveByGrid"
			 || cmdCategory == "SelectionSizeByGrid"
			 || cmdCategory == "SelectionMoveByPixel"
			 || cmdCategory == "SelectionSizeByPixel"
		)
	{
		bool enableItem = selection->first () ? true : false;
		if (enableItem && cmdCategory.contains ("Size") == false)
		{
			if (selection->contains (editView->getEditView ()))
				enableItem = false;
		}
		item->setEnabled (enableItem);
		return kMessageNotified;
	}
	else if (cmdCategory == "SelectionZOrder")
	{
		bool enableItem = selection->total () == 1;
		if (enableItem)
		{
			bool lower = cmdName == "Lower" ? true : false;
			CView* view = selection->first ();
			if (auto parent = view->getParentView ()->asViewContainer ())
			{
				if (lower)
				{
					ViewIterator it (parent);
					if (*it == view)
						enableItem = false;
				}
				else
				{
					ReverseViewIterator it (parent);
					if (*it == view)
						enableItem = false;
				}
			}
		}
		item->setEnabled (enableItem);
		return kMessageNotified;
	}
	else if (cmdCategory == "Selection")
	{
		if (cmdName == "Select All Children")
		{
			bool enable = selection->total () == 1 && selection->first () &&
			              selection->first ()->asViewContainer ();
			item->setEnabled (enable);
			return kMessageNotified;
		}
		if (cmdName == "Select Parent(s)")
		{
			bool enable =
			    selection->total () > 0 && selection->first () != editView->getEditView ();
			item->setEnabled (enable);
			item->setTitle (selection->total () > 1 ? "Select Parents" : "Select Parent");
			return kMessageNotified;
		}
		if (cmdName == "Select View in Hierarchy Browser")
		{
			item->setEnabled (selection->total () == 1);
			return kMessageNotified;
		}
	}
	return kMessageUnknown;
}

//----------------------------------------------------------------------------------------------------
bool UIEditController::doSelectionMove (const UTF8String& commandName, bool useGrid) const
{
	CPoint diff;
	if (commandName == "Move Up")
		diff.y = -(useGrid ? gridController->getSize ().y : 1);
	else if (commandName == "Move Down")
		diff.y = (useGrid ? gridController->getSize ().y : 1);
	else if (commandName == "Move Left")
		diff.x = -(useGrid ? gridController->getSize ().x : 1);
	else if (commandName == "Move Right")
		diff.x = (useGrid ? gridController->getSize ().x : 1);
	if (diff.x != 0 || diff.y != 0)
	{
		editView->doKeyMove (diff);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
bool UIEditController::doSelectionSize (const UTF8String& commandName, bool useGrid) const
{
	CPoint diff;
	if (commandName == "Increase Size Width")
		diff.x = (useGrid ? gridController->getSize ().x : 1);
	else if (commandName == "Increase Size Height")
		diff.y = (useGrid ? gridController->getSize ().y : 1);
	else if (commandName == "Decrease Size Width")
		diff.x = -(useGrid ? gridController->getSize ().x : 1);
	else if (commandName == "Decrease Size Height")
		diff.y = -(useGrid ? gridController->getSize ().y : 1);
	if (diff.x != 0 || diff.y != 0)
	{
		editView->doKeySize (diff);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
bool UIEditController::doZOrderAction (bool lower)
{
	if (selection->total () == 1)
	{
		CView* view = selection->first ();
		undoManager->pushAndPerform (
		    new HierarchyMoveViewOperation (view, selection, lower ? -1 : 1));
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
void UIEditController::doSelectAllChildren ()
{
	UISelection::DeferChange dc (*selection);
	CViewContainer* container = selection->first ()->asViewContainer ();
	selection->clear ();
	auto factory = static_cast<const UIViewFactory*> (editDescription->getViewFactory ());
	container->forEachChild ([&] (CView* view) {
		if (factory->getViewName (view))
			selection->add (view);
	});
}

//----------------------------------------------------------------------------------------------------
void UIEditController::doSelectParents ()
{
	UISelection::DeferChange dc (*selection);
	std::vector<CView*> parents;
	auto factory = static_cast<const UIViewFactory*> (editDescription->getViewFactory ());
	for (auto& view : *selection)
	{
		if (auto parent = view->getParentView ())
		{
			while (factory->getViewName (parent) == nullptr)
			{
				parent = parent->getParentView ();
			}
			if (parent && std::find (parents.begin (), parents.end (), parent) == parents.end ())
				parents.emplace_back (parent);
		}
	}
	selection->clear ();
	for (auto& parent : parents)
		selection->add (parent);
}

//----------------------------------------------------------------------------------------------------
void UIEditController::doSelectViewInHierarchyBrowser (CView* view)
{
	templateController->navigateTo (view);
}

//----------------------------------------------------------------------------------------------------
void UIEditController::onUndoManagerChanged ()
{
	if (undoManager->isSavePosition ())
	{
		updateTemplate (editTemplateName.data ());
		setDirty (false);
	}
	else
		setDirty (true);
	CView* view = selection->first ();
	if (view)
	{
		if (auto templateView = editView->getEditView () ? editView->getEditView ()->asViewContainer () : nullptr)
		{
			if (view == templateView || templateView->isChild (view, true))
			{
				return;
			}
		}
		for (auto& it : templates)
		{
			CViewContainer* container = it.view->asViewContainer ();
			if (container && (view == container || container->isChild (view, true)))
			{
				templateController->selectTemplate (it.name.c_str ());
				return;
			}
		}
		selection->clear ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditController::resetScrollViewOffsets (CViewContainer* view)
{
	view->forEachChild ([&] (CView* view) {
		auto* scrollView = dynamic_cast<CScrollView*>(view);
		if (scrollView)
		{
			scrollView->resetScrollOffset ();
		}
		if (auto container = view->asViewContainer ())
			resetScrollViewOffsets (container);
	});
}

//----------------------------------------------------------------------------------------------------
int32_t UIEditController::onKeyDown (const VstKeyCode& code, CFrame* frame)
{
	if (frame->getModalView () == nullptr)
	{
		if (frame->getFocusView ())
		{
			auto* edit = dynamic_cast<CTextEdit*>(frame->getFocusView ());
			if (edit && edit->getPlatformTextEdit ())
				return -1;
		}
		return menuController->processKeyCommand (code);
	}
	return -1;
}

//----------------------------------------------------------------------------------------------------
int32_t UIEditController::onKeyUp (const VstKeyCode& code, CFrame* frame)
{
	return -1;
}

//----------------------------------------------------------------------------------------------------
SharedPointer<UIAttributes> UIEditController::getSettings ()
{
	return editDescription->getCustomAttributes ("UIEditController", true);
}

//----------------------------------------------------------------------------------------------------
int32_t UIEditController::getSaveOptions ()
{
	int32_t flags = 0;
	auto attributes = getSettings ();
	bool val;
	if (attributes->getBooleanAttribute (UIEditController::kEncodeBitmapsSettingsKey, val) && val == true)
	{
		flags |= UIDescription::kWriteImagesIntoUIDescFile;
	}
	if (attributes->getBooleanAttribute (UIEditController::kWriteWindowsRCFileSettingsKey, val) && val == true)
	{
		flags |= UIDescription::kWriteWindowsResourceFile;
	}
	return flags;
}

//----------------------------------------------------------------------------------------------------
int32_t UIEditController::getSplitViewIndex (CSplitView* splitView)
{
	int32_t index = 0;
	for (auto& sv : splitViews)
	{
		if (sv == splitView)
			return index;
		index++;
	}
	return -1;
}

//----------------------------------------------------------------------------------------------------
bool UIEditController::getSplitViewSizeConstraint (int32_t index, CCoord& minSize, CCoord& maxSize, CSplitView* splitView)
{
	return false;
}

//----------------------------------------------------------------------------------------------------
ISplitViewSeparatorDrawer* UIEditController::getSplitViewSeparatorDrawer (CSplitView* splitView)
{
	int32_t si = getSplitViewIndex (splitView);
	if (si >= 0)
	{
		return this;
	}
	return nullptr;
}

//----------------------------------------------------------------------------------------------------
bool UIEditController::storeViewSize (int32_t index, const CCoord& size, CSplitView* splitView)
{
	int32_t si = getSplitViewIndex (splitView);
	if (si >= 0)
	{
		std::stringstream str;
		str << "SplitViewSize_";
		str << si;
		str << "_";
		str << index;
		double value;
		if (splitView->getStyle () == CSplitView::kHorizontal)
			value = size / splitView->getWidth ();
		else
			value = size / splitView->getHeight ();
		getSettings ()->setDoubleAttribute (str.str (), value);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
bool UIEditController::restoreViewSize (int32_t index, CCoord& size, CSplitView* splitView)
{
	int32_t version = 0;
	getSettings ()->getIntegerAttribute ("Version", version);
	if (version == 0)
		return false;
	int32_t si = getSplitViewIndex (splitView);
	if (si >= 0)
	{
		std::stringstream str;
		str << "SplitViewSize_";
		str << si;
		str << "_";
		str << index;
		double value;
		if (getSettings ()->getDoubleAttribute (str.str ().c_str (), value))
		{
			if (splitView->getStyle () == CSplitView::kHorizontal)
				value = floor (splitView->getWidth () * value + 0.5);
			else
				value = floor (splitView->getHeight () * value + 0.5);
			size = value;
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
void UIEditController::drawSplitViewSeparator (CDrawContext* context, const CRect& size, int32_t flags, int32_t index, CSplitView* splitView)
{
	if (splitView->getStyle () == CSplitView::kHorizontal)
	{
		UIEditControllerShadingView::drawGradient (context, size, true);
	}
	else
	{
		UIEditControllerShadingView::drawGradient (context, size, false);
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditController::setDirty (bool state)
{
	if (dirty != state)
	{
		dirty = state;
		if (notSavedControl && notSavedControl->isAttached ())
		{
			notSavedControl->invalid ();
			notSavedControl->addAnimation ("AlphaValueAnimation", new Animation::AlphaValueAnimation (dirty ? 1.f : 0.f), new Animation::LinearTimingFunction (80));
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performAction (IAction* action)
{
	undoManager->pushAndPerform (action);
}

//----------------------------------------------------------------------------------------------------
void UIEditController::beginGroupAction (UTF8StringPtr name)
{
	undoManager->startGroupAction (name);
}

//----------------------------------------------------------------------------------------------------
void UIEditController::finishGroupAction ()
{
	undoManager->endGroupAction ();
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performChangeFocusDrawingSettings (const FocusDrawingSettings& newSettings)
{
	undoManager->pushAndPerform (new ChangeFocusDrawingAction (editDescription, newSettings));
}

//----------------------------------------------------------------------------------------------------
void UIEditController::getTemplateViews (std::list<CView*>& views) const
{
	for (const auto& templateDesc : templates)
		views.emplace_back (templateDesc.view);
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performColorChange (UTF8StringPtr colorName, const CColor& newColor, bool remove)
{
	std::list<CView*> views;
	getTemplateViews (views);

	auto* action = new ColorChangeAction (editDescription, colorName, newColor, remove, true);
	undoManager->startGroupAction (remove ? "Delete Color" : action->isAddColor () ? "Add New Color" : "Change Color");
	undoManager->pushAndPerform (action);
	undoManager->pushAndPerform (new MultipleAttributeChangeAction (editDescription, views, IViewCreator::kColorType, colorName, remove ? "" : colorName));
	undoManager->pushAndPerform (new ColorChangeAction (editDescription, colorName, newColor, remove, false));
	undoManager->endGroupAction ();
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performTagChange (UTF8StringPtr tagName, UTF8StringPtr tagStr, bool remove)
{
	std::list<CView*> views;
	getTemplateViews (views);

	auto* action = new TagChangeAction (editDescription, tagName, tagStr, remove, true);
	undoManager->startGroupAction (remove ? "Delete Tag" : action->isAddTag () ? "Add New Tag" : "Change Tag");
	undoManager->pushAndPerform (action);
	undoManager->pushAndPerform (new MultipleAttributeChangeAction (editDescription, views, IViewCreator::kTagType, tagName, remove ? "" : tagName));
	undoManager->pushAndPerform (new TagChangeAction (editDescription, tagName, tagStr, remove, false));
	undoManager->endGroupAction ();
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performBitmapChange (UTF8StringPtr bitmapName, UTF8StringPtr bitmapPath, bool remove)
{
	std::list<CView*> views;
	getTemplateViews (views);

	auto* action = new BitmapChangeAction (editDescription, bitmapName, bitmapPath, remove, true);
	undoManager->startGroupAction (remove ? "Delete Bitmap" : action->isAddBitmap () ? "Add New Bitmap" :"Change Bitmap");
	undoManager->pushAndPerform (action);
	undoManager->pushAndPerform (new MultipleAttributeChangeAction (editDescription, views, IViewCreator::kBitmapType, bitmapName, remove ? "" : bitmapName));
	undoManager->pushAndPerform (new BitmapChangeAction (editDescription, bitmapName, bitmapPath, remove, false));
	undoManager->endGroupAction ();
}

//------------------------------------------------------------------------
void UIEditController::performGradientChange (UTF8StringPtr gradientName, CGradient* newGradient, bool remove)
{
	std::list<CView*> views;
	getTemplateViews (views);
	
	auto* action = new GradientChangeAction (editDescription, gradientName, newGradient, remove, true);
	undoManager->startGroupAction (remove ? "Delete Bitmap" : action->isAddGradient () ? "Add New Gradient" :"Change Gradient");
	undoManager->pushAndPerform (action);
	undoManager->pushAndPerform (new MultipleAttributeChangeAction (editDescription, views, IViewCreator::kGradientType, gradientName, remove ? "" : gradientName));
	undoManager->pushAndPerform (new GradientChangeAction (editDescription, gradientName, newGradient, remove, false));
	undoManager->endGroupAction ();
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performFontChange (UTF8StringPtr fontName, CFontRef newFont, bool remove)
{
	std::list<CView*> views;
	getTemplateViews (views);

	auto* action = new FontChangeAction (editDescription, fontName, newFont, remove, true);
	undoManager->startGroupAction (remove ? "Delete Font" : action->isAddFont () ? "Add New Font" : "Change Font");
	undoManager->pushAndPerform (action);
	undoManager->pushAndPerform (new MultipleAttributeChangeAction (editDescription, views, IViewCreator::kFontType, fontName, remove ? "" : fontName));
	undoManager->pushAndPerform (new FontChangeAction (editDescription, fontName, newFont, remove, false));
	undoManager->endGroupAction ();
}

//----------------------------------------------------------------------------------------------------
template<typename NameChangeAction, IViewCreator::AttrType attrType> void UIEditController::performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName, IdStringPtr groupActionName)
{
	std::list<CView*> views;
	getTemplateViews (views);

	undoManager->startGroupAction (groupActionName);
	undoManager->pushAndPerform (new NameChangeAction (editDescription, oldName, newName, true));
	undoManager->pushAndPerform (new MultipleAttributeChangeAction (editDescription, views, attrType, oldName, newName));
	undoManager->pushAndPerform (new NameChangeAction (editDescription, oldName, newName, false));
	undoManager->endGroupAction ();
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performColorNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	performNameChange<ColorNameChangeAction, IViewCreator::kColorType> (oldName, newName, "Change Color Name");
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performTagNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	performNameChange<TagNameChangeAction, IViewCreator::kTagType> (oldName, newName, "Change Tag Name");
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performFontNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	performNameChange<FontNameChangeAction, IViewCreator::kFontType> (oldName, newName, "Change Font Name");
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performGradientNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	performNameChange<GradientNameChangeAction, IViewCreator::kGradientType> (oldName, newName, "Change Gradient Name");
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performBitmapNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	performNameChange<BitmapNameChangeAction, IViewCreator::kBitmapType> (oldName, newName, "Change Bitmap Name");
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performBitmapNinePartTiledChange (UTF8StringPtr bitmapName, const CRect* offsets)
{
	std::list<CView*> views;
	getTemplateViews (views);

	undoManager->startGroupAction ("Change NinePartTiled Bitmap");
	undoManager->pushAndPerform (new NinePartTiledBitmapChangeAction (editDescription, bitmapName, offsets, true));
	undoManager->pushAndPerform (new MultipleAttributeChangeAction (editDescription, views, IViewCreator::kBitmapType, bitmapName, bitmapName));
	undoManager->pushAndPerform (new NinePartTiledBitmapChangeAction (editDescription, bitmapName, offsets, false));
	undoManager->endGroupAction ();
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performBitmapFiltersChange (UTF8StringPtr bitmapName, const std::list<SharedPointer<UIAttributes> >& filterDescription)
{
	std::list<CView*> views;
	getTemplateViews (views);

	undoManager->startGroupAction ("Change Bitmap Filter");
	undoManager->pushAndPerform (new BitmapFilterChangeAction (editDescription, bitmapName, filterDescription, true));
	undoManager->pushAndPerform (new MultipleAttributeChangeAction (editDescription, views, IViewCreator::kBitmapType, bitmapName, bitmapName));
	undoManager->pushAndPerform (new BitmapFilterChangeAction (editDescription, bitmapName, filterDescription, false));
	undoManager->endGroupAction ();
}


//----------------------------------------------------------------------------------------------------
void UIEditController::performAlternativeFontChange (UTF8StringPtr fontName, UTF8StringPtr newAlternativeFonts)
{
	undoManager->pushAndPerform (new AlternateFontChangeAction (editDescription, fontName, newAlternativeFonts));
}

//----------------------------------------------------------------------------------------------------
void UIEditController::beginLiveColorChange (UTF8StringPtr colorName)
{
	undoManager->startGroupAction ("Change Color");
	CColor color;
	editDescription->getColor(colorName, color);
	performColorChange (colorName, color, false);
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performLiveColorChange (UTF8StringPtr _colorName, const CColor& newColor)
{
	std::string colorName (_colorName);

	IAction* action =
	    new ColorChangeAction (editDescription, colorName.data (), newColor, false, true);
	action->perform ();
	delete action;

	std::list<CView*> views;
	getTemplateViews (views);

	action = new MultipleAttributeChangeAction (editDescription, views, IViewCreator::kColorType,
	                                            colorName.data (), colorName.data ());
	action->perform ();
	delete action;
}

//----------------------------------------------------------------------------------------------------
void UIEditController::endLiveColorChange (UTF8StringPtr colorName)
{
	UIDescriptionListenerOff lo (this, editDescription);
	CColor color;
	editDescription->getColor (colorName, color);
	performColorChange (colorName, color, false);
	undoManager->endGroupAction ();
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performTemplateNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	undoManager->pushAndPerform (new TemplateNameChangeAction (editDescription, this, oldName, newName));
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performTemplateMinMaxSizeChange (UTF8StringPtr templateName, CPoint minSize, CPoint maxSize)
{
	undoManager->pushAndPerform (
	    new ChangeTemplateMinMaxAction (editDescription, templateName, minSize, maxSize));
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performCreateNewTemplate (UTF8StringPtr name, UTF8StringPtr baseViewClassName)
{
	undoManager->pushAndPerform (new CreateNewTemplateAction (editDescription, this, name, baseViewClassName));
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performDeleteTemplate (UTF8StringPtr name)
{
	auto it = std::find (templates.begin (), templates.end (), name);
	if (it != templates.end ())
		undoManager->pushAndPerform (new DeleteTemplateAction (editDescription, this, (*it).view, (*it).name.c_str ()));
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performDuplicateTemplate (UTF8StringPtr name, UTF8StringPtr dupName)
{
	updateTemplate (name);
	UIDescriptionListenerOff lo (this, editDescription);
	undoManager->pushAndPerform (new DuplicateTemplateAction (editDescription, this, name, dupName));
}

//----------------------------------------------------------------------------------------------------
void UIEditController::onTemplateCreation (UTF8StringPtr name, CView* view)
{
	auto it = std::find (templates.begin (), templates.end (), name);
	if (it == templates.end ())
		templates.emplace_back (name, view);
	templateController->selectTemplate (name);
}

//----------------------------------------------------------------------------------------------------
void UIEditController::onTemplateNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	auto it = std::find (templates.begin (), templates.end (), oldName);
	if (it != templates.end ())
	{
		(*it).name = newName;
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditController::updateTemplate (const std::vector<Template>::const_iterator& it)
{
	if (it != templates.end ())
	{
		CView* view = (*it).view;
		if (auto container = view->asViewContainer ())
			resetScrollViewOffsets (container);
		editDescription->updateViewDescription ((*it).name.c_str (), view);
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditController::updateTemplate (UTF8StringPtr name)
{
	auto it = std::find (templates.begin (), templates.end (), name);
	updateTemplate (it);
}

//----------------------------------------------------------------------------------------------------
void UIEditController::onTemplatesChanged ()
{
	std::list<const std::string*> templateNames;
	editDescription->collectTemplateViewNames (templateNames);
	for (auto& it : templateNames)
	{
		if (std::find (templates.begin (), templates.end (), *it) == templates.end ())
		{
			auto view = owned (editDescription->createView (it->c_str (), editDescription->getController ()));
			templates.emplace_back (*it, view);
		}
	}
	for (std::vector<Template>::iterator it = templates.begin (); it != templates.end ();)
	{
		Template& t = (*it);
		bool found = false;
		for (auto& it2 : templateNames)
		{
			if (t.name == *it2)
			{
				found = true;
				it++;
				break;
			}
		}
		if (!found)
		{
			it = templates.erase (it);
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditController::appendContextMenuItems (COptionMenu& contextMenu, CView* view, const CPoint& where)
{
	auto vc = view->asViewContainer ();
	if (!vc || editView == nullptr)
		return;
	view = vc->getViewAt (where, GetViewOptions ().deep ().includeViewContainer ());
	while (view && view != editView)
	{
		view = view->getParentView ();
	}
	if (view != editView)
		return;
	auto editMenu = getMenuController ()->getEditMenu ();
	for (auto& entry : *editMenu->getItems ())
	{
		if (auto item = entry.cast<CCommandMenuItem> ())
		{
			item->validate ();
		}
		if (!entry->isEnabled ())
			continue;
		entry->remember ();
		contextMenu.addEntry (entry);
	}
}

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
