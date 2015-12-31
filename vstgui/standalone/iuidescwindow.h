#pragma once

#include "fwd.h"
#include "iwindow.h"
#include "icommand.h"
#include "ivalue.h"
#include "../uidescription/uidescriptionfwd.h"
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace UIDesc {

//------------------------------------------------------------------------
class IModelBinding : public Interface
{
public:
	using ValueList = std::vector<ValuePtr>;
	
	virtual const ValueList& getValues () const = 0;
};

//------------------------------------------------------------------------
struct Config
{
	UTF8String uiDescFileName;
	UTF8String viewName;
	ModelBindingPtr modelBinding;
	WindowConfiguration windowConfig;
};

//------------------------------------------------------------------------
WindowPtr makeWindow (const Config& config);

//------------------------------------------------------------------------
class ValueList : public IModelBinding
{
public:
	const IModelBinding::ValueList& getValues () const { return list; }

	void addValue (const ValuePtr& value) { list.push_back (value); }
private:
	IModelBinding::ValueList list;
};

//------------------------------------------------------------------------
} // UIDesc
} // Standalone
} // VSTGUI
