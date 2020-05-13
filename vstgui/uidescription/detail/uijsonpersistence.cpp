// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../uiattributes.h"
#include "uijsonpersistence.h"

#define RAPIDJSON_HAS_STDSTRING 1
#include "../rapidjson/include/rapidjson/prettywriter.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Detail {

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
static bool writeAttributes (UIAttributes& attr, JSONWriter& writer)
{
	if (attr.empty ())
		return true;

	writer.Key ("Attributes");
	writer.StartObject ();
	for (const auto& a : attr)
	{
		writer.Key (a.first.data (), a.first.size ());
		writer.String (a.second);
	}
	writer.EndObject ();
	return true;
}

//------------------------------------------------------------------------
static bool writeNode (UINode* node, JSONWriter& writer)
{
	if (node->noExport ())
		return true;
	writer.StartObject ();
	writer.Key ("Name");
	writer.String (node->getName ());
	if (auto attr = node->getAttributes ())
		writeAttributes (*attr, writer);
	if (!node->getData ().empty ())
	{
		writer.Key ("Data");
		writer.String (node->getData ());
	}
	if (!node->getChildren ().empty ())
	{
		writer.Key ("Children");
		writer.StartArray ();
		for (auto& child : node->getChildren ())
		{
			writeNode (child, writer);
		}
		writer.EndArray ();
	}
	writer.EndObject ();
	return true;
}

//------------------------------------------------------------------------
bool write (OutputStream& stream, UINode* rootNode)
{
	DefaultOutputStreamWrapper output (stream);
	JSONWriter writer (output);
	writer.SetIndent ('\t', 1);
	auto result = writeNode (rootNode, writer);
	return result;
}

//------------------------------------------------------------------------
} // UIJsonDescWriter

//------------------------------------------------------------------------
} // Detail
} // VSTGUI
