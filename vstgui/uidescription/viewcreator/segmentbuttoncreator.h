// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iviewcreator.h"
#include "../uiattributes.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
struct SegmentButtonCreator : ViewCreatorAdapter
{
	SegmentButtonCreator ();
	IdStringPtr getViewName () const override;
	IdStringPtr getBaseViewName () const override;
	UTF8StringPtr getDisplayName () const override;
	CView* create (const UIAttributes& attributes,
	               const IUIDescription* description) const override;
	bool apply (CView* view, const UIAttributes& attributes,
	            const IUIDescription* description) const override;
	bool getAttributeNames (StringList& attributeNames) const override;
	AttrType getAttributeType (const string& attributeName) const override;
	bool getPossibleListValues (const string& attributeName,
	                            ConstStringPtrList& values) const override;
	bool getAttributeValue (CView* view, const string& attributeName, string& stringValue,
	                        const IUIDescription* desc) const override;

private:
	void updateSegmentCount (CSegmentButton* button, uint32_t numSegments) const;
	void updateSegments (CSegmentButton* button, const UIAttributes::StringArray& names) const;

	using SelectionModeStrings = std::array<string, 3>;
	static SelectionModeStrings& selectionModeStrings ();
};

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
