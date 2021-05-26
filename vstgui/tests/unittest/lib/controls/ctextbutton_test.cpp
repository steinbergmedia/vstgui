// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cbuttons.h"
#include "../../../../lib/events.h"
#include "../../unittests.h"

namespace VSTGUI {

TEST_CASE (CTextButtonTest, MouseEventsKickStyle)
{
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
	EXPECT (b->onMouseCancel () == kMouseEventHandled);
	EXPECT (b->isEditing () == false);

	EXPECT (b->onMouseMoved (p, kLButton) == kMouseEventNotHandled);
}

TEST_CASE (CTextButtonTest, MouseEventsOnOffStyle)
{
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
	EXPECT (b->onMouseCancel () == kMouseEventHandled);
	EXPECT (b->isEditing () == false);

	EXPECT (b->onMouseMoved (p, kLButton) == kMouseEventNotHandled);
}

TEST_CASE (CTextButtonTest, KeyEvents)
{
	auto b = owned (new CTextButton (CRect (10, 10, 50, 20)));
	b->setStyle (CTextButton::kOnOffStyle);
	b->setValue (b->getMin ());
	KeyboardEvent retEvent;
	retEvent.virt = VirtualKey::Return;
	b->onKeyboardEvent (retEvent);
	EXPECT_TRUE (retEvent.consumed);
	EXPECT (b->getValue () == b->getMax ());
	retEvent.consumed.reset();
	b->onKeyboardEvent (retEvent);
	EXPECT_TRUE (retEvent.consumed);
	EXPECT (b->getValue () == b->getMin ());

	KeyboardEvent charEvent;
	charEvent.character = 't';
	b->onKeyboardEvent (charEvent);
	EXPECT_FALSE (charEvent.consumed);

	b->setStyle (CTextButton::kKickStyle);
	retEvent.consumed.reset();
	b->onKeyboardEvent (retEvent);
	EXPECT_TRUE (retEvent.consumed);
	EXPECT (b->getValue () == b->getMin ());
	retEvent.consumed.reset();
	b->onKeyboardEvent (retEvent);
	EXPECT_TRUE (retEvent.consumed);
	EXPECT (b->getValue () == b->getMin ());

	KeyboardEvent upEvent;
	upEvent.type = EventType::KeyUp;
	upEvent.virt = VirtualKey::Return;
	b->onKeyboardEvent (upEvent);
	EXPECT_FALSE (upEvent.consumed);
}

TEST_CASE (CTextButtonTest, FocusPathSetting)
{
	auto b = owned (new CTextButton (CRect (10, 10, 50, 20)));
	EXPECT (b->drawFocusOnTop () == false);
}

} // VSTGUI
