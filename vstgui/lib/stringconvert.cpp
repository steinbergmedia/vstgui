// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "stringconvert.h"
#include "vstguidebug.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
std::string toUTF8 (std::u16string_view str)
{
	std::string result;
	result.reserve (str.size () * 3); // Reserve approximate space
	for (size_t i = 0; i < str.size (); ++i)
	{
		char16_t high = str[i];
		uint32_t codepoint;

		// Check if this is a high surrogate
		if (high >= 0xD800 && high <= 0xDBFF)
		{
			if (i + 1 < str.size ())
			{
				char16_t low = str[i + 1];

				// Check if the next character is a valid low surrogate
				if (low >= 0xDC00 && low <= 0xDFFF)
				{
					// Valid surrogate pair: decode to codepoint
					codepoint =
						0x10000 + (((uint32_t)high - 0xD800) << 10) + ((uint32_t)low - 0xDC00);
					++i; // Skip the low surrogate
				}
				else
				{
					// Invalid: high surrogate not followed by low surrogate
					vstgui_assert (false, "invalid UTF-16 string");
					return {};
				}
			}
			else
			{
				// Invalid: high surrogate at end of string
				vstgui_assert (false, "invalid UTF-16 string");
				return {};
			}
		}
		else if (high >= 0xDC00 && high <= 0xDFFF)
		{
			// Invalid: lone low surrogate
			vstgui_assert (false, "invalid UTF-16 string");
			return {};
		}
		else
		{
			// Valid BMP character (no surrogate pair)
			codepoint = high;
		}

		// Encode the codepoint as UTF-8
		if (codepoint <= 0x7F)
		{
			// Single-byte character (0xxxxxxx)
			result.push_back (static_cast<char> (codepoint));
		}
		else if (codepoint <= 0x7FF)
		{
			// Two-byte character (110xxxxx 10xxxxxx)
			result.push_back (static_cast<char> (0xC0 | (codepoint >> 6)));
			result.push_back (static_cast<char> (0x80 | (codepoint & 0x3F)));
		}
		else if (codepoint <= 0xFFFF)
		{
			// Three-byte character (1110xxxx 10xxxxxx 10xxxxxx)
			result.push_back (static_cast<char> (0xE0 | (codepoint >> 12)));
			result.push_back (static_cast<char> (0x80 | ((codepoint >> 6) & 0x3F)));
			result.push_back (static_cast<char> (0x80 | (codepoint & 0x3F)));
		}
		else
		{
			// Four-byte character (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
			result.push_back (static_cast<char> (0xF0 | (codepoint >> 18)));
			result.push_back (static_cast<char> (0x80 | ((codepoint >> 12) & 0x3F)));
			result.push_back (static_cast<char> (0x80 | ((codepoint >> 6) & 0x3F)));
			result.push_back (static_cast<char> (0x80 | (codepoint & 0x3F)));
		}
	}
	result.shrink_to_fit ();
	return result;
}

//------------------------------------------------------------------------
std::string toUTF8 (std::u32string_view str)
{
	std::string result;
	result.reserve (str.size () * 4); // Reserve approximate space (worst case)

	for (char32_t codepoint : str)
	{
		uint32_t cp = static_cast<uint32_t> (codepoint);

		// Validate the codepoint
		if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
		{
			vstgui_assert (false, "invalid UTF-32 string");
			return {};
		}

		if (cp <= 0x7F)
		{
			// Single-byte character (0xxxxxxx)
			result.push_back (static_cast<char> (cp));
		}
		else if (cp <= 0x7FF)
		{
			// Two-byte character (110xxxxx 10xxxxxx)
			result.push_back (static_cast<char> (0xC0 | (cp >> 6)));
			result.push_back (static_cast<char> (0x80 | (cp & 0x3F)));
		}
		else if (cp <= 0xFFFF)
		{
			// Three-byte character (1110xxxx 10xxxxxx 10xxxxxx)
			result.push_back (static_cast<char> (0xE0 | (cp >> 12)));
			result.push_back (static_cast<char> (0x80 | ((cp >> 6) & 0x3F)));
			result.push_back (static_cast<char> (0x80 | (cp & 0x3F)));
		}
		else
		{
			// Four-byte character (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
			result.push_back (static_cast<char> (0xF0 | (cp >> 18)));
			result.push_back (static_cast<char> (0x80 | ((cp >> 12) & 0x3F)));
			result.push_back (static_cast<char> (0x80 | ((cp >> 6) & 0x3F)));
			result.push_back (static_cast<char> (0x80 | (cp & 0x3F)));
		}
	}
	result.shrink_to_fit ();
	return result;
}

//------------------------------------------------------------------------
std::u16string toUTF16 (std::string_view str)
{
	std::u16string result;
	result.reserve (str.size ()); // Reserve approximate space

	for (size_t i = 0; i < str.size (); ++i)
	{
		unsigned char byte = static_cast<unsigned char> (str[i]);
		uint32_t codepoint;
		size_t numBytes;

		// Determine the number of bytes in this UTF-8 sequence
		if ((byte & 0x80) == 0)
		{
			// Single-byte character (0xxxxxxx)
			codepoint = byte;
			numBytes = 1;
		}
		else if ((byte & 0xE0) == 0xC0)
		{
			// Two-byte character (110xxxxx)
			codepoint = byte & 0x1F;
			numBytes = 2;
		}
		else if ((byte & 0xF0) == 0xE0)
		{
			// Three-byte character (1110xxxx)
			codepoint = byte & 0x0F;
			numBytes = 3;
		}
		else if ((byte & 0xF8) == 0xF0)
		{
			// Four-byte character (11110xxx)
			codepoint = byte & 0x07;
			numBytes = 4;
		}
		else
		{
			// Invalid UTF-8 start byte
			vstgui_assert (false, "invalid UTF-8 string");
			return {};
		}

		// Decode continuation bytes
		for (size_t j = 1; j < numBytes; ++j)
		{
			if (i + j >= str.size ())
			{
				// Incomplete sequence
				vstgui_assert (false, "invalid UTF-8 string");
				return {};
			}

			unsigned char continuationByte = static_cast<unsigned char> (str[i + j]);

			// Check if this is a valid continuation byte (10xxxxxx)
			if ((continuationByte & 0xC0) != 0x80)
			{
				// Invalid continuation byte
				vstgui_assert (false, "invalid UTF-8 string");
				return {};
			}

			// Extract the 6 bits from this continuation byte
			codepoint = (codepoint << 6) | (continuationByte & 0x3F);
		}

		// Check for overlong encodings and invalid ranges
		if ((numBytes == 2 && codepoint < 0x80) || (numBytes == 3 && codepoint < 0x800) ||
			(numBytes == 4 && codepoint < 0x10000) || (codepoint > 0x10FFFF) ||
			(codepoint >= 0xD800 && codepoint <= 0xDFFF))
		{
			// Invalid codepoint
			vstgui_assert (false, "invalid UTF-8 string");
			return {};
		}

		// Encode the codepoint as UTF-16
		if (codepoint <= 0xFFFF)
		{
			// BMP character: fits in a single UTF-16 code unit
			result.push_back (static_cast<char16_t> (codepoint));
		}
		else
		{
			// Non-BMP character: encode as surrogate pair
			uint32_t cp = codepoint - 0x10000;
			char16_t high = static_cast<char16_t> (0xD800 + (cp >> 10));
			char16_t low = static_cast<char16_t> (0xDC00 + (cp & 0x3FF));
			result.push_back (high);
			result.push_back (low);
		}

		i += numBytes - 1; // Skip the continuation bytes
	}

	return result;
}

//------------------------------------------------------------------------
std::u16string toUTF16 (std::u32string_view str)
{
	std::u16string result;
	result.reserve (str.size () * 2); // Reserve approximate space (worst case)
	for (char32_t codepoint : str)
	{
		uint32_t cp = static_cast<uint32_t> (codepoint);

		// Validate the codepoint
		if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
		{
			// Invalid codepoint
			vstgui_assert (false, "invalid UTF-32 string");
			return {};
		}

		if (cp <= 0xFFFF)
		{
			// BMP character: fits in a single UTF-16 code unit
			result.push_back (static_cast<char16_t> (cp));
		}
		else
		{
			// Non-BMP character: encode as surrogate pair
			cp -= 0x10000;
			char16_t high = static_cast<char16_t> (0xD800 + (cp >> 10));
			char16_t low = static_cast<char16_t> (0xDC00 + (cp & 0x3FF));
			result.push_back (high);
			result.push_back (low);
		}
	}
	result.shrink_to_fit ();
	return result;
}

//------------------------------------------------------------------------
std::u32string toUTF32 (std::string_view str)
{
	std::u32string result;
	result.reserve (str.size ()); // Reserve approximate space

	for (size_t i = 0; i < str.size (); ++i)
	{
		unsigned char byte = static_cast<unsigned char> (str[i]);
		uint32_t codepoint;
		size_t numBytes;

		// Determine the number of bytes in this UTF-8 sequence
		if ((byte & 0x80) == 0)
		{
			// Single-byte character (0xxxxxxx)
			codepoint = byte;
			numBytes = 1;
		}
		else if ((byte & 0xE0) == 0xC0)
		{
			// Two-byte character (110xxxxx)
			codepoint = byte & 0x1F;
			numBytes = 2;
		}
		else if ((byte & 0xF0) == 0xE0)
		{
			// Three-byte character (1110xxxx)
			codepoint = byte & 0x0F;
			numBytes = 3;
		}
		else if ((byte & 0xF8) == 0xF0)
		{
			// Four-byte character (11110xxx)
			codepoint = byte & 0x07;
			numBytes = 4;
		}
		else
		{
			vstgui_assert (false, "invalid UTF-8 string");
			return {};
		}

		// Decode continuation bytes
		for (size_t j = 1; j < numBytes; ++j)
		{
			if (i + j >= str.size ())
			{
				// Incomplete sequence
				vstgui_assert (false, "invalid UTF-8 string");
				return {};
			}

			unsigned char continuationByte = static_cast<unsigned char> (str[i + j]);

			// Check if this is a valid continuation byte (10xxxxxx)
			if ((continuationByte & 0xC0) != 0x80)
			{
				// Invalid continuation byte
				vstgui_assert (false, "invalid UTF-8 string");
				return {};
			}

			// Extract the 6 bits from this continuation byte
			codepoint = (codepoint << 6) | (continuationByte & 0x3F);
		}

		// Check for overlong encodings and invalid ranges
		if ((numBytes == 2 && codepoint < 0x80) || (numBytes == 3 && codepoint < 0x800) ||
			(numBytes == 4 && codepoint < 0x10000) || (codepoint > 0x10FFFF) ||
			(codepoint >= 0xD800 && codepoint <= 0xDFFF))
		{
			// Invalid codepoint
			vstgui_assert (false, "invalid UTF-8 string");
			return {};
		}

		result.push_back (static_cast<char32_t> (codepoint));
		i += numBytes - 1; // Skip the continuation bytes
	}

	return result;
}

//------------------------------------------------------------------------
std::u32string toUTF32 (std::u16string_view str)
{
	std::u32string result;
	result.reserve (str.size ()); // Reserve approximate space

	for (size_t i = 0; i < str.size (); ++i)
	{
		char16_t high = str[i];
		uint32_t codepoint;

		// Check if this is a high surrogate
		if (high >= 0xD800 && high <= 0xDBFF)
		{
			if (i + 1 < str.size ())
			{
				char16_t low = str[i + 1];
				// Check if the next character is a valid low surrogate
				if (low >= 0xDC00 && low <= 0xDFFF)
				{
					// Valid surrogate pair: decode to codepoint
					codepoint =
						0x10000 + (((uint32_t)high - 0xD800) << 10) + ((uint32_t)low - 0xDC00);
					++i; // Skip the low surrogate
				}
				else
				{
					// Invalid: high surrogate not followed by low surrogate
					vstgui_assert (false, "invalid UTF-16 string");
					return {};
				}
			}
			else
			{
				// Invalid: high surrogate at end of string
				vstgui_assert (false, "invalid UTF-16 string");
				return {};
			}
		}
		else if (high >= 0xDC00 && high <= 0xDFFF)
		{
			// Invalid: lone low surrogate
			vstgui_assert (false, "invalid UTF-16 string");
			return {};
		}
		else
		{
			// Valid BMP character (no surrogate pair)
			codepoint = high;
		}
		result.push_back (static_cast<char32_t> (codepoint));
	}

	return result;
}

//------------------------------------------------------------------------
} // VSTGUI
