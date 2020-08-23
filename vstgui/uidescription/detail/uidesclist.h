// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../lib/cstring.h"
#include "../../lib/vstguibase.h"
#include <unordered_map>
#include <vector>

namespace VSTGUI {
namespace Detail {

class UINode;

using UIDescListContainerType = std::vector<UINode*>;
//-----------------------------------------------------------------------------
class UIDescList : public NonAtomicReferenceCounted, private UIDescListContainerType
{
public:
	using UIDescListContainerType::begin;
	using UIDescListContainerType::end;
	using UIDescListContainerType::rbegin;
	using UIDescListContainerType::rend;
	using UIDescListContainerType::iterator;
	using UIDescListContainerType::const_iterator;
	using UIDescListContainerType::const_reverse_iterator;
	using UIDescListContainerType::empty;
	using UIDescListContainerType::size;

	explicit UIDescList (bool ownsObjects = true);
	UIDescList (const UIDescList& uiDesc);
	~UIDescList () noexcept override;

	virtual void add (UINode* obj);
	virtual void remove (UINode* obj);
	virtual void removeAll ();
	virtual UINode* findChildNode (UTF8StringView nodeName) const;
	virtual UINode* findChildNodeWithAttributeValue (const std::string& attributeName,
	                                                 const std::string& attributeValue) const;

	virtual void nodeAttributeChanged (UINode* child, const std::string& attributeName,
	                                   const std::string& oldAttributeValue)
	{
	}

	void sort ();

protected:
	bool ownsObjects;
};

//-----------------------------------------------------------------------------
class UIDescListWithFastFindAttributeNameChild : public UIDescList
{
private:
	using ChildMap = std::unordered_map<std::string, UINode*>;

public:
	UIDescListWithFastFindAttributeNameChild ();

	void add (UINode* obj) override;
	void remove (UINode* obj) override;
	void removeAll () override;
	UINode* findChildNodeWithAttributeValue (const std::string& attributeName,
	                                         const std::string& attributeValue) const override;
	void nodeAttributeChanged (UINode* node, const std::string& attributeName,
	                           const std::string& oldAttributeValue) override;

private:
	ChildMap childMap;
};

} // Detail
} // VSTGUI
