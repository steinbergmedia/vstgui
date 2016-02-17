#pragma once

#include "../ivalue.h"
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
class IStringListValue : public Interface
{
public:
	using StringList = std::vector<UTF8String>;
	virtual bool updateStringList (const StringList& newStrings) = 0;
};

//------------------------------------------------------------------------
namespace Value {

//------------------------------------------------------------------------
ValuePtr make (const UTF8String& id, IValue::Type initialValue = 0.,
               const ValueStringConverterPtr& stringConverter = nullptr);

//------------------------------------------------------------------------
ValuePtr makeStepValue (const UTF8String& id, IStepValue::StepType numSteps,
                        IValue::Type initialValue = 0.,
                        const ValueStringConverterPtr& stringConverter = nullptr);

//------------------------------------------------------------------------
ValuePtr makeStringListValue (const UTF8String& id,
                              const std::initializer_list<UTF8String>& strings,
                              IValue::Type initialValue = 0.);

//------------------------------------------------------------------------
ValuePtr makeStringListValue (const UTF8String& id, const IStringListValue::StringList& strings);

//------------------------------------------------------------------------
ValueStringConverterPtr makePercentConverter ();
	
//------------------------------------------------------------------------
} // Value

} // Standalone
} // VSTGUI
