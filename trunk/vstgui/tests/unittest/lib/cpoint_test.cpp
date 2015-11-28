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

#include "../../../lib/cpoint.h"
#include "../unittests.h"

namespace VSTGUI {

TESTCASE(CPointTest,

	TEST(unequal,
		EXPECT(CPoint (0, 0) != CPoint (1, 1));
		EXPECT(CPoint (0, 0) != CPoint (0, 1));
		EXPECT(CPoint (0, 0) != CPoint (1, 0));
	);

	TEST(equal,
		EXPECT(CPoint (0, 0) == CPoint (0, 0));
	);

	TEST(operatorAddAssign,
		CPoint p (1, 1);
		p += CPoint (1, 1);
		EXPECT(p == CPoint (2, 2));
	);

	TEST(operatorSubtractAssign,
		CPoint p (2, 2);
		p -= CPoint (1, 1);
		EXPECT(p == CPoint (1, 1));
	);

	TEST(operatorAdd,
		CPoint p (2, 2);
		auto p2 = p + CPoint (1, 1);
		EXPECT(p2 == CPoint (3, 3));
		EXPECT(p == CPoint (2, 2));
	);

	TEST(operatorSubtract,
		CPoint p (2, 2);
		auto p2 = p - CPoint (1, 1);
		EXPECT(p2 == CPoint (1, 1));
		EXPECT(p == CPoint (2, 2));
	);

	TEST(operatorInverse,
		CPoint p (2, 2);
		auto p2 = -p;
		EXPECT(p2 == CPoint (-2, -2));
		EXPECT(p == CPoint (2, 2));
	);

	TEST(offsetCoords,
		CPoint p (1, 2);
		p.offset (1, 2);
		EXPECT(p == CPoint (2, 4));
	);
	TEST(offsetPoint,
		CPoint p (1, 2);
		p.offset (CPoint (2, 3));
		EXPECT(p == CPoint (3, 5));
	);

	TEST(offsetInverse,
		CPoint p (5, 3);
		p.offsetInverse (CPoint(2, 1));
		EXPECT(p == CPoint (3, 2));
	);

	TEST(makeIntegral,
		CPoint p (5.3, 4.2);
		p.makeIntegral ();
		EXPECT(p == CPoint (5, 4));
		p (5.5, 4.5);
		p.makeIntegral ();
		EXPECT(p == CPoint (6, 5));
		p (5.9, 4.1);
		p.makeIntegral ();
		EXPECT(p == CPoint (6, 4));
		p (5.1, 4.501);
		p.makeIntegral ();
		EXPECT(p == CPoint (5, 5));
	);
);

} // VSTGUI
