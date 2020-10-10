
#include "x11utils.h"
#include <xcb/xcb.h>
#include <xcb/xcb_util.h>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace X11 {
namespace {

//------------------------------------------------------------------------
static xcb_visualtype_t* getVisualType (const xcb_screen_t* screen)
{
	auto depth_iter = xcb_screen_allowed_depths_iterator (screen);
	for (; depth_iter.rem; xcb_depth_next (&depth_iter))
	{
		xcb_visualtype_iterator_t visual_iter;

		visual_iter = xcb_depth_visuals_iterator (depth_iter.data);
		for (; visual_iter.rem; xcb_visualtype_next (&visual_iter))
		{
			if (screen->root_visual == visual_iter.data->visual_id)
			{
				return visual_iter.data;
			}
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
ChildWindow::ChildWindow (::Window parentId, CPoint size)
	: size (size), id (xcb_generate_id (RunLoop::instance ().getXcbConnection ()))

{
	auto connection = RunLoop::instance ().getXcbConnection ();
	auto setup = xcb_get_setup (connection);
	auto iter = xcb_setup_roots_iterator (setup);
	auto screen = iter.data;
	visual = getVisualType (screen);
#if 0
		parentId = screen->root;
#endif
	uint32_t paramMask = XCB_CW_BACK_PIXMAP | XCB_CW_BACKING_STORE | XCB_CW_EVENT_MASK;
	xcb_params_cw_t params{};
	params.back_pixel = XCB_BACK_PIXMAP_NONE;
	params.backing_store = XCB_BACKING_STORE_WHEN_MAPPED;
	params.event_mask =
		XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_BUTTON_PRESS |
		XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
		XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_POINTER_MOTION_HINT |
		XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_PROPERTY_CHANGE |
		XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_FOCUS_CHANGE;

	xcb_aux_create_window (connection, XCB_COPY_FROM_PARENT, getID (), parentId, 0, 0, size.x,
						   size.y, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, XCB_COPY_FROM_PARENT,
						   paramMask, &params);

	// setup XEMBED
	if (Atoms::xEmbedInfo.valid ())
	{
		XEmbedInfo info;
		xcb_change_property (connection, XCB_PROP_MODE_REPLACE, getID (), Atoms::xEmbedInfo (),
							 Atoms::xEmbedInfo (), 32, 2, &info);
	}

	// setup Xdnd
	if (Atoms::xDndAware.valid ())
	{
		uint32_t version = 5;
		xcb_change_property (connection, XCB_PROP_MODE_REPLACE, getID (), Atoms::xDndAware (),
							 XCB_ATOM_ATOM, 32, 1, &version);
	}
	if (Atoms::xDndProxy.valid ())
	{
		uint32_t proxy = getID ();
		xcb_change_property (connection, XCB_PROP_MODE_REPLACE, getID (), Atoms::xDndProxy (),
							 XCB_ATOM_WINDOW, 32, 1, &proxy);
	}

	xcb_flush (connection);
}

//------------------------------------------------------------------------
ChildWindow::~ChildWindow () noexcept {}

//------------------------------------------------------------------------
xcb_window_t ChildWindow::getID () const
{
	return id;
}

//------------------------------------------------------------------------
xcb_visualtype_t* ChildWindow::getVisual () const
{
	return visual;
}

//------------------------------------------------------------------------
void ChildWindow::setSize (const CRect& rect)
{
	size = rect.getSize ();
	auto connection = RunLoop::instance ().getXcbConnection ();
	uint16_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
					XCB_CONFIG_WINDOW_HEIGHT;
	uint32_t values[] = {static_cast<uint32_t> (rect.left), static_cast<uint32_t> (rect.top),
						 static_cast<uint32_t> (rect.getWidth ()),
						 static_cast<uint32_t> (rect.getHeight ())};
	xcb_configure_window (connection, getID (), mask, values);
	xcb_flush (connection);
}

//------------------------------------------------------------------------
const CPoint& ChildWindow::getSize () const
{
	return size;
}

//------------------------------------------------------------------------
Atom::Atom (const char* name) : name (name) {}

//------------------------------------------------------------------------
bool Atom::valid () const
{
	create ();
	return value ? true : false;
}

//------------------------------------------------------------------------
auto Atom::operator() () const -> xcb_atom_t
{
	create ();
	return *value;
}

//------------------------------------------------------------------------
void Atom::create () const
{
	if (value)
		return;
	auto connection = RunLoop::instance ().getXcbConnection ();
	auto cookie = xcb_intern_atom (connection, 0, name.size (), name.data ());
	if (auto reply = xcb_intern_atom_reply (connection, cookie, nullptr))
	{
		value = Optional<xcb_atom_t> (reply->atom);
		free (reply);
	}
}

//------------------------------------------------------------------------
namespace Atoms {

Atom xEmbedInfo ("_XEMBED_INFO");
Atom xEmbed ("_XEMBED");
Atom xDndAware ("XdndAware");
Atom xDndProxy ("XdndProxy");
Atom xDndEnter ("XdndEnter");
Atom xDndPosition ("XdndPosition");
Atom xDndLeave ("XdndLeave");
Atom xDndStatus ("XdndStatus");
Atom xDndDrop ("XdndDrop");
Atom xDndTypeList ("XdndTypeList");
Atom xDndSelection ("XdndSelection");
Atom xDndFinished ("XdndFinished");
Atom xDndActionCopy ("XdndActionCopy");
Atom xDndActionMove ("XdndActionMove");
Atom xMimeTypeTextPlain ("text/plain");
Atom xMimeTypeTextPlainUtf8 ("text/plain;charset=utf-8");
Atom xMimeTypeUriList ("text/uri-list");
Atom xMimeTypeApplicationOctetStream ("application/octet-stream");
Atom xVstguiSelection ("XVSTGUISelection");

}

//------------------------------------------------------------------------
std::string getAtomName (xcb_atom_t atom)
{
	std::string name;
	auto xcb = RunLoop::instance ().getXcbConnection ();
	auto cookie = xcb_get_atom_name (xcb, atom);
	if (auto reply = xcb_get_atom_name_reply (xcb, cookie, nullptr))
	{
		auto length = xcb_get_atom_name_name_length (reply);
		name.assign (
			xcb_get_atom_name_name (reply),
			xcb_get_atom_name_name_length (reply));
		free (reply);
	}
	return name;
}

//------------------------------------------------------------------------
} // X11
} // VSTGUI
