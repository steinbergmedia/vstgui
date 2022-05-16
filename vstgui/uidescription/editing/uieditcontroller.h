// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "../iviewcreator.h"
#include "../icontroller.h"
#include "../uidescriptionlistener.h"
#include "iaction.h"
#include "uiundomanager.h"
#include "uitemplatecontroller.h"
#include "../../lib/csplitview.h"
#include "../../lib/cframe.h"
#include "../../lib/controls/icommandmenuitemtarget.h"

#include <vector>

namespace VSTGUI {
class UIEditView;
class UISelection;
class UITemplateController;
class UIEditMenuController;
class UIGridController;
class UIZoomSettingController;
class GenericStringListDataBrowserSource;
class CCommandMenuItem;

//----------------------------------------------------------------------------------------------------
class UIEditController : public CBaseObject,
                         public IController,
                         public IContextMenuController2,
                         public ISplitViewController,
                         public ISplitViewSeparatorDrawer,
                         public IActionPerformer,
                         public IKeyboardHook,
                         public CommandMenuItemTargetAdapter,
                         public UIDescriptionListenerAdapter,
                         public IUIUndoManagerListener,
                         public IUITemplateControllerListener
{
public:
	UIEditController (UIDescription* description);
	void setDarkTheme (bool state); // must be called before createEditView
	bool usesDarkTheme () const;
	CView* createEditView ();
	UIEditMenuController* getMenuController () const { return menuController; }
	UIUndoManager* getUndoManager () const { return undoManager; }
	const std::string& getEditTemplateName () const { return editTemplateName; }
	SharedPointer<UIAttributes> getSettings ();
	int32_t getSaveOptions ();
	
	void onZoomChanged (double zoom);

	void addSelectionToCurrentView (UISelection* selection);

	static SharedPointer<UIDescription> getEditorDescription ();
	static void setupDataSource (GenericStringListDataBrowserSource* source);
	static bool std__stringCompare (const std::string* lhs, const std::string* rhs);
	static const UTF8StringPtr kEncodeBitmapsSettingsKey;
	static const UTF8StringPtr kWriteWindowsRCFileSettingsKey;
protected:
	~UIEditController () override;

	static void resetScrollViewOffsets (CViewContainer* view);

	int32_t getSplitViewIndex (CSplitView* splitView);
	void setDirty (bool state);

	void valueChanged (CControl* pControl) override;
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	IController* createSubController (UTF8StringPtr name, const IUIDescription* description) override;

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;

	// IUITemplateControllerListener
	void onTemplateSelectionChanged () override;

	// IUIUndoManagerListener
	void onUndoManagerChange () override;

	// IContextMenuController2
	void appendContextMenuItems (COptionMenu& contextMenu, CView* view, const CPoint& where) override;

	// ISplitViewController
	bool getSplitViewSizeConstraint (int32_t index, CCoord& minSize, CCoord& maxSize, CSplitView* splitView) override;
	ISplitViewSeparatorDrawer* getSplitViewSeparatorDrawer (CSplitView* splitView) override;
	bool storeViewSize (int32_t index, const CCoord& size, CSplitView* splitView) override;
	bool restoreViewSize (int32_t index, CCoord& size, CSplitView* splitView) override;

	// ISplitViewSeparatorDrawer
	void drawSplitViewSeparator (CDrawContext* context, const CRect& size, int32_t flags, int32_t index, CSplitView* splitView) override;

	// IActionPerformer
	void performAction (IAction* action) override;
	void performColorChange (UTF8StringPtr colorName, const CColor& newColor, bool remove = false) override;
	void performTagChange (UTF8StringPtr tagName, UTF8StringPtr tagString, bool remove = false) override;
	void performBitmapChange (UTF8StringPtr bitmapName, UTF8StringPtr bitmapPath, bool remove = false) override;
	void performGradientChange (UTF8StringPtr gradientName, CGradient* newGradient, bool remove = false) override;
	void performFontChange (UTF8StringPtr fontName, CFontRef newFont, bool remove = false) override;
	void performColorNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) override;
	void performTagNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) override;
	void performFontNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) override;
	void performGradientNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) override;
	void performBitmapNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) override;
	void performBitmapNinePartTiledChange (UTF8StringPtr bitmapName, const CRect* offsets) override;
	void performBitmapFiltersChange (UTF8StringPtr bitmapName, const std::list<SharedPointer<UIAttributes> >& filterDescription) override;
	void performAlternativeFontChange (UTF8StringPtr fontName, UTF8StringPtr newAlternativeFonts) override;

	void beginLiveColorChange (UTF8StringPtr colorName) override;
	void performLiveColorChange (UTF8StringPtr colorName, const CColor& newColor) override;
	void endLiveColorChange (UTF8StringPtr colorName) override;

	void performTemplateNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) override;
	void performTemplateMinMaxSizeChange (UTF8StringPtr templateName, CPoint minSize, CPoint maxSize) override;
	void performCreateNewTemplate (UTF8StringPtr name, UTF8StringPtr baseViewClassName) override;
	void performDeleteTemplate (UTF8StringPtr name) override;
	void performDuplicateTemplate (UTF8StringPtr name, UTF8StringPtr dupName) override;

	void onTemplateCreation (UTF8StringPtr name, CView* view) override;
	void onTemplateNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) override;

	void performChangeFocusDrawingSettings (const FocusDrawingSettings& newSettings) override;

	void beginGroupAction (UTF8StringPtr name) override;
	void finishGroupAction () override;

	// IKeyboardHook
	void onKeyboardEvent (KeyboardEvent& event, CFrame* frame) override;

	// CommandMenuItemTargetAdapter
	bool validateCommandMenuItem (CCommandMenuItem* item) override;
	bool onCommandMenuItemSelected (CCommandMenuItem* item) override;

	SharedPointer<UIDescription> editDescription;
	SharedPointer<UIDescription> editorDesc;
	SharedPointer<UISelection> selection;
	SharedPointer<UIUndoManager> undoManager;
	SharedPointer<UIGridController> gridController;
	CView* baseView {nullptr};
	UIEditView* editView {nullptr};
	SharedPointer<UITemplateController> templateController;
	SharedPointer<UIEditMenuController> menuController;
	SharedPointer<UIZoomSettingController> zoomSettingController;
	SharedPointer<CControl> enableEditingControl;
	SharedPointer<CControl> notSavedControl;
	SharedPointer<CControl> tabSwitchControl;
	
	std::string editTemplateName;
	std::list<SharedPointer<CSplitView> > splitViews;
	
	bool dirty;
	
	struct Template {
		std::string name;
		SharedPointer<CView> view;

		Template (const std::string& n, CView* v) : name (n), view (v) {}
		Template (const Template& c) : name (c.name), view (c.view) {}
		bool operator==(const Template& t) { return name == t.name && view == t.view; }
		bool operator==(const std::string& n) { return name == n; }
		Template& operator=(const Template& t) { name = t.name; view = t.view; return *this; }
		Template (Template&& t) noexcept { *this = std::move (t); }
		Template& operator=(Template&& t) noexcept { name = std::move (t.name); view = std::move (t.view); return *this; }
	};
	void updateTemplate (UTF8StringPtr name);
	void updateTemplate (const std::vector<Template>::const_iterator& it);
	void onTemplatesChanged ();
	void getTemplateViews (std::list<CView*>& views) const;

	std::vector<Template> templates;
private:
	void beforeUIDescSave (UIDescription* desc) override;
	void onUIDescTemplateChanged (UIDescription* desc) override;
	bool doUIDescTemplateUpdate (UIDescription* desc, UTF8StringPtr name) override;

	void beforeSave ();
	CMessageResult validateMenuItem (CCommandMenuItem* item);
	CMessageResult onMenuItemSelection (CCommandMenuItem* item);
	void doCopy (bool cut = false);
	void doPaste ();
	void showTemplateSettings ();
	void showFocusSettings ();
	bool doSelectionMove (const UTF8String& commandName, bool useGrid) const;
	bool doSelectionSize (const UTF8String& commandName, bool useGrid) const;
	bool doZOrderAction (bool lower);
	void doSelectAllChildren ();
	void doSelectParents ();
	void doSelectViewInHierarchyBrowser (CView* view);
	void doChangeTheme (bool dark);
	
	void onUndoManagerChanged ();
	template<typename NameChangeAction, IViewCreator::AttrType attrType> void performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName, IdStringPtr groupActionName);

	std::string onlyTemplateToUpdateName;
};

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
