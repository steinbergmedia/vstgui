// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformoptionmenu.h"

#include "../../ccolor.h"
#include "../../cfont.h"
#include "../../iviewlistener.h"

#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
struct GenericOptionMenuTheme
{
	SharedPointer<CFontDesc> font {kSystemFont};
	CColor backgroundColor {MakeCColor (0x39, 0x3c, 0x3f, 252)};
	CColor selectedBackgroundColor {MakeCColor (200, 200, 200, 235)};
	CColor textColor {MakeCColor (255, 255, 255, 255)};
	CColor selectedTextColor {MakeCColor (0, 0, 0, 255)};
	CColor disabledTextColor {MakeCColor (150, 150, 150, 255)};
	CColor titleTextColor {MakeCColor (150, 150, 150, 255)};
	CColor separatorColor {MakeCColor (100, 100, 100, 255)};
	CPoint inset {6., 6.};
	uint32_t menuAnimationTime {240};
};

//------------------------------------------------------------------------
struct IGenericOptionMenuListener
{
	virtual ~IGenericOptionMenuListener () noexcept = default;

	virtual void optionMenuPopupStarted () = 0;
	virtual void optionMenuPopupStopped () = 0;
};

//------------------------------------------------------------------------
class GenericOptionMenu : public IPlatformOptionMenu, public ViewMouseListenerAdapter
{
public:
	GenericOptionMenu (CFrame* frame, CButtonState initialButtons,
	                   GenericOptionMenuTheme theme = {});
	~GenericOptionMenu () noexcept override;

	void setListener (IGenericOptionMenuListener* listener);

	void popup (COptionMenu* optionMenu, const Callback& callback) override;

private:
	void removeModalView (PlatformOptionMenuResult result);
	CMouseEventResult viewOnMouseDown (CView* view, CPoint pos, CButtonState buttons) override;
	CMouseEventResult viewOnMouseUp (CView* view, CPoint pos, CButtonState buttons) override;

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // VSTGUI
