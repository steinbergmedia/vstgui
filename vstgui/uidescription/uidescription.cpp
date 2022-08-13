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
#include "uicontentprovider.h"
#include "uiviewswitchcontainer.h"
#include "icontroller.h"
#include "xmlparser.h"
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
#include "detail/locale.h"
#include "detail/parsecolor.h"
#include "detail/scalefactorutils.h"
#include "detail/uidesclist.h"
#include "detail/uijsonpersistence.h"
#include "detail/uinode.h"
#include "detail/uiviewcreatorattributes.h"
#include "detail/uixmlpersistence.h"
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


/// @endcond

//-----------------------------------------------------------------------------
static UIViewFactory* getGenericViewFactory ()
{
	static UIViewFactory genericViewFactory;
	return &genericViewFactory;
}

IdStringPtr IUIDescription::kCustomViewName = "custom-view-name";

//-----------------------------------------------------------------------------
struct UIDescription::Impl : ListenerProvider<Impl, UIDescriptionListener>
{
	using UINode = Detail::UINode;
	
	CResourceDescription uidescFile;
	std::string filePath;
	
	mutable IController* controller {nullptr};
	IViewFactory* viewFactory {nullptr};
	IContentProvider* contentProvider {nullptr};
	IBitmapCreator* bitmapCreator { nullptr};
	IBitmapCreator2* bitmapCreator2 { nullptr};
	AttributeSaveFilterFunc attributeSaveFilterFunc {nullptr};

	SharedPointer<UINode> nodes;
	SharedPointer<UIDescription> sharedResources;
	
	mutable std::deque<IController*> subControllerStack;
	
	Optional<UINode*> variableBaseNode;

	UINode* getVariableBaseNode ()
	{
		if (!variableBaseNode)
		{
			if (nodes)
				variableBaseNode = nodes->getChildren ().findChildNode (Detail::MainNodeNames::kVariable);
		}
		return *variableBaseNode;
	}
};

//-----------------------------------------------------------------------------
UIDescription::UIDescription (const CResourceDescription& uidescFile, IViewFactory* _viewFactory)
{
	impl = std::unique_ptr<Impl> (new Impl);
	impl->uidescFile = uidescFile;
	impl->viewFactory = _viewFactory;
	if (uidescFile.type == CResourceDescription::kStringType && uidescFile.u.name != nullptr)
		setFilePath (uidescFile.u.name);
	if (impl->viewFactory == nullptr)
		impl->viewFactory = getGenericViewFactory ();
}

//-----------------------------------------------------------------------------
UIDescription::UIDescription (IContentProvider* contentProvider, IViewFactory* _viewFactory)
{
	impl = std::unique_ptr<Impl> (new Impl);
	impl->viewFactory = _viewFactory;
	impl->contentProvider = contentProvider;
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
	impl->uidescFile.u.name = impl->filePath.data (); // make sure that xmlFile.u.name points to valid memory
}

//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::getFilePath () const
{
	return impl->filePath.data ();
}

//-----------------------------------------------------------------------------
const CResourceDescription& UIDescription::getUIDescFile () const
{
	return impl->uidescFile;
}

//-----------------------------------------------------------------------------
void UIDescription::addDefaultNodes ()
{
	if (impl->sharedResources)
		return;
	UINode* fontsNode = getBaseNode (Detail::MainNodeNames::kFont);
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
			auto attr = makeOwned<UIAttributes> ();
			attr->setAttribute ("name", defaultFonts[i].name);
			Detail::UIFontNode* node = new Detail::UIFontNode ("font", attr);
			node->setFont (defaultFonts[i].font);
			node->noExport (true);
			fontsNode->getChildren ().add (node);
			i++;
		}
	}
	UINode* colorsNode = getBaseNode (Detail::MainNodeNames::kColor);
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
			auto attr = makeOwned<UIAttributes> ();
			attr->setAttribute ("name", defaultColors[i].name);
			std::string colorStr;
			UIViewCreator::colorToString (defaultColors[i].color, colorStr, nullptr);
			attr->setAttribute ("rgba", colorStr);
			Detail::UIColorNode* node = new Detail::UIColorNode ("color", attr);
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
void UIDescription::setContentProvider (IContentProvider* provider)
{
	impl->contentProvider = provider;
}

//-----------------------------------------------------------------------------
bool UIDescription::parse ()
{
	if (parsed ())
		return true;
		
	static auto parseUIDesc = [] (IContentProvider* contentProvider) -> SharedPointer<UINode> {
		if (auto nodes = Detail::UIJsonDescReader::read (*contentProvider))
			return nodes;
#if VSTGUI_ENABLE_XML_PARSER
		Detail::UIXMLParser parser;
		if (auto nodes = parser.parse (contentProvider))
			return nodes;
#endif
		return nullptr;
	};

	if (impl->contentProvider)
	{
		if ((impl->nodes = parseUIDesc (impl->contentProvider)))
		{
			addDefaultNodes ();
			return true;
		}
	}
	else
	{
		CResourceInputStream resInputStream;
		if (resInputStream.open (impl->uidescFile))
		{
			InputStreamContentProvider contentProvider (resInputStream);
			if ((impl->nodes = parseUIDesc (&contentProvider)))
			{
				addDefaultNodes ();
				return true;
			}
		}
		else if (impl->uidescFile.type == CResourceDescription::kStringType)
		{
			CFileStream fileStream;
			if (fileStream.open (impl->uidescFile.u.name, CFileStream::kReadMode))
			{
				InputStreamContentProvider contentProvider (fileStream);
				if ((impl->nodes = parseUIDesc (&contentProvider)))
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
	impl->registerListener (listener);
}

//-----------------------------------------------------------------------------
void UIDescription::unregisterListener (UIDescriptionListener* listener)
{
	impl->unregisterListener (listener);
}

//-----------------------------------------------------------------------------
void UIDescription::setBitmapCreator (IBitmapCreator* creator)
{
	impl->bitmapCreator = creator;
}

//------------------------------------------------------------------------
void UIDescription::setBitmapCreator2 (IBitmapCreator2* creator)
{
	impl->bitmapCreator2 = creator;
}

//-----------------------------------------------------------------------------
static void FreeNodePlatformResources (Detail::UINode* node)
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

//------------------------------------------------------------------------
auto UIDescription::getRootNode () const -> SharedPointer<UINode>
{
	return impl->nodes;
}

//-----------------------------------------------------------------------------
bool UIDescription::saveWindowsRCFile (UTF8StringPtr filename)
{
	if (impl->sharedResources)
		return true;
	bool result = false;
	UINode* bitmapNodes = getBaseNode (Detail::MainNodeNames::kBitmap);
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
bool UIDescription::save (UTF8StringPtr filename, int32_t flags, AttributeSaveFilterFunc func)
{
	std::string oldName = moveOldFile (filename);
	bool result = false;
	CFileStream stream;
	if (stream.open (filename, CFileStream::kWriteMode|CFileStream::kTruncateMode))
	{
		result = saveToStream (stream, flags, func);
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
bool UIDescription::saveToStream (OutputStream& stream, int32_t flags, AttributeSaveFilterFunc func)
{
	impl->attributeSaveFilterFunc = func;
	impl->forEachListener ([this] (UIDescriptionListener* l) {
		l->beforeUIDescSave (this);
	});
	impl->attributeSaveFilterFunc = nullptr;
	if (!impl->sharedResources)
	{
		UINode* bitmapNodes = getBaseNode (Detail::MainNodeNames::kBitmap);
		if (bitmapNodes)
		{
			for (auto& childNode : bitmapNodes->getChildren ())
			{
				if (auto* bitmapNode = dynamic_cast<Detail::UIBitmapNode*> (childNode))
				{
					if (flags & kWriteImagesIntoUIDescFile)
					{
						if (!(flags & kDoNotVerifyImageData) || !bitmapNode->hasXMLData ())
							bitmapNode->createXMLData (impl->filePath);
					}
					else
						bitmapNode->removeXMLData ();
				}
			}
		}
	}
	impl->nodes->getAttributes ()->setAttribute ("version", "1");
	
	BufferedOutputStream bufferedStream (stream);
	if (flags & kWriteAsXML)
	{
#if VSTGUI_ENABLE_XML_PARSER
		Detail::UIXMLDescWriter writer;
		return writer.write (bufferedStream, impl->nodes);
#else
#if DEBUG
		DebugPrint ("XML not available.");
#endif
		return false;
#endif
	}
	return Detail::UIJsonDescWriter::write (bufferedStream, impl->nodes);
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
Detail::UINode* UIDescription::findNodeForView (CView* view) const
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
			if (itNode->getName () == Detail::MainNodeNames::kTemplate)
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
				auto nodeIterator = node->getChildren ().begin ();
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
	auto nodeList = makeOwned<Detail::UIDescList> (false);
	for (auto& view : views)
	{
		UINode* node = findNodeForView (view);
		if (node)
		{
			nodeList->add (node);
		}
		else
		{
		#if VSTGUI_LIVE_EDITING
			if (auto* factory = dynamic_cast<UIViewFactory*> (impl->viewFactory))
			{
				auto attr = makeOwned<UIAttributes> ();
				if (factory->getAttributesForView (view, const_cast<UIDescription*> (this), *attr) == false)
					return false;
				UINode* newNode = new UINode ("view", attr);
				nodeList->add (newNode);
				newNode->forget ();
			}
		#endif
		}
	}
	if (!nodeList->empty ())
	{
		if (customData)
		{
			UINode* customNode = new UINode (Detail::MainNodeNames::kCustom, customData);
			nodeList->add (customNode);
			customNode->forget ();
			customData->remember ();
		}
		UINode baseNode ("vstgui-ui-description-view-list", nodeList);
		return Detail::UIJsonDescWriter::write (stream, &baseNode, false);
	}
	return false;
}

//-----------------------------------------------------------------------------
bool UIDescription::restoreViews (InputStream& stream, std::list<SharedPointer<CView> >& views, UIAttributes** customData)
{
	InputStreamContentProvider contentProvider (stream);
	if (auto baseNode = Detail::UIJsonDescReader::read (contentProvider))
	{
		Detail::UIDescList& children = baseNode->getChildren ();
		for (auto& childNode : children)
		{
			if (childNode->getName() == Detail::MainNodeNames::kCustom)
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
	const auto* templateName = node->getAttributes ()->getAttributeValue (Detail::MainNodeNames::kTemplate);
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
		const auto* subControllerName = node->getAttributes ()->getAttributeValue (UIViewCreator::kAttrSubController);
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
			result->setAttribute (kCViewControllerAttribute, subController);
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
			if (itNode->getName () == Detail::MainNodeNames::kTemplate)
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
			if (itNode->getName () == Detail::MainNodeNames::kTemplate)
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
Detail::UINode* UIDescription::getBaseNode (UTF8StringPtr name) const
{
	UTF8StringView nameView (name);
	nameView.calculateByteCount ();
	if (impl->sharedResources)
	{
		if (nameView == Detail::MainNodeNames::kBitmap || nameView == Detail::MainNodeNames::kFont || nameView == Detail::MainNodeNames::kColor || nameView == Detail::MainNodeNames::kGradient)
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
Detail::UINode* UIDescription::findChildNodeByNameAttribute (UINode* node, UTF8StringPtr nameAttribute) const
{
	if (node)
		return node->getChildren ().findChildNodeWithAttributeValue ("name", nameAttribute);
	return nullptr;
}

//-----------------------------------------------------------------------------
int32_t UIDescription::getTagForName (UTF8StringPtr name) const
{
	int32_t tag = -1;
	if (auto* controlTagNode = dynamic_cast<Detail::UIControlTagNode*> (findChildNodeByNameAttribute (getBaseNode (Detail::MainNodeNames::kControlTag), name)))
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
	auto* node = dynamic_cast<Detail::UIColorNode*> (findChildNodeByNameAttribute (getBaseNode (Detail::MainNodeNames::kColor), name));
	return node ? true : false;
}

//-----------------------------------------------------------------------------
bool UIDescription::hasTagName (UTF8StringPtr name) const
{
	auto* node = dynamic_cast<Detail::UIControlTagNode*> (findChildNodeByNameAttribute (getBaseNode (Detail::MainNodeNames::kControlTag), name));
	return node ? true : false;
}

//-----------------------------------------------------------------------------
bool UIDescription::hasFontName (UTF8StringPtr name) const
{
	auto* node = dynamic_cast<Detail::UIFontNode*> (findChildNodeByNameAttribute (getBaseNode (Detail::MainNodeNames::kFont), name));
	return node ? true : false;
}

//-----------------------------------------------------------------------------
bool UIDescription::hasBitmapName (UTF8StringPtr name) const
{
	auto* node = dynamic_cast<Detail::UIBitmapNode*> (findChildNodeByNameAttribute (getBaseNode (Detail::MainNodeNames::kBitmap), name));
	return node ? true : false;
}

//-----------------------------------------------------------------------------
bool UIDescription::hasGradientName (UTF8StringPtr name) const
{
	auto* node = dynamic_cast<Detail::UIGradientNode*> (findChildNodeByNameAttribute (getBaseNode (Detail::MainNodeNames::kGradient), name));
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
	auto* bitmapNode = dynamic_cast<Detail::UIBitmapNode*> (findChildNodeByNameAttribute (getBaseNode (Detail::MainNodeNames::kBitmap), name));
	if (bitmapNode)
	{
		CBitmap* bitmap = bitmapNode->getBitmap (impl->filePath);
		if (impl->bitmapCreator && bitmap && bitmap->getPlatformBitmap () == nullptr)
		{
			auto platformBitmap = impl->bitmapCreator->createBitmap (*bitmapNode->getAttributes ());
			if (platformBitmap)
			{
				double scaleFactor;
				if (Detail::decodeScaleFactorFromName (name, scaleFactor))
					platformBitmap->setScaleFactor (scaleFactor);
				bitmap->setPlatformBitmap (platformBitmap);
			}
		}
		if (impl->bitmapCreator2 && bitmap && bitmap->getPlatformBitmap () == nullptr)
		{
			if (auto b = impl->bitmapCreator2->createBitmap (*bitmapNode->getAttributes (), this))
			{
				bitmap->setPlatformBitmap (b->getPlatformBitmap ());
				auto it = b->begin ();
				++it;
				while (it != b->end ())
				{
					bitmap->addBitmap (*it);
					++it;
				}
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
						const std::string* propName = propertyNode->getAttributes ()->getAttributeValue ("name");
						if (propName == nullptr)
							continue;
						switch (filter->getProperty (propName->c_str ()).getType ())
						{
							case BitmapFilter::Property::kInteger:
							{
								int32_t intValue;
								if (propertyNode->getAttributes ()->getIntegerAttribute ("value", intValue))
									filter->setProperty (propName->c_str (), intValue);
								break;
							}
							case BitmapFilter::Property::kFloat:
							{
								double floatValue;
								if (propertyNode->getAttributes ()->getDoubleAttribute ("value", floatValue))
									filter->setProperty (propName->c_str (), floatValue);
								break;
							}
							case BitmapFilter::Property::kPoint:
							{
								CPoint pointValue;
								if (propertyNode->getAttributes ()->getPointAttribute ("value", pointValue))
									filter->setProperty (propName->c_str (), pointValue);
								break;
							}
							case BitmapFilter::Property::kRect:
							{
								CRect rectValue;
								if (propertyNode->getAttributes ()->getRectAttribute ("value", rectValue))
									filter->setProperty (propName->c_str (), rectValue);
								break;
							}
							case BitmapFilter::Property::kColor:
							{
								const std::string* colorString = propertyNode->getAttributes()->getAttributeValue ("value");
								if (colorString)
								{
									CColor color;
									if (getColor (colorString->c_str (), color))
										filter->setProperty(propName->c_str (), color);
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
					if (auto* outputBitmap = dynamic_cast<CBitmap*>(obj))
					{
						bitmap->setPlatformBitmap (outputBitmap->getPlatformBitmap ());
					}
				}
			}
			bitmapNode->setFilterProcessed ();
		}
		if (bitmap && bitmapNode->getScaledBitmapsAdded () == false)
		{
			double scaleFactor;
			auto decoded = Detail::decodeScaleFactorFromName (bitmap->getResourceDescription ().u.name, scaleFactor);
			if (!decoded || scaleFactor == 1.)
			{
				std::string bitmapName = name;
				if (decoded)
					bitmapName = Detail::removeScaleFactorFromName (bitmapName);
				// find scaled versions for this bitmap
				UINode* bitmapsNode = getBaseNode (Detail::MainNodeNames::kBitmap);
				for (auto& it : bitmapsNode->getChildren ())
				{
					auto* childNode = dynamic_cast<Detail::UIBitmapNode*>(it);
					if (childNode == nullptr || childNode == bitmapNode)
						continue;
					const std::string* childNodeBitmapName = childNode->getAttributes()->getAttributeValue ("name");
					if (childNodeBitmapName == nullptr)
						continue;
					std::string nameWithoutScaleFactor = Detail::removeScaleFactorFromName (*childNodeBitmapName);
					if (nameWithoutScaleFactor == bitmapName)
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
	auto* fontNode = dynamic_cast<Detail::UIFontNode*> (findChildNodeByNameAttribute (getBaseNode (Detail::MainNodeNames::kFont), name));
	if (fontNode)
		return fontNode->getFont ();
	return nullptr;
}

//-----------------------------------------------------------------------------
bool UIDescription::getColor (UTF8StringPtr name, CColor& color) const
{
	auto* colorNode = dynamic_cast<Detail::UIColorNode*> (findChildNodeByNameAttribute (getBaseNode (Detail::MainNodeNames::kColor), name));
	if (colorNode)
	{
		color = colorNode->getColor ();
		return true;
	}
	if (Detail::parseColor (name, color))
		return true;
	return false;
}

//-----------------------------------------------------------------------------
CGradient* UIDescription::getGradient (UTF8StringPtr name) const
{
	auto* gradientNode = dynamic_cast<Detail::UIGradientNode*> (findChildNodeByNameAttribute (getBaseNode(Detail::MainNodeNames::kGradient), name));
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
		auto& children = baseNode->getChildren ();
		for (const auto& itNode : children)
		{
			auto* node = dynamic_cast<NodeType*>(itNode);
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
	return lookupName<Detail::UIColorNode> (color, Detail::MainNodeNames::kColor, [] (const UIDescription* desc, Detail::UIColorNode* node, const CColor& color) {
		return node->getColor() == color;
	});
}

//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::lookupFontName (const CFontRef font) const
{
	return font ? lookupName<Detail::UIFontNode> (font, Detail::MainNodeNames::kFont, [] (const UIDescription* desc, Detail::UIFontNode* node, const CFontRef& font) {
		return node->getFont () && node->getFont () == font;
	}) : nullptr;
}

//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::lookupBitmapName (const CBitmap* bitmap) const
{
	return bitmap ? lookupName<Detail::UIBitmapNode> (bitmap, Detail::MainNodeNames::kBitmap, [] (const UIDescription* desc, Detail::UIBitmapNode* node, const CBitmap* bitmap) {
		return node->getBitmap (desc->impl->filePath) == bitmap;
	}) : nullptr;
}

//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::lookupGradientName (const CGradient* gradient) const
{
	return gradient ? lookupName<Detail::UIGradientNode> (gradient, Detail::MainNodeNames::kGradient, [] (const UIDescription* desc, Detail::UIGradientNode* node, const CGradient* gradient) {
		return node->getGradient() == gradient || (node->getGradient () && gradient->getColorStops () == node->getGradient ()->getColorStops ());
	}) : nullptr;
}
	
//-----------------------------------------------------------------------------
UTF8StringPtr UIDescription::lookupControlTagName (const int32_t tag) const
{
	return lookupName<Detail::UIControlTagNode> (tag, Detail::MainNodeNames::kControlTag, [] (const UIDescription* desc, Detail::UIControlTagNode* node, const int32_t tag) {
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
template<typename NodeType>
void UIDescription::changeNodeName (UTF8StringPtr oldName, UTF8StringPtr newName, IdStringPtr mainNodeName)
{
	UINode* mainNode = getBaseNode (mainNodeName);
	auto* node = dynamic_cast<NodeType*> (findChildNodeByNameAttribute(mainNode, oldName));
	if (node)
	{
		node->getAttributes ()->setAttribute ("name", newName);
		mainNode->childAttributeChanged (node, "name", oldName);
		mainNode->sortChildren ();
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeColorName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	changeNodeName<Detail::UIColorNode> (oldName, newName, Detail::MainNodeNames::kColor);
	impl->forEachListener ([this] (UIDescriptionListener* l) {
		l->onUIDescColorChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::changeTagName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	changeNodeName<Detail::UIControlTagNode> (oldName, newName, Detail::MainNodeNames::kControlTag);
	impl->forEachListener ([this] (UIDescriptionListener* l) {
		l->onUIDescTagChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::changeFontName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	changeNodeName<Detail::UIFontNode> (oldName, newName, Detail::MainNodeNames::kFont);
	impl->forEachListener ([this] (UIDescriptionListener* l) {
		l->onUIDescFontChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::changeBitmapName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	changeNodeName<Detail::UIBitmapNode> (oldName, newName, Detail::MainNodeNames::kBitmap);
	impl->forEachListener ([this] (UIDescriptionListener* l) {
		l->onUIDescBitmapChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::changeGradientName (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	changeNodeName<Detail::UIGradientNode> (oldName, newName, Detail::MainNodeNames::kGradient);
	impl->forEachListener ([this] (UIDescriptionListener* l) {
		l->onUIDescGradientChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::changeColor (UTF8StringPtr name, const CColor& newColor)
{
	UINode* colorsNode = getBaseNode (Detail::MainNodeNames::kColor);
	auto* node = dynamic_cast<Detail::UIColorNode*> (findChildNodeByNameAttribute (colorsNode, name));
	if (node)
	{
		if (!node->noExport ())
		{
			node->setColor (newColor);
			impl->forEachListener ([this] (UIDescriptionListener* l) {
				l->onUIDescColorChanged (this);
			});
		}
	}
	else
	{
		if (colorsNode)
		{
			auto attr = makeOwned<UIAttributes> ();
			attr->setAttribute ("name", name);
			std::string colorStr;
			UIViewCreator::colorToString (newColor, colorStr, nullptr);
			attr->setAttribute ("rgba", colorStr);
			auto* newNode = new Detail::UIColorNode ("color", attr);
			colorsNode->getChildren ().add (newNode);
			colorsNode->sortChildren ();
			impl->forEachListener ([this] (UIDescriptionListener* l) {
				l->onUIDescColorChanged (this);
			});
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeFont (UTF8StringPtr name, CFontRef newFont)
{
	UINode* fontsNode = getBaseNode (Detail::MainNodeNames::kFont);
	auto* node = dynamic_cast<Detail::UIFontNode*> (findChildNodeByNameAttribute (fontsNode, name));
	if (node)
	{
		if (!node->noExport ())
		{
			node->setFont (newFont);
			impl->forEachListener ([this] (UIDescriptionListener* l) {
				l->onUIDescFontChanged (this);
			});
		}
	}
	else
	{
		if (fontsNode)
		{
			auto attr = makeOwned<UIAttributes> ();
			attr->setAttribute ("name", name);
			auto* newNode = new Detail::UIFontNode ("font", attr);
			newNode->setFont (newFont);
			fontsNode->getChildren ().add (newNode);
			fontsNode->sortChildren ();
			impl->forEachListener ([this] (UIDescriptionListener* l) {
				l->onUIDescFontChanged (this);
			});
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeGradient (UTF8StringPtr name, CGradient* newGradient)
{
	UINode* gradientsNode = getBaseNode (Detail::MainNodeNames::kGradient);
	auto* node = dynamic_cast<Detail::UIGradientNode*> (findChildNodeByNameAttribute (gradientsNode, name));
	if (node)
	{
		if (!node->noExport ())
		{
			node->setGradient (newGradient);
			impl->forEachListener ([this] (UIDescriptionListener* l) {
				l->onUIDescGradientChanged (this);
			});
		}
	}
	else
	{
		if (gradientsNode)
		{
			auto attr = makeOwned<UIAttributes> ();
			attr->setAttribute ("name", name);
			auto* newNode = new Detail::UIGradientNode ("gradient", attr);
			newNode->setGradient (newGradient);
			gradientsNode->getChildren ().add (newNode);
			gradientsNode->sortChildren ();
			impl->forEachListener ([this] (UIDescriptionListener* l) {
				l->onUIDescGradientChanged (this);
			});
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeBitmap (UTF8StringPtr name, UTF8StringPtr newName, const CRect* nineparttiledOffset)
{
	UINode* bitmapsNode = getBaseNode (Detail::MainNodeNames::kBitmap);
	auto* node = dynamic_cast<Detail::UIBitmapNode*> (findChildNodeByNameAttribute (bitmapsNode, name));
	if (node)
	{
		if (!node->noExport ())
		{
			node->setBitmap (newName);
			node->setNinePartTiledOffset (nineparttiledOffset);
			impl->forEachListener ([this] (UIDescriptionListener* l) {
				l->onUIDescBitmapChanged (this);
			});
		}
	}
	else
	{
		if (bitmapsNode)
		{
			auto attr = makeOwned<UIAttributes> ();
			attr->setAttribute ("name", name);
			auto* newNode = new Detail::UIBitmapNode ("bitmap", attr);
			if (nineparttiledOffset)
				newNode->setNinePartTiledOffset (nineparttiledOffset);
			newNode->setBitmap (newName);
			bitmapsNode->getChildren ().add (newNode);
			bitmapsNode->sortChildren ();
			impl->forEachListener (
				[this] (UIDescriptionListener* l) { l->onUIDescBitmapChanged (this); });
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeMultiFrameBitmap (UTF8StringPtr name, UTF8StringPtr newName,
											const CMultiFrameBitmapDescription* desc)
{
	UINode* bitmapsNode = getBaseNode (Detail::MainNodeNames::kBitmap);
	auto* node =
		dynamic_cast<Detail::UIBitmapNode*> (findChildNodeByNameAttribute (bitmapsNode, name));
	if (node)
	{
		if (!node->noExport ())
		{
			node->setBitmap (newName);
			node->setMultiFrameDesc (desc);
			impl->forEachListener (
				[this] (UIDescriptionListener* l) { l->onUIDescBitmapChanged (this); });
		}
	}
	else
	{
		if (bitmapsNode)
		{
			auto attr = makeOwned<UIAttributes> ();
			attr->setAttribute ("name", name);
			auto* newNode = new Detail::UIBitmapNode ("bitmap", attr);
			if (desc)
				newNode->setMultiFrameDesc (desc);
			newNode->setBitmap (newName);
			bitmapsNode->getChildren ().add (newNode);
			bitmapsNode->sortChildren ();
			impl->forEachListener ([this] (UIDescriptionListener* l) {
				l->onUIDescBitmapChanged (this);
			});
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescription::changeBitmapFilters (UTF8StringPtr bitmapName, const std::list<SharedPointer<UIAttributes> >& filters)
{
	auto* bitmapNode = dynamic_cast<Detail::UIBitmapNode*> (findChildNodeByNameAttribute (getBaseNode (Detail::MainNodeNames::kBitmap), bitmapName));
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
		impl->forEachListener ([this] (UIDescriptionListener* l) {
			l->onUIDescBitmapChanged (this);
		});
	}
}

//-----------------------------------------------------------------------------
void UIDescription::collectBitmapFilters (UTF8StringPtr bitmapName, std::list<SharedPointer<UIAttributes> >& filters) const
{
	auto* bitmapNode = dynamic_cast<Detail::UIBitmapNode*> (findChildNodeByNameAttribute (getBaseNode (Detail::MainNodeNames::kBitmap), bitmapName));
	if (bitmapNode)
	{
		for (auto& childNode : bitmapNode->getChildren ())
		{
			if (childNode->getName () == "filter")
			{
				const std::string* filterName = childNode->getAttributes ()->getAttributeValue ("name");
				if (filterName == nullptr)
					continue;
				auto attributes = makeOwned<UIAttributes> ();
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
			}
		}
	}
}

//-----------------------------------------------------------------------------
static void removeChildNode (Detail::UINode* baseNode, UTF8StringPtr nodeName)
{
	auto& children = baseNode->getChildren ();
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
void UIDescription::removeNode (UTF8StringPtr name, IdStringPtr mainNodeName)
{
	UINode* node = getBaseNode (mainNodeName);
	if (node)
	{
		removeChildNode (node, name);
	}
}

//-----------------------------------------------------------------------------
void UIDescription::removeColor (UTF8StringPtr name)
{
	removeNode (name, Detail::MainNodeNames::kColor);
	impl->forEachListener ([this] (UIDescriptionListener* l) {
		l->onUIDescColorChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::removeTag (UTF8StringPtr name)
{
	removeNode (name, Detail::MainNodeNames::kControlTag);
	impl->forEachListener ([this] (UIDescriptionListener* l) {
		l->onUIDescTagChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::removeFont (UTF8StringPtr name)
{
	removeNode (name, Detail::MainNodeNames::kFont);
	impl->forEachListener ([this] (UIDescriptionListener* l) {
		l->onUIDescFontChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::removeBitmap (UTF8StringPtr name)
{
	removeNode (name, Detail::MainNodeNames::kBitmap);
	impl->forEachListener ([this] (UIDescriptionListener* l) {
		l->onUIDescBitmapChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::removeGradient (UTF8StringPtr name)
{
	removeNode (name, Detail::MainNodeNames::kGradient);
	impl->forEachListener ([this] (UIDescriptionListener* l) {
		l->onUIDescGradientChanged (this);
	});
}

//-----------------------------------------------------------------------------
void UIDescription::changeAlternativeFontNames (UTF8StringPtr name, UTF8StringPtr alternativeFonts)
{
	auto* node = dynamic_cast<Detail::UIFontNode*> (findChildNodeByNameAttribute (getBaseNode (Detail::MainNodeNames::kFont), name));
	if (node)
	{
		node->setAlternativeFontNames (alternativeFonts);
		impl->forEachListener ([this] (UIDescriptionListener* l) {
			l->onUIDescFontChanged (this);
		});
	}
}

//-----------------------------------------------------------------------------
bool UIDescription::getAlternativeFontNames (UTF8StringPtr name, std::string& alternativeFonts) const
{
	auto* node = dynamic_cast<Detail::UIFontNode*> (findChildNodeByNameAttribute (getBaseNode (Detail::MainNodeNames::kFont), name));
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
		if (itNode->getName () == Detail::MainNodeNames::kTemplate)
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
	if (UINode* node = getBaseNode (mainNodeName))
	{
		auto& children = node->getChildren ();
		for (const auto& itNode : children)
		{
			if (auto* nodeType = dynamic_cast<NodeType*>(itNode))
			{
				const std::string* name = nodeType->getAttributes ()->getAttributeValue ("name");
				if (name)
					names.emplace_back (name);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void UIDescription::collectColorNames (std::list<const std::string*>& names) const
{
	collectNamesFromNode<Detail::UIColorNode> (Detail::MainNodeNames::kColor, names);
}

//-----------------------------------------------------------------------------
void UIDescription::collectFontNames (std::list<const std::string*>& names) const
{
	collectNamesFromNode<Detail::UIFontNode> (Detail::MainNodeNames::kFont, names);
}

//-----------------------------------------------------------------------------
void UIDescription::collectBitmapNames (std::list<const std::string*>& names) const
{
	collectNamesFromNode<Detail::UIBitmapNode> (Detail::MainNodeNames::kBitmap, names);
}

//-----------------------------------------------------------------------------
void UIDescription::collectGradientNames (std::list<const std::string*>& names) const
{
	collectNamesFromNode<Detail::UIGradientNode> (Detail::MainNodeNames::kGradient, names);
}

//-----------------------------------------------------------------------------
void UIDescription::collectControlTagNames (std::list<const std::string*>& names) const
{
	collectNamesFromNode<Detail::UIControlTagNode> (Detail::MainNodeNames::kControlTag, names);
}

//-----------------------------------------------------------------------------
bool UIDescription::updateAttributesForView (UINode* node, CView* view, bool deep)
{
	bool result = false;
#if VSTGUI_LIVE_EDITING
	auto* factory = dynamic_cast<UIViewFactory*> (impl->viewFactory);
	std::list<std::string> attributeNames;
	CViewContainer* container = view->asViewContainer ();
	if (factory->getAttributeNamesForView (view, attributeNames))
	{
		for (auto& name : attributeNames)
		{
			if (impl->attributeSaveFilterFunc &&
				impl->attributeSaveFilterFunc (view, name) == false)
				continue;
			std::string value;
			if (factory->getAttributeValue (view, name, value, this))
				node->getAttributes ()->setAttribute (name, std::move (value));
		}
		node->getAttributes ()->setAttribute (UIViewCreator::kAttrClass, factory->getViewName (view));
		result = true;
	}
	if (deep && container && dynamic_cast<UIViewSwitchContainer*> (container) == nullptr)
	{
		ViewIterator it (container);
		while (*it)
		{
			CView* subView = *it;
			std::string subTemplateName;
			if (getTemplateNameFromView (subView, subTemplateName))
			{
				auto attr = makeOwned<UIAttributes> ();
				attr->setAttribute (Detail::MainNodeNames::kTemplate, subTemplateName);
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
	impl->forEachListener ([&] (UIDescriptionListener* l) {
		if (!l->doUIDescTemplateUpdate (this, name))
			doIt = false;
	});
	if (!doIt)
		return;

	auto* factory = dynamic_cast<UIViewFactory*> (impl->viewFactory);
	if (factory && impl->nodes)
	{
		UINode* node = nullptr;
		for (auto& childNode : impl->nodes->getChildren ())
		{
			if (childNode->getName () == Detail::MainNodeNames::kTemplate)
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
			node = new UINode (Detail::MainNodeNames::kTemplate);
		}
		node->getChildren ().removeAll ();
		updateAttributesForView (node, view);
	}
#endif
}

//-----------------------------------------------------------------------------
bool UIDescription::addNewTemplate (UTF8StringPtr name, const SharedPointer<UIAttributes>& attr)
{
#if VSTGUI_LIVE_EDITING
	vstgui_assert (impl->nodes);
	UINode* templateNode = findChildNodeByNameAttribute (impl->nodes, name);
	if (templateNode == nullptr)
	{
		auto* newNode = new UINode (Detail::MainNodeNames::kTemplate, attr);
		attr->setAttribute ("name", name);
		impl->nodes->getChildren ().add (newNode);
		impl->forEachListener ([this] (UIDescriptionListener* l) {
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
		impl->forEachListener ([this] (UIDescriptionListener* l) {
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
		impl->forEachListener ([this] (UIDescriptionListener* l) {
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
		auto* duplicate = new UINode (*templateNode);
		vstgui_assert (duplicate);
		if (duplicate)
		{
			duplicate->getAttributes()->setAttribute ("name", duplicateName);
			impl->nodes->getChildren ().add (duplicate);
			impl->forEachListener ([this] (UIDescriptionListener* l) {
				l->onUIDescTemplateChanged (this);
			});
			return true;
		}
	}
#endif
	return false;
}

//-----------------------------------------------------------------------------
bool UIDescription::setCustomAttributes (UTF8StringPtr name, const SharedPointer<UIAttributes>& attr)
{
	UINode* customNode = findChildNodeByNameAttribute (getBaseNode (Detail::MainNodeNames::kCustom), name);
	if (customNode)
		return false;
	UINode* parent = getBaseNode (Detail::MainNodeNames::kCustom);
	vstgui_assert (parent != nullptr);
	if (!parent)
		return false;
	attr->setAttribute ("name", name);
	customNode = new UINode ("attributes", attr);
	parent->getChildren ().add (customNode);
	return true;
}

//-----------------------------------------------------------------------------
SharedPointer<UIAttributes> UIDescription::getCustomAttributes (UTF8StringPtr name) const
{
	auto node = findChildNodeByNameAttribute (getBaseNode (Detail::MainNodeNames::kCustom), name);
	return node ? node->getAttributes () : nullptr;
}

//-----------------------------------------------------------------------------
SharedPointer<UIAttributes> UIDescription::getCustomAttributes (UTF8StringPtr name, bool create)
{
	auto attributes = getCustomAttributes (name);
	if (attributes)
		return attributes;
	if (create)
	{
		auto attrs = makeOwned<UIAttributes> ();
		if (setCustomAttributes (name, attrs))
			return attrs;
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
	auto* controlTagNode = dynamic_cast<Detail::UIControlTagNode*> (findChildNodeByNameAttribute (getBaseNode (Detail::MainNodeNames::kControlTag), tagName));
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
	UINode* tagsNode = getBaseNode (Detail::MainNodeNames::kControlTag);
	if (auto* controlTagNode =
			dynamic_cast<Detail::UIControlTagNode*> (findChildNodeByNameAttribute (tagsNode, tagName)))
	{
		if (create)
			return false;
		controlTagNode->setTagString (newTagString);
		impl->forEachListener ([this](UIDescriptionListener* l) { l->onUIDescTagChanged (this); });
		return true;
	}
	if (create)
	{
		if (tagsNode)
		{
			auto attr = makeOwned<UIAttributes> ();
			attr->setAttribute ("name", tagName);
			auto* node = new Detail::UIControlTagNode ("control-tag", attr);
			node->setTagString (newTagString);
			tagsNode->getChildren ().add (node);
			tagsNode->sortChildren ();
			impl->forEachListener ([this] (UIDescriptionListener* l) {
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
	auto* node = dynamic_cast<Detail::UIVariableNode*> (
	    findChildNodeByNameAttribute (impl->getVariableBaseNode (), name));
	if (node)
	{
		if (node->getType () == Detail::UIVariableNode::kNumber)
		{
			value = node->getNumber ();
			return true;
		}
		if (node->getType () == Detail::UIVariableNode::kString)
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
	auto* node = dynamic_cast<Detail::UIVariableNode*> (findChildNodeByNameAttribute (impl->getVariableBaseNode (), name));
	if (node)
	{
		value = node->getString ();
		return true;
	}
	return false;
}

namespace UIDescriptionPrivate {

using Locale = Detail::Locale;

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

} // UIDescriptionPrivate

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

} // VSTGUI
