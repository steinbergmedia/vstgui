// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../../lib/controls/ccontrol.h"
#include "../../unittests.h"

namespace VSTGUI {

namespace {

class Control : public CControl
{
public:
	Control () : CControl (CRect (0, 0, 10, 10)) {}
	void draw (CDrawContext* pContext) override {}

	static int32_t mapVstKeyModifier (int32_t vstModifier)
	{ return CControl::mapVstKeyModifier(vstModifier); }

	CLASS_METHODS(Control, CControl)
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

TESTCASE(CControlTest,

	TEST(editing,
		Control c;
		c.beginEdit ();
		EXPECT (c.isEditing ());
		c.setValue (0.5f);
		c.endEdit ();
		EXPECT (c.isEditing () == false);
	);

	TEST(listener,
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
	);

	TEST(subListener,
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
	);
	
	TEST(setValueOutOfRange,
		Control c;
		EXPECT(c.getMin () == 0.f);
		EXPECT(c.getMax () == 1.f);
		c.setValue (0.5f);
		EXPECT (c.getValue () == 0.5f);
		c.setValue (-0.5f);
		EXPECT (c.getValue () == 0.f);
		c.setValue (1.5f);
		EXPECT (c.getValue () == 1.f);
	);
	
	TEST(setValueNormalized,
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
	);
	
	TEST(checkDefaultValue,
		CButtonState buttons = kLButton | CControl::kDefaultValueModifier;
		Control c;
		c.setValue (c.getDefaultValue () + 0.1f);
		EXPECT (c.checkDefaultValue (buttons));
		EXPECT (c.getValue () == c.getDefaultValue ());
		EXPECT (c.checkDefaultValue (kRButton) == false);
	);

	TEST(mapVstKeyModifier,
		EXPECT (Control::mapVstKeyModifier (MODIFIER_SHIFT) == kShift);
		EXPECT (Control::mapVstKeyModifier (MODIFIER_ALTERNATE) == kAlt);
		EXPECT (Control::mapVstKeyModifier (MODIFIER_COMMAND) == kApple);
		EXPECT (Control::mapVstKeyModifier (MODIFIER_CONTROL) == kControl);
	);

);

} // VSTGUI
