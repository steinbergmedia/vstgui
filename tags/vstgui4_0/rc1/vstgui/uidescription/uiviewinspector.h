//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2011, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef __uiviewinspector__
#define __uiviewinspector__

#if VSTGUI_LIVE_EDITING

#include "../vstgui.h"
#include "uidescription.h"
#include "uieditframe.h"
#include "platformsupport.h"
#include <list>

namespace VSTGUI {

class UISelection;
class PlatformWindow;
class CScrollView;

//-----------------------------------------------------------------------------
class UIViewInspector : public VSTGUIEditorInterface, public CControlListener, public CBaseObject, public IPlatformWindowDelegate
{
public:
	UIViewInspector (UISelection* selection, IActionOperator* actionOperator, void* parentPlatformWindow = 0);
	~UIViewInspector ();

	void show ();
	void hide ();
	bool isVisible () { return platformWindow ? true : false; }

	void setUIDescription (UIDescription* desc);

	void valueChanged (CControl* pControl);
	void beforeSave ();
	CMessageResult notify (CBaseObject* sender, IdStringPtr message);

	static COptionMenu* createMenuFromList (const CRect& size, CControlListener* listener, std::list<const std::string*>& names, const std::string& defaultValue, bool addNoneItem = false);
protected:
	CView* createAttributesView (CCoord width);
	void updateAttributeViews ();
	CView* createViewForAttribute (const std::string& attrName, CCoord width);
	void updateAttributeValueView (const std::string& attrName);

	void windowSizeChanged (const CRect& newSize, PlatformWindow* platformWindow);
	void windowClosed (PlatformWindow* platformWindow);
	void checkWindowSizeConstraints (CPoint& size, PlatformWindow* platformWindow);

	void addColorBitmapsToColorMenu (COptionMenu* menu, IUIDescription* desc);

	UISelection* selection;
	IActionOperator* actionOperator;
	UIDescription* description;
	CViewContainer* attributesView;
	CTextLabel* viewNameLabel;
	CScrollView* scrollView;
	PlatformWindow* platformWindow;
	void* parentPlatformWindow;
	CRect windowSize;
	std::list<CView*> attributeViews;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uiviewinspector__
