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

	static UIDescription& getEditorDescription ();
	static void setupDataSource (GenericStringListDataBrowserSource* source);
	static bool std__stringCompare (const std::string* lhs, const std::string* rhs);

protected:
	~UIEditController ();

	static void resetScrollViewOffsets (CViewContainer* view);

	UIAttributes* getSettings ();
	int32_t getSplitViewIndex (CSplitView* splitView);
	void setDirty (bool state);

	virtual void valueChanged (CControl* pControl);
	virtual CView* createView (const UIAttributes& attributes, IUIDescription* description);
	virtual CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description);
	virtual IController* createSubController (UTF8StringPtr name, IUIDescription* description);

	CMessageResult notify (CBaseObject* sender, IdStringPtr message);

	// ISplitViewController
	virtual bool getSplitViewSizeConstraint (int32_t index, CCoord& minSize, CCoord& maxSize, CSplitView* splitView);
	virtual ISplitViewSeparatorDrawer* getSplitViewSeparatorDrawer (CSplitView* splitView);
	virtual bool storeViewSize (int32_t index, const CCoord& size, CSplitView* splitView);
	virtual bool restoreViewSize (int32_t index, CCoord& size, CSplitView* splitView);

	// ISplitViewSeparatorDrawer
	virtual void drawSplitViewSeparator (CDrawContext* context, const CRect& size, int32_t flags, int32_t index, CSplitView* splitView);

	// IActionPerformer
	virtual void performAction (IAction* action);
	virtual void performColorChange (UTF8StringPtr colorName, const CColor& newColor, bool remove = false);
	virtual void performTagChange (UTF8StringPtr tagName, int32_t tag, bool remove = false);
	virtual void performBitmapChange (UTF8StringPtr bitmapName, UTF8StringPtr bitmapPath, bool remove = false);
	virtual void performFontChange (UTF8StringPtr fontName, CFontRef newFont, bool remove = false);
	virtual void performColorNameChange (UTF8StringPtr oldName, UTF8StringPtr newName);
	virtual void performTagNameChange (UTF8StringPtr oldName, UTF8StringPtr newName);
	virtual void performFontNameChange (UTF8StringPtr oldName, UTF8StringPtr newName);
	virtual void performBitmapNameChange (UTF8StringPtr oldName, UTF8StringPtr newName);
	virtual void performBitmapNinePartTiledChange (UTF8StringPtr bitmapName, const CRect* offsets);
	virtual void performAlternativeFontChange (UTF8StringPtr fontName, UTF8StringPtr newAlternativeFonts);

	virtual void beginLiveColorChange (UTF8StringPtr colorName);
	virtual void performLiveColorChange (UTF8StringPtr colorName, const CColor& newColor);
	virtual void endLiveColorChange (UTF8StringPtr colorName);

	virtual void performTemplateNameChange (UTF8StringPtr oldName, UTF8StringPtr newName);
	virtual void performCreateNewTemplate (UTF8StringPtr name, UTF8StringPtr baseViewClassName);
	virtual void performDeleteTemplate (UTF8StringPtr name);
	virtual void performDuplicateTemplate (UTF8StringPtr name, UTF8StringPtr dupName);

	virtual void onTemplateCreation (UTF8StringPtr name, CView* view);
	virtual void onTemplateNameChange (UTF8StringPtr oldName, UTF8StringPtr newName);

	// IKeyboardHook
	virtual int32_t onKeyDown (const VstKeyCode& code, CFrame* frame);
	virtual int32_t onKeyUp (const VstKeyCode& code, CFrame* frame);

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
		bool operator==(const std::string& n) { return name == n; }
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
