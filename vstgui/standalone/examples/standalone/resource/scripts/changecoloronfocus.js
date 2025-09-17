colors = [];
colors["black"] = "black";
colors["white"] = "white";

view.onLostFocus = function (view) {
	view.setAttribute("font-color", colors["white"]);
	view.setAttribute("back-color", colors["black"]);
	view.setAttribute("frame-color", colors["white"]);
};

view.onTookFocus = function (view) {
	view.setAttribute("font-color", "navy");
	view.setAttribute("back-color", "teal");
	view.setAttribute("frame-color", "midnightblue");
};
