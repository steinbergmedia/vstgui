#pragma once

#include "ivalue.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
class IValueListener : public Interface
{
public:
	virtual void onBeginEdit (const IValue& value) = 0;
	virtual void onPerformEdit (const IValue& value, IValue::Type newValue) = 0;
	virtual void onEndEdit (const IValue& value) = 0;
	virtual void onStateChange (const IValue& value) = 0;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
