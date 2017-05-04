// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "platform_helper.h"

namespace VSTGUI {
namespace UnitTest {

SharedPointer<PlatformParentHandle> PlatformParentHandle::create ()
{
	return nullptr;
}

PlatformType PlatformParentHandle::getType () const
{
	return PlatformType::kX11EmbedWindowID;
}

void* PlatformParentHandle::getHandle () const
{
	return nullptr;
}

void PlatformParentHandle::forceRedraw ()
{

}

} // UnitTest
} // VSTGUI

