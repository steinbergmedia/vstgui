// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "../delegationcontroller.h"
#include "../iviewcreator.h"
#include "../uidescriptionlistener.h"
#include "uiundomanager.h"
#include "../../lib/controls/ctextedit.h"

namespace VSTGUI {
class CRowColumnView;
class UIViewFactory;

namespace UIAttributeControllers {
class Controller;
}

//----------------------------------------------------------------------------------------------------
class UIAttributesController : public NonAtomicReferenceCounted,
                               public DelegationController,
                               public UIDescriptionListenerAdapter,
                               public UISelectionListenerAdapter,
                               public IUIUndoManagerListener
{
public:
	UIAttributesController (IController* baseController, UISelection* selection, UIUndoManager* undoManager, UIDescription* description);
	~UIAttributesController () override;
	
	void beginLiveAttributeChange (const std::string& name, const std::string& currentValue);
	void endLiveAttributeChange ();
	void performAttributeChange (const std::string& name, const std::string& value);
protected:
	using StringList = std::list<std::string>;

	CView* createViewForAttribute (const std::string& attrName);
	void rebuildAttributesView ();
	void validateAttributeViews ();
	CView* createValueViewForAttributeType (const UIViewFactory* viewFactory, CView* view, const std::string& attrName, IViewCreator::AttrType attrType);
	void getConsolidatedAttributeNames (StringList& result, const std::string& filter);

	void valueChanged (CControl* pControl) override;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	IController* createSubController (IdStringPtr name, const IUIDescription* description) override;
	IControlListener* getControlListener (UTF8StringPtr name) override;

	void onUIDescTagChanged (UIDescription* desc) override;
	void onUIDescColorChanged (UIDescription* desc) override;
	void onUIDescFontChanged (UIDescription* desc) override;
	void onUIDescBitmapChanged (UIDescription* desc) override;
	void onUIDescTemplateChanged (UIDescription* desc) override;
	void onUIDescGradientChanged (UIDescription* desc) override;

	void selectionDidChange (UISelection* selection) override;
	void selectionViewsDidChange (UISelection* selection) override;

	void onUndoManagerChange () override;

	SharedPointer<UISelection> selection;
	SharedPointer<UIUndoManager> undoManager;
	SharedPointer<UIDescription> editDescription;
	IAction* liveAction;

	using UIAttributeControllerList = std::list<UIAttributeControllers::Controller*>;
	UIAttributeControllerList attributeControllers;

	enum {
		kSearchFieldTag = 100,
		kViewNameTag = 101
	};

	SharedPointer<CTextEdit> searchField;
	CTextLabel* viewNameLabel;
	CRowColumnView* attributeView;

	std::string filterString;

	const std::string* currentAttributeName;
	
	bool rebuildRequested{false};
};

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
