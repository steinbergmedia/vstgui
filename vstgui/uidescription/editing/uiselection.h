// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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

using UISelectionViewList = std::list<SharedPointer<CView>>;
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
	~UISelection () override;

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

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uiselection__
