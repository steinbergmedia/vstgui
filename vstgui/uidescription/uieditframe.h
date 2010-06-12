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

#ifndef __uieditframe__
#define __uieditframe__

#if VSTGUI_LIVE_EDITING

#include "../lib/cframe.h"
#include "uiselection.h"
#include "uidescription.h"
#include "uiviewfactory.h"

#include <list>
#include <map>

namespace VSTGUI {

class CrossLines;
class Grid;
class UIViewInspector;
class IActionOperation;
class UIViewHierarchyBrowserWindow;
class CVSTGUITimer;

//----------------------------------------------------------------------------------------------------
class IActionOperator
{
public:
	virtual ~IActionOperator () {}
	virtual void performAction (IActionOperation* action) = 0;

	virtual void performColorChange (UTF8StringPtr colorName, const CColor& newColor, bool remove = false) = 0;
	virtual void performTagChange (UTF8StringPtr tagName, int32_t tag, bool remove = false) = 0;
	virtual void performBitmapChange (UTF8StringPtr bitmapName, UTF8StringPtr bitmapPath, bool remove = false) = 0;
	virtual void performFontChange (UTF8StringPtr fontName, CFontRef newFont, bool remove = false) = 0;

	virtual void performColorNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) = 0;
	virtual void performTagNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) = 0;
	virtual void performFontNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) = 0;
	virtual void performBitmapNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) = 0;

	virtual void performBitmapNinePartTiledChange (UTF8StringPtr bitmapName, const CRect* offsets) = 0;
	virtual void makeSelection (CView* view) = 0;
};

//----------------------------------------------------------------------------------------------------
class UIEditFrame : public CFrame, public IActionOperator, public IKeyboardHook
//----------------------------------------------------------------------------------------------------
{
public:
	enum EditMode {
		kNoEditMode,
		kEditMode
	};

	UIEditFrame (const CRect& size, void* windowPtr, VSTGUIEditorInterface* editor, EditMode editMode, UISelection* selection, UIDescription* description, UTF8StringPtr uiDescViewName);
	~UIEditFrame ();

	EditMode getEditMode () const { return editMode; }
	void setEditMode (EditMode mode);

	void setGrid (int32_t size);
	int32_t getGrid () const;

	void setUIDescription (UIDescription* description);
	UIDescription* getUIDescription () const { return uiDescription; }

	void performAction (IActionOperation* action);
	void performColorChange (UTF8StringPtr colorName, const CColor& newColor, bool remove = false);
	void performTagChange (UTF8StringPtr tagName, int32_t tag, bool remove = false);
	void performBitmapChange (UTF8StringPtr bitmapName, UTF8StringPtr bitmapPath, bool remove = false);
	void performFontChange (UTF8StringPtr fontName, CFontRef newFont, bool remove = false);

	void performColorNameChange (UTF8StringPtr oldName, UTF8StringPtr newName);
	void performTagNameChange (UTF8StringPtr oldName, UTF8StringPtr newName);
	void performFontNameChange (UTF8StringPtr oldName, UTF8StringPtr newName);
	void performBitmapNameChange (UTF8StringPtr oldName, UTF8StringPtr newName);

	void performBitmapNinePartTiledChange (UTF8StringPtr bitmapName, const CRect* offsets);

	void makeSelection (CView* view);

	static IdStringPtr kMsgPerformOptionsMenuAction;
	static IdStringPtr kMsgShowOptionsMenu;
	static IdStringPtr kMsgEditEnding;
//----------------------------------------------------------------------------------------------------
protected:
	CBitmap* createBitmapFromSelection (UISelection* selection);
	UISelection* getSelectionOutOfDrag (CDragContainer* drag);
	CMessageResult notify (CBaseObject* sender, IdStringPtr message);
	void showOptionsMenu (const CPoint& where);
	void createNewSubview (const CPoint& where, UTF8StringPtr viewName);
	void insertTemplate (const CPoint& where, UTF8StringPtr templateName);
	void embedSelectedViewsInto (IdStringPtr containerViewName);
	void invalidSelection ();
	void deleteSelectedViews ();
	void updateResourceBitmaps ();
	void performUndo ();
	void performRedo ();
	bool canUndo ();
	bool canRedo ();
	UTF8StringPtr getUndoName ();
	UTF8StringPtr getRedoName ();
	void emptyUndoStack ();

	void startDrag (CPoint& where);

	void drawSizingHandles (CDrawContext* context, const CRect& r);
	int32_t selectionHitTest (const CPoint& where, CView** resultView);

	void storeAttributes ();
	void restoreAttributes ();

	// overwrites
	void draw (CDrawContext *pContext);
	void drawRect (CDrawContext *pContext, const CRect& updateRect);

	CView* getViewAt (const CPoint& p, bool deep = false) const;
	CViewContainer* getContainerAt (const CPoint& p, bool deep = true) const;
	
	CMouseEventResult onMouseDown (CPoint &where, const CButtonState& buttons);
	CMouseEventResult onMouseUp (CPoint &where, const CButtonState& buttons);
	CMouseEventResult onMouseMoved (CPoint &where, const CButtonState& buttons);

	bool onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const CButtonState &buttons);

	bool onDrop (CDragContainer* drag, const CPoint& where);
	void onDragEnter (CDragContainer* drag, const CPoint& where);
	void onDragLeave (CDragContainer* drag, const CPoint& where);
	void onDragMove (CDragContainer* drag, const CPoint& where);

	void invalidRect (const CRect& rect);

	int32_t onKeyDown (VstKeyCode& keyCode);
	int32_t onKeyUp (VstKeyCode& keyCode);

	int32_t onKeyDown (const VstKeyCode& code, CFrame* frame);
	int32_t onKeyUp (const VstKeyCode& code, CFrame* frame);

	void onViewAdded (CView* pView);
	void onViewRemoved (CView* pView);
	
	enum MouseEditMode {
		kNoEditing,
		kDragEditing,
		kSizeEditing
	};

	CVSTGUITimer* timer;
	CVSTGUITimer* editTimer;
	CrossLines* lines;
	Grid* grid;
	UISelection* selection;
	UISelection* dragSelection;
	UIDescription* uiDescription;
	UIViewHierarchyBrowserWindow* hierarchyBrowser;
	UIViewInspector* inspector;
	IActionOperation* moveSizeOperation;
	CView* highlightView;
	EditMode editMode;
	MouseEditMode mouseEditMode;
	int32_t mouseSizeMode;
	bool showLines;
	
	CPoint mouseStartPoint;
	CPoint dragSelectionOffset;
	
	std::string templateName;
	std::string savePath;

	std::list<IActionOperation*> undoStackList;
	std::list<IActionOperation*>::iterator undoStack;
};

//----------------------------------------------------------------------------------------------------
class IActionOperation
{
public:
	virtual ~IActionOperation () {}
	
	virtual UTF8StringPtr getName () = 0;
	virtual void perform () = 0;
	virtual void undo () = 0;
};

//-----------------------------------------------------------------------------
static bool std__stringCompare (const std::string* lhs, const std::string* rhs)
{
  return *lhs < *rhs;
}

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uieditframe__
