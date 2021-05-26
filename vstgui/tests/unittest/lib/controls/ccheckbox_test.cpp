// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/cbuttons.h"
#include "../../../../lib/events.h"
#include "../../unittests.h"

namespace VSTGUI {

TEST_CASE (CCheckboxTest, MouseEvents)
{
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
	EXPECT (b->onMouseCancel () == kMouseEventHandled);
	EXPECT (b->isEditing () == false);
}

TEST_CASE (CCheckboxTest, KeyEvents)
{
	auto b = owned (new CCheckBox (CRect (10, 10, 50, 20)));
	b->setValue (b->getMin ());
	KeyboardEvent event;
	event.virt = VirtualKey::Return;
	b->onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b->getValue () == b->getMax ());
	event.consumed.reset ();
	b->onKeyboardEvent (event);
	EXPECT_TRUE (event.consumed);
	EXPECT (b->getValue () == b->getMin ());

	event.virt = VirtualKey::None;
	event.character = 't';
	event.consumed.reset ();
	b->onKeyboardEvent (event);
	EXPECT_FALSE (event.consumed);
}

} // VSTGUI
