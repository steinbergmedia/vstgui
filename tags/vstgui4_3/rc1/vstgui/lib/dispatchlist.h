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
	bool inForEach;
};

//------------------------------------------------------------------------
template <typename T>
DispatchList<T>::DispatchList ()
: inForEach (false)
{
}

//------------------------------------------------------------------------
template <typename T>
void DispatchList<T>::add (T* obj)
{
	if (inForEach)
		toAdd.push_back (obj);
	else
		entries.push_back (obj);
}

//------------------------------------------------------------------------
template <typename T>
void DispatchList<T>::remove (T* obj)
{
	if (inForEach)
		toRemove.push_back (obj);
	else
	{
		typename Array::iterator it = std::find (entries.begin (), entries.end (), obj);
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

	inForEach = true;
	for (typename Array::const_iterator it = entries.begin (), end = entries.end (); it != end; ++it)
		proc (*it);
	inForEach = false;
	for (typename Array::const_iterator it = toAdd.begin (), end = toAdd.end (); it != end; ++it)
		add (*it);
	for (typename Array::const_iterator it = toRemove.begin (), end = toRemove.end (); it != end; ++it)
		remove (*it);
	toAdd.clear ();
	toRemove.clear ();
}

//------------------------------------------------------------------------
} // VSTGUI

#endif // __dispatchlist__
