// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../lib/cstring.h"
#include "../../lib/optional.h"
#include "interface.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
enum class CommonDirectoryLocation
{
	AppPath,
	AppPreferencesPath,
	AppCachesPath,
	UserDocumentsPath,
};

//------------------------------------------------------------------------
class ICommonDirectories : public Interface
{
public:
	/** Get a common directory.
	 *
	 *	@param location the location of the directory
	 *	@param subDir optional sub directory
	 *	@param create create directory if it does not exist
	 *	@return If location does exist the string is the path to the directory with the last
	 *			character the path separator.
	 */
	virtual Optional<UTF8String> get (CommonDirectoryLocation location,
	                                  const UTF8String& subDir = "", bool create = false) const = 0;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
