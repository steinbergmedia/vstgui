// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <string>
#include <string_view>

//------------------------------------------------------------------------
namespace VSTGUI {

/** @name Unicode String Conversion
 *
 *  Functions for converting between UTF-8, UTF-16 and UTF-32 encoded strings.
 *  Invalid input generates an empty string.
 */
//@{
/** @brief Convert a UTF-16 string to UTF-8
 *
 *  Converts the given UTF-16 string view into a newly created UTF-8 encoded std::string.
 *  @param str UTF-16 input string view
 *  @return UTF-8 encoded std::string
 */
std::string toUTF8 (std::u16string_view str);
/** @brief Convert a UTF-32 string to UTF-8
 *
 *  Converts the given UTF-32 string view into a newly created UTF-8 encoded std::string.
 *  @param str UTF-32 input string view
 *  @return UTF-8 encoded std::string
 */
std::string toUTF8 (std::u32string_view str);
/** @brief Convert a UTF-8 string to UTF-16
 *
 *  Converts the given UTF-8 string view into a newly created UTF-16 encoded std::u16string.
 *  @param str UTF-8 input string view
 *  @return UTF-16 encoded std::u16string
 */
std::u16string toUTF16 (std::string_view str);
/** @brief Convert a UTF-32 string to UTF-16
 *
 *  Converts the given UTF-32 string view into a newly created UTF-16 encoded std::u16string.
 *  @param str UTF-32 input string view
 *  @return UTF-16 encoded std::u16string
 */
std::u16string toUTF16 (std::u32string_view str);
/** @brief Convert a UTF-8 string to UTF-32
 *
 *  Converts the given UTF-8 string view into a newly created UTF-32 encoded std::u32string.
 *  @param str UTF-8 input string view
 *  @return UTF-32 encoded std::u32string
 */
std::u32string toUTF32 (std::string_view str);
/** @brief Convert a UTF-16 string to UTF-32
 *
 *  Converts the given UTF-16 string view into a newly created UTF-32 encoded std::u32string.
 *  @param str UTF-16 input string view
 *  @return UTF-32 encoded std::u32string
 */
std::u32string toUTF32 (std::u16string_view str);
//@}

//------------------------------------------------------------------------
} // VSTGUI
