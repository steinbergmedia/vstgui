// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uiattributes__
#define __uiattributes__

#include "../lib/vstguifwd.h"
#include "../lib/cstring.h"

#include <vector>
#include "../lib/platform/std_unorderedmap.h"

namespace VSTGUI {
class OutputStream;
class InputStream;

using UIAttributesMap = std::unordered_map<std::string,std::string>;

//-----------------------------------------------------------------------------
class UIAttributes : public NonAtomicReferenceCounted, private UIAttributesMap
{
public:
	using StringArray = std::vector<std::string>;
	
	explicit UIAttributes (UTF8StringPtr* attributes = nullptr);
	~UIAttributes () noexcept override = default;

	using UIAttributesMap::begin;
	using UIAttributesMap::end;
	using UIAttributesMap::iterator;
	using UIAttributesMap::const_iterator;

	bool hasAttribute (const std::string& name) const;
	const std::string* getAttributeValue (const std::string& name) const;
	void setAttribute (const std::string& name, const std::string& value);
	void setAttribute (const std::string& name, std::string&& value);
	void setAttribute (std::string&& name, std::string&& value);
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
