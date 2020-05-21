// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../lib/cbitmap.h"
#include "../../lib/cfont.h"
#include "../../lib/cgradient.h"
#include "../../lib/platform/iplatformfont.h"
#include "../base64codec.h"
#include "../cstream.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "locale.h"
#include "parsecolor.h"
#include "scalefactorutils.h"
#include "uinode.h"
#include <list>
#include <string>

namespace VSTGUI {
namespace Detail {

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UINode::UINode (const std::string& _name, const SharedPointer<UIAttributes>& _attributes,
                bool needsFastChildNameAttributeLookup)
: name (_name), attributes (_attributes), flags (0)
{
	if (needsFastChildNameAttributeLookup)
		children = makeOwned<UIDescListWithFastFindAttributeNameChild> ();
	else
		children = makeOwned<UIDescList> ();
	if (attributes == nullptr)
		attributes = makeOwned<UIAttributes> ();
}

//-----------------------------------------------------------------------------
UINode::UINode (const std::string& _name, const SharedPointer<UIDescList>& _children,
                const SharedPointer<UIAttributes>& _attributes)
: name (_name), attributes (_attributes), children (_children), flags (0)
{
	vstgui_assert (children != nullptr);
	if (attributes == nullptr)
		attributes = makeOwned<UIAttributes> ();
}

//-----------------------------------------------------------------------------
UINode::UINode (const UINode& n)
: name (n.name)
, data (n.data)
, attributes (makeOwned<UIAttributes> (*n.attributes))
, children (makeOwned<UIDescList> (*n.children))
, flags (n.flags)
{
}

//-----------------------------------------------------------------------------
UINode::~UINode () noexcept
{
}

//-----------------------------------------------------------------------------
bool UINode::hasChildren () const
{
	return !children->empty ();
}

//-----------------------------------------------------------------------------
void UINode::childAttributeChanged (UINode* child, const char* attributeName,
                                    const char* oldAttributeValue)
{
	children->nodeAttributeChanged (child, attributeName, oldAttributeValue);
}

//-----------------------------------------------------------------------------
void UINode::sortChildren ()
{
	children->sort ();
}

//------------------------------------------------------------------------
void UINode::setData (DataStorage&& newData)
{
	data = std::move (newData);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UICommentNode::UICommentNode (const std::string& comment) : UINode ("comment")
{
	data = comment;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UIVariableNode::UIVariableNode (const std::string& name,
                                const SharedPointer<UIAttributes>& attributes)
: UINode (name, attributes), type (kUnknown), number (0)
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
		Detail::Locale localeResetter;

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
UIControlTagNode::UIControlTagNode (const std::string& name,
                                    const SharedPointer<UIAttributes>& attributes)
: UINode (name, attributes), tag (-1)
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
				tag = ((((int32_t)c1) << 24) | (((int32_t)c2) << 16) | (((int32_t)c3) << 8) |
				       (((int32_t)c4) << 0));
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
UIBitmapNode::UIBitmapNode (const std::string& name, const SharedPointer<UIAttributes>& attributes)
: UINode (name, attributes), bitmap (nullptr), filterProcessed (false), scaledBitmapsAdded (false)
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
bool UIBitmapNode::imagesEqual (IPlatformBitmap* b1, IPlatformBitmap* b2)
{
	if (b1 == b2)
		return true;
	if (b1->getSize () != b2->getSize () || b1->getScaleFactor () != b2->getScaleFactor ())
		return false;
	auto ac1 = b1->lockPixels (true);
	if (!ac1)
		return false;
	auto ac2 = b2->lockPixels (true);
	if (!ac2)
		return false;
	auto rowBytes = ac1->getBytesPerRow ();
	if (rowBytes != ac2->getBytesPerRow ())
		return false;
	if (ac1->getPixelFormat () != ac2->getPixelFormat ())
		return false;
	auto adr1 = ac1->getAddress ();
	if (!adr1)
		return false;
	auto adr2 = ac2->getAddress ();
	if (!adr2)
		return false;
	uint32_t rows = static_cast<uint32_t> (b1->getSize ().y);
	for (uint32_t y = 0; y < rows; ++y, adr1 += rowBytes, adr2 += rowBytes)
	{
		if (memcmp (adr1, adr2, rowBytes) != 0)
			return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool UIBitmapNode::hasXMLData () const
{
	return getChildren ().findChildNode ("data") != nullptr;
}

//-----------------------------------------------------------------------------
void UIBitmapNode::createXMLData (const std::string& pathHint)
{
	UINode* node = getChildren ().findChildNode ("data");
	if (node)
	{
		if (node->getData ().empty ())
		{
			getChildren ().remove (node);
			node = nullptr;
		}
		else if (auto bm = getBitmap (pathHint))
		{
			if (auto platformBitmap = bm->getPlatformBitmap ())
			{
				if (auto dataBitmap = createBitmapFromDataNode ())
				{
					if (!imagesEqual (platformBitmap, dataBitmap))
					{
						removeXMLData ();
						node = nullptr;
					}
				}
			}
		}
	}
	if (node == nullptr)
	{
		if (auto bm = getBitmap (pathHint))
		{
			if (auto platformBitmap = bm->getPlatformBitmap ())
			{
				auto buffer = IPlatformBitmap::createMemoryPNGRepresentation (platformBitmap);
				if (!buffer.empty ())
				{
					auto result = Base64Codec::encode (buffer.data (),
					                                   static_cast<uint32_t> (buffer.size ()));
					UINode* dataNode = new UINode ("data");
					dataNode->getAttributes ()->setAttribute ("encoding", "base64");
					dataNode->getData ().append (reinterpret_cast<const char*> (result.data.get ()),
					                             static_cast<std::streamsize> (result.dataSize));
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
CBitmap* UIBitmapNode::createBitmap (const std::string& str,
                                     CNinePartTiledDescription* partDesc) const
{
	if (partDesc)
		return new CNinePartTiledBitmap (CResourceDescription (str.c_str ()), *partDesc);
	return new CBitmap (CResourceDescription (str.c_str ()));
}

//------------------------------------------------------------------------
UINode* UIBitmapNode::dataNode () const
{
	UINode* node = getChildren ().findChildNode ("data");
	return (node && !node->getData ().empty ()) ? node : nullptr;
}

//------------------------------------------------------------------------
SharedPointer<IPlatformBitmap> UIBitmapNode::createBitmapFromDataNode () const
{
	if (auto node = dataNode ())
	{
		auto codecStr = node->getAttributes ()->getAttributeValue ("encoding");
		if (codecStr && *codecStr == "base64")
		{
			auto result = Base64Codec::decode (node->getData ());
			if (auto platformBitmap =
			        IPlatformBitmap::createFromMemory (result.data.get (), result.dataSize))
			{
				double scaleFactor = 1.;
				if (attributes->getDoubleAttribute ("scale-factor", scaleFactor))
					platformBitmap->setScaleFactor (scaleFactor);
				return platformBitmap;
			}
		}
	}
	return nullptr;
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
				partDesc = CNinePartTiledDescription (offsets.left, offsets.top, offsets.right,
				                                      offsets.bottom);
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
			if (auto platformBitmap = createBitmapFromDataNode ())
				bitmap->setPlatformBitmap (platformBitmap);
		}
		if (bitmap && path && bitmap->getPlatformBitmap () &&
		    bitmap->getPlatformBitmap ()->getScaleFactor () == 1.)
		{
			double scaleFactor = 1.;
			if (Detail::decodeScaleFactorFromName (*path, scaleFactor))
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
	std::string name (bitmapName);
	attributes->setAttribute ("path", name);
	if (bitmap)
		bitmap->forget ();
	bitmap = nullptr;
	double scaleFactor = 1.;
	if (Detail::decodeScaleFactorFromName (name, scaleFactor))
		attributes->setDoubleAttribute ("scale-factor", scaleFactor);
	removeXMLData ();
}

//-----------------------------------------------------------------------------
void UIBitmapNode::setNinePartTiledOffset (const CRect* offsets)
{
	if (bitmap)
	{
		auto* tiledBitmap = dynamic_cast<CNinePartTiledBitmap*> (bitmap);
		if (offsets && tiledBitmap)
		{
			tiledBitmap->setPartOffsets (CNinePartTiledDescription (
			    offsets->left, offsets->top, offsets->right, offsets->bottom));
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
UIFontNode::UIFontNode (const std::string& name, const SharedPointer<UIAttributes>& attributes)
: UINode (name, attributes), font (nullptr)
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
					if (std::find (fontNames.begin (), fontNames.end (), *nameAttr) ==
					    fontNames.end ())
					{
						std::vector<std::string> alternativeFontNames;
						attributes->getStringArrayAttribute ("alternative-font-names",
						                                     alternativeFontNames);
						for (auto& alternateFontName : alternativeFontNames)
						{
							auto trimmedString = trim (UTF8String (alternateFontName));
							if (std::find (fontNames.begin (), fontNames.end (),
							               trimmedString.getString ()) != fontNames.end ())
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
		attributes->setAttribute ("alternative-font-names", fontNames);
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
UIColorNode::UIColorNode (const std::string& name, const SharedPointer<UIAttributes>& attributes)
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
		parseColor (*rgb, color);
	if (rgba)
		parseColor (*rgba, color);
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
UIGradientNode::UIGradientNode (const std::string& name,
                                const SharedPointer<UIAttributes>& attributes)
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
				if (rgba == nullptr ||
				    colorNode->getAttributes ()->getDoubleAttribute ("start", start) == false)
					continue;
				if (parseColor (*rgba, color) == false)
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

} // Detail
} // VSTGUI
