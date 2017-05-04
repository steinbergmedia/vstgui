// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uiviewfactory__
#define __uiviewfactory__

#include "../lib/vstguifwd.h"
#include "iuidescription.h"
#include "iviewfactory.h"
#include "iviewcreator.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
/// @brief Default view factory
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class UIViewFactory : public NonAtomicReferenceCounted, public IViewFactory
{
public:
	UIViewFactory ();
	~UIViewFactory () noexcept override = default;

	// IViewFactory
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) const override;
	bool applyAttributeValues (CView* view, const UIAttributes& attributes, const IUIDescription* desc) const override;
	IdStringPtr getViewName (CView* view) const override;
	bool applyCustomViewAttributeValues (CView* customView, IdStringPtr baseViewName, const UIAttributes& attributes, const IUIDescription* desc) const override;
	
	static void registerViewCreator (const IViewCreator& viewCreator);
	static void unregisterViewCreator (const IViewCreator& viewCreator);

#if VSTGUI_LIVE_EDITING
	using StringPtrList = std::list<const std::string*>;
	using StringList = std::list<std::string>;
	using ViewAndDisplayNameList = std::list<std::pair<const std::string*, const std::string>>;
	
	bool getAttributeNamesForView (CView* view, StringList& attributeNames) const;
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const;
	IViewCreator::AttrType getAttributeType (CView* view, const std::string& attributeName) const;
	void collectRegisteredViewNames (StringPtrList& viewNames, IdStringPtr baseClassNameFilter = nullptr) const;
	bool getAttributesForView (CView* view, const IUIDescription* desc, UIAttributes& attr) const;
	// list type support
	bool getPossibleAttributeListValues (CView* view, const std::string& attributeName, StringPtrList& values) const;
	bool getAttributeValueRange (CView* view, const std::string& attributeName, double& minValue, double& maxValue) const;

	ViewAndDisplayNameList collectRegisteredViewAndDisplayNames () const;

#if ENABLE_UNIT_TESTS
	bool disableRememberAttributes {false};
#endif
#endif

protected:
	void evaluateAttributesAndRemember (CView* view, const UIAttributes& attributes, UIAttributes& evaluatedAttributes, const IUIDescription* description) const;
	CView* createViewByName (const std::string* className, const UIAttributes& attributes, const IUIDescription* description) const;

#if VSTGUI_LIVE_EDITING
	static size_t createHash (const std::string& str);
	void rememberAttribute (CView* view, IdStringPtr attrName, const std::string& value) const;
	bool getRememberedAttribute (CView* view, IdStringPtr attrName, std::string& value) const;
#endif
};

} // namespace

#endif // __uiviewfactory__

