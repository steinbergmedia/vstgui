#include "platform_helper.h"

namespace VSTGUI {
namespace UnitTest {

SharedPointer<PlatformParentHandle> PlatformParentHandle::create ()
{
	return nullptr;
}

PlatformType PlatformParentHandle::getType () const
{
	return PlatformType::kX11Parent;
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

