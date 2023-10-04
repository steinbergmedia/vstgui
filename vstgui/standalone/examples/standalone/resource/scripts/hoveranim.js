view.default_opacity = 0.5;
view.opacity = view.default_opacity;
view.opacityTimer = createTimer (view, 16, function (view) {
	view.opacity += view.opacityChange;
	if (view.opacityChange > 0)
	{
		if (view.opacity > 1)
		{
			view.opacity = 1;
			view.opacityTimer.stop();
		}
	}
	else
	{
		if (view.opacity <= view.default_opacity)
		{
			view.opacity = view.default_opacity;
			view.opacityTimer.stop();
		}
	}
	view.setAttribute("opacity", view.opacity);
});
view.setAttribute("mouse-enabled", true);
view.setAttribute("opacity", view.opacity);
view.onRemoved = function (view) { view.opacityTimer.stop(); };
view.onMouseEnter = function (view, event) {
	view.opacityChange = 0.1;
	view.opacityTimer.start();
};
view.onMouseExit = function (view, event) {
	view.opacityChange = -0.05;
	view.opacityTimer.start();
	event.consume = 1;
};
