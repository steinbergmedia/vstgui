// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "cstring.h"
#include "idatapackage.h"
#include "optional.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
struct CClipboard
{
	/** get the global clipboard data */
	static SharedPointer<IDataPackage> get ();
	/** set the global clipboard data */
	static bool set (const SharedPointer<IDataPackage>& data);

	/** get the string from the global clipboard if it exists */
	static Optional<UTF8String> getString ();
	/** get the file path from the global clipboard if it exists */
	static Optional<UTF8String> getFilePath ();

	/** set the string of the global clipboard */
	static bool setString (UTF8StringPtr str);
	/** set the file path of the global clipboard */
	static bool setFilePath (UTF8StringPtr str);
};

} // VSTGUI
