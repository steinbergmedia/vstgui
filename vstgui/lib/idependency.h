// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __idependency__
#define __idependency__

#include "vstguibase.h"
#include "vstguidebug.h"
#include <list>
#include <set>
#include <algorithm>
#include <cassert>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
/** @brief simple dependency between objects.
	@ingroup new_in_4_0

	@details You can inject this implementation into CBaseObjects whenever you need other CBaseObjects to be informed about
	changes to that class instance. Note that you need to handle recursions yourself and that no reference counting is done
	and that you must make sure that the dependent objects are alife while added as dependent.
*/
//----------------------------------------------------------------------------------------------------
class IDependency
{
public:
	/** add a dependent object */
	virtual void addDependency (CBaseObject* obj);
	/** remove a dependent object. */
	virtual void removeDependency (CBaseObject* obj);
	/** notify dependent objects of change with message. */
	virtual void changed (IdStringPtr message);
	/** defer changes until later. can be nested. If you use this, you must make sure that all message pointers are valid the whole time. */
	virtual void deferChanges (bool state);

	/** helper class to defer changes until instance is destroyed. */
	class DeferChanges
	{
	public:
		explicit DeferChanges (IDependency* dep) : dep (dep) { dep->deferChanges (true); }
		~DeferChanges () noexcept { dep->deferChanges (false); }
	protected:
		IDependency* dep;
	};

//----------------------------------------------------------------------------------------------------
protected:
	IDependency () = default;
	virtual ~IDependency () noexcept;

	static void rememberObject (CBaseObject* obj) { obj->remember (); }
	static void forgetObject (CBaseObject* obj) { obj->forget (); }

	using DeferedChangesSet = std::set<IdStringPtr>;
	using DependentList = std::list<CBaseObject*>;

	int32_t deferChangeCount {0};
	DeferedChangesSet deferedChanges;
	DependentList dependents;
};

//----------------------------------------------------------------------------------------------------
inline void IDependency::addDependency (CBaseObject* obj)
{
	dependents.emplace_back (obj);
}

//----------------------------------------------------------------------------------------------------
inline void IDependency::removeDependency (CBaseObject* obj)
{
	dependents.remove (obj);
}

//----------------------------------------------------------------------------------------------------
inline void IDependency::changed (IdStringPtr message)
{
	if (deferChangeCount)
	{
		deferedChanges.emplace (message);
	}
	else if (dependents.empty () == false)
	{
		CBaseObject* This = dynamic_cast<CBaseObject*> (this);
		DependentList localList (dependents);
		std::for_each (localList.begin (), localList.end (), rememberObject);
		for (auto& obj : localList)
			obj->notify (This, message);
		std::for_each (localList.begin (), localList.end (), forgetObject);
	}
}

//----------------------------------------------------------------------------------------------------
inline void IDependency::deferChanges (bool state)
{
	if (state)
	{
		deferChangeCount++;
	}
	else if (--deferChangeCount == 0)
	{
		for (auto& msg : deferedChanges)
			changed (msg);
		deferedChanges.clear ();
	}
}

//----------------------------------------------------------------------------------------------------
inline IDependency::~IDependency () noexcept
{
	vstgui_assert (dependents.size () == 0);
}

} // namespace

#endif
