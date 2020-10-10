// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../optional.h"
#include "x11platform.h"
#include <X11/Xlib.h>
#include <string>

struct xcb_visualtype_t;

//------------------------------------------------------------------------
namespace VSTGUI {
namespace X11 {

//------------------------------------------------------------------------
struct ChildWindow
{
	using xcb_window_t = uint32_t;

	ChildWindow (::Window parentId, CPoint size);

	~ChildWindow () noexcept;

	xcb_window_t getID () const;
	xcb_visualtype_t* getVisual () const;

	void setSize (const CRect& rect);

	const CPoint& getSize () const;

private:
	xcb_window_t id;
	CPoint size;
	xcb_visualtype_t* visual{nullptr};
};

//------------------------------------------------------------------------
struct Atom
{
	using xcb_atom_t = uint32_t;

	Atom (const char* name);

	bool valid () const;
	xcb_atom_t operator() () const;

private:
	void create () const;

	std::string name;
	mutable Optional<xcb_atom_t> value;
};

//------------------------------------------------------------------------
struct XEmbedInfo
{
	uint32_t version{1};
	uint32_t flags{0};
};

//------------------------------------------------------------------------
/* XEMBED messages */
enum class XEMBED
{
	EMBEDDED_NOTIFY = 0,
	WINDOW_ACTIVATE = 1,
	WINDOW_DEACTIVATE = 2,
	REQUEST_FOCUS = 3,
	FOCUS_IN = 4,
	FOCUS_OUT = 5,
	FOCUS_NEXT = 6,
	FOCUS_PREV = 7,
	/* 8-9 were used for GRAB_KEY/UNGRAB_KEY */
	MODALITY_ON = 10,
	MODALITY_OFF = 11,
	REGISTER_ACCELERATOR = 12,
	UNREGISTER_ACCELERATOR = 13,
	ACTIVATE_ACCELERATOR = 14,
};

//------------------------------------------------------------------------
namespace Atoms {

extern Atom xEmbedInfo;
extern Atom xEmbed;
extern Atom xDndAware;
extern Atom xDndProxy;
extern Atom xDndEnter;
extern Atom xDndPosition;
extern Atom xDndLeave;
extern Atom xDndStatus;
extern Atom xDndDrop;
extern Atom xDndTypeList;
extern Atom xDndSelection;
extern Atom xDndFinished;
extern Atom xDndActionCopy;
extern Atom xDndActionMove;
extern Atom xVstguiSelection;

//------------------------------------------------------------------------
}

using xcb_atom_t = uint32_t;
std::string getAtomName (xcb_atom_t atom);

} // X11
} // VSTGUI
