//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
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

#ifndef __uiselection__
#define __uiselection__

#if VSTGUI_LIVE_EDITING

#include "../../lib/cview.h"
#include "../../lib/idependency.h"
#include <list>
#include <string>

namespace VSTGUI {
class UIViewFactory;
class IUIDescription;
class OutputStream;
class InputStream;

typedef std::list<SharedPointer<CView> > UISelectionViewList;
//----------------------------------------------------------------------------------------------------
class UISelection : public CBaseObject, protected UISelectionViewList, public IDependency
//----------------------------------------------------------------------------------------------------
{
public:
	using UISelectionViewList::const_iterator;
	using UISelectionViewList::const_reverse_iterator;
	using UISelectionViewList::begin;
	using UISelectionViewList::end;
	using UISelectionViewList::rbegin;
	using UISelectionViewList::rend;
	
	enum {
		kMultiSelectionStyle,
		kSingleSelectionStyle
	};
	
	UISelection (int32_t style = kMultiSelectionStyle);
	~UISelection ();

	void setStyle (int32_t style);

	void add (CView* view);
	void remove (CView* view);
	void setExclusive (CView* view);
	void empty ();

	CView* first () const;

	bool contains (CView* view) const;
	bool containsParent (CView* view) const;

	int32_t total () const;
	CRect getBounds () const;
	static CRect getGlobalViewCoordinates (CView* view);

	void moveBy (const CPoint& p);
	void invalidRects () const;

	void setDragOffset (const CPoint& p) { dragOffset = p; }
	const CPoint& getDragOffset () const { return dragOffset; }
	
	static IdStringPtr kMsgSelectionWillChange;
	static IdStringPtr kMsgSelectionChanged;
	static IdStringPtr kMsgSelectionViewWillChange;
	static IdStringPtr kMsgSelectionViewChanged;

	bool store (OutputStream& stream, IUIDescription* uiDescription);
	bool restore (InputStream& stream, IUIDescription* uiDescription);
protected:

	std::list<CBaseObject*> dependencies;
	int32_t style;
	
	CPoint dragOffset;
};

//----------------------------------------------------------------------------------------------------
#define FOREACH_IN_SELECTION(__selection, view) \
	{ \
	UISelection::const_iterator __it = __selection->begin (); \
	while (__it != __selection->end ()) \
	{ \
		CView* view = (*__it);

//----------------------------------------------------------------------------------------------------
#define FOREACH_IN_SELECTION_REVERSE(__selection, view) \
	{ \
	UISelection::const_reverse_iterator __it = __selection->rbegin (); \
	while (__it != __selection->rend ()) \
	{ \
		CView* view = (*__it);

//----------------------------------------------------------------------------------------------------
#define FOREACH_IN_SELECTION_END \
		__it++; \
	} \
	}

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uiselection__
