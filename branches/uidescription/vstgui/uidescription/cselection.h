//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __cselection__
#define __cselection__

#if VSTGUI_LIVE_EDITING

#include "../lib/cview.h"
#include <list>
#include <string>

namespace VSTGUI {
class ViewFactory;
class IUIDescription;
class OutputStream;
class InputStream;

//----------------------------------------------------------------------------------------------------
class CSelection : public CBaseObject, protected std::list<CView*>
//----------------------------------------------------------------------------------------------------
{
public:
	typedef std::list<CView*>::const_iterator const_iterator;
	
	enum {
		kMultiSelectionStyle,
		kSingleSelectionStyle
	};
	
	CSelection (int style = kMultiSelectionStyle);
	~CSelection ();

	void setStyle (int style);

	void add (CView* view);
	void remove (CView* view);
	void setExclusive (CView* view);
	void empty ();

	const_iterator begin () const { return std::list<CView*>::begin (); }
	const_iterator end () const { return std::list<CView*>::end (); }

	CView* first () const;

	bool contains (CView* view) const;
	bool containsParent (CView* view) const;

	int total () const;
	CRect getBounds () const;
	static CRect getGlobalViewCoordinates (CView* view);

	void moveBy (const CPoint& p);

	void addDependent (CBaseObject* obj);
	void removeDependent (CBaseObject* obj);

	void setDragOffset (const CPoint& p) { dragOffset = p; }
	const CPoint& getDragOffset () const { return dragOffset; }
	
	static const char* kMsgSelectionChanged;
	static const char* kMsgSelectionViewChanged;
	void changed (const char* what);

	bool store (OutputStream& stream, ViewFactory* viewFactory, IUIDescription* uiDescription);
	bool restore (InputStream& str, ViewFactory* viewFactory, IUIDescription* uiDescription);
protected:

	std::list<CBaseObject*> dependencies;
	int style;
	
	CPoint dragOffset;
};

//----------------------------------------------------------------------------------------------------
#define FOREACH_IN_SELECTION(__selection, view) \
	{ \
	CSelection::const_iterator __it = __selection->begin (); \
	while (__it != __selection->end ()) \
	{ \
		CView* view = (*__it);

//----------------------------------------------------------------------------------------------------
#define FOREACH_IN_SELECTION_END \
		__it++; \
	} \
	}

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif
