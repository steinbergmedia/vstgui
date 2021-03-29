// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../common/fileresourceinputstream.h"
#include "../iplatformfont.h"
#include "../iplatformframe.h"
#include "../iplatformframecallback.h"
#include "../iplatformresourceinputstream.h"
#include "../iplatformstring.h"
#include "../iplatformtimer.h"
#include "carbon/hiviewframe.h"
#include "cfontmac.h"
#include "cgbitmap.h"
#include "cgdrawcontext.h"
#include "cocoa/nsviewframe.h"
#include "ios/uiviewframe.h"
#include "macclipboard.h"
#include "macfactory.h"
#include "macglobals.h"
#include "macstring.h"
#include "mactimer.h"
#include <list>
#include <mach/mach_time.h>
#include <memory>

//-----------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
struct MacFactory::Impl
{
	struct mach_timebase_info timebaseInfo;
	CFBundleRef bundle {nullptr};
	bool useAsynchronousLayerDrawing {true};
};

//-----------------------------------------------------------------------------
MacFactory::MacFactory (CFBundleRef bundle)
{
	impl = std::unique_ptr<Impl> (new Impl);
	impl->bundle = bundle;
	mach_timebase_info (&impl->timebaseInfo);
}

//-----------------------------------------------------------------------------
CFBundleRef MacFactory::getBundle () const noexcept
{
	return impl->bundle;
}

//-----------------------------------------------------------------------------
void MacFactory::setUseAsynchronousLayerDrawing (bool state) const noexcept
{
	impl->useAsynchronousLayerDrawing = state;
}

//-----------------------------------------------------------------------------
bool MacFactory::getUseAsynchronousLayerDrawing () const noexcept
{
	return impl->useAsynchronousLayerDrawing;
}

//-----------------------------------------------------------------------------
uint64_t MacFactory::getTicks () const noexcept
{
	uint64_t absTime = mach_absolute_time ();
	auto d = ((absTime * impl->timebaseInfo.numer) / impl->timebaseInfo.denom) / 1000000;
	return d;
}

//-----------------------------------------------------------------------------
PlatformFramePtr MacFactory::createFrame (IPlatformFrameCallback* frame, const CRect& size,
										  void* parent, PlatformType parentType,
										  IPlatformFrameConfig* config) const noexcept
{
#if TARGET_OS_IPHONE
	return makeOwned<UIViewFrame> (frame, size, (__bridge UIView*)parent);
#else
#if MAC_CARBON
	if (platformType == PlatformType::kWindowRef || platformType == PlatformType::kDefaultNative)
		return makeOwned<HIViewFrame> (frame, size, reinterpret_cast<WindowRef> (parent));
#endif // MAC_CARBON
	return makeOwned<NSViewFrame> (frame, size, reinterpret_cast<NSView*> (parent), config);
#endif
}

//-----------------------------------------------------------------------------
PlatformFontPtr MacFactory::createFont (const UTF8String& name, const CCoord& size,
										const int32_t& style) const noexcept
{
	auto font = makeOwned<CoreTextFont> (name, size, style);
	if (font->getFontRef ())
		return std::move (font);
	return nullptr;
}

//-----------------------------------------------------------------------------
bool MacFactory::getAllFontFamilies (const FontFamilyCallback& callback) const noexcept
{
	return CoreTextFont::getAllFontFamilies (callback);
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr MacFactory::createBitmap (const CPoint& size) const noexcept
{
	return CGBitmap::create (&const_cast<CPoint&> (size));
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr MacFactory::createBitmap (const CResourceDescription& desc) const noexcept
{
	if (auto bitmap = makeOwned<CGBitmap> ())
	{
		if (bitmap->load (desc))
			return bitmap;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr MacFactory::createBitmapFromPath (UTF8StringPtr absolutePath) const noexcept
{
	return CGBitmap::createFromPath (absolutePath);
}

//-----------------------------------------------------------------------------
PlatformBitmapPtr MacFactory::createBitmapFromMemory (const void* ptr,
													  uint32_t memSize) const noexcept
{
	return CGBitmap::createFromMemory (ptr, memSize);
}

//-----------------------------------------------------------------------------
PNGBitmapBuffer
	MacFactory::createBitmapMemoryPNGRepresentation (const PlatformBitmapPtr& bitmap) const noexcept
{
	return CGBitmap::createMemoryPNGRepresentation (bitmap);
}

//-----------------------------------------------------------------------------
PlatformResourceInputStreamPtr
	MacFactory::createResourceInputStream (const CResourceDescription& desc) const noexcept
{
	if (desc.type == CResourceDescription::kIntegerType)
		return nullptr;
	if (auto bundle = getBundleRef ())
	{
		PlatformResourceInputStreamPtr result;
		CFStringRef cfStr = CFStringCreateWithCString (nullptr, desc.u.name, kCFStringEncodingUTF8);
		if (cfStr)
		{
			CFURLRef url = CFBundleCopyResourceURL (getBundleRef (), cfStr, nullptr, nullptr);
			if (url)
			{
				char filePath[PATH_MAX];
				if (CFURLGetFileSystemRepresentation (url, true, (UInt8*)filePath, PATH_MAX))
				{
					result = FileResourceInputStream::create (filePath);
				}
				CFRelease (url);
			}
			CFRelease (cfStr);
		}
		return result;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
PlatformStringPtr MacFactory::createString (UTF8StringPtr utf8String) const noexcept
{
	return makeOwned<MacString> (utf8String);
}

//-----------------------------------------------------------------------------
PlatformTimerPtr MacFactory::createTimer (IPlatformTimerCallback* callback) const noexcept
{
	return makeOwned<MacTimer> (callback);
}

//------------------------------------------------------------------------
bool MacFactory::setClipboard (const DataPackagePtr& data) const noexcept
{
#if TARGET_OS_IPHONE
	return false;
#else
	MacClipboard::setClipboard (data);
	return true;
#endif
}

//------------------------------------------------------------------------
auto MacFactory::getClipboard () const noexcept -> DataPackagePtr
{
#if TARGET_OS_IPHONE
	return nullptr;
#else
	return MacClipboard::createClipboardDataPackage ();
#endif
}

//------------------------------------------------------------------------
auto MacFactory::createOffscreenContext (const CPoint& size, double scaleFactor) const noexcept
	-> COffscreenContextPtr
{
	auto bitmap = makeOwned<CGBitmap> (size * scaleFactor);
	bitmap->setScaleFactor (scaleFactor);
	auto context = makeOwned<CGDrawContext> (bitmap);
	if (context->getCGContext ())
		return std::move (context);
	return nullptr;
}

//-----------------------------------------------------------------------------
const LinuxFactory* MacFactory::asLinuxFactory () const noexcept
{
	return nullptr;
}

//-----------------------------------------------------------------------------
const MacFactory* MacFactory::asMacFactory () const noexcept
{
	return this;
}

//-----------------------------------------------------------------------------
const Win32Factory* MacFactory::asWin32Factory () const noexcept
{
	return nullptr;
}

//-----------------------------------------------------------------------------
} // VSTGUI
