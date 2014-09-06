//
//  itimingfunction.h
//  vstgui
//
//  Created by Arne Scheffler on 06/08/14.
//
//

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
	virtual ~ITimingFunction () {}

	virtual float getPosition (uint32_t milliseconds) = 0;
	virtual bool isDone (uint32_t milliseconds) = 0;
};

}} // namespaces

#endif // __itimingfunction__
