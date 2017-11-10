// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uidescription.h"
#include "uidescriptionlistener.h"
#include "uiattributes.h"
#include "uiviewfactory.h"
#include "uiviewcreator.h"
#include "cstream.h"
#include "base64codec.h"
#include "icontroller.h"
#include "../lib/cfont.h"
#include "../lib/cstring.h"
#include "../lib/cframe.h"
#include "../lib/cdrawcontext.h"
#include "../lib/cgradient.h"
#include "../lib/cgraphicspath.h"
#include "../lib/cbitmap.h"
#include "../lib/cbitmapfilter.h"
#include "../lib/dispatchlist.h"
#include "../lib/platform/std_unorderedmap.h"
#include "../lib/platform/iplatformbitmap.h"
#include "../lib/platform/iplatformfont.h"
#include "detail/uiviewcreatorattributes.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <deque>

namespace VSTGUI {

//-----------------------------------------------------------------------------
/// @cond ignore
//-----------------------------------------------------------------------------
template <class T> class ScopePointer
{
public:
	ScopePointer (T** pointer, T* obj) : pointer (pointer), oldObject (nullptr)
	{
		if (pointer)
		{
			oldObject = *pointer;
			*pointer = obj;
		}
	}
	~ScopePointer () noexcept
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

using UIDescListContainerType = std::vector<UINode*>;
//-----------------------------------------------------------------------------
class UIDescList : public NonAtomicReferenceCounted, private UIDescListContainerType
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
	
	explicit UIDescList (bool ownsObjects = true);
	UIDescList (const UIDescList& uiDesc);
	~UIDescList () noexcept override;

	virtual void add (UINode* obj);
	virtual void remove (UINode* obj);
	virtual void removeAll ();
	virtual UINode* findChildNode (UTF8StringView nodeName) const;
	virtual UINode* findChildNodeWithAttributeValue (const std::string& attributeName, const std::string& attributeValue) const;

	virtual void nodeAttributeChanged (UINode* child, const std::string& attributeName, const std::string& oldAttributeValue) {}

	void sort ();
	
protected:
	bool ownsObjects;
};

//-----------------------------------------------------------------------------
class UINode : public NonAtomicReferenceCounted
{
public:
	using DataStorage = std::string;

	UINode (const std::string& name, UIAttributes* attributes = nullptr, bool needsFastChildNameAttributeLookup = false);
	UINode (const std::string& name, UIDescList* children, UIAttributes* attributes = nullptr);
	UINode (const UINode& n);
	~UINode () noexcept override;

	const std::string& getName () const { return name; }
	DataStorage& getData () { return data; }
	const DataStorage& getData () const { return data; }

	UIAttributes* getAttributes () const { return attributes; }
	UIDescList& getChildren () const { return *children; }
	bool hasChildren () const;
	void childAttributeChanged (UINode* child, const char* attributeName, const char* oldAttributeValue);

	enum {
		kNoExport = 1 << 0
	};
	
	bool noExport () const { return hasBit (flags, kNoExport); }
	void noExport (bool state) { setBit (flags, kNoExport, state); }

	bool operator== (const UINode& n) const { return name == n.name; }
	
	void sortChildren ();
	virtual void freePlatformResources () {}

protected:
	std::string name;
	DataStorage data;
	UIAttributes* attributes;
	UIDescList* children;
	int32_t flags;
};

//-----------------------------------------------------------------------------
class UICommentNode : public UINode
{
public:
	explicit UICommentNode (const std::string& comment);
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

	void freePlatformResources () override;
protected:
	~UIBitmapNode () noexcept override;
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

	void freePlatformResources () override;
protected:
	~UIFontNode () noexcept override;
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
class UIGradientNode : public UINode
{
public:
	UIGradientNode (const std::string& name, UIAttributes* attributes);
	CGradient* getGradient ();
	void setGradient (CGradient* g);

	void freePlatformResources () override;
protected:
	SharedPointer<CGradient> gradient;
	
};

//-----------------------------------------------------------------------------
class UIDescListWithFastFindAttributeNameChild : public UIDescList
{
private:
	using ChildMap = std::unordered_map<std::string, UINode*>;
public:
	UIDescListWithFastFindAttributeNameChild () {}
	
	void add (UINode* obj) override
	{
		UIDescList::add (obj);
		const std::string* nameAttributeValue = obj->getAttributes ()->getAttributeValue ("name");
		if (nameAttributeValue)
			childMap.emplace (*nameAttributeValue, obj);
	}

	void remove (UINode* obj) override
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

	void removeAll () override
	{
		childMap.clear ();
		UIDescList::removeAll ();
	}

	UINode* findChildNodeWithAttributeValue (const std::string& attributeName, const std::string& attributeValue) const override
	{
		if (attributeName != "name")
			return UIDescList::findChildNodeWithAttributeValue (attributeName, attributeValue);
		ChildMap::const_iterator it = childMap.find (attributeValue);
		if (it != childMap.end ())
			return it->second;
		return nullptr;
	}

	void nodeAttributeChanged (UINode* node, const std::string& attributeName, const std::string& oldAttributeValue) override
	{
		if (attributeName != "name")
			return;
		ChildMap::iterator it = childMap.find (oldAttributeValue);
		if (it != childMap.end ())
			childMap.erase (it);
		const std::string* nameAttributeValue = node->getAttributes ()->getAttributeValue ("name");
		if (nameAttributeValue)
			childMap.emplace (*nameAttributeValue, node);
	}
private:
	ChildMap childMap;
};


//-----------------------------------------------------------------------------
UIDescList::UIDescList (bool ownsObjects)
: ownsObjects (ownsObjects)
{
}

//------------------------------------------------------------------------
UIDescList::UIDescList (const UIDescList& uiDesc)
: ownsObjects (false)
{
	for (auto& child : uiDesc)
		add (child);
}

//-----------------------------------------------------------------------------
UIDescList::~UIDescList () noexcept
{
	removeAll ();
}

//-----------------------------------------------------------------------------
void UIDescList::add (UINode* obj)
{
	if (!ownsObjects)
		obj->remember ();
	UIDescListContainerType::emplace_back (obj);
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
UINode* UIDescList::findChildNode (UTF8StringView nodeName) const
{
	for (const auto& node : *this)
	{
		auto& name = node->getName ();
		if (nodeName == UTF8StringView (name))
			return node;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
UINode* UIDescList::findChildNodeWithAttributeValue (const std::string& attributeName, const std::string& attributeValue) const
{
	for (const auto& node : *this)
	{
		const std::string* attributeValuePtr = node->getAttributes ()->getAttributeValue (attributeName);
		if (attributeValuePtr && *attributeValuePtr == attributeValue)
			return node;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
void UIDescList::sort ()
{
	std::sort (begin (), end (), [] (const UINode* n1, const UINode* n2) {
		const std::string* str1 = n1->getAttributes ()->getAttributeValue ("name");
		const std::string* str2 = n2->getAttributes ()->getAttributeValue ("name");
		if (str1 && str2)
			return *str1 < *str2;
		else if (str1)
			return true;
		return false;
	});
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
	bool writeNodeData (UINode::DataStorage& str, OutputStream& stream);
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
	using SortedAttributes = std::map<std::string,std::string>;
	SortedAttributes sortedAttributes (attr->begin (), attr->end ());
	for (auto& sa : sortedAttributes)
	{
		if (sa.second.length () > 0)
		{
			stream << " ";
			stream << sa.first;
			stream << "=\"";
			std::string value (sa.second);
			encodeAttributeString (value);
			stream << value;
			stream << "\"";
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
bool UIDescWriter::writeNodeData (UINode::DataStorage& str, OutputStream& stream)
{
	for (int32_t i = 0; i < intendLevel; i++) stream << "\t";
	uint32_t i = 0;
	for (auto c : str)
	{
		stream << static_cast<int8_t> (c);
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
	stream << node->getData ();
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
			if (!node->getData ().empty ())
				result = writeNodeData (node->getData (), stream);
			for (auto& childNode : children)
			{
				if (!writeNode (childNode, stream))
					return false;
			}
			intendLevel--;
			for (int32_t i = 0; i < intendLevel; i++) stream << "\t";
			stream << "</";
			stream << node->getName ();
			stream << ">\n";
		}
		else if (!node->getData ().empty ())
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
			color.red = (uint8_t)strtol (rv.c_str (), nullptr, 16);
			color.green = (uint8_t)strtol (gv.c_str (), nullptr, 16);
			color.blue = (uint8_t)strtol (bv.c_str (), nullptr, 16);
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
			color.red = (uint8_t)strtol (rv.c_str (), nullptr, 16);
			color.green = (uint8_t)strtol (gv.c_str (), nullptr, 16);
			color.blue = (uint8_t)strtol (bv.c_str (), nullptr, 16);
			color.alpha = (uint8_t)strtol (av.c_str (), nullptr, 16);
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
static bool decodeScaleFactorFromName (const std::string& name, const char* identicator,
                                       double& scaleFactor)
{
	size_t indicatorIndex = name.find_last_of (identicator);
	if (indicatorIndex == std::string::npos)
		return false;
	size_t xIndex = name.find_last_of ("x");
	if (xIndex == std::string::npos)
		return false;
	std::string tmp (name);
	tmp.erase (0, ++indicatorIndex);
	tmp.erase (xIndex - indicatorIndex);
	scaleFactor = UTF8StringView (tmp.c_str ()).toDouble ();
	return scaleFactor != 0;
}

//-----------------------------------------------------------------------------
static bool decodeScaleFactorFromName (const std::string& name, double& scaleFactor)
{
	if (!decodeScaleFactorFromName (name, "#", scaleFactor))
	{
		if (!decodeScaleFactorFromName (name, "_", scaleFactor))
			return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
static std::string removeScaleFactorFromName (std::string name)
{
	size_t index = name.find_last_of ("#");
	if (index == std::string::npos)
	{
		index = name.find_last_of ("_");
		if (index == std::string::npos)
			return "";
	}
	auto xIndex = name.find_last_of ("x");
	if (xIndex == std::string::npos || index > xIndex)
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
struct UIDescription::Impl
{
	CResourceDescription xmlFile;
	std::string filePath;
	
	mutable IController* controller {nullptr};
	IViewFactory* viewFactory {nullptr};
	Xml::IContentProvider* xmlContentProvider {nullptr};
	IBitmapCreator* bitmapCreator { nullptr};

	SharedPointer<UINode> nodes;
	SharedPointer<UIDescription> sharedResources;
	
	mutable std::deque<IController*> subControllerStack;
	std::deque<UINode*> nodeStack;
	
	bool restoreViewsMode {false};

	Optional<UINode*> variableBaseNode;

	UINode* getVariableBaseNode ()
	{
		if (!variableBaseNode)
		{
			if (nodes)
				variableBaseNode = nodes->getChildren ().findChildNode (MainNodeNames::kVariable);
		}
		return *variableBaseNode;
	}

	DispatchList<UIDescriptionListener*> listeners;
};

//-----------------------------------------------------------------------------
UIDescription::UIDescription (const CResourceDescription& xmlFile, IViewFactory* _viewFactory)
{
	impl = std::unique_ptr<Impl> (new Impl);
	impl->xmlFile = xmlFile;
	impl->viewFactory = _viewFactory;
	if (xmlFile.type == CResourceDescription::kStringType && xmlFile.u.name != nullptr)
		setFilePath (xmlFile.u.name);
	if (impl->viewFactory == nullptr)
		impl->viewFactory = getGenericViewFactory ();
}

//-----------------------------------------------------------------------------
UIDescription::UIDescription (Xml::IContentProvider* xmlContentProvider, IViewFactory* _viewFactory)
{
	impl = std::unique_ptr<Impl> (new Impl);
	impl->viewFactory = _viewFactory;
	impl->xmlContentProvider = xmlContentProvider;
	if (impl->viewFactory == nullptr)
		impl->viewFactory = getGenericViewFactory ();
}

//-----------------------------------------------------------------------------
UIDescription::~UIDescription () noexcept
{
}

//------------------------------------------------------------------------
void UIDescription::setFilePath (UTF8StringPtr path)
{
	impl->filePath = path;
	impl->xmlFile.u.name = impl->filePath.data (); // make sure that xmlFile.u.name points to valid memory
}

//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::getFilePath () const
{
	return impl->filePath.data ();
}

//-----------------------------------------------------------------------------
const CResourceDescription& UIDescription::getXmlFile () const
{
	return impl->xmlFile;
}

//-----------------------------------------------------------------------------
void UIDescription::addDefaultNodes ()
{
	if (impl->sharedResources)
		return;
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
			{ nullptr, nullptr },
		};
		int32_t i = 0;
		while (defaultFonts[i].name != nullptr)
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
			{ nullptr, kBlackCColor }
		};
		
		int32_t i = 0;
		while (defaultColors[i].name != nullptr)
		{
			UIAttributes* attr = new UIAttributes;
			attr->setAttribute ("name", defaultColors[i].name);
			std::string colorStr;
			UIViewCreator::colorToString (defaultColors[i].color, colorStr, nullptr);
			attr->setAttribute ("rgba", colorStr);
			UIColorNode* node = new UIColorNode ("color", attr);
			node->noExport (true);
			colorsNode->getChildren ().add (node);
			i++;
		}
	}
}

//-----------------------------------------------------------------------------
bool UIDescription::parsed () const
{
	return impl->nodes != nullptr;
}

//-----------------------------------------------------------------------------
void UIDescription::setXmlContentProvider (Xml::IContentProvider* provider)
{
	impl->xmlContentProvider = provider;
}

//-----------------------------------------------------------------------------
bool UIDescription::parse ()
{
	if (parsed ())
		return true;
	Xml::Parser parser;
	if (impl->xmlContentProvider)
	{
		if (parser.parse (impl->xmlContentProvider, this))
		{
			addDefaultNodes ();
			return true;
		}
	}
	else
	{
		CResourceInputStream resInputStream;
		if (resInputStream.open (impl->xmlFile))
		{
			Xml::InputStreamContentProvider contentProvider (resInputStream);
			if (parser.parse (&contentProvider, this))
			{
				addDefaultNodes ();
				return true;
			}
		}
		else if (impl->xmlFile.type == CResourceDescription::kStringType)
		{
			CFileStream fileStream;
			if (fileStream.open (impl->xmlFile.u.name, CFileStream::kReadMode))
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
	if (!impl->nodes)
	{
		impl->nodes = makeOwned<UINode> ("vstgui-ui-description");
		addDefaultNodes ();
	}
	return false;
}

//-----------------------------------------------------------------------------
void UIDescription::setController (IController* inController) const
{
	impl->controller = inController;
}

//-----------------------------------------------------------------------------
IController* UIDescription::getController () const
{
	return impl->controller;
}

//-----------------------------------------------------------------------------
const IViewFactory* UIDescription::getViewFactory () const
{
	return impl->viewFactory;
}

//-----------------------------------------------------------------------------
void UIDescription::registerListener (UIDescriptionListener* listener)
{
	impl->listeners.add (listener);
}

//-----------------------------------------------------------------------------
void UIDescription::unregisterListener (UIDescriptionListener* listener)
{
	impl->listeners.remove (listener);
}

//-----------------------------------------------------------------------------
void UIDescription::setBitmapCreator (IBitmapCreator* creator)
{
	impl->bitmapCreator = creator;
}

//-----------------------------------------------------------------------------
static void FreeNodePlatformResources (UINode* node)
{
	for (auto& child : node->getChildren ())
	{
		child->freePlatformResources ();
		FreeNodePlatformResources (child);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::freePlatformResources ()
{
	if (impl->nodes)
		FreeNodePlatformResources (impl->nodes);
}

//-----------------------------------------------------------------------------
bool UIDescription::saveWindowsRCFile (UTF8StringPtr filename)
{
	if (impl->sharedResources)
		return true;
	bool result = false;
	UINode* bitmapNodes = getBaseNode (MainNodeNames::kBitmap);
	if (bitmapNodes && !bitmapNodes->getChildren().empty ())
	{
		CFileStream stream;
		if (stream.open (filename, CFileStream::kWriteMode|CFileStream::kTruncateMode))
		{
			for (auto& childNode : bitmapNodes->getChildren ())
			{
				UIAttributes* attr = childNode->getAttributes ();
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
	impl->listeners.forEach ([this] (UIDescriptionListener* l) {
		l->beforeUIDescSave (this);
	});
	if (!impl->sharedResources)
	{
		UINode* bitmapNodes = getBaseNode (MainNodeNames::kBitmap);
		if (bitmapNodes)
		{
			for (auto& childNode : bitmapNodes->getChildren ())
			{
				UIBitmapNode* bitmapNode = dynamic_cast<UIBitmapNode*> (childNode);
				if (bitmapNode)
				{
					if (flags & kWriteImagesIntoXMLFile)
						bitmapNode->createXMLData (impl->filePath);
					else
						bitmapNode->removeXMLData ();
					
				}
			}
		}
	}
	impl->nodes->getAttributes ()->setAttribute ("version", "1");
	UIDescWriter writer;
	return writer.write (stream, impl->nodes);
}

//-----------------------------------------------------------------------------
void UIDescription::setSharedResources (const SharedPointer<UIDescription>& resources)
{
	impl->sharedResources = resources;
}

//-----------------------------------------------------------------------------
const SharedPointer<UIDescription>& UIDescription::getSharedResources () const
{
	return impl->sharedResources;
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
		UINode* node = nullptr;
		for (const auto& itNode : impl->nodes->getChildren ())
		{
			if (itNode->getName () == MainNodeNames::kTemplate)
			{
				const std::string* nodeName = itNode->getAttributes ()->getAttributeValue ("name");
				if (nodeName && *nodeName == templateName)
				{
					node = itNode;
					break;
				}
			}
		}
		if (node)
		{
			while (view != parentView)
			{
				if (view == parentView)
					return node;
				CViewContainer* container = parentView->asViewContainer ();
				vstgui_assert (container != nullptr);
				UIDescList::iterator nodeIterator = node->getChildren ().begin ();
				CViewContainer* childContainer = nullptr;
				ViewIterator it (container);
				while (*it && nodeIterator != node->getChildren ().end ())
				{
					if (*it == view)
					{
						node = *nodeIterator;
						parentView = view;
						break;
					}
					childContainer = (*it)->asViewContainer ();
					if (childContainer && childContainer->isChild (view, true))
					{
						break;
					}
					childContainer = nullptr;
					++nodeIterator;
					++it;
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
	return nullptr;
}

//-----------------------------------------------------------------------------
bool UIDescription::storeViews (const std::list<CView*>& views, OutputStream& stream, UIAttributes* customData) const
{
	UIDescList nodeList (false);
	for (auto& view : views)
	{
		UINode* node = findNodeForView (view);
		if (node)
		{
			nodeList.add (node);
		}
		else
		{
		#if VSTGUI_LIVE_EDITING
			UIAttributes* attr = new UIAttributes;
			UIViewFactory* factory = dynamic_cast<UIViewFactory*> (impl->viewFactory);
			if (factory)
			{
				if (factory->getAttributesForView (view, const_cast<UIDescription*> (this), *attr) == false)
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
	SharedPointer<UINode> baseNode;
	if (impl->nodes)
	{
		auto origNodes = std::move (impl->nodes);
		impl->nodes = baseNode;

		Xml::InputStreamContentProvider contentProvider (stream);
		Xml::Parser parser;
		if (parser.parse (&contentProvider, this))
		{
			baseNode = impl->nodes;
		}
		impl->nodes = std::move (origNodes);
	}
	if (baseNode)
	{
		UIDescList& children = baseNode->getChildren ();
		for (auto& childNode : children)
		{
			if (childNode->getName() == MainNodeNames::kCustom)
			{
				if (customData)
				{
					*customData = childNode->getAttributes ();
					(*customData)->remember ();
				}
			}
			else
			{
				CView* view = createViewFromNode (childNode);
				if (view)
				{
					views.emplace_back (view);
					view->forget ();
				}
			}
		}
	}
	return views.empty () == false;
}

//-----------------------------------------------------------------------------
CView* UIDescription::createViewFromNode (UINode* node) const
{
	const std::string* templateName = node->getAttributes ()->getAttributeValue (MainNodeNames::kTemplate);
	if (templateName)
	{
		CView* view = createView (templateName->c_str (), impl->controller);
		if (view)
			impl->viewFactory->applyAttributeValues (view, *node->getAttributes (), this);
		return view;
	}

	IController* subController = nullptr;
	CView* result = nullptr;
	if (impl->controller)
	{
		const std::string* subControllerName = node->getAttributes ()->getAttributeValue ("sub-controller");
		if (subControllerName)
		{
			subController = impl->controller->createSubController (subControllerName->c_str (), this);
			if (subController)
			{
				impl->subControllerStack.emplace_back (impl->controller);
				setController (subController);
			}
		}
		result = impl->controller->createView (*node->getAttributes (), this);
		if (result && impl->viewFactory)
		{
			const std::string* viewClass = node->getAttributes ()->getAttributeValue (UIViewCreator::kAttrClass);
			if (viewClass)
				impl->viewFactory->applyCustomViewAttributeValues (result, viewClass->c_str (), *node->getAttributes (), this);
		}
	}
	if (result == nullptr && impl->viewFactory)
	{
		result = impl->viewFactory->createView (*node->getAttributes (), this);
		if (result == nullptr)
		{
			result = new CViewContainer (CRect (0, 0, 0, 0));
			impl->viewFactory->applyCustomViewAttributeValues (result, "CViewContainer", *node->getAttributes (), this);
		}
	}
	if (result && node->hasChildren ())
	{
		CViewContainer* viewContainer = result->asViewContainer ();
		for (const auto& itNode : node->getChildren ())
		{
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
						attrId = (CViewAttributeID)strtol (attrName->c_str (), nullptr, 10);
					if (attrId)
						result->setAttribute (attrId, static_cast<uint32_t> (attrValue->size () + 1), attrValue->c_str ());
				}
			}
		}
	}
	if (result && impl->controller)
		result = impl->controller->verifyView (result, *node->getAttributes (), this);
	if (subController)
	{
		if (result)
			result->setAttribute (kCViewControllerAttribute, sizeof (IController*), &subController);
		setController (impl->subControllerStack.back ());
		impl->subControllerStack.pop_back ();
		if (result == nullptr)
		{
			auto obj = dynamic_cast<IReference*> (subController);
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
	ScopePointer<IController> sp (&impl->controller, _controller);
	if (impl->nodes)
	{
		for (const auto& itNode : impl->nodes->getChildren ())
		{
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
		}
	}
	return nullptr;
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
	if (impl->nodes)
	{
		for (const auto& itNode : impl->nodes->getChildren ())
		{
			if (itNode->getName () == MainNodeNames::kTemplate)
			{
				const std::string* nodeName = itNode->getAttributes ()->getAttributeValue ("name");
				if (nodeName && *nodeName == name)
					return itNode->getAttributes ();
			}
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
UINode* UIDescription::getBaseNode (UTF8StringPtr name) const
{
	UTF8StringView nameView (name);
	nameView.calculateByteCount ();
	if (impl->sharedResources)
	{
		if (nameView == MainNodeNames::kBitmap || nameView == MainNodeNames::kFont || nameView == MainNodeNames::kColor || nameView == MainNodeNames::kGradient)
		{
			return impl->sharedResources->getBaseNode (name);
		}
	}
	if (impl->nodes)
	{
		UINode* node = impl->nodes->getChildren ().findChildNode (nameView);
		if (node)
			return node;

		node = new UINode (name);
		impl->nodes->getChildren ().add (node);
		return node;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
UINode* UIDescription::findChildNodeByNameAttribute (UINode* node, UTF8StringPtr nameAttribute) const
{
	if (node)
		return node->getChildren ().findChildNodeWithAttributeValue ("name", nameAttribute);
	return nullptr;
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
	if (impl->controller)
		tag = impl->controller->getTagForName (name, tag);
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
	if (impl->controller)
		return impl->controller->getControlListener (name);
	return nullptr;
}

//-----------------------------------------------------------------------------
CBitmap* UIDescription::getBitmap (UTF8StringPtr name) const
{
	UIBitmapNode* bitmapNode = dynamic_cast<UIBitmapNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kBitmap), name));
	if (bitmapNode)
	{
		CBitmap* bitmap = bitmapNode->getBitmap (impl->filePath);
		if (impl->bitmapCreator && bitmap && bitmap->getPlatformBitmap () == nullptr)
		{
			auto platformBitmap = impl->bitmapCreator->createBitmap (*bitmapNode->getAttributes ());
			if (platformBitmap)
			{
				double scaleFactor;
				if (UIDescriptionPrivate::decodeScaleFactorFromName (name, scaleFactor))
					platformBitmap->setScaleFactor (scaleFactor);
				bitmap->setPlatformBitmap (platformBitmap);
			}
		}
		if (bitmap && bitmapNode->getFilterProcessed () == false)
		{
			std::list<SharedPointer<BitmapFilter::IFilter> > filters;
			for (auto& childNode : bitmapNode->getChildren ())
			{
				const std::string* filterName = nullptr;
				if (childNode->getName () == "filter" && (filterName = childNode->getAttributes ()->getAttributeValue ("name")))
				{
					auto filter = owned (BitmapFilter::Factory::getInstance().createFilter (filterName->c_str ()));
					if (filter == nullptr)
						continue;
					filters.emplace_back (filter);
					for (auto& propertyNode : childNode->getChildren ())
					{
						if (propertyNode->getName () != "property")
							continue;
						const std::string* name = propertyNode->getAttributes ()->getAttributeValue ("name");
						if (name == nullptr)
							continue;
						switch (filter->getProperty (name->c_str ()).getType ())
						{
							case BitmapFilter::Property::kInteger:
							{
								int32_t intValue;
								if (propertyNode->getAttributes ()->getIntegerAttribute ("value", intValue))
									filter->setProperty (name->c_str (), intValue);
								break;
							}
							case BitmapFilter::Property::kFloat:
							{
								double floatValue;
								if (propertyNode->getAttributes ()->getDoubleAttribute ("value", floatValue))
									filter->setProperty (name->c_str (), floatValue);
								break;
							}
							case BitmapFilter::Property::kPoint:
							{
								CPoint pointValue;
								if (propertyNode->getAttributes ()->getPointAttribute ("value", pointValue))
									filter->setProperty (name->c_str (), pointValue);
								break;
							}
							case BitmapFilter::Property::kRect:
							{
								CRect rectValue;
								if (propertyNode->getAttributes ()->getRectAttribute ("value", rectValue))
									filter->setProperty (name->c_str (), rectValue);
								break;
							}
							case BitmapFilter::Property::kColor:
							{
								const std::string* colorString = propertyNode->getAttributes()->getAttributeValue ("value");
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
			for (auto& filter : filters)
			{
				filter->setProperty (BitmapFilter::Standard::Property::kInputBitmap, bitmap);
				if (filter->run ())
				{
					auto obj = filter->getProperty (BitmapFilter::Standard::Property::kOutputBitmap).getObject ();
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
				for (auto& it : bitmapsNode->getChildren ())
				{
					UIBitmapNode* childNode = dynamic_cast<UIBitmapNode*>(it);
					if (childNode == nullptr || childNode == bitmapNode)
						continue;
					const std::string* childNodeBitmapName = childNode->getAttributes()->getAttributeValue ("name");
					if (childNodeBitmapName == nullptr)
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
	return nullptr;
}

//-----------------------------------------------------------------------------
CFontRef UIDescription::getFont (UTF8StringPtr name) const
{
	UIFontNode* fontNode = dynamic_cast<UIFontNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kFont), name));
	if (fontNode)
		return fontNode->getFont ();
	return nullptr;
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
	return nullptr;
}

//-----------------------------------------------------------------------------
template<typename NodeType, typename ObjType, typename CompareFunction> UTF8StringPtr UIDescription::lookupName (const ObjType& obj, IdStringPtr mainNodeName, CompareFunction compare) const
{
	UINode* baseNode = getBaseNode (mainNodeName);
	if (baseNode)
	{
		UIDescList& children = baseNode->getChildren ();
		for (const auto& itNode : children)
		{
			NodeType* node = dynamic_cast<NodeType*>(itNode);
			if (node && compare (this, node, obj))
			{
				const std::string* name = node->getAttributes ()->getAttributeValue ("name");
				return name ? name->c_str () : nullptr;
			}
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::lookupColorName (const CColor& color) const
{
	return lookupName<UIColorNode> (color, MainNodeNames::kColor, [] (const UIDescription* desc, UIColorNode* node, const CColor& color) {
		return node->getColor() == color;
	});
}

//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::lookupFontName (const CFontRef font) const
{
	return font ? lookupName<UIFontNode> (font, MainNodeNames::kFont, [] (const UIDescription* desc, UIFontNode* node, const CFontRef& font) {
		return node->getFont () && node->getFont () == font;
	}) : nullptr;
}

//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::lookupBitmapName (const CBitmap* bitmap) const
{
	return bitmap ? lookupName<UIBitmapNode> (bitmap, MainNodeNames::kBitmap, [] (const UIDescription* desc, UIBitmapNode* node, const CBitmap* bitmap) {
		return node->getBitmap (desc->impl->filePath) == bitmap;
	}) : nullptr;
}

//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::lookupGradientName (const CGradient* gradient) const
{
	return gradient ? lookupName<UIGradientNode> (gradient, MainNodeNames::kGradient, [] (const UIDescription* desc, UIGradientNode* node, const CGradient* gradient) {
		return node->getGradient() == gradient || (node->getGradient () && gradient->getColorStops () == node->getGradient ()->getColorStops ());
	}) : nullptr;
}
	
//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::lookupControlTagName (const int32_t tag) const
{
	return lookupName<UIControlTagNode> (tag, MainNodeNames::kControlTag, [] (const UIDescription* desc, UIControlTagNode* node, const int32_t tag) {
		int32_t nodeTag = node->getTag ();
		if (nodeTag == -1 && node->getTagString ())
		{
			double v;
			if (desc->calculateStringValue (node->getTagString ()->c_str (), v))
				nodeTag = (int32_t)v;
		}
		return nodeTag == tag;
	});
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
	impl->listeners.forEach ([this] (UIDescriptionListener* l) {
		l->onUIDescColorChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::changeTagName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	changeNodeName<UIControlTagNode> (oldName, newName, MainNodeNames::kControlTag, kMessageTagChanged);
	impl->listeners.forEach ([this] (UIDescriptionListener* l) {
		l->onUIDescTagChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::changeFontName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	changeNodeName<UIFontNode> (oldName, newName, MainNodeNames::kFont, kMessageFontChanged);
	impl->listeners.forEach ([this] (UIDescriptionListener* l) {
		l->onUIDescFontChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::changeBitmapName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	changeNodeName<UIBitmapNode> (oldName, newName, MainNodeNames::kBitmap, kMessageBitmapChanged);
	impl->listeners.forEach ([this] (UIDescriptionListener* l) {
		l->onUIDescBitmapChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::changeGradientName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	changeNodeName<UIGradientNode> (oldName, newName, MainNodeNames::kGradient, kMessageGradientChanged);
	impl->listeners.forEach ([this] (UIDescriptionListener* l) {
		l->onUIDescGradientChanged (this);
	});
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
			impl->listeners.forEach ([this] (UIDescriptionListener* l) {
				l->onUIDescColorChanged (this);
			});
		}
	}
	else
	{
		if (colorsNode)
		{
			UIAttributes* attr = new UIAttributes;
			attr->setAttribute ("name", name);
			std::string colorStr;
			UIViewCreator::colorToString (newColor, colorStr, nullptr);
			attr->setAttribute ("rgba", colorStr);
			UIColorNode* node = new UIColorNode ("color", attr);
			colorsNode->getChildren ().add (node);
			colorsNode->sortChildren ();
			changed (kMessageColorChanged);
			impl->listeners.forEach ([this] (UIDescriptionListener* l) {
				l->onUIDescColorChanged (this);
			});
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
			impl->listeners.forEach ([this] (UIDescriptionListener* l) {
				l->onUIDescFontChanged (this);
			});
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
			impl->listeners.forEach ([this] (UIDescriptionListener* l) {
				l->onUIDescFontChanged (this);
			});
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
			impl->listeners.forEach ([this] (UIDescriptionListener* l) {
				l->onUIDescGradientChanged (this);
			});
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
			impl->listeners.forEach ([this] (UIDescriptionListener* l) {
				l->onUIDescGradientChanged (this);
			});
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
			impl->listeners.forEach ([this] (UIDescriptionListener* l) {
				l->onUIDescBitmapChanged (this);
			});
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
			impl->listeners.forEach ([this] (UIDescriptionListener* l) {
				l->onUIDescBitmapChanged (this);
			});
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
		for (const auto& filter : filters)
		{
			const std::string* filterName = filter->getAttributeValue ("name");
			if (filterName == nullptr)
				continue;
			UINode* filterNode = new UINode ("filter");
			filterNode->getAttributes ()->setAttribute ("name", *filterName);
			for (auto& it2 : *filter)
			{
				if (it2.first == "name")
					continue;
				UINode* propertyNode = new UINode ("property");
				propertyNode->getAttributes ()->setAttribute("name", it2.first);
				propertyNode->getAttributes ()->setAttribute("value", it2.second);
				filterNode->getChildren ().add (propertyNode);
			}
			bitmapNode->getChildren ().add (filterNode);
		}
		bitmapNode->invalidBitmap ();
		changed (kMessageBitmapChanged);
		impl->listeners.forEach ([this] (UIDescriptionListener* l) {
			l->onUIDescBitmapChanged (this);
		});
	}
}

//-----------------------------------------------------------------------------
void UIDescription::collectBitmapFilters (UTF8StringPtr bitmapName, std::list<SharedPointer<UIAttributes> >& filters) const
{
	UIBitmapNode* bitmapNode = dynamic_cast<UIBitmapNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kBitmap), bitmapName));
	if (bitmapNode)
	{
		for (auto& childNode : bitmapNode->getChildren ())
		{
			if (childNode->getName () == "filter")
			{
				const std::string* filterName = childNode->getAttributes ()->getAttributeValue ("name");
				if (filterName == nullptr)
					continue;
				UIAttributes* attributes = new UIAttributes ();
				attributes->setAttribute ("name", *filterName);
				for (auto& it2 : childNode->getChildren ())
				{
					if (it2->getName () == "property")
					{
						const std::string* name = it2->getAttributes ()->getAttributeValue ("name");
						const std::string* value = it2->getAttributes ()->getAttributeValue ("value");
						if (name && value)
						{
							attributes->setAttribute (*name, *value);
						}
					}
				}
				filters.emplace_back (attributes);
				attributes->forget ();
			}
		}
	}
}

//-----------------------------------------------------------------------------
static void removeChildNode (UINode* baseNode, UTF8StringPtr nodeName)
{
	UIDescList& children = baseNode->getChildren ();
	for (const auto& itNode : children)
	{
		const std::string* name = itNode->getAttributes ()->getAttributeValue ("name");
		if (name && *name == nodeName)
		{
			if (!itNode->noExport ())
				children.remove (itNode);
			return;
		}
	}
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
	impl->listeners.forEach ([this] (UIDescriptionListener* l) {
		l->onUIDescColorChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::removeTag (UTF8StringPtr name)
{
	removeNode (name, MainNodeNames::kControlTag, kMessageTagChanged);
	impl->listeners.forEach ([this] (UIDescriptionListener* l) {
		l->onUIDescTagChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::removeFont (UTF8StringPtr name)
{
	removeNode (name, MainNodeNames::kFont, kMessageFontChanged);
	impl->listeners.forEach ([this] (UIDescriptionListener* l) {
		l->onUIDescFontChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::removeBitmap (UTF8StringPtr name)
{
	removeNode (name, MainNodeNames::kBitmap, kMessageBitmapChanged);
	impl->listeners.forEach ([this] (UIDescriptionListener* l) {
		l->onUIDescBitmapChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::removeGradient (UTF8StringPtr name)
{
	removeNode (name, MainNodeNames::kGradient, kMessageGradientChanged);
	impl->listeners.forEach ([this] (UIDescriptionListener* l) {
		l->onUIDescGradientChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::changeAlternativeFontNames (UTF8StringPtr name, UTF8StringPtr alternativeFonts)
{
	UIFontNode* node = dynamic_cast<UIFontNode*> (findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kFont), name));
	if (node)
	{
		node->setAlternativeFontNames (alternativeFonts);
		changed (kMessageFontChanged);
		impl->listeners.forEach ([this] (UIDescriptionListener* l) {
			l->onUIDescFontChanged (this);
		});
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
	if (!impl->nodes)
		return;
	for (const auto& itNode : impl->nodes->getChildren ())
	{
		if (itNode->getName () == MainNodeNames::kTemplate)
		{
			const std::string* nodeName = itNode->getAttributes ()->getAttributeValue ("name");
			if (nodeName)
				names.emplace_back (nodeName);
		}
	}
}

//-----------------------------------------------------------------------------
template<typename NodeType> void UIDescription::collectNamesFromNode (IdStringPtr mainNodeName, std::list<const std::string*>& names) const
{
	UINode* node = getBaseNode (mainNodeName);
	if (node)
	{
		UIDescList& children = node->getChildren ();
		for (const auto& itNode : children)
		{
			NodeType* node = dynamic_cast<NodeType*>(itNode);
			if (node)
			{
				const std::string* name = node->getAttributes ()->getAttributeValue ("name");
				if (name)
					names.emplace_back (name);
			}
		}
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
	UIViewFactory* factory = dynamic_cast<UIViewFactory*> (impl->viewFactory);
	std::list<std::string> attributeNames;
	CViewContainer* container = view->asViewContainer ();
	if (factory->getAttributeNamesForView (view, attributeNames))
	{
		for (auto& name : attributeNames)
		{
			std::string value;
			if (factory->getAttributeValue (view, name, value, this))
				node->getAttributes ()->setAttribute (name, std::move (value));
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
						for (auto& childNode : subNode->getChildren ())
						{
							childNode->remember ();
							node->getChildren ().add (childNode);
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
	bool doIt = true;
	impl->listeners.forEach ([&] (UIDescriptionListener* l) {
		if (!l->doUIDescTemplateUpdate (this, name))
			doIt = false;
	});
	if (!doIt)
		return;

	UIViewFactory* factory = dynamic_cast<UIViewFactory*> (impl->viewFactory);
	if (factory && impl->nodes)
	{
		UINode* node = nullptr;
		for (auto& childNode : impl->nodes->getChildren ())
		{
			if (childNode->getName () == MainNodeNames::kTemplate)
			{
				const std::string* nodeName = childNode->getAttributes ()->getAttributeValue ("name");
				if (*nodeName == name)
				{
					node = childNode;
					break;
				}
			}
		}
		if (node == nullptr)
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
	vstgui_assert (impl->nodes);
	UINode* templateNode = findChildNodeByNameAttribute (impl->nodes, name);
	if (templateNode == nullptr)
	{
		UINode* newNode = new UINode (MainNodeNames::kTemplate, attr);
		attr->setAttribute ("name", name);
		impl->nodes->getChildren ().add (newNode);
		changed (kMessageTemplateChanged);
		impl->listeners.forEach ([this] (UIDescriptionListener* l) {
			l->onUIDescTemplateChanged (this);
		});
		return true;
	}
#endif
	return false;
}

//-----------------------------------------------------------------------------
bool UIDescription::removeTemplate (UTF8StringPtr name)
{
#if VSTGUI_LIVE_EDITING
	UINode* templateNode = findChildNodeByNameAttribute (impl->nodes, name);
	if (templateNode)
	{
		impl->nodes->getChildren ().remove (templateNode);
		changed (kMessageTemplateChanged);
		impl->listeners.forEach ([this] (UIDescriptionListener* l) {
			l->onUIDescTemplateChanged (this);
		});
		return true;
	}
#endif
	return false;
}

//-----------------------------------------------------------------------------
bool UIDescription::changeTemplateName (UTF8StringPtr name, UTF8StringPtr newName)
{
#if VSTGUI_LIVE_EDITING
	UINode* templateNode = findChildNodeByNameAttribute (impl->nodes, name);
	if (templateNode)
	{
		templateNode->getAttributes()->setAttribute ("name", newName);
		changed (kMessageTemplateChanged);
		impl->listeners.forEach ([this] (UIDescriptionListener* l) {
			l->onUIDescTemplateChanged (this);
		});
		return true;
	}
#endif
	return false;
}

//-----------------------------------------------------------------------------
bool UIDescription::duplicateTemplate (UTF8StringPtr name, UTF8StringPtr duplicateName)
{
#if VSTGUI_LIVE_EDITING
	UINode* templateNode = findChildNodeByNameAttribute (impl->nodes, name);
	if (templateNode)
	{
		UINode* duplicate = new UINode (*templateNode);
		vstgui_assert (duplicate);
		if (duplicate)
		{
			duplicate->getAttributes()->setAttribute ("name", duplicateName);
			impl->nodes->getChildren ().add (duplicate);
			changed (kMessageTemplateChanged);
			impl->listeners.forEach ([this] (UIDescriptionListener* l) {
				l->onUIDescTemplateChanged (this);
			});
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
	UINode* parent = getBaseNode (MainNodeNames::kCustom);
	vstgui_assert (parent != nullptr);
	attr->setAttribute ("name", name);
	customNode = new UINode ("attributes", attr);
	parent->getChildren ().add (customNode);
	return true;
}

//-----------------------------------------------------------------------------
UIAttributes* UIDescription::getCustomAttributes (UTF8StringPtr name) const
{
	auto node = findChildNodeByNameAttribute (getBaseNode (MainNodeNames::kCustom), name);
	return node ? node->getAttributes () : nullptr;
}

//-----------------------------------------------------------------------------
UIAttributes* UIDescription::getCustomAttributes (UTF8StringPtr name, bool create)
{
	auto attributes = getCustomAttributes (name);
	if (attributes)
		return attributes;
	if (create)
	{
		UIAttributes* attributes = new UIAttributes ();
		setCustomAttributes (name, attributes);
		return attributes;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
UIDescription::FocusDrawing UIDescription::getFocusDrawingSettings () const
{
	FocusDrawing fd;
	auto attributes = getCustomAttributes ("FocusDrawing");
	if (attributes)
	{
		attributes->getBooleanAttribute ("enabled", fd.enabled);
		attributes->getDoubleAttribute ("width", fd.width);
		if (auto colorAttr = attributes->getAttributeValue ("color"))
			fd.colorName = *colorAttr;
	}
	return fd;
}

//-----------------------------------------------------------------------------
void UIDescription::setFocusDrawingSettings (const FocusDrawing& fd)
{
	auto attributes = getCustomAttributes ("FocusDrawing", true);
	if (!attributes)
		return;
	attributes->setBooleanAttribute ("enabled", fd.enabled);
	attributes->setDoubleAttribute ("width", fd.width);
	attributes->setAttribute ("color", fd.colorName.getString ());
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
		impl->listeners.forEach ([this] (UIDescriptionListener* l) {
			l->onUIDescTagChanged (this);
		});
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
			impl->listeners.forEach ([this] (UIDescriptionListener* l) {
				l->onUIDescTagChanged (this);
			});
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
bool UIDescription::getVariable (UTF8StringPtr name, double& value) const
{
	UIVariableNode* node = dynamic_cast<UIVariableNode*> (findChildNodeByNameAttribute (impl->getVariableBaseNode (), name));
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
	UIVariableNode* node = dynamic_cast<UIVariableNode*> (findChildNodeByNameAttribute (impl->getVariableBaseNode (), name));
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

	~Locale () noexcept
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
	
	explicit StringToken (const std::string& str) : std::string (str), type (kString), result (0) {}
	StringToken (const StringToken& token) = default;
	StringToken (Type type, double value = 0) : type (type), result (value) {}
	
	Type type;
	double result;
};

using StringTokenList = std::list<StringToken>;
	
//-----------------------------------------------------------------------------
static bool tokenizeString (std::string& str, StringTokenList& tokens)
{
	UTF8CodePointIterator<std::string::const_iterator> start (str.begin ());
	UTF8CodePointIterator<std::string::const_iterator> end (str.end ());
	auto iterator = start;
	while (iterator != end)
	{
		auto codePoint = *iterator;
		if (isspace (codePoint))
		{
			if (start != iterator)
				tokens.emplace_back (std::string {start.base (), iterator.base ()});
			start = iterator;
			++start;
		}
		else
		{
			switch (codePoint)
			{
				case '+':
				{
					if (start != iterator)
						tokens.emplace_back (std::string {start.base (), iterator.base ()});
					tokens.emplace_back (StringToken::kAdd);
					start = iterator;
					++start;
					break;
				}
				case '-':
				{
					if (start != iterator)
						tokens.emplace_back (std::string {start.base (), iterator.base ()});
					tokens.emplace_back (StringToken::kSubtract);
					start = iterator;
					++start;
					break;
				}
				case '*':
				{
					if (start != iterator)
						tokens.emplace_back (std::string {start.base (), iterator.base ()});
					tokens.emplace_back (StringToken::kMulitply);
					start = iterator;
					++start;
					break;
				}
				case '/':
				{
					if (start != iterator)
						tokens.emplace_back (std::string {start.base (), iterator.base ()});
					tokens.emplace_back (StringToken::kDivide);
					start = iterator;
					++start;
					break;
				}
				case '(':
				{
					if (start != iterator)
						tokens.emplace_back (std::string {start.base (), iterator.base ()});
					tokens.emplace_back (StringToken::kOpenParenthesis);
					start = iterator;
					++start;
					break;
				}
				case ')':
				{
					if (start != iterator)
						tokens.emplace_back (std::string {start.base (), iterator.base ()});
					tokens.emplace_back (StringToken::kCloseParenthesis);
					start = iterator;
					++start;
					break;
				}
			}
		}
		++iterator;
	}
	if (start != iterator)
		tokens.emplace_back (std::string {start.base (), iterator.base ()});
	return true;
}

//-----------------------------------------------------------------------------
static bool computeTokens (StringTokenList& tokens, double& result)
{
	int32_t openCount = 0;
	StringTokenList::iterator openPosition = tokens.end ();
	// first check parentheses
	for (StringTokenList::iterator it = tokens.begin (); it != tokens.end (); ++it)
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
					--openPosition;
					++it;
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
	for (StringTokenList::iterator it = tokens.begin (); it != tokens.end (); ++it)
	{
		if (prevToken != it)
		{
			if ((*it).type == StringToken::kMulitply)
			{
				if ((*prevToken).type == StringToken::kResult)
				{
					++it;
					if ((*it).type == StringToken::kResult)
					{
						double value = (*prevToken).result * (*it).result;
						++it;
						tokens.erase (prevToken, it);
						tokens.insert (it, StringToken (StringToken::kResult, value));
						--it;
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
					++it;
					if ((*it).type == StringToken::kResult)
					{
						double value = (*prevToken).result / (*it).result;
						++it;
						tokens.erase (prevToken, it);
						tokens.insert (it, StringToken (StringToken::kResult, value));
						--it;
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
	for (StringTokenList::const_iterator it = tokens.begin (); it != tokens.end (); ++it)
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

	char* endPtr = nullptr;
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
	for (auto& token : tokens)
	{
		if (token.type == UIDescriptionPrivate::StringToken::kString)
		{
			const char* tokenStr = token.c_str ();
			double value = strtod (tokenStr, &endPtr);
			if (endPtr != tokenStr + token.length ())
			{
				// if it is not pure numeric try to substitute the string with a control tag or variable
				if (token.find ("tag.") == 0)
				{
					value = getTagForName (token.c_str () + 4);
					if (value == -1)
					{
					#if DEBUG
						DebugPrint("Tag not found :%s\n", tokenStr);
					#endif
						return false;
					}
				}
				else if (token.find ("var.") == 0)
				{
					double v;
					if (getVariable (token.c_str () + 4, v))
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
			token.result = value;
			token.type = UIDescriptionPrivate::StringToken::kResult;
		}
	}
	result = 0;
	return UIDescriptionPrivate::computeTokens (tokens, result);
}

//-----------------------------------------------------------------------------
void UIDescription::startXmlElement (Xml::Parser* parser, IdStringPtr elementName, UTF8StringPtr* elementAttributes)
{
	std::string name (elementName);
	if (impl->nodes)
	{
		UINode* parent = impl->nodeStack.back ();
		UINode* newNode = nullptr;
		if (impl->restoreViewsMode)
		{
			if (name != "view" && name != MainNodeNames::kCustom)
			{
				parser->stop ();
			}
			newNode = new UINode (name, new UIAttributes (elementAttributes));
		}
		else
		{
			if (parent == impl->nodes)
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
			impl->nodeStack.emplace_back (newNode);
		}
	}
	else if (name == "vstgui-ui-description")
	{
		impl->nodes = makeOwned<UINode> (name, new UIAttributes (elementAttributes));
		impl->nodeStack.emplace_back (impl->nodes);
	}
	else if (name == "vstgui-ui-description-view-list")
	{
		vstgui_assert (impl->nodes == nullptr);
		impl->nodes = makeOwned<UINode> (name, new UIAttributes (elementAttributes));
		impl->nodeStack.emplace_back (impl->nodes);
		impl->restoreViewsMode = true;
	}
}

//-----------------------------------------------------------------------------
void UIDescription::endXmlElement (Xml::Parser* parser, IdStringPtr name)
{
	if (impl->nodeStack.back () == impl->nodes)
		impl->restoreViewsMode = false;
	impl->nodeStack.pop_back ();
}

//-----------------------------------------------------------------------------
void UIDescription::xmlCharData (Xml::Parser* parser, const int8_t* data, int32_t length)
{
	if (impl->nodeStack.size () == 0)
		return;
	auto& nodeData = impl->nodeStack.back ()->getData ();
	const int8_t* dataStart = nullptr;
	uint32_t validChars = 0;
	for (int32_t i = 0; i < length; i++, ++data)
	{
		if (*data < 0x21)
		{
			if (dataStart)
			{
				nodeData.append (reinterpret_cast<const char*> (dataStart), validChars);
				dataStart = nullptr;
				validChars = 0;
			}
			continue;
		}
		if (dataStart == nullptr)
			dataStart = data;
		++validChars;
	}
	if (dataStart && validChars > 0)
		nodeData.append (reinterpret_cast<const char*> (dataStart), validChars);
}

//-----------------------------------------------------------------------------
void UIDescription::xmlComment (Xml::Parser* parser, IdStringPtr comment)
{
#if VSTGUI_LIVE_EDITING
	if (impl->nodeStack.size () == 0)
	{
	#if DEBUG
		DebugPrint ("*** WARNING : Comment outside of root tag will be removed on save !\nComment: %s\n", comment);
	#endif
		return;
	}
	UINode* parent = impl->nodeStack.back ();
	if (parent && comment)
	{
		std::string commentStr (comment);
		if (!commentStr.empty ())
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
	if (attributes == nullptr)
		attributes = new UIAttributes ();
}

//-----------------------------------------------------------------------------
UINode::UINode (const std::string& _name, UIDescList* _children, UIAttributes* _attributes)
: name (_name)
, attributes (_attributes)
, children (_children)
, flags (0)
{
	vstgui_assert (children != nullptr);
	children->remember ();
	if (attributes == nullptr)
		attributes = new UIAttributes ();
}

//-----------------------------------------------------------------------------
UINode::UINode (const UINode& n)
: name (n.name)
, data (n.data)
, attributes (new UIAttributes (*n.attributes))
, children (new UIDescList (*n.children))
, flags (n.flags)
{
}

//-----------------------------------------------------------------------------
UINode::~UINode () noexcept
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
	data = comment;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UIVariableNode::UIVariableNode (const std::string& name, UIAttributes* attributes)
: UINode (name, attributes)
, type (kUnknown)
, number (0)
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
			char* endPtr = nullptr;
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
			number = strtod (strPtr, nullptr);
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
				char* endPtr = nullptr;
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
, bitmap (nullptr)
, filterProcessed (false)
, scaledBitmapsAdded (false)
{
}

//-----------------------------------------------------------------------------
UIBitmapNode::~UIBitmapNode () noexcept
{
	if (bitmap)
		bitmap->forget ();
}

//-----------------------------------------------------------------------------
void UIBitmapNode::freePlatformResources ()
{
	if (bitmap)
		bitmap->forget ();
	bitmap = nullptr;
}

//-----------------------------------------------------------------------------
void UIBitmapNode::createXMLData (const std::string& pathHint)
{
	UINode* node = getChildren ().findChildNode ("data");
	if (node == nullptr)
	{
		CBitmap* bitmap = getBitmap (pathHint);
		if (bitmap)
		{
			if (auto platformBitmap = bitmap->getPlatformBitmap ())
			{
				auto buffer = IPlatformBitmap::createMemoryPNGRepresentation (platformBitmap);
				if (!buffer.empty ())
				{
					auto result = Base64Codec::encode (buffer.data(), static_cast<uint32_t> (buffer.size ()));
					UINode* dataNode = new UINode ("data");
					dataNode->getAttributes ()->setAttribute ("encoding", "base64");
					dataNode->getData ().append (reinterpret_cast<const char*> (result.data.get ()), static_cast<std::streamsize> (result.dataSize));
					getChildren ().add (dataNode);
				}
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
	if (bitmap == nullptr)
	{
		const std::string* path = attributes->getAttributeValue ("path");
		if (path)
		{
			CNinePartTiledDescription partDesc;
			CNinePartTiledDescription* partDescPtr = nullptr;
			CRect offsets;
			if (attributes->getRectAttribute ("nineparttiled-offsets", offsets))
			{
				partDesc = CNinePartTiledDescription (offsets.left, offsets.top, offsets.right, offsets.bottom);
				partDescPtr = &partDesc;
			}
			bitmap = createBitmap (*path, partDescPtr);
			if (bitmap->getPlatformBitmap () == nullptr && pathIsAbsolute (pathHint))
			{
				std::string absPath = pathHint;
				if (removeLastPathComponent (absPath))
				{
					absPath += "/" + *path;
					if (auto platformBitmap = IPlatformBitmap::createFromPath (absPath.c_str ()))
						bitmap->setPlatformBitmap (platformBitmap);
				}
			}
		}
		if (bitmap && bitmap->getPlatformBitmap () == nullptr)
		{
			UINode* node = getChildren ().findChildNode ("data");
			if (node && !node->getData ().empty ())
			{
				const std::string* codec = node->getAttributes ()->getAttributeValue ("encoding");
				if (codec && *codec == "base64")
				{
					auto result = Base64Codec::decode (node->getData ());
					if (auto platformBitmap = IPlatformBitmap::createFromMemory (result.data.get (), result.dataSize))
					{
						double scaleFactor = 1.;
						if (attributes->getDoubleAttribute ("scale-factor", scaleFactor))
							platformBitmap->setScaleFactor (scaleFactor);
						bitmap->setPlatformBitmap (platformBitmap);
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
	bitmap = nullptr;
	double scaleFactor = 1.;
	if (UIDescriptionPrivate::decodeScaleFactorFromName (bitmapName, scaleFactor))
		attributes->setDoubleAttribute ("scale-factor", scaleFactor);
	removeXMLData ();
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
			bitmap = nullptr;
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
	bitmap = nullptr;
	filterProcessed = false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UIFontNode::UIFontNode (const std::string& name, UIAttributes* attributes)
: UINode (name, attributes)
, font (nullptr)
{
}

//-----------------------------------------------------------------------------
UIFontNode::~UIFontNode () noexcept
{
	if (font)
		font->forget ();
}

//-----------------------------------------------------------------------------
void UIFontNode::freePlatformResources ()
{
	if (font)
		font->forget ();
	font = nullptr;
}

//-----------------------------------------------------------------------------
CFontRef UIFontNode::getFont ()
{
	if (font == nullptr)
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
				size = (int32_t)strtol (sizeAttr->c_str (), nullptr, 10);
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
						for (auto& alternateFontName : alternativeFontNames)
						{
							auto trimmedString = trim (UTF8String (alternateFontName));
							if (std::find (fontNames.begin (), fontNames.end (), trimmedString.getString ()) != fontNames.end ())
							{
								font = new CFontDesc (trimmedString.data (), size, fontStyle);
								break;
							}
						}
					}
				}
			}
			if (font == nullptr)
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
	attributes->setAttribute ("font-name", newFont->getName ().getString ());
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
		color.red = (uint8_t)strtol (red->c_str (), nullptr, 10);
	if (green)
		color.green = (uint8_t)strtol (green->c_str (), nullptr, 10);
	if (blue)
		color.blue = (uint8_t)strtol (blue->c_str (), nullptr, 10);
	if (alpha)
		color.alpha = (uint8_t)strtol (alpha->c_str (), nullptr, 10);
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
	UIViewCreator::colorToString (newColor, colorString, nullptr);
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
	gradient = nullptr;
}

//-----------------------------------------------------------------------------
CGradient* UIGradientNode::getGradient ()
{
	if (gradient == nullptr)
	{
		CGradient::ColorStopMap colorStops;
		double start;
		CColor color;
		for (auto& colorNode : getChildren ())
		{
			if (colorNode->getName () == "color-stop")
			{
				const std::string* rgba = colorNode->getAttributes ()->getAttributeValue ("rgba");
				if (rgba == nullptr || colorNode->getAttributes()->getDoubleAttribute ("start", start) == false)
					continue;
				if (UIDescription::parseColor (*rgba, color) == false)
					continue;
				colorStops.emplace (start, color);
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
	if (gradient == nullptr)
		return;

	const CGradient::ColorStopMap colorStops = gradient->getColorStops ();
	for (const auto& colorStop : colorStops)
	{
		UINode* node = new UINode ("color-stop");
		node->getAttributes ()->setDoubleAttribute ("start", colorStop.first);
		std::string colorString;
		UIViewCreator::colorToString (colorStop.second, colorString, nullptr);
		node->getAttributes ()->setAttribute ("rgba", colorString);
		getChildren ().add (node);
	}
}

} // namespace
