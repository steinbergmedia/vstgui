// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "scriptobject.h"
#include "../../lib/vstguifwd.h"
#include "../../uidescription/iuidescription.h"

#include <unordered_map>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ScriptingInternal {

struct IViewScriptObjectContext;

//------------------------------------------------------------------------
struct ViewScriptObject : ScriptObject,
						  TJS::IScriptVarLifeTimeObserver
{
	ViewScriptObject (CView* view, IViewScriptObjectContext* context);
	~ViewScriptObject () noexcept;

	IViewScriptObjectContext* getContext () const { return context; }

	void onDestroy (CScriptVar* v) override;

private:
	CView* view {nullptr};
	IViewScriptObjectContext* context {nullptr};
};

using ViewScriptMap = std::unordered_map<CView*, std::unique_ptr<ViewScriptObject>>;

//------------------------------------------------------------------------
struct IViewScriptObjectContext
{
	virtual ~IViewScriptObjectContext () = default;

	virtual IUIDescription* getUIDescription () const = 0;
	virtual ViewScriptObject* addView (CView* view) = 0;
	virtual ViewScriptMap::iterator removeView (CView* view) = 0;
	virtual bool evalScript (std::string_view script) noexcept = 0;
	virtual TJS::CScriptVar* getRoot () const = 0;
};

//------------------------------------------------------------------------
} // ScriptingInternal
} // VSTGUI
