// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
	explicit CLayeredViewContainer (const CRect& r = CRect (0, 0, 0, 0));
	~CLayeredViewContainer () noexcept override = default;
	
	IPlatformViewLayer* getPlatformLayer () const { return layer; }

	void setZIndex (uint32_t zIndex);
	uint32_t getZIndex () const { return zIndex; }
	
	bool removed (CView* parent) override;
	bool attached (CView* parent) override;
	void invalid () override;
	void invalidRect (const CRect& rect) override;
	void parentSizeChanged () override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	void setAlphaValue (float alpha) override;
//-----------------------------------------------------------------------------
protected:
	void drawRect (CDrawContext* pContext, const CRect& updateRect) override;
	void drawViewLayer (CDrawContext* context, const CRect& dirtyRect) override;
	void viewContainerTransformChanged (CViewContainer* container) override;
	void updateLayerSize ();
	CGraphicsTransform getDrawTransform () const;
	void registerListeners (bool state);

	SharedPointer<IPlatformViewLayer> layer;
	CLayeredViewContainer* parentLayerView {nullptr};
	uint32_t zIndex {0};
};

} // namespace

#endif // __clayeredviewcontainer__
