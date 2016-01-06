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
/** Model binding interface
 *
 *	Make values available in the UIDescription window to be able to bind to controls.
 */
class IModelBinding : public Interface
{
public:
	using ValueList = std::vector<ValuePtr>;
	
	virtual const ValueList& getValues () const = 0;
};

//------------------------------------------------------------------------
/** Configuration for a UIDescription window */
struct Config
{
	/** Filename of the UIDescription xml file */
	UTF8String uiDescFileName;
	/** view to show in the window */
	UTF8String viewName;
	/** model binding */
	ModelBindingPtr modelBinding;
	/** window configuration */
	WindowConfiguration windowConfig;
};

//------------------------------------------------------------------------
/** Create a window with an UIDescription
 *
 *	@param config window configuration
 *	@see Config
 */
WindowPtr makeWindow (const Config& config);

//------------------------------------------------------------------------
} // UIDesc
} // Standalone
} // VSTGUI
