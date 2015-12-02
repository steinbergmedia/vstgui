//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

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
