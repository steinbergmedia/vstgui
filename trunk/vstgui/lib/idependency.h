//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

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
		DeferChanges (IDependency* dep) : dep (dep) { dep->deferChanges (true); }
		~DeferChanges () { dep->deferChanges (false); }
	protected:
		IDependency* dep;
	};

//----------------------------------------------------------------------------------------------------
protected:
	IDependency ();
	virtual ~IDependency ();

	static void rememberObject (CBaseObject* obj) { obj->remember (); }
	static void forgetObject (CBaseObject* obj) { obj->forget (); }

	int32_t deferChangeCount;
	typedef std::set<IdStringPtr> DeferedChangesSet;
	DeferedChangesSet deferedChanges;
	typedef std::list<CBaseObject*> DependentList;
	DependentList dependents;
};

//----------------------------------------------------------------------------------------------------
inline void IDependency::addDependency (CBaseObject* obj)
{
	dependents.push_back (obj);
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
		deferedChanges.insert (message);
	}
	else if (dependents.empty () == false)
	{
		CBaseObject* This = dynamic_cast<CBaseObject*> (this);
		DependentList localList (dependents);
		std::for_each (localList.begin (), localList.end (), rememberObject);
		for (DependentList::const_iterator it = localList.begin (); it != localList.end (); it++)
		{
			CBaseObject* obj = (*it);
			obj->notify (This, message);
		}
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
		for (DeferedChangesSet::const_iterator it = deferedChanges.begin (); it != deferedChanges.end (); it++)
			changed (*it);
		deferedChanges.clear ();
	}
}

//----------------------------------------------------------------------------------------------------
inline IDependency::IDependency ()
: deferChangeCount (0)
{
}

//----------------------------------------------------------------------------------------------------
inline IDependency::~IDependency ()
{
	vstgui_assert (dependents.size () == 0);
}

} // namespace

#endif
