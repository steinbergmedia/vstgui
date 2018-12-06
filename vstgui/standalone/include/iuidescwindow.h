// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../uidescription/uidescriptionfwd.h"
#include "fwd.h"
#include "icommand.h"
#include "ivalue.h"
#include "iwindow.h"
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace UIDesc {

//------------------------------------------------------------------------
/** Model binding interface
 *
 *	Make values available in the UIDescription window to be able to bind to controls.
 *
 *	@ingroup standalone
 */
class IModelBinding : public Interface
{
public:
	using ValueList = std::vector<ValuePtr>;

	virtual const ValueList& getValues () const = 0;
};

//------------------------------------------------------------------------
/** UIDesc window customization interface
 *
 *	@ingroup standalone
 */
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
	/** Notification that the UIDescription was sucessfully parsed
	 *
	 *	This can be used to get some resources from the UIDescription instance.
	 *	@param uiDesc the UIDescription instance
	 */
	virtual void onUIDescriptionParsed (const IUIDescription* uiDesc) = 0;
};

//------------------------------------------------------------------------
/** Configuration for an UIDescription window
 *
 *	@ingroup standalone
 */
struct Config
{
	/** Filename of the UIDescription xml file */
	UTF8String uiDescFileName;

	/** Template name of the view in the uidesc file to show in the window */
	UTF8String viewName;

	/** Window configuration */
	WindowConfiguration windowConfig;

	/** Model binding
	 *
	 *	Additioanlly to the IModelBinding features, if this object implements the ICommandHandler
	 *	interface all commands send to the window will be dispatched to this object.
	 *
	 */
	ModelBindingPtr modelBinding;

	/** %Optional UI customization
	 *
	 *	Additionally to the ICustomization features, if this object implements the IWindowController
	 *	interface, all window controller functions will be dispatched to this object.
	 *
	 */
	CustomizationPtr customization;
};

//------------------------------------------------------------------------
/** Create a window with an UIDescription
 *
 *	@param config window configuration
 *	@see Config
 *
 *	@ingroup standalone
 */
WindowPtr makeWindow (const Config& config);

//------------------------------------------------------------------------
} // UIDesc
} // Standalone
} // VSTGUI
