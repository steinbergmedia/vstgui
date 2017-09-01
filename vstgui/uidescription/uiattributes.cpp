// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
		while (attributes[i] != nullptr && attributes[i+1] != nullptr)
		{
			emplace (attributes[i], attributes[i+1]);
			i += 2;
		}
	}
}

//-----------------------------------------------------------------------------
bool UIAttributes::hasAttribute (const std::string& name) const
{
	if (getAttributeValue (name) != nullptr)
		return true;
	return false;
}

//-----------------------------------------------------------------------------
const std::string* UIAttributes::getAttributeValue (const std::string& name) const
{
	const_iterator iter = find (name);
	if (iter != end ())
		return &iter->second;
	return nullptr;
}

//-----------------------------------------------------------------------------
void UIAttributes::setAttribute (const std::string& name, const std::string& value)
{
	iterator iter = find (name);
	if (iter != end ())
		iter->second = value;
	else
		emplace (name, value);
}

//-----------------------------------------------------------------------------
void UIAttributes::setAttribute (const std::string& name, std::string&& value)
{
	iterator iter = find (name);
	if (iter != end ())
		iter->second = std::move (value);
	else
		emplace (name, std::move (value));
}

//-----------------------------------------------------------------------------
void UIAttributes::setAttribute (std::string&& name, std::string&& value)
{
	iterator iter = find (name);
	if (iter != end ())
		iter->second = std::move (value);
	else
		emplace (std::move (name), std::move (value));
}

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
		value = (int32_t)strtol (str->c_str (), nullptr, 10);
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
				subStrings.emplace_back (*str, start, pos - start);
				start = pos+1;
				pos = str->find (",", start, 1);
			}
			subStrings.emplace_back (*str, start, std::string::npos);
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
				subStrings.emplace_back (*str, start, pos - start);
				start = pos+1;
				pos = str->find (",", start, 1);
			}
			subStrings.emplace_back (*str, start, std::string::npos);
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
			values.emplace_back (std::move (item));
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
		++it;
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
			setAttribute (std::move (key), std::move (value));
		}
		return true;
	}
	return false;
}

}
