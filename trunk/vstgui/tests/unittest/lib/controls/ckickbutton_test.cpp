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
