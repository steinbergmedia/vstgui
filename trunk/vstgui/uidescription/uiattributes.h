//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef __uiattributes__
#define __uiattributes__

#include "../lib/vstguibase.h"

#include <map>
#include <vector>

namespace VSTGUI {
struct CPoint;
struct CRect;
class OutputStream;
class InputStream;

//-----------------------------------------------------------------------------
class UIAttributes : public CBaseObject, private std::map<std::string,std::string>
{
public:
	UIAttributes (UTF8StringPtr* attributes = 0);
	~UIAttributes ();

	using std::map<std::string,std::string>::begin;
	using std::map<std::string,std::string>::end;
	using std::map<std::string,std::string>::iterator;
	using std::map<std::string,std::string>::const_iterator;

	bool hasAttribute (UTF8StringPtr name) const;
	const std::string* getAttributeValue (UTF8StringPtr name) const;
	void setAttribute (UTF8StringPtr name, UTF8StringPtr value);
	void removeAttribute (UTF8StringPtr name);

	void setBooleanAttribute (UTF8StringPtr name, bool value);
	bool getBooleanAttribute (UTF8StringPtr name, bool& value) const;

	void setIntegerAttribute (UTF8StringPtr name, int32_t value);
	bool getIntegerAttribute (UTF8StringPtr name, int32_t& value) const;

	void setDoubleAttribute (UTF8StringPtr name, double value);
	bool getDoubleAttribute (UTF8StringPtr name, double& value) const;
	
	void setPointAttribute (UTF8StringPtr name, const CPoint& p);
	bool getPointAttribute (UTF8StringPtr name, CPoint& p) const;
	
	void setRectAttribute (UTF8StringPtr name, const CRect& r);
	bool getRectAttribute (UTF8StringPtr name, CRect& r) const;
	
	void setAttributeArray (UTF8StringPtr name, const std::vector<std::string>& values);
	bool getAttributeArray (UTF8StringPtr name, std::vector<std::string>& values) const;
	
	void removeAll () { clear (); }

	bool store (OutputStream& stream);
	bool restore (InputStream& stream);
};

}

#endif // __uiattributes__
