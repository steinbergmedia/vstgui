// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../tiny-js/TinyJS.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ScriptingInternal {

//------------------------------------------------------------------------
struct ScriptAddChildScoped
{
	using CScriptVar = TJS::CScriptVar;
	using CScriptVarLink = TJS::CScriptVarLink;

	ScriptAddChildScoped (CScriptVar& var, std::string_view name, CScriptVar* obj) : var (var)
	{
		if ((link = var.findChild (name)))
		{
			oldVar = link->getVar ();
			oldVar->addRef ();
			link->setVar (obj);
		}
		else
		{
			link = var.addChild (name, obj);
		}
	}
	~ScriptAddChildScoped () noexcept
	{
		if (oldVar && link)
		{
			link->setVar (oldVar);
			oldVar->release ();
		}
		else if (link)
		{
			var.removeLink (link);
		}
	}

private:
	CScriptVar& var;
	CScriptVarLink* link {nullptr};
	CScriptVar* oldVar {nullptr};
};

//------------------------------------------------------------------------
inline TJS::CScriptVar* createJSFunction (TJS::JSCallback&& proc)
{
	auto funcVar = new TJS::CScriptVar (TJS::TINYJS_BLANK_DATA,
										TJS::SCRIPTVAR_FUNCTION | TJS::SCRIPTVAR_NATIVE);
	funcVar->setCallback (std::move (proc));
	return funcVar;
}

//------------------------------------------------------------------------
inline TJS::CScriptVar* createJSFunction (TJS::JSCallback&& proc,
										  const std::initializer_list<const char*>& argNames)
{
	auto f = createJSFunction (std::move (proc));
	for (auto name : argNames)
		f->addChildNoDup (name);
	return f;
}

//------------------------------------------------------------------------
struct ScriptObject
{
	using CScriptVar = TJS::CScriptVar;

	ScriptObject ()
	{
		scriptVar = new CScriptVar ();
		scriptVar->addRef ();
	}
	ScriptObject (ScriptObject&& o) { *this = std::move (o); }
	ScriptObject& operator= (ScriptObject&& o)
	{
		if (scriptVar)
			scriptVar->release ();
		scriptVar = nullptr;
		std::swap (scriptVar, o.scriptVar);
		return *this;
	}
	virtual ~ScriptObject () noexcept
	{
		if (scriptVar)
		{
			scriptVar->removeAllChildren ();
			scriptVar->release ();
		}
	}

	CScriptVar* getVar () const { return scriptVar; }
	CScriptVar* take ()
	{
		auto var = scriptVar;
		scriptVar = nullptr;
		return var;
	}

	void addChild (std::string_view name, ScriptObject&& obj)
	{
		auto var = obj.take ();
		scriptVar->addChild (name, var);
	}
	void addChild (std::string_view name, double d)
	{
		scriptVar->addChild (name, new CScriptVar (d));
	}
	void addChild (std::string_view name, int64_t i)
	{
		scriptVar->addChild (name, new CScriptVar (i));
	}
	void addChild (std::string_view name, int32_t i)
	{
		scriptVar->addChild (name, new CScriptVar (static_cast<int64_t> (i)));
	}
	void addChild (std::string_view name, const std::string& value)
	{
		scriptVar->addChild (name, new CScriptVar (value));
	}
	void addFunc (std::string_view name, std::function<void (CScriptVar*)>&& func)
	{
		scriptVar->addChild (name, createJSFunction (std::move (func)));
	}
	void addFunc (std::string_view name, std::function<void (CScriptVar*)>&& func,
				  const std::initializer_list<const char*>& argNames)
	{
		scriptVar->addChild (name, createJSFunction (std::move (func), argNames));
	}

protected:
	CScriptVar* scriptVar {nullptr};
};

//------------------------------------------------------------------------
} // ScriptingInternal
} // VSTGUI
