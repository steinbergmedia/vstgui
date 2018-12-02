// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#if VSTGUI_LIVE_EDITING

#include "../../lib/cview.h"
#include "../../lib/dispatchlist.h"
#include <list>
#include <string>

namespace VSTGUI {
class UIViewFactory;
class IUIDescription;
class OutputStream;
class InputStream;
class UISelection;

//------------------------------------------------------------------------
class IUISelectionListener
{
public:
	virtual ~IUISelectionListener () noexcept = default;
	
	virtual void selectionWillChange (UISelection* selection) = 0;
	virtual void selectionDidChange (UISelection* selection) = 0;
	virtual void selectionViewsWillChange (UISelection* selection) = 0;
	virtual void selectionViewsDidChange (UISelection* selection) = 0;
};

//----------------------------------------------------------------------------------------------------
class UISelectionListenerAdapter : public IUISelectionListener
{
public:
	void selectionWillChange (UISelection* selection) override {}
	void selectionDidChange (UISelection* selection) override {}
	void selectionViewsWillChange (UISelection* selection) override {}
	void selectionViewsDidChange (UISelection* selection) override {}
};

//----------------------------------------------------------------------------------------------------
class UISelection : public NonAtomicReferenceCounted,
                    protected ListenerProvider<UISelection, IUISelectionListener>
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

	void willChange ();
	void didChange ();
	
	void viewsWillChange ();
	void viewsDidChange ();

	bool store (OutputStream& stream, IUIDescription* uiDescription);
	bool restore (InputStream& stream, IUIDescription* uiDescription);

	struct DeferChange
	{
		DeferChange (UISelection& selection) : selection (selection)
		{
			selection.willChange ();
		}
		~DeferChange () noexcept
		{
			selection.didChange ();
		}
	private:
		UISelection& selection;
	};
	
	using ListenerProvider<UISelection, IUISelectionListener>::registerListener;
	using ListenerProvider<UISelection, IUISelectionListener>::unregisterListener;
protected:
	int32_t style;
	
	CPoint dragOffset;
	
	UISelectionViewList viewList;
	
	int32_t inChange {0};
	int32_t inViewsChange {0};
};

//----------------------------------------------------------------------------------------------------
SharedPointer<CBitmap> createBitmapFromSelection (UISelection* selection, CFrame* frame, CViewContainer* anchorView = nullptr);

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
