// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "scriptobject.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ScriptingInternal {

//------------------------------------------------------------------------
struct UIDescScriptObject : ScriptObject
{
	using StringList = std::list<const std::string*>;
	using CScriptException = TJS::CScriptException;

	UIDescScriptObject () = default;
	UIDescScriptObject (IUIDescription* desc, TJS::CTinyJS* scriptContext)
	{
		using namespace std::literals;

		addFunc ("colorNames"sv, [desc] (CScriptVar* var) {
			StringList names;
			desc->collectColorNames (names);
			var->setReturnVar (createArrayFromNames (names));
		});
		addFunc ("fontNames"sv, [desc] (CScriptVar* var) {
			StringList names;
			desc->collectFontNames (names);
			var->setReturnVar (createArrayFromNames (names));
		});
		addFunc ("bitmapNames"sv, [desc] (CScriptVar* var) {
			StringList names;
			desc->collectBitmapNames (names);
			var->setReturnVar (createArrayFromNames (names));
		});
		addFunc ("gradientNames"sv, [desc] (CScriptVar* var) {
			StringList names;
			desc->collectGradientNames (names);
			var->setReturnVar (createArrayFromNames (names));
		});
		addFunc ("controlTagNames"sv, [desc] (CScriptVar* var) {
			StringList names;
			desc->collectControlTagNames (names);
			var->setReturnVar (createArrayFromNames (names));
		});
		addFunc ("getTagForName"sv,
				 [desc] (CScriptVar* var) {
					 auto param = var->getParameter ("name"sv);
					 if (!param)
					 {
						 throw CScriptException ("Expect 'name' argument for getTagForName ");
					 }
					 std::string name = param->getString ();
					 auto tag = desc->getTagForName (name.data ());
					 var->setReturnVar (new CScriptVar (static_cast<int64_t> (tag)));
				 },
				 {"name"});
		addFunc ("lookupTagName"sv,
				 [desc] (CScriptVar* var) {
					 auto param = var->getParameter ("tag"sv);
					 if (!param)
					 {
						 throw CScriptException ("Expect 'tag' argument for lookupTagName ");
					 }
					 if (!param->isInt ())
					 {
						 throw CScriptException ("Expect 'tag' argument to be an integer ");
					 }
					 if (auto tagName =
							 desc->lookupControlTagName (static_cast<int32_t> (param->getInt ())))
					 {
						 var->setReturnVar (new CScriptVar (std::string (tagName)));
					 }
				 },
				 {"tag"});
	}

	static CScriptVar* createArrayFromNames (const StringList& names)
	{
		auto array = new CScriptVar ();
		array->setArray ();
		int index = 0;
		for (auto name : names)
		{
			array->addChild (std::to_string (index), new CScriptVar (*name));
			++index;
		}
		return array;
	}
};

//------------------------------------------------------------------------
} // ScriptingInternal
} // VSTGUI
