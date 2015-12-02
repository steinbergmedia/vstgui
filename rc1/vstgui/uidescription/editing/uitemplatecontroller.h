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
class UITemplateController : public CBaseObject, public DelegationController, public IGenericStringListDataBrowserSourceSelectionChanged, public IDependency
{
public:
	UITemplateController (IController* baseController, UIDescription* description, UISelection* selection, UIUndoManager* undoManager, IActionPerformer* actionPerformer);
	~UITemplateController ();

	const std::string* getSelectedTemplateName () const { return selectedTemplateName; }

	void selectTemplate (UTF8StringPtr name);
	void setTemplateView (CViewContainer* view);
	
	static void setupDataBrowser (CDataBrowser* orignalBrowser, CDataBrowser* dataBrowser);

	static IdStringPtr kMsgTemplateChanged;
	static IdStringPtr kMsgTemplateNameChanged;
protected:
	virtual void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD {}
	virtual CView* createView (const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	virtual CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	virtual IController* createSubController (UTF8StringPtr name, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;

	virtual void dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source) VSTGUI_OVERRIDE_VMETHOD;

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;

	SharedPointer<UIDescription> editDescription;
	SharedPointer<UISelection> selection;
	SharedPointer<UIUndoManager> undoManager;
	IActionPerformer* actionPerformer;
	CViewContainer* templateView;
	CDataBrowser* templateDataBrowser;
	UIViewListDataSource* mainViewDataSource;
	typedef std::vector<std::string> StringVector;
	StringVector templateNames;
	std::string* selectedTemplateName;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uitemplatecontroller__
