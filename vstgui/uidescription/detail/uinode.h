// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../lib/vstguifwd.h"
#include "../uidescriptionfwd.h"
#include "../../lib/ccolor.h"
#include "uidesclist.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Detail {

//------------------------------------------------------------------------
namespace MainNodeNames {

static constexpr IdStringPtr kBitmap = "bitmaps";
static constexpr IdStringPtr kFont = "fonts";
static constexpr IdStringPtr kColor = "colors";
static constexpr IdStringPtr kControlTag = "control-tags";
static constexpr IdStringPtr kVariable = "variables";
static constexpr IdStringPtr kTemplate = "template";
static constexpr IdStringPtr kCustom = "custom";
static constexpr IdStringPtr kGradient = "gradients";

} // MainNodeNames

//-----------------------------------------------------------------------------
class UINode : public NonAtomicReferenceCounted
{
public:
	using DataStorage = std::string;

	UINode (const std::string& name, const SharedPointer<UIAttributes>& attributes = {},
	        bool needsFastChildNameAttributeLookup = false);
	UINode (const std::string& name, const SharedPointer<UIDescList>& children,
	        const SharedPointer<UIAttributes>& attributes = {});
	UINode (const UINode& n);
	~UINode () noexcept override;

	const std::string& getName () const { return name; }
	DataStorage& getData () { return data; }
	const DataStorage& getData () const { return data; }

	void setData (DataStorage&& newData);

	const SharedPointer<UIAttributes>& getAttributes () const { return attributes; }
	UIDescList& getChildren () const { return *children; }
	bool hasChildren () const;
	void childAttributeChanged (UINode* child, const char* attributeName,
	                            const char* oldAttributeValue);

	enum
	{
		kNoExport = 1 << 0
	};

	bool noExport () const { return hasBit (flags, kNoExport); }
	void noExport (bool state) { setBit (flags, kNoExport, state); }

	bool operator== (const UINode& n) const { return name == n.name; }

	void sortChildren ();
	virtual void freePlatformResources () {}

protected:
	std::string name;
	DataStorage data;
	SharedPointer<UIAttributes> attributes;
	SharedPointer<UIDescList> children;
	int32_t flags;
};

//-----------------------------------------------------------------------------
class UICommentNode : public UINode
{
public:
	explicit UICommentNode (const std::string& comment);
};

//-----------------------------------------------------------------------------
class UIVariableNode : public UINode
{
public:
	UIVariableNode (const std::string& name, const SharedPointer<UIAttributes>& attributes);

	enum Type
	{
		kNumber,
		kString,
		kUnknown
	};

	Type getType () const;
	double getNumber () const;
	const std::string& getString () const;

protected:
	Type type;
	double number;
};

//-----------------------------------------------------------------------------
class UIControlTagNode : public UINode
{
public:
	UIControlTagNode (const std::string& name, const SharedPointer<UIAttributes>& attributes);
	int32_t getTag ();
	void setTag (int32_t newTag);

	const std::string* getTagString () const;
	void setTagString (const std::string& str);

protected:
	int32_t tag;
};

//-----------------------------------------------------------------------------
class UIBitmapNode : public UINode
{
public:
	UIBitmapNode (const std::string& name, const SharedPointer<UIAttributes>& attributes);
	CBitmap* getBitmap (const std::string& pathHint);
	void setBitmap (UTF8StringPtr bitmapName);
	void setNinePartTiledOffset (const CRect* offsets);
	void invalidBitmap ();
	bool getFilterProcessed () const { return filterProcessed; }
	void setFilterProcessed () { filterProcessed = true; }
	bool getScaledBitmapsAdded () const { return scaledBitmapsAdded; }
	void setScaledBitmapsAdded () { scaledBitmapsAdded = true; }

	void createXMLData (const std::string& pathHint);
	void removeXMLData ();
	bool hasXMLData () const;

	void freePlatformResources () override;

protected:
	~UIBitmapNode () noexcept override;
	CBitmap* createBitmap (const std::string& str, CNinePartTiledDescription* partDesc) const;
	PlatformBitmapPtr createBitmapFromDataNode () const;
	static bool imagesEqual (IPlatformBitmap* b1, IPlatformBitmap* b2);
	UINode* dataNode () const;
	CBitmap* bitmap;
	bool filterProcessed;
	bool scaledBitmapsAdded;
};

//-----------------------------------------------------------------------------
class UIFontNode : public UINode
{
public:
	UIFontNode (const std::string& name, const SharedPointer<UIAttributes>& attributes);
	CFontRef getFont ();
	void setFont (CFontRef newFont);
	void setAlternativeFontNames (UTF8StringPtr fontNames);
	bool getAlternativeFontNames (std::string& fontNames);

	void freePlatformResources () override;

protected:
	~UIFontNode () noexcept override;
	CFontRef font;
};

//-----------------------------------------------------------------------------
class UIColorNode : public UINode
{
public:
	UIColorNode (const std::string& name, const SharedPointer<UIAttributes>& attributes);
	const CColor& getColor () const { return color; }
	void setColor (const CColor& newColor);

protected:
	CColor color;
};

//-----------------------------------------------------------------------------
class UIGradientNode : public UINode
{
public:
	UIGradientNode (const std::string& name, const SharedPointer<UIAttributes>& attributes);
	CGradient* getGradient ();
	void setGradient (CGradient* g);

	void freePlatformResources () override;

protected:
	SharedPointer<CGradient> gradient;
};

//------------------------------------------------------------------------
} // Detail
} // VSTGUI
