// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "fwd.h"
#include "../../lib/cstring.h"
#include "interface.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
/** Value interface
 *
 *	@ingroup standalone
 */
class IValue : public Interface
{
public:
	/** floating point value in the range of 0 to 1 */
	using Type = double;
	/** indicates an invalid value */
	static constexpr Type InvalidValue = std::numeric_limits<Type>::min ();

	/** Begin editing the value. */
	virtual void beginEdit () = 0;
	/** Perform a value edit. */
	virtual bool performEdit (Type newValue) = 0;
	/** End editing the value. */
	virtual void endEdit () = 0;

	/** Set active state. */
	virtual void setActive (bool state) = 0;
	/** Is value active? */
	virtual bool isActive () const = 0;

	/** Get the normalized value. */
	virtual Type getValue () const = 0;
	/** Is value in edit mode. */
	virtual bool isEditing () const = 0;

	/** Get value identifier. */
	virtual const UTF8String& getID () const = 0;

	/** Get value converter. */
	virtual const IValueConverter& getConverter () const = 0;

	/** register a value listener. */
	virtual void registerListener (IValueListener* listener) = 0;
	/** unregister a value listener. */
	virtual void unregisterListener (IValueListener* listener) = 0;
};

//------------------------------------------------------------------------
/** extension to IValue for a non continous value with discrete steps
 *
 *	@ingroup standalone
 */
class IStepValue : public Interface
{
public:
	using StepType = uint32_t;
	static constexpr StepType InvalidStep = std::numeric_limits<uint32_t>::max ();

	/** Get number of steps. */
	virtual StepType getSteps () const = 0;
	/** Convert step to normalized value. */
	virtual IValue::Type stepToValue (StepType step) const = 0;
	/** Convert normalized value to step. */
	virtual StepType valueToStep (IValue::Type) const = 0;
};

//------------------------------------------------------------------------
/** %Value converter interface
 *
 *	@ingroup standalone
 */
class IValueConverter : public Interface
{
public:
	/** Convert value to string. */
	virtual UTF8String valueAsString (IValue::Type value) const = 0;
	/** Convert string to value. */
	virtual IValue::Type stringAsValue (const UTF8String& string) const = 0;

	/** Convert plain to normalized value. */
	virtual IValue::Type plainToNormalized (IValue::Type plain) const = 0;
	/** Convert normalized to plain value. */
	virtual IValue::Type normalizedToPlain (IValue::Type normalized) const = 0;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
