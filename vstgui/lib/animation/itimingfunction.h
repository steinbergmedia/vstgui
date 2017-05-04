// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __itimingfunction__
#define __itimingfunction__

#include "../vstguibase.h"

namespace VSTGUI {
namespace Animation {

//-----------------------------------------------------------------------------
/// @brief Animation timing function interface
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class ITimingFunction
{
public:
	virtual ~ITimingFunction () noexcept = default;

	virtual float getPosition (uint32_t milliseconds) = 0;
	virtual bool isDone (uint32_t milliseconds) = 0;
};

}} // namespaces

#endif // __itimingfunction__
