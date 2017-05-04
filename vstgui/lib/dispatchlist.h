// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __dispatchlist__
#define __dispatchlist__

#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {

template <typename T>
class DispatchList
{
public:
	DispatchList ();

	void add (const T& obj);
	void add (T&& obj);
	void remove (const T& obj);
	void remove (T&& obj);
	bool empty () const;

	template <typename Procedure>
	void forEach (Procedure proc);

	template <typename Procedure>
	void forEachReverse (Procedure proc);

private:
	using Array = std::vector<T>;

	void postForEach ();

	Array entries;
	Array toRemove;
	Array toAdd;
	bool inForEach {false};
};

//------------------------------------------------------------------------
template <typename T>
inline DispatchList<T>::DispatchList ()
{
}

//------------------------------------------------------------------------
template <typename T>
inline void DispatchList<T>::add (const T& obj)
{
	if (inForEach)
		toAdd.emplace_back (obj);
	else
		entries.emplace_back (obj);
}

//------------------------------------------------------------------------
template <typename T>
inline void DispatchList<T>::add (T&& obj)
{
	if (inForEach)
		toAdd.emplace_back (std::move (obj));
	else
		entries.emplace_back (std::move (obj));
}

//------------------------------------------------------------------------
template <typename T>
inline void DispatchList<T>::remove (const T& obj)
{
	if (inForEach)
		toRemove.emplace_back (obj);
	else
	{
		auto it = std::find (entries.begin (), entries.end (), obj);
		if (it != entries.end ())
			entries.erase (it);
	}
}

//------------------------------------------------------------------------
template <typename T>
inline void DispatchList<T>::remove (T&& obj)
{
	if (inForEach)
		toRemove.emplace_back (std::move (obj));
	else
	{
		auto it = std::find (entries.begin (), entries.end (), obj);
		if (it != entries.end ())
			entries.erase (it);
	}
}

//------------------------------------------------------------------------
template <typename T>
inline bool DispatchList<T>::empty () const
{
	return entries.empty ();
}

//------------------------------------------------------------------------
template <typename T>
inline void DispatchList<T>::postForEach ()
{
	if (!toAdd.empty ())
	{
		for (auto&& it : toAdd)
			add (std::move (it));
		toAdd.clear ();
	}
	if (!toRemove.empty ())
	{
		for (auto&& it : toRemove)
			remove (std::move (it));
		toRemove.clear ();
	}
}

//------------------------------------------------------------------------
template <typename T>
template <typename Procedure>
inline void DispatchList<T>::forEach (Procedure proc)
{
	if (entries.empty ())
		return;

	bool wasInForEach = inForEach;
	inForEach = true;
	for (auto& it : entries)
		proc (it);
	inForEach = wasInForEach;
	if (!inForEach)
		postForEach ();
}

//------------------------------------------------------------------------
template <typename T>
template <typename Procedure>
inline void DispatchList<T>::forEachReverse (Procedure proc)
{
	if (entries.empty ())
		return;
	
	bool wasInForEach = inForEach;
	inForEach = true;
	for (auto it = entries.rbegin (); it != entries.rend (); ++it)
		proc (*it);
	inForEach = wasInForEach;
	if (!inForEach)
		postForEach ();
}

//------------------------------------------------------------------------
} // VSTGUI

#endif // __dispatchlist__
