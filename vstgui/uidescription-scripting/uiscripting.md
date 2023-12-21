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

```js
//scriptfile $SCRIPT_FILENAME
```

The script is executed directly after the view was created and before its other attributes are set and before child views are added.

### The **view** variable

The script contains a view variable which among other things can be used to add listeners on actions on the view.
To install a function that is executed whenever the mouse enters the view do:

```js
view.onMouseEnter = function (view, event) {
  // do something when the mouse enters this view...
};
```

See [**View Listeners**](#view-listeners) which listeners can be installed on views.
And see [**View methods and properties**](#view-methods-and-properties) for the other methods a view supports.

If the view is a control it additionally supports the methods described in [**Control methods and properties**](#control-methods-and-properties)
and the listeners as described in [**Control listeners**](#control-listeners).

If the view is a view container then it supports the additional listeners described in [**View container listeners**](#view-container-listeners).

### The **uiDesc** variable

The script also contains the uiDesc variable you can use to query information on the uiDesc file.
The following methods are implemented on that object:

|type    |name           |arguments    |return type    |comments                                      |
|--------|---------------|-------------|---------------|----------------------------------------------|
|method  |colorNames     |             |`array<string>`|get all color names from the UIDesc file      |
|method  |fontNames      |             |`array<string>`|get all font names from the UIDesc file       |
|method  |bitmapNames    |             |`array<string>`|get all bitmap names from the UIDesc file     |
|method  |gradientNames  |             |`array<string>`|get all gradient names from the UIDesc file   |
|method  |controlTagNames|             |`array<string>`|get all control tag names from the UIDesc file|
|method  |getTagForName  |name:`string`|`integer`      |get the tag for the control tag name          |
|method  |lookupTagName  |tag:`integer`|`string`       |get the control tag name from the tag         |

### Interacting with c++ code

To interact with the c++ code extend the `IController` with `IScriptControllerExtension`
and implement its methods. From the script you can call the view methods `getControllerProperty` or `setControllerProperty`.

A property can either be an integer, floating point, string or undefined.

### View methods and properties

|type    |name                 |arguments                     |return type|comments                                            |
|--------|---------------------|------------------------------|-----------|----------------------------------------------------|
|property|type                 |                              |`string`   |contains the type of the view like `CAnimKnob`      |
|method  |isTypeOf             |name:`string`                 |`integer`  |returns true if the view is of type `name`          |
|method  |getParent            |                              |`object`   |returns the parent view or undefined if it has none |
|method  |getAttribute         |key:`string`                  |`string`   |get the attribute with name `key`                   |
|method  |setAttribute         |key:`string`,value:`string`   |`string`   |set the attribute value with name `key` to `value`  |
|method  |getControllerProperty|name:`string`                 |`property` |set a controller property. returns true if succeeded|
|method  |setControllerProperty|name:`string`,value:`property`|`property` |get a controller property. returns a property       |

For example to set the opacity attribute of a view write:

```js
view.setAttribute("opacity", 0.5);
```

You can set any attribute as shown in the WYSIWYG editor.

### View listeners

|name          |arguments                    |
|--------------|-----------------------------|
|onAttached    |view:`object`                |
|onRemoved     |view:`object`                |
|onSizeChanged |view:`object`,newSize:`rect` |
|onLostFocus   |view:`object`                |
|onTookFocus   |view:`object`                |
|onMouseEnabled|view:`object`,state:`boolean`|
|onMouseEnter  |view:`object`,event:`object` |
|onMouseExit   |view:`object`,event:`object` |
|onMouseDown   |view:`object`,event:`object` |
|onMouseUp     |view:`object`,event:`object` |
|onMouseMove   |view:`object`,event:`object` |
|onMouseWheel  |view:`object`,event:`object` |
|onKeyDown     |view:`object`,event:`object` |
|onKeyUp       |view:`object`,event:`object` |

### Control methods and properties

|type    |name                 |arguments     |return type|comments                                            |
|--------|---------------------|--------------|-----------|----------------------------------------------------|
|method  |setValue             |value:`double`|`void`     | set the control value to `value`                   |
|method  |getValue             |              |`double`   | get the control value                              |
|method  |setValueNormalized   |value:`double`|`void`     | set the control value from the normalized `value`  |
|method  |getValueNormalized   |              |`double`   | get the normalized control value                   |
|method  |beginEdit            |              |`void`     | begin editing the control                          |
|method  |endEdit              |              |`void`     | end editing the control                            |
|method  |getMinValue          |              |`double`   | get the minimum value of the control               |
|method  |getMaxValue          |              |`double`   | get the maximum value of the control               |
|method  |getTag               |              |`integer`  | get the control tag                                |

### Control listeners

|name          |arguments                   |
|--------------|----------------------------|
|onValueChanged|view:`object`,value:`double`|
|onBeginEdit   |view:`object`               |
|onEndEdit     |view:`object`               |

### View container listeners

|name         |arguments                   |
|-------------|----------------------------|
|onViewAdded  |view:`object`,child:`object`|
|onViewRemoved|view:`object`,child:`object`|

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

### Example

```js
// Hover Opacity Animation Script
// This example script changes the opacity of the view
// when the mouse enters or exits the view

/* the default opacity of the view is stored in view.default_opacity */
view.default_opacity = 0.6;

/* the current opacity of the view is stored in view.opacity */
view.opacity = view.default_opacity;

/* the timer to change the opacity is stored in view.opacity_timer */
view.opacity_timer = createTimer(view, 16, function(view) {
	view.opacity += view.opacity_change;
	if (view.opacity_change > 0)
	{
		if (view.opacity > 1)
		{
			view.opacity = 1;
			view.opacity_timer.stop();
		}
	}
	else
	{
		if (view.opacity <= view.default_opacity)
		{
			view.opacity = view.default_opacity;
			view.opacity_timer.stop();
		}
	}
	view.setAttribute("opacity", view.opacity);
});

/* the view will be shown with full opacity when focused so the state of the focus is stored in view.hasFocus */
view.has_focus = false;

/* to correctly restore the hover state after focus lost, the state if the mouse is inside the view or outside 
   is stored in view.mouseInside
*/
view.mouse_inside = false;

/* we install a mouse enter listener
   when the mouse enters the view we start the opacity change timer 
*/
view.onMouseEnter = function(view, event) {
	view.mouse_inside = true;
	if (view.has_focus)
		return;
	view.opacity_change = 0.075;
	view.opacity_timer.start();
	event.consume = true;
};

/* we also install a mouse exit listener
   when the mouse exits the view we start the opacity change timer again 
   now with a negative opacity_change variable so that in the timer callback 
   the opacity is going back to the default opacity 
*/
view.onMouseExit = function(view, event) {
	view.mouse_inside = false;
	if (view.has_focus)
		return;
	view.opacity_change = -0.05;
	view.opacity_timer.start();
	event.consumed = true;
};

/* we also install a view removed listener so that we can cleanup and stop the timer */
view.onRemoved = function(view) {
	// cleanup, when the view is removed, stop the timer
	view.opacity_timer.stop();
};

/* when the view takes focus we show the view with full opacity */
view.onTookFocus = function(view) {
	view.has_focus = true;
	view.opacity_timer.stop();
	view.setAttribute("opacity", 1);
	view.opacity = 1;
};

/* when the view lost focus we start the opacity animation when the mouse is not inside this view */
view.onLostFocus = function(view) {
	view.has_focus = false;
	if (!view.mouse_inside)
	{
		view.onMouseExit(view, undefind);
	}
};

/* enable the mouse, otherwise no mouse listener is called */
view.setAttribute("mouse-enabled", true);

/* set the initial view opacity*/
view.setAttribute("opacity", view.opacity);
```
