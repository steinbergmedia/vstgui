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

#include "uieditcontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uieditview.h"
#include "uiattributescontroller.h"
#include "uibitmapscontroller.h"
#include "uicolorscontroller.h"
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
#include "../../lib/controls/coptionmenu.h"
#include "../../lib/animation/animations.h"
#include "../../lib/animation/timingfunctions.h"
#include "../../lib/cdropsource.h"

#include <sstream>
#include <algorithm>
#include <assert.h>

/*

Ideas & Problems:
----------------
	- Problem: Undo Manager is cleared when template switched
		- Idea: instead of creating the template views when switching, create all template views at start and update it only when saving
		        - this may has the problem that embedded template views won't get updated on switching templates

*/

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
UIDescription& UIEditController::getEditorDescription ()
{
	static UIDescription* gUIDescription = 0;
	static bool once = true;
	if (once)
	{
		std::string descPath (__FILE__);
		unixfyPath (descPath);
		size_t sepPos = descPath.find_last_of (unixPathSeparator);
		if (sepPos != std::string::npos)
		{
			descPath.erase (sepPos+1);
			descPath += "uidescriptioneditor.uidesc";
			CFileStream stream;
			if (stream.open (descPath.c_str (), CFileStream::kReadMode))
			{
				static Xml::InputStreamContentProvider xmlProvider (stream);
				static UIDescription editorDesc (&xmlProvider);
				if (editorDesc.parse ())
					gUIDescription = &editorDesc;
			}
		}
		once = false;
	}
	return *gUIDescription;
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
		char c1 = tolower (*it);
		char c2 = tolower (*it2);
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

	void draw (CDrawContext* context)
	{
		drawGradient (context, getViewSize (), horizontal, drawTopLine, drawBottomLine);
	}

	static void drawGradient (CDrawContext* context, const CRect& _size, bool horizontal, bool drawTopLine = true, bool drawBottomLine = true)
	{
		CGraphicsPath* path = context->createGraphicsPath ();
		if (path)
		{
			static CColor lineColor, shadingTopColor, shadingBottomColor;
			static bool once = true;
			if (once)
			{
				UIEditController::getEditorDescription ().getColor ("shading.frame", lineColor);
				UIEditController::getEditorDescription ().getColor ("shading.top", shadingTopColor);
				UIEditController::getEditorDescription ().getColor ("shading.bottom", shadingBottomColor);
				once = false;
			}

			CRect size (_size);
			size.makeIntegral ();
			context->setDrawMode (kAliasing);
			context->setLineStyle (kLineSolid);
			context->setLineWidth (1.);
			context->setFrameColor (lineColor);

			CGradient* shading = path->createGradient (0., 1., shadingTopColor, shadingBottomColor);
			if (shading)
			{
				path->addRect (size);
				if (horizontal)
				{
					context->fillLinearGradient (path, *shading, CPoint (size.left, size.top), CPoint (size.right, size.top));
					CPoint p (size.left, size.top);
					context->moveTo (p);
					p.y = size.bottom;
					if (drawBottomLine)
						context->lineTo (p);
					p.x = size.right-1;
					context->moveTo (p);
					p.y = size.top;
					if (drawTopLine)
						context->lineTo (p);
				}
				else
				{
					context->fillLinearGradient (path, *shading, CPoint (size.left, size.top), CPoint (size.left, size.bottom));
					CPoint p (size.left, size.top+1);
					context->moveTo (p);
					p.x = size.right;
					if (drawTopLine)
						context->lineTo (p);
					p.y = size.bottom;
					context->moveTo (p);
					p.x = size.left;
					if (drawBottomLine)
						context->lineTo (p);
				}
				shading->forget ();
			}
			path->forget ();
		}
	}

protected:
	bool horizontal;
	bool drawTopLine;
	bool drawBottomLine;
};

//----------------------------------------------------------------------------------------------------
class UIEditControllerTextSwitch : public CParamDisplay
{
public:
	UIEditControllerTextSwitch ();

	bool attached (CView *parent);
	void setViewSize (const CRect& rect, bool invalid = true);
	void addValue (UTF8StringPtr name);
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	int32_t onKeyDown (VstKeyCode& keyCode);
	void draw (CDrawContext* pContext);
protected:
	virtual void setMax (float val) {}
	void updateValues ();

	struct Value {
		std::string name;
		CRect rect;
	};
	
	std::vector<Value> values;
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
CView* UIEditController::createView (const UIAttributes& attributes, IUIDescription* description)
{
	const std::string* name = attributes.getAttributeValue ("custom-view-name");
	if (name)
	{
		if (*name == "UIEditView")
		{
			assert (editView == 0);
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
		else if (*name == "UIEditControllerTextSwitch")
		{
			UIEditControllerTextSwitch* textSwitch = new UIEditControllerTextSwitch ();
			textSwitch->addValue ("Views");
			textSwitch->addValue ("Tags");
			textSwitch->addValue ("Colors");
			textSwitch->addValue ("Bitmaps");
			textSwitch->addValue ("Fonts");
			tabSwitchControl = textSwitch;
			tabSwitchControl->addDependency (this);
			int32_t value = 0;
			getSettings ()->getIntegerAttribute ("TabSwitchValue", value);
			textSwitch->setValue ((float)value);
			return textSwitch;
		}
		
	}
	return 0;
}

//----------------------------------------------------------------------------------------------------
CView* UIEditController::verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
{
	if (view == editView)
	{
		editView->setTransparency (true);
	}
	CSplitView* splitView = dynamic_cast<CSplitView*>(view);
	if (splitView)
	{
		splitViews.push_back (splitView);
	}
	CControl* control = dynamic_cast<CControl*>(view);
	if (control)
	{
		if (control->getTag() == 666)
		{
			notSavedControl = control;
			notSavedControl->setAlphaValue (dirty ? 1.f : 0.f);
		}
		else
		{
			const std::string* name = attributes.getAttributeValue ("custom-view-name");
			if (name)
			{
				if (*name == "enableEditingControl")
				{
					enableEditingControl = control;
					enableEditingControl->setValue (1.);
					enableEditingControl->setListener (this);
				}
			}
		}
	}
	return view;
}

//----------------------------------------------------------------------------------------------------
IController* UIEditController::createSubController (UTF8StringPtr name, IUIDescription* description)
{
	if (strcmp (name, "TemplatesController") == 0)
	{
		assert (templateController == 0);
		templateController = new UITemplateController (this, editDescription, selection, undoManager, this);
		templateController->addDependency (this);
		return templateController;
	}
	else if (strcmp (name, "MenuController") == 0)
	{
		return menuController;
	}
	else if (strcmp (name, "ViewCreatorController") == 0)
	{
		return new UIViewCreatorController (this, editDescription);
	}
	else if (strcmp (name, "AttributesController") == 0)
	{
		return new UIAttributesController (this, selection, undoManager, editDescription);
	}
	else if (strcmp (name, "TagEditController") == 0)
	{
		return new UITagsController (this, editDescription, this);
	}
	else if (strcmp (name, "ColorEditController") == 0)
	{
		return new UIColorsController (this, editDescription, this);
	}
	else if (strcmp (name, "BitmapEditController") == 0)
	{
		return new UIBitmapsController (this, editDescription, this);
	}
	else if (strcmp (name, "FontEditController") == 0)
	{
		return new UIFontsController (this, editDescription, this);
	}
	else if (strcmp (name, "GridController") == 0)
	{
		gridController->remember ();
		return gridController;
	}
	return 0;
}

//----------------------------------------------------------------------------------------------------
void UIEditController::valueChanged (CControl* control)
{
	if (editView && control == enableEditingControl)
	{
		selection->empty ();
		CViewContainer* container = editView->getEditView () ? dynamic_cast<CViewContainer*>(editView->getEditView ()) : 0;
		if (container)
			resetScrollViewOffsets (container);
		editView->enableEditing (control->getValue () == control->getMax () ? true : false);
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
		return kMessageNotified;
	}
	else if (message == UIDescription::kMessageTemplateChanged)
	{
		onTemplatesChanged ();
		return kMessageNotified;
	}
	else if (message == UIEditView::kMsgAttached)
	{
		assert (editView);
		editView->getFrame ()->registerKeyboardHook (this);
		return kMessageNotified;
	}
	else if (message == UIEditView::kMsgRemoved)
	{
		editView->getFrame ()->unregisterKeyboardHook (this);
		notify (0, UIDescription::kMessageBeforeSave);
		return kMessageNotified;
	}
	else if (message == UIDescription::kMessageBeforeSave)
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
			setDirty (false);
		}
		return kMessageNotified;
	}
	else if (message == CCommandMenuItem::kMsgMenuItemValidate)
	{
		CCommandMenuItem* item = dynamic_cast<CCommandMenuItem*>(sender);
		if (strcmp (item->getCommandCategory (), "Edit") == 0)
		{
			if (strcmp (item->getCommandName (), "Template Settings...") == 0)
			{
				item->setEnabled (editTemplateName.empty () ? false : true);
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Copy") == 0 || strcmp (item->getCommandName (), "Cut") == 0)
			{
				if (editView && selection->first () && selection->contains (editView->getEditView ()) == false)
					item->setEnabled (true);
				else
					item->setEnabled (false);
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Paste") == 0)
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
		else if (strcmp (item->getCommandCategory (), "File") == 0)
		{
			if (strcmp (item->getCommandName (), "Encode Bitmaps in XML") == 0)
			{
				UIAttributes* attr = getSettings ();
				bool encodeBitmaps = false;
				if (attr && attr->getBooleanAttribute (kEncodeBitmapsSettingsKey, encodeBitmaps))
				{
					item->setChecked (encodeBitmaps);
				}
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Write Windows RC File on Save") == 0)
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
	}
	else if (message == CCommandMenuItem::kMsgMenuItemSelected)
	{
		CCommandMenuItem* item = dynamic_cast<CCommandMenuItem*>(sender);
		if (strcmp (item->getCommandCategory (), "Edit") == 0)
		{
			if (strcmp (item->getCommandName (), "Copy") == 0 || strcmp (item->getCommandName (), "Cut") == 0)
			{
				if (!editTemplateName.empty ())
					updateTemplate (editTemplateName.c_str ());
				CMemoryStream stream (1024, 1024, false);
				selection->store (stream, dynamic_cast<UIViewFactory*> (editDescription->getViewFactory ()), editDescription);
				CDropSource* dataSource = new CDropSource (stream.getBuffer (), (int32_t)stream.tell (), IDataPackage::kText);
				editView->getFrame ()->setClipboard (dataSource);
				dataSource->forget ();
				if (strcmp (item->getCommandName (), "Cut") == 0)
				{
					undoManager->pushAndPerform (new DeleteOperation (selection));
				}
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Paste") == 0)
			{
				IDataPackage* clipboard = editView->getFrame ()->getClipboard ();
				if (clipboard)
				{
					if (clipboard->getDataType (0) == IDataPackage::kText)
					{
						const void* data;
						IDataPackage::Type type;
						int32_t size = clipboard->getData (0, data, type);
						if (size > 0)
						{
							CPoint offset;
							CViewContainer* container = dynamic_cast<CViewContainer*> (selection->first ());
							if (container == 0)
							{
								container = dynamic_cast<CViewContainer*> (selection->first ()->getParentView ());
								offset = selection->first ()->getViewSize ().getTopLeft ();
								offset.offset (gridController->getSize ().x, gridController->getSize ().y);
							}
							UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*> (editDescription->getViewFactory ());
							CMemoryStream stream ((const int8_t*)data, size, false);
							UISelection* copySelection = new UISelection ();
							if (copySelection->restore (stream, viewFactory, editDescription))
							{
								IAction* action = new ViewCopyOperation (copySelection, selection, container, offset, viewFactory, editDescription);
								undoManager->pushAndPerform (action);
								if (!editTemplateName.empty ())
									updateTemplate (editTemplateName.c_str ());
							}
							copySelection->forget ();
						}
					}
					clipboard->forget ();
				}
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Template Settings...") == 0)
			{
				if (undoManager->canUndo () && !editTemplateName.empty ())
				{
					updateTemplate (editTemplateName.c_str ());
				}
				UIDialogController* dc = new UIDialogController (this, editView->getFrame ());
				UITemplateSettingsController* tsController = new UITemplateSettingsController (editTemplateName, editDescription);
				dc->run ("template.settings", "Template Settings", "OK", "Cancel", tsController, &getEditorDescription ());
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Focus Drawing Settings...") == 0)
			{
				UIDialogController* dc = new UIDialogController (this, editView->getFrame ());
				UIFocusSettingsController* fsController = new UIFocusSettingsController (editDescription);
				dc->run ("focus.settings", "Focus Drawing Settings", "OK", "Cancel", fsController, &getEditorDescription ());
				return kMessageNotified;
			}
		}
		else if (strcmp (item->getCommandCategory (), "File") == 0)
		{
			if (strcmp (item->getCommandName (), "Encode Bitmaps in XML") == 0)
			{
				UIAttributes* attr = getSettings ();
				bool val = false;
				if (attr)
				{
					attr->getBooleanAttribute (kEncodeBitmapsSettingsKey, val);
					attr->setBooleanAttribute (kEncodeBitmapsSettingsKey, !val);
				}
				return kMessageNotified;
			}
			else if (strcmp (item->getCommandName (), "Write Windows RC File on Save") == 0)
			{
				UIAttributes* attr = getSettings ();
				bool val = false;
				if (attr)
				{
					attr->getBooleanAttribute (kWriteWindowsRCFileSettingsKey, val);
					attr->setBooleanAttribute (kWriteWindowsRCFileSettingsKey, !val);
				}
				return kMessageNotified;
			}
		}
	}
	else if (message == UIUndoManager::kMsgChanged)
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
					return kMessageNotified;
				}
			}
			for (std::vector<Template>::const_iterator it = templates.begin (); it != templates.end (); it++)
			{
				CViewContainer* container = dynamic_cast<CViewContainer*>((CView*)(*it).view);
				if (container && (view == container || container->isChild (view, true)))
				{
					templateController->selectTemplate ((*it).name.c_str ());
					return kMessageNotified;
				}
			}
			selection->empty ();
		}
		return kMessageNotified;
	}
	
	return kMessageUnknown;
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
		getSettings ()->setDoubleAttribute (str.str ().c_str (), value);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
bool UIEditController::restoreViewSize (int32_t index, CCoord& size, CSplitView* splitView)
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
			notSavedControl->addAnimation ("AlphaValueAnimation", new Animation::AlphaValueAnimation (dirty ? 1.f : 0.f), new Animation::LinearTimingFunction (400));
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
void UIEditController::getTemplateViews (std::list<CView*>& views)
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
void UIEditController::performColorNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	std::list<CView*> views;
	getTemplateViews (views);

	undoManager->startGroupAction ("Change Color Name");
	undoManager->pushAndPerform (new ColorNameChangeAction (editDescription, oldName, newName, true));
	undoManager->pushAndPerform (new MultipleAttributeChangeAction (editDescription, views, IViewCreator::kColorType, oldName, newName));
	undoManager->pushAndPerform (new ColorNameChangeAction (editDescription, oldName, newName, false));
	undoManager->endGroupAction ();
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performTagNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	std::list<CView*> views;
	getTemplateViews (views);

	undoManager->startGroupAction ("Change Tag Name");
	undoManager->pushAndPerform (new TagNameChangeAction (editDescription, oldName, newName, true));
	undoManager->pushAndPerform (new MultipleAttributeChangeAction (editDescription, views, IViewCreator::kTagType, oldName, newName));
	undoManager->pushAndPerform (new TagNameChangeAction (editDescription, oldName, newName, false));
	undoManager->endGroupAction ();
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performFontNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	std::list<CView*> views;
	getTemplateViews (views);

	undoManager->startGroupAction ("Change Font Name");
	undoManager->pushAndPerform (new FontNameChangeAction (editDescription, oldName, newName, true));
	undoManager->pushAndPerform (new MultipleAttributeChangeAction (editDescription, views, IViewCreator::kFontType, oldName, newName));
	undoManager->pushAndPerform (new FontNameChangeAction (editDescription, oldName, newName, false));
	undoManager->endGroupAction ();
}

//----------------------------------------------------------------------------------------------------
void UIEditController::performBitmapNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	std::list<CView*> views;
	getTemplateViews (views);

	undoManager->startGroupAction ("Change Bitmap Name");
	undoManager->pushAndPerform (new BitmapNameChangeAction (editDescription, oldName, newName, true));
	undoManager->pushAndPerform (new MultipleAttributeChangeAction (editDescription, views, IViewCreator::kBitmapType, oldName, newName));
	undoManager->pushAndPerform (new BitmapNameChangeAction (editDescription, oldName, newName, false));
	undoManager->endGroupAction ();
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
	undoManager->pushAndPerform (new BitmapFilterChangeAction(editDescription, bitmapName, filterDescription, true));
	undoManager->pushAndPerform (new MultipleAttributeChangeAction (editDescription, views, IViewCreator::kBitmapType, bitmapName, bitmapName));
	undoManager->pushAndPerform (new BitmapFilterChangeAction(editDescription, bitmapName, filterDescription, false));
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

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIEditControllerTextSwitch::UIEditControllerTextSwitch ()
: CParamDisplay (CRect (0, 0, 0, 0))
{
	setWantsFocus (true);
}

//----------------------------------------------------------------------------------------------------
bool UIEditControllerTextSwitch::attached (CView *parent)
{
	if (CParamDisplay::attached (parent))
	{
		updateValues ();
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
void UIEditControllerTextSwitch::addValue (UTF8StringPtr name)
{
	Value v;
	v.name = name;
	values.push_back (v);
	CControl::setMax ((float)values.size ());
	updateValues ();
}

//----------------------------------------------------------------------------------------------------
void UIEditControllerTextSwitch::setViewSize (const CRect& rect, bool invalid)
{
	CParamDisplay::setViewSize (rect, invalid);
	updateValues ();
}

//----------------------------------------------------------------------------------------------------
CMouseEventResult UIEditControllerTextSwitch::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		int32_t newValue = 0;
		for (std::vector<Value>::const_iterator it = values.begin (); it != values.end (); it++, newValue++)
		{
			if ((*it).rect.pointInside (where))
			{
				setValue ((float)newValue);
				valueChanged ();
				invalid ();
				break;
			}
		}
	}
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

//----------------------------------------------------------------------------------------------------
int32_t UIEditControllerTextSwitch::onKeyDown (VstKeyCode& keyCode)
{
	if (keyCode.modifier == 0 && keyCode.character == 0)
	{
		switch (keyCode.virt)
		{
			case VKEY_LEFT:
			{
				if (getValue () > getMin ())
				{
					beginEdit ();
					setValue (getValue () - 1.f);
					valueChanged ();
					endEdit ();
					invalid ();
					return 1;
				}
				break;
			}
			case VKEY_RIGHT:
			{
				if (getValue () < getMax () - 1.f)
				{
					beginEdit ();
					setValue (getValue () + 1.f);
					valueChanged ();
					endEdit ();
					invalid ();
					return 1;
				}
				break;
			}
		}
	}
	return -1;
}

//----------------------------------------------------------------------------------------------------
void UIEditControllerTextSwitch::draw (CDrawContext* pContext)
{
	CRect valueRect (values.at ((int32_t)getValue ()).rect);

	pContext->setDrawMode (kAliasing);
	if (!getTransparency ())
	{
		pContext->setFillColor (backColor);
		pContext->drawRect (valueRect, kDrawFilled);

		if (!(style & (k3DIn|k3DOut|kNoFrame))) 
		{
			pContext->setLineStyle (kLineSolid);
			pContext->setLineWidth (1);
			pContext->setFrameColor (frameColor);
			pContext->drawRect (valueRect);
		}
	}
	if (style & (k3DIn|k3DOut)) 
	{
		CRect r (valueRect);
		r.right--; r.top++;
		pContext->setLineWidth (1);
		pContext->setLineStyle (kLineSolid);
		if (style & k3DIn)
			pContext->setFrameColor (backColor);
		else
			pContext->setFrameColor (frameColor);
		CPoint p;
		pContext->moveTo (p (r.left, r.bottom));
		pContext->lineTo (p (r.left, r.top));
		pContext->lineTo (p (r.right, r.top));

		if (style & k3DIn)
			pContext->setFrameColor (frameColor);
		else
			pContext->setFrameColor (backColor);
		pContext->moveTo (p (r.right, r.top));
		pContext->lineTo (p (r.right, r.bottom));
		pContext->lineTo (p (r.left, r.bottom));
	}
	
	for (std::vector<Value>::const_iterator it = values.begin (); it != values.end (); it++)
	{
		drawText (pContext, (*it).name.c_str (), (*it).rect);
	}
	
}

//----------------------------------------------------------------------------------------------------
void UIEditControllerTextSwitch::updateValues ()
{
	if (isAttached ())
	{
		CCoord width = getWidth () / values.size ();
		CRect r (getViewSize ());
		r.setWidth (width);
		for (std::vector<Value>::iterator it = values.begin (); it != values.end (); it++)
		{
			(*it).rect = r;
			r.offset (width, 0);
		}
	}
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
