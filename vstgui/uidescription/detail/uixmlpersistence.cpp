// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "uixmlpersistence.h"

#if VSTGUI_ENABLE_XML_PARSER

#include "../uiattributes.h"
#include "../cstream.h"
#include <map>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Detail {

//-----------------------------------------------------------------------------
SharedPointer<UINode> UIXMLParser::parse (IContentProvider* provider)
{
	Xml::Parser parser;
	if (parser.parse (provider, this))
		return std::move (nodes);
	return nullptr;
}

//-----------------------------------------------------------------------------
void UIXMLParser::startXmlElement (Xml::Parser* parser, IdStringPtr elementName, UTF8StringPtr* elementAttributes)
{
	std::string name (elementName);
	if (nodes)
	{
		UINode* parent = nodeStack.back ();
		UINode* newNode = nullptr;
		if (restoreViewsMode)
		{
			if (name != "view" && name != MainNodeNames::kCustom)
			{
				parser->stop ();
			}
			newNode = new UINode (name, makeOwned<UIAttributes> (elementAttributes));
		}
		else
		{
			if (parent == nodes)
			{
				// only allowed second level elements
				if (name == MainNodeNames::kControlTag || name == MainNodeNames::kColor || name == MainNodeNames::kBitmap)
					newNode = new UINode (name, makeOwned<UIAttributes> (elementAttributes), true);
				else if (name == MainNodeNames::kFont || name == MainNodeNames::kTemplate
					  || name == MainNodeNames::kControlTag || name == MainNodeNames::kCustom
					  || name == MainNodeNames::kVariable || name == MainNodeNames::kGradient)
					newNode = new UINode (name, makeOwned<UIAttributes> (elementAttributes));
				else
					parser->stop ();
			}
			else if (parent->getName () == MainNodeNames::kBitmap)
			{
				if (name == "bitmap")
					newNode = new UIBitmapNode (name, makeOwned<UIAttributes> (elementAttributes));
				else
					parser->stop ();
			}
			else if (parent->getName () == MainNodeNames::kFont)
			{
				if (name == "font")
					newNode = new UIFontNode (name, makeOwned<UIAttributes> (elementAttributes));
				else
					parser->stop ();
			}
			else if (parent->getName () == MainNodeNames::kColor)
			{
				if (name == "color")
					newNode = new UIColorNode (name, makeOwned<UIAttributes> (elementAttributes));
				else
					parser->stop ();
			}
			else if (parent->getName () == MainNodeNames::kControlTag)
			{
				if (name == "control-tag")
					newNode = new UIControlTagNode (name, makeOwned<UIAttributes> (elementAttributes));
				else
					parser->stop ();
			}
			else if (parent->getName () == MainNodeNames::kVariable)
			{
				if (name == "var")
					newNode = new UIVariableNode (name, makeOwned<UIAttributes> (elementAttributes));
				else
					parser->stop ();
			}
			else if (parent->getName () == MainNodeNames::kGradient)
			{
				if (name == "gradient")
					newNode = new UIGradientNode (name, makeOwned<UIAttributes> (elementAttributes));
				else
					parser->stop ();
			}
			else
				newNode = new UINode (name, makeOwned<UIAttributes> (elementAttributes));
		}
		if (newNode)
		{
			parent->getChildren ().add (newNode);
			nodeStack.emplace_back (newNode);
		}
	}
	else if (name == "vstgui-ui-description")
	{
		nodes = makeOwned<UINode> (name, makeOwned<UIAttributes> (elementAttributes));
		nodeStack.emplace_back (nodes);
	}
	else if (name == "vstgui-ui-description-view-list")
	{
		vstgui_assert (nodes == nullptr);
		nodes = makeOwned<UINode> (name, makeOwned<UIAttributes> (elementAttributes));
		nodeStack.emplace_back (nodes);
		restoreViewsMode = true;
	}
}

//-----------------------------------------------------------------------------
void UIXMLParser::endXmlElement (Xml::Parser* parser, IdStringPtr name)
{
	if (nodeStack.back () == nodes)
		restoreViewsMode = false;
	nodeStack.pop_back ();
}

//-----------------------------------------------------------------------------
void UIXMLParser::xmlCharData (Xml::Parser* parser, const int8_t* data, int32_t length)
{
	if (nodeStack.empty ())
		return;
	auto& nodeData = nodeStack.back ()->getData ();
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
void UIXMLParser::xmlComment (Xml::Parser* parser, IdStringPtr comment)
{
#if VSTGUI_LIVE_EDITING
	if (nodeStack.empty ())
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
		if (!commentStr.empty ())
		{
			UICommentNode* commentNode = new UICommentNode (comment);
			parent->getChildren ().add (commentNode);
		}
	}
#endif
}

//-----------------------------------------------------------------------------
bool UIXMLDescWriter::write (OutputStream& stream, UINode* rootNode)
{
	intendLevel = 0;
	stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	return writeNode (rootNode, stream);
}

//-----------------------------------------------------------------------------
void UIXMLDescWriter::encodeAttributeString (std::string& str)
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
bool UIXMLDescWriter::writeAttributes (UIAttributes* attr, OutputStream& stream)
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
bool UIXMLDescWriter::writeNodeData (UINode::DataStorage& str, OutputStream& stream)
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
			for (int32_t i2 = 0; i2 < intendLevel; i2++) stream << "\t";
		}
	}
	stream << "\n";
	return true;
}

//-----------------------------------------------------------------------------
bool UIXMLDescWriter::writeComment (UICommentNode* node, OutputStream& stream)
{
	stream << "<!--";
	stream << node->getData ();
	stream << "-->\n";
	return true;
}

//-----------------------------------------------------------------------------
bool UIXMLDescWriter::writeNode (UINode* node, OutputStream& stream)
{
	if (!node)
		return false;

	bool result = true;
	if (node->noExport ())
		return result;
	for (int32_t i = 0; i < intendLevel; i++) stream << "\t";
	if (auto* commentNode = dynamic_cast<UICommentNode*> (node))
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

//------------------------------------------------------------------------
} // Detail
} // VSTGUI

#endif // VSTGUI_ENABLE_XML_PARSER
