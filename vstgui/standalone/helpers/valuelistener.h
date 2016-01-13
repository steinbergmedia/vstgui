#pragma once

#include "../ivaluelistener.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
class ValueListenerAdapter : public IValueListener
{
public:
	void onBeginEdit (const IValue& value) override {}
	void onPerformEdit (const IValue& value, IValue::Type newValue) override {}
	void onEndEdit (const IValue& value) override {}
	void onStateChange (const IValue& value) override {}
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
