
#ifndef __uidescription__
#define __uidescription__

#include "../vstgui.h"
#include "xmlparser.h"
#include <map>
#include <deque>
#include <string>

BEGIN_NAMESPACE_VSTGUI

class UIDescList;
class UINode;
class IViewFactory;
class UIAttributes;
class UIDescription;

//-----------------------------------------------------------------------------
class IController : public CControlListener
{
public:
	virtual long getTagForName (const char* name) { return -1; };
	virtual CControlListener* getControlListener (const char* name) { return this; }
	virtual CView* createView (const UIAttributes& attributes, UIDescription* description) { return 0; }
	virtual CView* verifyView (CView* view, const UIAttributes& attributes, UIDescription* description) { return view; }
};

//-----------------------------------------------------------------------------
class UIDescription : public CBaseObject, public Xml::IHandler
{
public:
	UIDescription (const CResourceDescription& xmlFile);
	UIDescription (Xml::IContentProvider* xmlContentProvider);
	~UIDescription ();

	bool parse ();
	
	CView* createView (const char* name, IController* controller, IViewFactory* viewFactory = 0);
	const UIAttributes* getViewAttributes (const char* name);

	CBitmap* getBitmap (const char* name);
	CFontRef getFont (const char* name);
	bool getColor (const char* name, CColor& color);
	long getTagForName (const char* name);
	CControlListener* getControlListener (const char* name);
	IController* getController () const { return controller; }
protected:
	CView* createViewFromNode (UINode* node, IViewFactory* viewFactory, IController* controller);
	UINode* getBaseNode (const char* name);
	UINode* findChildNodeByNameAttribute (UINode* node, const char* nameAttribute);

	// Xml::IHandler
	void startXmlElement (Xml::Parser* parser, const char* elementName, const char** elementAttributes);
	void endXmlElement (Xml::Parser* parser, const char* name);
	void xmlCharData (Xml::Parser* parser, const char* data, int length);
	void xmlComment (Xml::Parser* parser, const char* comment);

	CResourceDescription xmlFile;
	UINode* nodes;
	IController* controller;
	Xml::IContentProvider* xmlContentProvider;

	std::deque<UINode*> nodeStack;
	int parseState;
};
 
//-----------------------------------------------------------------------------
class UIAttributes : public CBaseObject, protected std::map<std::string,std::string>
{
public:
	UIAttributes (const char** attributes);

	bool hasAttribute (const char* name) const;
	const std::string* getAttributeValue (const char* name) const;
	void setAttribute (const char* name, const char* value);
protected:
	~UIAttributes ();
};

//-----------------------------------------------------------------------------
class IViewFactory
{
public:
	virtual ~IViewFactory () {}
	
	virtual CView* createView (const UIAttributes& attributes, UIDescription* description) = 0;
};

END_NAMESPACE_VSTGUI

#endif
