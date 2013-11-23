//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
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
#include "cstream.h"
#include "base64codec.h"
#include "../lib/cfont.h"
#include "../lib/cstring.h"
#include "../lib/cframe.h"
#include "../lib/cdrawcontext.h"
#include "../lib/cbitmapfilter.h"
#include "../lib/platform/win32/win32support.h"
#include "../lib/platform/mac/macglobals.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <assert.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
/// @cond ignore
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
	UINode (const std::string& name, UIAttributes* attributes = 0);
	UINode (const std::string& name, UIDescList* children, UIAttributes* attributes = 0);
	UINode (const UINode& n);
	~UINode ();

	const std::string& getName () const { return name; }
	std::stringstream& getData () { return data; }
	const std::stringstream& getData () const { return data; }

	UIAttributes* getAttributes () const { return attributes; }
	UIDescList& getChildren () const { return *children; }
	bool hasChildren () const;

	enum {
		kNoExport = 1 << 0
	};
	
	bool noExport () const { return flags & kNoExport; }
	void noExport (bool state) { if (state) flags |= kNoExport; else flags &= ~kNoExport; }

	bool operator== (const UINode& n) const { return name == n.name; }
	
	void sortChildren ();

	CLASS_METHODS(UINode, CBaseObject)
protected:
	std::string name;
	std::stringstream data;
	UIAttributes* attributes;
	UIDescList* children;
	int32_t flags;
};

//-----------------------------------------------------------------------------
class UIVariableNode : public UINode
{
public:
	UIVariableNode (const std::string& name, UIAttributes* attributes);
	UIVariableNode (const UIVariableNode& n);
	
	enum Type {
		kNumber,
		kString,
		kUnknown
	};
	
	Type getType () const;
	double getNumber () const;
	const std::string& getString () const;

	CLASS_METHODS(UIVariableNode, UINode)
protected:
	Type type;
	double number;
};

//-----------------------------------------------------------------------------
class UIControlTagNode : public UINode
{
public:
	UIControlTagNode (const std::string& name, UIAttributes* attributes);
	UIControlTagNode (const UIControlTagNode& n);
	int32_t getTag ();
	void setTag (int32_t newTag);
	
	const std::string* getTagString () const;
	void setTagString (const std::string& str);
	
	CLASS_METHODS(UIControlTagNode, UINode)
protected:
	int32_t tag;
};

//-----------------------------------------------------------------------------
class UIBitmapNode : public UINode
{
public:
	UIBitmapNode (const std::string& name, UIAttributes* attributes);
	UIBitmapNode (const UIBitmapNode& n);
	CBitmap* getBitmap ();
	void setBitmap (UTF8StringPtr bitmapName);
	void setNinePartTiledOffset (const CRect* offsets);
	void invalidBitmap ();
	bool getFilterProcessed () const { return filterProcessed; }
	void setFilterProcessed () { filterProcessed = true; }
	
	void createXMLData ();
	void removeXMLData ();
	CLASS_METHODS(UIBitmapNode, UINode)
protected:
	~UIBitmapNode ();
	CBitmap* bitmap;
	bool filterProcessed;
};

//-----------------------------------------------------------------------------
class UIFontNode : public UINode
{
public:
	UIFontNode (const std::string& name, UIAttributes* attributes);
	UIFontNode (const UIFontNode& n);
	CFontRef getFont ();
	void setFont (CFontRef newFont);
	void setAlternativeFontNames (UTF8StringPtr fontNames);
	bool getAlternativeFontNames (std::string& fontNames);
	CLASS_METHODS(UIFontNode, UINode)
protected:
	~UIFontNode ();
	CFontRef font;
};

//-----------------------------------------------------------------------------
class UIColorNode : public UINode
{
public:
	UIColorNode (const std::string& name, UIAttributes* attributes);
	UIColorNode (const UIColorNode& n);
	const CColor& getColor () const { return color; }
	void setColor (const CColor& newColor);
	CLASS_METHODS(UIColorNode, UINode)
protected:
	CColor color;
};

typedef std::vector<UINode*> UIDescListContainerType;

//-----------------------------------------------------------------------------
class UIDescList : public CBaseObject, protected UIDescListContainerType
{
public:
	typedef UIDescListContainerType::const_reverse_iterator	riterator;
	typedef UIDescListContainerType::const_iterator		iterator;
	
	UIDescList (bool ownsObjects = true) : ownsObjects (ownsObjects) {}
	UIDescList (const UIDescList& l) : ownsObjects (l.ownsObjects)
	{
		for (const_iterator it = l.begin (); it != l.end (); it++)
			add (static_cast<UINode*>((*it)->newCopy ()));
	}
	
	~UIDescList () { removeAll (); }

	void add (UINode* obj) { if (!ownsObjects) obj->remember (); UIDescListContainerType::push_back (obj); }
	void remove (UINode* obj)
	{
		UIDescListContainerType::iterator pos = std::find (UIDescListContainerType::begin (), UIDescListContainerType::end (), obj);
		if (pos != UIDescListContainerType::end ())
		{
			UIDescListContainerType::erase (pos);
			obj->forget ();
		}
	}

	int32_t total () { return (int32_t)size (); }
	void removeAll ()
	{
		riterator it = rbegin ();
		while (it != rend ())
		{
			(*it)->forget ();
			it++;
		}
		clear ();
	}

	iterator begin () const { return UIDescListContainerType::begin (); }
	iterator end () const { return UIDescListContainerType::end (); }
	riterator rbegin () const { return UIDescListContainerType::rbegin (); }
	riterator rend () const { return UIDescListContainerType::rend (); }
	
	iterator get (const std::string& nodeName)
	{
		for (iterator it = begin (); it != end (); it++)
		{
			if (*(*it) == nodeName)
				return it;
		}
		return end ();
	}

	bool empty () const { return UIDescListContainerType::empty (); }
	
	void sort ()
	{
		std::sort (UIDescListContainerType::begin (), UIDescListContainerType::end (), nodeCompare);
	}
protected:
	static bool nodeCompare (UINode* n1, UINode* n2)
	{
		const std::string* str1 = n1->getAttributes ()->getAttributeValue ("name");
		const std::string* str2 = n2->getAttributes ()->getAttributeValue ("name");
		if (str1 && str2)
			return *str1 < *str2;
		else if (str1)
			return true;
		return false;
	}
	bool ownsObjects;
};

//-----------------------------------------------------------------------------
class UIDescWriter
{
public:
	bool write (OutputStream& stream, UINode* rootNode);
	bool write (OutputStream& stream, std::list<UINode*> nodeList);
protected:
	static void encodeAttributeString (std::string& str);

	bool writeNode (UINode* node, OutputStream& stream);
	bool writeNodeData (std::stringstream& str, OutputStream& stream);
	bool writeAttributes (UIAttributes* attr, OutputStream& stream);
	int32_t intendLevel;
};

//-----------------------------------------------------------------------------
bool UIDescWriter::write (OutputStream& stream, UINode* rootNode)
{
	intendLevel = 0;
	stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	return writeNode (rootNode, stream);
}

//-----------------------------------------------------------------------------
bool UIDescWriter::write (OutputStream& stream, std::list<UINode*> nodeList)
{
	intendLevel = 0;
	stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	for (std::list<UINode*>::const_iterator it = nodeList.begin (); it != nodeList.end (); it++)
	{
		if (!writeNode (*it, stream))
			return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
void UIDescWriter::encodeAttributeString (std::string& str)
{
	const int8_t entities[] = {'&', '<','>', '\'', '\"', 0};
	const char* replacements[] = {"&amp;", "&lt;", "&gt;", "&apos;", "&quot;"};
	int32_t i = 0;
	while (entities[i] != 0)
	{
		size_t pos = 0;
		while ((pos = str.find (entities[i], pos)) != std::string::npos)
		{
			str.replace (pos, 1, replacements[i]);
			pos++;
		}
		i++;
	}
}

//-----------------------------------------------------------------------------
bool UIDescWriter::writeAttributes (UIAttributes* attr, OutputStream& stream)
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
			std::string value ((*it).second);
			encodeAttributeString (value);
			stream << value;
			stream << "\"";
		}
		it++;
	}
	return result;
}

//-----------------------------------------------------------------------------
bool UIDescWriter::writeNodeData (std::stringstream& str, OutputStream& stream)
{
	for (int32_t i = 0; i < intendLevel; i++) stream << "\t";
	str.seekg (0);
	uint32_t i = 0;
	while (str.tellg () < str.tellp ())
	{
		int8_t byte;
		str >> byte;
		stream << byte;
		if (i++ > 80)
		{
			stream << "\n";
			i = 0;
			for (int32_t i = 0; i < intendLevel; i++) stream << "\t";
		}
	}
	stream << "\n";
	return true;
}

//-----------------------------------------------------------------------------
bool UIDescWriter::writeNode (UINode* node, OutputStream& stream)
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
			if (node->getData ().str ().length () > 0)
				result = writeNodeData (node->getData (), stream);
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
		else if (node->getData ().str ().length () > 0)
		{
			stream << ">\n";
			intendLevel++;
			result = writeNodeData (node->getData (), stream);
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
/// @endcond

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
IdStringPtr UIDescription::kMessageTagChanged = "kMessageTagChanged";
IdStringPtr UIDescription::kMessageColorChanged = "kMessageColorChanged";
IdStringPtr UIDescription::kMessageFontChanged = "kMessageFontChanged";
IdStringPtr UIDescription::kMessageBitmapChanged = "kMessageBitmapChanged";
IdStringPtr UIDescription::kMessageTemplateChanged = "kMessageTemplateChanged";
IdStringPtr UIDescription::kMessageBeforeSave = "kMessageBeforeSave";

//-----------------------------------------------------------------------------
UIDescription::UIDescription (const CResourceDescription& xmlFile, IViewFactory* _viewFactory)
: xmlFile (xmlFile)
, nodes (0)
, controller (0)
, viewFactory (_viewFactory)
, xmlContentProvider (0)
, bitmapCreator (0)
, restoreViewsMode (false)
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
, bitmapCreator (0)
, restoreViewsMode (false)
{
	memset (&xmlFile, 0, sizeof (CResourceDescription));
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
			UIViewCreator::colorToString (defaultColors[i].color, colorStr, 0);
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
		CResourceInputStream resInputStream;
		if (resInputStream.open (xmlFile))
		{
			Xml::InputStreamContentProvider contentProvider (resInputStream);
			if (parser.parse (&contentProvider, this))
			{
				addDefaultNodes ();
				return true;
			}
		}
		else if (xmlFile.type == CResourceDescription::kStringType)
		{
			CFileStream fileStream;
			if (fileStream.open (xmlFile.u.name, CFileStream::kReadMode))
			{
				Xml::InputStreamContentProvider contentProvider (fileStream);
				if (parser.parse (&contentProvider, this))
				{
					addDefaultNodes ();
					return true;
				}
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
void UIDescription::setBitmapCreator (IBitmapCreator* creator)
{
	bitmapCreator = creator;
}

//-----------------------------------------------------------------------------
bool UIDescription::saveWindowsRCFile (UTF8StringPtr filename)
{
	bool result = false;
	UINode* bitmapNodes = getBaseNode ("bitmaps");
	if (bitmapNodes && bitmapNodes->getChildren().total() > 0)
	{
		CFileStream stream;
		if (stream.open (filename, CFileStream::kWriteMode|CFileStream::kTruncateMode))
		{
			for (UIDescList::iterator it = bitmapNodes->getChildren ().begin (); it != bitmapNodes->getChildren ().end (); it++)
			{
				UIAttributes* attr = (*it)->getAttributes ();
				if (attr)
				{
					const std::string* path = attr->getAttributeValue ("path");
					if (path && !path->empty ())
					{
						stream << *path;
						stream << "\t PNG \"";
						stream << *path;
						stream << "\"\r";
					}
				}
			}
			result = true;
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
bool UIDescription::save (UTF8StringPtr filename, int32_t flags)
{
	bool result = false;
	CFileStream stream;
	if (stream.open (filename, CFileStream::kWriteMode|CFileStream::kTruncateMode))
	{
		result = saveToStream (stream, flags);
	}
	if (result && flags & kWriteWindowsResourceFile)
	{
		std::string rcFileName (filename);
		size_t extPos = rcFileName.find_last_of ('.');
		if (extPos != std::string::npos)
		{
			rcFileName.erase (extPos+1);
			rcFileName += "rc";
			saveWindowsRCFile (rcFileName.c_str ());
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
bool UIDescription::saveToStream (OutputStream& stream, int32_t flags)
{
	changed (kMessageBeforeSave);
	UINode* bitmapNodes = getBaseNode ("bitmaps");
	if (bitmapNodes)
	{
		for (UIDescList::iterator it = bitmapNodes->getChildren ().begin (); it != bitmapNodes->getChildren ().end (); it++)
		{
			UIBitmapNode* bitmapNode = dynamic_cast<UIBitmapNode*> (*it);
			if (bitmapNode)
			{
				if (flags & kWriteImagesIntoXMLFile)
					bitmapNode->createXMLData ();
				else
					bitmapNode->removeXMLData ();
				
			}
		}
	}
	nodes->getAttributes ()->setAttribute ("version", "1");
	UIDescWriter writer;
	return writer.write (stream, nodes);
}

//-----------------------------------------------------------------------------
UINode* UIDescription::findNodeForView (CView* view) const
{
	CView* parentView = view;
	std::string templateName;
	while (parentView && getTemplateNameFromView (parentView, templateName) == false)
		parentView = parentView->getParentView ();
	if (parentView)
	{
		UINode* node = 0;
		UIDescList::iterator it = nodes->getChildren ().begin ();
		while (it != nodes->getChildren ().end ())
		{
			if ((*it)->getName () == "template")
			{
				const std::string* nodeName = (*it)->getAttributes ()->getAttributeValue ("name");
				if (nodeName && *nodeName == templateName)
				{
					node = *it;
					break;
				}
			}
			it++;
		}
		if (node)
		{
			while (view != parentView)
			{
				if (view == parentView)
					return node;
				CViewContainer* container = dynamic_cast<CViewContainer*> (parentView);
				assert (container != 0);
				UIDescList::iterator nodeIterator = node->getChildren ().begin ();
				CViewContainer* childContainer = 0;
				ViewIterator it (container);
				while (*it && nodeIterator != node->getChildren ().end ())
				{
					if (*it == view)
					{
						node = *nodeIterator;
						parentView = view;
						break;
					}
					childContainer = dynamic_cast<CViewContainer*>(*it);
					if (childContainer && childContainer->isChild (view, true))
					{
						break;
					}
					childContainer = 0;
					nodeIterator++;
					it++;
				}
				if (childContainer)
				{
					node = *nodeIterator;
					parentView = childContainer;
				}
				else
				{
					break;
				}
			}
			if (view == parentView)
				return node;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
bool UIDescription::storeViews (const std::list<CView*> views, OutputStream& stream, UIAttributes* customData) const
{
	UIDescList nodeList (false);
	for (std::list<CView*>::const_iterator it = views.begin (); it != views.end (); it++)
	{
		UINode* node = findNodeForView (*it);
		if (node)
		{
			nodeList.add (node);
		}
		else
		{
		#if VSTGUI_LIVE_EDITING
			UIAttributes* attr = new UIAttributes;
			UIViewFactory* factory = dynamic_cast<UIViewFactory*> (viewFactory);
			if (factory)
			{
				if (factory->getAttributesForView (*it, const_cast<UIDescription*> (this), *attr) == false)
					return false;
				UINode* node = new UINode ("view", attr);
				nodeList.add (node);
				node->forget ();
			}
		#endif
		}
	}
	if (!nodeList.empty ())
	{
		if (customData)
		{
			UINode* customNode = new UINode ("custom", customData);
			nodeList.add (customNode);
			customNode->forget ();
			customData->remember ();
		}
		UINode baseNode ("vstgui-ui-description-view-list", &nodeList);
		UIDescWriter writer;
		return writer.write (stream, &baseNode);
	}
	return false;
}

//-----------------------------------------------------------------------------
bool UIDescription::restoreViews (InputStream& stream, std::list<SharedPointer<CView> >& views, UIAttributes** customData)
{
	UINode* baseNode = 0;
	if (nodes)
	{
		ScopePointer<UINode> sp (&nodes, baseNode);
		Xml::InputStreamContentProvider contentProvider (stream);
		Xml::Parser parser;
		if (parser.parse (&contentProvider, this))
		{
			baseNode = nodes;
		}
	}
	if (baseNode)
	{
		UIDescList& children = baseNode->getChildren ();
		for (UIDescList::iterator it = children.begin (); it != children.end (); it++)
		{
			if ((*it)->getName() == "custom")
			{
				if (customData)
				{
					*customData = (*it)->getAttributes ();
					(*customData)->remember ();
				}
			}
			else
			{
				CView* view = createViewFromNode (*it);
				if (view)
				{
					views.push_back (view);
					view->forget ();
				}
			}
		}
		baseNode->forget ();
	}
	return views.empty () == false;
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

	IController* subController = 0;
	CView* result = 0;
	if (controller)
	{
		const std::string* subControllerName = node->getAttributes ()->getAttributeValue ("sub-controller");
		if (subControllerName)
		{
			subController = controller->createSubController (subControllerName->c_str (), this);
			if (subController)
			{
				subControllerStack.push_back (controller);
				setController (subController);
			}
		}
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
		if (result == 0)
		{
			result = new CViewContainer (CRect (0, 0, 0, 0));
			UIViewFactory* _viewFactory = dynamic_cast<UIViewFactory*> (viewFactory);
			if (_viewFactory)
			{
				_viewFactory->applyCustomViewAttributeValues (result, "CViewContainer", *node->getAttributes (), this);
			}
		}
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
						attrId = (CViewAttributeID)strtol (attrName->c_str (), 0, 10);
					if (attrId)
						result->setAttribute (attrId, (int32_t)attrValue->size ()+1, attrValue->c_str ());
				}
			}
			it++;
		}
	}
	if (result && controller)
		result = controller->verifyView (result, *node->getAttributes (), this);
	if (subController)
	{
		if (result)
			result->setAttribute (kCViewControllerAttribute, sizeof (IController*), &subController);
		setController (subControllerStack.back ());
		subControllerStack.pop_back ();
		if (result == 0)
		{
			CBaseObject* obj = dynamic_cast<CBaseObject*> (subController);
			if (obj)
				obj->forget ();
			else
				delete subController;
		}
	}
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
				if (nodeName && *nodeName == name)
				{
					CView* view = createViewFromNode (*it);
					if (view)
						view->setAttribute (kTemplateNameAttributeID, (int32_t)strlen (name)+1, name);
					return view;
				}
			}
			it++;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
bool UIDescription::getTemplateNameFromView (CView* view, std::string& templateName) const
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
				if (nodeName && *nodeName == name)
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
int32_t UIDescription::getTagForName (UTF8StringPtr name) const
{
	int32_t tag = -1;
	UIControlTagNode* controlTagNode = dynamic_cast<UIControlTagNode*> (findChildNodeByNameAttribute (getBaseNode ("control-tags"), name));
	if (controlTagNode)
	{
		tag = controlTagNode->getTag ();
		if (tag == -1)
		{
			const std::string* tagStr = controlTagNode->getTagString ();
			if (tagStr)
			{
				double value;
				if (calculateStringValue (tagStr->c_str (), value))
				{
					tag = (int32_t)value;
					controlTagNode->setTag (tag);
				}
			}
		}
	}
	if (controller)
		tag = controller->getTagForName (name, tag);
	return tag;
}

//-----------------------------------------------------------------------------
bool UIDescription::hasColorName (UTF8StringPtr name)
{
	UIColorNode* node = dynamic_cast<UIColorNode*> (findChildNodeByNameAttribute (getBaseNode ("colors"), name));
	return node ? true : false;
}

//-----------------------------------------------------------------------------
bool UIDescription::hasTagName (UTF8StringPtr name)
{
	UIControlTagNode* node = dynamic_cast<UIControlTagNode*> (findChildNodeByNameAttribute (getBaseNode ("control-tags"), name));
	return node ? true : false;
}

//-----------------------------------------------------------------------------
bool UIDescription::hasFontName (UTF8StringPtr name)
{
	UIFontNode* node = dynamic_cast<UIFontNode*> (findChildNodeByNameAttribute (getBaseNode ("fonts"), name));
	return node ? true : false;
}

//-----------------------------------------------------------------------------
bool UIDescription::hasBitmapName (UTF8StringPtr name)
{
	UIBitmapNode* node = dynamic_cast<UIBitmapNode*> (findChildNodeByNameAttribute (getBaseNode ("bitmaps"), name));
	return node ? true : false;
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
		CBitmap* bitmap = bitmapNode->getBitmap ();
		if (bitmapCreator && bitmap && bitmap->getPlatformBitmap () == 0)
		{
			IPlatformBitmap* platformBitmap = bitmapCreator->createBitmap (*bitmapNode->getAttributes ());
			if (platformBitmap)
			{
				bitmap->setPlatformBitmap (platformBitmap);
				platformBitmap->forget ();
			}
		}
		if (bitmap && bitmapNode->getFilterProcessed () == false)
		{
			std::list<OwningPointer<BitmapFilter::IFilter> > filters;
			for (UIDescList::iterator it = bitmapNode->getChildren ().begin (); it != bitmapNode->getChildren ().end (); it++)
			{
				const std::string* filterName = 0;
				if ((*it)->getName () == "filter" && (filterName = (*it)->getAttributes ()->getAttributeValue ("name")))
				{
					BitmapFilter::IFilter* filter = BitmapFilter::Factory::getInstance().createFilter (filterName->c_str ());
					if (filter == 0)
						continue;
					filters.push_back (filter);
					for (UIDescList::iterator it2 = (*it)->getChildren ().begin (); it2 != (*it)->getChildren ().end (); it2++)
					{
						if ((*it2)->getName () != "property")
							continue;
						const std::string* name = (*it2)->getAttributes ()->getAttributeValue ("name");
						if (name == 0)
							continue;
						switch (filter->getProperty (name->c_str ()).getType ())
						{
							case BitmapFilter::Property::kInteger:
							{
								int32_t intValue;
								if ((*it2)->getAttributes ()->getIntegerAttribute ("value", intValue))
									filter->setProperty (name->c_str (), intValue);
								break;
							}
							case BitmapFilter::Property::kFloat:
							{
								double floatValue;
								if ((*it2)->getAttributes ()->getDoubleAttribute ("value", floatValue))
									filter->setProperty (name->c_str (), floatValue);
								break;
							}
							case BitmapFilter::Property::kPoint:
							{
								CPoint pointValue;
								if ((*it2)->getAttributes ()->getPointAttribute ("value", pointValue))
									filter->setProperty (name->c_str (), pointValue);
								break;
							}
							case BitmapFilter::Property::kRect:
							{
								CRect rectValue;
								if ((*it2)->getAttributes ()->getRectAttribute ("value", rectValue))
									filter->setProperty (name->c_str (), rectValue);
								break;
							}
							case BitmapFilter::Property::kColor:
							{
								const std::string* colorString = (*it2)->getAttributes()->getAttributeValue ("value");
								if (colorString)
								{
									CColor color;
									if (getColor (colorString->c_str (), color))
										filter->setProperty(name->c_str (), color);
								}
								break;
							}
							case BitmapFilter::Property::kTransformMatrix:
							{
								// TODO
								break;
							}
							case BitmapFilter::Property::kObject: // objects can not be stored/restored
							case BitmapFilter::Property::kNotFound:
								break;
						}
					}
				}
			}
			for (std::list<OwningPointer<BitmapFilter::IFilter> >::const_iterator it = filters.begin (); it != filters.end (); it++)
			{
				(*it)->setProperty (BitmapFilter::Standard::Property::kInputBitmap, bitmap);
				if ((*it)->run ())
				{
					CBaseObject* obj = (*it)->getProperty (BitmapFilter::Standard::Property::kOutputBitmap).getObject ();
					CBitmap* outputBitmap = dynamic_cast<CBitmap*>(obj);
					if (outputBitmap)
					{
						bitmap->setPlatformBitmap (outputBitmap->getPlatformBitmap ());
					}
				}
			}
			bitmapNode->setFilterProcessed ();
		}
		return bitmap;
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
			if (node && node->getFont () && *node->getFont () == *font)
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
			if (node)
			{
				int32_t nodeTag = node->getTag ();
				if (nodeTag == -1 && node->getTagString ())
				{
					double v;
					if (calculateStringValue (node->getTagString ()->c_str (), v))
						nodeTag = (int32_t)v;
				}
				if (nodeTag == tag)
				{
					const std::string* tagName = node->getAttributes ()->getAttributeValue ("name");
					return tagName ? tagName->c_str () : 0;
				}
			}
			it++;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
void UIDescription::changeColorName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	UINode* colorNodes = getBaseNode ("colors");
	UIColorNode* node = dynamic_cast<UIColorNode*> (findChildNodeByNameAttribute (colorNodes, oldName));
	if (node)
	{
		node->getAttributes ()->setAttribute ("name", newName);
		colorNodes->sortChildren ();
		changed (kMessageColorChanged);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeTagName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	UINode* controlTags = getBaseNode ("control-tags");
	UIControlTagNode* node = dynamic_cast<UIControlTagNode*> (findChildNodeByNameAttribute (controlTags, oldName));
	if (node)
	{
		node->getAttributes ()->setAttribute ("name", newName);
		controlTags->sortChildren ();
		changed (kMessageTagChanged);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeFontName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	UINode* fontNodes = getBaseNode ("fonts");
	UIFontNode* node = dynamic_cast<UIFontNode*> (findChildNodeByNameAttribute (fontNodes, oldName));
	if (node)
	{
		node->getAttributes ()->setAttribute ("name", newName);
		fontNodes->sortChildren ();
		changed (kMessageFontChanged);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeBitmapName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	UINode* bitmapNodes = getBaseNode ("bitmaps");
	UIBitmapNode* node = dynamic_cast<UIBitmapNode*> (findChildNodeByNameAttribute (bitmapNodes, oldName));
	if (node)
	{
		node->getAttributes ()->setAttribute ("name", newName);
		bitmapNodes->sortChildren ();
		changed (kMessageBitmapChanged);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeColor (UTF8StringPtr name, const CColor& newColor)
{
	UINode* colorsNode = getBaseNode ("colors");
	UIColorNode* node = dynamic_cast<UIColorNode*> (findChildNodeByNameAttribute (colorsNode, name));
	if (node)
	{
		if (!node->noExport ())
		{
			node->setColor (newColor);
			changed (kMessageColorChanged);
		}
	}
	else
	{
		if (colorsNode)
		{
			UIAttributes* attr = new UIAttributes;
			attr->setAttribute ("name", name);
			std::string colorStr;
			UIViewCreator::colorToString (newColor, colorStr, 0);
			attr->setAttribute ("rgba", colorStr.c_str ());
			UIColorNode* node = new UIColorNode ("color", attr);
			colorsNode->getChildren ().add (node);
			colorsNode->sortChildren ();
			changed (kMessageColorChanged);
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeFont (UTF8StringPtr name, CFontRef newFont)
{
	UINode* fontsNode = getBaseNode ("fonts");
	UIFontNode* node = dynamic_cast<UIFontNode*> (findChildNodeByNameAttribute (fontsNode, name));
	if (node)
	{
		if (!node->noExport ())
		{
			node->setFont (newFont);
			changed (kMessageFontChanged);
		}
	}
	else
	{
		if (fontsNode)
		{
			UIAttributes* attr = new UIAttributes;
			attr->setAttribute ("name", name);
			UIFontNode* node = new UIFontNode ("font", attr);
			node->setFont (newFont);
			fontsNode->getChildren ().add (node);
			fontsNode->sortChildren ();
			changed (kMessageFontChanged);
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeBitmap (UTF8StringPtr name, UTF8StringPtr newName, const CRect* nineparttiledOffset)
{
	UINode* bitmapsNode = getBaseNode ("bitmaps");
	UIBitmapNode* node = dynamic_cast<UIBitmapNode*> (findChildNodeByNameAttribute (bitmapsNode, name));
	if (node)
	{
		if (!node->noExport ())
		{
			node->setBitmap (newName);
			node->setNinePartTiledOffset (nineparttiledOffset);
			changed (kMessageBitmapChanged);
		}
	}
	else
	{
		if (bitmapsNode)
		{
			UIAttributes* attr = new UIAttributes;
			attr->setAttribute ("name", name);
			UIBitmapNode* node = new UIBitmapNode ("bitmap", attr);
			if (nineparttiledOffset)
				node->setNinePartTiledOffset (nineparttiledOffset);
			node->setBitmap (newName);
			bitmapsNode->getChildren ().add (node);
			bitmapsNode->sortChildren ();
			changed (kMessageBitmapChanged);
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeBitmapFilters (UTF8StringPtr bitmapName, const std::list<SharedPointer<UIAttributes> >& filters)
{
	UIBitmapNode* bitmapNode = dynamic_cast<UIBitmapNode*> (findChildNodeByNameAttribute (getBaseNode ("bitmaps"), bitmapName));
	if (bitmapNode)
	{
		bitmapNode->getChildren().removeAll ();
		for (std::list<SharedPointer<UIAttributes> >::const_iterator it = filters.begin (); it != filters.end (); it++)
		{
			const std::string* filterName = (*it)->getAttributeValue ("name");
			if (filterName == 0)
				continue;
			UINode* filterNode = new UINode ("filter");
			filterNode->getAttributes ()->setAttribute ("name", filterName->c_str ());
			for (UIAttributes::const_iterator it2 = (*it)->begin (); it2 != (*it)->end (); it2++)
			{
				if ((*it2).first == "name")
					continue;
				UINode* propertyNode = new UINode ("property");
				propertyNode->getAttributes ()->setAttribute("name", (*it2).first.c_str ());
				propertyNode->getAttributes ()->setAttribute("value", (*it2).second.c_str ());
				filterNode->getChildren ().add (propertyNode);
			}
			bitmapNode->getChildren ().add (filterNode);
		}
		bitmapNode->invalidBitmap ();
		changed (kMessageBitmapChanged);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::collectBitmapFilters (UTF8StringPtr bitmapName, std::list<SharedPointer<UIAttributes> >& filters) const
{
	UIBitmapNode* bitmapNode = dynamic_cast<UIBitmapNode*> (findChildNodeByNameAttribute (getBaseNode ("bitmaps"), bitmapName));
	if (bitmapNode)
	{
		for (UIDescList::iterator it = bitmapNode->getChildren ().begin (); it != bitmapNode->getChildren ().end (); it++)
		{
			if ((*it)->getName () == "filter")
			{
				const std::string* filterName = (*it)->getAttributes ()->getAttributeValue ("name");
				if (filterName == 0)
					continue;
				UIAttributes* attributes = new UIAttributes ();
				attributes->setAttribute ("name", filterName->c_str ());
				for (UIDescList::iterator it2 = (*it)->getChildren ().begin (); it2 != (*it)->getChildren ().end (); it2++)
				{
					if ((*it2)->getName () == "property")
					{
						const std::string* name = (*it2)->getAttributes ()->getAttributeValue ("name");
						const std::string* value = (*it2)->getAttributes ()->getAttributeValue ("value");
						if (name && value)
						{
							attributes->setAttribute (name->c_str (), value->c_str ());
						}
					}
				}
				filters.push_back (attributes);
				attributes->forget ();
			}
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
		if (name && *name == nodeName)
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
	{
		removeChildNode (node, name);
		changed (kMessageColorChanged);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::removeTag (UTF8StringPtr name)
{
	UINode* node = getBaseNode ("control-tags");
	if (node)
	{
		removeChildNode (node, name);
		changed (kMessageTagChanged);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::removeFont (UTF8StringPtr name)
{
	UINode* node = getBaseNode ("fonts");
	if (node)
	{
		removeChildNode (node, name);
		changed (kMessageFontChanged);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::removeBitmap (UTF8StringPtr name)
{
	UINode* node = getBaseNode ("bitmaps");
	if (node)
	{
		removeChildNode (node, name);
		changed (kMessageBitmapChanged);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeAlternativeFontNames (UTF8StringPtr name, UTF8StringPtr alternativeFonts)
{
	UIFontNode* node = dynamic_cast<UIFontNode*> (findChildNodeByNameAttribute (getBaseNode ("fonts"), name));
	if (node)
	{
		node->setAlternativeFontNames (alternativeFonts);
		changed (kMessageFontChanged);
	}
}

//-----------------------------------------------------------------------------
bool UIDescription::getAlternativeFontNames (UTF8StringPtr name, std::string& alternativeFonts)
{
	UIFontNode* node = dynamic_cast<UIFontNode*> (findChildNodeByNameAttribute (getBaseNode ("fonts"), name));
	if (node)
	{
		if (node->getAlternativeFontNames (alternativeFonts))
			return true;
	}
	return false;
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
bool UIDescription::updateAttributesForView (UINode* node, CView* view, bool deep)
{
	bool result = false;
#if VSTGUI_LIVE_EDITING
	UIViewFactory* factory = dynamic_cast<UIViewFactory*> (viewFactory);
	std::list<std::string> attributeNames;
	CViewContainer* container = dynamic_cast<CViewContainer*> (view);
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
		result = true;
	}
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
				// check if subview is created via UIDescription
				// if it is, it's just added to this node
				UINode* subNode = new UINode ("view");
				if (updateAttributesForView (subNode, subView))
				{
					node->getChildren ().add (subNode);
				}
				else
				{
					// if it is not, we check if it has children. This can happen per example for a CScrollView for its container view.
					if (subNode->getChildren ().total () > 0)
					{
						for (UIDescList::iterator it = subNode->getChildren ().begin (); it != subNode->getChildren ().end (); it++)
						{
							(*it)->remember ();
							node->getChildren ().add (*it);
						}
					}
					subNode->forget ();
				}
			}
			++it;
		}
	}
#endif
	return result;
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
			node = new UINode ("template");
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
		addDefaultNodes ();
	}
	UINode* templateNode = findChildNodeByNameAttribute (nodes, name);
	if (templateNode == 0)
	{
		UINode* newNode = new UINode ("template", attr);
		attr->setAttribute ("name", name);
		nodes->getChildren ().add (newNode);
		changed (kMessageTemplateChanged);
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
		changed (kMessageTemplateChanged);
		return true;
	}
#endif
	return false;
}

//-----------------------------------------------------------------------------
bool UIDescription::changeTemplateName (UTF8StringPtr name, UTF8StringPtr newName)
{
#if VSTGUI_LIVE_EDITING
	UINode* templateNode = findChildNodeByNameAttribute (nodes, name);
	if (templateNode)
	{
		templateNode->getAttributes()->setAttribute ("name", newName);
		changed (kMessageTemplateChanged);
		return true;
	}
#endif
	return false;
}

//-----------------------------------------------------------------------------
bool UIDescription::duplicateTemplate (UTF8StringPtr name, UTF8StringPtr duplicateName)
{
#if VSTGUI_LIVE_EDITING
	UINode* templateNode = findChildNodeByNameAttribute (nodes, name);
	if (templateNode)
	{
		UINode* duplicate = static_cast<UINode*> (templateNode->newCopy ());
		if (duplicate)
		{
			duplicate->getAttributes()->setAttribute ("name", duplicateName);
			nodes->getChildren ().add (duplicate);
			changed (kMessageTemplateChanged);
			return true;
		}
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
UIAttributes* UIDescription::getCustomAttributes (UTF8StringPtr name, bool create)
{
	UINode* customNode = findChildNodeByNameAttribute (getBaseNode ("custom"), name);
	if (customNode)
		return customNode->getAttributes ();
	if (create)
	{
		UIAttributes* attributes = new UIAttributes ();
		setCustomAttributes (name, attributes);
		return attributes;
	}
	return 0;
}

//-----------------------------------------------------------------------------
bool UIDescription::getControlTagString (UTF8StringPtr tagName, std::string& tagString) const
{
	UIControlTagNode* controlTagNode = dynamic_cast<UIControlTagNode*> (findChildNodeByNameAttribute (getBaseNode ("control-tags"), tagName));
	if (controlTagNode)
	{
		const std::string* tagStr = controlTagNode->getTagString ();
		if (tagStr)
		{
			tagString = *tagStr;
			return true;
		}		
	}
	return false;
}

//-----------------------------------------------------------------------------
bool UIDescription::changeControlTagString  (UTF8StringPtr tagName, const std::string& newTagString, bool create)
{
	UINode* tagsNode = getBaseNode ("control-tags");
	UIControlTagNode* controlTagNode = dynamic_cast<UIControlTagNode*> (findChildNodeByNameAttribute (tagsNode, tagName));
	if (controlTagNode)
	{
		if (create)
			return false;
		controlTagNode->setTagString (newTagString);
		changed (kMessageTagChanged);
		return true;
	}
	if (create)
	{
		if (tagsNode)
		{
			UIAttributes* attr = new UIAttributes;
			attr->setAttribute ("name", tagName);
			UIControlTagNode* node = new UIControlTagNode ("control-tag", attr);
			node->setTagString(newTagString);
			tagsNode->getChildren ().add (node);
			tagsNode->sortChildren ();
			changed (kMessageTagChanged);
			return true;
		}
	}
	return false;	
}

//-----------------------------------------------------------------------------
bool UIDescription::getVariable (UTF8StringPtr name, double& value) const
{
	UIVariableNode* node = dynamic_cast<UIVariableNode*> (findChildNodeByNameAttribute (getBaseNode ("variables"), name));
	if (node)
	{
		if (node->getType () == UIVariableNode::kNumber)
		{
			value = node->getNumber ();
			return true;
		}
		if (node->getType () == UIVariableNode::kString)
		{
			double v;
			if (calculateStringValue (node->getString ().c_str (), v))
			{
				value = v;
				return true;
			}
		}
	}
	return false;
}

namespace UIDescriptionPrivate {

//-----------------------------------------------------------------------------
class StringToken : public std::string
{
public:
	enum Type {
		kString,
		kAdd,
		kSubtract,
		kMulitply,
		kDivide,
		kOpenParenthesis,
		kCloseParenthesis,
		kResult
	};
	
	StringToken (const std::string& str) : std::string (str), type (kString), result (0) {}
	StringToken (const StringToken& token) : std::string (token), type (token.type), result (token.result) {}
	StringToken (Type type, double value = 0) : type (type), result (value) {}
	
	Type type;
	double result;
};

//-----------------------------------------------------------------------------
static bool tokenizeString (std::string& str, std::list<StringToken>& tokens)
{
	UTF8CharacterIterator iterator (str);
	uint8_t* tokenStart = iterator;
	do {
		if (iterator.getByteLength () == 1)
		{
			uint8_t character = *iterator;
			if (isspace (character))
			{
				if (tokenStart != iterator)
				{
					std::string token ((const char*)tokenStart, iterator - tokenStart);
					tokens.push_back (token);
				}
				tokenStart = iterator + 1;
			}
			else
			{
				switch (character)
				{
					case '+':
					{
						if (tokenStart != iterator)
						{
							std::string token ((const char*)tokenStart, iterator - tokenStart);
							tokens.push_back (token);
						}
						tokens.push_back (StringToken (StringToken::kAdd));
						tokenStart = iterator + 1;
						break;
					}
					case '-':
					{
						if (tokenStart != iterator)
						{
							std::string token ((const char*)tokenStart, iterator - tokenStart);
							tokens.push_back (token);
						}
						tokens.push_back (StringToken (StringToken::kSubtract));
						tokenStart = iterator + 1;
						break;
					}
					case '*':
					{
						if (tokenStart != iterator)
						{
							std::string token ((const char*)tokenStart, iterator - tokenStart);
							tokens.push_back (token);
						}
						tokens.push_back (StringToken (StringToken::kMulitply));
						tokenStart = iterator + 1;
						break;
					}
					case '/':
					{
						if (tokenStart != iterator)
						{
							std::string token ((const char*)tokenStart, iterator - tokenStart);
							tokens.push_back (token);
						}
						tokens.push_back (StringToken (StringToken::kDivide));
						tokenStart = iterator + 1;
						break;
					}
					case '(':
					{
						if (tokenStart != iterator)
						{
							std::string token ((const char*)tokenStart, iterator - tokenStart);
							tokens.push_back (token);
						}
						tokens.push_back (StringToken (StringToken::kOpenParenthesis));
						tokenStart = iterator + 1;
						break;
					}
					case ')':
					{
						if (tokenStart != iterator)
						{
							std::string token ((const char*)tokenStart, iterator - tokenStart);
							tokens.push_back (token);
						}
						tokens.push_back (StringToken (StringToken::kCloseParenthesis));
						tokenStart = iterator + 1;
						break;
					}
				}
			}
		}
	} while (iterator.next () != iterator.back ());
	if (tokenStart != iterator)
	{
		std::string token ((const char*)tokenStart, iterator - tokenStart);
		tokens.push_back (token);
	}
	return true;	
}

//-----------------------------------------------------------------------------
static bool computeTokens (std::list<StringToken>& tokens, double& result)
{
	int32_t openCount = 0;
	std::list<UIDescriptionPrivate::StringToken>::iterator openPosition = tokens.end ();
	// first check parentheses
	for (std::list<UIDescriptionPrivate::StringToken>::iterator it = tokens.begin (); it != tokens.end (); it++)
	{
		if ((*it).type == StringToken::kOpenParenthesis)
		{
			openCount++;
			if (openCount == 1)
				openPosition = it;
		}
		else if ((*it).type == StringToken::kCloseParenthesis)
		{
			openCount--;
			if (openCount == 0)
			{
				std::list<StringToken> tmp (++openPosition, it);
				double value = 0;
				if (computeTokens (tmp, value))
				{
					openPosition--;
					it++;
					tokens.erase (openPosition, it);
					tokens.insert (it, StringToken (StringToken::kResult, value));
					if (it == tokens.end ())
					{
						break;
					}

				}
				else
					return false;
			}
		}
	}
	// now multiply and divide
	std::list<UIDescriptionPrivate::StringToken>::iterator prevToken = tokens.begin ();
	for (std::list<UIDescriptionPrivate::StringToken>::iterator it = tokens.begin (); it != tokens.end (); it++)
	{
		if (prevToken != it)
		{
			if ((*it).type == StringToken::kMulitply)
			{
				if ((*prevToken).type == StringToken::kResult)
				{
					it++;
					if ((*it).type == StringToken::kResult)
					{
						double value = (*prevToken).result * (*it).result;
						it++;
						tokens.erase (prevToken, it);
						tokens.insert (it, StringToken (StringToken::kResult, value));
						it--;
						prevToken = it;
					}
					else
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			else if ((*it).type == StringToken::kDivide)
			{
				if ((*prevToken).type == StringToken::kResult)
				{
					it++;
					if ((*it).type == StringToken::kResult)
					{
						double value = (*prevToken).result / (*it).result;
						it++;
						tokens.erase (prevToken, it);
						tokens.insert (it, StringToken (StringToken::kResult, value));
						it--;
						prevToken = it;
					}
					else
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			else
			{
				prevToken = it;
			}
		}
	}
	// now add and subtract
	int32_t lastType = -1;
	for (std::list<UIDescriptionPrivate::StringToken>::const_iterator it = tokens.begin (); it != tokens.end (); it++)
	{
		if ((*it).type == UIDescriptionPrivate::StringToken::kResult)
		{
			double value = (*it).result;
			if (lastType == -1)
				result = value;
			else if (lastType == StringToken::kAdd)
				result += value;
			else if (lastType == StringToken::kSubtract)
				result -= value;
			else
			{
			#if DEBUG
				DebugPrint ("Wrong Expression: %d\n", (*it).type);
			#endif
				return false;
			}
		}
		else if (!(lastType == -1 || lastType == UIDescriptionPrivate::StringToken::kResult))
		{
		#if DEBUG
			DebugPrint ("Wrong Expression: %d\n", (*it).type);
		#endif
			return false;
		}
		lastType = (*it).type;
		
	}	
	return true;	
}

} // namespace UIDescriptionPrivate

//-----------------------------------------------------------------------------
bool UIDescription::calculateStringValue (UTF8StringPtr _str, double& result) const
{
	char* endPtr = 0;
	result = strtod (_str, &endPtr);
	if (endPtr == _str + strlen (_str))
		return true;
	std::string str (_str);
	std::list<UIDescriptionPrivate::StringToken> tokens;
	if (!UIDescriptionPrivate::tokenizeString (str, tokens))
	{
	#if DEBUG
		DebugPrint("TokenizeString failed :%s\n", _str);
	#endif
		return false;
	}
	// first make substituation
	for (std::list<UIDescriptionPrivate::StringToken>::iterator it = tokens.begin (); it != tokens.end (); it++)
	{
		if ((*it).type == UIDescriptionPrivate::StringToken::kString)
		{
			const char* tokenStr = (*it).c_str ();
			double value = strtod (tokenStr, &endPtr);
			if (endPtr != tokenStr + (*it).length ())
			{
				// if it is not pure numeric try to substitute the string with a control tag or variable
				size_t pos;
				if ((pos = (*it).find ("tag.")) == 0)
				{
					value = getTagForName ((*it).c_str () + 4);
					if (value == -1)
					{
					#if DEBUG
						DebugPrint("Tag not found :%s\n", tokenStr);
					#endif
						return false;
					}
				}
				else if ((pos = (*it).find ("var.")) == 0)
				{
					double v;
					if (getVariable ((*it).c_str () + 4, v))
					{
						value = v;
					}
					else
					{
					#if DEBUG
						DebugPrint("Variable not found :%s\n", tokenStr);
					#endif
						return false;
					}
				}
				else
				{
				#if DEBUG
					DebugPrint("Substitution failed :%s\n", tokenStr);
				#endif
					return false;
				}
			}
			(*it).result = value;
			(*it).type = UIDescriptionPrivate::StringToken::kResult;
		}
	}
	result = 0;
	return UIDescriptionPrivate::computeTokens (tokens, result);
}

//-----------------------------------------------------------------------------
void UIDescription::startXmlElement (Xml::Parser* parser, IdStringPtr elementName, UTF8StringPtr* elementAttributes)
{
	std::string name (elementName);
	if (nodes)
	{
		UINode* parent = nodeStack.back ();
		UINode* newNode = 0;
		if (restoreViewsMode)
		{
			if (name != "view" && name != "custom")
			{
				parser->stop ();
			}
			newNode = new UINode (name, new UIAttributes (elementAttributes));
		}
		else
		{
			if (parent == nodes)
			{
				// only allowed second level elements
				if (name == "bitmaps" || name == "fonts" || name == "colors" || name == "template" || name == "control-tags" || name == "custom" || name == "variables")
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
			else if (parent->getName () == "variables")
			{
				if (name == "var")
					newNode = new UIVariableNode (name, new UIAttributes (elementAttributes));
				else
					parser->stop ();
			}
			else
				newNode = new UINode (name, new UIAttributes (elementAttributes));
		}
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
	else if (name == "vstgui-ui-description-view-list")
	{
		assert (nodes == 0);
		nodes = new UINode (name, new UIAttributes (elementAttributes));
		nodeStack.push_back (nodes);
		restoreViewsMode = true;
	}
}

//-----------------------------------------------------------------------------
void UIDescription::endXmlElement (Xml::Parser* parser, IdStringPtr name)
{
	if (nodeStack.back () == nodes)
		restoreViewsMode = false;
	nodeStack.pop_back ();
}

//-----------------------------------------------------------------------------
void UIDescription::xmlCharData (Xml::Parser* parser, const int8_t* data, int32_t length)
{
	if (nodeStack.back () != 0)
	{
		std::stringstream& sstream = nodeStack.back ()->getData ();
		for (int32_t i = 0; i < length; i++)
		{
			if (data[i] == '\t' || data[i] == '\n' || data[i] == '\r')
				continue;
			sstream << (char)data[i];
		}
	}
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
	data.clear ();
	if (attributes == 0)
		attributes = new UIAttributes ();
}

//-----------------------------------------------------------------------------
UINode::UINode (const std::string& _name, UIDescList* _children, UIAttributes* _attributes)
: name (_name)
, children (_children)
, attributes (_attributes)
, flags (0)
{
	assert (children != 0);
	data.clear ();
	children->remember ();
	if (attributes == 0)
		attributes = new UIAttributes ();
}

//-----------------------------------------------------------------------------
UINode::UINode (const UINode& n)
: name (n.name)
, flags (n.flags)
, attributes (new UIAttributes (*n.attributes))
, children (new UIDescList (*n.children))
{
	data.clear ();
	data << n.getData ().str ();
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
void UINode::sortChildren ()
{
	children->sort ();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UIVariableNode::UIVariableNode (const std::string& name, UIAttributes* attributes)
: UINode (name, attributes)
, number (0)
, type (kUnknown)
{
	const std::string* typeStr = attributes->getAttributeValue ("type");
	const std::string* valueStr = attributes->getAttributeValue ("value");
	if (typeStr)
	{
		if (*typeStr == "number")
			type = kNumber;
		else if (*typeStr == "string")
			type = kString;
	}
	if (valueStr)
	{
		const char* strPtr = valueStr->c_str ();
		if (type == kUnknown)
		{
			char* endPtr = 0;
			double numberCheck = strtod (strPtr, &endPtr);
			if (endPtr == strPtr + strlen (strPtr))
			{
				number = numberCheck;
				type = kNumber;
			}
			else
				type = kString;
		}
		else if (type == kNumber)
		{
			number = strtod (strPtr, 0);
		}
	}
}

//-----------------------------------------------------------------------------
UIVariableNode::UIVariableNode (const UIVariableNode& n)
: UINode (n)
, type (n.type)
, number (n.number)
{
}

//-----------------------------------------------------------------------------
UIVariableNode::Type UIVariableNode::getType () const
{
	return type;
}

//-----------------------------------------------------------------------------
double UIVariableNode::getNumber () const
{
	return number;
}

//-----------------------------------------------------------------------------
const std::string& UIVariableNode::getString () const
{
	const std::string* value = attributes->getAttributeValue ("value");
	if (value)
		return *value;
	static std::string kEmpty;
	return kEmpty;
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
UIControlTagNode::UIControlTagNode (const UIControlTagNode& n)
: UINode (n)
, tag (n.tag)
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
			{
				char* endPtr = 0;
				tag = (int32_t)strtol (tagStr->c_str (), &endPtr, 10);
				if (endPtr != tagStr->c_str () + tagStr->length ())
					tag = -1;
			}
		}
	}
	return tag;
}

//-----------------------------------------------------------------------------
void UIControlTagNode::setTag (int32_t newTag)
{
	tag = newTag;
}

//-----------------------------------------------------------------------------
const std::string* UIControlTagNode::getTagString () const
{
	return attributes->getAttributeValue ("tag");
}

//-----------------------------------------------------------------------------
void UIControlTagNode::setTagString (const std::string& str)
{
	attributes->setAttribute ("tag", str.c_str ());
	tag = -1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UIBitmapNode::UIBitmapNode (const std::string& name, UIAttributes* attributes)
: UINode (name, attributes)
, bitmap (0)
, filterProcessed (false)
{
}

//-----------------------------------------------------------------------------
UIBitmapNode::UIBitmapNode (const UIBitmapNode& n)
: UINode (n)
, bitmap (n.bitmap)
, filterProcessed (n.filterProcessed)
{
	if (bitmap)
		bitmap->remember ();
}

//-----------------------------------------------------------------------------
UIBitmapNode::~UIBitmapNode ()
{
	if (bitmap)
		bitmap->forget ();
}

//-----------------------------------------------------------------------------
void UIBitmapNode::createXMLData ()
{
	CBitmap* bitmap = getBitmap ();
	if (bitmap)
	{
		IPlatformBitmap* platformBitmap = bitmap->getPlatformBitmap ();
		if (platformBitmap)
		{
			void* data;
			uint32_t dataSize;
			if (IPlatformBitmap::createMemoryPNGRepresentation (platformBitmap, &data, dataSize))
			{
				Base64Codec bd;
				if (bd.init (data, dataSize))
				{
					UIDescList::iterator data = getChildren ().get ("data");
					if (data != getChildren ().end ())
						getChildren ().remove (*data);
					UINode* dataNode = new UINode ("data");
					dataNode->getAttributes ()->setAttribute ("encoding", "base64");
					dataNode->getData ().write ((const char*)bd.getData (), bd.getDataSize ());
					getChildren ().add (dataNode);
				}
				free (data);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void UIBitmapNode::removeXMLData ()
{
	UIDescList::iterator data = getChildren ().get ("data");
	if (data != getChildren ().end ())
		getChildren ().remove (*data);
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
				bitmap = new CNinePartTiledBitmap (CResourceDescription (path->c_str ()), CNinePartTiledBitmap::PartOffsets (offsets.left, offsets.top, offsets.right, offsets.bottom));
			}
			else
			{
				bitmap = new CBitmap (CResourceDescription (path->c_str ()));
			}
		}
		if (bitmap && bitmap->getPlatformBitmap () == 0)
		{
			UIDescList::iterator data = getChildren ().get ("data");
			if (data != getChildren ().end () && (*data)->getData ().str ().length () > 0)
			{
				const std::string* codec = (*data)->getAttributes ()->getAttributeValue ("encoding");
				if (codec && *codec == "base64")
				{
					Base64Codec bd;
					if (bd.init ((*data)->getData ().str ()))
					{
						OwningPointer<IPlatformBitmap> platformBitmap = IPlatformBitmap::createFromMemory (bd.getData (), bd.getDataSize ());
						if (platformBitmap)
						{
							bitmap->setPlatformBitmap (platformBitmap);
						}
					}
				}
			}
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
void UIBitmapNode::invalidBitmap ()
{
	if (bitmap)
		bitmap->forget ();
	bitmap = 0;
	filterProcessed = false;
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
UIFontNode::UIFontNode (const UIFontNode& n)
: UINode (n)
, font (n.font)
{
	if (font)
		font->remember ();
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
		const std::string* strikethroughAttr = attributes->getAttributeValue ("strike-through");
		if (nameAttr)
		{
			int32_t size = 12;
			if (sizeAttr)
				size = (int32_t)strtol (sizeAttr->c_str (), 0, 10);
			int32_t fontStyle = 0;
			if (boldAttr && *boldAttr == "true")
				fontStyle |= kBoldFace;
			if (italicAttr && *italicAttr == "true")
				fontStyle |= kItalicFace;
			if (underlineAttr && *underlineAttr == "true")
				fontStyle |= kUnderlineFace;
			if (strikethroughAttr && *strikethroughAttr == "true")
				fontStyle |= kStrikethroughFace;
			if (attributes->hasAttribute ("alternative-font-names"))
			{
				std::list<std::string> fontNames;
				if (IPlatformFont::getAllPlatformFontFamilies (fontNames))
				{
					if (std::find (fontNames.begin (), fontNames.end (), *nameAttr) == fontNames.end ())
					{
						std::vector<std::string> alternativeFontNames;
						attributes->getAttributeArray ("alternative-font-names", alternativeFontNames);
						for (std::vector<std::string>::const_iterator it = alternativeFontNames.begin (); it != alternativeFontNames.end (); it++)
						{
							if (std::find (fontNames.begin (), fontNames.end (), *it) != fontNames.end ())
							{
								font = new CFontDesc ((*it).c_str (), size, fontStyle);
								break;
							}
						}
					}
				}
			}
			if (font == 0)
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
	std::string alternativeNames;
	getAlternativeFontNames (alternativeNames);

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
	if (newFont->getStyle () & kStrikethroughFace)
		attributes->setAttribute ("strike-through", "true");

	setAlternativeFontNames (alternativeNames.c_str ());
}

//-----------------------------------------------------------------------------
void UIFontNode::setAlternativeFontNames (UTF8StringPtr fontNames)
{
	if (fontNames && fontNames[0] != 0)
	{
		attributes->setAttribute("alternative-font-names", fontNames);
	}
	else
	{
		attributes->removeAttribute ("alternative-font-names");
	}
}

//-----------------------------------------------------------------------------
bool UIFontNode::getAlternativeFontNames (std::string& fontNames)
{
	const std::string* value = attributes->getAttributeValue ("alternative-font-names");
	if (value)
	{
		fontNames = *value;
		return true;
	}
	return false;
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
UIColorNode::UIColorNode (const UIColorNode& n)
: UINode (n)
, color (n.color)
{
}

//-----------------------------------------------------------------------------
void UIColorNode::setColor (const CColor& newColor)
{
	std::string name (*attributes->getAttributeValue ("name"));
	attributes->removeAll ();
	attributes->setAttribute ("name", name.c_str ());

	std::string colorString;
	UIViewCreator::colorToString (newColor, colorString, 0);
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
void UIAttributes::setDoubleAttribute (UTF8StringPtr name, double value)
{
	std::stringstream str;
	str.precision (40);
	str << value;
	setAttribute (name, str.str ().c_str ());
}

//-----------------------------------------------------------------------------
bool UIAttributes::getDoubleAttribute (UTF8StringPtr name, double& value) const
{
	const std::string* str = getAttributeValue (name);
	if (str)
	{
		value = strtod (str->c_str (), 0);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void UIAttributes::setBooleanAttribute (UTF8StringPtr name, bool value)
{
	setAttribute (name, value ? "true" : "false");
}

//-----------------------------------------------------------------------------
bool UIAttributes::getBooleanAttribute (UTF8StringPtr name, bool& value) const
{
	const std::string* str = getAttributeValue (name);
	if (str)
	{
		if (*str == "true")
		{
			value = true;
			return true;
		}
		else if (*str == "false")
		{
			value = false;
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
void UIAttributes::setIntegerAttribute (UTF8StringPtr name, int32_t value)
{
	std::stringstream str;
	str << value;
	setAttribute (name, str.str ().c_str ());
}

//-----------------------------------------------------------------------------
bool UIAttributes::getIntegerAttribute (UTF8StringPtr name, int32_t& value) const
{
	const std::string* str = getAttributeValue (name);
	if (str)
	{
		value = (int32_t)strtol (str->c_str (), 0, 10);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void UIAttributes::setPointAttribute (UTF8StringPtr name, const CPoint& p)
{
	std::stringstream str;
	str << p.x;
	str << ", ";
	str << p.y;
	setAttribute (name, str.str ().c_str ());
}

//-----------------------------------------------------------------------------
bool UIAttributes::getPointAttribute (UTF8StringPtr name, CPoint& p) const
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
			if (subStrings.size () == 2)
			{
				p.x = strtod (subStrings[0].c_str (), 0);
				p.y = strtod (subStrings[1].c_str (), 0);
				return true;
			}
		}
	}
	return false;
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
void UIAttributes::setAttributeArray (UTF8StringPtr name, const std::vector<std::string>& values)
{
	std::string value;
	size_t numValues = values.size ();
	for (size_t i = 0; i < numValues - 1; i++)
	{
		value += values[i];
		value += ',';
	}
	value += values[numValues-1];
}

//-----------------------------------------------------------------------------
bool UIAttributes::getAttributeArray (UTF8StringPtr name, std::vector<std::string>& values) const
{
	const std::string* str = getAttributeValue (name);
	if (str)
	{
		std::stringstream ss (*str);
		std::string item;
		while (std::getline (ss, item, ','))
		{
			values.push_back(item);
		}
		return true;
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

} // namespace
