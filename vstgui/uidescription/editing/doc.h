/**

TODO:
	Musts:
		- template changes on the undo stack (we need to create all template views...)
		- finish edit menu
			- insert template
		- fix TODOs
		- write documentation

	Nice To Have:
		- better selection mouse handling in UIEditView
		- grid handling improvements (like custom magnetic lines)
		- write Windows .rc file
		- more custom attribute editing
		- view z-index editing via drag&drop
		
	Bugs:
		- Unembed views out of CScrollView deletes all views
		- Changing a bitmap to a nineparttiled bitmap does not change the views which uses that bitmap
		- tag name, color name, bitmap name and font name changes are not reflected by views in non selected templates

*/
