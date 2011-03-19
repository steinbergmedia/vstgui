//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2011, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
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

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
/** @brief simple dependency between objects.

	You can inject this implementation into CBaseObjects whenever you need other CBaseObjects to be informed about
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
//----------------------------------------------------------------------------------------------------
protected:
	IDependency () {}
	virtual ~IDependency ();
	std::list<CBaseObject*> dependents;
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
	CBaseObject* This = dynamic_cast<CBaseObject*> (this);
	for (std::list<CBaseObject*>::iterator it = dependents.begin (); it != dependents.end (); it++)
		(*it)->notify (This, message);
}

//----------------------------------------------------------------------------------------------------
inline IDependency::~IDependency ()
{
#if DEBUG
	if (dependents.size () != 0)
	{
		DebugPrint ("IDependency has depentent objects on destruction.\n");
	}
#endif
}

}
#endif
