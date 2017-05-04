// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __getpluginbundle__
#define __getpluginbundle__

//-----------------------------------------------------------------------------
/// @cond ignore
//-----------------------------------------------------------------------------
#if MAC
#include <dlfcn.h>
#include <CoreFoundation/CFBundle.h>
#include <string>

//-----------------------------------------------------------------------------
static CFBundleRef GetPluginBundle ()
{
	CFBundleRef pluginBundle = 0;
	Dl_info info;
	if (dladdr ((const void*)GetPluginBundle, &info))
	{
		if (info.dli_fname)
		{
			std::string name;
			name.assign (info.dli_fname);
			for (int i = 0; i < 3; i++)
			{
				int delPos = name.find_last_of ('/');
				if (delPos == -1)
				{
					fprintf (stdout, "Could not determine bundle location.\n");
					return 0; // unexpected
				}
				name.erase (delPos, name.length () - delPos);
			}
			CFURLRef bundleUrl = CFURLCreateFromFileSystemRepresentation (0, (const UInt8*)name.c_str (), name.length (), true);
			if (bundleUrl)
			{
				pluginBundle = CFBundleCreate (0, bundleUrl);
				CFRelease (bundleUrl);
			}
		}
	}
	return pluginBundle;
}

#endif

//-----------------------------------------------------------------------------
/// @endcond
//-----------------------------------------------------------------------------

#endif // __getpluginbundle__
