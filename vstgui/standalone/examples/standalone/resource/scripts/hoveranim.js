// Hover Opacity Animation Script
// This example script changes the opacity of the view
// when the mouse enters or exits the view

// option: use full opacity on focus
// when true the opacity is 1 when the view has focus, even if the mouse is outside
// otherwise the view will use the default opacity instead
view.option.use_full_opacity_on_focus = true;

// the opacity to use when the view is not hovered by the mouse
view.option.default_opacity = 0.6;

// the current opacity of the view is stored in view.opacity
view.opacity = view.option.default_opacity;

// the function which is called when the timer fires (see view.opacity_timer)
view.onOpacityTimer = function(view) {
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
		if (view.opacity <= view.option.default_opacity)
		{
			view.opacity = view.option.default_opacity;
			view.opacity_timer.stop();
		}
	}
	view.setAttribute("opacity", view.opacity);
};

// the timer to change the opacity is stored in view.opacity_timer
view.opacity_timer = createTimer(view, 16, view.onOpacityTimer);

// variable to remember the state of the focus
view.has_focus = false;

// variable to remember if the mouse is inside when the view took focus
view.mouse_inside = false;

// we install a mouse enter listener
// when the mouse enters the view we start the opacity change timer
view.onMouseEnter = function(view, event) {
	view.mouse_inside = true;
	if (view.has_focus)
		return;
	view.opacity_change = 0.075;
	view.opacity_timer.start();
	event.consume = true;
};

// we also install a mouse exit listener
// when the mouse exits the view we start the opacity change timer again
// now with a negative opacity_change variable so that in the timer callback
// the opacity is going back to the default opacity
view.onMouseExit = function(view, event) {
	view.mouse_inside = false;
	if (view.has_focus)
		return;
	view.opacity_change = -0.05;
	view.opacity_timer.start();
	event.consumed = true;
};

// we also install a view removed listener so that we can cleanup and stop the timer
view.onRemoved = function(view) {
	// cleanup, when the view is removed, stop the timer
	view.opacity_timer.stop();
};

// when the view takes focus we show the view with full opacity if the option is set
view.onTookFocus = function(view) {
	if(view.option.use_full_opacity_on_focus)
	{
		view.has_focus = true;
		view.opacity_timer.stop();
		view.setAttribute("opacity", 1);
		view.opacity = 1;
	}
};

// when the view lost focus we start the opacity animation when the mouse is not
// inside this view if the option is set
view.onLostFocus = function(view) {
	if(view.option.use_full_opacity_on_focus)
	{
		view.has_focus = false;
		if (!view.mouse_inside)
		{
			view.onMouseExit(view, undefind);
		}
	}
};

// enable the mouse, otherwise no mouse listener is called
view.setAttribute("mouse-enabled", true);

// set the initial view opacity
view.setAttribute("opacity", view.opacity);

