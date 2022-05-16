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

	void erase (RectList::iterator it) { list.erase (it); }

	void clear () { list.clear (); }
	const RectList& data () const { return list; }
	bool empty () const { return list.empty (); }

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
inline void joinNearbyInvalidRects (CInvalidRectList& list, CCoord maxDistance)
{
	for (auto it = list.begin (); it != list.end (); ++it)
	{
		for (auto it2 = list.begin (); it2 != list.end (); ++it2)
		{
			if (it2 == it)
				continue;
			if (it->left == it2->left && it->right == it2->right)
			{
				CCoord distance;
				if (it->bottom < it2->top)
					distance = it2->top - it->bottom;
				else
					distance = it->top - it2->bottom;
				if (distance <= maxDistance)
				{
					it->unite (*it2);
					list.erase (it2);
					joinNearbyInvalidRects (list, maxDistance);
					return;
				}
			}
			if (it->top == it2->top && it->bottom == it2->bottom)
			{
				CCoord distance;
				if (it->right < it2->left)
					distance = it2->left - it->right;
				else
					distance = it->left - it2->right;
				if (distance <= maxDistance)
				{
					it->unite (*it2);
					list.erase (it2);
					joinNearbyInvalidRects (list, maxDistance);
					return;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
} // VSTGUI
