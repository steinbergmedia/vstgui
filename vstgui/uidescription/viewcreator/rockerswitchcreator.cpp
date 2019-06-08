// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "rockerswitchcreator.h"

#include "../../lib/controls/cswitch.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
RockerSwitchCreator::RockerSwitchCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr RockerSwitchCreator::getViewName () const
{
	return kCRockerSwitch;
}

//------------------------------------------------------------------------
IdStringPtr RockerSwitchCreator::getBaseViewName () const
{
	return kCControl;
}

//------------------------------------------------------------------------
UTF8StringPtr RockerSwitchCreator::getDisplayName () const
{
	return "Rocker Switch";
}

//------------------------------------------------------------------------
CView* RockerSwitchCreator::create (const UIAttributes& attributes,
                                    const IUIDescription* description) const
{
	return new CRockerSwitch (CRect (0, 0, 0, 0), nullptr, -1, nullptr);
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
