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
	UIUndoManager* getUndoManger ();

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
	CMouseEventResult onMouseDown (CPoint &where, const CButtonState& buttons);
	CMouseEventResult onMouseUp (CPoint &where, const CButtonState& buttons);
	CMouseEventResult onMouseMoved (CPoint &where, const CButtonState& buttons);
	CMessageResult notify (CBaseObject* sender, IdStringPtr message);


	CBitmap* createBitmapFromSelection (UISelection* selection);
	void startDrag (CPoint& where);
	UISelection* getSelectionOutOfDrag (CDragContainer* drag);
	bool onDrop (CDragContainer* drag, const CPoint& where);
	void onDragEnter (CDragContainer* drag, const CPoint& where);
	void onDragLeave (CDragContainer* drag, const CPoint& where);
	void onDragMove (CDragContainer* drag, const CPoint& where);

	void draw (CDrawContext *pContext);
	void drawRect (CDrawContext *pContext, const CRect& updateRect);
	CView* getViewAt (const CPoint& p, bool deep) const;
	CViewContainer* getContainerAt (const CPoint& p, bool deep) const;
	bool advanceNextFocusView (CView* oldFocus, bool reverse);
	bool onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons);

	void looseFocus ();
	void takeFocus ();
	bool removed (CView* parent);
	bool attached (CView* parent);

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
