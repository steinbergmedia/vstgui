// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __xmlparser__
#define __xmlparser__

#include "../lib/vstguibase.h"
#include "cstream.h"
#include <memory>

namespace VSTGUI {
namespace Xml {

class Parser;

//-----------------------------------------------------------------------------
class IContentProvider
{
public:
	virtual uint32_t readRawXmlData (int8_t* buffer, uint32_t size) = 0;
	virtual void rewind () = 0;
};

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

//-----------------------------------------------------------------------------
class MemoryContentProvider : public CMemoryStream, public IContentProvider
{
public:
	MemoryContentProvider (const void* data, uint32_t dataSize);		// data must be valid the whole lifetime of this object
	uint32_t readRawXmlData (int8_t* buffer, uint32_t size) override;
	void rewind () override;
};

//-----------------------------------------------------------------------------
class InputStreamContentProvider : public IContentProvider
{
public:
	explicit InputStreamContentProvider (InputStream& stream);

	uint32_t readRawXmlData (int8_t* buffer, uint32_t size) override;
	void rewind () override;
protected:
	InputStream& stream;
	int64_t startPos;
};

}} // namespaces

#endif
