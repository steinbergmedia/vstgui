// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../lib/vstguibase.h"

#if VSTGUI_ENABLE_XML_PARSER

#include "uinode.h"
#include "../xmlparser.h"
#include <deque>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Detail {

//-----------------------------------------------------------------------------
struct UIXMLParser : public Xml::IHandler
{
	SharedPointer<UINode> parse (IContentProvider* provider);

	void startXmlElement (Xml::Parser* parser, IdStringPtr elementName, UTF8StringPtr* elementAttributes) override;
	void endXmlElement (Xml::Parser* parser, IdStringPtr name) override;
	void xmlCharData (Xml::Parser* parser, const int8_t* data, int32_t length) override;
	void xmlComment (Xml::Parser* parser, IdStringPtr comment) override;

	const SharedPointer<UINode> getNodes () const { return nodes; }

private:
	SharedPointer<UINode> nodes;
	std::deque<UINode*> nodeStack;
	bool restoreViewsMode {false};
};

//-----------------------------------------------------------------------------
class UIXMLDescWriter
{
public:
	using UINode = Detail::UINode;
	using UICommentNode = Detail::UICommentNode;
	bool write (OutputStream& stream, UINode* rootNode);
protected:
	static void encodeAttributeString (std::string& str);

	bool writeNode (UINode* node, OutputStream& stream);
	bool writeComment (UICommentNode* node, OutputStream& stream);
	bool writeNodeData (UINode::DataStorage& str, OutputStream& stream);
	bool writeAttributes (UIAttributes* attr, OutputStream& stream);
	int32_t intendLevel;
};


//------------------------------------------------------------------------
} // Detail
} // VSTGUI

#endif

