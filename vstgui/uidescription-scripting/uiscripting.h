// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../uidescription/icontroller.h"
#include "../uidescription/iviewfactory.h"
#include "../uidescription/iuidescription.h"
#include "../uidescription/iuidescriptionaddon.h"
#include <functional>
#include <variant>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
class UIScripting : public UIDescriptionAddOnAdapter
{
public:
	using OnScriptException = std::function<void (const std::string& reason)>;

	static void init (const OnScriptException& func = {});

	~UIScripting () noexcept;

private:
	UIScripting ();

	void afterParsing (IUIDescription* desc) override;
	void beforeSaving (IUIDescription* desc) override;
	void onDestroy (IUIDescription* desc) override;
	CreateTemplateViewFunc onCreateTemplateView (const IUIDescription* desc,
												 const CreateTemplateViewFunc& f) override;
	IViewFactory* getViewFactory (IUIDescription* desc, IViewFactory* originalFactory) override;
	void onEditingStart (IUIDescription* desc) override;
	void onEditingEnd (IUIDescription* desc) override;

	struct Impl;
	std::unique_ptr<Impl> impl;

	friend std::unique_ptr<UIScripting> std::make_unique<UIScripting> ();
};

//------------------------------------------------------------------------
/** extends IController
 *
 *	The script controller extension adds script related methods to the controller.
 *
 *	It can alter the scripts for the views if needed and scripts can get and set properties.
 */
struct IScriptControllerExtension
{
	/** a property value is either an integer, double, string or undefined (nullptr_t) */
	using PropertyValue = std::variant<nullptr_t, int64_t, double, std::string>;

	/** verify the script for a view
	 *
	 *	called before the script is executed
	 *
	 *	@param view the view
	 *	@param script the script
	 *	@return optional new script. if the optional is empty the original script is used.
	 */
	virtual std::optional<std::string> verifyScript (CView* view, const std::string& script) = 0;

	/** get a property
	 *
	 *	called from a script
	 *
	 *	if the propery exists, the value should be set and the return value should be true.
	 *	Otherwise return false.
	 *
	 *	@param view the view
	 *	@param name the name of the property
	 *	@param value the property value
	 *	@return true on success.
	 */
	virtual bool getProperty (CView* view, std::string_view name, PropertyValue& value) const = 0;

	/** set a property
	 *
	 *	called from a script
	 *
	 *	@param view the view
	 *	@param name the name of the property
	 *	@param value the value of the property
	 *	@return true on success.
	 */
	virtual bool setProperty (CView* view, std::string_view name, const PropertyValue& value) = 0;
};

//------------------------------------------------------------------------
/** adapter for IScriptControllerExtension */
struct ScriptControllerExtensionAdapter : IScriptControllerExtension
{
	std::optional<std::string> verifyScript (CView*, const std::string&) override { return {}; }
	bool getProperty (CView*, std::string_view, PropertyValue&) const override { return false; }
	bool setProperty (CView*, std::string_view, const PropertyValue&) override { return false; }
};

//------------------------------------------------------------------------
} // VSTGUI
