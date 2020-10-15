// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "x11dragging.h"
#include "x11utils.h"
#include <glib.h>
#include <cstdio>
#include <cassert>
#undef None

#if 0
#define DndTrace(fmt, ...) fprintf (stderr, "Dnd " fmt "\n", ## __VA_ARGS__);
#else
#define DndTrace(fmt, ...)
#endif

/*
  Notes:

  This is implemented according to the XDND specification, version 5. (1)

  The Xdnd receiver:
  * receive the `XdndEnter` message
  * extract the types present in the dragged data
    - identify a matching mimetype for either path, text, or binary
  * receive the initial `XdndPosition` message
    - request the X server to retrieve the data for the matched type
    - identify this data with our proprietary atom `XVSTGUISelection`
    - cache this message to process it when ready, in the next step
  * receive a `SelectionNotify` message for our desired type
    - data is ready, extract it
    - finish building the IDataPackage, call `OnDragEnter`
    - send the XdndStatus message, with the desired drag operation
  * receive additional `XdndPosition` messages
    - call `OnDragMove`
    - send the `XdndStatus` message, with the desired drag operation
  * either:
    * receive the `XdndLeave` message
      - call `OnDragLeave`
      - clean up
    * receive the `XdndDrop` message
      - call `OnDrop` if data is accepted, otherwise `OnDragLeave`
      - send `XdndFinished`
      - clean up

  Remark:
    If the receiver is proxied by another window, the replies of type
    `XdndStatus` and `XdndFinished` should also be directed to the proxy.
    This is not supposed to be necessary, but it fixes GTK2 hosts.
    This GTK2 problem will likely not be fixed. (2)

  References
  (1) the XDND specification
      https://freedesktop.org/wiki/Specifications/XDND/
  (2) GTK does not forward XDND protocol messages for X11 embedded windows
      https://gitlab.gnome.org/GNOME/gtk/-/issues/2329

 */

//------------------------------------------------------------------------
namespace VSTGUI {
namespace X11 {

uint32_t XdndDataPackage::getCount () const
{
	return packageData.size ();
}

uint32_t XdndDataPackage::getDataSize (uint32_t index) const
{
	if (index >= packageData.size ())
		return 0;

	return packageData[index].size ();
}

IDataPackage::Type XdndDataPackage::getDataType (uint32_t index) const
{
	if (index >= packageData.size ())
		return Type::kError;

	return packageType;
}

uint32_t XdndDataPackage::getData (uint32_t index, const void*& buffer, Type& type) const
{
	if (index >= packageData.size ())
	{
		buffer = nullptr;
		type = Type::kError;
		return 0;
	}

	buffer = packageData[index].data ();
	type = packageType;
	return packageData[index].size ();
}

//------------------------------------------------------------------------
XdndHandler::XdndHandler (ChildWindow* window, IPlatformFrameCallback* frame)
	: window (window), frame (frame)
{
}

void XdndHandler::enter (xcb_client_message_event_t& event, xcb_window_t targetId)
{
	clearState ();

	unsigned protocolVersion = event.data.data32[1] >> 24;
	if (protocolVersion < 5)
		return;

	DndTrace ("[recv] Enter window=%08X, source=%08X",
			  event.window,
			  event.data.data32[0]);

	if (!Atoms::xDndSelection.valid () || !Atoms::xVstguiSelection.valid ())
		return;

	std::vector<xcb_atom_t> typeList = getTypeList (event);
	IDataPackage::Type packageType = IDataPackage::Type::kError;

	if (dndType == XCB_ATOM_NONE)
	{
		dndType = findFilePathType(typeList);
		if (dndType != XCB_ATOM_NONE)
			packageType = IDataPackage::kFilePath;
	}
	if (dndType == XCB_ATOM_NONE)
	{
		dndType = findTextType(typeList);
		if (dndType != XCB_ATOM_NONE)
			packageType = IDataPackage::kText;
	}
	if (dndType == XCB_ATOM_NONE)
	{
		dndType = findBinaryType(typeList);
		if (dndType != XCB_ATOM_NONE)
			packageType = IDataPackage::kBinary;
	}

	if (packageType != IDataPackage::Type::kError)
	{
		package = makeOwned<XdndDataPackage> ();
		package->setPackageType (packageType);

		state = State::DragInitiated;
		dndTarget = targetId;
		dndSource = event.data.data32[0];
	}
}

void XdndHandler::position (xcb_client_message_event_t& event)
{
	DndTrace ("[recv] Position window=%08X, source=%08X, x=%d, y=%d, action=%s",
			  event.window,
			  event.data.data32[0],
			  event.data.data32[2] >> 16, event.data.data32[2] & 0xffff,
			  getAtomName (event.data.data32[4]).c_str ());

	if (event.data.data32[0] != dndSource)
		return;

	switch (state)
	{
	case State::DragInitiated:
		{
			dndPosition = Optional<xcb_client_message_event_t> (event);

			auto xcb = RunLoop::instance ().getXcbConnection ();

			xcb_delete_property (
				xcb, window->getID (), Atoms::xVstguiSelection ());

			xcb_convert_selection (
				xcb, window->getID (), Atoms::xDndSelection (), dndType,
				Atoms::xVstguiSelection (), dndPosition->data.data32[3]);
		}
		break;
	case State::DragEntering:
		dragOperation = frame->platformOnDragEnter (getEventData ());
		state = State::DragMoving;
		replyStatus ();
		break;
	case State::DragMoving:
		dragOperation = frame->platformOnDragMove (getEventData ());
		replyStatus ();
		break;
	default:
		break;
	}
}

void XdndHandler::leave (xcb_client_message_event_t& event)
{
	DndTrace ("[recv] Leave window=%08X, source=%08X",
			  event.window,
			  event.data.data32[0]);

	if (event.data.data32[0] != dndSource)
		return;

	if (dndPosition)
	{
		frame->platformOnDragLeave (getEventData ());
	}

	clearState ();
}

void XdndHandler::drop (xcb_client_message_event_t& event)
{
	DndTrace ("[recv] Drop window=%08X, source=%08X",
			  event.window,
			  event.data.data32[0]);

	if (event.data.data32[0] != dndSource)
		return;

	if (dndPosition)
	{
		if (dragOperation != DragOperation::None)
			frame->platformOnDrop (getEventData ());
		else
			frame->platformOnDragLeave (getEventData ());
		replyFinished ();
	}

	clearState ();
}

void XdndHandler::selectionNotify (xcb_selection_notify_event_t& event)
{
	if (state == State::DragInitiated &&
		event.requestor == window->getID () && event.target == dndType &&
		Atoms::xDndSelection.valid () && Atoms::xVstguiSelection.valid () &&
		event.selection == Atoms::xDndSelection () &&
		event.property == Atoms::xVstguiSelection ())
	{
		auto xcb = RunLoop::instance ().getXcbConnection ();

		auto cookie = xcb_get_property (
			xcb, true, window->getID (), Atoms::xVstguiSelection (),
			XCB_GET_PROPERTY_TYPE_ANY, 0, 4096);

		std::vector<std::string> packageData;

		auto reply = xcb_get_property_reply (xcb, cookie, nullptr);
		if (reply)
		{
			std::string data (
				reinterpret_cast<char*> (xcb_get_property_value (reply)),
				xcb_get_property_value_length (reply));

			if (Atoms::xMimeTypeUriList.valid () &&
				dndType == Atoms::xMimeTypeUriList ())
			{
				extractFilePathsFromUriList (data, packageData);
			}
			else
			{
				packageData.resize (1);
				packageData[0] = std::move (data);
			}

			free (reply);
		}

		if (packageData.empty ())
			clearState ();
		else
		{
			package->setPackageData (std::move (packageData));
			state = State::DragEntering;

			if (dndPosition)
				position (*dndPosition);
		}
	}
}

void XdndHandler::clearState ()
{
	state = State::DragClear;
	dndTarget = 0;
	dndSource = 0;
	dndType = XCB_ATOM_NONE;
	dndPosition.reset ();
	package = SharedPointer<XdndDataPackage> ();
	dragOperation = DragOperation::None;
}

DragEventData XdndHandler::getEventData () const
{
	assert (package);

	DragEventData eventData;

	eventData.drag = package.get ();
	eventData.pos = getEventPosition ();
	// TODO: the modifiers

	return eventData;
}

CPoint XdndHandler::getEventPosition () const
{
	assert (dndPosition);

	xcb_client_message_event_t event = *dndPosition;

	int x = event.data.data32[2] >> 16;
	int y = event.data.data32[2] & 0xffff;

	auto xcb = RunLoop::instance ().getXcbConnection ();
	auto setup = xcb_get_setup (xcb);
	auto iter = xcb_setup_roots_iterator (setup);
	auto screen = iter.data;

	auto cookie = xcb_translate_coordinates (
		xcb, screen->root, window->getID (), x, y);
	auto reply = xcb_translate_coordinates_reply (xcb, cookie, nullptr);
	if (reply)
	{
		x = reply->dst_x;
		y = reply->dst_y;
		free (reply);
	}

	return CPoint (x, y);
}

void XdndHandler::replyStatus ()
{
	if (!Atoms::xDndStatus.valid ())
		return;

	bool accepted = dragOperation != DragOperation::None;
	xcb_window_t dndSource = dndPosition->data.data32[0];

	xcb_client_message_event_t event {};
	event.response_type = XCB_CLIENT_MESSAGE;
	event.format = 32;
	event.window = dndSource;
	event.type = Atoms::xDndStatus ();
	event.data.data32[0] = dndTarget;
	event.data.data32[1] = accepted;

	switch (dragOperation)
	{
	case DragOperation::Copy:
		if (Atoms::xDndActionCopy.valid ())
			event.data.data32[4] = Atoms::xDndActionCopy ();
		break;
	case DragOperation::Move:
		if (Atoms::xDndActionMove.valid ())
			event.data.data32[4] = Atoms::xDndActionMove ();
		break;
	default:
		break;
	}

	auto xcb = RunLoop::instance ().getXcbConnection ();

	xcb_window_t receiver = getXdndProxy (dndSource);
	if (receiver == 0)
		receiver = dndSource;

	DndTrace ("[send] Status receiver=%08X, window=%08X, target=%08X, accept=%d, x=%d, y=%d, w=%d, h=%d, action=%s",
			  receiver,
			  event.window,
			  event.data.data32[0],
			  event.data.data32[1] & 1,
			  event.data.data32[2] >> 16, event.data.data32[2] & 0xffff,
			  event.data.data32[3] >> 16, event.data.data32[3] & 0xffff,
			  event.data.data32[4] ? getAtomName (event.data.data32[4]).c_str () : "None");

	xcb_send_event (
		xcb, false, receiver, XCB_EVENT_MASK_NO_EVENT,
		reinterpret_cast<const char*> (&event));
}

void XdndHandler::replyFinished ()
{
	if (!Atoms::xDndFinished.valid ())
		return;

	bool accepted = dragOperation != DragOperation::None;
	xcb_window_t dndSource = dndPosition->data.data32[0];

	xcb_client_message_event_t event {};
	event.response_type = XCB_CLIENT_MESSAGE;
	event.format = 32;
	event.window = dndSource;
	event.type = Atoms::xDndFinished ();
	event.data.data32[0] = dndTarget;
	event.data.data32[1] = accepted;

	switch (dragOperation)
	{
	case DragOperation::Copy:
		if (Atoms::xDndActionCopy.valid ())
			event.data.data32[2] = Atoms::xDndActionCopy ();
		break;
	case DragOperation::Move:
		if (Atoms::xDndActionMove.valid ())
			event.data.data32[2] = Atoms::xDndActionMove ();
		break;
	default:
		break;
	}

	auto xcb = RunLoop::instance ().getXcbConnection ();

	xcb_window_t receiver = getXdndProxy (dndSource);
	if (receiver == 0)
		receiver = dndSource;

	DndTrace ("[send] Finished receiver=%08X, window=%08X, target=%08X, accept=%d, action=%s",
			  receiver,
			  event.window,
			  event.data.data32[0],
			  event.data.data32[1] & 1,
			  event.data.data32[2] ? getAtomName (event.data.data32[2]).c_str () : "None");

	xcb_send_event (
		xcb, false, receiver, XCB_EVENT_MASK_NO_EVENT,
		reinterpret_cast<const char*> (&event));
}

XdndHandler::TypeList XdndHandler::getTypeList (xcb_client_message_event_t& event)
{
	TypeList typeList;
	typeList.reserve (32);

	xcb_window_t sourceId = event.data.data32[0];
	bool longTypeList = event.data.data32[1] & 1;

	if (!longTypeList)
	{
		for (int i = 2; i < 5; ++i) {
			uint32_t type = event.data.data32[i];
			if (type != XCB_NONE)
				typeList.push_back (type);
		}
	}
	else if (Atoms::xDndTypeList.valid ())
	{
		auto xcb = RunLoop::instance ().getXcbConnection ();

		auto cookie = xcb_get_property (
			xcb, false, sourceId, Atoms::xDndTypeList (), XCB_ATOM_ATOM,
			0, typeList.capacity ());
		auto reply = xcb_get_property_reply (xcb, cookie, nullptr);
		if (reply)
		{
			int length = xcb_get_property_value_length (reply) / 4;
			const uint32_t* data =	static_cast<uint32_t*> (xcb_get_property_value (reply));
			for (int i = 0; i < length; ++i)
				typeList.push_back (data[i]);
			free (reply);
		}
	}

	return typeList;
}

xcb_atom_t XdndHandler::findFilePathType (const TypeList& typeList)
{
	xcb_atom_t type = XCB_ATOM_NONE;

	if (type == XCB_ATOM_NONE)
		type = searchType (typeList, Atoms::xMimeTypeUriList);

	return type;
}

xcb_atom_t XdndHandler::findTextType (const TypeList& typeList)
{
	xcb_atom_t type = XCB_ATOM_NONE;

	if (type == XCB_ATOM_NONE)
		type = searchType (typeList, Atoms::xMimeTypeTextPlainUtf8);
	if (type == XCB_ATOM_NONE)
		type = searchType (typeList, Atoms::xMimeTypeTextPlain);

	return type;
}

xcb_atom_t XdndHandler::findBinaryType (const TypeList& typeList)
{
	xcb_atom_t type = XCB_ATOM_NONE;

	if (type == XCB_ATOM_NONE)
		type = searchType (typeList, Atoms::xMimeTypeApplicationOctetStream);

	return type;
}

xcb_atom_t XdndHandler::searchType (const TypeList& typeList, const Atom& atom)
{
	if (typeList.empty () || !atom.valid ())
		return XCB_ATOM_NONE;

	xcb_atom_t needle = atom ();
	for (xcb_atom_t type : typeList) {
		if (type == needle)
			return type;
	}

		return XCB_ATOM_NONE;
}

void XdndHandler::extractFilePathsFromUriList (const std::string& data, std::vector<std::string>& filePaths)
{
	filePaths.clear ();
	filePaths.reserve (8);

	char** uriList = g_uri_list_extract_uris (data.c_str ());
	if (!uriList)
		return;

	for (char** uriPtr = uriList; *uriPtr; ++uriPtr)
	{
		char* uriHostname = nullptr;
		char* uriFilename = g_filename_from_uri (*uriPtr, &uriHostname, nullptr);
		if (uriFilename)
		{
			if (!uriHostname)
				filePaths.push_back (uriFilename);
			g_free (uriFilename);
			g_free (uriHostname);
		}
	}

	g_strfreev (uriList);
}

//------------------------------------------------------------------------
bool isXdndClientMessage (const xcb_client_message_event_t& event)
{
	if ((event.response_type & ~0x80) != XCB_CLIENT_MESSAGE)
		return false;

	const std::string name = getAtomName (event.type);
	return name.size () >= 4 && !memcmp (name.data (), "Xdnd", 4);
}

xcb_window_t getXdndProxy (xcb_window_t windowId)
{
	auto xcb = RunLoop::instance ().getXcbConnection ();
	xcb_window_t proxyId = 0;
	xcb_get_property_cookie_t cookie = xcb_get_property (
		xcb, false, windowId, Atoms::xDndProxy (),
		XCB_ATOM_WINDOW, 0, 1);
	xcb_get_property_reply_t *reply = xcb_get_property_reply (
		xcb, cookie, nullptr);
	if (reply) {
		if (xcb_get_property_value_length (reply) == 4)
			proxyId = *static_cast<uint32_t*> (xcb_get_property_value (reply));
		free (reply);
	}
	return proxyId;
}

//------------------------------------------------------------------------
} // X11
} // VSTGUI
