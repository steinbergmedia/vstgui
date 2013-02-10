#ifndef __uieditview__
#define __uieditview__

#include "../../lib/cviewcontainer.h"

#if VSTGUI_LIVE_EDITING

namespace VSTGUI {
class UIUndoManager;
class UISelection;
class UIDescription;
class IUIDescription;
class UICrossLines;
class IAction;
class UIGrid;
class CVSTGUITimer;

//----------------------------------------------------------------------------------------------------
class UIEditView : public CViewContainer
{
public:
	UIEditView (const CRect& size, UIDescription* uidescription);
	~UIEditView ();

	void enableEditing (bool state);
	void setEditView (CView* view);
	CView* getEditView () const;

	void setUndoManager (UIUndoManager* manager);
	UIUndoManager* getUndoManager ();

	void setSelection (UISelection* selection);
	UISelection* getSelection ();
	
	void setGrid (UIGrid* grid);

	void setupColors (IUIDescription* description);
	
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
	CMouseEventResult onMouseDown (CPoint &where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult onMouseUp (CPoint &where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult onMouseMoved (CPoint &where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;


	CBitmap* createBitmapFromSelection (UISelection* selection);
	void startDrag (CPoint& where);
	UISelection* getSelectionOutOfDrag (IDataPackage* drag);
	bool onDrop (IDataPackage* drag, const CPoint& where) VSTGUI_OVERRIDE_VMETHOD;
	void onDragEnter (IDataPackage* drag, const CPoint& where) VSTGUI_OVERRIDE_VMETHOD;
	void onDragLeave (IDataPackage* drag, const CPoint& where) VSTGUI_OVERRIDE_VMETHOD;
	void onDragMove (IDataPackage* drag, const CPoint& where) VSTGUI_OVERRIDE_VMETHOD;

	void draw (CDrawContext *pContext) VSTGUI_OVERRIDE_VMETHOD;
	void drawRect (CDrawContext *pContext, const CRect& updateRect) VSTGUI_OVERRIDE_VMETHOD;
	CView* getViewAt (const CPoint& p, bool deep, bool mustbeMouseEnabled = false) const VSTGUI_OVERRIDE_VMETHOD;
	CViewContainer* getContainerAt (const CPoint& p, bool deep) const VSTGUI_OVERRIDE_VMETHOD;
	bool advanceNextFocusView (CView* oldFocus, bool reverse) VSTGUI_OVERRIDE_VMETHOD;
	bool onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons) VSTGUI_OVERRIDE_VMETHOD;

	void looseFocus () VSTGUI_OVERRIDE_VMETHOD;
	void takeFocus () VSTGUI_OVERRIDE_VMETHOD;
	bool removed (CView* parent) VSTGUI_OVERRIDE_VMETHOD;
	bool attached (CView* parent) VSTGUI_OVERRIDE_VMETHOD;

	bool editing;
	MouseEditMode mouseEditMode;
	MouseSizeMode mouseSizeMode;
	CPoint mouseStartPoint;
 
	SharedPointer<UIUndoManager> undoManger;
	SharedPointer<UISelection> selection;
	UISelection* dragSelection;
	UIDescription* description;
	SharedPointer<UIGrid> grid;
	
	CView* highlightView;
	UICrossLines* lines;
	IAction* moveSizeOperation;
	CVSTGUITimer* editTimer;
	
	CColor crosslineForegroundColor;
	CColor crosslineBackgroundColor;
	CColor viewHighlightColor;
	CColor viewSelectionColor;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uieditview__
