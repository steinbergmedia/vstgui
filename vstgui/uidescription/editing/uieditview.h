// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../lib/cviewcontainer.h"

#if VSTGUI_LIVE_EDITING

#include "../../lib/cbitmap.h"
#include "../../lib/ccolor.h"
#include "../../lib/dragging.h"

namespace VSTGUI {
class UIUndoManager;
class UISelection;
class UIDescription;
class IUIDescription;
class UICrossLines;
class ViewSizeChangeOperation;
class IGridProcessor;
namespace UIEditViewInternal {
	class UIHighlightView;
} // UIEditViewInternal

//----------------------------------------------------------------------------------------------------
class UIEditView : public CViewContainer, public IDropTarget
{
public:
	UIEditView (const CRect& size, const SPtr<UIDescription>& uidescription);
	~UIEditView () override;

	void enableEditing (bool state);
	void enableAutosizing (bool state);
	void setScale (double scale);

	void setEditView (const SPtr<CView>& view);
	SPtr<CView> getEditView () const;

	void doKeyMove (const CPoint& delta);
	void doKeySize (const CPoint& delta);

	void setUndoManager (const SPtr<UIUndoManager>& manager);
	SPtr<UIUndoManager> getUndoManager ();

	void setSelection (const SPtr<UISelection>& selection);
	SPtr<UISelection> getSelection ();

	void setGridProcessor (const SPtr<IGridProcessor>& grid);

	void setupColors (const IUIDescription& description);

protected:
	enum class MouseEditMode {
		NoEditing,
		DragEditing,
		SizeEditing,
		LassoSelection,
		WaitDrag,
		WaitLasso,
	};

	enum class MouseSizeMode {
		None,
		BottomRight,
		BottomLeft,
		TopRight,
		TopLeft,
		Left,
		Right,
		Top,
		Bottom
	};

	void updateSize ();
	void invalidSelection ();
	MouseSizeMode selectionHitTest (const CPoint& where, SPtr<CView>& resultView);
	bool hitTestSubViews (const CPoint& where, const Event& event) override;
	CMouseEventResult onMouseDown (CPoint &where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint &where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint &where, const CButtonState& buttons) override;
	CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons) override;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;
	void onKeyboardEvent (KeyboardEvent& event) override;

	std::vector<SPtr<CView>> findChildsInArea (CViewContainer& view, CRect r) const;

	void doDragEditingMove (CPoint& where);
	void doSizeEditingMove (CPoint& where);
	void onDoubleClickEditing (CView& view);

	void startDrag (CPoint& where);
	SPtr<UISelection> getSelectionOutOfDrag (const IDataPackage& drag) const;

	SPtr<IDropTarget> getDropTarget () override;
	bool onDrop (DragEventData data) override;
	DragOperation onDragEnter (DragEventData data) override;
	void onDragLeave (DragEventData data) override;
	DragOperation onDragMove (DragEventData data) override;

	void draw (CDrawContext& context) override;
	void drawRect (CDrawContext& context, const CRect& updateRect) override;
	SPtr<CView> getViewAt (const CPoint& p,
						   const GetViewOptions& options = GetViewOptions ()) const override;
	SPtr<CViewContainer> getContainerAt (
		const CPoint& p, const GetViewOptions& options = GetViewOptions ().deep ()) const override;
	bool advanceNextFocusView (const SPtr<CView>& oldFocus, bool reverse) override;
	void onMouseWheelEvent (MouseWheelEvent& event) override;
	void onZoomGestureEvent (ZoomGestureEvent& event) override;

	void looseFocus () override;
	void takeFocus () override;
	bool removed (CViewContainer& parent) override;
	bool attached (CViewContainer& parent) override;

	bool editing {true};
	bool autosizing {true};
	bool inlineAttrTextEditOpen {false};
	MouseEditMode mouseEditMode {MouseEditMode::NoEditing};
	MouseSizeMode mouseSizeMode {MouseSizeMode::None};
	CPoint mouseStartPoint;

	SPtr<UIUndoManager> undoManger;
	SPtr<UISelection> selection;
	SPtr<UISelection> dragSelection;
	SPtr<UIDescription> description;
	SPtr<IGridProcessor> gridProcessor;

	SPtr<UIEditViewInternal::UIHighlightView> highlightView;
	SPtr<CLayeredViewContainer> overlayView;
	SPtr<UICrossLines> lines;
	SPtr<ViewSizeChangeOperation> moveSizeOperation;
	SPtr<CVSTGUITimer> editTimer;
	DragStartMouseObserver dragStartMouseObserver;
	
	CColor crosslineForegroundColor;
	CColor crosslineBackgroundColor;
	CColor lassoFillColor;
	CColor lassoFrameColor;
	CColor viewHighlightColor;
	CColor viewSelectionColor;

	struct ViewAddedObserver;
	std::unique_ptr<ViewAddedObserver> editingViewAddedObserver;
	void disableExternalViewsOnInlineEditing (bool state);
};

} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
