//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef __xmlparser__
#define __xmlparser__

namespace VSTGUI {
namespace Xml {

class Parser;

//-----------------------------------------------------------------------------
class IContentProvider
{
public:
	virtual int readRawXmlData (char* buffer, int size) = 0;
	virtual void rewind () = 0;
};

//-----------------------------------------------------------------------------
class IHandler
{
public:
	virtual void startXmlElement (Parser* parser, const char* elementName, const char** elementAttributes) = 0;
	virtual void endXmlElement (Parser* parser, const char* name) = 0;
	virtual void xmlCharData (Parser* parser, const char* data, int length) = 0;
	virtual void xmlComment (Parser* parser, const char* comment) = 0;
};

//-----------------------------------------------------------------------------
class Parser
{
public:
	Parser ();
	virtual ~Parser ();

	bool parse (IContentProvider* provider, IHandler* handler);

	bool stop ();

	IHandler* getHandler () const { return handler; }
protected:
	void* parser;
	IHandler* handler;
};

//-----------------------------------------------------------------------------
class MemoryContentProvider : public IContentProvider
{
public:
	MemoryContentProvider (const void* data, int dataSize);		// data must be valid the whole lifetime of this object
	int readRawXmlData (char* buffer, int size);
	void rewind ();
protected:
	const void* data;
	int dataSize;
	int pos;
};

}} // namespaces

#endif
