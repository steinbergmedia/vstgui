
#ifndef __viewfactory__
#define __viewfactory__

#include "../vstgui.h"
#include "uidescription.h"
#include <string>

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
typedef CView* (*ViewCreateFunction)(const UIAttributes& attributes, IViewFactory* factory, UIDescription* description);
typedef bool (*ViewApplyAttributesFunction)(CView* view, const UIAttributes& attributes, IViewFactory* factory, UIDescription* description);

//-----------------------------------------------------------------------------
class ViewFactory : public CBaseObject, public IViewFactory
{
public:
	ViewFactory ();
	~ViewFactory ();

	// IViewFactory
	CView* createView (const UIAttributes& attributes, UIDescription* description);
	
	static void registerViewCreateFunction (const char* className, const char* baseClass, ViewCreateFunction createFunction, ViewApplyAttributesFunction applyAttributesFunction);

protected:
	CView* createViewByName (const std::string* className, const UIAttributes& attributes, UIDescription* description);
};

//-----------------------------------------------------------------------------
#define REGISTER_VIEW_CREATOR(id, className, baseClass, createFunction, applyFunction) \
	class RegisterViewCreator_##id { \
		public: RegisterViewCreator_##id () { ViewFactory::registerViewCreateFunction (className, baseClass, createFunction, applyFunction); } \
	}; \
	static RegisterViewCreator_##id __gRegisterViewCreator_##id; \
	RegisterViewCreator_##id* ___RegisterViewCreator_##id = &__gRegisterViewCreator_##id;

END_NAMESPACE_VSTGUI

#endif

