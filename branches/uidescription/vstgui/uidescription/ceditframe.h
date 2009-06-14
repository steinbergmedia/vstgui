/*
 *  ceditframe.h
 *
 *  Created by Arne Scheffler on 12/8/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __ceditframe__
#define __ceditframe__

#if VSTGUI_LIVE_EDITING

#include "../vstgui.h"
#include "../cvstguitimer.h"
#include "cselection.h"
#include "uidescription.h"
#include "viewfactory.h"

#include <list>
#include <map>

BEGIN_NAMESPACE_VSTGUI

class CrossLines;
class Grid;
class CViewInspector;
class IActionOperation;
class ViewHierarchyBrowserWindow;

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
	
	virtual void makeSelection (CView* view) = 0;
};

//----------------------------------------------------------------------------------------------------
class CEditFrame : public CFrame, public IActionOperator
//----------------------------------------------------------------------------------------------------
{
public:
	enum EditMode {
		kNoEditMode,
		kEditMode,
		kPaletteMode
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

	void makeSelection (CView* view);

	static const char* kMsgPerformOptionsMenuAction;
	static const char* kMsgShowOptionsMenu;
	static const char* kMsgEditEnding;
//----------------------------------------------------------------------------------------------------
protected:
	CBitmap* createBitmapFromSelection (CSelection* selection);
	CMessageResult notify (CBaseObject* sender, const char* message);
	void showOptionsMenu (const CPoint& where);
	void createNewSubview (const CPoint& where, const char* viewName);
	void insertTemplate (const CPoint& where, const char* templateName);
	void invalidSelection ();
	void deleteSelectedViews ();
	void performUndo ();
	void performRedo ();
	bool canUndo ();
	bool canRedo ();
	const char* getUndoName ();
	const char* getRedoName ();
	void emptyUndoStack ();

	void startDrag (CPoint& where);

	// overwrites
	void draw (CDrawContext *pContext);
	void drawRect (CDrawContext *pContext, const CRect& updateRect);
	
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
	UIDescription* uiDescription;
	ViewHierarchyBrowserWindow* hierarchyBrowser;
	CViewInspector* inspector;
	IActionOperation* moveSizeOperation;
	CView* highlightView;
	EditMode editMode;
	MouseEditMode mouseEditMode;
	bool showLines;
	
	CPoint mouseStartPoint;
	CPoint dragSelectionOffset;
	
	std::string templateName;

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

END_NAMESPACE_VSTGUI

#endif // VSTGUI_LIVE_EDITING

#endif
