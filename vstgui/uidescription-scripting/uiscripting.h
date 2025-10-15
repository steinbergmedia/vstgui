// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../uidescription/icontroller.h"
#include "../uidescription/iviewfactory.h"
#include "../uidescription/iuidescription.h"
#include "../uidescription/iuidescriptionaddon.h"
#include <functional>
#include <optional>
#include <variant>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
/** UIDescription scripting support
 *
 *	@ingroup new_in_4_15
 */
class UIScripting : public UIDescriptionAddOnAdapter
{
public:
	using OnScriptExceptionFunc = std::function<void (std::string_view reason)>;
	using ReadScriptContentsFunc = std::function<std::string (std::string_view filename)>;

	/** Initialize the UIScripting library
	 *
	 *	Must be called once before creating UIDescription objects
	 *
	 *	@param onExceptionFunc			[optional] Called when a script context throws an exception
	 *	@param readScriptContentsFunc	[optional] Called to load the script from a filename,
	 *											   uses the resource folder as default.
	 */
	static void init (const OnScriptExceptionFunc& onExceptionFunc = {},
					  const ReadScriptContentsFunc& readScriptContentsFunc = {});

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
/** Script context interface
 *
 *	@ingroup new_in_4_15
 */
struct IScriptContext
{
	virtual ~IScriptContext () = default;

	/** Evaluate custom code in the script context
	 *
	 *	@param script The script to execute
	 *	@return Result object as json string
	 */
	virtual std::string eval (std::string_view script) const = 0;
};

//------------------------------------------------------------------------
/** Extends IController
 *
 *	The script controller extension adds script related methods to the controller.
 *
 *	It can alter the scripts for the views if needed and scripts can get and set properties.
 *
 *	@ingroup new_in_4_15
 */
struct IScriptControllerExtension
{
	/** A property value is either an integer, double, string or undefined (nullptr_t) */
	using PropertyValue = std::variant<std::nullptr_t, int64_t, double, std::string>;

	/** Verify the script for a view
	 *
	 *	called before the script is executed
	 *
	 *	@param view The view
	 *	@param script The script
	 *	@param context The script context where the script is executed in
	 *	@return Optional new script. If the optional is empty the original script is used.
	 */
	virtual std::optional<std::string> verifyScript (CView* view, const std::string& script,
													 const IScriptContext* context) = 0;

	/** Notification that the script context is destroyed
	 *
	 *	don't call the context anymore after this call
	 *
	 *	@param context The context which is destroyed
	 */
	virtual void scriptContextDestroyed (const IScriptContext* context) = 0;

	/** Get a property
	 *
	 *	called from a script
	 *
	 *	if the propery exists, the value should be set and the return value should be true.
	 *	Otherwise return false.
	 *
	 *	@param view The view
	 *	@param name The name of the property
	 *	@param value The property value
	 *	@return True on success.
	 */
	virtual bool getProperty (CView* view, std::string_view name, PropertyValue& value) const = 0;

	/** Set a property
	 *
	 *	called from a script
	 *
	 *	@param view The view
	 *	@param name The name of the property
	 *	@param value The value of the property
	 *	@return True on success.
	 */
	virtual bool setProperty (CView* view, std::string_view name, const PropertyValue& value) = 0;
};

//------------------------------------------------------------------------
/** Adapter for IScriptControllerExtension */
struct ScriptControllerExtensionAdapter : IScriptControllerExtension
{
	std::optional<std::string> verifyScript (CView*, const std::string&,
											 const IScriptContext*) override
	{
		return {};
	}
	void scriptContextDestroyed (const IScriptContext* context) override {}
	bool getProperty (CView*, std::string_view, PropertyValue&) const override { return false; }
	bool setProperty (CView*, std::string_view, const PropertyValue&) override { return false; }
};

//------------------------------------------------------------------------
} // VSTGUI
