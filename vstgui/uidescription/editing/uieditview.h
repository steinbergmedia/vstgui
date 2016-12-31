//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins
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

#ifndef __uieditview__
#define __uieditview__

#include "../../lib/cviewcontainer.h"
#include "../../lib/ccolor.h"

#if VSTGUI_LIVE_EDITING

namespace VSTGUI {
class UIUndoManager;
class UISelection;
class UIDescription;
class IUIDescription;
class UICrossLines;
class IAction;
class UIGrid;
namespace UIEditViewInternal {
	class UIHighlightView;
} // namespace UIEditViewInternal

//----------------------------------------------------------------------------------------------------
class UIEditView : public CViewContainer
{
public:
	UIEditView (const CRect& size, UIDescription* uidescription);
	~UIEditView ();

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
	IAction* moveSizeOperation {nullptr};
	CVSTGUITimer* editTimer {nullptr};
	
	CColor crosslineForegroundColor;
	CColor crosslineBackgroundColor;
	CColor viewHighlightColor;
	CColor viewSelectionColor;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uieditview__
