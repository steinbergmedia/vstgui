// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../vstguifwd.h"
#include "../../dragging.h"
#include "../../optional.h"
#include <xcb/xcb.h>
#include <vector>
#undef None

//------------------------------------------------------------------------
namespace VSTGUI {
namespace X11 {

struct Atom;
struct ChildWindow;

//------------------------------------------------------------------------
class XdndDataPackage : public IDataPackage
{
public:
	uint32_t getCount () const override;
	uint32_t getDataSize (uint32_t index) const override;
	Type getDataType (uint32_t index) const override;
	uint32_t getData (uint32_t index, const void*& buffer, Type& type) const override;

	void setPackageType (Type t) { packageType = t; }
	void setPackageData (std::vector<std::string>&& d) { packageData = std::move(d); }

private:
	Type packageType = Type::kError;
	std::vector<std::string> packageData;
};

//------------------------------------------------------------------------
class XdndHandler
{
public:
	XdndHandler (ChildWindow* window, IPlatformFrameCallback* frame);
	void enter (xcb_client_message_event_t& event, xcb_window_t targetId);
	void position (xcb_client_message_event_t& event);
	void leave (xcb_client_message_event_t& event);
	void drop (xcb_client_message_event_t& event);
	void selectionNotify (xcb_selection_notify_event_t& event);

private:
	enum class State {
		DragClear,
		DragInitiated,
		DragEntering,
		DragMoving,
	};

	ChildWindow* window = nullptr;
	IPlatformFrameCallback* frame = nullptr;
	State state = State::DragClear;
	xcb_window_t dndTarget = 0;
	xcb_window_t dndSource = 0;
	xcb_atom_t dndType = XCB_ATOM_NONE;
	Optional<xcb_client_message_event_t> dndPosition;
	SharedPointer<XdndDataPackage> package;
	DragOperation dragOperation = DragOperation::None;

	void clearState ();
	DragEventData getEventData () const;
	CPoint getEventPosition () const;
	void replyStatus ();
	void replyFinished ();

	typedef std::vector<xcb_atom_t> TypeList;
	static TypeList getTypeList (xcb_client_message_event_t& event);
	static xcb_atom_t findFilePathType (const TypeList& typeList);
	static xcb_atom_t findTextType (const TypeList& typeList);
	static xcb_atom_t findBinaryType (const TypeList& typeList);
	static xcb_atom_t searchType (const TypeList& typeList, const Atom& atom);
	static void extractFilePathsFromUriList (const std::string& data, std::vector<std::string>& filePaths);
};

bool isXdndClientMessage (const xcb_client_message_event_t& event);
xcb_window_t getXdndProxy (xcb_window_t windowId);

//------------------------------------------------------------------------
} // X11
} // VSTGUI
