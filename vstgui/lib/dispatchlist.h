// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <vector>
#include <algorithm>

//------------------------------------------------------------------------
namespace VSTGUI {

template<typename T>
class DispatchList
{
public:
	DispatchList () = default;

	void add (const T& obj);
	void add (T&& obj);
	void remove (const T& obj);
	void remove (T&& obj);
	bool empty () const;

	template<typename Procedure>
	void forEach (Procedure proc);

	template<typename Procedure, typename Condition>
	void forEach (Procedure proc, Condition condition);

	template<typename Procedure>
	void forEachReverse (Procedure proc);

	template<typename Procedure, typename Condition>
	void forEachReverse (Procedure proc, Condition condition);

private:
	using Array = std::vector<std::pair<bool, T>>;
	using AddArray = std::vector<T>;

	void postForEach ();

	Array entries;
	AddArray toAdd;
	bool inForEach{false};
};

//------------------------------------------------------------------------
template <typename T, typename ListenerInterface>
struct ListenerProvider
{
//------------------------------------------------------------------------
	using List = DispatchList<ListenerInterface*>;

	void registerListener (ListenerInterface* listener) { listeners.add (listener); }
	void unregisterListener (ListenerInterface* listener) { listeners.remove (listener); }

	template<typename Proc>
	void forEachListener (Proc proc)
	{
		listeners.forEach (
		    [&] (ListenerInterface* listener) { proc (listener); });
	}

	List& getListeners () { return listeners; }
	const List& getListeners () const { return listeners; }
private:
	List listeners;
};

//------------------------------------------------------------------------
template<typename T>
inline void DispatchList<T>::add (const T& obj)
{
	if (inForEach)
		toAdd.emplace_back (obj);
	else
		entries.emplace_back (std::make_pair (true, obj));
}

//------------------------------------------------------------------------
template<typename T>
inline void DispatchList<T>::add (T&& obj)
{
	if (inForEach)
		toAdd.emplace_back (std::move (obj));
	else
		entries.emplace_back (std::make_pair (true, std::move (obj)));
}

//------------------------------------------------------------------------
template<typename T>
inline void DispatchList<T>::remove (const T& obj)
{
	auto it = std::find_if (
		entries.begin (), entries.end (),
		[&](const typename Array::value_type& element) { return element.second == obj; });
	if (it != entries.end ())
	{
		if (inForEach)
			it->first = false;
		else
			entries.erase (it);
	}
}

//------------------------------------------------------------------------
template<typename T>
inline void DispatchList<T>::remove (T&& obj)
{
	auto it = std::find (
		entries.begin (), entries.end (),
		[&](const typename Array::value_type& element) { return element.second == obj; });
	if (it != entries.end ())
	{
		if (inForEach)
			it->first = false;
		else
			entries.erase (it);
	}
}

//------------------------------------------------------------------------
template<typename T>
inline bool DispatchList<T>::empty () const
{
	return entries.empty ();
}

//------------------------------------------------------------------------
template<typename T>
inline void DispatchList<T>::postForEach ()
{
	AddArray toRemove;
	for (auto& element : entries)
	{
		if (element.first)
			continue;
		toRemove.emplace_back (std::move (element.second));
	}
	if (!toRemove.empty ())
	{
		entries.erase (std::remove_if (entries.begin (), entries.end (),
		                               [] (const typename Array::value_type& element) {
			                               return element.first == false;
		                               }),
		               entries.end ());
	}
	if (!toAdd.empty ())
	{
		AddArray tmp;
		toAdd.swap (tmp);
		for (auto&& it : tmp)
			add (std::move (it));
	}
}

//------------------------------------------------------------------------
template<typename T>
template<typename Procedure>
inline void DispatchList<T>::forEach (Procedure proc)
{
	if (entries.empty ())
		return;

	bool wasInForEach = inForEach;
	inForEach = true;
	for (auto& it : entries)
	{
		if (it.first == false)
			continue;
		proc (it.second);
	}
	inForEach = wasInForEach;
	if (!inForEach)
		postForEach ();
}

//------------------------------------------------------------------------
template<typename T>
template<typename Procedure>
inline void DispatchList<T>::forEachReverse (Procedure proc)
{
	if (entries.empty ())
		return;
	
	bool wasInForEach = inForEach;
	inForEach = true;
	for (auto it = entries.rbegin (); it != entries.rend (); ++it)
	{
		if ((*it).first == false)
			continue;
		proc ((*it).second);
	}
	inForEach = wasInForEach;
	if (!inForEach)
		postForEach ();
}

//------------------------------------------------------------------------
template<typename T>
template<typename Procedure, typename Condition>
inline void DispatchList<T>::forEach (Procedure proc, Condition condition)
{
	if (entries.empty ())
		return;

	bool wasInForEach = inForEach;
	inForEach = true;
	for (auto& it : entries)
	{
		if (it.first == false)
			continue;
		if (condition (proc (it.second)))
			break;
	}
	inForEach = wasInForEach;
	if (!inForEach)
		postForEach ();
}

//------------------------------------------------------------------------
template<typename T>
template<typename Procedure, typename Condition>
inline void DispatchList<T>::forEachReverse (Procedure proc, Condition condition)
{
	if (entries.empty ())
		return;

	bool wasInForEach = inForEach;
	inForEach = true;
	for (auto it = entries.rbegin (); it != entries.rend (); ++it)
	{
		if ((*it).first == false)
			continue;
		if (condition (proc ((*it).second)))
			break;
	}
	inForEach = wasInForEach;
	if (!inForEach)
		postForEach ();
}

//------------------------------------------------------------------------
} // VSTGUI
