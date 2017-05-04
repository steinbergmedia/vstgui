// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../unittests.h"
#include "../../../uidescription/xmlparser.h"
#include <string>

namespace VSTGUI {
using namespace Xml;

namespace {

struct Handler : public IHandler
{
	bool stopOnStartElement {false};
	void startXmlElement (Parser* parser, IdStringPtr elementName, UTF8StringPtr* elementAttributes) override
	{
		if (stopOnStartElement)
			parser->stop ();
	}
	void endXmlElement (Parser* parser, IdStringPtr name) override
	{
	}
	void xmlCharData (Parser* parser, const int8_t* data, int32_t length) override
	{
	}
	void xmlComment (Parser* parser, IdStringPtr comment) override
	{
	}

};

} // anonymous

constexpr auto validXML =
R"(<?xml version="1.0" encoding="UTF-8"?>
<tag attr="bla">
CHARDATA
<!-- comment -->
</tag>
)";

constexpr auto validXMLWithJunkAtEnd =
R"(<?xml version="1.0" encoding="UTF-8"?>
<tag attr="bla">
CHARDATA
<!-- comment -->
</tag>
this is junk
)";

constexpr auto invalidXML =
R"(
<tag attr="bla">
CHARDATA
<!-- comment -->
</tag2>
)";

TESTCASE(XMLParserTest,
	
	TEST(validParse,
		MemoryContentProvider provider (validXML, strlen (validXML));
		Handler handler;
		Parser p;
		EXPECT(p.parse (&provider, &handler) == true);
	);

	TEST(validParseWithJunkAtEnd,
		MemoryContentProvider provider (validXMLWithJunkAtEnd, strlen (validXMLWithJunkAtEnd));
		Handler handler;
		Parser p;
		EXPECT(p.parse (&provider, &handler) == true);
	);

	TEST(invalidParse,
		MemoryContentProvider provider (invalidXML, strlen (invalidXML));
		Handler handler;
		Parser p;
		EXPECT(p.parse (&provider, &handler) == false);
	);

	TEST(stopParse,
		MemoryContentProvider provider (validXML, strlen (validXML));
		Handler handler;
		handler.stopOnStartElement = true;
		Parser p;
		EXPECT(p.parse (&provider, &handler) == false);
	);

);

} // VSTGUI
