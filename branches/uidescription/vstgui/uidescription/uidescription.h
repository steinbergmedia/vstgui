
#ifndef __uidescription__
#define __uidescription__

#include "../vstgui.h"
#include "xmlparser.h"
#include <map>
#include <deque>
#include <list>
#include <string>

BEGIN_NAMESPACE_VSTGUI

class UIDescList;
class UINode;
class UIAttributes;
class IViewFactory;
class IUIDescription;

//-----------------------------------------------------------------------------
class IController : public CControlListener
{
public:
	virtual long getTagForName (const char* name) { return -1; };
	virtual CControlListener* getControlListener (const char* name) { return this; }
	virtual CView* createView (const UIAttributes& attributes, IUIDescription* description) { return 0; }
	virtual CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description) { return view; }
};

//-----------------------------------------------------------------------------
class IUIDescription
{
public:
	virtual ~IUIDescription () {}

	virtual CBitmap* getBitmap (const char* name) = 0;
	virtual CFontRef getFont (const char* name) = 0;
	virtual bool getColor (const char* name, CColor& color) = 0;
	virtual long getTagForName (const char* name) = 0;
	virtual CControlListener* getControlListener (const char* name) = 0;
	virtual IController* getController () const = 0;

	virtual const char* lookupColorName (const CColor& color) const = 0;
	virtual const char* lookupFontName (const CFontRef font) const = 0;
	virtual const char* lookupBitmapName (const CBitmap* bitmap) const = 0;
	virtual const char* lookupControlTagName (const long tag) const = 0;
};

//-----------------------------------------------------------------------------
class UIDescription : public CBaseObject, public IUIDescription, public Xml::IHandler
{
public:
	UIDescription (const CResourceDescription& xmlFile, IViewFactory* viewFactory = 0);
	UIDescription (Xml::IContentProvider* xmlContentProvider, IViewFactory* viewFactory = 0);
	~UIDescription ();

	bool parse ();
	bool save (const char* filename);
	const char* getXmFileName () const { return xmlFile.u.name; }
	
	CView* createView (const char* name, IController* controller);
	const UIAttributes* getViewAttributes (const char* name);

	CBitmap* getBitmap (const char* name);
	CFontRef getFont (const char* name);
	bool getColor (const char* name, CColor& color);
	long getTagForName (const char* name);
	CControlListener* getControlListener (const char* name);
	IController* getController () const { return controller; }
	IViewFactory* getViewFactory () const { return viewFactory; }
	
	const char* lookupColorName (const CColor& color) const;
	const char* lookupFontName (const CFontRef font) const;
	const char* lookupBitmapName (const CBitmap* bitmap) const;
	const char* lookupControlTagName (const long tag) const;
	
	void collectTemplateViewNames (std::list<const std::string*>& names) const;
	void collectColorNames (std::list<const std::string*>& names) const;
	void collectFontNames (std::list<const std::string*>& names) const;
	void collectBitmapNames (std::list<const std::string*>& names) const;
	void collectControlTagNames (std::list<const std::string*>& names) const;
	
	void changeColorName (const char* oldName, const char* newName);
	void changeTagName (const char* oldName, const char* newName);
	void changeFontName (const char* oldName, const char* newName);
	void changeBitmapName (const char* oldName, const char* newName);

	void changeColor (const char* name, const CColor& newColor);
	void changeTag (const char* name, long tag);
	void changeFont (const char* name, CFontRef newFont);
	void changeBitmap (const char* name, const char* newName);
	
	void removeColor (const char* name);
	void removeTag (const char* name);
	void removeFont (const char* name);
	void removeBitmap (const char* name);

	void updateViewDescription (const char* name, CView* view);
	bool getTemplateNameFromView (CView* view, std::string& templateName);
	void addNewTemplate (const char* name, UIAttributes* attr); // owns attributes
	bool removeTemplate (const char* name);

	bool setCustomAttributes (const char* name, UIAttributes* attr); //owns attributes
	UIAttributes* getCustomAttributes (const char* name) const;

	static bool parseColor (const std::string& colorString, CColor& color);
	static CViewAttributeID kTemplateNameAttributeID;
protected:
	CView* createViewFromNode (UINode* node, IController* controller);
	UINode* getBaseNode (const char* name) const;
	UINode* findChildNodeByNameAttribute (UINode* node, const char* nameAttribute) const;
	void updateAttributesForView (UINode* node, CView* view, bool deep = true);

	// Xml::IHandler
	void startXmlElement (Xml::Parser* parser, const char* elementName, const char** elementAttributes);
	void endXmlElement (Xml::Parser* parser, const char* name);
	void xmlCharData (Xml::Parser* parser, const char* data, int length);
	void xmlComment (Xml::Parser* parser, const char* comment);

	CResourceDescription xmlFile;
	UINode* nodes;
	IController* controller;
	IViewFactory* viewFactory;
	Xml::IContentProvider* xmlContentProvider;

	std::deque<UINode*> nodeStack;
	int parseState;
};
 
//-----------------------------------------------------------------------------
class UIAttributes : public CBaseObject, public std::map<std::string,std::string>
{
public:
	UIAttributes (const char** attributes = 0);
	~UIAttributes ();

	bool hasAttribute (const char* name) const;
	const std::string* getAttributeValue (const char* name) const;
	void setAttribute (const char* name, const char* value);
	
	void setRectAttribute (const char* name, const CRect& r);
	bool getRectAttribute (const char* name, CRect& r) const;
	
	void removeAll () { clear (); }
protected:
};

//-----------------------------------------------------------------------------
class IViewFactory
{
public:
	virtual ~IViewFactory () {}
	
	virtual CView* createView (const UIAttributes& attributes, IUIDescription* description) = 0;
	virtual bool applyAttributeValues (CView* view, const UIAttributes& attributes, IUIDescription* desc) const = 0;
};

END_NAMESPACE_VSTGUI

#endif
