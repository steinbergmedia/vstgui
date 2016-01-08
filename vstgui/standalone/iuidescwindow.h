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
class ICustomization : public Interface
{
public:
	/** Create a sub controller
	 *
	 *	A sub controller can be defined in the UI editor for a view and will be responsible
	 *	as a controller for the view and its children.
	 *
	 *	The controller will be automatically destroyed when the view is destroyed. You should
	 *	always create a new controller instance here and do not cache it.
	 *
	 *	@param name name of the sub controller
	 *	@param parent the parent controller
	 *	@param uiDesc the UIDescription instance
	 */
	virtual IController* createController (const UTF8StringView& name, IController* parent,
	                                       const IUIDescription* uiDesc) = 0;
};

//------------------------------------------------------------------------
/** Configuration for a UIDescription window */
struct Config
{
	/** Filename of the UIDescription xml file */
	UTF8String uiDescFileName;
	/** view to show in the window */
	UTF8String viewName;
	/** window configuration */
	WindowConfiguration windowConfig;
	/** model binding */
	ModelBindingPtr modelBinding;
	/** optional UI customization */
	CustomizationPtr customization;
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
