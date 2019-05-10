// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "uidescription.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
class CompressedUIDescription : public UIDescription
{
protected:
	enum CompressedSaveFlagBits
	{
		NoPlainXmlFileBackupBit = UIDescription::LastSaveFlagBit,
		ForceWriteCompressedDesc,
		LastCompressedSaveFlagBit,
	};
public:
	CompressedUIDescription (const CResourceDescription& compressedUIDescFile);

	enum SaveFlags
	{
		kNoPlainXmlFileBackup = 1 << NoPlainXmlFileBackupBit,
		kForceWriteCompressedDesc = 1 << ForceWriteCompressedDesc
	};

	bool parse () override;
	bool save (UTF8StringPtr filename, int32_t flags = kWriteWindowsResourceFile) override;

	bool getOriginalIsCompressed () const { return originalIsCompressed; }
	void setCompressionLevel (uint32_t level) { compressionLevel = level; }

private:
	bool parseWithStream (InputStream& stream);

	bool originalIsCompressed {false};
	uint32_t compressionLevel {1};
};

//------------------------------------------------------------------------
} // VSTGUI
