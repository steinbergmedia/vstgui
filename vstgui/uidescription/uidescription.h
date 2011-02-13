//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2009, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef __uidescription__
#define __uidescription__

#include "../lib/controls/ccontrol.h"
#include "../lib/ccolor.h"
#include "../lib/cfont.h"
#include "../lib/cbitmap.h"
#include "xmlparser.h"
#include <map>
#include <deque>
#include <list>
#include <string>

namespace VSTGUI {

class UIDescList;
class UINode;
class UIAttributes;
class IViewFactory;
class IUIDescription;
class InputStream;
class OutputStream;

//-----------------------------------------------------------------------------
//! @brief extension to CControlListener used by UIDescription [new in 4.0]
//! @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class IController : public CControlListener
{
public:
	virtual int32_t getTagForName (UTF8StringPtr name, int32_t registeredTag) { return registeredTag; };
	virtual CControlListener* getControlListener (UTF8StringPtr controlTagName) { return this; }
	virtual CView* createView (const UIAttributes& attributes, IUIDescription* description) { return 0; }
	virtual CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description) { return view; }
};

//-----------------------------------------------------------------------------
class IUIDescription
{
public:
	virtual ~IUIDescription () {}

	virtual CBitmap* getBitmap (UTF8StringPtr name) = 0;
	virtual CFontRef getFont (UTF8StringPtr name) = 0;
	virtual bool getColor (UTF8StringPtr name, CColor& color) = 0;
	virtual int32_t getTagForName (UTF8StringPtr name) = 0;
	virtual CControlListener* getControlListener (UTF8StringPtr name) = 0;
	virtual IController* getController () const = 0;

	virtual UTF8StringPtr lookupColorName (const CColor& color) const = 0;
	virtual UTF8StringPtr lookupFontName (const CFontRef font) const = 0;
	virtual UTF8StringPtr lookupBitmapName (const CBitmap* bitmap) const = 0;
	virtual UTF8StringPtr lookupControlTagName (const int32_t tag) const = 0;
};

//-----------------------------------------------------------------------------
//! @brief XML description parser and view creator [new in 4.0]
//! @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class UIDescription : public CBaseObject, public IUIDescription, public Xml::IHandler
{
public:
	UIDescription (const CResourceDescription& xmlFile, IViewFactory* viewFactory = 0);
	UIDescription (Xml::IContentProvider* xmlContentProvider, IViewFactory* viewFactory = 0);
	~UIDescription ();

	virtual bool parse ();
	virtual bool save (UTF8StringPtr filename);

	UTF8StringPtr getXmFileName () const { return xmlFile.u.name; }
	
	CView* createView (UTF8StringPtr name, IController* controller);
	const UIAttributes* getViewAttributes (UTF8StringPtr name);

	void setController (IController* controller);

	CBitmap* getBitmap (UTF8StringPtr name);
	CFontRef getFont (UTF8StringPtr name);
	bool getColor (UTF8StringPtr name, CColor& color);
	int32_t getTagForName (UTF8StringPtr name);
	CControlListener* getControlListener (UTF8StringPtr name);
	IController* getController () const { return controller; }
	IViewFactory* getViewFactory () const { return viewFactory; }
	
	UTF8StringPtr lookupColorName (const CColor& color) const;
	UTF8StringPtr lookupFontName (const CFontRef font) const;
	UTF8StringPtr lookupBitmapName (const CBitmap* bitmap) const;
	UTF8StringPtr lookupControlTagName (const int32_t tag) const;
	
	void collectTemplateViewNames (std::list<const std::string*>& names) const;
	void collectColorNames (std::list<const std::string*>& names) const;
	void collectFontNames (std::list<const std::string*>& names) const;
	void collectBitmapNames (std::list<const std::string*>& names) const;
	void collectControlTagNames (std::list<const std::string*>& names) const;
	
	void changeColorName (UTF8StringPtr oldName, UTF8StringPtr newName);
	void changeTagName (UTF8StringPtr oldName, UTF8StringPtr newName);
	void changeFontName (UTF8StringPtr oldName, UTF8StringPtr newName);
	void changeBitmapName (UTF8StringPtr oldName, UTF8StringPtr newName);

	void changeColor (UTF8StringPtr name, const CColor& newColor);
	void changeTag (UTF8StringPtr name, int32_t tag);
	void changeFont (UTF8StringPtr name, CFontRef newFont);
	void changeBitmap (UTF8StringPtr name, UTF8StringPtr newName, const CRect* nineparttiledOffset = 0);
	
	void removeColor (UTF8StringPtr name);
	void removeTag (UTF8StringPtr name);
	void removeFont (UTF8StringPtr name);
	void removeBitmap (UTF8StringPtr name);

	void updateViewDescription (UTF8StringPtr name, CView* view);
	bool getTemplateNameFromView (CView* view, std::string& templateName);
	bool addNewTemplate (UTF8StringPtr name, UIAttributes* attr); // owns attributes
	bool removeTemplate (UTF8StringPtr name);

	bool setCustomAttributes (UTF8StringPtr name, UIAttributes* attr); //owns attributes
	UIAttributes* getCustomAttributes (UTF8StringPtr name) const;

	static bool parseColor (const std::string& colorString, CColor& color);
	static CViewAttributeID kTemplateNameAttributeID;
protected:
	CView* createViewFromNode (UINode* node);
	UINode* getBaseNode (UTF8StringPtr name) const;
	UINode* findChildNodeByNameAttribute (UINode* node, UTF8StringPtr nameAttribute) const;
	bool updateAttributesForView (UINode* node, CView* view, bool deep = true);

	void addDefaultNodes ();

	bool saveToStream (OutputStream& stream);

	// Xml::IHandler
	void startXmlElement (Xml::Parser* parser, IdStringPtr elementName, UTF8StringPtr* elementAttributes);
	void endXmlElement (Xml::Parser* parser, IdStringPtr name);
	void xmlCharData (Xml::Parser* parser, const int8_t* data, int32_t length);
	void xmlComment (Xml::Parser* parser, IdStringPtr comment);

	CResourceDescription xmlFile;
	UINode* nodes;
	IController* controller;
	IViewFactory* viewFactory;
	Xml::IContentProvider* xmlContentProvider;

	std::deque<UINode*> nodeStack;
	int32_t parseState;
};
 
//-----------------------------------------------------------------------------
class UIAttributes : public CBaseObject, public std::map<std::string,std::string>
{
public:
	UIAttributes (UTF8StringPtr* attributes = 0);
	~UIAttributes ();

	bool hasAttribute (UTF8StringPtr name) const;
	const std::string* getAttributeValue (UTF8StringPtr name) const;
	void setAttribute (UTF8StringPtr name, UTF8StringPtr value);
	void removeAttribute (UTF8StringPtr name);
	
	void setRectAttribute (UTF8StringPtr name, const CRect& r);
	bool getRectAttribute (UTF8StringPtr name, CRect& r) const;
	
	void removeAll () { clear (); }

	bool store (OutputStream& stream);
	bool restore (InputStream& stream);
};

//-----------------------------------------------------------------------------
class IViewFactory
{
public:
	virtual ~IViewFactory () {}
	
	virtual CView* createView (const UIAttributes& attributes, IUIDescription* description) = 0;
	virtual bool applyAttributeValues (CView* view, const UIAttributes& attributes, IUIDescription* desc) const = 0;
};

//-----------------------------------------------------------------------------
class DelegationController : public IController
{
public:
	DelegationController (IController* controller) : controller (controller) {}

	// CControlListener
	void valueChanged (CControl* pControl) { controller->valueChanged (pControl); }
	int32_t controlModifierClicked (CControl* pControl, CButtonState button) { return controller->controlModifierClicked (pControl, button); }
	void controlBeginEdit (CControl* pControl) { controller->controlBeginEdit (pControl); }
	void controlEndEdit (CControl* pControl) { controller->controlEndEdit (pControl); }
	void controlTagWillChange (VSTGUI::CControl* pControl) { controller->controlTagWillChange (pControl); }
	void controlTagDidChange (VSTGUI::CControl* pControl) { controller->controlTagDidChange (pControl); }
	// IController
	int32_t getTagForName (UTF8StringPtr name, int32_t registeredTag) { return controller->getTagForName (name, registeredTag); }
	CControlListener* getControlListener (UTF8StringPtr name) { return controller->getControlListener (name); }
	CView* createView (const UIAttributes& attributes, IUIDescription* description) { return controller->createView (attributes, description); }
	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description) { return controller->verifyView (view, attributes, description); }
protected:
	IController* controller;
};

} // namespace

#endif
