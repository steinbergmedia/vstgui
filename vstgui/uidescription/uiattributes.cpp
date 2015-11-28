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

#include "uiattributes.h"
#include "cstream.h"
#include "../lib/cpoint.h"
#include "../lib/crect.h"
#include "../lib/cstring.h"
#include <sstream>
#include <algorithm>

namespace VSTGUI {

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UIAttributes::UIAttributes (UTF8StringPtr* attributes)
{
	if (attributes)
	{
		int32_t i = 0;
		while (attributes[i] != NULL && attributes[i+1] != NULL)
		{
			insert (std::make_pair (attributes[i], attributes[i+1]));
			i += 2;
		}
	}
}

//-----------------------------------------------------------------------------
UIAttributes::~UIAttributes ()
{
}

//-----------------------------------------------------------------------------
bool UIAttributes::hasAttribute (const std::string& name) const
{
	if (getAttributeValue (name) != 0)
		return true;
	return false;
}

//-----------------------------------------------------------------------------
const std::string* UIAttributes::getAttributeValue (const std::string& name) const
{
	const_iterator iter = find (name);
	if (iter != end ())
		return &iter->second;
	return 0;
}

//-----------------------------------------------------------------------------
void UIAttributes::setAttribute (const std::string& name, const std::string& value)
{
	iterator iter = find (name);
	if (iter != end ())
		erase (iter);
	insert (std::make_pair (name, value));
}

#if VSTGUI_RVALUE_REF_SUPPORT
//-----------------------------------------------------------------------------
void UIAttributes::setAttribute (const std::string& name, std::string&& value)
{
	iterator iter = find (name);
	if (iter != end ())
		erase (iter);
	insert (std::make_pair (name, std::move (value)));
}

//-----------------------------------------------------------------------------
void UIAttributes::setAttribute (std::string&& name, std::string&& value)
{
	iterator iter = find (name);
	if (iter != end ())
		erase (iter);
	insert (std::make_pair (std::move (name), std::move (value)));
}
#endif

//-----------------------------------------------------------------------------
void UIAttributes::removeAttribute (const std::string& name)
{
	iterator iter = find (name);
	if (iter != end ())
		erase (iter);
}

//-----------------------------------------------------------------------------
void UIAttributes::setDoubleAttribute (const std::string& name, double value)
{
	std::stringstream str;
	str.imbue (std::locale::classic ());
	str.precision (40);
	str << value;
	setAttribute (name, str.str ());
}

//-----------------------------------------------------------------------------
bool UIAttributes::getDoubleAttribute (const std::string& name, double& value) const
{
	const std::string* str = getAttributeValue (name);
	if (str)
	{
		std::istringstream sstream (*str);
		sstream.imbue (std::locale::classic ());
		sstream.precision (40);
		sstream >> value;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void UIAttributes::setBooleanAttribute (const std::string& name, bool value)
{
	setAttribute (name, value ? "true" : "false");
}

//-----------------------------------------------------------------------------
bool UIAttributes::getBooleanAttribute (const std::string& name, bool& value) const
{
	const std::string* str = getAttributeValue (name);
	if (str)
	{
		if (*str == "true")
		{
			value = true;
			return true;
		}
		else if (*str == "false")
		{
			value = false;
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
void UIAttributes::setIntegerAttribute (const std::string& name, int32_t value)
{
	std::stringstream str;
	str << value;
	setAttribute (name, str.str ());
}

//-----------------------------------------------------------------------------
bool UIAttributes::getIntegerAttribute (const std::string& name, int32_t& value) const
{
	const std::string* str = getAttributeValue (name);
	if (str)
	{
		value = (int32_t)strtol (str->c_str (), 0, 10);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void UIAttributes::setPointAttribute (const std::string& name, const CPoint& p)
{
	std::stringstream str;
	str << p.x;
	str << ", ";
	str << p.y;
	setAttribute (name, str.str ());
}

//-----------------------------------------------------------------------------
bool UIAttributes::getPointAttribute (const std::string& name, CPoint& p) const
{
	const std::string* str = getAttributeValue (name);
	if (str)
	{
		size_t start = 0;
		size_t pos = str->find (",", start, 1);
		if (pos != std::string::npos)
		{
			StringArray subStrings;
			while (pos != std::string::npos)
			{
				std::string name (*str, start, pos - start);
				subStrings.push_back (name);
				start = pos+1;
				pos = str->find (",", start, 1);
			}
			std::string name (*str, start, std::string::npos);
			subStrings.push_back (name);
			if (subStrings.size () == 2)
			{
				p.x = UTF8StringView (subStrings[0].c_str ()).toDouble ();
				p.y = UTF8StringView (subStrings[1].c_str ()).toDouble ();
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
void UIAttributes::setRectAttribute (const std::string& name, const CRect& r)
{
	std::stringstream str;
	str << r.left;
	str << ", ";
	str << r.top;
	str << ", ";
	str << r.right;
	str << ", ";
	str << r.bottom;
	setAttribute (name, str.str ());
}

//-----------------------------------------------------------------------------
bool UIAttributes::getRectAttribute (const std::string& name, CRect& r) const
{
	const std::string* str = getAttributeValue (name);
	if (str)
	{
		size_t start = 0;
		size_t pos = str->find (",", start, 1);
		if (pos != std::string::npos)
		{
			StringArray subStrings;
			while (pos != std::string::npos)
			{
				std::string name (*str, start, pos - start);
				subStrings.push_back (name);
				start = pos+1;
				pos = str->find (",", start, 1);
			}
			std::string name (*str, start, std::string::npos);
			subStrings.push_back (name);
			if (subStrings.size () == 4)
			{
				r.left = UTF8StringView (subStrings[0].c_str ()).toDouble ();
				r.top = UTF8StringView (subStrings[1].c_str ()).toDouble ();
				r.right = UTF8StringView (subStrings[2].c_str ()).toDouble ();
				r.bottom = UTF8StringView (subStrings[3].c_str ()).toDouble ();
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
void UIAttributes::setStringArrayAttribute (const std::string& name, const StringArray& values)
{
	setAttribute (name, createStringArrayValue (values));
}

//-----------------------------------------------------------------------------
bool UIAttributes::getStringArrayAttribute (const std::string& name, StringArray& values) const
{
	const std::string* str = getAttributeValue (name);
	if (str)
	{
		std::stringstream ss (*str);
		std::string item;
		while (std::getline (ss, item, ','))
		{
			values.push_back (item);
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
std::string UIAttributes::createStringArrayValue (const StringArray& values)
{
	std::string value;
	size_t numValues = values.size ();
	for (size_t i = 0; i < numValues - 1; i++)
	{
		value += values[i];
		value += ',';
	}
	value += values[numValues-1];
	return value;
}

//-----------------------------------------------------------------------------
bool UIAttributes::store (OutputStream& stream) const
{
	if (!(stream << (int32_t)'UIAT')) return false;
	if (!(stream << (uint32_t)size ())) return false;
	const_iterator it = begin ();
	while (it != end ())
	{
		if (!(stream << (*it).first)) return false;
		if (!(stream << (*it).second)) return false;
		it++;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool UIAttributes::restore (InputStream& stream)
{
	int32_t identifier;
	if (!(stream >> identifier)) return false;
	if (identifier == 'UIAT')
	{
		uint32_t numAttr;
		if (!(stream >> numAttr)) return false;
		for (uint32_t i = 0; i < numAttr; i++)
		{
			std::string key, value;
			if (!(stream >> key)) return false;
			if (!(stream >> value)) return false;
			setAttribute (key, value);
		}
		return true;
	}
	return false;
}

}