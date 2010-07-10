//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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

#include "uidescription.h"
#include "uiviewfactory.h"
#include "uiviewcreator.h"
#include "uiviewswitchcontainer.h"
#include "cstream.h"
#include "../lib/cfont.h"
#include "../lib/cframe.h"
#include "../lib/cdrawcontext.h"
#include "../lib/platform/win32/win32support.h"
#include "../lib/platform/mac/macglobals.h"
#include <list>
#include <sstream>
#include <fstream>
#include <vector>

namespace VSTGUI {

//-----------------------------------------------------------------------------
template <class T> class ScopePointer
{
public:
	ScopePointer (T** pointer, T* obj) : pointer (pointer), oldObject (0)
	{
		if (pointer)
		{
			oldObject = *pointer;
			*pointer = obj;
		}
	}
	~ScopePointer ()
	{
		if (pointer)
			*pointer = oldObject;
	}
protected:
	T** pointer;
	T* oldObject;
};

class UIDescList;
//-----------------------------------------------------------------------------
class UINode : public CBaseObject
{
public:
	UINode (const std::string& name, UIAttributes* attributes);

	const std::string& getName () const { return name; }
	UIAttributes* getAttributes () const { return attributes; }
	UIDescList& getChildren () const { return *children; }
	bool hasChildren () const;

	enum {
		kNoExport = 1 << 0
	};
	
	bool noExport () const { return flags & kNoExport; }
	void noExport (bool state) { if (state) flags |= kNoExport; else flags &= ~kNoExport; }
protected:
	~UINode ();
	std::string name;
	UIAttributes* attributes;
	UIDescList* children;
	int32_t flags;
};

//-----------------------------------------------------------------------------
class UIControlTagNode : public UINode
{
public:
	UIControlTagNode (const std::string& name, UIAttributes* attributes);
	int32_t getTag ();
	void setTag (int32_t newTag);
protected:
	int32_t tag;
};

//-----------------------------------------------------------------------------
class UIBitmapNode : public UINode
{
public:
	UIBitmapNode (const std::string& name, UIAttributes* attributes);
	CBitmap* getBitmap ();
	void setBitmap (UTF8StringPtr bitmapName);
	void setNinePartTiledOffset (const CRect* offsets);
protected:
	~UIBitmapNode ();
	CBitmap* bitmap;
};

//-----------------------------------------------------------------------------
class UIFontNode : public UINode
{
public:
	UIFontNode (const std::string& name, UIAttributes* attributes);
	CFontRef getFont ();
	void setFont (CFontRef newFont);
protected:
	~UIFontNode ();
	CFontRef font;
};

//-----------------------------------------------------------------------------
class UIColorNode : public UINode
{
public:
	UIColorNode (const std::string& name, UIAttributes* attributes);
	const CColor& getColor () const { return color; }
	void setColor (const CColor& newColor);
protected:
	CColor color;
};

//-----------------------------------------------------------------------------
class UIDescList : public CBaseObject, protected std::list<UINode*>
{
public:
	typedef std::list<UINode*>::reverse_iterator	riterator;
	typedef std::list<UINode*>::iterator			iterator;
	
	UIDescList (bool ownsObjects = true) : ownsObjects (ownsObjects) {}
	~UIDescList () { removeAll (); }

	void add (UINode* obj) { if (!ownsObjects) obj->remember (); std::list<UINode*>::push_back (obj); }
	void remove (UINode* obj) { std::list<UINode*>::remove (obj); obj->forget (); }

	int32_t total () { return size (); }
	void removeAll () { riterator it = rbegin (); while (it != rend ()) remove (*it); }

	iterator begin () { return std::list<UINode*>::begin (); }
	iterator end () { return std::list<UINode*>::end (); }
	riterator rbegin () { return std::list<UINode*>::rbegin (); }
	riterator rend () { return std::list<UINode*>::rend (); }
protected:
	bool ownsObjects;
};

//-----------------------------------------------------------------------------
class ResourceReader : public Xml::IContentProvider
{
public:
	ResourceReader (const CResourceDescription& resFile);
	~ResourceReader ();

	bool open ();

protected:
	int32_t readRawXmlData (int8_t* buffer, int32_t size);
	void rewind ();

	CResourceDescription resFile;
	void* platformHandle;
};

//-----------------------------------------------------------------------------
class UIDescWriter
{
public:
	bool write (UTF8StringPtr filename, UINode* rootNode);
protected:
	bool writeNode (UINode* node, std::iostream& stream);
	bool writeAttributes (UIAttributes* attr, std::iostream& stream);
	int32_t intendLevel;
};

//-----------------------------------------------------------------------------
bool UIDescWriter::write (UTF8StringPtr filename, UINode* rootNode)
{
	bool result = false;
	intendLevel = 0;
	std::fstream stream (filename, std::ios::out | std::ios::trunc);
	if (stream.is_open ())
	{
		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
		result = writeNode (rootNode, stream);
		stream.close ();
	}
	else
		result = false;
	return result;
}

//-----------------------------------------------------------------------------
bool UIDescWriter::writeAttributes (UIAttributes* attr, std::iostream& stream)
{
	bool result = true;
	UIAttributes::iterator it = attr->begin ();
	while (it != attr->end ())
	{
		if ((*it).second.length () > 0)
		{
			stream << " ";
			stream << (*it).first;
			stream << "=\"";
			stream << (*it).second;
			stream << "\"";
		}
		it++;
	}
	return result;
}

//-----------------------------------------------------------------------------
bool UIDescWriter::writeNode (UINode* node, std::iostream& stream)
{
	bool result = true;
	if (node->noExport ())
		return result;
	for (int32_t i = 0; i < intendLevel; i++) stream << "\t";
	stream << "<";
	stream << node->getName ();
	result = writeAttributes (node->getAttributes (), stream);
	if (result)
	{
		UIDescList& children = node->getChildren ();
		if (children.total () > 0)
		{
			stream << ">\n";
			intendLevel++;
			UIDescList::iterator it = children.begin ();
			while (it != children.end ())
			{
				if (!writeNode (*it, stream))
					return false;
				it++;
			}
			intendLevel--;
			for (int32_t i = 0; i < intendLevel; i++) stream << "\t";
			stream << "</";
			stream << node->getName ();
			stream << ">\n";
		}
		else
			stream << "/>\n";
	}
	return result;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool UIDescription::parseColor (const std::string& colorString, CColor& color)
{
	if (colorString.length () == 7)
	{
		if (colorString[0] == '#')
		{
			std::string rv (colorString.substr (1, 2));
			std::string gv (colorString.substr (3, 2));
			std::string bv (colorString.substr (5, 2));
			color.red = (uint8_t)strtol (rv.c_str (), 0, 16);
			color.green = (uint8_t)strtol (gv.c_str (), 0, 16);
			color.blue = (uint8_t)strtol (bv.c_str (), 0, 16);
			color.alpha = 255;
			return true;
		}
	}
	if (colorString.length () == 9)
	{
		if (colorString[0] == '#')
		{
			std::string rv (colorString.substr (1, 2));
			std::string gv (colorString.substr (3, 2));
			std::string bv (colorString.substr (5, 2));
			std::string av (colorString.substr (7, 2));
			color.red = (uint8_t)strtol (rv.c_str (), 0, 16);
			color.green = (uint8_t)strtol (gv.c_str (), 0, 16);
			color.blue = (uint8_t)strtol (bv.c_str (), 0, 16);
			color.alpha = (uint8_t)strtol (av.c_str (), 0, 16);
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
static UIViewFactory* getGenericViewFactory ()
{
	static UIViewFactory genericViewFactory;
	return &genericViewFactory;
}

//-----------------------------------------------------------------------------
UIDescription::UIDescription (const CResourceDescription& xmlFile, IViewFactory* _viewFactory)
: xmlFile (xmlFile)
, nodes (0)
, controller (0)
, viewFactory (_viewFactory)
, xmlContentProvider (0)
{
	if (viewFactory == 0)
		viewFactory = getGenericViewFactory ();
}

//-----------------------------------------------------------------------------
UIDescription::UIDescription (Xml::IContentProvider* xmlContentProvider, IViewFactory* _viewFactory)
: nodes (0)
, controller (0)
, viewFactory (_viewFactory)
, xmlContentProvider (xmlContentProvider)
{
	if (viewFactory == 0)
		viewFactory = getGenericViewFactory ();
}

//-----------------------------------------------------------------------------
UIDescription::~UIDescription ()
{
	if (nodes)
		nodes->forget ();
}

//-----------------------------------------------------------------------------
void UIDescription::addDefaultNodes ()
{
	UINode* fontsNode = getBaseNode ("fonts");
	if (fontsNode)
	{
		struct DefaultFont {
			UTF8StringPtr name;
			CFontRef font;
		};

		const DefaultFont defaultFonts [] = {
			{ "~ SystemFont", kSystemFont },
			{ "~ NormalFontVeryBig", kNormalFontVeryBig },
			{ "~ NormalFontBig", kNormalFontBig },
			{ "~ NormalFont", kNormalFont },
			{ "~ NormalFontSmall", kNormalFontSmall },
			{ "~ NormalFontSmaller", kNormalFontSmaller },
			{ "~ NormalFontVerySmall", kNormalFontVerySmall },
			{ "~ SymbolFont", kSymbolFont },
			{ 0, 0 },
		};
		int32_t i = 0;
		while (defaultFonts[i].name != 0)
		{
			UIAttributes* attr = new UIAttributes;
			attr->setAttribute ("name", defaultFonts[i].name);
			UIFontNode* node = new UIFontNode ("font", attr);
			node->setFont (defaultFonts[i].font);
			node->noExport (true);
			fontsNode->getChildren ().add (node);
			i++;
		}
	}
	UINode* colorsNode = getBaseNode ("colors");
	if (colorsNode)
	{
		struct DefaultColor {
			UTF8StringPtr name;
			CColor color;
		};

		const DefaultColor defaultColors [] = {
			{ "~ BlackCColor", kBlackCColor },
			{ "~ WhiteCColor", kWhiteCColor },
			{ "~ GreyCColor", kGreyCColor },
			{ "~ RedCColor", kRedCColor },
			{ "~ GreenCColor", kGreenCColor },
			{ "~ BlueCColor", kBlueCColor },
			{ "~ YellowCColor", kYellowCColor },
			{ "~ CyanCColor", kCyanCColor },
			{ "~ MagentaCColor", kMagentaCColor },
			{ "~ TransparentCColor", kTransparentCColor },
			{ 0, kBlackCColor }
		};
		
		int32_t i = 0;
		while (defaultColors[i].name != 0)
		{
			UIAttributes* attr = new UIAttributes;
			attr->setAttribute ("name", defaultColors[i].name);
			std::string colorStr;
			colorToString (defaultColors[i].color, colorStr, 0);
			attr->setAttribute ("rgba", colorStr.c_str ());
			UIColorNode* node = new UIColorNode ("color", attr);
			node->noExport (true);
			colorsNode->getChildren ().add (node);
			i++;
		}
	}
}

//-----------------------------------------------------------------------------
bool UIDescription::parse ()
{
	if (nodes)
		return true;
	Xml::Parser parser;
	if (xmlContentProvider)
	{
		if (parser.parse (xmlContentProvider, this))
		{
			addDefaultNodes ();
			return true;
		}
	}
	else
	{
		ResourceReader reader (xmlFile);
		if (reader.open ())
		{
			if (parser.parse (&reader, this))
			{
				addDefaultNodes ();
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
void UIDescription::setController (IController* inController)
{
	controller = inController;
}

//-----------------------------------------------------------------------------
bool UIDescription::save (UTF8StringPtr filename)
{
	nodes->getAttributes ()->setAttribute ("version", "1");
	UIDescWriter writer;
	return writer.write (filename, nodes);
}

//-----------------------------------------------------------------------------
CView* UIDescription::createViewFromNode (UINode* node)
{
	const std::string* templateName = node->getAttributes ()->getAttributeValue ("template");
	if (templateName)
	{
		CView* view = createView (templateName->c_str (), controller);
		if (view)
			viewFactory->applyAttributeValues (view, *node->getAttributes (), this);
		return view;
	}

	CView* result = 0;
	if (controller)
	{
		result = controller->createView (*node->getAttributes (), this);
		if (result && viewFactory)
		{
			UIViewFactory* _viewFactory = dynamic_cast<UIViewFactory*> (viewFactory);
			if (_viewFactory)
			{
				const std::string* viewClass = node->getAttributes ()->getAttributeValue ("class");
				if (viewClass)
					_viewFactory->applyCustomViewAttributeValues (result, viewClass->c_str (), *node->getAttributes (), this);
			}
		}
	}
	if (result == 0 && viewFactory)
	{
		result = viewFactory->createView (*node->getAttributes (), this);
	}
	if (result && node->hasChildren ())
	{
		CViewContainer* viewContainer = dynamic_cast<CViewContainer*> (result);
		UIDescList::iterator it = node->getChildren ().begin ();
		while (it != node->getChildren ().end ())
		{
			if (viewContainer)
			{
				if ((*it)->getName () == "view")
				{
					CView* childView = createViewFromNode (*it);
					if (childView)
					{
						if (!viewContainer->addView (childView))
							childView->forget ();
					}
				}
			}
			if ((*it)->getName () == "attribute")
			{
				const std::string* attrName = (*it)->getAttributes ()->getAttributeValue ("id");
				const std::string* attrValue = (*it)->getAttributes ()->getAttributeValue ("value");
				if (attrName && attrValue)
				{
					CViewAttributeID attrId = 0;
					if (attrName->size () == 4)
					{
						char c1 = (*attrName)[0];
						char c2 = (*attrName)[1];
						char c3 = (*attrName)[2];
						char c4 = (*attrName)[3];
						attrId = ((((int32_t)c1) << 24) | (((int32_t)c2) << 16) | (((int32_t)c3) << 8) | (((int32_t)c4) << 0));
					}
					else
						attrId = strtol (attrName->c_str (), 0, 10);
					if (attrId)
						result->setAttribute (attrId, attrValue->size ()+1, attrValue->c_str ());
				}
			}
			it++;
		}
	}
	if (result && controller)
		result = controller->verifyView (result, *node->getAttributes (), this);
	return result;
}

//-----------------------------------------------------------------------------
CViewAttributeID UIDescription::kTemplateNameAttributeID = 'uitl';

//-----------------------------------------------------------------------------
CView* UIDescription::createView (UTF8StringPtr name, IController* _controller)
{
	ScopePointer<IController> sp (&controller, _controller);
	if (nodes)
	{
		UIDescList::iterator it = nodes->getChildren ().begin ();
		while (it != nodes->getChildren ().end ())
		{
			if ((*it)->getName () == "template")
			{
				const std::string* nodeName = (*it)->getAttributes ()->getAttributeValue ("name");
				if (*nodeName == name)
				{
					CView* view = createViewFromNode (*it);
					if (view)
						view->setAttribute (kTemplateNameAttributeID, strlen (name)+1, name);
					return view;
				}
			}
			it++;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
bool UIDescription::getTemplateNameFromView (CView* view, std::string& templateName)
{
	bool result = false;
	int32_t attrSize = 0;
	if (view->getAttributeSize (kTemplateNameAttributeID, attrSize))
	{
		char* str = new char[attrSize];
		if (view->getAttribute (kTemplateNameAttributeID, attrSize, str, attrSize))
		{
			templateName = str;
			result = true;
		}
		delete [] str;
	}
	return result;
}

//-----------------------------------------------------------------------------
const UIAttributes* UIDescription::getViewAttributes (UTF8StringPtr name)
{
	if (nodes)
	{
		UIDescList::iterator it = nodes->getChildren ().begin ();
		while (it != nodes->getChildren ().end ())
		{
			if ((*it)->getName () == "template")
			{
				const std::string* nodeName = (*it)->getAttributes ()->getAttributeValue ("name");
				if (*nodeName == name)
					return (*it)->getAttributes ();
			}
			it++;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
UINode* UIDescription::getBaseNode (UTF8StringPtr name) const
{
	if (nodes)
	{
		UIDescList::iterator it = nodes->getChildren ().begin ();
		while (it != nodes->getChildren ().end ())
		{
			if ((*it)->getName () == name)
			{
				return *it;
			}
			it++;
		}
		UINode* node = new UINode (name, new UIAttributes);
		nodes->getChildren ().add (node);
		return node;
	}
	return 0;
}

//-----------------------------------------------------------------------------
UINode* UIDescription::findChildNodeByNameAttribute (UINode* node, UTF8StringPtr nameAttribute) const
{
	if (node)
	{
		UIDescList::iterator it = node->getChildren ().begin ();
		while (it != node->getChildren ().end ())
		{
			const std::string* nameAttr = (*it)->getAttributes ()->getAttributeValue ("name");
			if (nameAttr && *nameAttr == nameAttribute)
			{
				return *it;
			}
			it++;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
int32_t UIDescription::getTagForName (UTF8StringPtr name)
{
	int32_t tag = -1;
	UIControlTagNode* controlTagNode = dynamic_cast<UIControlTagNode*> (findChildNodeByNameAttribute (getBaseNode ("control-tags"), name));
	if (controlTagNode)
		tag = controlTagNode->getTag ();
	if (controller)
		tag = controller->getTagForName (name, tag);
	return tag;
}

//-----------------------------------------------------------------------------
CControlListener* UIDescription::getControlListener (UTF8StringPtr name)
{
	if (controller)
		return controller->getControlListener (name);
	return 0;
}

//-----------------------------------------------------------------------------
CBitmap* UIDescription::getBitmap (UTF8StringPtr name)
{
	UIBitmapNode* bitmapNode = dynamic_cast<UIBitmapNode*> (findChildNodeByNameAttribute (getBaseNode ("bitmaps"), name));
	if (bitmapNode)
	{
		return bitmapNode->getBitmap ();
	}
	return 0;
}

//-----------------------------------------------------------------------------
CFontRef UIDescription::getFont (UTF8StringPtr name)
{
	UIFontNode* fontNode = dynamic_cast<UIFontNode*> (findChildNodeByNameAttribute (getBaseNode ("fonts"), name));
	if (fontNode)
	{
		return fontNode->getFont ();
	}
	return 0;
}

//-----------------------------------------------------------------------------
bool UIDescription::getColor (UTF8StringPtr name, CColor& color)
{
	UIColorNode* colorNode = dynamic_cast<UIColorNode*> (findChildNodeByNameAttribute (getBaseNode ("colors"), name));
	if (colorNode)
	{
		color = colorNode->getColor ();
		return true;
	}
	std::string colorName (name);
	if (parseColor (name, color))
		return true;
	return false;
}

//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::lookupColorName (const CColor& color) const
{
	UINode* colorsNode = getBaseNode ("colors");
	if (colorsNode)
	{
		UIDescList& children = colorsNode->getChildren ();
		UIDescList::iterator it = children.begin ();
		while (it != children.end ())
		{
			UIColorNode* node = dynamic_cast<UIColorNode*>(*it);
			if (node && node->getColor () == color)
			{
				const std::string* colorName = node->getAttributes ()->getAttributeValue ("name");
				return colorName ? colorName->c_str () : 0;
			}
			it++;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::lookupFontName (const CFontRef font) const
{
	UINode* fontsNode = getBaseNode ("fonts");
	if (fontsNode)
	{
		UIDescList& children = fontsNode->getChildren ();
		UIDescList::iterator it = children.begin ();
		while (it != children.end ())
		{
			UIFontNode* node = dynamic_cast<UIFontNode*>(*it);
			if (node && *node->getFont () == *font)
			{
				const std::string* fontName = node->getAttributes ()->getAttributeValue ("name");
				return fontName ? fontName->c_str () : 0;
			}
			it++;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::lookupBitmapName (const CBitmap* bitmap) const
{
	UINode* bitmapsNode = getBaseNode ("bitmaps");
	if (bitmapsNode)
	{
		UIDescList& children = bitmapsNode->getChildren ();
		UIDescList::iterator it = children.begin ();
		while (it != children.end ())
		{
			UIBitmapNode* node = dynamic_cast<UIBitmapNode*>(*it);
			if (node && node->getBitmap () == bitmap)
			{
				const std::string* bitmapName = node->getAttributes ()->getAttributeValue ("name");
				return bitmapName ? bitmapName->c_str () : 0;
			}
			it++;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::lookupControlTagName (const int32_t tag) const
{
	UINode* tagsNode = getBaseNode ("control-tags");
	if (tagsNode)
	{
		UIDescList& children = tagsNode->getChildren ();
		UIDescList::iterator it = children.begin ();
		while (it != children.end ())
		{
			UIControlTagNode* node = dynamic_cast<UIControlTagNode*>(*it);
			if (node && node->getTag () == tag)
			{
				const std::string* tagName = node->getAttributes ()->getAttributeValue ("name");
				return tagName ? tagName->c_str () : 0;
			}
			it++;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
void UIDescription::changeColorName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	UIColorNode* node = dynamic_cast<UIColorNode*> (findChildNodeByNameAttribute (getBaseNode ("colors"), oldName));
	if (node)
	{
		node->getAttributes ()->setAttribute ("name", newName);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeTagName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	UIControlTagNode* node = dynamic_cast<UIControlTagNode*> (findChildNodeByNameAttribute (getBaseNode ("control-tags"), oldName));
	if (node)
	{
		node->getAttributes ()->setAttribute ("name", newName);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeFontName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	UIFontNode* node = dynamic_cast<UIFontNode*> (findChildNodeByNameAttribute (getBaseNode ("fonts"), oldName));
	if (node)
	{
		node->getAttributes ()->setAttribute ("name", newName);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeBitmapName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	UIBitmapNode* node = dynamic_cast<UIBitmapNode*> (findChildNodeByNameAttribute (getBaseNode ("bitmaps"), oldName));
	if (node)
	{
		node->getAttributes ()->setAttribute ("name", newName);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeColor (UTF8StringPtr name, const CColor& newColor)
{
	UIColorNode* node = dynamic_cast<UIColorNode*> (findChildNodeByNameAttribute (getBaseNode ("colors"), name));
	if (node)
	{
		if (!node->noExport ())
			node->setColor (newColor);
	}
	else
	{
		UINode* colorsNode = getBaseNode ("colors");
		if (colorsNode)
		{
			UIAttributes* attr = new UIAttributes;
			attr->setAttribute ("name", name);
			std::string colorStr;
			colorToString (newColor, colorStr, 0);
			attr->setAttribute ("rgba", colorStr.c_str ());
			UIColorNode* node = new UIColorNode ("color", attr);
			colorsNode->getChildren ().add (node);
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeTag (UTF8StringPtr name, int32_t tag)
{
	UIControlTagNode* node = dynamic_cast<UIControlTagNode*> (findChildNodeByNameAttribute (getBaseNode ("control-tags"), name));
	if (node)
	{
		if (!node->noExport ())
			node->setTag (tag);
	}
	else
	{
		UINode* tagsNode = getBaseNode ("control-tags");
		if (tagsNode)
		{
			UIAttributes* attr = new UIAttributes;
			attr->setAttribute ("name", name);
			UIControlTagNode* node = new UIControlTagNode ("control-tag", attr);
			node->setTag (tag);
			tagsNode->getChildren ().add (node);
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeFont (UTF8StringPtr name, CFontRef newFont)
{
	UIFontNode* node = dynamic_cast<UIFontNode*> (findChildNodeByNameAttribute (getBaseNode ("fonts"), name));
	if (node)
	{
		if (!node->noExport ())
			node->setFont (newFont);
	}
	else
	{
		UINode* fontsNode = getBaseNode ("fonts");
		if (fontsNode)
		{
			UIAttributes* attr = new UIAttributes;
			attr->setAttribute ("name", name);
			UIFontNode* node = new UIFontNode ("font", attr);
			node->setFont (newFont);
			fontsNode->getChildren ().add (node);
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeBitmap (UTF8StringPtr name, UTF8StringPtr newName, const CRect* nineparttiledOffset)
{
	UIBitmapNode* node = dynamic_cast<UIBitmapNode*> (findChildNodeByNameAttribute (getBaseNode ("bitmaps"), name));
	if (node)
	{
		if (!node->noExport ())
		{
			node->setBitmap (newName);
			node->setNinePartTiledOffset (nineparttiledOffset);
		}
	}
	else
	{
		UINode* bitmapsNode = getBaseNode ("bitmaps");
		if (bitmapsNode)
		{
			UIAttributes* attr = new UIAttributes;
			attr->setAttribute ("name", name);
			UIBitmapNode* node = new UIBitmapNode ("bitmap", attr);
			if (nineparttiledOffset)
				node->setNinePartTiledOffset (nineparttiledOffset);
			node->setBitmap (newName);
			bitmapsNode->getChildren ().add (node);
		}
	}
}

//-----------------------------------------------------------------------------
static void removeChildNode (UINode* baseNode, UTF8StringPtr nodeName)
{
	UIDescList& children = baseNode->getChildren ();
	UIDescList::iterator it = children.begin ();
	while (it != children.end ())
	{
		const std::string* name = (*it)->getAttributes ()->getAttributeValue ("name");
		if (*name == nodeName)
		{
			if (!(*it)->noExport ())
				children.remove (*it);
			return;
		}
		it++;
	}
}

//-----------------------------------------------------------------------------
void UIDescription::removeColor (UTF8StringPtr name)
{
	UINode* node = getBaseNode ("colors");
	if (node)
		removeChildNode (node, name);
}

//-----------------------------------------------------------------------------
void UIDescription::removeTag (UTF8StringPtr name)
{
	UINode* node = getBaseNode ("control-tags");
	if (node)
		removeChildNode (node, name);
}

//-----------------------------------------------------------------------------
void UIDescription::removeFont (UTF8StringPtr name)
{
	UINode* node = getBaseNode ("fonts");
	if (node)
		removeChildNode (node, name);
}

//-----------------------------------------------------------------------------
void UIDescription::removeBitmap (UTF8StringPtr name)
{
	UINode* node = getBaseNode ("bitmaps");
	if (node)
		removeChildNode (node, name);
}

//-----------------------------------------------------------------------------
void UIDescription::collectTemplateViewNames (std::list<const std::string*>& names) const
{
	UIDescList::iterator it = nodes->getChildren ().begin ();
	while (it != nodes->getChildren ().end ())
	{
		if ((*it)->getName () == "template")
		{
			const std::string* nodeName = (*it)->getAttributes ()->getAttributeValue ("name");
			if (nodeName)
				names.push_back (nodeName);
		}
		it++;
	}
}

//-----------------------------------------------------------------------------
void UIDescription::collectColorNames (std::list<const std::string*>& names) const
{
	UINode* colorsNode = getBaseNode ("colors");
	if (colorsNode)
	{
		UIDescList& children = colorsNode->getChildren ();
		UIDescList::iterator it = children.begin ();
		while (it != children.end ())
		{
			UIColorNode* node = dynamic_cast<UIColorNode*>(*it);
			if (node)
			{
				const std::string* name = node->getAttributes ()->getAttributeValue ("name");
				if (name)
					names.push_back (name);
			}
			it++;
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescription::collectFontNames (std::list<const std::string*>& names) const
{
	UINode* fontsNode = getBaseNode ("fonts");
	if (fontsNode)
	{
		UIDescList& children = fontsNode->getChildren ();
		UIDescList::iterator it = children.begin ();
		while (it != children.end ())
		{
			UIFontNode* node = dynamic_cast<UIFontNode*>(*it);
			if (node)
			{
				const std::string* name = node->getAttributes ()->getAttributeValue ("name");
				if (name)
					names.push_back (name);
			}
			it++;
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescription::collectBitmapNames (std::list<const std::string*>& names) const
{
	UINode* bitmapsNode = getBaseNode ("bitmaps");
	if (bitmapsNode)
	{
		UIDescList& children = bitmapsNode->getChildren ();
		UIDescList::iterator it = children.begin ();
		while (it != children.end ())
		{
			UIBitmapNode* node = dynamic_cast<UIBitmapNode*>(*it);
			if (node)
			{
				const std::string* name = node->getAttributes ()->getAttributeValue ("name");
				if (name)
					names.push_back (name);
			}
			it++;
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescription::collectControlTagNames (std::list<const std::string*>& names) const
{
	UINode* tagsNode = getBaseNode ("control-tags");
	if (tagsNode)
	{
		UIDescList& children = tagsNode->getChildren ();
		UIDescList::iterator it = children.begin ();
		while (it != children.end ())
		{
			UIControlTagNode* node = dynamic_cast<UIControlTagNode*>(*it);
			if (node)
			{
				const std::string* name = node->getAttributes ()->getAttributeValue ("name");
				if (name)
					names.push_back (name);
			}
			it++;
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescription::updateAttributesForView (UINode* node, CView* view, bool deep)
{
#if VSTGUI_LIVE_EDITING
	UIViewFactory* factory = dynamic_cast<UIViewFactory*> (viewFactory);
	std::list<std::string> attributeNames;
	if (factory->getAttributeNamesForView (view, attributeNames))
	{
		std::list<std::string>::const_iterator it = attributeNames.begin ();
		while (it != attributeNames.end ())
		{
			std::string value;
			if (factory->getAttributeValue (view, (*it), value, this))
				node->getAttributes ()->setAttribute ((*it).c_str (), value.c_str ());
			it++;
		}
		node->getAttributes ()->setAttribute ("class", factory->getViewName (view));
		CViewContainer* container = dynamic_cast<CViewContainer*> (view);
		if (deep && container)
		{
			ViewIterator it (container);
			while (*it)
			{
				CView* subView = *it;
				std::string subTemplateName;
				if (getTemplateNameFromView (subView, subTemplateName))
				{
					UIAttributes* attr = new UIAttributes;
					attr->setAttribute ("template", subTemplateName.c_str ());
					UINode* subNode = new UINode ("view", attr);
					node->getChildren ().add (subNode);
					updateAttributesForView (subNode, subView, false);
					CRect r = subView->getViewSize ();
					CRect r2 (r);
					r.offset (-r.left, -r.top);
					subView->setViewSize (r);
					subView->setMouseableArea (r);
					updateViewDescription (subTemplateName.c_str (), subView);
					subView->setViewSize (r2);
					subView->setMouseableArea (r2);
				}
				else
				{
					UINode* subNode = new UINode ("view", 0);
					updateAttributesForView (subNode, subView);
					node->getChildren ().add (subNode);
				}
				++it;
			}
		}
	}
#endif
}

//-----------------------------------------------------------------------------
void UIDescription::updateViewDescription (UTF8StringPtr name, CView* view)
{
#if VSTGUI_LIVE_EDITING
	UIViewFactory* factory = dynamic_cast<UIViewFactory*> (viewFactory);
	if (factory && nodes)
	{
		UINode* node = 0;
		UIDescList::iterator it = nodes->getChildren ().begin ();
		while (it != nodes->getChildren ().end () && node == 0)
		{
			if ((*it)->getName () == "template")
			{
				const std::string* nodeName = (*it)->getAttributes ()->getAttributeValue ("name");
				if (*nodeName == name)
				{
					node = (*it);
				}
			}
			it++;
		}
		if (node == 0)
		{
			node = new UINode ("template", 0);
		}
		node->getChildren ().removeAll ();
		updateAttributesForView (node, view);
	}
#endif
}

//-----------------------------------------------------------------------------
bool UIDescription::addNewTemplate (UTF8StringPtr name, UIAttributes* attr)
{
#if VSTGUI_LIVE_EDITING
	if (!nodes)
	{
		nodes = new UINode ("vstgui-ui-description", new UIAttributes);
	}
	UINode* templateNode = findChildNodeByNameAttribute (nodes, name);
	if (templateNode == 0)
	{
		UINode* newNode = new UINode ("template", attr);
		attr->setAttribute ("name", name);
		nodes->getChildren ().add (newNode);
		return true;
	}
#endif
	return false;
}

//-----------------------------------------------------------------------------
bool UIDescription::removeTemplate (UTF8StringPtr name)
{
#if VSTGUI_LIVE_EDITING
	UINode* templateNode = findChildNodeByNameAttribute (nodes, name);
	if (templateNode)
	{
		nodes->getChildren ().remove (templateNode);
		return true;
	}
#endif
	return false;
}

//-----------------------------------------------------------------------------
bool UIDescription::setCustomAttributes (UTF8StringPtr name, UIAttributes* attr)
{
	UINode* customNode = findChildNodeByNameAttribute (getBaseNode ("custom"), name);
	if (customNode)
		return false;
	attr->setAttribute ("name", name);
	customNode = new UINode ("attributes", attr);
	UINode* parent = getBaseNode ("custom");
	parent->getChildren ().add (customNode);
	return true;
}

//-----------------------------------------------------------------------------
UIAttributes* UIDescription::getCustomAttributes (UTF8StringPtr name) const
{
	UINode* customNode = findChildNodeByNameAttribute (getBaseNode ("custom"), name);
	if (customNode)
		return customNode->getAttributes ();
	return 0;
}

//-----------------------------------------------------------------------------
void UIDescription::startXmlElement (Xml::Parser* parser, IdStringPtr elementName, UTF8StringPtr* elementAttributes)
{
	std::string name (elementName);
	if (nodes)
	{
		UINode* parent = nodeStack.back ();
		UINode* newNode = 0;
		if (parent == nodes)
		{
			// only allowed second level elements
			if (name == "bitmaps" || name == "fonts" || name == "colors" || name == "template" || name == "control-tags" || name == "custom")
				newNode = new UINode (name, new UIAttributes (elementAttributes));
			else
				parser->stop ();
		}
		else if (parent->getName () == "bitmaps")
		{
			if (name == "bitmap")
				newNode = new UIBitmapNode (name, new UIAttributes (elementAttributes));
			else
				parser->stop ();
		}
		else if (parent->getName () == "fonts")
		{
			if (name == "font")
				newNode = new UIFontNode (name, new UIAttributes (elementAttributes));
			else
				parser->stop ();
		}
		else if (parent->getName () == "colors")
		{
			if (name == "color")
				newNode = new UIColorNode (name, new UIAttributes (elementAttributes));
			else
				parser->stop ();
		}
		else if (parent->getName () == "control-tags")
		{
			if (name == "control-tag")
				newNode = new UIControlTagNode (name, new UIAttributes (elementAttributes));
			else
				parser->stop ();
		}
		else
			newNode = new UINode (name, new UIAttributes (elementAttributes));
		if (newNode)
		{
			parent->getChildren ().add (newNode);
			nodeStack.push_back (newNode);
		}
	}
	else if (name == "vstgui-ui-description")
	{
		nodes = new UINode (name, new UIAttributes (elementAttributes));
		nodeStack.push_back (nodes);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::endXmlElement (Xml::Parser* parser, IdStringPtr name)
{
	nodeStack.pop_back ();
}

//-----------------------------------------------------------------------------
void UIDescription::xmlCharData (Xml::Parser* parser, const int8_t* data, int32_t length)
{
}

//-----------------------------------------------------------------------------
void UIDescription::xmlComment (Xml::Parser* parser, IdStringPtr comment)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UINode::UINode (const std::string& _name, UIAttributes* _attributes)
: name (_name)
, attributes (_attributes)
, children (new UIDescList)
, flags (0)
{
	if (attributes == 0)
		attributes = new UIAttributes ();
}

//-----------------------------------------------------------------------------
UINode::~UINode ()
{
	if (attributes)
		attributes->forget ();
	children->forget ();
}

//-----------------------------------------------------------------------------
bool UINode::hasChildren () const
{
	return children->total () > 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UIControlTagNode::UIControlTagNode (const std::string& name, UIAttributes* attributes)
: UINode (name, attributes)
, tag (-1)
{
}

//-----------------------------------------------------------------------------
int32_t UIControlTagNode::getTag ()
{
	if (tag == -1)
	{
		const std::string* tagStr = attributes->getAttributeValue ("tag");
		if (tagStr)
		{
			if (tagStr->size () == 6 && (*tagStr)[0] == '\'' && (*tagStr)[6] == '\'')
			{
				char c1 = (*tagStr)[1];
				char c2 = (*tagStr)[2];
				char c3 = (*tagStr)[3];
				char c4 = (*tagStr)[4];
				tag = ((((int32_t)c1) << 24) | (((int32_t)c2) << 16) | (((int32_t)c3) << 8) | (((int32_t)c4) << 0));
			}
			else
				tag = strtol (tagStr->c_str (), 0, 10);
		}
	}
	return tag;
}

//-----------------------------------------------------------------------------
void UIControlTagNode::setTag (int32_t newTag)
{
	tag = newTag;
	std::stringstream str;
	str << tag;
	attributes->setAttribute ("tag", str.str ().c_str ());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UIBitmapNode::UIBitmapNode (const std::string& name, UIAttributes* attributes)
: UINode (name, attributes)
, bitmap (0)
{
}

//-----------------------------------------------------------------------------
UIBitmapNode::~UIBitmapNode ()
{
	if (bitmap)
		bitmap->forget ();
}

//-----------------------------------------------------------------------------
CBitmap* UIBitmapNode::getBitmap ()
{
	if (bitmap == 0)
	{
		const std::string* path = attributes->getAttributeValue ("path");
		if (path)
		{
			CRect offsets;
			if (attributes->getRectAttribute ("nineparttiled-offsets", offsets))
			{
				bitmap = new CNinePartTiledBitmap (path->c_str (), CNinePartTiledBitmap::PartOffsets (offsets.left, offsets.top, offsets.right, offsets.bottom));
			}
			else
				bitmap = new CBitmap (path->c_str ());
		}
	}
	return bitmap;
}

//-----------------------------------------------------------------------------
void UIBitmapNode::setBitmap (UTF8StringPtr bitmapName)
{
	std::string attrValue (bitmapName);
	attributes->setAttribute ("path", attrValue.c_str ());
	if (bitmap)
		bitmap->forget ();
	bitmap = 0;
}

//-----------------------------------------------------------------------------
void UIBitmapNode::setNinePartTiledOffset (const CRect* offsets)
{
	if (bitmap)
	{
		CNinePartTiledBitmap* tiledBitmap = dynamic_cast<CNinePartTiledBitmap*> (bitmap);
		if (offsets && tiledBitmap)
		{
			tiledBitmap->setPartOffsets (CNinePartTiledBitmap::PartOffsets (offsets->left, offsets->top, offsets->right, offsets->bottom));
		}
		else
		{
			bitmap->forget ();
			bitmap = 0;
		}
	}
	if (offsets)
		attributes->setRectAttribute ("nineparttiled-offsets", *offsets);
	else
		attributes->removeAttribute ("nineparttiled-offsets");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UIFontNode::UIFontNode (const std::string& name, UIAttributes* attributes)
: UINode (name, attributes)
, font (0)
{
}

//-----------------------------------------------------------------------------
UIFontNode::~UIFontNode ()
{
	if (font)
		font->forget ();
}

//-----------------------------------------------------------------------------
CFontRef UIFontNode::getFont ()
{
	if (font == 0)
	{
		const std::string* nameAttr = attributes->getAttributeValue ("font-name");
		const std::string* sizeAttr = attributes->getAttributeValue ("size");
		const std::string* boldAttr = attributes->getAttributeValue ("bold");
		const std::string* italicAttr = attributes->getAttributeValue ("italic");
		const std::string* underlineAttr = attributes->getAttributeValue ("underline");
		if (nameAttr)
		{
			int32_t size = 12;
			if (sizeAttr)
				size = strtol (sizeAttr->c_str (), 0, 10);
			int32_t fontStyle = 0;
			if (boldAttr && *boldAttr == "true")
				fontStyle |= kBoldFace;
			if (italicAttr && *italicAttr == "true")
				fontStyle |= kItalicFace;
			if (underlineAttr && *underlineAttr == "true")
				fontStyle |= kUnderlineFace;
			font = new CFontDesc (nameAttr->c_str (), size, fontStyle);
		}
	}
	return font;
}

//-----------------------------------------------------------------------------
void UIFontNode::setFont (CFontRef newFont)
{
	if (font)
		font->forget ();
	font = newFont;
	font->remember ();

	std::string name (*attributes->getAttributeValue ("name"));
	attributes->removeAll ();
	attributes->setAttribute ("name", name.c_str ());
	attributes->setAttribute ("font-name", newFont->getName ());
	std::stringstream str;
	str << newFont->getSize ();
	attributes->setAttribute ("size", str.str ().c_str ());
	if (newFont->getStyle () & kBoldFace)
		attributes->setAttribute ("bold", "true");
	if (newFont->getStyle () & kItalicFace)
		attributes->setAttribute ("italic", "true");
	if (newFont->getStyle () & kUnderlineFace)
		attributes->setAttribute ("underline", "true");
}

//-----------------------------------------------------------------------------
UIColorNode::UIColorNode (const std::string& name, UIAttributes* attributes)
: UINode (name, attributes)
{
	color.alpha = 255;
	const std::string* red = attributes->getAttributeValue ("red");
	const std::string* green = attributes->getAttributeValue ("green");
	const std::string* blue = attributes->getAttributeValue ("blue");
	const std::string* alpha = attributes->getAttributeValue ("alpha");
	const std::string* rgb = attributes->getAttributeValue ("rgb");
	const std::string* rgba = attributes->getAttributeValue ("rgba");
	if (red)
		color.red = (uint8_t)strtol (red->c_str (), 0, 10);
	if (green)
		color.green = (uint8_t)strtol (green->c_str (), 0, 10);
	if (blue)
		color.blue = (uint8_t)strtol (blue->c_str (), 0, 10);
	if (alpha)
		color.alpha = (uint8_t)strtol (alpha->c_str (), 0, 10);
	if (rgb)
		UIDescription::parseColor (*rgb, color);
	if (rgba)
		UIDescription::parseColor (*rgba, color);
}

//-----------------------------------------------------------------------------
void UIColorNode::setColor (const CColor& newColor)
{
	std::string name (*attributes->getAttributeValue ("name"));
	attributes->removeAll ();
	attributes->setAttribute ("name", name.c_str ());

	std::string colorString;
	colorToString (newColor, colorString, 0);
	attributes->setAttribute ("rgba", colorString.c_str ());
	color = newColor;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UIAttributes::UIAttributes (UTF8StringPtr* attributes)
{
	if (attributes)
	{
		int32_t i = 0;
		while (attributes[i] != NULL && attributes[i+1] != NULL)
		{
			insert (std::make_pair (attributes[i], attributes[i+1]));
			i += 2;
		}
	}
}

//-----------------------------------------------------------------------------
UIAttributes::~UIAttributes ()
{
}

//-----------------------------------------------------------------------------
bool UIAttributes::hasAttribute (UTF8StringPtr name) const
{
	if (getAttributeValue (name) != 0)
		return true;
	return false;
}

//-----------------------------------------------------------------------------
const std::string* UIAttributes::getAttributeValue (UTF8StringPtr name) const
{
	std::map<std::string, std::string>::const_iterator iter = find (name);
	if (iter != end ())
		return &iter->second;
	return 0;
}

//-----------------------------------------------------------------------------
void UIAttributes::setAttribute (UTF8StringPtr name, UTF8StringPtr value)
{
	std::map<std::string, std::string>::iterator iter = find (name);
	if (iter != end ())
		erase (iter);
	insert (std::make_pair (name, value));
}

//-----------------------------------------------------------------------------
void UIAttributes::removeAttribute (UTF8StringPtr name)
{
	std::map<std::string, std::string>::iterator iter = find (name);
	if (iter != end ())
		erase (iter);
}

//-----------------------------------------------------------------------------
void UIAttributes::setRectAttribute (UTF8StringPtr name, const CRect& r)
{
	std::stringstream str;
	str << r.left;
	str << ", ";
	str << r.top;
	str << ", ";
	str << r.right;
	str << ", ";
	str << r.bottom;
	setAttribute (name, str.str ().c_str ());
}

//-----------------------------------------------------------------------------
bool UIAttributes::getRectAttribute (UTF8StringPtr name, CRect& r) const
{
	const std::string* str = getAttributeValue (name);
	if (str)
	{
		size_t start = 0;
		size_t pos = str->find (",", start, 1);
		if (pos != std::string::npos)
		{
			std::vector<std::string> subStrings;
			while (pos != std::string::npos)
			{
				std::string name (*str, start, pos - start);
				subStrings.push_back (name);
				start = pos+1;
				pos = str->find (",", start, 1);
			}
			std::string name (*str, start, std::string::npos);
			subStrings.push_back (name);
			if (subStrings.size () == 4)
			{
				r.left = strtod (subStrings[0].c_str (), 0);
				r.top = strtod (subStrings[1].c_str (), 0);
				r.right = strtod (subStrings[2].c_str (), 0);
				r.bottom = strtod (subStrings[3].c_str (), 0);
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
bool UIAttributes::store (OutputStream& stream)
{
	if (!(stream << (int32_t)'UIAT')) return false;
	if (!(stream << (uint32_t)size ())) return false;
	iterator it = begin ();
	while (it != end ())
	{
		if (!(stream << (*it).first)) return false;
		if (!(stream << (*it).second)) return false;
		it++;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool UIAttributes::restore (InputStream& stream)
{
	int32_t identifier;
	if (!(stream >> identifier)) return false;
	if (identifier == 'UIAT')
	{
		uint32_t numAttr;
		if (!(stream >> numAttr)) return false;
		for (uint32_t i = 0; i < numAttr; i++)
		{
			std::string key, value;
			if (!(stream >> key)) return false;
			if (!(stream >> value)) return false;
			setAttribute (key.c_str (), value.c_str ());
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ResourceReader::ResourceReader (const CResourceDescription& resFile)
: resFile (resFile)
, platformHandle (0)
{
}

//-----------------------------------------------------------------------------
ResourceReader::~ResourceReader ()
{
	if (platformHandle)
	{
		#if MAC
		fclose ((FILE*)platformHandle);
		#elif WINDOWS
		delete ((ResourceStream*)platformHandle);
		#endif
	}
}

//-----------------------------------------------------------------------------
bool ResourceReader::open ()
{
	bool result = false;
	#if MAC
	if (resFile.type == CResourceDescription::kStringType && resFile.u.name[0] == '/')
	{
		// it's an absolute path, we can use it as is
		platformHandle = fopen (resFile.u.name, "r");
		if (platformHandle)
			result = true;
	}
	if (!result && getBundleRef ())
	{
		char filename [PATH_MAX];
		if (resFile.type == CResourceDescription::kIntegerType)
			sprintf (filename, "%05d.uidesc", (int32_t)resFile.u.id);
		else
			strcpy (filename, resFile.u.name);
		CFStringRef cfStr = CFStringCreateWithCString (NULL, filename, kCFStringEncodingUTF8);
		if (cfStr)
		{
			CFURLRef url = CFBundleCopyResourceURL (getBundleRef (), cfStr, 0, NULL);
			if (url)
			{
				char filePath[PATH_MAX];
				if (CFURLGetFileSystemRepresentation (url, true, (UInt8*)filePath, PATH_MAX))
				{
					platformHandle = fopen (filePath, "r");
					if (platformHandle)
						result = true;
				}
				CFRelease (url);
			}
			CFRelease (cfStr);
		}
	}
	#elif WINDOWS
	platformHandle = new ResourceStream ();
	if (!((ResourceStream*)platformHandle)->open (resFile, "DATA"))
	{
		delete ((ResourceStream*)platformHandle);
		platformHandle = 0;
	}
	else
		result = true;
	#endif
	return result;
}

//-----------------------------------------------------------------------------
int32_t ResourceReader::readRawXmlData (int8_t* buffer, int32_t size)
{
	if (platformHandle)
	{
		#if MAC
		return fread (buffer, 1, size, (FILE*)platformHandle);
		#elif WINDOWS
		ULONG read = 0;
		((ResourceStream*)platformHandle)->Read (buffer, size, &read);
		return read;
		#endif
	}
	return 0;
}

//-----------------------------------------------------------------------------
void ResourceReader::rewind ()
{
	if (platformHandle)
	{
		#if MAC
		fseek ((FILE*)platformHandle, 0L, SEEK_SET);
		#elif WINDOWS
		((ResourceStream*)platformHandle)->Revert ();
		#endif
	}
}

} // namespace
