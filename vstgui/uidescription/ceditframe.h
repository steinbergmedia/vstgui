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

#ifndef __ceditframe__
#define __ceditframe__

#if VSTGUI_LIVE_EDITING

#include "../lib/cframe.h"
#include "cselection.h"
#include "uidescription.h"
#include "viewfactory.h"

#include <list>
#include <map>

namespace VSTGUI {

class CrossLines;
class Grid;
class CViewInspector;
class IActionOperation;
class ViewHierarchyBrowserWindow;
class CVSTGUITimer;

//----------------------------------------------------------------------------------------------------
class IActionOperator
{
public:
	virtual ~IActionOperator () {}
	virtual void performAction (IActionOperation* action) = 0;

	virtual void performColorChange (const char* colorName, const CColor& newColor, bool remove = false) = 0;
	virtual void performTagChange (const char* tagName, long tag, bool remove = false) = 0;
	virtual void performBitmapChange (const char* bitmapName, const char* bitmapPath, bool remove = false) = 0;
	virtual void performFontChange (const char* fontName, CFontRef newFont, bool remove = false) = 0;

	virtual void performColorNameChange (const char* oldName, const char* newName) = 0;
	virtual void performTagNameChange (const char* oldName, const char* newName) = 0;
	virtual void performFontNameChange (const char* oldName, const char* newName) = 0;
	virtual void performBitmapNameChange (const char* oldName, const char* newName) = 0;

	virtual void performBitmapNinePartTiledChange (const char* bitmapName, const CRect* offsets) = 0;
	virtual void makeSelection (CView* view) = 0;
};

//----------------------------------------------------------------------------------------------------
class CEditFrame : public CFrame, public IActionOperator, public IKeyboardHook
//----------------------------------------------------------------------------------------------------
{
public:
	enum EditMode {
		kNoEditMode,
		kEditMode
	};

	CEditFrame (const CRect& size, void* windowPtr, VSTGUIEditorInterface* editor, EditMode editMode, CSelection* selection, UIDescription* description, const char* uiDescViewName);
	~CEditFrame ();

	EditMode getEditMode () const { return editMode; }
	void setEditMode (EditMode mode);

	void setGrid (int size);
	int getGrid () const;

	void setUIDescription (UIDescription* description);
	UIDescription* getUIDescription () const { return uiDescription; }

	void performAction (IActionOperation* action);
	void performColorChange (const char* colorName, const CColor& newColor, bool remove = false);
	void performTagChange (const char* tagName, long tag, bool remove = false);
	void performBitmapChange (const char* bitmapName, const char* bitmapPath, bool remove = false);
	void performFontChange (const char* fontName, CFontRef newFont, bool remove = false);

	void performColorNameChange (const char* oldName, const char* newName);
	void performTagNameChange (const char* oldName, const char* newName);
	void performFontNameChange (const char* oldName, const char* newName);
	void performBitmapNameChange (const char* oldName, const char* newName);

	void performBitmapNinePartTiledChange (const char* bitmapName, const CRect* offsets);

	void makeSelection (CView* view);

	static const char* kMsgPerformOptionsMenuAction;
	static const char* kMsgShowOptionsMenu;
	static const char* kMsgEditEnding;
//----------------------------------------------------------------------------------------------------
protected:
	CBitmap* createBitmapFromSelection (CSelection* selection);
	CSelection* getSelectionOutOfDrag (CDragContainer* drag);
	CMessageResult notify (CBaseObject* sender, const char* message);
	void showOptionsMenu (const CPoint& where);
	void createNewSubview (const CPoint& where, const char* viewName);
	void insertTemplate (const CPoint& where, const char* templateName);
	void embedSelectedViewsInto (const char* containerViewName);
	void invalidSelection ();
	void deleteSelectedViews ();
	void updateResourceBitmaps ();
	void performUndo ();
	void performRedo ();
	bool canUndo ();
	bool canRedo ();
	const char* getUndoName ();
	const char* getRedoName ();
	void emptyUndoStack ();

	void startDrag (CPoint& where);

	void drawSizingHandles (CDrawContext* context, const CRect& r);
	long selectionHitTest (const CPoint& where, CView** resultView);

	void storeAttributes ();
	void restoreAttributes ();

	// overwrites
	void draw (CDrawContext *pContext);
	void drawRect (CDrawContext *pContext, const CRect& updateRect);

	CView* getViewAt (const CPoint& p, bool deep = false) const;
	CViewContainer* getContainerAt (const CPoint& p, bool deep = true) const;
	
	CMouseEventResult onMouseDown (CPoint &where, const long& buttons);
	CMouseEventResult onMouseUp (CPoint &where, const long& buttons);
	CMouseEventResult onMouseMoved (CPoint &where, const long& buttons);

	bool onWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const long &buttons);

	bool onDrop (CDragContainer* drag, const CPoint& where);
	void onDragEnter (CDragContainer* drag, const CPoint& where);
	void onDragLeave (CDragContainer* drag, const CPoint& where);
	void onDragMove (CDragContainer* drag, const CPoint& where);

	void invalidRect (const CRect rect);

	long onKeyDown (VstKeyCode& keyCode);
	long onKeyUp (VstKeyCode& keyCode);

	long onKeyDown (const VstKeyCode& code, CFrame* frame);
	long onKeyUp (const VstKeyCode& code, CFrame* frame);

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
	CSelection* selection;
	CSelection* dragSelection;
	UIDescription* uiDescription;
	ViewHierarchyBrowserWindow* hierarchyBrowser;
	CViewInspector* inspector;
	IActionOperation* moveSizeOperation;
	CView* highlightView;
	EditMode editMode;
	MouseEditMode mouseEditMode;
	long mouseSizeMode;
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
	
	virtual const char* getName () = 0;
	virtual void perform () = 0;
	virtual void undo () = 0;
};

//----------------------------------------------------------------------------------------------------
class CFileBitmap : public CBitmap
//----------------------------------------------------------------------------------------------------
{
public:
	CFileBitmap (const char* path);
	~CFileBitmap ();

	virtual bool load (const char* path);

	char* getPath () const { return path; }

protected:
	void setPath (const char* path);
	char* path;
};

static const CColor kDefaultUIDescriptionScrollerColor = MakeCColor (255, 255, 255, 140);

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif
