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
										  const std::initializer_list<std::string_view>& argNames)
{
	auto f = createJSFunction (std::move (proc));
	for (auto name : argNames)
		f->addChildNoDup (name);
	return f;
}

//------------------------------------------------------------------------
inline TJS::CScriptVar* getArgument (TJS::CScriptVar* var, std::string_view argName,
									 std::string_view funcSignature)
{
	auto child = var->findChild (argName);
	auto result = child ? child->getVar () : nullptr;
	if (!result || result->isUndefined ())
	{
		TJS::string s ("Missing `");
		s.append (argName);
		s.append ("` argument in ");
		s.append (funcSignature);
		throw TJS::CScriptException (std::move (s));
	}
	return result;
}

//------------------------------------------------------------------------
inline TJS::CScriptVar* getOptionalArgument (TJS::CScriptVar* var, std::string_view argName)
{
	if (auto child = var->findChild (argName))
		return child->getVar ();
	return nullptr;
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
	ScriptObject (CScriptVar* var) : scriptVar (var) {}
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
			scriptVar->release ();
		}
	}

	CScriptVar* getVar () const { return scriptVar; }
	CScriptVar* take ()
	{
		auto v = scriptVar;
		scriptVar = nullptr;
		return v;
	}

	void addChild (std::string_view name, ScriptObject&& obj)
	{
		scriptVar->addChild (name, obj.getVar ());
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
	void addChild (std::string_view name, std::string_view value)
	{
		scriptVar->addChild (name, new CScriptVar (TJS::string {value.data (), value.size ()}));
	}
	void addFunc (std::string_view name, std::function<void (CScriptVar*)>&& func)
	{
		scriptVar->addChild (name, createJSFunction (std::move (func)));
	}
	void addFunc (std::string_view name, std::function<void (CScriptVar*)>&& func,
				  const std::initializer_list<std::string_view>& argNames)
	{
		scriptVar->addChild (name, createJSFunction (std::move (func), argNames));
	}

protected:
	CScriptVar* scriptVar {nullptr};
};

//------------------------------------------------------------------------
} // ScriptingInternal
} // VSTGUI
