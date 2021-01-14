// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguifwd.h"
#include "platformfwd.h"

//-----------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
/** Init the platform layer of VSTGUI.

	The instance is depended on the platform:
	- HINSTANCE on Windows
	- CFBundleRef on macOS
	- void* on Linux (the handle returned from dlopen)
 */
void initPlatform (PlatformInstanceHandle instance);

//-----------------------------------------------------------------------------
/** exit the platform layer of VSTGUI. */
void exitPlatform ();

//-----------------------------------------------------------------------------
/** get the global platform factory instance */
const IPlatformFactory& getPlatformFactory ();

//-----------------------------------------------------------------------------
class IPlatformFactory
{
public:
	using DataPackagePtr = SharedPointer<IDataPackage>;
	using COffscreenContextPtr = SharedPointer<COffscreenContext>;

	virtual ~IPlatformFactory () noexcept = default;

	/** Return platform ticks (millisecond resolution)
	 *	@return ticks
	 */
	virtual uint64_t getTicks () const noexcept = 0;

	/** Create a platform frame object
	 *	@param frame callback
	 *	@param size size
	 *	@param parent platform parent object
	 *	@param parentType type of platform parent object
	 *	@param config optional config object
	 *	@return platform frame or nullptr on failure
	 */
	virtual PlatformFramePtr createFrame (
		IPlatformFrameCallback* frame, const CRect& size, void* parent, PlatformType parentType,
		IPlatformFrameConfig* config = nullptr) const noexcept = 0;

	/** Create a platform font object
	 *	@param name name of the font
	 *	@param size font size
	 *	@param style font style
	 *	@return platform font or nullptr on failure
	 */
	virtual PlatformFontPtr createFont (const UTF8String& name, const CCoord& size,
										const int32_t& style) const noexcept = 0;
	/** Query all platform font families
	 *	@param callback callback called for every font
	 *	@return true on success
	 */
	virtual bool getAllFontFamilies (const FontFamilyCallback& callback) const noexcept = 0;

	/** Create an empty platform bitmap object
	 *	@param size size of the bitmap
	 *	@return platform bitmap or nullptr on failure
	 */
	virtual PlatformBitmapPtr createBitmap (const CPoint& size) const noexcept = 0;

	/** Create a platform bitmap object from a resource description
	 *	@param desc description where to find the bitmap
	 *	@return platform bitmap or nullptr on failure
	 */
	virtual PlatformBitmapPtr createBitmap (const CResourceDescription& desc) const noexcept = 0;

	/** Create a platform bitmap object from a file
	 *	@param absolutePath the absolute path of the bitmap file location
	 *	@return platform bitmap or nullptr on failure
	 */
	virtual PlatformBitmapPtr createBitmapFromPath (UTF8StringPtr absolutePath) const noexcept = 0;

	/** Create a platform bitmap object from memory
	 *	@param ptr memory location
	 *	@param memSize memory size
	 *	@return platform bitmap or nullptr on failure
	 */
	virtual PlatformBitmapPtr createBitmapFromMemory (const void* ptr,
													  uint32_t memSize) const noexcept = 0;
	/** Create a memory representation of the platform bitmap in PNG format.
	 *	@param bitmap the platform bitmap object
	 *	@return memory buffer containing the PNG representation of the bitmap
	 */
	virtual PNGBitmapBuffer
		createBitmapMemoryPNGRepresentation (const PlatformBitmapPtr& bitmap) const noexcept = 0;

	/** Create a platform resource input stream
	 *	@param desc description where to find the file to open
	 *	@return platform resource input stream or nullptr if not found
	 */
	virtual PlatformResourceInputStreamPtr
		createResourceInputStream (const CResourceDescription& desc) const noexcept = 0;

	/** Create a platform string object
	 *	@param utf8String optional initial UTF-8 encoded string
	 *	@return platform string object or nullptr on failure
	 */
	virtual PlatformStringPtr createString (UTF8StringPtr utf8String = nullptr) const noexcept = 0;

	/** Create a platform timer object
	 *	@param callback timer callback object
	 *	@return platform timer object or nullptr on failure
	 */
	virtual PlatformTimerPtr createTimer (IPlatformTimerCallback* callback) const noexcept = 0;

	/** Set clipboard data
	 *	@param data data to put on the clipboard
	 *	@return true on success
	 */
	virtual bool setClipboard (const DataPackagePtr& data) const noexcept = 0;

	/** Get clipboard data
	 *	@return data package pointer
	 */
	virtual DataPackagePtr getClipboard () const noexcept = 0;

	/** create an offscreen draw device
	 *	@param size the size of the bitmap where the offscreen renders to
	 *	@param scaleFactor the scale factor for drawing
	 *	@return an offscreen context object or nullptr on failure
	 */
	virtual COffscreenContextPtr
		createOffscreenContext (const CPoint& size, double scaleFactor = 1.) const noexcept = 0;

	virtual const LinuxFactory* asLinuxFactory () const noexcept = 0;
	virtual const MacFactory* asMacFactory () const noexcept = 0;
	virtual const Win32Factory* asWin32Factory () const noexcept = 0;
};

//-----------------------------------------------------------------------------
} // VSTGUI
