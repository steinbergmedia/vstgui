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
	UIDescScriptObject (WeakPointer<IUIDescription> desc, TJS::CTinyJS* scriptContext)
	{
		using namespace std::literals;

		addFunc ("colorNames"sv, [desc] (CScriptVar* var) {
			StringList names;
			if (auto descObj = desc.lock ())
				descObj->collectColorNames (names);
			var->setReturnVar (createArrayFromNames (names));
		});
		addFunc ("fontNames"sv, [desc] (CScriptVar* var) {
			StringList names;
			if (auto descObj = desc.lock ())
				descObj->collectFontNames (names);
			var->setReturnVar (createArrayFromNames (names));
		});
		addFunc ("bitmapNames"sv, [desc] (CScriptVar* var) {
			StringList names;
			if (auto descObj = desc.lock ())
				descObj->collectBitmapNames (names);
			var->setReturnVar (createArrayFromNames (names));
		});
		addFunc ("gradientNames"sv, [desc] (CScriptVar* var) {
			StringList names;
			if (auto descObj = desc.lock ())
				descObj->collectGradientNames (names);
			var->setReturnVar (createArrayFromNames (names));
		});
		addFunc ("controlTagNames"sv, [desc] (CScriptVar* var) {
			StringList names;
			if (auto descObj = desc.lock ())
				descObj->collectControlTagNames (names);
			var->setReturnVar (createArrayFromNames (names));
		});
		addFunc ("getTagForName"sv,
				 [desc] (CScriptVar* var) {
					 auto param = var->getParameter ("name"sv);
					 if (!param)
					 {
						 throw CScriptException ("Expect 'name' argument for getTagForName ");
					 }
					 if (auto descObj = desc.lock ())
					 {
						 auto name = param->getString ();
						 auto tag = descObj->getTagForName (name.data ());
						 var->setReturnVar (new CScriptVar (static_cast<int64_t> (tag)));
					 }
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
					 if (auto descObj = desc.lock ())
					 {
						 if (auto tagName = descObj->lookupControlTagName (
								 static_cast<int32_t> (param->getInt ())))
						 {
							 var->setReturnVar (new CScriptVar (std::string (tagName)));
						 }
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
