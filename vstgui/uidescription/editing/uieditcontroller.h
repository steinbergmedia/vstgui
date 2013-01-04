#ifndef __uieditcontroller__
#define __uieditcontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "../uiviewfactory.h"
#include "iaction.h"
#include "../../lib/csplitview.h"
#include "../../lib/cframe.h"

namespace VSTGUI {
class UIEditView;
class UISelection;
class UIUndoManager;
class UITemplateController;
class UIEditMenuController;
class UIGridController;
class GenericStringListDataBrowserSource;

//----------------------------------------------------------------------------------------------------
class UIEditController : public CBaseObject, public IController, public ISplitViewController, public ISplitViewSeparatorDrawer, public IActionPerformer, public IKeyboardHook
{
public:
	UIEditController (UIDescription* description);

	CView* createEditView ();
	UIEditMenuController* getMenuController () const { return menuController; }
	const std::string& getEditTemplateName () const { return editTemplateName; }
	UIAttributes* getSettings ();

	static UIDescription& getEditorDescription ();
	static void setupDataSource (GenericStringListDataBrowserSource* source);
	static bool std__stringCompare (const std::string* lhs, const std::string* rhs);
	static const UTF8StringPtr kEncodeBitmapsSettingsKey;
	static const UTF8StringPtr kWriteWindowsRCFileSettingsKey;
protected:
	~UIEditController ();

	static void resetScrollViewOffsets (CViewContainer* view);

	int32_t getSplitViewIndex (CSplitView* splitView);
	void setDirty (bool state);

	virtual void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
	virtual CView* createView (const UIAttributes& attributes, IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	virtual CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	virtual IController* createSubController (UTF8StringPtr name, IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;

	// ISplitViewController
	virtual bool getSplitViewSizeConstraint (int32_t index, CCoord& minSize, CCoord& maxSize, CSplitView* splitView) VSTGUI_OVERRIDE_VMETHOD;
	virtual ISplitViewSeparatorDrawer* getSplitViewSeparatorDrawer (CSplitView* splitView) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool storeViewSize (int32_t index, const CCoord& size, CSplitView* splitView) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool restoreViewSize (int32_t index, CCoord& size, CSplitView* splitView) VSTGUI_OVERRIDE_VMETHOD;

	// ISplitViewSeparatorDrawer
	virtual void drawSplitViewSeparator (CDrawContext* context, const CRect& size, int32_t flags, int32_t index, CSplitView* splitView) VSTGUI_OVERRIDE_VMETHOD;

	// IActionPerformer
	virtual void performAction (IAction* action) VSTGUI_OVERRIDE_VMETHOD;
	virtual void performColorChange (UTF8StringPtr colorName, const CColor& newColor, bool remove = false) VSTGUI_OVERRIDE_VMETHOD;
	virtual void performTagChange (UTF8StringPtr tagName, UTF8StringPtr tagString, bool remove = false) VSTGUI_OVERRIDE_VMETHOD;
	virtual void performBitmapChange (UTF8StringPtr bitmapName, UTF8StringPtr bitmapPath, bool remove = false) VSTGUI_OVERRIDE_VMETHOD;
	virtual void performFontChange (UTF8StringPtr fontName, CFontRef newFont, bool remove = false) VSTGUI_OVERRIDE_VMETHOD;
	virtual void performColorNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) VSTGUI_OVERRIDE_VMETHOD;
	virtual void performTagNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) VSTGUI_OVERRIDE_VMETHOD;
	virtual void performFontNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) VSTGUI_OVERRIDE_VMETHOD;
	virtual void performBitmapNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) VSTGUI_OVERRIDE_VMETHOD;
	virtual void performBitmapNinePartTiledChange (UTF8StringPtr bitmapName, const CRect* offsets) VSTGUI_OVERRIDE_VMETHOD;
	virtual void performBitmapFiltersChange (UTF8StringPtr bitmapName, const std::list<SharedPointer<UIAttributes> >& filterDescription) VSTGUI_OVERRIDE_VMETHOD;
	virtual void performAlternativeFontChange (UTF8StringPtr fontName, UTF8StringPtr newAlternativeFonts) VSTGUI_OVERRIDE_VMETHOD;

	virtual void beginLiveColorChange (UTF8StringPtr colorName) VSTGUI_OVERRIDE_VMETHOD;
	virtual void performLiveColorChange (UTF8StringPtr colorName, const CColor& newColor) VSTGUI_OVERRIDE_VMETHOD;
	virtual void endLiveColorChange (UTF8StringPtr colorName) VSTGUI_OVERRIDE_VMETHOD;

	virtual void performTemplateNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) VSTGUI_OVERRIDE_VMETHOD;
	virtual void performCreateNewTemplate (UTF8StringPtr name, UTF8StringPtr baseViewClassName) VSTGUI_OVERRIDE_VMETHOD;
	virtual void performDeleteTemplate (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD;
	virtual void performDuplicateTemplate (UTF8StringPtr name, UTF8StringPtr dupName) VSTGUI_OVERRIDE_VMETHOD;

	virtual void onTemplateCreation (UTF8StringPtr name, CView* view) VSTGUI_OVERRIDE_VMETHOD;
	virtual void onTemplateNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) VSTGUI_OVERRIDE_VMETHOD;

	// IKeyboardHook
	virtual int32_t onKeyDown (const VstKeyCode& code, CFrame* frame) VSTGUI_OVERRIDE_VMETHOD;
	virtual int32_t onKeyUp (const VstKeyCode& code, CFrame* frame) VSTGUI_OVERRIDE_VMETHOD;

	SharedPointer<UIDescription> editDescription;
	OwningPointer<UISelection> selection;
	OwningPointer<UIUndoManager> undoManager;
	OwningPointer<UIGridController> gridController;
	UIEditView* editView;
	SharedPointer<UITemplateController> templateController;
	SharedPointer<UIEditMenuController> menuController;
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
	#if VSTGUI_RVALUE_REF_SUPPORT
		Template (Template&& t) { *this = std::move (t); }
		Template& operator=(Template&& t) { name = std::move (t.name); view = std::move (t.view); return *this; }
	#endif
	};
	void updateTemplate (UTF8StringPtr name);
	void updateTemplate (const std::vector<Template>::const_iterator& it);
	void onTemplatesChanged ();
	void getTemplateViews (std::list<CView*>& views);

	std::vector<Template> templates;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uieditcontroller__
