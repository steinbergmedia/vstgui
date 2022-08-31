// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../uiattributes.h"
#include "uijsonpersistence.h"
#include <array>
#include <deque>
#include <map>

#if __cplusplus > 201402L
#include <string_view>
#endif

#define RAPIDJSON_HAS_STDSTRING 1
#if DEBUG
#include "../rapidjson/include/rapidjson/error/en.h"
#endif
#include "../rapidjson/include/rapidjson/document.h"
#include "../rapidjson/include/rapidjson/prettywriter.h"
#include "../rapidjson/include/rapidjson/reader.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Detail {

static constexpr auto attributeNameStr = "name";
static constexpr auto attributeValueStr = "value";
static constexpr auto attributeClassStr = "class";
static constexpr auto attributeTagStr = "tag";
static constexpr auto attributeRGBAStr = "rgba";
static constexpr auto keyDataStr = "data";
static constexpr auto keyChildrenStr = "children";
static constexpr auto keyTemplatesStr = "templates";
static constexpr auto keyViewsStr = "views";
static constexpr auto attributesStr = "attributes";
static constexpr auto colorStr = "color";
static constexpr auto controlTagStr = "control-tag";
static constexpr auto bitmapStr = "bitmap";
static constexpr auto fontStr = "font";
static constexpr auto templateStr = "template";
static constexpr auto colorStopStr = "color-stop";
static constexpr auto viewStr = "view";
static constexpr auto gradientStr = "gradient";

//------------------------------------------------------------------------
namespace UIJsonDescReader {

//------------------------------------------------------------------------
template <size_t size>
struct ContentProviderWrapper
{
	using Ch = uint8_t;

	ContentProviderWrapper (IContentProvider& s) : stream (s)
	{
		bufferLeft = bufferSize = stream.readRawData (reinterpret_cast<int8_t*> (buffer.data ()),
													  static_cast<int32_t> (buffer.size ()));
		if (bufferLeft == kStreamIOError)
			bufferSize = bufferLeft = 0;
		if (bufferSize == 0)
		{
			current = 0;
			bufferLeft = 1;
		}
		else
			current = buffer[bufferSize - bufferLeft];
	}

	Ch Peek () const { return current; }
	Ch Take ()
	{
		auto result = Peek ();
		++pos;
		if (bufferLeft == 1)
		{
			bufferLeft = bufferSize = stream.readRawData (
				reinterpret_cast<int8_t*> (buffer.data ()), static_cast<int32_t> (buffer.size ()));
			if (bufferLeft == kStreamIOError)
				bufferSize = bufferLeft = 0;
			if (bufferSize == 0)
			{
				current = 0;
				return result;
			}
		}
		else
			--bufferLeft;
		current = buffer[bufferSize - bufferLeft];
		return result;
	}
	size_t Tell () { return pos; }

	Ch* PutBegin ()
	{
		vstgui_assert (false);
		return 0;
	}
	void Put (Ch c) { vstgui_assert (false); }
	void Flush () { vstgui_assert (false); };
	size_t PutEnd (Ch* begin)
	{
		vstgui_assert (false);
		return 0;
	}

	Ch current {};
	size_t pos {0};
	IContentProvider& stream;

	std::array<Ch, size> buffer;
	size_t bufferLeft {};
	size_t bufferSize {};
};

//------------------------------------------------------------------------
struct Handler
{
	using Ch = char;
	using SizeType = rapidjson::SizeType;

	enum class State
	{
		Uninitialized = 0,
		Initialized,
		InRootNode,
		InBitmapRootNode,
		InFontRootNode,
		InColorRootNode,
		InGradientRootNode,
		InControlTagRootNode,
		InCustomRootNode,
		InVariableRootNode,
		InTemplateRootNode,
		BitmapNode,
		FontNode,
		GradientNode,
		TemplateNode,
		ChildrenNode,
		ViewNode,
		DataNode,
		ViewAttributes,
	};

	bool Null () { return false; }
	bool Bool (bool b) { return false; }
	bool Int (int i) { return false; }
	bool Uint (unsigned i) { return false; }
	bool Int64 (int64_t i) { return false; }
	bool Uint64 (uint64_t i) { return false; }
	bool Double (double d) { return false; }
	bool RawNumber (const Ch* str, SizeType length, bool copy) { return false; }
	bool String (const Ch* str, SizeType length, bool copy)
	{
		if (state == State::InColorRootNode)
		{
			auto attrs = newAttributesWithNameAttr (keyStr);
			attrs->setAttribute (attributeRGBAStr, {str, length});
			nodeStack.back ()->getChildren ().add (new UIColorNode (colorStr, attrs));
		}
		else if (state == State::InControlTagRootNode)
		{
			auto attrs = newAttributesWithNameAttr (keyStr);
			attrs->setAttribute (attributeTagStr, {str, length});
			nodeStack.back ()->getChildren ().add (new UIControlTagNode (controlTagStr, attrs));
		}
		else if (state == State::InVariableRootNode)
		{
			auto attrs = newAttributesWithNameAttr (keyStr);
			attrs->setAttribute (attributeValueStr, {str, length});
			nodeStack.back ()->getChildren ().add (new UIVariableNode (controlTagStr, attrs));
		}
		else if (state == State::DataNode && keyStr == "data")
		{
			nodeStack.back ()->setData ({str, length});
		}
		else
		{
			nodeStack.back ()->getAttributes ()->setAttribute (keyStr, {str, length});
		}
		keyStr.clear ();
		return true;
	}

	bool Key (const Ch* str, SizeType length, bool copy)
	{
		keyStr = {str, length};
		return true;
	}

	bool StartObject ()
	{
		UINode* newNode = nullptr;
		State newState {};
		switch (state)
		{
			case State::Uninitialized:
			{
				newState = State::Initialized;
				break;
			}
			case State::Initialized:
			{
				vstgui_assert (keyStr == "vstgui-ui-description" ||
				               keyStr == "vstgui-ui-description-view-list");
				rootNode = makeOwned<UINode> (std::move (keyStr));
				newNode = rootNode;
				newState = State::InRootNode;
				break;
			}
			case State::InRootNode:
			{
				if (keyStr == keyTemplatesStr || keyStr == keyViewsStr)
				{
					newState = State::InTemplateRootNode;
					break;
				}
				auto needsFastChildNameAttributeLookup = false;
				if (keyStr == MainNodeNames::kBitmap)
				{
					newState = State::InBitmapRootNode;
					needsFastChildNameAttributeLookup = true;
				}
				else if (keyStr == MainNodeNames::kFont)
					newState = State::InFontRootNode;
				else if (keyStr == MainNodeNames::kColor)
				{
					newState = State::InColorRootNode;
					needsFastChildNameAttributeLookup = true;
				}
				else if (keyStr == MainNodeNames::kGradient)
					newState = State::InGradientRootNode;
				else if (keyStr == MainNodeNames::kControlTag)
				{
					newState = State::InControlTagRootNode;
					needsFastChildNameAttributeLookup = true;
				}
				else if (keyStr == MainNodeNames::kCustom)
					newState = State::InCustomRootNode;
				else if (keyStr == MainNodeNames::kVariable)
					newState = State::InVariableRootNode;
				else
					return false;
				newNode = new UINode (keyStr, nullptr, needsFastChildNameAttributeLookup);
				break;
			}
			case State::InBitmapRootNode:
			{
				newNode = new UIBitmapNode (bitmapStr, newAttributesWithNameAttr (keyStr));
				newState = State::BitmapNode;
				break;
			}
			case State::InFontRootNode:
			{
				newNode = new UIFontNode (fontStr, newAttributesWithNameAttr (keyStr));
				newState = State::FontNode;
				break;
			}
			case State::InCustomRootNode:
			{
				newNode = new UINode (attributesStr, newAttributesWithNameAttr (keyStr));
				newState = State::DataNode;
				break;
			}
			case State::InTemplateRootNode:
			{
				newNode = new UINode (templateStr, newAttributesWithNameAttr (keyStr));
				newState = State::TemplateNode;
				break;
			}
			case State::BitmapNode:
			{
				vstgui_assert (keyStr == keyDataStr);
				newNode = new UINode (keyStr);
				newState = State::DataNode;
				break;
			}
			case State::GradientNode:
			{
				vstgui_assert (keyStr.empty ());
				newNode = new UINode (colorStopStr);
				newState = State::DataNode;
				break;
			}
			case State::TemplateNode:
			{
				if (keyStr == attributesStr)
					newState = State::ViewAttributes;
				else if (keyStr == keyChildrenStr)
					newState = State::ChildrenNode;
				break;
			}
			case State::ChildrenNode:
			{
				auto attr = makeOwned<UIAttributes> (15);
				newNode = new UINode (viewStr, attr);
				newState = State::ViewNode;
				break;
			}
			case State::ViewNode:
			{
				newState = State::ChildrenNode;
				break;
			}
			case State::InColorRootNode:
			case State::InControlTagRootNode:
			case State::InVariableRootNode:
			case State::InGradientRootNode:
			case State::FontNode:
			case State::DataNode:
			case State::ViewAttributes:
			{
				// not allowed here, invalid JSON data!
				return false;
			}
		}
		keyStr.clear ();
		pushNode (newNode);
		pushState (newState);
		return true;
	}

	bool EndObject (SizeType memberCount)
	{
		if (state == State::InTemplateRootNode || state == State::ChildrenNode ||
		    state == State::ViewAttributes)
		{
			popState ();
			return true;
		}
		popState ();
		return popNode ();
	}

	bool StartArray ()
	{
		if (state == State::InGradientRootNode)
		{
			auto newNode = new UIGradientNode (gradientStr, newAttributesWithNameAttr (keyStr));
			pushNode (newNode);
			pushState (State::GradientNode);
			keyStr.clear ();
			return true;
		}
		return false;
	}

	bool EndArray (SizeType elementCount)
	{
		if (state != State::GradientNode)
			return false;
		popState ();
		if (!popNode ())
			return false;
		return true;
	}

	bool popNode ()
	{
		if (nodeStack.empty ())
			return state == State::Uninitialized;
		nodeStack.pop_back ();
		return true;
	}

	void pushNode (UINode* newNode)
	{
		if (newNode)
		{
			if (newNode != rootNode)
				nodeStack.back ()->getChildren ().add (newNode);
			nodeStack.emplace_back (newNode);
		}
	}

	void popState ()
	{
		stateStack.pop_back ();
		state = stateStack.back ();
	}

	void pushState (State newState)
	{
		stateStack.emplace_back (newState);
		state = newState;
	}

	static SharedPointer<UIAttributes> newAttributesWithNameAttr (const std::string& name)
	{
		auto attributes = makeOwned<UIAttributes> ();
		attributes->setAttribute (attributeNameStr, name);
		return attributes;
	}

	SharedPointer<UINode> rootNode;
	std::deque<UINode*> nodeStack;
	std::deque<State> stateStack {State::Uninitialized};
	State state {};
	std::string keyStr;
};

//------------------------------------------------------------------------
SharedPointer<UINode> read (IContentProvider& stream)
{
	ContentProviderWrapper<1024> streamWrapper (stream);
	Handler handler;
	rapidjson::Reader reader;

	auto result = reader.Parse<rapidjson::kParseStopWhenDoneFlag> (streamWrapper, handler);
	if (result.IsError ())
	{
#if DEBUG
		DebugPrint ("JSON Parsing Error:");
		if (auto errorString = rapidjson::GetParseError_En (result.Code ()))
			DebugPrint (" %s", errorString);
		else
			DebugPrint (" %d", result.Code ());
		DebugPrint ("\n\tAt byte offset: %d\n", result.Offset ());
#endif
		return nullptr;
	}
	return handler.rootNode;
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

//------------------------------------------------------------------------
static const std::string* getNodeAttributeName (const UINode* node)
{
	if (auto attributes = node->getAttributes ())
		return attributes->getAttributeValue (attributeNameStr);
	return nullptr;
}

//------------------------------------------------------------------------
static const std::string* getNodeAttributeViewClass (const UINode* node)
{
	if (auto attributes = node->getAttributes ())
		return attributes->getAttributeValue (attributeClassStr);
	return nullptr;
}

//------------------------------------------------------------------------
template <typename JSONWriter>
void writeAttributes (const UIAttributes& attributes, JSONWriter& writer,
                      bool ignoreNameAttribute = false)
{
#if __cplusplus > 201402L
	std::map<std::string_view, std::string_view> ordered (attributes.begin (), attributes.end ());
#else
	std::map<std::string, std::string> ordered (attributes.begin (), attributes.end ());
#endif
	for (const auto& attr : ordered)
	{
		if (ignoreNameAttribute && attr.first == attributeNameStr)
			continue;
		if (attr.second.empty ()) // don't write empty attributes
			continue;
		writer.Key (attr.first.data (), static_cast<rapidjson::SizeType> (attr.first.size ()));
		writer.String (attr.second.data (), static_cast<rapidjson::SizeType> (attr.second.size ()));
	}
}

//------------------------------------------------------------------------
template <typename JSONWriter>
void writeNode (const UINode* node, JSONWriter& writer)
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
template <typename JSONWriter>
void writeGradientNode (const UINode* node, JSONWriter& writer)
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
template <typename JSONWriter>
void writeSingleAttributeNode (const char* attrName, const UINode* node, JSONWriter& writer)
{
	auto name = getNodeAttributeName (node);
	vstgui_assert (name);
	writer.Key (*name);
	vstgui_assert (node->getAttributes ());
	if (auto tag = node->getAttributes ()->getAttributeValue (attrName))
		writer.String (*tag);
	else
		writer.String ("");
}

//------------------------------------------------------------------------
template <typename JSONWriter>
void writeColorAttributeNode (const UINode* node, JSONWriter& writer)
{
	auto name = getNodeAttributeName (node);
	vstgui_assert (name);
	writer.Key (*name);
	vstgui_assert (node->getAttributes ());
	if (auto color = node->getAttributes ()->getAttributeValue (attributeRGBAStr))
	{
		writer.String (*color);
	}
	else
	{
		auto colorNode = dynamic_cast<const UIColorNode*> (node);
		vstgui_assert (colorNode);
		writer.String (colorNode->getColor ().toString ().getString ());
	}
}

//------------------------------------------------------------------------
template <typename JSONWriter, typename Proc>
void writeResourceNode (const char* name, const UINode* resNode, Proc proc, JSONWriter& writer)
{
	writer.Key (name);
	writer.StartObject ();
	if (resNode->getAttributes () && resNode->getAttributes ()->empty () == false)
		writeAttributes (*resNode->getAttributes (), writer);
	for (auto& child : resNode->getChildren ())
	{
		if (child->noExport () == false)
			proc (child, writer);
	}
	writer.EndObject ();
}

//------------------------------------------------------------------------
template <typename JSONWriter>
void writeTemplateNode (const std::string* name, const UINode* node, JSONWriter& writer)
{
	if (name)
		writer.Key (*name);
	writer.StartObject ();
	writer.String (attributesStr);
	writer.StartObject ();
	writeAttributes (*node->getAttributes (), writer, name != nullptr);
	writer.EndObject ();
	if (node->getChildren ().empty () == false)
	{
		writer.Key (keyChildrenStr);
		writer.StartObject ();
		for (const auto& child : node->getChildren ())
		{
			writeTemplateNode (getNodeAttributeViewClass (child), child, writer);
		}
		writer.EndObject ();
	}
	writer.EndObject ();
}

//------------------------------------------------------------------------
template <typename JSONWriter>
void writeViewNodes (const std::vector<const UINode*>& views, JSONWriter& writer)
{
	if (views.empty ())
		return;
	writer.Key (keyViewsStr);
	writer.StartObject ();
	for (auto& child : views)
	{
		writeTemplateNode (getNodeAttributeViewClass (child), child, writer);
	}
	writer.EndObject ();
}

//------------------------------------------------------------------------
template <typename JSONWriter>
void writeTemplates (const std::vector<const UINode*>& templates, JSONWriter& writer)
{
	if (templates.empty ())
		return;
	writer.Key (keyTemplatesStr);
	writer.StartObject ();
	for (auto& child : templates)
	{
		writeTemplateNode (getNodeAttributeName (child), child, writer);
	}
	writer.EndObject ();
}

//------------------------------------------------------------------------
template <typename JSONWriter>
bool writeRootNode (UINode* rootNode, JSONWriter& writer)
{
	writer.StartObject ();
	writer.Key (rootNode->getName ());
	writer.StartObject ();
	writeAttributes (*rootNode->getAttributes (), writer);
	bool result = true;
	std::vector<const UINode*> templateNodes;
	std::vector<const UINode*> viewNodes;
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
			templateNodes.emplace_back (child);
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
		else if (child->getName () == viewStr)
			viewNodes.emplace_back (child);
		else if (child->getName () == "comment")
		{
			// comments are removed
			continue;
		}
		else
			return false; // unexpected input
	}
	if (variablesNode)
	{
		writeResourceNode (MainNodeNames::kVariable, variablesNode,
		                   [] (UINode* node, JSONWriter& writer) {
			                   writeSingleAttributeNode (attributeValueStr, node, writer);
		                   },
		                   writer);
	}
	if (bitmapsNode)
	{
		writeResourceNode (MainNodeNames::kBitmap, bitmapsNode, writeNode<JSONWriter>, writer);
	}
	if (fontsNode)
	{
		writeResourceNode (MainNodeNames::kFont, fontsNode, writeNode<JSONWriter>, writer);
	}
	if (colorsNode)
	{
		writeResourceNode (MainNodeNames::kColor, colorsNode, writeColorAttributeNode<JSONWriter>,
		                   writer);
	}
	if (gradientsNode)
	{
		writeResourceNode (MainNodeNames::kGradient, gradientsNode, writeGradientNode<JSONWriter>,
		                   writer);
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
		writeResourceNode (MainNodeNames::kCustom, customNode, writeNode<JSONWriter>, writer);
	}
	writeViewNodes (viewNodes, writer);
	writeTemplates (templateNodes, writer);
	writer.EndObject ();
	writer.EndObject ();
	return result;
}

//------------------------------------------------------------------------
bool write (OutputStream& stream, UINode* rootNode, bool pretty)
{
	DefaultOutputStreamWrapper output (stream);

	if (pretty)
	{
		rapidjson::PrettyWriter<DefaultOutputStreamWrapper> writer (output);
		writer.SetIndent ('\t', 1);
		auto result = writeRootNode (rootNode, writer);
		return result;
	}
	rapidjson::Writer<DefaultOutputStreamWrapper> writer (output);
	auto result = writeRootNode (rootNode, writer);
	return result;
}

//------------------------------------------------------------------------
} // UIJsonDescWriter

//------------------------------------------------------------------------
} // Detail
} // VSTGUI
