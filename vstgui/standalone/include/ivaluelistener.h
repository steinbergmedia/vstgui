// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
	/** %Value begins editing. */
	virtual void onBeginEdit (IValue& value) = 0;
	/** %Value performed an edit. */
	virtual void onPerformEdit (IValue& value, IValue::Type newValue) = 0;
	/** %Value ends editing. */
	virtual void onEndEdit (IValue& value) = 0;
	/** %Value changed some of its state. */
	virtual void onStateChange (IValue& value) = 0;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
