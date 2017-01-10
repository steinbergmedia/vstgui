//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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
#include "uiattributes.h"
#include "uiviewfactory.h"
#include "uiviewcreator.h"
#include "cstream.h"
#include "base64codec.h"
#include "../lib/cfont.h"
#include "../lib/cstring.h"
#include "../lib/cframe.h"
#include "../lib/cdrawcontext.h"
#include "../lib/cgradient.h"
#include "../lib/cgraphicspath.h"
#include "../lib/cbitmapfilter.h"
#include "../lib/platform/std_unorderedmap.h"
#include "../lib/platform/iplatformbitmap.h"
#include "../lib/platform/iplatformfont.h"
#include "detail/uiviewcreatorattributes.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cassert>

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

namespace MainNodeNames {
	static const IdStringPtr kBitmap = "bitmaps";
	static const IdStringPtr kFont = "fonts";
	static const IdStringPtr kColor = "colors";
	static const IdStringPtr kControlTag = "control-tags";
	static const IdStringPtr kVariable = "variables";
	static const IdStringPtr kTemplate = "template";
	static const IdStringPtr kCustom = "custom";
	static const IdStringPtr kGradient = "gradients";
}

class UINode;

typedef std::vector<UINode*> UIDescListContainerType;
//-----------------------------------------------------------------------------
class UIDescList : public CBaseObject, private UIDescListContainerType
{
public:
	using UIDescListContainerType::begin;
	using UIDescListContainerType::end;
	using UIDescListContainerType::rbegin;
	using UIDescListContainerType::rend;
	using UIDescListContainerType::iterator;
	using UIDescListContainerType::const_iterator;
	using UIDescListContainerType::const_reverse_iterator;
	using UIDescListContainerType::empty;
	using UIDescListContainerType::size;
	
	UIDescList (bool ownsObjects = true);
	UIDescList (const UIDescList& descList);
	virtual ~UIDescList ();

	virtual void add (UINode* obj);
	virtual void remove (UINode* obj);
	virtual void removeAll ();
	virtual UINode* findChildNode (const std::string& nodeName) const;
	virtual UINode* findChildNodeWithAttributeValue (const std::string& attributeName, const std::string& attributeValue) const;

	virtual void nodeAttributeChanged (UINode* child, const std::string& attributeName, const std::string& oldAttributeValue) {}

	void sort ();
	
	CLASS_METHODS(UIDescList, CBaseObject)
protected:
	bool ownsObjects;
};

//-----------------------------------------------------------------------------
class UINode : public CBaseObject
{
public:
	UINode (const std::string& name, UIAttributes* attributes = 0, bool needsFastChildNameAttributeLookup = false);
	UINode (const std::string& name, UIDescList* children, UIAttributes* attributes = 0);
	UINode (const UINode& n);
	~UINode ();

	const std::string& getName () const { return name; }
	std::stringstream& getData () { return data; }
	const std::stringstream& getData () const { return data; }

	UIAttributes* getAttributes () const { return attributes; }
	UIDescList& getChildren () const { return *children; }
	bool hasChildren () const;
	void childAttributeChanged (UINode* child, const char* attributeName, const char* oldAttributeValue);

	enum {
		kNoExport = 1 << 0
	};
	
	bool noExport () const { return flags & kNoExport; }
	void noExport (bool state) { if (state) flags |= kNoExport; else flags &= ~kNoExport; }

	bool operator== (const UINode& n) const { return name == n.name; }
	
	void sortChildren ();
	virtual void freePlatformResources () {}

	CLASS_METHODS(UINode, CBaseObject)
protected:
	std::string name;
	std::stringstream data;
	UIAttributes* attributes;
	UIDescList* children;
	int32_t flags;
};

//-----------------------------------------------------------------------------
class UICommentNode : public UINode
{
public:
	UICommentNode (const std::string& comment);
	CLASS_METHODS_NOCOPY(UICommentNode, UINode)
};

//-----------------------------------------------------------------------------
class UIVariableNode : public UINode
{
public:
	UIVariableNode (const std::string& name, UIAttributes* attributes);
	
	enum Type {
		kNumber,
		kString,
		kUnknown
	};
	
	Type getType () const;
	double getNumber () const;
	const std::string& getString () const;

	CLASS_METHODS_NOCOPY(UIVariableNode, UINode)
protected:
	Type type;
	double number;
};

//-----------------------------------------------------------------------------
class UIControlTagNode : public UINode
{
public:
	UIControlTagNode (const std::string& name, UIAttributes* attributes);
	int32_t getTag ();
	void setTag (int32_t newTag);
	
	const std::string* getTagString () const;
	void setTagString (const std::string& str);
	
	CLASS_METHODS_NOCOPY(UIControlTagNode, UINode)
protected:
	int32_t tag;
};

//-----------------------------------------------------------------------------
class UIBitmapNode : public UINode
{
public:
	UIBitmapNode (const std::string& name, UIAttributes* attributes);
	CBitmap* getBitmap (const std::string& pathHint);
	void setBitmap (UTF8StringPtr bitmapName);
	void setNinePartTiledOffset (const CRect* offsets);
	void invalidBitmap ();
	bool getFilterProcessed () const { return filterProcessed; }
	void setFilterProcessed () { filterProcessed = true; }
	bool getScaledBitmapsAdded () const { return scaledBitmapsAdded; }
	void setScaledBitmapsAdded () { scaledBitmapsAdded = true; }
	
	void createXMLData (const std::string& pathHint);
	void removeXMLData ();

	void freePlatformResources () VSTGUI_OVERRIDE_VMETHOD;
	CLASS_METHODS_NOCOPY(UIBitmapNode, UINode)
protected:
	~UIBitmapNode ();
	CBitmap* createBitmap (const std::string& str, CNinePartTiledDescription* partDesc) const;
	CBitmap* bitmap;
	bool filterProcessed;
	bool scaledBitmapsAdded;
};

//-----------------------------------------------------------------------------
class UIFontNode : public UINode
{
public:
	UIFontNode (const std::string& name, UIAttributes* attributes);
	CFontRef getFont ();
	void setFont (CFontRef newFont);
	void setAlternativeFontNames (UTF8StringPtr fontNames);
	bool getAlternativeFontNames (std::string& fontNames);

	void freePlatformResources () VSTGUI_OVERRIDE_VMETHOD;
	CLASS_METHODS_NOCOPY(UIFontNode, UINode)
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
	CLASS_METHODS_NOCOPY(UIColorNode, UINode)
protected:
	CColor color;
};

//-----------------------------------------------------------------------------
class UIGradientNode : public UINode
{
public:
	UIGradientNode (const std::string& name, UIAttributes* attributes);
	CGradient* getGradient ();
	void setGradient (CGradient* g);

	void freePlatformResources () VSTGUI_OVERRIDE_VMETHOD;
	CLASS_METHODS_NOCOPY(UIGradientNode, UINode)
protected:
	SharedPointer<CGradient> gradient;
	
};

//-----------------------------------------------------------------------------
class UIDescListWithFastFindAttributeNameChild : public UIDescList
{
private:
	typedef std::unordered_map<std::string, UINode*> ChildMap;
public:
	UIDescListWithFastFindAttributeNameChild () {}
	
	void add (UINode* obj) VSTGUI_OVERRIDE_VMETHOD
	{
		UIDescList::add (obj);
		const std::string* nameAttributeValue = obj->getAttributes ()->getAttributeValue ("name");
		if (nameAttributeValue)
			childMap.insert (std::make_pair (*nameAttributeValue, obj));
	}

	void remove (UINode* obj) VSTGUI_OVERRIDE_VMETHOD
	{
		const std::string* nameAttributeValue = obj->getAttributes ()->getAttributeValue ("name");
		if (nameAttributeValue)
		{
			ChildMap::iterator it = childMap.find (*nameAttributeValue);
			if (it != childMap.end ())
				childMap.erase (it);
		}
		UIDescList::remove (obj);
	}

	void removeAll () VSTGUI_OVERRIDE_VMETHOD
	{
		childMap.clear ();
		UIDescList::removeAll ();
	}

	UINode* findChildNodeWithAttributeValue (const std::string& attributeName, const std::string& attributeValue) const VSTGUI_OVERRIDE_VMETHOD
	{
		if (attributeName != "name")
			return UIDescList::findChildNodeWithAttributeValue (attributeName, attributeValue);
		ChildMap::const_iterator it = childMap.find (attributeValue);
		if (it != childMap.end ())
			return it->second;
		return 0;
	}

	void nodeAttributeChanged (UINode* node, const std::string& attributeName, const std::string& oldAttributeValue) VSTGUI_OVERRIDE_VMETHOD
	{
		if (attributeName != "name")
			return;
		ChildMap::iterator it = childMap.find (oldAttributeValue);
		if (it != childMap.end ())
			childMap.erase (it);
		const std::string* nameAttributeValue = node->getAttributes ()->getAttributeValue ("name");
		if (nameAttributeValue)
			childMap.insert (std::make_pair (*nameAttributeValue, node));
	}
private:
	ChildMap childMap;
};


//-----------------------------------------------------------------------------
UIDescList::UIDescList (bool ownsObjects)
: ownsObjects (ownsObjects)
{
}

//-----------------------------------------------------------------------------
UIDescList::UIDescList (const UIDescList& descList)
{
	for (const_iterator it = descList.begin (); it != descList.end (); it++)
	{
		UINode* node = static_cast<UINode*> ((*it)->newCopy ());
		vstgui_assert (node);
		add (node);
	}
}

//-----------------------------------------------------------------------------
UIDescList::~UIDescList ()
{
	removeAll ();
}

//-----------------------------------------------------------------------------
void UIDescList::add (UINode* obj)
{
	if (!ownsObjects)
		obj->remember ();
	UIDescListContainerType::push_back (obj);
}

//-----------------------------------------------------------------------------
void UIDescList::remove (UINode* obj)
{
	UIDescListContainerType::iterator pos = std::find (UIDescListContainerType::begin (), UIDescListContainerType::end (), obj);
	if (pos != UIDescListContainerType::end ())
	{
		UIDescListContainerType::erase (pos);
		obj->forget ();
	}
}

//-----------------------------------------------------------------------------
void UIDescList::removeAll ()
{
	for (const_reverse_iterator it = rbegin (), end = rend (); it != end; ++it)
		(*it)->forget ();
	clear ();
}

//-----------------------------------------------------------------------------
UINode* UIDescList::findChildNode (const std::string& nodeName) const
{
	VSTGUI_RANGE_BASED_FOR_LOOP (UIDescList, *this, UINode*, node)
		if (node->getName () == nodeName)
			return node;
	VSTGUI_RANGE_BASED_FOR_LOOP_END
	return 0;
}

//-----------------------------------------------------------------------------
UINode* UIDescList::findChildNodeWithAttributeValue (const std::string& attributeName, const std::string& attributeValue) const
{
	VSTGUI_RANGE_BASED_FOR_LOOP (UIDescList, *this, UINode*, node)
		const std::string* attributeValuePtr = node->getAttributes ()->getAttributeValue (attributeName);
		if (attributeValuePtr && *attributeValuePtr == attributeValue)
			return node;
	VSTGUI_RANGE_BASED_FOR_LOOP_END
	return 0;
}

//-----------------------------------------------------------------------------
void UIDescList::sort ()
{
	struct Compare {
		bool operator () (const UINode* n1, const UINode* n2) const
		{
			const std::string* str1 = n1->getAttributes ()->getAttributeValue ("name");
			const std::string* str2 = n2->getAttributes ()->getAttributeValue ("name");
			if (str1 && str2)
				return *str1 < *str2;
			else if (str1)
				return true;
			return false;
		}
	};
	std::sort (begin (), end (), Compare ());
}

//-----------------------------------------------------------------------------
class UIDescWriter
{
public:
	bool write (OutputStream& stream, UINode* rootNode);
protected:
	static void encodeAttributeString (std::string& str);

	bool writeNode (UINode* node, OutputStream& stream);
	bool writeComment (UICommentNode* node, OutputStream& stream);
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
	typedef std::map<std::string,std::string> SortedAttributes;
	SortedAttributes sortedAttributes (attr->begin (), attr->end ());
	for (SortedAttributes::const_iterator it = sortedAttributes.begin (), end = sortedAttributes.end (); it != end; ++it)
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
bool UIDescWriter::writeComment (UICommentNode* node, OutputStream& stream)
{
	stream << "<!--";
	stream << node->getData ().str ();
	stream << "-->\n";
	return true;
}

//-----------------------------------------------------------------------------
bool UIDescWriter::writeNode (UINode* node, OutputStream& stream)
{
	bool result = true;
	if (node->noExport ())
		return result;
	for (int32_t i = 0; i < intendLevel; i++) stream << "\t";
	if (UICommentNode* commentNode = dynamic_cast<UICommentNode*> (node))
	{
		return writeComment (commentNode, stream);
	}
	stream << "<";
	stream << node->getName ();
	result = writeAttributes (node->getAttributes (), stream);
	if (result)
	{
		UIDescList& children = node->getChildren ();
		if (!children.empty ())
		{
			stream << ">\n";
			intendLevel++;
			if (node->getData ().str ().length () > 0)
				result = writeNodeData (node->getData (), stream);
			for (UIDescList::iterator it = children.begin (), end = children.end (); it != end; ++it)
			{
				if (!writeNode (*it, stream))
					return false;
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

namespace UIDescriptionPrivate {
//-----------------------------------------------------------------------------
static bool decodeScaleFactorFromName (std::string name, double& scaleFactor)
{
	size_t index = name.find_last_of ("#");
	if (index == std::string::npos)
		return false;
	name.erase (0, index+1);
	index = name.find_last_of ("x");
	if (index == std::string::npos)
		return false;
	name.erase (index);
	scaleFactor = UTF8StringView (name.c_str ()).toDouble ();
	return true;
}

//-----------------------------------------------------------------------------
static std::string removeScaleFactorFromName (std::string name)
{
	size_t index = name.find_last_of ("#");
	if (index == std::string::npos)
		return "";
	name.erase (index);
	return name;
}
} // UIDescriptionPrivate

IdStringPtr IUIDescription::kCustomViewName = "custom-view-name";

//-----------------------------------------------------------------------------
IdStringPtr UIDescription::kMessageTagChanged = "kMessageTagChanged";
IdStringPtr UIDescription::kMessageColorChanged = "kMessageColorChanged";
IdStringPtr UIDescription::kMessageFontChanged = "kMessageFontChanged";
IdStringPtr UIDescription::kMessageBitmapChanged = "kMessageBitmapChanged";
IdStringPtr UIDescription::kMessageTemplateChanged = "kMessageTemplateChanged";
IdStringPtr UIDescription::kMessageGradientChanged = "kMessageGradientChanged";
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
	if (xmlFile.type == CResourceDescription::kStringType && xmlFile.u.name != 0)
		setFilePath (xmlFile.u.name);
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

//------------------------------------------------------------------------
void UIDescription::setFilePath (UTF8StringPtr path)
{
	filePath = path;
	xmlFile.u.name = filePath.c_str (); // make sure that xmlFile.u.name points to valid memory
}

//-----------------------------------------------------------------------------
void UIDescription::addDefaultNodes ()
{
	UINode* fontsNode = getBaseNode (MainNodeNames::kFont);
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
	UINode* colorsNode = getBaseNode (MainNodeNames::kColor);
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
			attr->setAttribute ("rgba", colorStr);
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
void UIDescription::setController (IController* inController) const
{
	controller = inController;
}

//-----------------------------------------------------------------------------
void UIDescription::setBitmapCreator (IBitmapCreator* creator)
{
	bitmapCreator = creator;
}

//-----------------------------------------------------------------------------
static void FreeNodePlatformResources (UINode* node)
{
	for (UIDescList::iterator it = node->getChildren ().begin (), end = node->getChildren ().end (); it != end; ++it)
	{
		(*it)->freePlatformResources ();
		FreeNodePlatformResources (*it);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::freePlatformResources ()
{
	if (nodes)
		FreeNodePlatformResources (nodes);
}

//-----------------------------------------------------------------------------
bool UIDescription::saveWindowsRCFile (UTF8StringPtr filename)
{
	bool result = false;
	UINode* bitmapNodes = getBaseNode (MainNodeNames::kBitmap);
	if (bitmapNodes && !bitmapNodes->getChildren().empty ())
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
static std::string moveOldFile (UTF8StringPtr filename)
{
	FILE* file = fopen (filename, "r");
	if (file)
	{
		fclose (file);
		std::string newName = filename;
		newName += ".old";
		if (std::rename (filename, newName.c_str ()) == 0)
			return newName;
	}
	return "";
}

//-----------------------------------------------------------------------------
bool UIDescription::save (UTF8StringPtr filename, int32_t flags)
{
	std::string oldName = moveOldFile (filename);
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
	if (result && oldName.empty () == false)
		std::remove (oldName.c_str ());

	return result;
}

//-----------------------------------------------------------------------------
bool UIDescription::saveToStream (OutputStream& stream, int32_t flags)
{
	changed (kMessageBeforeSave);
	UINode* bitmapNodes = getBaseNode (MainNodeNames::kBitmap);
	if (bitmapNodes)
	{
		for (UIDescList::iterator it = bitmapNodes->getChildren ().begin (); it != bitmapNodes->getChildren ().end (); it++)
		{
			UIBitmapNode* bitmapNode = dynamic_cast<UIBitmapNode*> (*it);
			if (bitmapNode)
			{
				if (flags & kWriteImagesIntoXMLFile)
					bitmapNode->createXMLData (filePath);
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
		VSTGUI_RANGE_BASED_FOR_LOOP (UIDescList, nodes->getChildren (), UINode*, itNode)
			if (itNode->getName () == MainNodeNames::kTemplate)
			{
				const std::string* nodeName = itNode->getAttributes ()->getAttributeValue ("name");
				if (nodeName && *nodeName == templateName)
				{
					node = itNode;
					break;
				}
			}
		VSTGUI_RANGE_BASED_FOR_LOOP_END
		if (node)
		{
			while (view != parentView)
			{
				if (view == parentView)
					return node;
				CViewContainer* container = dynamic_cast<CViewContainer*> (parentView);
				vstgui_assert (container != 0);
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
			UINode* customNode = new UINode (MainNodeNames::kCustom, customData);
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
			if ((*it)->getName() == MainNodeNames::kCustom)
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
CView* UIDescription::createViewFromNode (UINode* node) const
{
	const std::string* templateName = node->getAttributes ()->getAttributeValue (MainNodeNames::kTemplate);
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
			const std::string* viewClass = node->getAttributes ()->getAttributeValue (UIViewCreator::kAttrClass);
			if (viewClass)
				viewFactory->applyCustomViewAttributeValues (result, viewClass->c_str (), *node->getAttributes (), this);
		}
	}
	if (result == 0 && viewFactory)
	{
		result = viewFactory->createView (*node->getAttributes (), this);
		if (result == 0)
		{
			result = new CViewContainer (CRect (0, 0, 0, 0));
			viewFactory->applyCustomViewAttributeValues (result, "CViewContainer", *node->getAttributes (), this);
		}
	}
	if (result && node->hasChildren ())
	{
		CViewContainer* viewContainer = dynamic_cast<CViewContainer*> (result);
		VSTGUI_RANGE_BASED_FOR_LOOP (UIDescList, node->getChildren (), UINode*, itNode)
			if (viewContainer)
			{
				if (itNode->getName () == "view")
				{
					CView* childView = createViewFromNode (itNode);
					if (childView)
					{
						if (!viewContainer->addView (childView))
							childView->forget ();
					}
				}
			}
			if (itNode->getName () == "attribute")
			{
				const std::string* attrName = itNode->getAttributes ()->getAttributeValue ("id");
				const std::string* attrValue = itNode->getAttributes ()->getAttributeValue ("value");
				if (attrName && attrValue)
				{
					CViewAttributeID attrId = 0;
					if (attrName->size () == 4)
					{
						char c1 = (*attrName)[0];
						char c2 = (*attrName)[1];
						char c3 = (*attrName)[2];
						char c4 = (*attrName)[3];
						attrId = ((((size_t)c1) << 24) | (((size_t)c2) << 16) | (((size_t)c3) << 8) | (((size_t)c4) << 0));
					}
					else
						attrId = (CViewAttributeID)strtol (attrName->c_str (), 0, 10);
					if (attrId)
						result->setAttribute (attrId, static_cast<uint32_t> (attrValue->size () + 1), attrValue->c_str ());
				}
			}
		VSTGUI_RANGE_BASED_FOR_LOOP_END
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
CView* UIDescription::createView (UTF8StringPtr name, IController* _controller) const
{
	ScopePointer<IController> sp (&controller, _controller);
	if (nodes)
	{
		VSTGUI_RANGE_BASED_FOR_LOOP (UIDescList, nodes->getChildren (), UINode*, itNode)
			if (itNode->getName () == MainNodeNames::kTemplate)
			{
				const std::string* nodeName = itNode->getAttributes ()->getAttributeValue ("name");
				if (nodeName && *nodeName == name)
				{
					CView* view = createViewFromNode (itNode);
					if (view)
						view->setAttribute (kTemplateNameAttributeID, static_cast<uint32_t> (strlen (name) + 1), name);
					return view;
				}
			}
		VSTGUI_RANGE_BASED_FOR_LOOP_END
	}
	return 0;
}

//-----------------------------------------------------------------------------
bool UIDescription::getTemplateNameFromView (CView* view, std::string& templateName) const
{
	bool result = false;
	uint32_t attrSize = 0;
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
const UIAttributes* UIDescription::getViewAttributes (UTF8StringPtr name) const
{
	if (nodes)
	{
		VSTGUI_RANGE_BASED_FOR_LOOP (UIDescList, nodes->getChildren (), UINode*, itNode)
			if (itNode->getName () == MainNodeNames::kTemplate)
			{
				const std::string* nodeName = itNode->getAttributes ()->getAttributeValue ("name");
				if (nodeName && *nodeName == name)
					return itNode->getAttributes ();
			}
		VSTGUI_RANGE_BASED_FOR_LOOP_END
	}
	return 0;
}

//-----------------------------------------------------------------------------
UINode* UIDescription::getBaseNode (UTF8StringPtr name) const
{
	if (nodes)
	{
		UINode* node = nodes->getChildren ().findChildNode (name);
		if (node)
			return node;

		node = new UINode (name);
		nodes->getChildren ().add (node);
		return node;
	}
	return 0;
}

//-----------------------------------------------------------------------------
UINode* UIDescription::findChildNodeByNameAttribute (UINode* node, UTF8StringPtr nameAttribute) const
{
	if (node)
		return node->getChildren ().findChildNodeWithAttributeValue ("name", nameAttribute);
	return 0;
}

//-----------------------------------------------------------------------------
int32_t UIDescription::getTagForName (UTF8StringPtr name) const
{
	int32_t tag = -1;
	UIControlTagNode* controlTagNode = dynamic_cast<UIControlTagNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kControlTag), name));
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
bool UIDescription::hasColorName (UTF8StringPtr name) const
{
	UIColorNode* node = dynamic_cast<UIColorNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kColor), name));
	return node ? true : false;
}

//-----------------------------------------------------------------------------
bool UIDescription::hasTagName (UTF8StringPtr name) const
{
	UIControlTagNode* node = dynamic_cast<UIControlTagNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kControlTag), name));
	return node ? true : false;
}

//-----------------------------------------------------------------------------
bool UIDescription::hasFontName (UTF8StringPtr name) const
{
	UIFontNode* node = dynamic_cast<UIFontNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kFont), name));
	return node ? true : false;
}

//-----------------------------------------------------------------------------
bool UIDescription::hasBitmapName (UTF8StringPtr name) const
{
	UIBitmapNode* node = dynamic_cast<UIBitmapNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kBitmap), name));
	return node ? true : false;
}

//-----------------------------------------------------------------------------
bool UIDescription::hasGradientName (UTF8StringPtr name) const
{
	UIGradientNode* node = dynamic_cast<UIGradientNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kGradient), name));
	return node ? true : false;
}

//-----------------------------------------------------------------------------
IControlListener* UIDescription::getControlListener (UTF8StringPtr name) const
{
	if (controller)
		return controller->getControlListener (name);
	return 0;
}

//-----------------------------------------------------------------------------
CBitmap* UIDescription::getBitmap (UTF8StringPtr name) const
{
	UIBitmapNode* bitmapNode = dynamic_cast<UIBitmapNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kBitmap), name));
	if (bitmapNode)
	{
		CBitmap* bitmap = bitmapNode->getBitmap (filePath);
		if (bitmapCreator && bitmap && bitmap->getPlatformBitmap () == 0)
		{
			IPlatformBitmap* platformBitmap = bitmapCreator->createBitmap (*bitmapNode->getAttributes ());
			if (platformBitmap)
			{
				double scaleFactor;
				if (UIDescriptionPrivate::decodeScaleFactorFromName (name, scaleFactor))
					platformBitmap->setScaleFactor (scaleFactor);
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
							case BitmapFilter::Property::kUnknown:
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
		if (bitmapNode->getScaledBitmapsAdded () == false)
		{
			double scaleFactor;
			if (!UIDescriptionPrivate::decodeScaleFactorFromName (name, scaleFactor))
			{
				// find scaled versions for this bitmap
				UINode* bitmapsNode = getBaseNode (MainNodeNames::kBitmap);
				for (UIDescList::const_iterator it = bitmapsNode->getChildren ().begin (), end = bitmapsNode->getChildren ().end (); it != end; ++it)
				{
					UIBitmapNode* childNode = dynamic_cast<UIBitmapNode*>(*it);
					if (childNode == 0 || childNode == bitmapNode)
						continue;
					const std::string* childNodeBitmapName = childNode->getAttributes()->getAttributeValue ("name");
					if (childNodeBitmapName == 0)
						continue;
					std::string nameWithoutScaleFactor = UIDescriptionPrivate::removeScaleFactorFromName (*childNodeBitmapName);
					if (nameWithoutScaleFactor == name)
					{
						childNode->setScaledBitmapsAdded ();
						CBitmap* childBitmap = getBitmap (childNodeBitmapName->c_str ());
						if (childBitmap && childBitmap->getPlatformBitmap ())
							bitmap->addBitmap (childBitmap->getPlatformBitmap ());
					}
				}
			}
			bitmapNode->setScaledBitmapsAdded ();
		}
		return bitmap;
	}
	return 0;
}

//-----------------------------------------------------------------------------
CFontRef UIDescription::getFont (UTF8StringPtr name) const
{
	UIFontNode* fontNode = dynamic_cast<UIFontNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kFont), name));
	if (fontNode)
		return fontNode->getFont ();
	return 0;
}

//-----------------------------------------------------------------------------
bool UIDescription::getColor (UTF8StringPtr name, CColor& color) const
{
	UIColorNode* colorNode = dynamic_cast<UIColorNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kColor), name));
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
CGradient* UIDescription::getGradient (UTF8StringPtr name) const
{
	UIGradientNode* gradientNode = dynamic_cast<UIGradientNode*> (findChildNodeByNameAttribute (getBaseNode(MainNodeNames::kGradient), name));
	if (gradientNode)
		return gradientNode->getGradient ();
	return 0;
}

//-----------------------------------------------------------------------------
template<typename NodeType, typename ObjType, typename CompareFunction> UTF8StringPtr UIDescription::lookupName (const ObjType& obj, IdStringPtr mainNodeName, CompareFunction compare) const
{
	UINode* baseNode = getBaseNode (mainNodeName);
	if (baseNode)
	{
		UIDescList& children = baseNode->getChildren ();
		VSTGUI_RANGE_BASED_FOR_LOOP (UIDescList, children, UINode*, itNode)
			NodeType* node = dynamic_cast<NodeType*>(itNode);
			if (node && compare (this, node, obj))
			{
				const std::string* name = node->getAttributes ()->getAttributeValue ("name");
				return name ? name->c_str () : 0;
			}
		VSTGUI_RANGE_BASED_FOR_LOOP_END
	}
	return 0;
}

//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::lookupColorName (const CColor& color) const
{
	struct Compare {
		bool operator () (const UIDescription* desc, UIColorNode* node, const CColor& color) const {
			return node->getColor() == color;
		}
	};
	return lookupName<UIColorNode> (color, MainNodeNames::kColor, Compare ());
}

//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::lookupFontName (const CFontRef font) const
{
	struct Compare {
		bool operator () (const UIDescription* desc, UIFontNode* node, const CFontRef& font) const {
			return node->getFont () && node->getFont () == font;
		}
	};
	return font ? lookupName<UIFontNode> (font, MainNodeNames::kFont, Compare ()) : 0;
}

//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::lookupBitmapName (const CBitmap* bitmap) const
{
	struct Compare {
		bool operator () (const UIDescription* desc, UIBitmapNode* node, const CBitmap* bitmap) const {
			return node->getBitmap (desc->filePath) == bitmap;
		}
	};
	return bitmap ? lookupName<UIBitmapNode> (bitmap, MainNodeNames::kBitmap, Compare ()) : 0;
}

//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::lookupGradientName (const CGradient* gradient) const
{
	struct Compare {
		bool operator () (const UIDescription* desc, UIGradientNode* node, const CGradient* gradient) const {
			return node->getGradient() == gradient || (node->getGradient () && gradient->getColorStops () == node->getGradient ()->getColorStops ());
		}
	};
	return gradient ? lookupName<UIGradientNode> (gradient, MainNodeNames::kGradient, Compare ()) : 0;
}
	
//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::lookupControlTagName (const int32_t tag) const
{
	struct Compare {
		bool operator () (const UIDescription* desc, UIControlTagNode* node, const int32_t tag) const {
			int32_t nodeTag = node->getTag ();
			if (nodeTag == -1 && node->getTagString ())
			{
				double v;
				if (desc->calculateStringValue (node->getTagString ()->c_str (), v))
					nodeTag = (int32_t)v;
			}
			return nodeTag == tag;
		}
	};
	return lookupName<UIControlTagNode> (tag, MainNodeNames::kControlTag, Compare ());
}

//-----------------------------------------------------------------------------
template<typename NodeType> void UIDescription::changeNodeName (UTF8StringPtr oldName, UTF8StringPtr newName, IdStringPtr mainNodeName, IdStringPtr changeMsg)
{
	UINode* mainNode = getBaseNode (mainNodeName);
	NodeType* node = dynamic_cast<NodeType*> (findChildNodeByNameAttribute(mainNode, oldName));
	if (node)
	{
		node->getAttributes ()->setAttribute ("name", newName);
		mainNode->childAttributeChanged (node, "name", oldName);
		mainNode->sortChildren ();
		changed (changeMsg);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeColorName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	changeNodeName<UIColorNode> (oldName, newName, MainNodeNames::kColor, kMessageColorChanged);
}

//-----------------------------------------------------------------------------
void UIDescription::changeTagName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	changeNodeName<UIControlTagNode> (oldName, newName, MainNodeNames::kControlTag, kMessageTagChanged);
}

//-----------------------------------------------------------------------------
void UIDescription::changeFontName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	changeNodeName<UIFontNode> (oldName, newName, MainNodeNames::kFont, kMessageFontChanged);
}

//-----------------------------------------------------------------------------
void UIDescription::changeBitmapName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	changeNodeName<UIBitmapNode> (oldName, newName, MainNodeNames::kBitmap, kMessageBitmapChanged);
}

//-----------------------------------------------------------------------------
void UIDescription::changeGradientName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	changeNodeName<UIGradientNode> (oldName, newName, MainNodeNames::kGradient, kMessageGradientChanged);
}

//-----------------------------------------------------------------------------
void UIDescription::changeColor (UTF8StringPtr name, const CColor& newColor)
{
	UINode* colorsNode = getBaseNode (MainNodeNames::kColor);
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
			attr->setAttribute ("rgba", colorStr);
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
	UINode* fontsNode = getBaseNode (MainNodeNames::kFont);
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
void UIDescription::changeGradient (UTF8StringPtr name, CGradient* newGradient)
{
	UINode* gradientsNode = getBaseNode (MainNodeNames::kGradient);
	UIGradientNode* node = dynamic_cast<UIGradientNode*> (findChildNodeByNameAttribute (gradientsNode, name));
	if (node)
	{
		if (!node->noExport ())
		{
			node->setGradient (newGradient);
			changed (kMessageGradientChanged);
		}
	}
	else
	{
		if (gradientsNode)
		{
			UIAttributes* attr = new UIAttributes;
			attr->setAttribute ("name", name);
			UIGradientNode* node = new UIGradientNode ("gradient", attr);
			node->setGradient (newGradient);
			gradientsNode->getChildren ().add (node);
			gradientsNode->sortChildren ();
			changed (kMessageGradientChanged);
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeBitmap (UTF8StringPtr name, UTF8StringPtr newName, const CRect* nineparttiledOffset)
{
	UINode* bitmapsNode = getBaseNode (MainNodeNames::kBitmap);
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
	UIBitmapNode* bitmapNode = dynamic_cast<UIBitmapNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kBitmap), bitmapName));
	if (bitmapNode)
	{
		bitmapNode->getChildren().removeAll ();
		for (std::list<SharedPointer<UIAttributes> >::const_iterator it = filters.begin (); it != filters.end (); it++)
		{
			const std::string* filterName = (*it)->getAttributeValue ("name");
			if (filterName == 0)
				continue;
			UINode* filterNode = new UINode ("filter");
			filterNode->getAttributes ()->setAttribute ("name", *filterName);
			for (UIAttributes::const_iterator it2 = (*it)->begin (); it2 != (*it)->end (); it2++)
			{
				if ((*it2).first == "name")
					continue;
				UINode* propertyNode = new UINode ("property");
				propertyNode->getAttributes ()->setAttribute("name", (*it2).first);
				propertyNode->getAttributes ()->setAttribute("value", (*it2).second);
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
	UIBitmapNode* bitmapNode = dynamic_cast<UIBitmapNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kBitmap), bitmapName));
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
				attributes->setAttribute ("name", *filterName);
				for (UIDescList::iterator it2 = (*it)->getChildren ().begin (); it2 != (*it)->getChildren ().end (); it2++)
				{
					if ((*it2)->getName () == "property")
					{
						const std::string* name = (*it2)->getAttributes ()->getAttributeValue ("name");
						const std::string* value = (*it2)->getAttributes ()->getAttributeValue ("value");
						if (name && value)
						{
							attributes->setAttribute (*name, *value);
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
	VSTGUI_RANGE_BASED_FOR_LOOP (UIDescList, children, UINode*, itNode)
		const std::string* name = itNode->getAttributes ()->getAttributeValue ("name");
		if (name && *name == nodeName)
		{
			if (!itNode->noExport ())
				children.remove (itNode);
			return;
		}
	VSTGUI_RANGE_BASED_FOR_LOOP_END
}

//-----------------------------------------------------------------------------
void UIDescription::removeNode (UTF8StringPtr name, IdStringPtr mainNodeName, IdStringPtr changeMsg)
{
	UINode* node = getBaseNode (mainNodeName);
	if (node)
	{
		removeChildNode (node, name);
		changed (changeMsg);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::removeColor (UTF8StringPtr name)
{
	removeNode (name, MainNodeNames::kColor, kMessageColorChanged);
}

//-----------------------------------------------------------------------------
void UIDescription::removeTag (UTF8StringPtr name)
{
	removeNode (name, MainNodeNames::kControlTag, kMessageTagChanged);
}

//-----------------------------------------------------------------------------
void UIDescription::removeFont (UTF8StringPtr name)
{
	removeNode (name, MainNodeNames::kFont, kMessageFontChanged);
}

//-----------------------------------------------------------------------------
void UIDescription::removeBitmap (UTF8StringPtr name)
{
	removeNode (name, MainNodeNames::kBitmap, kMessageBitmapChanged);
}

//-----------------------------------------------------------------------------
void UIDescription::removeGradient (UTF8StringPtr name)
{
	removeNode (name, MainNodeNames::kGradient, kMessageGradientChanged);
}

//-----------------------------------------------------------------------------
void UIDescription::changeAlternativeFontNames (UTF8StringPtr name, UTF8StringPtr alternativeFonts)
{
	UIFontNode* node = dynamic_cast<UIFontNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kFont), name));
	if (node)
	{
		node->setAlternativeFontNames (alternativeFonts);
		changed (kMessageFontChanged);
	}
}

//-----------------------------------------------------------------------------
bool UIDescription::getAlternativeFontNames (UTF8StringPtr name, std::string& alternativeFonts) const
{
	UIFontNode* node = dynamic_cast<UIFontNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kFont), name));
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
	VSTGUI_RANGE_BASED_FOR_LOOP (UIDescList, nodes->getChildren (), UINode*, itNode)
		if (itNode->getName () == MainNodeNames::kTemplate)
		{
			const std::string* nodeName = itNode->getAttributes ()->getAttributeValue ("name");
			if (nodeName)
				names.push_back (nodeName);
		}
	VSTGUI_RANGE_BASED_FOR_LOOP_END
}

//-----------------------------------------------------------------------------
template<typename NodeType> void UIDescription::collectNamesFromNode (IdStringPtr mainNodeName, std::list<const std::string*>& names) const
{
	UINode* node = getBaseNode (mainNodeName);
	if (node)
	{
		UIDescList& children = node->getChildren ();
		VSTGUI_RANGE_BASED_FOR_LOOP (UIDescList, children, UINode*, itNode)
			NodeType* node = dynamic_cast<NodeType*>(itNode);
			if (node)
			{
				const std::string* name = node->getAttributes ()->getAttributeValue ("name");
				if (name)
					names.push_back (name);
			}
		VSTGUI_RANGE_BASED_FOR_LOOP_END
	}
}

//-----------------------------------------------------------------------------
void UIDescription::collectColorNames (std::list<const std::string*>& names) const
{
	collectNamesFromNode<UIColorNode> (MainNodeNames::kColor, names);
}

//-----------------------------------------------------------------------------
void UIDescription::collectFontNames (std::list<const std::string*>& names) const
{
	collectNamesFromNode<UIFontNode> (MainNodeNames::kFont, names);
}

//-----------------------------------------------------------------------------
void UIDescription::collectBitmapNames (std::list<const std::string*>& names) const
{
	collectNamesFromNode<UIBitmapNode> (MainNodeNames::kBitmap, names);
}

//-----------------------------------------------------------------------------
void UIDescription::collectGradientNames (std::list<const std::string*>& names) const
{
	collectNamesFromNode<UIGradientNode> (MainNodeNames::kGradient, names);
}

//-----------------------------------------------------------------------------
void UIDescription::collectControlTagNames (std::list<const std::string*>& names) const
{
	collectNamesFromNode<UIControlTagNode> (MainNodeNames::kControlTag, names);
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
		for (std::list<std::string>::const_iterator it = attributeNames.begin (), end = attributeNames.end (); it != end; ++it)
		{
			std::string value;
			if (factory->getAttributeValue (view, (*it), value, this))
				node->getAttributes ()->setAttribute ((*it), value);
		}
		node->getAttributes ()->setAttribute (UIViewCreator::kAttrClass, factory->getViewName (view));
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
				attr->setAttribute (MainNodeNames::kTemplate, subTemplateName);
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
					if (!subNode->getChildren ().empty ())
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
		for (UIDescList::iterator it = nodes->getChildren ().begin (), end = nodes->getChildren ().end (); it != end && node == 0; ++it)
		{
			if ((*it)->getName () == MainNodeNames::kTemplate)
			{
				const std::string* nodeName = (*it)->getAttributes ()->getAttributeValue ("name");
				if (*nodeName == name)
				{
					node = (*it);
				}
			}
		}
		if (node == 0)
		{
			node = new UINode (MainNodeNames::kTemplate);
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
		nodes = new UINode ("vstgui-ui-description");
		addDefaultNodes ();
	}
	UINode* templateNode = findChildNodeByNameAttribute (nodes, name);
	if (templateNode == 0)
	{
		UINode* newNode = new UINode (MainNodeNames::kTemplate, attr);
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
		vstgui_assert (duplicate);
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
	UINode* customNode = findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kCustom), name);
	if (customNode)
		return false;
	attr->setAttribute ("name", name);
	customNode = new UINode ("attributes", attr);
	UINode* parent = getBaseNode (MainNodeNames::kCustom);
	parent->getChildren ().add (customNode);
	return true;
}

//-----------------------------------------------------------------------------
UIAttributes* UIDescription::getCustomAttributes (UTF8StringPtr name, bool create)
{
	UINode* customNode = findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kCustom), name);
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
	UIControlTagNode* controlTagNode = dynamic_cast<UIControlTagNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kControlTag), tagName));
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
	UINode* tagsNode = getBaseNode (MainNodeNames::kControlTag);
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
	UIVariableNode* node = dynamic_cast<UIVariableNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kVariable), name));
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

//-----------------------------------------------------------------------------
bool UIDescription::getVariable (UTF8StringPtr name, std::string& value) const
{
	UIVariableNode* node = dynamic_cast<UIVariableNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kVariable), name));
	if (node)
	{
		value = node->getString ();
		return true;
	}
	return false;
}

namespace UIDescriptionPrivate {

//-----------------------------------------------------------------------------
struct Locale
{
	Locale ()
	{
		origLocal = std::locale ();
		std::locale::global (std::locale::classic ());
	}

	~Locale ()
	{
		std::locale::global (origLocal);
	}

	std::locale origLocal;
};

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

typedef std::list<StringToken> StringTokenList;
	
//-----------------------------------------------------------------------------
static bool tokenizeString (std::string& str, StringTokenList& tokens)
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
					std::string token ((const char*)tokenStart, static_cast<size_t> (iterator - tokenStart));
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
							std::string token ((const char*)tokenStart, static_cast<size_t> (iterator - tokenStart));
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
							std::string token ((const char*)tokenStart, static_cast<size_t> (iterator - tokenStart));
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
							std::string token ((const char*)tokenStart, static_cast<size_t> (iterator - tokenStart));
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
							std::string token ((const char*)tokenStart, static_cast<size_t> (iterator - tokenStart));
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
							std::string token ((const char*)tokenStart, static_cast<size_t> (iterator - tokenStart));
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
							std::string token ((const char*)tokenStart, static_cast<size_t> (iterator - tokenStart));
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
		std::string token ((const char*)tokenStart, static_cast<size_t> (iterator - tokenStart));
		tokens.push_back (token);
	}
	return true;	
}

//-----------------------------------------------------------------------------
static bool computeTokens (StringTokenList& tokens, double& result)
{
	int32_t openCount = 0;
	StringTokenList::iterator openPosition = tokens.end ();
	// first check parentheses
	for (StringTokenList::iterator it = tokens.begin (); it != tokens.end (); it++)
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
				StringTokenList tmp (++openPosition, it);
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
	StringTokenList::iterator prevToken = tokens.begin ();
	for (StringTokenList::iterator it = tokens.begin (); it != tokens.end (); it++)
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
	for (StringTokenList::const_iterator it = tokens.begin (); it != tokens.end (); it++)
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
	UIDescriptionPrivate::Locale localeResetter;

	char* endPtr = 0;
	result = strtod (_str, &endPtr);
	if (endPtr == _str + strlen (_str))
		return true;
	std::string str (_str);
	UIDescriptionPrivate::StringTokenList tokens;
	if (!UIDescriptionPrivate::tokenizeString (str, tokens))
	{
	#if DEBUG
		DebugPrint("TokenizeString failed :%s\n", _str);
	#endif
		return false;
	}
	// first make substituation
	for (UIDescriptionPrivate::StringTokenList::iterator it = tokens.begin (); it != tokens.end (); it++)
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
			if (name != "view" && name != MainNodeNames::kCustom)
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
				if (name == MainNodeNames::kControlTag || name == MainNodeNames::kColor || name == MainNodeNames::kBitmap)
					newNode = new UINode (name, new UIAttributes (elementAttributes), true);
				else if (name == MainNodeNames::kFont || name == MainNodeNames::kTemplate
					  || name == MainNodeNames::kControlTag || name == MainNodeNames::kCustom
					  || name == MainNodeNames::kVariable || name == MainNodeNames::kGradient)
					newNode = new UINode (name, new UIAttributes (elementAttributes));
				else
					parser->stop ();
			}
			else if (parent->getName () == MainNodeNames::kBitmap)
			{
				if (name == "bitmap")
					newNode = new UIBitmapNode (name, new UIAttributes (elementAttributes));
				else
					parser->stop ();
			}
			else if (parent->getName () == MainNodeNames::kFont)
			{
				if (name == "font")
					newNode = new UIFontNode (name, new UIAttributes (elementAttributes));
				else
					parser->stop ();
			}
			else if (parent->getName () == MainNodeNames::kColor)
			{
				if (name == "color")
					newNode = new UIColorNode (name, new UIAttributes (elementAttributes));
				else
					parser->stop ();
			}
			else if (parent->getName () == MainNodeNames::kControlTag)
			{
				if (name == "control-tag")
					newNode = new UIControlTagNode (name, new UIAttributes (elementAttributes));
				else
					parser->stop ();
			}
			else if (parent->getName () == MainNodeNames::kVariable)
			{
				if (name == "var")
					newNode = new UIVariableNode (name, new UIAttributes (elementAttributes));
				else
					parser->stop ();
			}
			else if (parent->getName () == MainNodeNames::kGradient)
			{
				if (name == "gradient")
					newNode = new UIGradientNode (name, new UIAttributes (elementAttributes));
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
		vstgui_assert (nodes == 0);
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
	if (nodeStack.size () == 0)
		return;
	std::stringstream& sstream = nodeStack.back ()->getData ();
	for (int32_t i = 0; i < length; i++)
	{
		if (data[i] == '\t' || data[i] == '\n' || data[i] == '\r' || data[i] == 0x20)
			continue;
		sstream << (char)data[i];
	}
}

//-----------------------------------------------------------------------------
void UIDescription::xmlComment (Xml::Parser* parser, IdStringPtr comment)
{
#if VSTGUI_LIVE_EDITING
	if (nodeStack.size () == 0)
	{
	#if DEBUG
		DebugPrint ("*** WARNING : Comment outside of root tag will be removed on save !\nComment: %s\n", comment);
	#endif
		return;
	}
	UINode* parent = nodeStack.back ();
	if (parent && comment)
	{
		std::string commentStr (comment);
		if (commentStr.length () > 0)
		{
			UICommentNode* commentNode = new UICommentNode (comment);
			parent->getChildren ().add (commentNode);
		}
	}
#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UINode::UINode (const std::string& _name, UIAttributes* _attributes, bool needsFastChildNameAttributeLookup)
: name (_name)
, attributes (_attributes)
, children (needsFastChildNameAttributeLookup ? new UIDescListWithFastFindAttributeNameChild: new UIDescList)
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
	vstgui_assert (children != 0);
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
	return !children->empty ();
}

//-----------------------------------------------------------------------------
void UINode::childAttributeChanged (UINode* child, const char* attributeName, const char* oldAttributeValue)
{
	children->nodeAttributeChanged (child, attributeName, oldAttributeValue);
}

//-----------------------------------------------------------------------------
void UINode::sortChildren ()
{
	children->sort ();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UICommentNode::UICommentNode (const std::string& comment)
: UINode ("comment")
{
	data << comment;
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
		UIDescriptionPrivate::Locale localeResetter;

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
int32_t UIControlTagNode::getTag ()
{
	if (tag == -1)
	{
		const std::string* tagStr = attributes->getAttributeValue ("tag");
		if (tagStr)
		{
			if (tagStr->size () == 6 && (*tagStr)[0] == '\'' && (*tagStr)[5] == '\'')
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
	attributes->setAttribute ("tag", str);
	tag = -1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UIBitmapNode::UIBitmapNode (const std::string& name, UIAttributes* attributes)
: UINode (name, attributes)
, bitmap (0)
, filterProcessed (false)
, scaledBitmapsAdded (false)
{
}

//-----------------------------------------------------------------------------
UIBitmapNode::~UIBitmapNode ()
{
	if (bitmap)
		bitmap->forget ();
}

//-----------------------------------------------------------------------------
void UIBitmapNode::freePlatformResources ()
{
	if (bitmap)
		bitmap->forget ();
	bitmap = 0;
}

//-----------------------------------------------------------------------------
void UIBitmapNode::createXMLData (const std::string& pathHint)
{
	CBitmap* bitmap = getBitmap (pathHint);
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
					UINode* node = getChildren ().findChildNode ("data");
					if (node)
						getChildren ().remove (node);
					UINode* dataNode = new UINode ("data");
					dataNode->getAttributes ()->setAttribute ("encoding", "base64");
					dataNode->getData ().write ((const char*)bd.getData (), static_cast<std::streamsize> (bd.getDataSize ()));
					getChildren ().add (dataNode);
				}
				std::free (data);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void UIBitmapNode::removeXMLData ()
{
	UINode* node = getChildren ().findChildNode ("data");
	if (node)
		getChildren ().remove (node);
}

//-----------------------------------------------------------------------------
CBitmap* UIBitmapNode::createBitmap (const std::string& str, CNinePartTiledDescription* partDesc) const
{
	if (partDesc)
		return new CNinePartTiledBitmap (CResourceDescription (str.c_str()), *partDesc);
	return new CBitmap (CResourceDescription (str.c_str()));
}

//-----------------------------------------------------------------------------
CBitmap* UIBitmapNode::getBitmap (const std::string& pathHint)
{
	if (bitmap == 0)
	{
		const std::string* path = attributes->getAttributeValue ("path");
		if (path)
		{
			CNinePartTiledDescription partDesc;
			CNinePartTiledDescription* partDescPtr = 0;
			CRect offsets;
			if (attributes->getRectAttribute ("nineparttiled-offsets", offsets))
			{
				partDesc = CNinePartTiledDescription (offsets.left, offsets.top, offsets.right, offsets.bottom);
				partDescPtr = &partDesc;
			}
			bitmap = createBitmap (*path, partDescPtr);
			if (bitmap->getPlatformBitmap () == 0 && pathIsAbsolute (pathHint))
			{
				std::string absPath = pathHint;
				if (removeLastPathComponent (absPath))
				{
					absPath += "/" + *path;
					SharedPointer<IPlatformBitmap> platformBitmap = owned (IPlatformBitmap::createFromPath (absPath.c_str ()));
					if (platformBitmap)
						bitmap->setPlatformBitmap (platformBitmap);
				}
			}
		}
		if (bitmap && bitmap->getPlatformBitmap () == 0)
		{
			UINode* node = getChildren ().findChildNode ("data");
			if (node && node->getData ().str ().length () > 0)
			{
				const std::string* codec = node->getAttributes ()->getAttributeValue ("encoding");
				if (codec && *codec == "base64")
				{
					Base64Codec bd;
					if (bd.init (node->getData ().str ()))
					{
						OwningPointer<IPlatformBitmap> platformBitmap = IPlatformBitmap::createFromMemory (bd.getData (), bd.getDataSize ());
						if (platformBitmap)
						{
							double scaleFactor = 1.;
							if (attributes->getDoubleAttribute ("scale-factor", scaleFactor))
								platformBitmap->setScaleFactor (scaleFactor);
							bitmap->setPlatformBitmap (platformBitmap);
						}
					}
				}
			}
		}
		if (bitmap && path && bitmap->getPlatformBitmap () && bitmap->getPlatformBitmap ()->getScaleFactor () == 1.)
		{
			double scaleFactor = 1.;
			if (UIDescriptionPrivate::decodeScaleFactorFromName (*path, scaleFactor))
			{
				bitmap->getPlatformBitmap ()->setScaleFactor (scaleFactor);
				attributes->setDoubleAttribute ("scale-factor", scaleFactor);
			}
		}
	}
	return bitmap;
}

//-----------------------------------------------------------------------------
void UIBitmapNode::setBitmap (UTF8StringPtr bitmapName)
{
	std::string attrValue (bitmapName);
	attributes->setAttribute ("path", attrValue);
	if (bitmap)
		bitmap->forget ();
	bitmap = 0;
	double scaleFactor = 1.;
	if (UIDescriptionPrivate::decodeScaleFactorFromName (bitmapName, scaleFactor))
		attributes->setDoubleAttribute ("scale-factor", scaleFactor);
}

//-----------------------------------------------------------------------------
void UIBitmapNode::setNinePartTiledOffset (const CRect* offsets)
{
	if (bitmap)
	{
		CNinePartTiledBitmap* tiledBitmap = dynamic_cast<CNinePartTiledBitmap*> (bitmap);
		if (offsets && tiledBitmap)
		{
			tiledBitmap->setPartOffsets (CNinePartTiledDescription (offsets->left, offsets->top, offsets->right, offsets->bottom));
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
UIFontNode::~UIFontNode ()
{
	if (font)
		font->forget ();
}

//-----------------------------------------------------------------------------
void UIFontNode::freePlatformResources ()
{
	if (font)
		font->forget ();
	font = 0;
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
						attributes->getStringArrayAttribute ("alternative-font-names", alternativeFontNames);
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
	attributes->setAttribute ("name", name);
	attributes->setAttribute ("font-name", newFont->getName ());
	std::stringstream str;
	str << newFont->getSize ();
	attributes->setAttribute ("size", str.str ());
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
void UIColorNode::setColor (const CColor& newColor)
{
	std::string name (*attributes->getAttributeValue ("name"));
	attributes->removeAll ();
	attributes->setAttribute ("name", name);

	std::string colorString;
	UIViewCreator::colorToString (newColor, colorString, 0);
	attributes->setAttribute ("rgba", colorString);
	color = newColor;
}

//-----------------------------------------------------------------------------
UIGradientNode::UIGradientNode (const std::string& name, UIAttributes* attributes)
: UINode (name, attributes)
{
}

//-----------------------------------------------------------------------------
void UIGradientNode::freePlatformResources ()
{
	gradient = 0;
}

//-----------------------------------------------------------------------------
CGradient* UIGradientNode::getGradient ()
{
	if (gradient == 0)
	{
		CGradient::ColorStopMap colorStops;
		double start;
		CColor color;
		for (UIDescList::const_iterator it = getChildren ().begin (), end = getChildren ().end (); it != end; ++it)
		{
			UINode* colorNode = *it;
			if (colorNode->getName () == "color-stop")
			{
				const std::string* rgba = colorNode->getAttributes ()->getAttributeValue ("rgba");
				if (rgba == 0 || colorNode->getAttributes()->getDoubleAttribute ("start", start) == false)
					continue;
				if (UIDescription::parseColor (*rgba, color) == false)
					continue;
				colorStops.insert (std::make_pair (start, color));
			}
		}
		if (colorStops.size () > 1)
			gradient = owned (CGradient::create (colorStops));
	}
	return gradient;
}

//-----------------------------------------------------------------------------
void UIGradientNode::setGradient (CGradient* g)
{
	gradient = g;
	getChildren ().removeAll ();
	if (gradient == 0)
		return;

	const CGradient::ColorStopMap colorStops = gradient->getColorStops ();
	for (CGradient::ColorStopMap::const_iterator it = colorStops.begin (), end = colorStops.end (); it != end; ++it)
	{
		UINode* node = new UINode ("color-stop");
		node->getAttributes ()->setDoubleAttribute ("start", (*it).first);
		std::string colorString;
		UIViewCreator::colorToString ((*it).second, colorString, 0);
		node->getAttributes ()->setAttribute ("rgba", colorString);
		getChildren ().add (node);
	}
}

} // namespace
