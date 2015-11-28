//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins :
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __cshadowviewcontainer__
#define __cshadowviewcontainer__

#include "cviewcontainer.h"
#include "iviewlistener.h"
#include "iscalefactorchangedlistener.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CShadowViewContainer Declaration
//! @brief a view container which draws a shadow for it's subviews
/// @ingroup containerviews
/// @ingroup new_in_4_1
//-----------------------------------------------------------------------------
class CShadowViewContainer : public CViewContainer, public IScaleFactorChangedListener, public IViewContainerListenerAdapter
{
public:
	CShadowViewContainer (const CRect& size);
	CShadowViewContainer (const CShadowViewContainer& copy);
	~CShadowViewContainer ();

	//-----------------------------------------------------------------------------
	/// @name CShadowViewContainer Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setShadowOffset (const CPoint& offset);
	const CPoint& getShadowOffset () const { return shadowOffset; }
	
	virtual void setShadowIntensity (float intensity);
	float getShadowIntensity () const { return shadowIntensity; }

	virtual void setShadowBlurSize (double size);
	double getShadowBlurSize () const { return shadowBlurSize; }

	void invalidateShadow ();
	//@}

	// override
	bool removed (CView* parent) VSTGUI_OVERRIDE_VMETHOD;
	bool attached (CView* parent) VSTGUI_OVERRIDE_VMETHOD;
	void drawRect (CDrawContext* pContext, const CRect& updateRect) VSTGUI_OVERRIDE_VMETHOD;
	void drawBackgroundRect (CDrawContext* pContext, const CRect& _updateRect) VSTGUI_OVERRIDE_VMETHOD;
	void setViewSize (const CRect& rect, bool invalid = true) VSTGUI_OVERRIDE_VMETHOD;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;

	void onScaleFactorChanged (CFrame* frame) VSTGUI_OVERRIDE_VMETHOD;

	CLASS_METHODS(CShadowViewContainer, CViewContainer)
protected:
	void viewContainerViewAdded (CViewContainer* container, CView* view) VSTGUI_OVERRIDE_VMETHOD;
	void viewContainerViewRemoved (CViewContainer* container, CView* view) VSTGUI_OVERRIDE_VMETHOD;
	void viewContainerViewZOrderChanged (CViewContainer* container, CView* view) VSTGUI_OVERRIDE_VMETHOD;

	bool dontDrawBackground;
	CPoint shadowOffset;
	float shadowIntensity;
	double shadowBlurSize;
	double scaleFactorUsed;
};

} // namespace

#endif // __cshadowviewcontainer__
