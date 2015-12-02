//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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

#include "../lib/vstguifwd.h"

#include <string>
#include <vector>
#include "../lib/platform/std_unorderedmap.h"

namespace VSTGUI {
class OutputStream;
class InputStream;

typedef std::unordered_map<std::string,std::string> UIAttributesMap;

//-----------------------------------------------------------------------------
class UIAttributes : public CBaseObject, private UIAttributesMap
{
public:
	typedef std::vector<std::string> StringArray;
	
	UIAttributes (UTF8StringPtr* attributes = 0);
	~UIAttributes ();

	using UIAttributesMap::begin;
	using UIAttributesMap::end;
	using UIAttributesMap::iterator;
	using UIAttributesMap::const_iterator;

	bool hasAttribute (const std::string& name) const;
	const std::string* getAttributeValue (const std::string& name) const;
	void setAttribute (const std::string& name, const std::string& value);
#if VSTGUI_RVALUE_REF_SUPPORT
	void setAttribute (const std::string& name, std::string&& value);
	void setAttribute (std::string&& name, std::string&& value);
#endif
	void removeAttribute (const std::string& name);

	void setBooleanAttribute (const std::string& name, bool value);
	bool getBooleanAttribute (const std::string& name, bool& value) const;

	void setIntegerAttribute (const std::string& name, int32_t value);
	bool getIntegerAttribute (const std::string& name, int32_t& value) const;

	void setDoubleAttribute (const std::string& name, double value);
	bool getDoubleAttribute (const std::string& name, double& value) const;
	
	void setPointAttribute (const std::string& name, const CPoint& p);
	bool getPointAttribute (const std::string& name, CPoint& p) const;
	
	void setRectAttribute (const std::string& name, const CRect& r);
	bool getRectAttribute (const std::string& name, CRect& r) const;

	void setStringArrayAttribute (const std::string& name, const StringArray& values);
	bool getStringArrayAttribute (const std::string& name, StringArray& values) const;
	
	static std::string createStringArrayValue (const StringArray& values);
	
	void removeAll () { clear (); }

	bool store (OutputStream& stream) const;
	bool restore (InputStream& stream);
};

}

#endif // __uiattributes__
