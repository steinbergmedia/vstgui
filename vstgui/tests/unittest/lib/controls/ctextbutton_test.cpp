// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cbuttons.h"
#include "../../unittests.h"

namespace VSTGUI {

TESTCASE(CCheckboxTest,

	TEST(mouseEventsKickStyle,
		auto b = owned (new CTextButton (CRect (10, 10, 50, 20)));
		b->setStyle (CTextButton::kKickStyle);
		b->setValue (b->getMin ());
		CPoint p (10, 10);
		EXPECT (b->onMouseDown (p, kRButton) == kMouseEventNotHandled);
		EXPECT (b->isEditing () == false);
		EXPECT (b->onMouseDown (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing ());
		EXPECT (b->getValue () == b->getMax ());
		EXPECT (b->onMouseMoved (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing ());
		EXPECT (b->getValue () == b->getMax ());
		EXPECT (b->onMouseUp (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing () == false);
		EXPECT (b->getValue () == b->getMin ());
		
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

		EXPECT (b->onMouseMoved (p, kLButton) == kMouseEventNotHandled);
	);

	TEST(mouseEventsOnOffStyle,
		auto b = owned (new CTextButton (CRect (10, 10, 50, 20)));
		b->setStyle (CTextButton::kOnOffStyle);
		b->setValue (b->getMin ());
		CPoint p (10, 10);
		EXPECT (b->onMouseDown (p, kRButton) == kMouseEventNotHandled);
		EXPECT (b->isEditing () == false);
		EXPECT (b->onMouseDown (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing ());
		EXPECT (b->getValue () == b->getMax ());
		EXPECT (b->onMouseMoved (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing ());
		EXPECT (b->getValue () == b->getMax ());
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
		EXPECT (b->onMouseMoved (p, kLButton) == kMouseEventHandled);
		EXPECT (b->getValue () == b->getMin ());
		EXPECT (b->onMouseUp (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing () == false);
		EXPECT (b->getValue () == b->getMin ());

		p (10, 10);
		EXPECT (b->onMouseDown (p, kLButton) == kMouseEventHandled);
		EXPECT (b->isEditing () == true);
		EXPECT (b->onMouseCancel() == kMouseEventHandled);
		EXPECT (b->isEditing () == false);

		EXPECT (b->onMouseMoved (p, kLButton) == kMouseEventNotHandled);
	);

	TEST(keyEvents,
		auto b = owned (new CTextButton (CRect (10, 10, 50, 20)));
		b->setStyle (CTextButton::kOnOffStyle);
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

		b->setStyle (CTextButton::kKickStyle);
		keyCode.character = 0;
		keyCode.virt = VKEY_RETURN;
		EXPECT (b->onKeyDown (keyCode) == 1);
		EXPECT (b->getValue () == b->getMin ());
		EXPECT (b->onKeyDown (keyCode) == 1);
		EXPECT (b->getValue () == b->getMin ());


		EXPECT (b->onKeyUp (keyCode) == -1);
	);
	
	TEST(focusPathSetting,
		auto b = owned (new CTextButton (CRect (10, 10, 50, 20)));
		EXPECT (b->drawFocusOnTop () == false);
	);
);

} // VSTGUI
