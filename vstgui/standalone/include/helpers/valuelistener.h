#pragma once

#include "../ivaluelistener.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
/** %Value listener adapter
 *
 *	@ingroup standalone
 */
class ValueListenerAdapter : public IValueListener
{
public:
	void onBeginEdit (IValue& value) override {}
	void onPerformEdit (IValue& value, IValue::Type newValue) override {}
	void onEndEdit (IValue& value) override {}
	void onStateChange (IValue& value) override {}
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
