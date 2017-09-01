// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/vstguibase.h"
#include "../../../lib/platform/iplatformframe.h"

namespace VSTGUI {
namespace UnitTest {

struct PlatformParentHandle : public CBaseObject
{
	static SharedPointer<PlatformParentHandle> create ();
	
	virtual PlatformType getType () const = 0;
	virtual void* getHandle () const = 0;
	virtual void forceRedraw () = 0;
};

} // UnitTest
} // VSTGUI

