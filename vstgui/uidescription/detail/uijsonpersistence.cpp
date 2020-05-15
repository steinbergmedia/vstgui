// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../uiattributes.h"
#include "uijsonpersistence.h"
#include <map>
#include <string_view>

#define RAPIDJSON_HAS_STDSTRING 1
#include "../rapidjson/include/rapidjson/prettywriter.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Detail {

static constexpr auto attributeNameStr = "name";
static constexpr auto attributeTagStr = "tag";
static constexpr auto attributeRGBAStr = "rgba";
static constexpr auto keyDataStr = "data";
static constexpr auto keyChildrenStr = "children";
static constexpr auto keyTemplatesStr = "templates";

//------------------------------------------------------------------------
namespace UIJsonDescReader {

//------------------------------------------------------------------------
SharedPointer<UINode> read (InputStream& stream)
{
	return nullptr;
}

//------------------------------------------------------------------------
} // UIJsonDescReader

//------------------------------------------------------------------------
namespace UIJsonDescWriter {

//------------------------------------------------------------------------
template <typename CharT>
struct OutputStreamWrapper
{
	using Ch = CharT;

	OutputStreamWrapper (OutputStream& stream) : stream (stream) {}

	void Put (CharT c) { stream << c; }
	void Flush () {}

	OutputStream& stream;
};

using DefaultOutputStreamWrapper = OutputStreamWrapper<uint8_t>;
using JSONWriter = rapidjson::PrettyWriter<DefaultOutputStreamWrapper>;

//------------------------------------------------------------------------
static const std::string* getNodeAttributeName (const UINode* node)
{
	if (auto attributes = node->getAttributes ())
		return attributes->getAttributeValue (attributeNameStr);
	return nullptr;
}

//------------------------------------------------------------------------
static void writeAttributes (const UIAttributes& attributes, JSONWriter& writer,
                             bool ignoreNameAttribute = false)
{
	std::map<std::string_view, std::string_view> ordered (attributes.begin (), attributes.end ());
	for (const auto& attr : ordered)
	{
		if (ignoreNameAttribute && attr.first == attributeNameStr)
			continue;
		writer.Key (attr.first.data (), attr.first.size ());
		writer.String (attr.second.data (), attr.second.size ());
	}
}

//------------------------------------------------------------------------
static void writeNode (const UINode* node, JSONWriter& writer)
{
	auto name = getNodeAttributeName (node);
	if (name)
		writer.Key (*name);
	writer.StartObject ();
	writeAttributes (*node->getAttributes (), writer, name != nullptr);
	for (const auto& child : node->getChildren ())
	{
		writer.Key (child->getName ());
		writer.StartObject ();
		writeAttributes (*child->getAttributes (), writer);
		if (child->getData ().empty () == false)
		{
			writer.Key (keyDataStr);
			writer.String (child->getData ());
		}
		vstgui_assert (child->getChildren ().empty ());
		writer.EndObject ();
	}
	writer.EndObject ();
}

//------------------------------------------------------------------------
static void writeGradientNode (const UINode* node, JSONWriter& writer)
{
	auto name = getNodeAttributeName (node);
	vstgui_assert (name);
	writer.Key (*name);
	writer.StartArray ();
	for (const auto& child : node->getChildren ())
	{
		writer.StartObject ();
		writeAttributes (*child->getAttributes (), writer);
		vstgui_assert (child->getChildren ().empty ());
		writer.EndObject ();
	}
	writer.EndArray ();
}

//------------------------------------------------------------------------
static void writeSingleAttributeNode (const char* attrName, const UINode* node, JSONWriter& writer)
{
	auto name = getNodeAttributeName (node);
	vstgui_assert (name);
	writer.Key (*name);
	vstgui_assert (node->getAttributes ());
	auto tag = node->getAttributes ()->getAttributeValue (attrName);
	vstgui_assert (tag);
	writer.String (*tag);
}

//------------------------------------------------------------------------
template <typename Proc>
static void writeResourceNode (const char* name, const UINode* resNode, Proc proc,
                               JSONWriter& writer)
{
	if (resNode->getChildren ().empty ())
		return;
	writer.Key (name);
	writer.StartObject ();
	for (auto& child : resNode->getChildren ())
	{
		if (child->noExport () == false)
			proc (child, writer);
	}
	writer.EndObject ();
}

//------------------------------------------------------------------------
static void writeTemplateNode (const UINode* node, JSONWriter& writer)
{
	auto name = getNodeAttributeName (node);
	if (name)
		writer.Key (*name);
	writer.StartObject ();
	writeAttributes (*node->getAttributes (), writer, name != nullptr);
	if (node->getChildren ().empty () == false)
	{
		writer.Key (keyChildrenStr);
		writer.StartArray ();
		for (const auto& child : node->getChildren ())
			writeTemplateNode (child, writer);
		writer.EndArray ();
	}
	writer.EndObject ();
}

//------------------------------------------------------------------------
static void writeTemplates (const std::vector<const UINode*>& templates, JSONWriter& writer)
{
	writer.Key (keyTemplatesStr);
	writer.StartObject ();
	for (auto& child : templates)
	{
		writeTemplateNode (child, writer);
	}
	writer.EndObject ();
}

//------------------------------------------------------------------------
static bool writeRootNode (UINode* rootNode, JSONWriter& writer)
{
	writer.StartObject ();
	writer.Key (rootNode->getName ());
	writer.StartObject ();
	writeAttributes (*rootNode->getAttributes (), writer);
	bool result = true;
	std::vector<const UINode*> templateNodes;
	const UINode* bitmapsNode = nullptr;
	const UINode* fontsNode = nullptr;
	const UINode* colorsNode = nullptr;
	const UINode* controlTagsNode = nullptr;
	const UINode* variablesNode = nullptr;
	const UINode* gradientsNode = nullptr;
	const UINode* customNode = nullptr;
	for (const auto& child : rootNode->getChildren ())
	{
		if (child->getName () == MainNodeNames::kTemplate)
			templateNodes.push_back (child);
		else if (child->getName () == MainNodeNames::kBitmap)
			bitmapsNode = child;
		else if (child->getName () == MainNodeNames::kFont)
			fontsNode = child;
		else if (child->getName () == MainNodeNames::kColor)
			colorsNode = child;
		else if (child->getName () == MainNodeNames::kControlTag)
			controlTagsNode = child;
		else if (child->getName () == MainNodeNames::kVariable)
			variablesNode = child;
		else if (child->getName () == MainNodeNames::kGradient)
			gradientsNode = child;
		else if (child->getName () == MainNodeNames::kCustom)
			customNode = child;
		else
			return false; // unexpected input
	}
	if (variablesNode)
	{
		writeResourceNode (MainNodeNames::kVariable, variablesNode, writeNode, writer);
	}
	if (bitmapsNode)
	{
		writeResourceNode (MainNodeNames::kBitmap, bitmapsNode, writeNode, writer);
	}
	if (fontsNode)
	{
		writeResourceNode (MainNodeNames::kFont, fontsNode, writeNode, writer);
	}
	if (colorsNode)
	{
		writeResourceNode (MainNodeNames::kColor, colorsNode,
		                   [] (UINode* node, JSONWriter& writer) {
			                   writeSingleAttributeNode (attributeRGBAStr, node, writer);
		                   },
		                   writer);
	}
	if (gradientsNode)
	{
		writeResourceNode (MainNodeNames::kGradient, gradientsNode, writeGradientNode, writer);
	}
	if (controlTagsNode)
	{
		writeResourceNode (MainNodeNames::kControlTag, controlTagsNode,
		                   [] (UINode* node, JSONWriter& writer) {
			                   writeSingleAttributeNode (attributeTagStr, node, writer);
		                   },
		                   writer);
	}
	if (customNode)
	{
		writeResourceNode (MainNodeNames::kCustom, customNode, writeNode, writer);
	}
	writeTemplates (templateNodes, writer);
	writer.EndObject ();
	writer.EndObject ();
	return result;
}

//------------------------------------------------------------------------
bool write (OutputStream& stream, UINode* rootNode)
{
	DefaultOutputStreamWrapper output (stream);
	JSONWriter writer (output);
	writer.SetIndent ('\t', 1);
	auto result = writeRootNode (rootNode, writer);
	return result;
}

//------------------------------------------------------------------------
} // UIJsonDescWriter

//------------------------------------------------------------------------
} // Detail
} // VSTGUI
