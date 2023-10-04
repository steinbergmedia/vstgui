## UI Description Scripting

### Introduction

There is simple scripting support added to the uidescription editor when the cmake 
option `VSTGUI_UISCRIPTING` is turned on.
This scripting language is based on JavaScript via the TinyJS library and supports 
variables, arrays, structures and objects with inheritance (not fully implemented).

### Getting started

You need to add the scripting library to your target. With cmake this is done by adding
`target_link_libraries(${target} PRIVATE vstgui_uiscripting)` to your CMakeLists.txt.

The library needs to be initialized once at runtime before any UIDescription object is created. 
This is done with a call to `VSTGUI::UIScripting::init ();`. You need to include 
`"vstgui/uiscripting/uiscripting.h"` for this symbol.

All scripts of one UIDescription object will be executed in the same JavaScript context. So you
can access global variables from all your scripts if needed.

### The view script

A script runs when a view is created and has a script assigned to its `script` attribute.
The script attribute either contains the script text or a reference to a file inside
the resource directory of the plugin/application containing the script text.

A reference to a script file is done via 

	//scriptfile $SCRIPT_FILENAME

The script is executed directly after the view was created and before its other attributes are set and before child views are added.

### The **view** variable

The script contains a view variable which among other things can be used to add listeners on actions on the view.
To install a listener when the mouse enters the view do:

	view.onMouseEnter = function (view) {
	  // do something when the mouse enters this view...
	};

See "**View Listeners**" which listeners can be installed on views.
And see "**View methods and properties**" for the other methods a view supports.

If the view is a control it additionally supports the methods described in "**Control methods and properties**"
and the listeners as described in "**Control listeners**".

If the view is a view container then it supports the additional listeners described in "**View container listeners**".

### The **uiDesc** variable

The script also contains the uiDesc variable you can use to query information on the uiDesc file.
The following methods are implemented on that object:
- `colorNames() -> Array`
- `fontNames() -> Array`
- `bitmapNames() -> Array`
- `gradientNames() -> Array`
- `controlTagNames() -> Array`
- `getTagForName(String: name) -> Integer`
- `lookupTagName(Integer: tag) -> String`


### View methods and properties

- `type -> String`
	- contains the type of the view like "CAnimKnob"
- `isTypeOf(String: name); -> Integer`
	- returns a boolean if the view is of type "name"
- `getParent() -> ViewObject`
	- returns the parent view or undefined if it has none
- `getAttribute(String: key) -> String`
	- get the attribute with name "key"
- `setAttribute(String: key, String: value) -> Void`
	- set the attribute value with name "key" to "value"

For example to set the opacity attribute of a view write:

	view.setAttribute("opacity", 0.5);

You can set any attribute as shown in the WYSIWYG editor.

### View listeners

- `onAttached(view)`
- `onRemoved(view)`
- `onSizeChanged(view, newSize)`
- `onLostFocus(view)`
- `onTookFocus(view)`
- `onMouseEnabled(view, state)`
- `onMouseEnter(view, event)`
- `onMouseExit(view, event)`
- `onMouseDown(view, event)`
- `onMouseUp(view, event)`
- `onMouseMove(view, event)`
- `onMouseWheel(view, event)`
- `onKeyDown(view, event)`
- `onKeyUp(view, event)`

### Control methods and properties

- `setValue(Number: value) -> Void`
- `getValue() -> Number`
- `setValueNormalized(Number: normValue) -> Void`
- `getValueNormalized() -> Number`
- `beginEdit() -> Void`
- `endEdit() -> Void`
- `getMinValue() -> Number`
- `getMaxValue() -> Number`
- `getTag() -> Number`

### Control listeners
- `onValueChanged(view, value)`
- `onBeginEdit(view)`
- `onEndEdit(view)`

### View container listeners
- `onViewAdded(view, child)`
- `onViewRemoved(view, child)`

### Global functions

- `createTimer(context, fireTime, callback) -> TimerObject`
	- creates a new stopped timer object
	- fire time is in milliseconds
	- the context will be provided on every timer callback
	- the callback has the signature `function(context)`
	- a timer object has the methods:
		- start()
		- stop()
		- invalid()
- `iterateSubViews(view, context, callback) -> Void`
	- calls the callback with context as parameter for every child view of view
- `log(obj) -> Void`
	- logs the object to the debug console

### Other built in functions (from the TinyJS library)

- `exec(jsCode)`
- `eval(jsCode)`
- `trace()`
- `charToInt(ch)`
- `Object.dump()`
- `Object.clone()`
- `String.indexOf(search)`
- `String.substring(lo,hi)`
- `String.charAt(pos)`
- `String.charCodeAt(pos)`
- `String.fromCharCode(cha`
- `String.split(separator)`
- `Integer.parseInt(str)`
- `Integer.valueOf(str)`
- `JSON.stringify(obj)`
- `Array.contains(obj)`
- `Array.remove(obj)`
- `Array.join(separator)`
- `Math.rand()`
- `Math.randInt(min, max)`
- `Math.abs(a)`
- `Math.round(a)`
- `Math.min(a,b)`
- `Math.max(a,b)`
- `Math.range(x,a,b)`
- `Math.sign(a)`
- `Math.PI()`
- `Math.toDegrees(a)`
- `Math.toRadians(a)`
- `Math.sin(a)`
- `Math.asin(a)`
- `Math.cos(a)`
- `Math.acos(a)`
- `Math.tan(a)`
- `Math.atan(a)`
- `Math.sinh(a)`
- `Math.asinh(a)`
- `Math.cosh(a)`
- `Math.acosh(a)`
- `Math.tanh(a)`
- `Math.atanh(a)`
- `Math.E()`
- `Math.log(a)`
- `Math.log10(a)`
- `Math.exp(a)`
- `Math.pow(a,b)`
- `Math.sqr(a)`
- `Math.sqrt(a)`
