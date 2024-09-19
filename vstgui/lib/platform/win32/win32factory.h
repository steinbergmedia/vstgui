// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../platformfactory.h"
#include "../../optional.h"
#include "../../cstring.h"

struct ID2D1Factory;
struct IDWriteFactory;
struct IWICImagingFactory;

//-----------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
namespace DirectComposition {
struct Factory;
}

//-----------------------------------------------------------------------------
class Win32Factory final : public IPlatformFactory
{
public:
	Win32Factory (HINSTANCE instance);
	~Win32Factory () noexcept override;

	HINSTANCE getInstance () const noexcept;
	void setResourceBasePath (const UTF8String& path) const noexcept;
	Optional<UTF8String> getResourceBasePath () const noexcept;

	void useD2DHardwareRenderer (bool state) const noexcept;
	bool useD2DHardwareRenderer () const noexcept;

	void useGenericTextEdit (bool state) const noexcept;
	bool useGenericTextEdit () const noexcept;

	ID2D1Factory* getD2DFactory () const noexcept;
	IWICImagingFactory* getWICImagingFactory () const noexcept;
	IDWriteFactory* getDirectWriteFactory () const noexcept;

	DirectComposition::Factory* getDirectCompositionFactory () const noexcept;
	PlatformGraphicsDeviceContextPtr createGraphicsDeviceContext (void* hwnd) const noexcept;

	/** disable the use of direct composition. must be called before anything else or the behaviour
	 * is undefined. */
	void disableDirectComposition () const noexcept;

	void finalize () noexcept final;

	/** Return platform ticks (millisecond resolution)
	 *	@return ticks
	 */
	uint64_t getTicks () const noexcept final;

	/** Create a platform frame object
	 *	@param frame callback
	 *	@param size size
	 *	@param parent platform parent object
	 *	@param parentType type of platform parent object
	 *	@param config optional config object
	 *	@return platform frame or nullptr on failure
	 */
	PlatformFramePtr createFrame (IPlatformFrameCallback* frame, const CRect& size, void* parent,
								  PlatformType parentType,
								  IPlatformFrameConfig* config = nullptr) const noexcept final;

	/** Create a platform font object
	 *	@param name name of the font
	 *	@param size font size
	 *	@param style font style
	 *	@return platform font or nullptr on failure
	 */
	PlatformFontPtr createFont (const UTF8String& name, const CCoord& size,
								const int32_t& style) const noexcept final;
	/** Query all platform font families
	 *	@param callback callback called for every font
	 *	@return true on success
	 */
	bool getAllFontFamilies (const FontFamilyCallback& callback) const noexcept final;

	/** Create an empty platform bitmap object
	 *	@param size size of the bitmap
	 *	@return platform bitmap or nullptr on failure
	 */
	PlatformBitmapPtr createBitmap (const CPoint& size) const noexcept final;
	/** Create a platform bitmap object from a resource description
	 *	@param desc description where to find the bitmap
	 *	@return platform bitmap or nullptr on failure
	 */
	PlatformBitmapPtr createBitmap (const CResourceDescription& desc) const noexcept final;
	/** Create a platform bitmap object from a file
	 *	@param absolutePath the absolute path of the bitmap file location
	 *	@return platform bitmap or nullptr on failure
	 */
	PlatformBitmapPtr createBitmapFromPath (UTF8StringPtr absolutePath) const noexcept final;
	/** Create a platform bitmap object from memory
	 *	@param ptr memory location
	 *	@param memSize memory size
	 *	@return platform bitmap or nullptr on failure
	 */
	PlatformBitmapPtr createBitmapFromMemory (const void* ptr,
											  uint32_t memSize) const noexcept final;
	/** Create a memory representation of the platform bitmap in PNG format.
	 *	@param bitmap the platform bitmap object
	 *	@return memory buffer containing the PNG representation of the bitmap
	 */
	PNGBitmapBuffer
		createBitmapMemoryPNGRepresentation (const PlatformBitmapPtr& bitmap) const noexcept final;

	/** Create a platform resource input stream
	 *	@param desc description where to find the file to open
	 *	@return platform resource input stream or nullptr if not found
	 */
	PlatformResourceInputStreamPtr
		createResourceInputStream (const CResourceDescription& desc) const noexcept final;

	/** Create a platform string object
	 *	@param utf8String optional initial UTF-8 encoded string
	 *	@return platform string object or nullptr on failure
	 */
	PlatformStringPtr createString (UTF8StringPtr utf8String = nullptr) const noexcept final;

	/** Create a platform timer object
	 *	@param callback timer callback object
	 *	@return platform timer object or nullptr on failure
	 */
	PlatformTimerPtr createTimer (IPlatformTimerCallback* callback) const noexcept final;

	/** Set clipboard data
	 *	@param data data to put on the clipboard
	 *	@return true on success
	 */
	bool setClipboard (const DataPackagePtr& data) const noexcept final;

	/** Get clipboard data
	 *	@return data package pointer
	 */
	DataPackagePtr getClipboard () const noexcept final;

	/** Create a platform gradient object
	 *	@return platform gradient object or nullptr on failure
	 */
	PlatformGradientPtr createGradient () const noexcept final;

	/** Create a platform file selector
	 *	@param style file selector style
	 *	@param frame frame
	 *	@return platform file selector or nullptr on failure
	 */
	PlatformFileSelectorPtr createFileSelector (PlatformFileSelectorStyle style,
												IPlatformFrame* frame) const noexcept;

	/** Get the graphics device factory
	 *
	 *	@return platform graphics device factory
	 */
	const IPlatformGraphicsDeviceFactory& getGraphicsDeviceFactory () const noexcept final;

	const IPlatformConcurrency& getConcurrency () const noexcept final;

	const LinuxFactory* asLinuxFactory () const noexcept final;
	const MacFactory* asMacFactory () const noexcept final;
	const Win32Factory* asWin32Factory () const noexcept final;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // VSTGUI
