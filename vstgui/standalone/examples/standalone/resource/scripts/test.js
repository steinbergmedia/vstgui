log (uiDesc.colorNames());
log (uiDesc.fontNames());
log (uiDesc.bitmapNames());
log (uiDesc.gradientNames());
log (uiDesc.controlTagNames());

//----------------------------
// iterate all subviews
view.iterateFunc = function (child, context) {
	log (child.type);
	iterateSubViews (child, context, context.iterateFunc);
};

iterateSubViews (view, view, view.iterateFunc);
