// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uieditview__
#define __uieditview__

#include "../../lib/cviewcontainer.h"

#if VSTGUI_LIVE_EDITING

#include "../../lib/cbitmap.h"
#include "../../lib/ccolor.h"

namespace VSTGUI {
class UIUndoManager;
class UISelection;
class UIDescription;
class IUIDescription;
class UICrossLines;
class ViewSizeChangeOperation;
class UIGrid;
namespace UIEditViewInternal {
	class UIHighlightView;
} // namespace UIEditViewInternal

//----------------------------------------------------------------------------------------------------
class UIEditView : public CViewContainer
{
public:
	UIEditView (const CRect& size, UIDescription* uidescription);
	~UIEditView () override;

	void enableEditing (bool state);
	void enableAutosizing (bool state);
	void setScale (double scale);

	void setEditView (CView* view);
	CView* getEditView () const;

	void doKeyMove (const CPoint& delta);
	void doKeySize (const CPoint& delta);

	void setUndoManager (UIUndoManager* manager);
	UIUndoManager* getUndoManager ();

	void setSelection (UISelection* selection);
	UISelection* getSelection ();
	
	void setGrid (UIGrid* grid);

	void setupColors (const IUIDescription* description);
	
	static IdStringPtr kMsgAttached;
	static IdStringPtr kMsgRemoved;
protected:
	enum MouseEditMode {
		kNoEditing,
		kDragEditing,
		kSizeEditing
	};

	enum MouseSizeMode {
		kSizeModeNone = 0,
		kSizeModeBottomRight,
		kSizeModeBottomLeft,
		kSizeModeTopRight,
		kSizeModeTopLeft,
		kSizeModeLeft,
		kSizeModeRight,
		kSizeModeTop,
		kSizeModeBottom
	};

	void invalidSelection ();
	MouseSizeMode selectionHitTest (const CPoint& where, CView** resultView);
	bool hitTestSubViews (const CPoint& where, const CButtonState& buttons = -1) override;
	CMouseEventResult onMouseDown (CPoint &where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint &where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint &where, const CButtonState& buttons) override;
	CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons) override;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;

	void doDragEditingMove (CPoint& where);
	void doSizeEditingMove (CPoint& where);

	CBitmap* createBitmapFromSelection (UISelection* selection);
	void startDrag (CPoint& where);
	UISelection* getSelectionOutOfDrag (IDataPackage* drag);
	bool onDrop (IDataPackage* drag, const CPoint& where) override;
	void onDragEnter (IDataPackage* drag, const CPoint& where) override;
	void onDragLeave (IDataPackage* drag, const CPoint& where) override;
	void onDragMove (IDataPackage* drag, const CPoint& where) override;

	void draw (CDrawContext *pContext) override;
	void drawRect (CDrawContext *pContext, const CRect& updateRect) override;
	CView* getViewAt (const CPoint& p, const GetViewOptions& options = GetViewOptions (GetViewOptions::kNone)) const override;
	CViewContainer* getContainerAt (const CPoint& p, const GetViewOptions& options = GetViewOptions (GetViewOptions::kDeep)) const override;
	bool advanceNextFocusView (CView* oldFocus, bool reverse) override;
	bool onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons) override;

	void looseFocus () override;
	void takeFocus () override;
	bool removed (CView* parent) override;
	bool attached (CView* parent) override;

	bool editing {true};
	bool autosizing {true};
	MouseEditMode mouseEditMode {kNoEditing};
	MouseSizeMode mouseSizeMode {kSizeModeNone};
	CPoint mouseStartPoint;
 
	SharedPointer<UIUndoManager> undoManger;
	SharedPointer<UISelection> selection;
	UISelection* dragSelection {nullptr};
	UIDescription* description {nullptr};
	SharedPointer<UIGrid> grid;
	
	UIEditViewInternal::UIHighlightView* highlightView {nullptr};
	CLayeredViewContainer* overlayView {nullptr};
	UICrossLines* lines {nullptr};
	ViewSizeChangeOperation* moveSizeOperation {nullptr};
	CVSTGUITimer* editTimer {nullptr};
	
	CColor crosslineForegroundColor;
	CColor crosslineBackgroundColor;
	CColor viewHighlightColor;
	CColor viewSelectionColor;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uieditview__
