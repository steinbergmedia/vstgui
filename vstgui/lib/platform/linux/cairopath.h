// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../cgraphicspath.h"
#include "../iplatformgraphicspath.h"
#include "cairoutils.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Cairo {

//------------------------------------------------------------------------
class GraphicsPathFactory : public IPlatformGraphicsPathFactory
{
public:
	GraphicsPathFactory (const ContextHandle& cr);

	IPlatformGraphicsPathPtr createPath () override;
	IPlatformGraphicsPathPtr createTextPath (const PlatformFontPtr& font, UTF8StringPtr text) override;

private:
	ContextHandle context;
};

class GraphicsPath;
//------------------------------------------------------------------------
class Path : public CGraphicsPath
{
public:
	Path (const IPlatformGraphicsPathFactoryPtr& factory,
	      IPlatformGraphicsPathPtr&& path = nullptr) noexcept;
	~Path () noexcept;

	cairo_path_t* getPath (const ContextHandle& handle,
						   const CGraphicsTransform* alignTransform = nullptr);

	CGradient* createGradient (double color1Start, double color2Start, const CColor& color1,
							   const CColor& color2) override;

	bool hitTest (const CPoint& p, bool evenOddFilled = false,
				  CGraphicsTransform* transform = 0) override;
	CPoint getCurrentPosition () override;
	CRect getBoundingBox () override;

	void dirty () override;

//------------------------------------------------------------------------
private:
	bool ensurePathValid ();
	void makeGraphicsPath ();

	IPlatformGraphicsPathFactoryPtr factory;
	IPlatformGraphicsPathPtr path;
};

//------------------------------------------------------------------------
} // Cairo
} // VSTGUI
