// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "../delegationcontroller.h"
#include "../../lib/cframe.h"
#include "../../lib/iviewlistener.h"
#include "../../lib/copenglview.h"
#include <string>
#include <list>

namespace VSTGUI {

class UIDialogController;

//------------------------------------------------------------------------
class IDialogController : public virtual IReference
{
public:
	virtual void onDialogButton1Clicked (UIDialogController*) = 0;
	virtual void onDialogButton2Clicked (UIDialogController*) = 0;
	virtual void onDialogShow (UIDialogController*) = 0;
};

//----------------------------------------------------------------------------------------------------
class UIDialogController : public NonAtomicReferenceCounted,
                           public DelegationController,
                           public IKeyboardHook,
                           public ViewListenerAdapter
{
public:
	UIDialogController (IController* baseController, CFrame* frame);
	~UIDialogController () override = default;

	void run (UTF8StringPtr templateName, UTF8StringPtr dialogTitle, UTF8StringPtr button1,
	          UTF8StringPtr button2, const SharedPointer<IDialogController>& controller,
	          UIDescription* description);
protected:
	void valueChanged (CControl* pControl) override;
	IControlListener* getControlListener (UTF8StringPtr controlTagName) override;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;

	void viewSizeChanged (CView* view, const CRect& oldSize) override;
	void viewRemoved (CView* view) override;
	
	void close ();
	void layoutButtons ();
	void collectOpenGLViews (CViewContainer* container);
	void setOpenGLViewsVisible (bool state);

	void onKeyboardEvent (KeyboardEvent& event, CFrame* frame) override;

	CFrame* frame;
	Optional<ModalViewSessionID> modalSession;
	SharedPointer<IDialogController> dialogController;
	UIDescription* dialogDescription;
	SharedPointer<CControl> button1;
	SharedPointer<CControl> button2;
	CPoint sizeDiff;
	std::string templateName;
	std::string dialogTitle;
	std::string dialogButton1;
	std::string dialogButton2;

#if VSTGUI_OPENGL_SUPPORT
	std::list<SharedPointer<COpenGLView> > openglViews;
#endif

	enum {
		kButton1Tag,
		kButton2Tag,
		kTitleTag
	};
};

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
