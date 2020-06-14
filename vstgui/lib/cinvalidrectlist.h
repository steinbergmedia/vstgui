// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "crect.h"
#include <vector>

namespace VSTGUI {

//-----------------------------------------------------------------------------
struct CInvalidRectList
{
	using RectList = std::vector<CRect>;

	bool add (const CRect& r);

	RectList::iterator begin () { return list.begin (); }
	RectList::iterator end () { return list.end (); }
	RectList::const_iterator begin () const { return list.begin (); }
	RectList::const_iterator end () const { return list.end (); }

	void clear () { list.clear (); }
	const RectList& data () const { return list; }

private:
	RectList list;
};

//-----------------------------------------------------------------------------
inline bool CInvalidRectList::add (const CRect& r)
{
	for (auto it = list.begin (), end = list.end (); it != end; ++it)
	{
		// the same rectangle is already in the list
		if (*it == r)
			return false;
		// the new rectangle is part of one already in the list
		if (it->rectInside (r))
			return false;
		// if the new rectangle contains one of the previous rectangles
		if (r.rectInside (*it))
		{
			list.erase (it);
			return add (r);
		}
		// now check if the combined rect has the same or less area as both rects together
		auto area1 = r.getWidth () * r.getHeight ();
		auto area2 = it->getWidth () * it->getHeight ();
		CRect jr (*it);
		jr.unite (r);
		auto joinedArea = jr.getWidth () * jr.getHeight ();
		if (joinedArea <= (area1 + area2))
		{
			list.erase (it);
			return add (jr);
		}
	}
	list.emplace_back (r);
	return true;
}

//-----------------------------------------------------------------------------
} // VSTGUI
