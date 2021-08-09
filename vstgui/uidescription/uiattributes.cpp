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
namespace {

//------------------------------------------------------------------------
template<bool OnlyInteger>
Optional<std::string> trimmedNumericalString (const std::string& str, size_t from, size_t numChars)
{
	std::string result;
	auto strSize = str.size ();
	if (from >= strSize)
		return {};
	auto points = 0u;
	auto to = numChars == std::string::npos ? strSize : from + numChars;
	for (auto i = from; i < to && i < strSize; ++i)
	{
		auto c = str[i];
		if (!std::isspace (c))
		{
			if (!std::isdigit (c) && c != '-' && c != '+')
			{
				if (!OnlyInteger && c == '.' && points == 0u)
					++points;
				else if (points == 1u && c == 'e')
				{
				}
				else
					return {};
			}
			result.push_back (c);
		}
	}
	return Optional<std::string> {std::move (result)};
}

} // anonymous

//-----------------------------------------------------------------------------
std::string UIAttributes::pointToString (CPoint p)
{
	return doubleToString (p.x) + ", " + doubleToString (p.y);
}

//-----------------------------------------------------------------------------
bool UIAttributes::stringToPoint (const std::string& str, CPoint& p)
{
	size_t start = 0;
	size_t pos = str.find (",", start, 1);
	if (pos != std::string::npos)
	{
		std::vector<std::string> subStrings;
		while (pos != std::string::npos)
		{
			if (!subStrings.empty ())
				return false;
			if (auto subStr = trimmedNumericalString<false> (str, start, pos - start))
				subStrings.emplace_back (std::move (*subStr));
			else
				return false;
			start = pos + 1;
			pos = str.find (",", start, 1);
		}
		if (auto subStr = trimmedNumericalString<false> (str, start, std::string::npos))
			subStrings.emplace_back (std::move (*subStr));
		else
			return false;
		if (subStrings.size () == 2)
		{
			p.x = UTF8StringView (subStrings[0].data ()).toDouble ();
			p.y = UTF8StringView (subStrings[1].data ()).toDouble ();
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------
std::string UIAttributes::doubleToString (double value, uint32_t precision)
{
	std::stringstream str;
	str.imbue (std::locale::classic ());
	str.precision (precision);
	str << value;
	return str.str ();
}

//-----------------------------------------------------------------------------
bool UIAttributes::stringToDouble (const std::string& str, double& value)
{
	if (auto string = trimmedNumericalString<false> (str, 0, str.size ()))
	{
		std::istringstream sstream (*string);
		sstream.imbue (std::locale::classic ());
		sstream >> value;
		return sstream.fail () == false;
	}
	return false;
}

//-----------------------------------------------------------------------------
std::string UIAttributes::boolToString (bool value)
{
	return value ? "true" : "false";
}

//-----------------------------------------------------------------------------
bool UIAttributes::stringToBool (const std::string& str, bool& value)
{
	if (str == "true")
	{
		value = true;
		return true;
	}
	if (str == "false")
	{
		value = false;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
std::string UIAttributes::integerToString (int32_t value)
{
	std::stringstream str;
	str << value;
	return str.str ();
}

//-----------------------------------------------------------------------------
bool UIAttributes::stringToInteger (const std::string& str, int32_t& value)
{
	if (auto string = trimmedNumericalString<true> (str, 0, str.size ()))
	{
		std::istringstream sstream (*string);
		sstream.imbue (std::locale::classic ());
		sstream >> value;
		return sstream.fail () == false;
	}
	return false;
}

//-----------------------------------------------------------------------------
std::string UIAttributes::rectToString (CRect r, uint32_t precision)
{
	return doubleToString (r.left) + ", " + doubleToString (r.top) + ", " +
	       doubleToString (r.right) + ", " + doubleToString (r.bottom);
}

//-----------------------------------------------------------------------------
bool UIAttributes::stringToRect (const std::string& str, CRect& r)
{
	size_t start = 0;
	size_t pos = str.find (",", start, 1);
	if (pos != std::string::npos)
	{
		std::vector<std::string> subStrings;
		while (pos != std::string::npos)
		{
			if (subStrings.size () >= 3)
				return false;
			if (auto subStr = trimmedNumericalString<false> (str, start, pos - start))
				subStrings.emplace_back (std::move (*subStr));
			else
				return false;
			start = pos + 1;
			pos = str.find (",", start, 1);
		}
		if (auto subStr = trimmedNumericalString<false> (str, start, std::string::npos))
			subStrings.emplace_back (std::move (*subStr));
		else
			return false;
		if (subStrings.size () == 4)
		{
			r.left = UTF8StringView (subStrings[0].data ()).toDouble ();
			r.top = UTF8StringView (subStrings[1].data ()).toDouble ();
			r.right = UTF8StringView (subStrings[2].data ()).toDouble ();
			r.bottom = UTF8StringView (subStrings[3].data ()).toDouble ();
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
std::string UIAttributes::stringArrayToString (const StringArray& values)
{
	if (values.empty())
		return {};

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
bool UIAttributes::stringToStringArray (const std::string& str, std::vector<std::string>& values)
{
	std::stringstream ss (str);
	std::string item;
	while (std::getline (ss, item, ','))
	{
		values.emplace_back (std::move (item));
	}
	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UIAttributes::UIAttributes (UTF8StringPtr* attributes)
{
	if (attributes)
	{
		size_t count = 0;
		while (attributes[count] != nullptr && attributes[count+1] != nullptr)
			count += 2;
		if (count)
			UIAttributesMap::reserve (count / 2);
		
		int32_t i = 0;
		while (attributes[i] != nullptr && attributes[i+1] != nullptr)
		{
			emplace (attributes[i], attributes[i+1]);
			i += 2;
		}
	}
}

//------------------------------------------------------------------------
UIAttributes::UIAttributes (size_t reserve)
{
	UIAttributesMap::reserve (reserve);
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
	setAttribute (name, doubleToString (value, 40));
}

//-----------------------------------------------------------------------------
bool UIAttributes::getDoubleAttribute (const std::string& name, double& value) const
{
	if (auto str = getAttributeValue (name))
		return stringToDouble (*str, value);
	return false;
}

//-----------------------------------------------------------------------------
void UIAttributes::setBooleanAttribute (const std::string& name, bool value)
{
	setAttribute (name, boolToString (value));
}

//-----------------------------------------------------------------------------
bool UIAttributes::getBooleanAttribute (const std::string& name, bool& value) const
{
	if (auto str = getAttributeValue (name))
		return stringToBool (*str, value);
	return false;
}

//-----------------------------------------------------------------------------
void UIAttributes::setIntegerAttribute (const std::string& name, int32_t value)
{
	setAttribute (name, integerToString (value));
}

//-----------------------------------------------------------------------------
bool UIAttributes::getIntegerAttribute (const std::string& name, int32_t& value) const
{
	if (auto str = getAttributeValue (name))
		return stringToInteger(*str, value);
	return false;
}

//-----------------------------------------------------------------------------
void UIAttributes::setPointAttribute (const std::string& name, const CPoint& p)
{
	setAttribute (name, pointToString (p));
}

//-----------------------------------------------------------------------------
bool UIAttributes::getPointAttribute (const std::string& name, CPoint& p) const
{
	if (auto str = getAttributeValue (name))
		return stringToPoint (*str, p);
	return false;
}

//-----------------------------------------------------------------------------
void UIAttributes::setRectAttribute (const std::string& name, const CRect& r)
{
	setAttribute (name, rectToString (r));
}

//-----------------------------------------------------------------------------
bool UIAttributes::getRectAttribute (const std::string& name, CRect& r) const
{
	if (auto str = getAttributeValue (name))
		return stringToRect (*str, r);
	return false;
}

//-----------------------------------------------------------------------------
void UIAttributes::setStringArrayAttribute (const std::string& name, const StringArray& values)
{
	setAttribute (name, stringArrayToString (values));
}

//-----------------------------------------------------------------------------
bool UIAttributes::getStringArrayAttribute (const std::string& name, StringArray& values) const
{
	if (auto str = getAttributeValue (name))
		return stringToStringArray(*str, values);
	return false;
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
