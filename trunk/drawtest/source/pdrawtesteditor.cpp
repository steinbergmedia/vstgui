//-------------------------------------------------------------------------------------------------------
//  VSTGUI Test View plugin
//  Copyright (c) 2004 Arne Scheffler. All rights reserved.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//-------------------------------------------------------------------------------------------------------

#include "pdrawtesteditor.h"
#include "pdrawtestview.h"
#include "cscrollview.h"
#include "ctabview.h"
#include "controlsgui.h"

enum {
	kBackgroundBitmap	= 1,
	kTabButtonBitmap	= 100,
	kTestBitmap			= 1000,
};

class MyTabView : public CTabView
{
public:
	MyTabView (const CRect& size, CFrame* parent, CBitmap* tabBitmap, CBitmap* background = 0, long tabPosition = kPositionTop, DrawTestEditor* editor = 0)
	: CTabView (size, parent, tabBitmap, background, tabPosition)
	, editor (editor) {}

	virtual void mouse (CDrawContext *pContext, CPoint &where, long button = -1)
	{
		if (button == kRButton)
		{
			CView* view = getViewAt (where);
			if (!view || view->isTypeOf ("CTabButton"))
			{
				CRect r;
				localToFrame (where);
				r.offset (where.x, where.y);
				r.offset (-size.left, -size.top);
				COptionMenu* menu = new COptionMenu (r, NULL, 0);
				menu->addEntry ("Tabs Left");
				menu->addEntry ("Tabs Right");
				menu->addEntry ("Tabs Top");
				menu->addEntry ("Tabs Bottom");
				getFrame ()->addView (menu);
				menu->takeFocus ();
				long res = menu->getLastResult ();
				getFrame ()->removeView (menu);
				if (res != -1)
				{
					r = size;
					editor->setTabView (getFrame (), r, res);
				}
				return;
			}
		}
		CTabView::mouse (pContext, where, button);
	}

protected:
	DrawTestEditor* editor;
};

DrawTestEditor::DrawTestEditor (AudioEffectX* effect)
: AEffGUIEditor (effect)
{
	backgroundBitmap = new CBitmap (kBackgroundBitmap);
	// setup size of editor
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = backgroundBitmap->getWidth ();
	rect.bottom = backgroundBitmap->getHeight ();
}

DrawTestEditor::~DrawTestEditor ()
{
	backgroundBitmap->forget ();
}

void DrawTestEditor::valueChanged (CDrawContext *pContext, CControl *pControl)
{
}

void DrawTestEditor::setTabView (CFrame* frame, const CRect& r, long position)
{
	frame->removeAll ();
	CBitmap* tabButtonBitmap = new CBitmap (kTabButtonBitmap);
	CTabView* tabView = new MyTabView (r, frame, tabButtonBitmap, NULL, position, this);
	tabView->setTransparency (true);
	frame->addView (tabView);
	CRect tabSize = tabView->getTabViewSize (tabSize);
	// add tabs
	CView* testView;
	CBitmap* testBitmap = new CBitmap (kTestBitmap);
	CRect containerSize;
	containerSize.right = testBitmap->getWidth ();
	containerSize.bottom = testBitmap->getHeight ();
	CScrollView* scrollview = new CScrollView (tabSize, containerSize, frame, CScrollView::kHorizontalScrollbar|CScrollView::kVerticalScrollbar);
	CPoint p (0,0);
	testView = new CMovieBitmap (containerSize, NULL, 0, 1, testBitmap->getHeight (), testBitmap, p);
	testBitmap->forget ();
	scrollview->addView (testView);
	tabView->addTab (scrollview, "Scroll View");

	testView = new CDrawTestView (tabSize);
	tabView->addTab (testView, "Primitives");
	
	CRect controlsGUISize (0, 0, 420, 210);
	controlsGUISize.offset (5, 5);
	testView = new ControlsGUI (controlsGUISize, frame);

	CViewContainer* controlContainer = new CViewContainer (tabSize, frame);
	controlContainer->setTransparency (true);
	controlContainer->setMode (CViewContainer::kOnlyDirtyUpdate);
	controlContainer->addView (testView);
	
	tabView->addTab (controlContainer, "Controls");


	tabButtonBitmap->forget ();
	frame->setDirty ();
}

long DrawTestEditor::open (void *ptr)
{
	AEffGUIEditor::open (ptr);
	CRect size (rect.left , rect.top, rect.right, rect.bottom);
	CFrame* frame = new CFrame (size, ptr, this);
	frame->setBackground (backgroundBitmap);
	size.inset (8, 8);
	setTabView (frame, size, CTabView::kPositionBottom);
	// last but not least set the class variable frame to our newly created frame
	this->frame = frame;
	return true;
}

void DrawTestEditor::close ()
{
	// don't forget to remove the frame !!
	if (frame)
		delete frame;
	frame = 0;
}

