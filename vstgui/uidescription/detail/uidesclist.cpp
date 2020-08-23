// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../uiattributes.h"
#include "uidesclist.h"
#include "uinode.h"

namespace VSTGUI {
namespace Detail {

//-----------------------------------------------------------------------------
UIDescList::UIDescList (bool ownsObjects) : ownsObjects (ownsObjects)
{
}

//------------------------------------------------------------------------
UIDescList::UIDescList (const UIDescList& uiDesc) : ownsObjects (false)
{
	for (auto& child : uiDesc)
		add (child);
}

//-----------------------------------------------------------------------------
UIDescList::~UIDescList () noexcept
{
	removeAll ();
}

//-----------------------------------------------------------------------------
void UIDescList::add (UINode* obj)
{
	if (!ownsObjects)
		obj->remember ();
	UIDescListContainerType::emplace_back (obj);
}

//-----------------------------------------------------------------------------
void UIDescList::remove (UINode* obj)
{
	UIDescListContainerType::iterator pos =
	    std::find (UIDescListContainerType::begin (), UIDescListContainerType::end (), obj);
	if (pos != UIDescListContainerType::end ())
	{
		UIDescListContainerType::erase (pos);
		obj->forget ();
	}
}

//-----------------------------------------------------------------------------
void UIDescList::removeAll ()
{
	for (const_reverse_iterator it = rbegin (), end = rend (); it != end; ++it)
		(*it)->forget ();
	clear ();
}

//-----------------------------------------------------------------------------
UINode* UIDescList::findChildNode (UTF8StringView nodeName) const
{
	for (const auto& node : *this)
	{
		auto& name = node->getName ();
		if (nodeName == UTF8StringView (name))
			return node;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
UINode* UIDescList::findChildNodeWithAttributeValue (const std::string& attributeName,
                                                     const std::string& attributeValue) const
{
	for (const auto& node : *this)
	{
		const std::string* attributeValuePtr =
		    node->getAttributes ()->getAttributeValue (attributeName);
		if (attributeValuePtr && *attributeValuePtr == attributeValue)
			return node;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
void UIDescList::sort ()
{
	std::sort (begin (), end (), [] (const UINode* n1, const UINode* n2) {
		const std::string* str1 = n1->getAttributes ()->getAttributeValue ("name");
		const std::string* str2 = n2->getAttributes ()->getAttributeValue ("name");
		if (str1 && str2)
			return *str1 < *str2;
		else if (str1)
			return true;
		return false;
	});
}

//------------------------------------------------------------------------
UIDescListWithFastFindAttributeNameChild::UIDescListWithFastFindAttributeNameChild ()
{
}

//------------------------------------------------------------------------
void UIDescListWithFastFindAttributeNameChild::add (UINode* obj)
{
	UIDescList::add (obj);
	const std::string* nameAttributeValue = obj->getAttributes ()->getAttributeValue ("name");
	if (nameAttributeValue)
		childMap.emplace (*nameAttributeValue, obj);
}

//------------------------------------------------------------------------
void UIDescListWithFastFindAttributeNameChild::remove (UINode* obj)
{
	const std::string* nameAttributeValue = obj->getAttributes ()->getAttributeValue ("name");
	if (nameAttributeValue)
	{
		ChildMap::iterator it = childMap.find (*nameAttributeValue);
		if (it != childMap.end ())
			childMap.erase (it);
	}
	UIDescList::remove (obj);
}

//------------------------------------------------------------------------
void UIDescListWithFastFindAttributeNameChild::removeAll ()
{
	childMap.clear ();
	UIDescList::removeAll ();
}

//------------------------------------------------------------------------
UINode* UIDescListWithFastFindAttributeNameChild::findChildNodeWithAttributeValue (
    const std::string& attributeName, const std::string& attributeValue) const
{
	if (attributeName != "name")
		return UIDescList::findChildNodeWithAttributeValue (attributeName, attributeValue);
	ChildMap::const_iterator it = childMap.find (attributeValue);
	if (it != childMap.end ())
		return it->second;
	return nullptr;
}

//------------------------------------------------------------------------
void UIDescListWithFastFindAttributeNameChild::nodeAttributeChanged (
    UINode* node, const std::string& attributeName, const std::string& oldAttributeValue)
{
	if (attributeName != "name")
		return;
	ChildMap::iterator it = childMap.find (oldAttributeValue);
	if (it != childMap.end ())
		childMap.erase (it);
	const std::string* nameAttributeValue = node->getAttributes ()->getAttributeValue ("name");
	if (nameAttributeValue)
		childMap.emplace (*nameAttributeValue, node);
}

//------------------------------------------------------------------------
} // Detail
} // VSTGUI
