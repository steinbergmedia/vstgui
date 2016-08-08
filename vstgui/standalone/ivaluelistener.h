#pragma once

#include "ivalue.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
/** %Value listener interface
 *
 *	@ingroup standalone
 */
class IValueListener : public Interface
{
public:
	virtual void onBeginEdit (IValue& value) = 0;
	virtual void onPerformEdit (IValue& value, IValue::Type newValue) = 0;
	virtual void onEndEdit (IValue& value) = 0;
	virtual void onStateChange (IValue& value) = 0;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
