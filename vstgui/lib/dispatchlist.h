//
//  Header.h
//  vstgui
//
//  Created by Arne Scheffler on 19.12.14.
//
//

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

	void add (T* obj);
	void remove (T* obj);
	bool empty () const;

	template <typename Procedure>
	void forEach (Procedure proc);

private:
	typedef std::vector<T*> Array;

	Array entries;
	Array toRemove;
	Array toAdd;
	bool inForEach {false};
};

//------------------------------------------------------------------------
template <typename T>
DispatchList<T>::DispatchList ()
{
}

//------------------------------------------------------------------------
template <typename T>
void DispatchList<T>::add (T* obj)
{
	if (inForEach)
		toAdd.emplace_back (obj);
	else
		entries.emplace_back (obj);
}

//------------------------------------------------------------------------
template <typename T>
void DispatchList<T>::remove (T* obj)
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
bool DispatchList<T>::empty () const
{
	return entries.empty ();
}

//------------------------------------------------------------------------
template <typename T>
template <typename Procedure>
void DispatchList<T>::forEach (Procedure proc)
{
	if (entries.empty ())
		return;

	bool wasInForEach = inForEach;
	inForEach = true;
	for (auto& it : entries)
		proc (it);
	inForEach = wasInForEach;
	if (!inForEach)
	{
		for (auto& it : toAdd)
			add (it);
		for (auto& it : toRemove)
			remove (it);
		toAdd.clear ();
		toRemove.clear ();
	}
}

//------------------------------------------------------------------------
} // VSTGUI

#endif // __dispatchlist__
