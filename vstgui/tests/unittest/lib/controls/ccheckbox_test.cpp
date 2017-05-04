// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cbuttons.h"
#include "../../unittests.h"

namespace VSTGUI {

TESTCASE(CCheckboxTest,

	TEST(mouseEvents,
		auto b = owned (new CCheckBox (CRect (10, 10, 50, 20)));
		b->setValue (b->getMin ());
		CPoint p (10, 10);
		EXPECT (b->onMouseDown (p, kRButton) == kMouseDownEventHandledButDontNeedMovedOrUpEvents);
		EXPECT (b->isEditing () == false);
		EXPECT (b->onMouseDown (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing ());
		EXPECT (b->onMouseMoved (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing ());
		EXPECT (b->onMouseUp (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing () == false);
		EXPECT (b->getValue () == b->getMax ());
		
		EXPECT (b->onMouseDown (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing ());
		EXPECT (b->onMouseUp (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing () == false);
		EXPECT (b->getValue () == b->getMin ());

		EXPECT (b->onMouseDown (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing ());
		p (0, 0);
		EXPECT (b->onMouseUp (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing () == false);
		EXPECT (b->getValue () == b->getMin ());

		p (10, 10);
		EXPECT (b->onMouseDown (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing () == true);
		EXPECT (b->onMouseCancel() == kMouseEventHandled);
		EXPECT (b->isEditing () == false);
	);

	TEST(keyEvents,
		auto b = owned (new CCheckBox (CRect (10, 10, 50, 20)));
		b->setValue (b->getMin());
		VstKeyCode keyCode {};
		keyCode.virt = VKEY_RETURN;
		EXPECT (b->onKeyDown (keyCode) == 1);
		EXPECT (b->getValue () == b->getMax ());
		EXPECT (b->onKeyDown (keyCode) == 1);
		EXPECT (b->getValue () == b->getMin ());
		
		keyCode.virt = 0;
		keyCode.character = 't';
		EXPECT (b->onKeyDown (keyCode) == -1);
	);
	
);

} // VSTGUI
