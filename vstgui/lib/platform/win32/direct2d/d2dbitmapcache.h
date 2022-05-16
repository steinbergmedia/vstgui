// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "d2dbitmap.h"

#if WINDOWS

struct ID2D1Device;

namespace VSTGUI {
namespace D2DBitmapCache {

void init ();
void terminate ();

ID2D1Bitmap* getBitmap (D2DBitmap* bitmap, ID2D1RenderTarget* renderTarget,
						ID2D1Device* device = nullptr);

void removeBitmap (D2DBitmap* bitmap);
void removeRenderTarget (ID2D1RenderTarget* renderTarget);
void removeDevice (ID2D1Device* device);

} // D2DBitmapCache
} // namespace VSTGUI

#endif // WINDOWS
