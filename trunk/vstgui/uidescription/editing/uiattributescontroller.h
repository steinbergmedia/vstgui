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

#ifndef __uiattributescontroller__
#define __uiattributescontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "../delegationcontroller.h"
#include "../iviewcreator.h"
#include "uiundomanager.h"
#include "../../lib/controls/ctextedit.h"

namespace VSTGUI {
class CRowColumnView;
class UIViewFactory;

namespace UIAttributeControllers {
class Controller;
}

//----------------------------------------------------------------------------------------------------
class UIAttributesController : public CBaseObject, public DelegationController
{
public:
	UIAttributesController (IController* baseController, UISelection* selection, UIUndoManager* undoManager, UIDescription* description);
	~UIAttributesController ();
	
	void beginLiveAttributeChange (const std::string& name, const std::string& currentValue);
	void endLiveAttributeChange ();
	void performAttributeChange (const std::string& name, const std::string& value);
protected:
	typedef std::list<std::string> StringList;

	CView* createViewForAttribute (const std::string& attrName);
	void rebuildAttributesView ();
	void validateAttributeViews ();
	CView* createValueViewForAttributeType (const UIViewFactory* viewFactory, CView* view, const std::string& attrName, IViewCreator::AttrType attrType);
	void getConsolidatedAttributeNames (StringList& result, const std::string& filter);

	void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;
	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	IController* createSubController (IdStringPtr name, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	IControlListener* getControlListener (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD;

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;

	SharedPointer<UISelection> selection;
	SharedPointer<UIUndoManager> undoManager;
	SharedPointer<UIDescription> editDescription;
	OwningPointer<CVSTGUITimer> timer;
	IAction* liveAction;

	typedef std::list<UIAttributeControllers::Controller*> UIAttributeControllerList;
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
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uiattributescontroller__
