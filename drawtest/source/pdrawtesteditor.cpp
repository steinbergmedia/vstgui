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

DrawTestEditor::DrawTestEditor (AudioEffectX* effect)
: AEffGUIEditor (effect)
{
	// setup size of editor
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = 400;
	rect.bottom = 400;
}

DrawTestEditor::~DrawTestEditor ()
{
}

long DrawTestEditor::open (void *ptr)
{
	AEffGUIEditor::open (ptr);
	CRect size (rect.left , rect.top, rect.right, rect.bottom);
	CFrame* frame = new CFrame (size, ptr, this);

	size.inset (5, 5);
	CDrawTestView* view = new CDrawTestView (size);
	frame->addView (view);

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

