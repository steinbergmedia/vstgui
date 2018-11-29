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

//----------------------------------------------------------------------------------------------------
class UISelection : public CBaseObject, public IDependency
//----------------------------------------------------------------------------------------------------
{
public:

	using UISelectionViewList = std::list<SharedPointer<CView>>;

	using const_iterator = UISelectionViewList::const_iterator;
	using const_reverse_iterator = UISelectionViewList::const_reverse_iterator;
	
	enum {
		kMultiSelectionStyle,
		kSingleSelectionStyle
	};
	
	UISelection (int32_t style = kMultiSelectionStyle);
	~UISelection () override;

	const_iterator begin () const;
	const_iterator end () const;
	const_reverse_iterator rbegin () const;
	const_reverse_iterator rend () const;
	
	void setStyle (int32_t style);

	void add (CView* view);
	void remove (CView* view);
	void setExclusive (CView* view);
	void clear ();

	CView* first () const;

	bool contains (CView* view) const;
	bool containsParent (CView* view) const;

	int32_t total () const;
	CRect getBounds () const;
	static CRect getGlobalViewCoordinates (CView* view);

	void moveBy (const CPoint& p);
	void sizeBy (const CRect& r);
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
	int32_t style;
	
	CPoint dragOffset;
	
	UISelectionViewList viewList;
};

//----------------------------------------------------------------------------------------------------
SharedPointer<CBitmap> createBitmapFromSelection (UISelection* selection, CFrame* frame, CViewContainer* anchorView = nullptr);

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uiselection__
