// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "vstgui/lib/cresourcedescription.h"
#include "vstgui/lib/cstring.h"
#include "vstgui/uidescription/compresseduidescription.h"
#include <string>

//------------------------------------------------------------------------
#if MAC
#include <CoreFoundation/CoreFoundation.h>
namespace VSTGUI { void* gBundleRef = CFBundleGetMainBundle (); }
#elif WINDOWS
struct IUnknown;
#include <windows.h>
#include <Shlobj.h>
void* hInstance = nullptr;
#elif LINUX
namespace VSTGUI { void* soHandle = nullptr; }
#endif

using namespace VSTGUI;

//------------------------------------------------------------------------
void printAndTerminate (const char* msg)
{
	if (msg)
		printf ("%s\n", msg);
	exit (-1);
}

//------------------------------------------------------------------------
int main (int argv, char* argc[])
{
#if WINDOWS
	CoInitialize (nullptr);
#endif
	std::string inputPath;
	std::string outputPath;
	bool noCompression = false;
	uint32_t compressionLevel = 1;
	for (auto i = 0; i < argv; ++i)
	{
		UTF8StringView arg (argc[i]);
		if (arg == "-i")
		{
			if (++i >= argv)
				break;
			inputPath = argc[i];
		}
		else if (arg == "-o")
		{
			if (++i >= argv)
				break;
			outputPath = argc[i];
		}
		else if (arg == "-c")
		{
			if (++i >= argv)
				break;
			compressionLevel = static_cast<uint32_t> (UTF8StringView (argc[i]).toInteger ());
		}
		else if (arg == "--nocompression")
		{
			noCompression = true;
		}
	}
	if (inputPath.empty () || outputPath.empty ())
	{
		printAndTerminate ("No input or output path specified!");
	}
	printf ("Copy %s to %s%s\n", inputPath.data (), outputPath.data (),
	        noCompression ? " [uncompressed]" : "[compressed]");

	CompressedUIDescription uiDesc (CResourceDescription (inputPath.data ()));
	if (!uiDesc.parse ())
	{
		printAndTerminate ("Parsing failed!");
	}
	int32_t flags = UIDescription::kWriteImagesIntoUIDescFile;
	if (noCompression)
	{
		if (inputPath == outputPath && uiDesc.getOriginalIsCompressed () == false)
			return 0;

		if (!uiDesc.UIDescription::save (outputPath.data (), flags))
		{
			printAndTerminate ("saving failed");
		}
	}
	else
	{
		if (inputPath == outputPath && uiDesc.getOriginalIsCompressed () == true)
			return 0;

		flags |= CompressedUIDescription::kNoPlainUIDescFileBackup |
		         CompressedUIDescription::kForceWriteCompressedDesc |
		         CompressedUIDescription::kDoNotVerifyImageData;
		uiDesc.setCompressionLevel (compressionLevel);
		if (!uiDesc.save (outputPath.data (), flags))
		{
			printAndTerminate ("saving failed");
		}
	}
	return 0;
}
