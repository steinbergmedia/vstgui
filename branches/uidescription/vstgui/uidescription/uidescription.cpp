
#include "uidescription.h"
#include "viewfactory.h"
#include <list>

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
template <class T> class ScopePointer
{
public:
	ScopePointer (T** pointer, T* obj) : pointer (pointer) { if (pointer) *pointer = obj; }
	~ScopePointer () { if (pointer) *pointer = 0; }
protected:
	T** pointer;
};

class UIDescList;
//-----------------------------------------------------------------------------
class UINode : public CBaseObject
{
public:
	UINode (const std::string& name, UIAttributes* attributes);

	const std::string& getName () const { return name; }
	const UIAttributes* getAttributes () const { return attributes; }
	UIDescList& getChildren () const { return *children; }
	bool hasChildren () const;
	
protected:
	~UINode ();
	std::string name;
	UIAttributes* attributes;
	UIDescList* children;
};

//-----------------------------------------------------------------------------
class UIControlTagNode : public UINode
{
public:
	UIControlTagNode (const std::string& name, UIAttributes* attributes);
	long getTag ();
protected:
	long tag;
};

//-----------------------------------------------------------------------------
class UIBitmapNode : public UINode
{
public:
	UIBitmapNode (const std::string& name, UIAttributes* attributes);
	CBitmap* getBitmap ();
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
	~UIDescList () { riterator it = rbegin (); while (it != rend ()) remove (*it); }

	void add (UINode* obj) { if (!ownsObjects) obj->remember (); std::list<UINode*>::push_back (obj); }
	void remove (UINode* obj) { std::list<UINode*>::remove (obj); obj->forget (); }

	int total () { return size (); }

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
	int readRawXmlData (char* buffer, int size);
	void rewind ();

	CResourceDescription resFile;
	void* platformHandle;
};

//-----------------------------------------------------------------------------
static bool parseColor (const std::string& colorString, CColor& color)
{
	if (colorString.length () == 7)
	{
		if (colorString[0] == '#')
		{
			std::string rv (colorString.substr (1, 2));
			std::string gv (colorString.substr (3, 2));
			std::string bv (colorString.substr (5, 2));
			color.red = strtol (rv.c_str (), 0, 16);
			color.green = strtol (gv.c_str (), 0, 16);
			color.blue = strtol (bv.c_str (), 0, 16);
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
			color.red = strtol (rv.c_str (), 0, 16);
			color.green = strtol (gv.c_str (), 0, 16);
			color.blue = strtol (bv.c_str (), 0, 16);
			color.alpha = strtol (av.c_str (), 0, 16);
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
UIDescription::UIDescription (const CResourceDescription& xmlFile)
: xmlFile (xmlFile)
, nodes (0)
, controller (0)
, xmlContentProvider (0)
{
}

//-----------------------------------------------------------------------------
UIDescription::UIDescription (Xml::IContentProvider* xmlContentProvider)
: nodes (0)
, controller (0)
, xmlContentProvider (xmlContentProvider)
{
}

//-----------------------------------------------------------------------------
UIDescription::~UIDescription ()
{
	if (nodes)
		nodes->forget ();
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
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
CView* UIDescription::createViewFromNode (UINode* node, IViewFactory* viewFactory, IController* controller)
{
	const std::string* templateName = node->getAttributes ()->getAttributeValue ("template");
	if (templateName)
		return createView (templateName->c_str (), controller, viewFactory);

	CView* result = 0;
	if (controller)
		result = controller->createView (*node->getAttributes (), this);
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
					CView* childView = createViewFromNode (*it, viewFactory, controller);
					if (childView)
						viewContainer->addView (childView);
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
						attrId = ((((int)c1) << 24) | (((int)c2) << 16) | (((int)c3) << 8) | (((int)c4) << 0));
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
		controller->verifyView (result, *node->getAttributes (), this);
	return result;
}

//-----------------------------------------------------------------------------
CView* UIDescription::createView (const char* name, IController* _controller, IViewFactory* _viewFactory)
{
	ScopePointer<IController> sp (&controller, _controller);
	if (nodes)
	{
		IViewFactory* viewFactory = _viewFactory;
		if (viewFactory == 0)
		{
			static ViewFactory genericViewFactory;
			viewFactory = &genericViewFactory;
		}
		UIDescList::iterator it = nodes->getChildren ().begin ();
		while (it != nodes->getChildren ().end ())
		{
			if ((*it)->getName () == "template")
			{
				const std::string* nodeName = (*it)->getAttributes ()->getAttributeValue ("name");
				if (*nodeName == name)
					return createViewFromNode (*it, viewFactory, controller);
			}
			it++;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
const UIAttributes* UIDescription::getViewAttributes (const char* name)
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
UINode* UIDescription::getBaseNode (const char* name)
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
	}
	return 0;
}

//-----------------------------------------------------------------------------
UINode* UIDescription::findChildNodeByNameAttribute (UINode* node, const char* nameAttribute)
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
long UIDescription::getTagForName (const char* name)
{
	long tag = -1;
	if (controller)
		tag = controller->getTagForName (name);
	if (tag == -1)
	{
		UIControlTagNode* controlTagNode = dynamic_cast<UIControlTagNode*> (findChildNodeByNameAttribute (getBaseNode ("control-tags"), name));
		if (controlTagNode)
			tag = controlTagNode->getTag ();
	}
	return tag;
}

//-----------------------------------------------------------------------------
CControlListener* UIDescription::getControlListener (const char* name)
{
	if (controller)
		return controller->getControlListener (name);
	return 0;
}

//-----------------------------------------------------------------------------
CBitmap* UIDescription::getBitmap (const char* name)
{
	UIBitmapNode* bitmapNode = dynamic_cast<UIBitmapNode*> (findChildNodeByNameAttribute (getBaseNode ("bitmaps"), name));
	if (bitmapNode)
	{
		return bitmapNode->getBitmap ();
	}
	return 0;
}

//-----------------------------------------------------------------------------
CFontRef UIDescription::getFont (const char* name)
{
	UIFontNode* fontNode = dynamic_cast<UIFontNode*> (findChildNodeByNameAttribute (getBaseNode ("fonts"), name));
	if (fontNode)
	{
		return fontNode->getFont ();
	}
	return 0;
}

//-----------------------------------------------------------------------------
bool UIDescription::getColor (const char* name, CColor& color)
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
	if (colorName == "black")
	{
		color = kBlackCColor;
		return true;
	}
	else if (colorName == "white")
	{
		color = kWhiteCColor;
		return true;
	}
	else if (colorName == "grey")
	{
		color = kGreyCColor;
		return true;
	}
	else if (colorName == "red")
	{
		color = kRedCColor;
		return true;
	}
	else if (colorName == "green")
	{
		color = kGreenCColor;
		return true;
	}
	else if (colorName == "blue")
	{
		color = kBlueCColor;
		return true;
	}
	else if (colorName == "yellow")
	{
		color = kYellowCColor;
		return true;
	}
	else if (colorName == "cyan")
	{
		color = kCyanCColor;
		return true;
	}
	else if (colorName == "magenta")
	{
		color = kMagentaCColor;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void UIDescription::startXmlElement (Xml::Parser* parser, const char* elementName, const char** elementAttributes)
{
	std::string name (elementName);
	if (nodes)
	{
		UINode* parent = nodeStack.back ();
		UINode* newNode = 0;
		if (parent == nodes)
		{
			// only allowed second level elements
			if (name == "bitmaps" || name == "fonts" || name == "colors" || name == "template" || name == "control-tags")
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
		nodes = new UINode ("root", new UIAttributes (elementAttributes));
		nodeStack.push_back (nodes);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::endXmlElement (Xml::Parser* parser, const char* name)
{
	nodeStack.pop_back ();
}

//-----------------------------------------------------------------------------
void UIDescription::xmlCharData (Xml::Parser* parser, const char* data, int length)
{
}

//-----------------------------------------------------------------------------
void UIDescription::xmlComment (Xml::Parser* parser, const char* comment)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UINode::UINode (const std::string& _name, UIAttributes* _attributes)
: name (_name)
, attributes (_attributes)
, children (new UIDescList)
{
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
long UIControlTagNode::getTag ()
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
				tag = ((((int)c1) << 24) | (((int)c2) << 16) | (((int)c3) << 8) | (((int)c4) << 0));
			}
			else
				tag = strtol (tagStr->c_str (), 0, 10);
		}
	}
	return tag;
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
			bitmap = new CBitmap (path->c_str ());
		}
	}
	return bitmap;
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
			int size = 12;
			if (sizeAttr)
				size = strtol (sizeAttr->c_str (), 0, 10);
			int fontStyle = 0;
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
		color.red = strtol (red->c_str (), 0, 10);
	if (green)
		color.green = strtol (green->c_str (), 0, 10);
	if (blue)
		color.blue = strtol (blue->c_str (), 0, 10);
	if (alpha)
		color.alpha = strtol (alpha->c_str (), 0, 10);
	if (rgb)
		parseColor (*rgb, color);
	if (rgba)
		parseColor (*rgba, color);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UIAttributes::UIAttributes (const char** attributes)
{
	if (attributes)
	{
		int i = 0;
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
bool UIAttributes::hasAttribute (const char* name) const
{
	if (getAttributeValue (name) != 0)
		return true;
	return false;
}

//-----------------------------------------------------------------------------
const std::string* UIAttributes::getAttributeValue (const char* name) const
{
	std::map<std::string, std::string>::const_iterator iter = find (name);
	if (iter != end ())
		return &iter->second;
	return 0;
}

//-----------------------------------------------------------------------------
void UIAttributes::setAttribute (const char* name, const char* value)
{
	std::map<std::string, std::string>::iterator iter = find (name);
	if (iter != end ())
		erase (iter);
	insert (std::make_pair (name, value));
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
	#if MAC
	if (platformHandle)
		fclose ((FILE*)platformHandle);
	#elif WINDOWS
	// TODO: Windows implementation
	#endif
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
			sprintf (filename, "%05d.uidesc", (int)resFile.u.id);
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
	// TODO: Windows implementation
	#endif
	return result;
}

//-----------------------------------------------------------------------------
int ResourceReader::readRawXmlData (char* buffer, int size)
{
	if (platformHandle)
	{
		#if MAC
		return fread (buffer, 1, size, (FILE*)platformHandle);
		#elif WINDOWS
		// TODO: Windows implementation
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
		// TODO: Windows implementation
		#endif
	}
}

END_NAMESPACE_VSTGUI
