
#ifndef __viewfactory__
#define __viewfactory__

#include "../vstgui.h"
#include "uidescription.h"
#include <string>
#include <list>

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
class IViewCreator
{
public:
	virtual ~IViewCreator () {}
	
	enum AttrType {
		kUnknownType,
		kBooleanType,
		kIntegerType,
		kFloatType,
		kStringType,
		kColorType,
		kFontType,
		kBitmapType,
		kPointType,
		kRectType,
		kTagType,
	};

	virtual const char* getViewName () const = 0;
	virtual const char* getBaseViewName () const = 0;
	virtual CView* create (const UIAttributes& attributes, IUIDescription* description) const = 0;
	virtual bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const = 0;
	virtual bool getAttributeNames (std::list<std::string>& attributeNames) const = 0;
	virtual AttrType getAttributeType (const std::string& attributeName) const = 0;
	virtual bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const = 0;
};

//-----------------------------------------------------------------------------
class ViewFactory : public CBaseObject, public IViewFactory
{
public:
	ViewFactory ();
	~ViewFactory ();

	// IViewFactory
	CView* createView (const UIAttributes& attributes, IUIDescription* description);
	
	static void registerViewCreator (const IViewCreator& viewCreator);

	#if VSTGUI_LIVE_EDITING
	bool getAttributeNamesForView (CView* view, std::list<std::string>& attributeNames) const;
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const;
	bool applyAttributeValues (CView* view, const UIAttributes& attributes, IUIDescription* desc) const;
	IViewCreator::AttrType getAttributeType (CView* view, const std::string& attributeName) const;
	void collectRegisteredViewNames (std::list<const std::string*>& viewNames, const char* baseClassNameFilter = 0) const;
	const char* getViewName (CView* view) const;
	bool getAttributesForView (CView* view, IUIDescription* desc, UIAttributes& attr) const;
	#endif

protected:
	CView* createViewByName (const std::string* className, const UIAttributes& attributes, IUIDescription* description);
};

END_NAMESPACE_VSTGUI

#endif

