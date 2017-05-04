// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include <cstdlib>

/// @cond ignore

#define XML_STATIC 1
#define XML_NS 1
#define XML_DTD 1
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
#define HAVE_MEMMOVE	1
namespace VSTGUI {
namespace Xml {
	#include "expat/expat.h"
}} // namespaces

#include "xmlparser.h"
#include <algorithm>

#define PARSER static_cast<XML_ParserStruct*>(parser)

namespace VSTGUI {
namespace Xml {

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
: parser (nullptr)
, handler (nullptr)
{
	parser = XML_ParserCreate ("UTF-8");
}

//-----------------------------------------------------------------------------
Parser::~Parser () noexcept
{
	if (parser)
		XML_ParserFree (PARSER);
}

//-----------------------------------------------------------------------------
bool Parser::parse (IContentProvider* provider, IHandler* _handler)
{
	if (provider == nullptr || _handler == nullptr)
		return false;

	handler = _handler;
	XML_SetUserData (PARSER, this);
	XML_SetStartElementHandler (PARSER, gStartElementHandler);
	XML_SetEndElementHandler (PARSER, gEndElementHandler);
	XML_SetCharacterDataHandler (PARSER, gCharacterDataHandler);
	XML_SetCommentHandler (PARSER, gCommentHandler);

	static const uint32_t kBufferSize = 0x8000;

	provider->rewind ();

	while (true) 
	{
		void* buffer = XML_GetBuffer (PARSER, kBufferSize);
		if (buffer == nullptr)
		{
			handler = nullptr;
			return false;
		}

		uint32_t bytesRead = provider->readRawXmlData ((int8_t*)buffer, kBufferSize);
		if (bytesRead == kStreamIOError)
			bytesRead = 0;
		XML_Status status = XML_ParseBuffer (PARSER, static_cast<int> (bytesRead), bytesRead == 0);
		switch (status) 
		{
			case XML_STATUS_ERROR:
			{
				XML_Error error = XML_GetErrorCode (PARSER);
				if (error == XML_ERROR_JUNK_AFTER_DOC_ELEMENT) // that's ok
				{
					handler = nullptr;
					return true;
				}
				#if DEBUG
				XML_Size currentLineNumber = XML_GetCurrentLineNumber (PARSER);
				DebugPrint ("XML Parser Error on line: %d\n", currentLineNumber);
				DebugPrint ("%s\n", XML_ErrorString (XML_GetErrorCode (PARSER)));
				int offset, size;
				const char* inputContext = XML_GetInputContext (PARSER, &offset, &size);
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
				handler = nullptr;
				return false;
			}
			case XML_STATUS_SUSPENDED:
			{
				handler = nullptr;
				return true;
			}
			default:
				break;
		}

		if (bytesRead == 0)
			break;
	}
	handler = nullptr;
	return true;
}

//-----------------------------------------------------------------------------
bool Parser::stop ()
{
	XML_StopParser (PARSER, false);
	return true;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
MemoryContentProvider::MemoryContentProvider (const void* data, uint32_t dataSize)
: CMemoryStream ((const int8_t*)data, dataSize, false)
{
}

//------------------------------------------------------------------------
uint32_t MemoryContentProvider::readRawXmlData (int8_t* buffer, uint32_t size)
{
	return readRaw (buffer, size);
}

//------------------------------------------------------------------------
void MemoryContentProvider::rewind ()
{
	CMemoryStream::rewind ();
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
InputStreamContentProvider::InputStreamContentProvider (InputStream& stream)
: stream (stream)
, startPos (0)
{
	SeekableStream* seekStream = dynamic_cast<SeekableStream*> (&stream);
	if (seekStream)
		startPos = seekStream->tell ();	
}

//------------------------------------------------------------------------
uint32_t InputStreamContentProvider::readRawXmlData (int8_t* buffer, uint32_t size)
{
	return stream.readRaw (buffer, size);
}

//------------------------------------------------------------------------
void InputStreamContentProvider::rewind ()
{
	SeekableStream* seekStream = dynamic_cast<SeekableStream*> (&stream);
	if (seekStream)
		seekStream->seek (startPos, SeekableStream::kSeekSet);
}

//------------------------------------------------------------------------
#ifdef __clang__
#pragma clang diagnostic ignored "-Wconversion"
#endif

#include "./expat/xmltok.c"
#include "./expat/xmlrole.c"
#include "./expat/xmlparse.c"

}} // namespaces

#ifdef OLD_BYTEORDER
	#undef BYTEORDER
	#define BYTEORDER = OLD_BYTEORDER
#endif

/// @endcond
