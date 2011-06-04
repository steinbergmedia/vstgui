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

#ifndef __uiviewhierarchybrowser__
#define __uiviewhierarchybrowser__

#if VSTGUI_LIVE_EDITING

#include "../lib/cframe.h"
#include "uidescription.h"
#include "uipanelbase.h"

namespace VSTGUI {

class CDataBrowser;
class UIViewHierarchyData;
class ViewHierarchyPathView;
class IActionOperator;

//-----------------------------------------------------------------------------
class UIViewHierarchyBrowser : public CViewContainer
{
public:
	UIViewHierarchyBrowser (const CRect& rect, CViewContainer* baseView, UIDescription* description, IActionOperator* actionOperator);
	~UIViewHierarchyBrowser ();
	
	void setCurrentView (CViewContainer* newView);
	CViewContainer* getCurrentView () const { return currentView; }
	CViewContainer* getBaseView () const { return baseView; }

	void changeBaseView (CViewContainer* newBaseView);
	void notifyHierarchyChange (CView* view, bool wasRemoved = false);
protected:
	CViewContainer* baseView;
	CViewContainer* currentView;

	CDataBrowser* browser;
	UIViewHierarchyData* data;
	ViewHierarchyPathView* pathView;
};

//-----------------------------------------------------------------------------
class UIViewHierarchyBrowserWindow : public UIPanelBase
{
public:
	UIViewHierarchyBrowserWindow (CViewContainer* baseView, CBaseObject* owner, UIDescription* description, void* parentPlatformWindow = 0);
	~UIViewHierarchyBrowserWindow ();

	void changeBaseView (CViewContainer* newBaseView);
	void notifyHierarchyChange (CView* view, bool wasRemoved = false);
	
protected:
	CFrame* createFrame (void* platformWindow, const CCoord& width, const CCoord& height);

	void windowClosed (PlatformWindow* platformWindow);
	void checkWindowSizeConstraints (CPoint& size, PlatformWindow* platformWindow);

	UIViewHierarchyBrowser* browser;
	UIDescription* description;
};


} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uiviewhierarchybrowser__
