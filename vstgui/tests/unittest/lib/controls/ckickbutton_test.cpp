// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cbuttons.h"
#include "../../unittests.h"

namespace VSTGUI {

TESTCASE(CKickButtonTest,

	TEST(mouseEvents,
		auto b = owned (new CKickButton (CRect (10, 10, 50, 20), nullptr, 0, nullptr));
		b->setValue (b->getMin());
		CPoint p (10, 10);
		EXPECT (b->onMouseDown (p, kRButton) == kMouseEventNotHandled);
		EXPECT (b->onMouseMoved (p, kLButton) == kMouseEventNotHandled);
		EXPECT (b->isEditing () == false);
		EXPECT (b->onMouseDown (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing ());
		EXPECT (b->getValue () == b->getMax ());
		p (0, 0);
		EXPECT (b->onMouseMoved (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing ());
		EXPECT (b->getValue () == b->getMin ());
		p (10, 10);
		EXPECT (b->onMouseMoved (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing ());
		EXPECT (b->getValue () == b->getMax ());
		EXPECT (b->onMouseUp (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing () == false);
		EXPECT (b->getValue () == b->getMin ());
		
		p (10, 10);
		EXPECT (b->onMouseDown (p, kLButton) == kMouseEventHandled);
		EXPECT (b->getValue () == b->getMax ());
		EXPECT (b->isEditing () == true);
		EXPECT (b->onMouseCancel () == kMouseEventHandled);
		EXPECT (b->isEditing () == false);
		EXPECT (b->getValue () == b->getMin ());
	);

	TEST(keyEvents,
		auto b = owned (new CKickButton (CRect (10, 10, 50, 20), nullptr, 0, nullptr));
		b->setValue (b->getMin());
		VstKeyCode keyCode {};
		keyCode.virt = VKEY_RETURN;
		EXPECT (b->onKeyDown (keyCode) == 1);
		EXPECT (b->getValue () == b->getMax ());
		EXPECT (b->onKeyDown (keyCode) == 1);
		EXPECT (b->getValue () == b->getMax ());
		EXPECT (b->onKeyUp (keyCode) == 1);
		EXPECT (b->getValue () == b->getMin ());
		
		keyCode.virt = 0;
		keyCode.character = 't';
		EXPECT (b->onKeyDown (keyCode) == -1);
		EXPECT (b->onKeyUp (keyCode) == -1);
	);
	
);

} // VSTGUI
