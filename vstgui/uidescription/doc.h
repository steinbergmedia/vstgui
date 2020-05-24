/**

@page uidescription Creating user interfaces

You can create user interfaces in VSTGUI via an uidescription file instead of using c++ code to create views and controls, just controller code needs to be coded in c++.

> The format has changed from XML to JSON in version 4.10, due to security and performance characteristics of XML and JSON.

It's preferred to not write the JSON file by hand, but use the included WYSIWYG editor.
You can use the editor when targeting a standalone application or an editor for a VST3 plug-in. If you need this for something else you can write the glue code yourself.

- @subpage page_uidescription_editor
- @subpage the_view_system
- @subpage create_your_own_view
- @ref standalone_library

<hr/>

> For the old documentation see @subpage uidescriptionXML

*/