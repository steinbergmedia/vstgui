// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "events.h"
#include "platform/platformfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace EventPrivate {
static uint64_t counter = 0;
} // EventPrivate

//------------------------------------------------------------------------
Event::Event () noexcept
: id (++EventPrivate::counter), timestamp (getPlatformFactory ().getTicks ())
{
}

//------------------------------------------------------------------------
} // VSTGUI
