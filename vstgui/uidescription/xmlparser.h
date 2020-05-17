// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../lib/vstguibase.h"

#if VSTGUI_ENABLE_XML_PARSER

#include "uidescriptionfwd.h"
#include <memory>

namespace VSTGUI {
namespace Xml {

class Parser;

//-----------------------------------------------------------------------------
class IHandler
{
public:
	virtual void startXmlElement (Parser* parser, IdStringPtr elementName, UTF8StringPtr* elementAttributes) = 0;
	virtual void endXmlElement (Parser* parser, IdStringPtr name) = 0;
	virtual void xmlCharData (Parser* parser, const int8_t* data, int32_t length) = 0;
	virtual void xmlComment (Parser* parser, IdStringPtr comment) = 0;
};

//-----------------------------------------------------------------------------
class Parser
{
public:
	Parser ();
	virtual ~Parser () noexcept;

	bool parse (IContentProvider* provider, IHandler* handler);

	bool stop ();

	IHandler* getHandler () const;
protected:
	struct Impl;
	std::unique_ptr<Impl> pImpl;
};

}} // namespaces

#endif // VSTGUI_ENABLE_XML_PARSER
