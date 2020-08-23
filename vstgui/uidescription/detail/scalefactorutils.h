// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

namespace VSTGUI {
namespace Detail {

//-----------------------------------------------------------------------------
template <bool nameHasExtension, size_t numIndicators>
inline std::pair<size_t, size_t> rangeOfScaleFactor (const std::string& name,
                                                     const char (&identicator)[numIndicators])
{
	auto result = std::make_pair (std::string::npos, std::string::npos);
	size_t xIndex;
	if (nameHasExtension)
	{
		xIndex = name.rfind ("x.");
	}
	else
	{
		if (name[name.size () - 1] != 'x')
			return result;
		xIndex = name.size () - 1;
	}
	if (xIndex == std::string::npos)
		return result;

	for (auto i = 0u; i < numIndicators; ++i)
	{
		size_t indicatorIndex = name.find_last_of (identicator[i]);
		if (indicatorIndex == std::string::npos)
			continue;
		if (xIndex < indicatorIndex)
			continue;
		result.first = xIndex;
		result.second = indicatorIndex;
		break;
	}
	return result;
}

//-----------------------------------------------------------------------------
template <size_t numIndicators>
inline bool decodeScaleFactorFromName (const std::string& name,
                                       const char (&identicator)[numIndicators],
                                       double& scaleFactor)
{
	auto range = rangeOfScaleFactor<true> (name, identicator);
	if (range.first == std::string::npos)
		return false;
	std::string tmp (name);
	tmp.erase (0, ++range.second);
	tmp.erase (range.first - range.second);
	scaleFactor = UTF8StringView (tmp.c_str ()).toDouble ();
	return scaleFactor != 0;
}

//-----------------------------------------------------------------------------
static constexpr const char scaleFactorIndicatorChars[] = "#_";

//-----------------------------------------------------------------------------
inline bool decodeScaleFactorFromName (const std::string& name, double& scaleFactor)
{
	if (!decodeScaleFactorFromName (name, scaleFactorIndicatorChars, scaleFactor))
		return false;
	return true;
}

//-----------------------------------------------------------------------------
inline std::string removeScaleFactorFromName (const std::string& name)
{
	auto range = rangeOfScaleFactor<false> (name, scaleFactorIndicatorChars);
	if (range.first == std::string::npos)
		return "";
	auto result = name.substr (0, range.second);
	return result;
}

} // Detail
} // VSTGUI
