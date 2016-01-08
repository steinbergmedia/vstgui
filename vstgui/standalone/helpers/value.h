#pragma once

#include "../ivalue.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Value {

//------------------------------------------------------------------------
ValuePtr make (const UTF8String& id, IValue::Type initialValue = 0.,
               const ValueStringConverterPtr& stringConverter = nullptr);

//------------------------------------------------------------------------
ValuePtr makeStepValue (const UTF8String& id, IStepValue::StepType initialSteps,
                        IValue::Type initialValue,
                        const ValueStringConverterPtr& stringConverter = nullptr);

//------------------------------------------------------------------------
ValuePtr makeStringListValue (const UTF8String& id,
                              const std::initializer_list<UTF8String>& strings);

//------------------------------------------------------------------------
} // Value
} // Standalone
} // VSTGUI
