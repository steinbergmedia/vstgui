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

#include "uiattributes.h"
#include "cstream.h"
#include "../lib/cpoint.h"
#include "../lib/crect.h"
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
bool UIAttributes::hasAttribute (UTF8StringPtr name) const
{
	if (getAttributeValue (name) != 0)
		return true;
	return false;
}

//-----------------------------------------------------------------------------
const std::string* UIAttributes::getAttributeValue (UTF8StringPtr name) const
{
	const_iterator iter = find (name);
	if (iter != end ())
		return &iter->second;
	return 0;
}

//-----------------------------------------------------------------------------
void UIAttributes::setAttribute (UTF8StringPtr name, UTF8StringPtr value)
{
	iterator iter = find (name);
	if (iter != end ())
		erase (iter);
	insert (std::make_pair (name, value));
}

//-----------------------------------------------------------------------------
void UIAttributes::removeAttribute (UTF8StringPtr name)
{
	iterator iter = find (name);
	if (iter != end ())
		erase (iter);
}

//-----------------------------------------------------------------------------
void UIAttributes::setDoubleAttribute (UTF8StringPtr name, double value)
{
	std::stringstream str;
	str.precision (40);
	str << value;
	setAttribute (name, str.str ().c_str ());
}

//-----------------------------------------------------------------------------
bool UIAttributes::getDoubleAttribute (UTF8StringPtr name, double& value) const
{
	const std::string* str = getAttributeValue (name);
	if (str)
	{
		value = strtod (str->c_str (), 0);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void UIAttributes::setBooleanAttribute (UTF8StringPtr name, bool value)
{
	setAttribute (name, value ? "true" : "false");
}

//-----------------------------------------------------------------------------
bool UIAttributes::getBooleanAttribute (UTF8StringPtr name, bool& value) const
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
void UIAttributes::setIntegerAttribute (UTF8StringPtr name, int32_t value)
{
	std::stringstream str;
	str << value;
	setAttribute (name, str.str ().c_str ());
}

//-----------------------------------------------------------------------------
bool UIAttributes::getIntegerAttribute (UTF8StringPtr name, int32_t& value) const
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
void UIAttributes::setPointAttribute (UTF8StringPtr name, const CPoint& p)
{
	std::stringstream str;
	str << p.x;
	str << ", ";
	str << p.y;
	setAttribute (name, str.str ().c_str ());
}

//-----------------------------------------------------------------------------
bool UIAttributes::getPointAttribute (UTF8StringPtr name, CPoint& p) const
{
	const std::string* str = getAttributeValue (name);
	if (str)
	{
		size_t start = 0;
		size_t pos = str->find (",", start, 1);
		if (pos != std::string::npos)
		{
			std::vector<std::string> subStrings;
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
				p.x = strtod (subStrings[0].c_str (), 0);
				p.y = strtod (subStrings[1].c_str (), 0);
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
void UIAttributes::setRectAttribute (UTF8StringPtr name, const CRect& r)
{
	std::stringstream str;
	str << r.left;
	str << ", ";
	str << r.top;
	str << ", ";
	str << r.right;
	str << ", ";
	str << r.bottom;
	setAttribute (name, str.str ().c_str ());
}

//-----------------------------------------------------------------------------
bool UIAttributes::getRectAttribute (UTF8StringPtr name, CRect& r) const
{
	const std::string* str = getAttributeValue (name);
	if (str)
	{
		size_t start = 0;
		size_t pos = str->find (",", start, 1);
		if (pos != std::string::npos)
		{
			std::vector<std::string> subStrings;
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
				r.left = strtod (subStrings[0].c_str (), 0);
				r.top = strtod (subStrings[1].c_str (), 0);
				r.right = strtod (subStrings[2].c_str (), 0);
				r.bottom = strtod (subStrings[3].c_str (), 0);
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
void UIAttributes::setAttributeArray (UTF8StringPtr name, const std::vector<std::string>& values)
{
	std::string value;
	size_t numValues = values.size ();
	for (size_t i = 0; i < numValues - 1; i++)
	{
		value += values[i];
		value += ',';
	}
	value += values[numValues-1];
}

//-----------------------------------------------------------------------------
bool UIAttributes::getAttributeArray (UTF8StringPtr name, std::vector<std::string>& values) const
{
	const std::string* str = getAttributeValue (name);
	if (str)
	{
		std::stringstream ss (*str);
		std::string item;
		while (std::getline (ss, item, ','))
		{
			values.push_back(item);
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool UIAttributes::store (OutputStream& stream)
{
	if (!(stream << (int32_t)'UIAT')) return false;
	if (!(stream << (uint32_t)size ())) return false;
	iterator it = begin ();
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
			setAttribute (key.c_str (), value.c_str ());
		}
		return true;
	}
	return false;
}

}