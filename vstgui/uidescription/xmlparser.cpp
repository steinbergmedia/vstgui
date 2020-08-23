// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../lib/vstguibase.h"

#if VSTGUI_ENABLE_XML_PARSER

#include "cstream.h"
#include "icontentprovider.h"

/// @cond ignore
#if VSTGUI_USE_SYSTEM_EXPAT
#include <expat.h>
#else
#define XML_STATIC 1
#define XML_NS 0
#define XML_DTD 0
#define XML_CONTEXT_BYTES 1024
#define XML_LARGE_SIZE 1

#ifdef BYTEORDER
	#define OLD_BYTEORDER = BYTEORDER
	#undef BYTEORDER
#endif

#if MAC && defined (__BIG_ENDIAN__)
	#define BYTEORDER 4321
#else
	#define BYTEORDER 1234
#endif
#define HAVE_MEMMOVE

namespace VSTGUI {
namespace Xml {
#include "expat/expat.h"
}}
#endif // VSTGUI_USE_SYSTEM_EXPAT

#include "xmlparser.h"
#include <algorithm>

namespace VSTGUI {
namespace Xml {

//------------------------------------------------------------------------
struct Parser::Impl
{
	XML_ParserStruct* parser {nullptr};
	IHandler* handler {nullptr};
};

//------------------------------------------------------------------------
static void XMLCALL gStartElementHandler (void* userData, const char* name, const char** atts)
{
	auto parser = static_cast<Parser*> (userData);
	IHandler* handler = parser ? parser->getHandler () : nullptr;
	if (handler)
		handler->startXmlElement (parser, name, atts);
}

//------------------------------------------------------------------------
static void XMLCALL gEndElementHandler (void* userData, const char* name)
{
	auto parser = static_cast<Parser*> (userData);
	IHandler* handler = parser ? parser->getHandler () : nullptr;
	if (handler)
		handler->endXmlElement (parser, name);
}

//------------------------------------------------------------------------
static void XMLCALL gCharacterDataHandler (void* userData, const char* s, int len)
{
	auto parser = static_cast<Parser*> (userData);
	IHandler* handler = parser ? parser->getHandler () : nullptr;
	if (handler)
		handler->xmlCharData (parser, (const int8_t*)s, len);
}

//------------------------------------------------------------------------
static void XMLCALL gCommentHandler (void* userData, const char* string)
{
	auto parser = static_cast<Parser*> (userData);
	IHandler* handler = parser ? parser->getHandler () : nullptr;
	if (handler)
		handler->xmlComment (parser, string);
}

//-----------------------------------------------------------------------------
Parser::Parser ()
{
	pImpl = std::unique_ptr<Impl> (new Impl ());
	pImpl->parser = XML_ParserCreate ("UTF-8");
}

//-----------------------------------------------------------------------------
Parser::~Parser () noexcept
{
	if (pImpl->parser)
		XML_ParserFree (pImpl->parser);
}

//-----------------------------------------------------------------------------
IHandler* Parser::getHandler () const
{
	return pImpl->handler;
}

//-----------------------------------------------------------------------------
bool Parser::parse (IContentProvider* provider, IHandler* handler)
{
	if (provider == nullptr || handler == nullptr)
		return false;

	pImpl->handler = handler;
	XML_SetUserData (pImpl->parser, this);
	XML_SetStartElementHandler (pImpl->parser, gStartElementHandler);
	XML_SetEndElementHandler (pImpl->parser, gEndElementHandler);
	XML_SetCharacterDataHandler (pImpl->parser, gCharacterDataHandler);
	XML_SetCommentHandler (pImpl->parser, gCommentHandler);

	static const uint32_t kBufferSize = 0x8000;

	provider->rewind ();

	while (true) 
	{
		void* buffer = XML_GetBuffer (pImpl->parser, kBufferSize);
		if (buffer == nullptr)
		{
			pImpl->handler = nullptr;
			return false;
		}

		uint32_t bytesRead = provider->readRawData ((int8_t*)buffer, kBufferSize);
		if (bytesRead == kStreamIOError)
			bytesRead = 0;
		XML_Status status = XML_ParseBuffer (pImpl->parser, static_cast<int> (bytesRead), bytesRead == 0);
		switch (status) 
		{
			case XML_STATUS_ERROR:
			{
				XML_Error error = XML_GetErrorCode (pImpl->parser);
				if (error == XML_ERROR_JUNK_AFTER_DOC_ELEMENT) // that's ok
				{
					pImpl->handler = nullptr;
					return true;
				}
				#if DEBUG
				XML_Size currentLineNumber = XML_GetCurrentLineNumber (pImpl->parser);
				DebugPrint ("XML Parser Error on line: %d\n", currentLineNumber);
				DebugPrint ("%s\n", XML_ErrorString (XML_GetErrorCode (pImpl->parser)));
				int offset, size;
				const char* inputContext = XML_GetInputContext (pImpl->parser, &offset, &size);
				if (inputContext)
				{
					int pos = offset;
					while (offset > 0 && pos - offset < 20)
					{
						if (inputContext[offset] == '\n')
						{
							offset++;
							break;
						}
						offset--;
					}
					for (int i = offset; i < size && i - offset < 40; i++)
					{
						if (inputContext[i] == '\n')
							break;
						if (inputContext[i] == '\t')
							DebugPrint (" ");
						else
							DebugPrint ("%c", inputContext[i]);
					}
					DebugPrint ("\n");
					for (int i = offset; i < pos; i++)
					{
						DebugPrint (" ");
					}
					DebugPrint ("^\n");
				}
				#endif
				pImpl->handler = nullptr;
				return false;
			}
			case XML_STATUS_SUSPENDED:
			{
				pImpl->handler = nullptr;
				return true;
			}
			default:
				break;
		}

		if (bytesRead == 0)
			break;
	}
	pImpl->handler = nullptr;
	return true;
}

//-----------------------------------------------------------------------------
bool Parser::stop ()
{
	XML_StopParser (pImpl->parser, false);
	return true;
}

//------------------------------------------------------------------------
}} // namespaces

#if !VSTGUI_USE_SYSTEM_EXPAT

//------------------------------------------------------------------------
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wshadow"
#endif

namespace VSTGUI {
namespace Xml {
#include "./expat/xmltok.c"
#include "./expat/xmlrole.c"
#include "./expat/xmlparse.c"
}}

#ifdef OLD_BYTEORDER
	#undef BYTEORDER
	#define BYTEORDER = OLD_BYTEORDER
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif

/// @endcond

#endif // VSTGUI_ENABLE_XML_PARSER
