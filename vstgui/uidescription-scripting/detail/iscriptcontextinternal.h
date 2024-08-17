// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../uiscripting.h"
#include <string>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ScriptingInternal {

//------------------------------------------------------------------------
struct IScriptContextInternal : public IScriptContext
{
	virtual void onViewCreated (CView* view, const std::string& script) = 0;
};

//------------------------------------------------------------------------
} // ScriptingInternal
} // VSTGUI
