// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uidialogcontroller__
#define __uidialogcontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "../delegationcontroller.h"
#include "../../lib/cframe.h"
#include "../../lib/iviewlistener.h"
#include "../../lib/copenglview.h"
#include <string>
#include <list>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
class UIDialogController : public CBaseObject, public DelegationController, public IKeyboardHook, public IViewListenerAdapter
{
public:
	UIDialogController (IController* baseController, CFrame* frame);
	~UIDialogController () override = default;
	
	void run (UTF8StringPtr templateName, UTF8StringPtr dialogTitle, UTF8StringPtr button1, UTF8StringPtr button2, IController* controller, UIDescription* description);

	static IdStringPtr kMsgDialogButton1Clicked;
	static IdStringPtr kMsgDialogButton2Clicked;
	static IdStringPtr kMsgDialogShow;
protected:
	void valueChanged (CControl* pControl) override;
	IControlListener* getControlListener (UTF8StringPtr controlTagName) override;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;

	void viewSizeChanged (CView* view, const CRect& oldSize) override;
	void viewRemoved (CView* view) override;
	
	void close ();
	void layoutButtons ();
	void collectOpenGLViews (CViewContainer* container);
	void setOpenGLViewsVisible (bool state);

	int32_t onKeyDown (const VstKeyCode& code, CFrame* frame) override;
	int32_t onKeyUp (const VstKeyCode& code, CFrame* frame) override;

	CFrame* frame;
	SharedPointer<CBaseObject> dialogController;
	UIDescription* dialogDescription;
	SharedPointer<CControl> button1;
	SharedPointer<CControl> button2;
	CPoint sizeDiff;
	std::string templateName;
	std::string dialogTitle;
	std::string dialogButton1;
	std::string dialogButton2;

	std::list<SharedPointer<COpenGLView> > openglViews;
		
	enum {
		kButton1Tag,
		kButton2Tag,
		kTitleTag
	};
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uidialogcontroller__
