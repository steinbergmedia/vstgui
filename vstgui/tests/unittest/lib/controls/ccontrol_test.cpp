// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/ccontrol.h"
#include "../../../../lib/controls/icontrollistener.h"
#include "../../unittests.h"
#include "../eventhelpers.h"

namespace VSTGUI {

namespace {

class Control : public CControl
{
public:
	Control () : CControl (CRect (0, 0, 10, 10)) {}
	void draw (CDrawContext* pContext) override {}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	static int32_t mapVstKeyModifier (int32_t vstModifier)
	{
#include "../../../../lib/private/disabledeprecatedmessage.h"
		return CControl::mapVstKeyModifier (vstModifier);
#include "../../../../lib/private/enabledeprecatedmessage.h"
	}
#endif

	CLASS_METHODS (Control, CControl)
};

struct Listener : IControlListener
{
	bool valueChangedCalled {false};
	bool beginEditCalled {false};
	bool endEditCalled {false};

	void valueChanged (CControl* pControl) override { valueChangedCalled = true; }
	void controlBeginEdit (CControl* pControl) override { beginEditCalled = true; }
	void controlEndEdit (CControl* pControl) override { endEditCalled = true; }
};
}

TEST_CASE (CControlTest, Editing)
{
	Control c;
	c.beginEdit ();
	EXPECT (c.isEditing ());
	c.setValue (0.5f);
	c.endEdit ();
	EXPECT (c.isEditing () == false);
}

TEST_CASE (CControlTest, Listener)
{
	Control c;
	Listener l;
	c.setListener (&l);
	c.beginEdit ();
	EXPECT (l.beginEditCalled);
	c.setValue (0.5f);
	c.valueChanged ();
	EXPECT (l.valueChangedCalled);
	c.endEdit ();
	EXPECT (l.endEditCalled);
}

TEST_CASE (CControlTest, SubListener)
{
	Control c;
	Listener l;
	c.registerControlListener (&l);
	c.beginEdit ();
	EXPECT (l.beginEditCalled);
	c.setValue (0.5f);
	c.valueChanged ();
	EXPECT (l.valueChangedCalled);
	c.endEdit ();
	EXPECT (l.endEditCalled);
	c.unregisterControlListener (&l);
}

TEST_CASE (CControlTest, SetValueOutOfRange)
{
	Control c;
	EXPECT (c.getMin () == 0.f);
	EXPECT (c.getMax () == 1.f);
	c.setValue (0.5f);
	EXPECT (c.getValue () == 0.5f);
	c.setValue (-0.5f);
	EXPECT (c.getValue () == 0.f);
	c.setValue (1.5f);
	EXPECT (c.getValue () == 1.f);
}

TEST_CASE (CControlTest, SetValueNormalized)
{
	Control c;
	c.setMin (1.f);
	c.setMax (2.f);
	c.setValueNormalized (0.f);
	EXPECT (c.getValue () == 1.f);
	c.setValueNormalized (1.f);
	EXPECT (c.getValue () == 2.f);
	c.setValueNormalized (0.5f);
	EXPECT (c.getValue () == 1.5f);

	c.setValueNormalized (-1.f);
	EXPECT (c.getValue () == 1.f);
	c.setValueNormalized (2.f);
	EXPECT (c.getValue () == 2.f);
}

TEST_CASE (CControlTest, CheckDefaultValue)
{
	Control c;
	c.setValue (c.getDefaultValue () + 0.1f);
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (&c, {0., 0.}, MouseButton::Left,
												   Modifiers (ModifierKey::Control)),
			   EventConsumeState::Handled + MouseDownUpMoveEvent::IgnoreFollowUpEventsMask);
	EXPECT (c.getValue () == c.getDefaultValue ());
	c.setValue (c.getDefaultValue () + 0.1f);
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (&c, {0., 0.}, MouseButton::Right,
												   Modifiers (ModifierKey::Control)),
			   0);
	EXPECT (c.getValue () == c.getDefaultValue () + 0.1f);

	auto oldCheckDefaultValueFunc = CControl::CheckDefaultValueEventFunc;
	CControl::CheckDefaultValueEventFunc = [] (CControl*, MouseDownEvent& event) {
		return (event.buttonState.isMiddle () && event.modifiers.is (ModifierKey::Shift));
	};

	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (&c, {0., 0.}, MouseButton::Left,
												   Modifiers (ModifierKey::Control)),
			   0);
	EXPECT_EQ (dispatchMouseEvent<MouseDownEvent> (&c, {0., 0.}, MouseButton::Middle,
												   Modifiers (ModifierKey::Shift)),
			   EventConsumeState::Handled + MouseDownUpMoveEvent::IgnoreFollowUpEventsMask);

	CControl::CheckDefaultValueEventFunc = oldCheckDefaultValueFunc;
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
TEST_CASE (CControlTest, mapVstKeyModifier)
{
	EXPECT (Control::mapVstKeyModifier (MODIFIER_SHIFT) == kShift);
	EXPECT (Control::mapVstKeyModifier (MODIFIER_ALTERNATE) == kAlt);
	EXPECT (Control::mapVstKeyModifier (MODIFIER_COMMAND) == kApple);
	EXPECT (Control::mapVstKeyModifier (MODIFIER_CONTROL) == kControl);
}
#endif

} // VSTGUI
