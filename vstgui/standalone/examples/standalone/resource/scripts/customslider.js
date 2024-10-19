view.data = {
	backColor: "control.back",
	frameColor: "control.frame",
	valueColor: "control.value",
	valueFrameColor: "control.valueframe",
	lineColor: "control.line",
	isVertical: false,
	handleWidth: 10,
	handleRange: 0,
	handleRect: {},
	numLines: 9,
	lineInset: 3,
	secondLineInset: 3,
	mouseDownStart: {x: -1, y: -1},
};

view.onSizeChanged = function(view, newRect) {
	bounds = view.getBounds ();
	view.data.handleRect = bounds;
	view.data.handleRect.left += 2;
	view.data.handleRect.right -= 2;
	view.data.handleRect.top += 2;
	view.data.handleRect.bottom -= 2;
	var width = bounds.getWidth();
	var height = bounds.getHeight();
	view.data.isVertical = width < height;
	if(view.data.isVertical)
	{
		view.data.handleRect.bottom = view.data.handleRect.top + view.data.handleWidth;
		view.data.handleRange = height - view.data.handleWidth;
	}
	else
	{
		view.data.handleRect.right = view.data.handleRect.left + view.data.handleWidth;
		view.data.handleRange = width - view.data.handleWidth;
	}
};

view.getHandleRect = function() {
	var handle = view.data.handleRect.clone();
	var offset = view.data.handleRange * view.getValue();
	if(view.data.isVertical)
	{
		handle.top += offset;
		handle.bottom += offset;
	}
	else
	{
		handle.left += offset;
		handle.right += offset;
	}
	return handle;
};

view.draw = function(context, dirtyRect) {
	var bounds = view.getBounds();

	roundRect = context.createRoundGraphicsPath(bounds, 4);

	context.setFillColor(view.data.backColor);
	context.setFrameColor(view.data.frameColor);
	context.setLineWidth(1);
	context.drawGraphicsPath(roundRect, "filled");
	context.drawGraphicsPath(roundRect, "stroked");

	context.setFrameColor(view.data.lineColor);
	context.setLineWidth(1);
	var handle = view.getHandleRect();

	numLines = view.data.numLines + 1;
	drawOffset = view.data.handleRange / numLines;
	lineInset = view.data.lineInset;
	if(view.data.isVertical)
	{
		y = drawOffset + view.data.handleWidth / 2;
		for(i = 1; i < numLines; i++)
		{
			start = {x: handle.left + lineInset, y: Math.round (y)};
			end = {x: handle.right - lineInset, y: Math.round (y)};
			if(!(i%2))
			{
				start.x += view.data.secondLineInset;
				end.x -= view.data.secondLineInset;
			}
			context.drawLine(start, end);
			y += drawOffset;
		}
	}
	else
	{
		x = drawOffset + view.data.handleWidth / 2;
		for(i = 1; i < numLines; i++)
		{
			start = {x: Math.round (x), y: handle.top + lineInset};
			end = {x: Math.round (x), y: handle.bottom - lineInset};
			if(!(i%2))
			{
				start.y += view.data.secondLineInset;
				end.y -= view.data.secondLineInset;
			}
			context.drawLine(start, end);
			x += drawOffset;
		}
	}

	roundRect = context.createRoundGraphicsPath(handle, 3);
	context.setFillColor(view.data.valueColor);
	context.setFrameColor(view.data.valueFrameColor);
	context.setLineWidth(1);
	context.drawGraphicsPath(roundRect, "filled");
	context.drawGraphicsPath(roundRect, "stroked");
};

view.drawFocusOnTop = function () { return false; };

view.getFocusPath = function (path, focusWidth) {
	var bounds = view.getBounds();
	path.addRoundRect(bounds, 4);
	bounds.left -= focusWidth;
	bounds.right += focusWidth;
	bounds.top -= focusWidth;
	bounds.bottom += focusWidth;
	path.addRoundRect(bounds, 4);
	return true;
};

view.onMouseDown = function(view, event) {
	if(!event.mouseButtons.left)
		return;
	if (event.modifiers.control)
	{
		// pass thru to default implementation for default handling
		return;
	}
	view.data.mouseDownStart = event.mousePosition;
	view.beginEdit();
	view.onMouseMove(view, event);
	event.consumed = true;
};

view.onMouseUp = function(view, event) {
	if(!event.mouseButtons.left || view.data.mouseDownStart.x == -1.0)
		return;
	view.data.mouseDownStart = {x: -1.0, y: -1.0};
	view.endEdit();
	event.consumed = true;
};

view.onMouseMove = function(view, event) {
	if(!event.mouseButtons.left || view.data.mouseDownStart.x === -1.0)
		return;
	var pos;
	if(view.data.isVertical)
	{
		event.mousePosition.y -= view.data.handleWidth / 2;
		pos = 1 - (view.data.handleRange - event.mousePosition.y) / view.data.handleRange;
	}
	else
	{
		event.mousePosition.x -= view.data.handleWidth / 2;
		pos = 1 - (view.data.handleRange - event.mousePosition.x) / view.data.handleRange;
	}
	if(pos < 0)
		pos = 0;
	else if(pos > 1)
		pos = 1;
	view.setValueNormalized(pos);
	event.consumed = true;
};

view.onMouseWheel = function(view, event) {
	if(view.data.isVertical)
	{
		if(event.mouseWheel.deltaY != 0)
		{
			if(event.mouseWheel.directionInvertedFromDevice)
				event.mouseWheel.deltaY = -event.mouseWheel.deltaY;
			if (event.modifiers.shift)
				event.mouseWheel.deltaY = event.mouseWheel.deltaY * 0.1;
			view.beginEdit();
			view.setValueNormalized(view.getValueNormalized() + event.mouseWheel.deltaY / 10);
			view.endEdit();
		}
	}
	else
	{
		if(event.mouseWheel.deltaX != 0)
		{
			if(event.mouseWheel.directionInvertedFromDevice)
				event.mouseWheel.deltaX = -event.mouseWheel.deltaX;
			if (event.modifiers.shift)
				event.mouseWheel.deltaX = event.mouseWheel.deltaX * 0.1;
			view.beginEdit();
			view.setValueNormalized(view.getValueNormalized() + event.mouseWheel.deltaX / 10);
			view.endEdit();
		}
	}
	event.consumed = true;
};

view.onSizeChanged(view, view.getBounds());

