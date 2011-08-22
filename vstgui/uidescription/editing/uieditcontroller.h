#ifndef __uieditcontroller__
#define __uieditcontroller__

#include "../uidescription.h"
#include "../uiviewfactory.h"
#include "iactionoperation.h"

namespace VSTGUI {
class UIEditView;
class UISelection;
class UIUndoManager;
class UITemplateController;
class UIEditMenuController;
class UIGridController;

//----------------------------------------------------------------------------------------------------
class UIEditController : public CBaseObject, public IController, public ISplitViewController, public ISplitViewSeparatorDrawer, public IActionOperator, public IKeyboardHook
{
public:
	UIEditController (UIDescription* description);

	CView* createEditView ();
	UIEditMenuController* getMenuController () const { return menuController; }

	static UIDescription& getEditorDescription ();
	static void setupDataSource (GenericStringListDataBrowserSource* source);
	static bool std__stringCompare (const std::string* lhs, const std::string* rhs);

protected:
	~UIEditController ();

	UIAttributes* getSettings ();
	int32_t getSplitViewIndex (CSplitView* splitView);

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

	// IActionOperator
	virtual void performAction (IActionOperation* action);
	virtual void performColorChange (UTF8StringPtr colorName, const CColor& newColor, bool remove = false);
	virtual void performTagChange (UTF8StringPtr tagName, int32_t tag, bool remove = false);
	virtual void performBitmapChange (UTF8StringPtr bitmapName, UTF8StringPtr bitmapPath, bool remove = false);
	virtual void performFontChange (UTF8StringPtr fontName, CFontRef newFont, bool remove = false);
	virtual void performColorNameChange (UTF8StringPtr oldName, UTF8StringPtr newName);
	virtual void performTagNameChange (UTF8StringPtr oldName, UTF8StringPtr newName);
	virtual void performFontNameChange (UTF8StringPtr oldName, UTF8StringPtr newName);
	virtual void performBitmapNameChange (UTF8StringPtr oldName, UTF8StringPtr newName);
	virtual void performBitmapNinePartTiledChange (UTF8StringPtr bitmapName, const CRect* offsets);
	virtual void makeSelection (CView* view);

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
	
	std::string editTemplateName;
	std::list<SharedPointer<CSplitView> > splitViews;
};

} // namespace

#endif // __uieditcontroller__
