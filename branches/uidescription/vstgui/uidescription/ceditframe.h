/*
 *  ceditframe.h
 *
 *  Created by Arne Scheffler on 12/8/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __ceditframe__
#define __ceditframe__

#include "../vstgui.h"
#include "../cvstguitimer.h"
#include "cselection.h"

BEGIN_NAMESPACE_VSTGUI

class CrossLines;
class Grid;

//----------------------------------------------------------------------------------------------------
class CEditFrame : public CFrame
//----------------------------------------------------------------------------------------------------
{
public:
	enum EditMode {
		kNoEditMode,
		kEditMode,
		kPaletteMode
	};

	CEditFrame (const CRect& size, void* windowPtr, VSTGUIEditorInterface* editor, EditMode editMode, CSelection* selection);
	~CEditFrame ();

	EditMode getEditMode () const { return editMode; }
	void setEditMode (EditMode mode) { editMode = mode; invalid ();}

	void setGrid (int size);
	int getGrid () const;

	void startDrag (CPoint& where);

	// overwrites
	void draw (CDrawContext *pContext);
	void drawRect (CDrawContext *pContext, const CRect& updateRect);
	
	CMouseEventResult onMouseDown (CPoint &where, const long& buttons);
	CMouseEventResult onMouseUp (CPoint &where, const long& buttons);
	CMouseEventResult onMouseMoved (CPoint &where, const long& buttons);

	bool onDrop (CDragContainer* drag, const CPoint& where);
	void onDragEnter (CDragContainer* drag, const CPoint& where);
	void onDragLeave (CDragContainer* drag, const CPoint& where);
	void onDragMove (CDragContainer* drag, const CPoint& where);

	void invalidRect (const CRect rect);

	long onKeyDown (VstKeyCode& keyCode);
	long onKeyUp (VstKeyCode& keyCode);
	
//----------------------------------------------------------------------------------------------------
protected:
	CBitmap* createBitmapFromSelection (CSelection* selection);
	CMessageResult notify (CBaseObject* sender, const char* message);

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
	CView* highlightView;
	EditMode editMode;
	MouseEditMode mouseEditMode;
	bool showLines;
	
	CPoint mouseStartPoint;
	CPoint dragSelectionOffset;

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

#endif
