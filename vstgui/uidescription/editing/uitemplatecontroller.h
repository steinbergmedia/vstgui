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

#ifndef __uitemplatecontroller__
#define __uitemplatecontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "uiundomanager.h"
#include "iaction.h"
#include "../delegationcontroller.h"
#include "../../lib/cdatabrowser.h"
#include <vector>
#include <list>

namespace VSTGUI {
class UIViewListDataSource;

//----------------------------------------------------------------------------------------------------
class UITemplateController : public CBaseObject, public DelegationController, public IContextMenuController2, public IGenericStringListDataBrowserSourceSelectionChanged, public IDependency
{
public:
	UITemplateController (IController* baseController, UIDescription* description, UISelection* selection, UIUndoManager* undoManager, IActionPerformer* actionPerformer);
	~UITemplateController () override;

	const UTF8String* getSelectedTemplateName () const { return selectedTemplateName; }

	void selectTemplate (UTF8StringPtr name);
	void setTemplateView (CViewContainer* view);
	
	static void setupDataBrowser (CDataBrowser* orignalBrowser, CDataBrowser* dataBrowser);

	static IdStringPtr kMsgTemplateChanged;
	static IdStringPtr kMsgTemplateNameChanged;
protected:
	void valueChanged (CControl* pControl) override {}
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	IController* createSubController (UTF8StringPtr name, const IUIDescription* description) override;

	void dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source) override;

	void appendContextMenuItems (COptionMenu& contextMenu, CView* view, const CPoint& where) override;

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;

	SharedPointer<UIDescription> editDescription;
	SharedPointer<UISelection> selection;
	SharedPointer<UIUndoManager> undoManager;
	IActionPerformer* actionPerformer;
	CViewContainer* templateView;
	CDataBrowser* templateDataBrowser;
	UIViewListDataSource* mainViewDataSource;
	GenericStringListDataBrowserSource::StringVector templateNames;
	const UTF8String* selectedTemplateName;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uitemplatecontroller__
