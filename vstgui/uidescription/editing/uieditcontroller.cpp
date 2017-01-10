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
#include "uisearchtextfield.h"
#include "uiactions.h"
#include "uiselection.h"
#include "uidialogcontroller.h"
#include "uitemplatesettingscontroller.h"
#include "uifocussettingscontroller.h"
#include "../uiattributes.h"
#include "../../lib/controls/coptionmenu.h"
#include "../../lib/controls/csegmentbutton.h"
#include "../../lib/animation/animations.h"
#include "../../lib/animation/timingfunctions.h"
#include "../../lib/cdropsource.h"
#include "../../lib/cgraphicspath.h"

#include <sstream>
#include <algorithm>
#include <cassert>

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
	UIDescription& get () const
	{
		if (uiDesc == 0)
		{
#ifdef HAVE_EDITORUIDESC_H
			Xml::MemoryContentProvider provider (editorUIDesc, strlen (editorUIDesc));
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
				SharedPointer<UIDescription> editorDesc = owned (new UIDescription (descPath.c_str ()));
				if (editorDesc->parse ())
				{
					uiDesc = editorDesc;
				}
				else
				{
					vstgui_assert (false, "the __FILE__ macro is relative, so it's not possible to find the uidescriptioneditor.uidesc. You can replace the macro with the absolute filename to make this work on your devel machine");
				}
			}
#endif
		}
		return *uiDesc;
	}

private:
	mutable SharedPointer<UIDescription> uiDesc;
};

static UIEditControllerDescription gUIDescription;

//----------------------------------------------------------------------------------------------------
UIDescription& UIEditController::getEditorDescription ()
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
	if (once)
	{
		UIEditController::getEditorDescription ().getColor ("db.selection", selectionColor);
		UIEditController::getEditorDescription ().getColor ("db.font", fontColor);
		UIEditController::getEditorDescription ().getColor ("db.row.line", rowlineColor);
		UIEditController::getEditorDescription ().getColor ("db.row.back", rowBackColor);
		UIEditController::getEditorDescription ().getColor ("db.row.alternate.back", rowAlternateBackColor);
		once = false;
	}
	CFontRef font = UIEditController::getEditorDescription ().getFont ("db.font");
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
	return true;
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

	void draw (CDrawContext* context) VSTGUI_OVERRIDE_VMETHOD
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
				UIEditController::getEditorDescription ().getColor ("shading.light.frame", lineColor);

			CRect size (_size);
			context->setDrawMode (kAliasing);
			context->setLineStyle (kLineSolid);
			context->setLineWidth (1.);
			context->setFrameColor (lineColor);

			CGradient* shading = UIEditController::getEditorDescription ().getGradient ("shading.light");
			if (shading)
			{
				path->addRect (size);
				if (horizontal)
				{
					context->fillLinearGradient (path, *shading, CPoint (size.left, size.top), CPoint (size.right, size.top));
					if (drawBottomLine)
						context->drawLine (CPoint (size.left, size.top), CPoint (size.left, size.bottom));
					if (drawTopLine)
						context->drawLine (CPoint (size.right-1, size.bottom), CPoint (size.right-1, size.top));
				}
				else
				{
					context->fillLinearGradient (path, *shading, CPoint (size.left, size.top), CPoint (size.left, size.bottom));
					if (drawTopLine)
						context->drawLine (CPoint (size.left, size.top), CPoint (size.right, size.top));
					if (drawBottomLine)
						context->drawLine (CPoint (size.right, size.bottom-1), CPoint (size.left, size.bottom-1));
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
class UIZoomSettingController : public IController, public CBaseObject
{
public:
	UIZoomSettingController (UIEditController* editController)
	: editController (editController)
	, zoomValueControl (0)
	{}

	~UIZoomSettingController ()
	{
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
	
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD
	{
		if (!zoomValueControl)
		{
			zoomValueControl = dynamic_cast<CTextEdit*> (view);
			if (zoomValueControl)
			{
				zoomValueControl->setMin (50.f);
				zoomValueControl->setMax (1000.f);
#if VSTGUI_HAS_FUNCTIONAL
				zoomValueControl->setStringToValueFunction ([] (UTF8StringPtr txt, float& result, CTextEdit*) {
					int32_t intValue = static_cast<int32_t> (strtol (txt, 0, 10));
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
#else
				// TODO: support non c++11 compilation
#endif
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
				zoomValueControl->setFrameWidth (1.);
			}
		}
		return view;
	}

	int32_t controlModifierClicked (CControl* pControl, CButtonState button) VSTGUI_OVERRIDE_VMETHOD
	{
		if (pControl == zoomValueControl && button.isRightButton ())
		{
			popupZoomMenu (pControl);
			return 1;
		}
		return 0;
	}

	void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD
	{
		if (pControl == zoomValueControl)
			editController->onZoomChanged (pControl->getValue () / 100.f);
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
	
	void popupZoomMenu (CView* anchor)
	{
		COptionMenu menu;
		menu.addEntry ("50%")->setTag (50);
		menu.addEntry ("100%")->setTag (100);
		menu.addEntry ("150%")->setTag (150);
		menu.addEntry ("200%")->setTag (200);
		CPoint location = anchor->getViewSize ().getTopLeft ();
		anchor->localToFrame (location);
		if (menu.popup (anchor->getFrame (), location))
		{
			if (CMenuItem* item = menu.getEntry (menu.getLastResult ()))
			{
				updateZoom (static_cast<float> (item->getTag ()));
			}
		}
	}

	UIEditController* editController;
	CTextEdit* zoomValueControl;
};

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIEditController::UIEditController (UIDescription* description)
: editDescription (description)
, selection (new UISelection ())
, undoManager (new UIUndoManager ())
, editView (0)
, templateController (0)
, dirty (false)
{
	gridController = new UIGridController (this, description);
	description->addDependency (this);
	undoManager->addDependency (this);
	menuController = new UIEditMenuController (this, selection, undoManager, editDescription, this);
	onTemplatesChanged ();
}

//----------------------------------------------------------------------------------------------------
UIEditController::~UIEditController ()
{
	if (tabSwitchControl)
		tabSwitchControl->removeDependency (this);
	if (templateController)
		templateController->removeDependency (this);
	undoManager->removeDependency (this);
	editDescription->removeDependency (this);
}

//----------------------------------------------------------------------------------------------------
CView* UIEditController::createEditView ()
{
	if (getEditorDescription ().parse ())
	{
		IController* controller = this;
		CView* view = getEditorDescription ().createView ("view", controller);
		if (view)
		{
			view->setAttribute (kCViewControllerAttribute, sizeof (IController*), &controller);
			CRect r;
			if (getSettings ()->getRectAttribute ("EditorSize", r))
			{
				view->setViewSize (r);
				view->setMouseableArea (r);
			}
			return view;
		}
	}
	return 0;
}

//----------------------------------------------------------------------------------------------------
CView* UIEditController::createView (const UIAttributes& attributes, const IUIDescription* description)
{
	const std::string* name = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (name)
	{
		if (*name == "UIEditView")
		{
			vstgui_assert (editView == 0);
			editView = new UIEditView (CRect (0, 0, 0, 0), editDescription);
			editView->setSelection (selection);
			editView->setUndoManager (undoManager);
			editView->setGrid (gridController);
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
		else if (*name == "UISearchTextField")
		{
			return new UISearchTextField (CRect (0, 0, 0, 0), 0, -2);
		}
	}
	return 0;
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
	kTabSwitchTag = 123456
};

//----------------------------------------------------------------------------------------------------
CView* UIEditController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	if (view == editView)
	{
		editView->setTransparency (true);
	}
	CSplitView* splitView = dynamic_cast<CSplitView*>(view);
	if (splitView)
	{
		splitViews.push_back (splitView);
		if (splitViews.size () == 1)
		{
			CFontRef font = description->getFont ("control.font");
			CColor fontColor = kWhiteCColor, frameColor = kBlackCColor, backColor = kBlackCColor;
			description->getColor ("control.font", fontColor);
			description->getColor ("control.frame", frameColor);
			description->getColor ("control.back", backColor);
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
			CTextEdit* textEdit = new CTextEdit (scaleMenuRect, zoomSettingController, 0);
			textEdit->setAttribute (kCViewControllerAttribute, sizeof (IController*), &zoomSettingController);
			CView* zoomView = zoomSettingController->verifyView (textEdit, UIAttributes (), &getEditorDescription ());
			zoomView->setAutosizeFlags (kAutosizeRight|kAutosizeTop|kAutosizeBottom);
			splitView->addViewToSeparator (0, zoomView);
			zoomSettingController->restoreSetting (*getSettings ());
		}
	}
	CControl* control = dynamic_cast<CControl*>(view);
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
				CSegmentButton* button = dynamic_cast<CSegmentButton*>(control);
				if (button)
				{
					size_t numSegments = button->getSegments ().size ();
					button->setMax (static_cast<float> (numSegments));
					tabSwitchControl = button;
					tabSwitchControl->addDependency (this);
					int32_t value = 0;
					getSettings ()->getIntegerAttribute ("TabSwitchValue", value);
					button->setSelectedSegment (static_cast<uint32_t> (value));
					static const char* segmentBitmapNames[] = {"segment-views", "segment-tags", "segment-colors", "segment-gradients", "segment-bitmaps", "segment-fonts", 0};
					size_t segmentBitmapNameIndex = 0;
					for (CSegmentButton::Segments::const_iterator it = button->getSegments().begin(), end = button->getSegments().end (); it != end; ++it)
					{
						if (segmentBitmapNames[segmentBitmapNameIndex])
						{
							CBitmap* bitmap = getEditorDescription().getBitmap (segmentBitmapNames[segmentBitmapNameIndex++]);
							if (!bitmap)
								continue;
							(*it).icon = bitmap;
							(*it).iconHighlighted = bitmap;
							(*it).iconPosition = CDrawMethods::kIconLeft;
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
		vstgui_assert (templateController == 0);
		templateController = new UITemplateController (this, editDescription, selection, undoManager, this);
		templateController->addDependency (this);
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
	return 0;
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
				selection->empty ();
				CViewContainer* container = editView->getEditView () ? dynamic_cast<CViewContainer*>(editView->getEditView ()) : 0;
				if (container)
					resetScrollViewOffsets (container);
				editView->enableEditing (control->getValue () == control->getMax () ? true : false);
				break;
			}
			case kAutosizeTag:
			{
				editView->enableAutosizing (control->getValue () == 1.f);
				break;
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIEditController::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == CControl::kMessageValueChanged)
	{
		if (tabSwitchControl == dynamic_cast<CControl*> (sender))
		{
			getSettings ()->setIntegerAttribute ("TabSwitchValue", (int32_t)tabSwitchControl->getValue ());
		}
		return kMessageNotified;
	}
	else if (message == UITemplateController::kMsgTemplateChanged)
	{
		onTemplateSelectionChanged ();
		return kMessageNotified;
	}
	else if (message == UIDescription::kMessageTemplateChanged)
	{
		onTemplatesChanged ();
		return kMessageNotified;
	}
	else if (message == UIUndoManager::kMsgChanged)
	{
		onUndoManagerChanged ();
		return kMessageNotified;
	}
	else if (message == CCommandMenuItem::kMsgMenuItemValidate)
	{
		CCommandMenuItem* item = dynamic_cast<CCommandMenuItem*>(sender);
		if (item)
			return validateMenuItem (item);
	}
	else if (message == CCommandMenuItem::kMsgMenuItemSelected)
	{
		CCommandMenuItem* item = dynamic_cast<CCommandMenuItem*>(sender);
		if (item)
			return onMenuItemSelection (item);
	}
	else if (message == UIEditView::kMsgAttached)
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
		getEditorDescription ().freePlatformResources ();
		return kMessageNotified;
	}
	else if (message == UIDescription::kMessageBeforeSave)
	{
		beforeSave ();
		return kMessageNotified;
	}
	
	return kMessageUnknown;
}

//----------------------------------------------------------------------------------------------------
void UIEditController::beforeSave ()
{
	if (editView && editView->getEditView ())
	{
		if (undoManager->canUndo ())
		{
			for (std::vector<Template>::const_iterator it = templates.begin (); it != templates.end (); it++)
				updateTemplate (it);
		}
		for (std::list<SharedPointer<CSplitView> >::const_iterator it = splitViews.begin (); it != splitViews.end (); it++)
			(*it)->storeViewSizes ();
		
		getSettings ()->setIntegerAttribute ("Version", 1);
		// find the view of this controller
		CViewContainer* container = dynamic_cast<CViewContainer*> (editView->getParentView ());
		while (container && container != container->getFrame ())
		{
			if (getViewController (container, false) == this)
			{
				getSettings ()->setRectAttribute ("EditorSize", container->getViewSize ());
				break;
			}
			container = dynamic_cast<CViewContainer*> (container->getParentView ());
		}
		undoManager->markSavePosition ();
		if (zoomSettingController)
			zoomSettingController->storeSetting (*getSettings ());
		setDirty (false);
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditController::onTemplateSelectionChanged ()
{
	if (editView && templateController)
	{
		const std::string* name = templateController->getSelectedTemplateName ();
		if ((name && *name != editTemplateName) || name == 0)
		{
			if (undoManager->canUndo () && !editTemplateName.empty ())
				updateTemplate (editTemplateName.c_str ());
			if (name)
			{
				for (std::vector<Template>::const_iterator it = templates.begin (); it != templates.end (); it++)
				{
					if (*name == (*it).name)
					{
						CView* view = (*it).view;
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
				selection->empty ();
				editView->setEditView (0);
				templateController->setTemplateView (0);
				editTemplateName = "";
			}
		}
		if (editView->getEditView ())
		{
			if (!(selection->first () && dynamic_cast<CViewContainer*> (editView->getEditView ())->isChild(selection->first (), true)))
				selection->setExclusive (editView->getEditView ());
		}
		else
			selection->empty ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditController::doCopy (bool cut)
{
	if (!editTemplateName.empty ())
		updateTemplate (editTemplateName.c_str ());
	CMemoryStream stream (1024, 1024, false);
	selection->store (stream, editDescription);
	CDropSource* dataSource = new CDropSource (stream.getBuffer (), static_cast<uint32_t> (stream.tell ()), IDataPackage::kText);
	editView->getFrame ()->setClipboard (dataSource);
	dataSource->forget ();
	if (cut)
		undoManager->pushAndPerform (new DeleteOperation (selection));
}

//----------------------------------------------------------------------------------------------------
void UIEditController::addSelectionToCurrentView (UISelection* copySelection)
{
	if (selection->total () == 0)
		return;
	CPoint offset;
	CViewContainer* container = dynamic_cast<CViewContainer*> (selection->first ());
	if (container == 0)
	{
		container = dynamic_cast<CViewContainer*> (selection->first ()->getParentView ());
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
	IDataPackage* clipboard = editView->getFrame ()->getClipboard ();
	if (clipboard)
	{
		if (clipboard->getDataType (0) == IDataPackage::kText)
		{
			const void* data;
			IDataPackage::Type type;
			uint32_t size = clipboard->getData (0, data, type);
			if (size > 0)
			{
				CMemoryStream stream ((const int8_t*)data, size, false);
				UISelection* copySelection = new UISelection ();
				if (copySelection->restore (stream, editDescription))
				{
					addSelectionToCurrentView (copySelection);
				}
				copySelection->forget ();
			}
		}
		clipboard->forget ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditController::showTemplateSettings ()
{
	if (undoManager->canUndo () && !editTemplateName.empty ())
	{
		updateTemplate (editTemplateName.c_str ());
	}
	UIDialogController* dc = new UIDialogController (this, editView->getFrame ());
	UITemplateSettingsController* tsController = new UITemplateSettingsController (editTemplateName, editDescription);
	dc->run ("template.settings", "Template Settings", "OK", "Cancel", tsController, &getEditorDescription ());
}

//----------------------------------------------------------------------------------------------------
void UIEditController::showFocusSettings ()
{
	UIDialogController* dc = new UIDialogController (this, editView->getFrame ());
	UIFocusSettingsController* fsController = new UIFocusSettingsController (editDescription);
	dc->run ("focus.settings", "Focus Drawing Settings", "OK", "Cancel", fsController, &getEditorDescription ());
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
				IDataPackage* clipboard = editView->getFrame ()->getClipboard ();
				if (clipboard)
				{
					if (clipboard->getDataType (0) == IDataPackage::kText)
						item->setEnabled (true);
					clipboard->forget ();
				}
			}
			return kMessageNotified;
		}
	}
	else if (cmdCategory == "File")
	{
		if (cmdName == "Encode Bitmaps in XML")
		{
			UIAttributes* attr = getSettings ();
			bool encodeBitmaps = false;
			if (attr && attr->getBooleanAttribute (kEncodeBitmapsSettingsKey, encodeBitmaps))
			{
				item->setChecked (encodeBitmaps);
			}
			return kMessageNotified;
		}
		else if (cmdName == "Write Windows RC File on Save")
		{
			UIAttributes* attr = getSettings ();
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
			CViewContainer* parent = dynamic_cast<CViewContainer*>(view->getParentView ());
			if (parent)
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
			bool enable = selection->total () == 1 && selection->first () && dynamic_cast<CViewContainer*> (selection->first ());
			item->setEnabled (enable);
			return kMessageNotified;
		}
	}
	return kMessageUnknown;
}

//----------------------------------------------------------------------------------------------------
bool UIEditController::doSelectionMove (const std::string& commandName, bool useGrid) const
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
bool UIEditController::doSelectionSize (const std::string& commandName, bool useGrid) const
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
		undoManager->pushAndPerform (new HierarchyMoveViewOperation (view, selection, lower));
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
void UIEditController::doSelectAllChildren ()
{
	CViewContainer* container = dynamic_cast<CViewContainer*> (selection->first ());
	selection->empty ();
	const IViewFactory* factory = editDescription->getViewFactory ();
	ViewIterator it (container);
	while (*it)
	{
		if (factory->getViewName (*it))
			selection->add (*it);
		it++;
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditController::onUndoManagerChanged ()
{
	setDirty (!undoManager->isSavePosition ());
	CView* view = selection->first ();
	if (view)
	{
		CViewContainer* templateView = dynamic_cast<CViewContainer*> (editView->getEditView ());
		if (templateView)
		{
			if (view == templateView || templateView->isChild (view, true))
			{
				return;
			}
		}
		for (std::vector<Template>::const_iterator it = templates.begin (); it != templates.end (); it++)
		{
			CViewContainer* container = dynamic_cast<CViewContainer*>((CView*)(*it).view);
			if (container && (view == container || container->isChild (view, true)))
			{
				templateController->selectTemplate ((*it).name.c_str ());
				return;
			}
		}
		selection->empty ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditController::resetScrollViewOffsets (CViewContainer* view)
{
	ViewIterator it (view);
	while (*it)
	{
		CScrollView* scrollView = dynamic_cast<CScrollView*>(*it);
		if (scrollView)
		{
			scrollView->resetScrollOffset ();
		}
		CViewContainer* container = dynamic_cast<CViewContainer*>(*it);
		if (container)
			resetScrollViewOffsets (container);
		it++;
	}
}

//----------------------------------------------------------------------------------------------------
int32_t UIEditController::onKeyDown (const VstKeyCode& code, CFrame* frame)
{
	if (frame->getModalView () == 0)
	{
		if (frame->getFocusView ())
		{
			CTextEdit* edit = dynamic_cast<CTextEdit*>(frame->getFocusView ());
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
UIAttributes* UIEditController::getSettings ()
{
	return editDescription->getCustomAttributes ("UIEditController", true);
}

//----------------------------------------------------------------------------------------------------
int32_t UIEditController::getSplitViewIndex (CSplitView* splitView)
{
	int32_t index = 0;
	for (std::list<SharedPointer<CSplitView> >::const_iterator it = splitViews.begin (); it != splitViews.end (); it++, index++)
	{
		if ((*it) == splitView)
			return index;
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
	return 0;
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
void UIEditController::getTemplateViews (std::list<CView*>& views) const
{
	for (std::vector<Template>::const_iterator it = templates.begin (); it != templates.end (); it++)
		views.push_back ((*it).view);
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performColorChange (UTF8StringPtr colorName, const CColor& newColor, bool remove)
{
	std::list<CView*> views;
	getTemplateViews (views);

	ColorChangeAction* action = new ColorChangeAction (editDescription, colorName, newColor, remove, true);
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

	TagChangeAction* action = new TagChangeAction (editDescription, tagName, tagStr, remove, true);
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

	BitmapChangeAction* action = new BitmapChangeAction (editDescription, bitmapName, bitmapPath, remove, true);
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
	
	GradientChangeAction* action = new GradientChangeAction (editDescription, gradientName, newGradient, remove, true);
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

	FontChangeAction* action = new FontChangeAction (editDescription, fontName, newFont, remove, true);
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
void UIEditController::performLiveColorChange (UTF8StringPtr colorName, const CColor& newColor)
{
	IAction* action = new ColorChangeAction (editDescription, colorName, newColor, false, true);
	action->perform ();
	delete action;

	std::list<CView*> views;
	getTemplateViews (views);

	action = new MultipleAttributeChangeAction (editDescription, views, IViewCreator::kColorType, colorName, colorName);
	action->perform ();
	delete action;
}

//----------------------------------------------------------------------------------------------------
void UIEditController::endLiveColorChange (UTF8StringPtr colorName)
{
	IDependency::DeferChanges dc (editDescription);
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
void UIEditController::performCreateNewTemplate (UTF8StringPtr name, UTF8StringPtr baseViewClassName)
{
	undoManager->pushAndPerform (new CreateNewTemplateAction (editDescription, this, name, baseViewClassName));
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performDeleteTemplate (UTF8StringPtr name)
{
	std::vector<Template>::iterator it = std::find (templates.begin (), templates.end (), name);
	if (it != templates.end ())
		undoManager->pushAndPerform (new DeleteTemplateAction (editDescription, this, (*it).view, (*it).name.c_str ()));
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performDuplicateTemplate (UTF8StringPtr name, UTF8StringPtr dupName)
{
	editDescription->changed (UIDescription::kMessageBeforeSave);
	undoManager->pushAndPerform (new DuplicateTemplateAction (editDescription, this, name, dupName));
}

//----------------------------------------------------------------------------------------------------
void UIEditController::onTemplateCreation (UTF8StringPtr name, CView* view)
{
	templates.push_back (Template (name, view));
	selection->setExclusive (view);
}

//----------------------------------------------------------------------------------------------------
void UIEditController::onTemplateNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	std::vector<Template>::iterator it = std::find (templates.begin (), templates.end (), oldName);
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
		CViewContainer* container = dynamic_cast<CViewContainer*>(view);
		if (container)
			resetScrollViewOffsets (container);
		editDescription->updateViewDescription ((*it).name.c_str (), view);
	}
}

//----------------------------------------------------------------------------------------------------
void UIEditController::updateTemplate (UTF8StringPtr name)
{
	std::vector<Template>::const_iterator it = std::find (templates.begin (), templates.end (), name);
	updateTemplate (it);
}

//----------------------------------------------------------------------------------------------------
void UIEditController::onTemplatesChanged ()
{
	std::list<const std::string*> templateNames;
	editDescription->collectTemplateViewNames (templateNames);
	for (std::list<const std::string*>::const_iterator it = templateNames.begin (); it != templateNames.end (); it++)
	{
		if (std::find (templates.begin (), templates.end (), *(*it)) == templates.end ())
		{
			OwningPointer<CView> view = editDescription->createView ((*it)->c_str (), editDescription->getController ());
			templates.push_back (Template (*(*it), view));
		}
	}
	for (std::vector<Template>::iterator it = templates.begin (); it != templates.end ();)
	{
		Template& t = (*it);
		bool found = false;
		for (std::list<const std::string*>::const_iterator it2 = templateNames.begin (); it2 != templateNames.end (); it2++)
		{
			if (t.name == *(*it2))
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

} // namespace

#endif // VSTGUI_LIVE_EDITING
