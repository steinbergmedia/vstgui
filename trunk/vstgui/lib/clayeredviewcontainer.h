//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins :
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

#ifndef __clayeredviewcontainer__
#define __clayeredviewcontainer__

#include "cviewcontainer.h"
#include "iviewlistener.h"
#include "platform/iplatformviewlayer.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CLayeredViewContainer Declaration
//! @brief a view container which draws into a platform layer on top of a parent layer or the platform view
//! @ingroup containerviews
//! @ingroup new_in_4_2
//! A CLayeredViewContainer creates a platform layer on top of a parent layer or the platform view of CFrame
//! if available on that platform and draws into it, otherwise it acts exactly like a CViewContainer
//-----------------------------------------------------------------------------
class CLayeredViewContainer : public CViewContainer, public IPlatformViewLayerDelegate, public IViewContainerListenerAdapter
{
public:
	CLayeredViewContainer (const CRect& r = CRect (0, 0, 0, 0));
	~CLayeredViewContainer ();
	
	IPlatformViewLayer* getPlatformLayer () const { return layer; }

	void setZIndex (uint32_t zIndex);
	uint32_t getZIndex () const { return zIndex; }
	
	bool removed (CView* parent) VSTGUI_OVERRIDE_VMETHOD;
	bool attached (CView* parent) VSTGUI_OVERRIDE_VMETHOD;
	void invalid () VSTGUI_OVERRIDE_VMETHOD;
	void invalidRect (const CRect& rect) VSTGUI_OVERRIDE_VMETHOD;
	void parentSizeChanged () VSTGUI_OVERRIDE_VMETHOD;
	void setViewSize (const CRect& rect, bool invalid = true) VSTGUI_OVERRIDE_VMETHOD;
	void setAlphaValue (float alpha) VSTGUI_OVERRIDE_VMETHOD;
//-----------------------------------------------------------------------------
protected:
	void drawRect (CDrawContext* pContext, const CRect& updateRect) VSTGUI_OVERRIDE_VMETHOD;
	void drawViewLayer (CDrawContext* context, const CRect& dirtyRect) VSTGUI_OVERRIDE_VMETHOD;
	void viewContainerTransformChanged (CViewContainer* container) VSTGUI_OVERRIDE_VMETHOD;
	void updateLayerSize ();
	CGraphicsTransform getDrawTransform () const;
	void registerListeners (bool state);

	OwningPointer<IPlatformViewLayer>layer;
	CLayeredViewContainer* parentLayerView;
	uint32_t zIndex;
};

} // namespace

#endif // __clayeredviewcontainer__
