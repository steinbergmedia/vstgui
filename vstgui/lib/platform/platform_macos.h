// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "iplatformframe.h"

#if MAC_COCOA

#ifdef __OBJC__
@class NSView, CALayer;
#else
struct NSView;
struct CALayer;
#endif

//-----------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
class CocoaFrameConfig : public IPlatformFrameConfig
{
public:
	enum Flags {
		kNoCALayer = 1 << 0,
	};
	uint32_t flags {0};
};

//-----------------------------------------------------------------------------
// extens IPlatformFrame on macOS
class ICocoaPlatformFrame
{
public:
	virtual NSView* getNSView () const = 0;
};

//-----------------------------------------------------------------------------
// extens IPlatformViewLayer on macOS
class ICocoaViewLayer
{
public:
	virtual CALayer* getCALayer () const = 0;
};

//-----------------------------------------------------------------------------
} // VSTGUI

#endif // MAC_COCOA

