// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <type_traits>
#include <utility>

//-----------------------------------------------------------------------------
// VSTGUI Version
//-----------------------------------------------------------------------------
#define VSTGUI_VERSION_MAJOR  4
#define VSTGUI_VERSION_MINOR  15
#define VSTGUI_VERSION_PATCHLEVEL  0

//-----------------------------------------------------------------------------
// Platform definitions
//-----------------------------------------------------------------------------
#if __APPLE_CC__
	#ifndef __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES
		#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
	#endif
	#include <AvailabilityMacros.h>
	#include <TargetConditionals.h>
	#if TARGET_OS_IPHONE
		#ifndef MAC
			#define MAC 1
		#endif
		#ifndef MAC_COCOA
			#define MAC_COCOA 1
		#endif
		#define VSTGUI_OPENGL_SUPPORT 0	// there's an implementation, but not yet tested, so this is zero
		#define VSTGUI_TOUCH_EVENT_HANDLING 1
	#else
		#ifndef MAC_OS_X_VERSION_10_9
			#error you need at least OSX SDK 10.9 to build vstgui
		#endif
		#ifndef MAC_COCOA
			#define MAC_COCOA 1
		#endif
		#ifndef MAC
			#define MAC 1
		#endif
	#endif

	#ifndef __has_feature
		#error compiler not supported
	#endif
	#if __has_feature (cxx_rvalue_references) == 0
		#error need cxx_rvalue_references support from compiler
	#endif
	#if __has_feature (cxx_range_for) == 0
		#error need cxx_range_for support from compiler
	#endif

	#if defined (__clang__) && __clang_major__ > 4
		#if defined (VSTGUI_WARN_EVERYTHING) && VSTGUI_WARN_EVERYTHING == 1
			#pragma clang diagnostic warning "-Weverything"
			#pragma clang diagnostic warning "-Wconversion"
			#pragma clang diagnostic ignored "-Wreorder"
		#else
			#pragma clang diagnostic warning "-Wunreachable-code"
			#if __clang_major__ > 7
				#pragma clang diagnostic warning "-Wshadow"
			#endif
		#endif
	#endif

#elif WIN32 || WINDOWS || defined(_WIN32)
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif

	#include <sdkddkver.h>
	#if _WIN32_WINNT < 0x601
		#error unsupported Platform SDK you need at least the Windows 7 Platform SDK to compile VSTGUI
	#endif
	#ifdef __GNUC__
		#if __cplusplus < 201103L
			#error compiler not supported
		#endif
	#elif _MSC_VER <= 1800
		#error Visual Studio 2015 or newer needed
	#endif
	#ifndef WINDOWS
		#define WINDOWS 1
	#endif
	#if (defined(_M_ARM64) || defined(_M_ARM))
		#if defined(VSTGUI_OPENGL_SUPPORT)
			#undef VSTGUI_OPENGL_SUPPORT
		#endif
		#define VSTGUI_OPENGL_SUPPORT 0
	#endif
	#ifdef _MSC_VER
		#pragma warning(3 : 4189) // local variable is initialized but not referenced
		#pragma warning(3 : 4702) // unreachable code
		#pragma warning(3 : 4995) // deprecated
		#pragma warning(3 : 4431) // missing type specifier - int assumed. Note: C no longer supports default-int
		#pragma warning(3 : 4254) // conversion from 'type1' to 'type2', possible loss of data
		#pragma warning(3 : 4388) // signed/unsigned mismatch
		#pragma warning(disable : 4250) // class1' : inherits 'class2::member' via dominance
	#endif

	#if defined (__clang__) && __clang__
		#if defined (VSTGUI_WARN_EVERYTHING) && VSTGUI_WARN_EVERYTHING == 1
			#pragma clang diagnostic warning "-Wconversion"
			#pragma clang diagnostic ignored "-Wreorder"
		#else
			#pragma clang diagnostic warning "-Wunreachable-code"
		#endif
	#endif

	#include <algorithm>
	using std::min;
	using std::max;

#elif defined(__linux__)
    #include <algorithm>
    #include <climits>
    using std::min;
    using std::max;
	#ifndef LINUX
		#define LINUX 1
	#endif

#else
#error unsupported system/compiler
#endif

#ifdef UNICODE
	#undef UNICODE
#endif
#define UNICODE 1

//----------------------------------------------------
// Deprecation setting
//----------------------------------------------------
#ifndef VSTGUI_ENABLE_DEPRECATED_METHODS
	#define VSTGUI_ENABLE_DEPRECATED_METHODS 1
#endif

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	#define VSTGUI_DEPRECATED(x)			[[deprecated]]	x
	#define VSTGUI_DEPRECATED_MSG(x, msg)	[[deprecated(msg)]]	x
#else
	#define VSTGUI_DEPRECATED(x)
	#define VSTGUI_DEPRECATED_MSG(x, msg)
#endif

//----------------------------------------------------
// Feature setting
//----------------------------------------------------
#ifndef VSTGUI_OPENGL_SUPPORT
	#define VSTGUI_OPENGL_SUPPORT 1
#endif

#ifndef VSTGUI_TOUCH_EVENT_HANDLING
	#define VSTGUI_TOUCH_EVENT_HANDLING 0
#endif

#ifndef VSTGUI_ENABLE_XML_PARSER
	#define VSTGUI_ENABLE_XML_PARSER 1
#endif

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	#define VSTGUI_OVERRIDE_VMETHOD	override
#define VSTGUI_FINAL_VMETHOD final
#else
	#define VSTGUI_OVERRIDE_VMETHOD	static_assert (false, "VSTGUI_OVERRIDE_VMETHOD is deprecated, just use override!");
	#define VSTGUI_FINAL_VMETHOD static_assert (false, "VSTGUI_FINAL_VMETHOD is deprecated, just use final!");
#endif

//----------------------------------------------------
// Helper makros
//----------------------------------------------------
#define	VSTGUI_MAKE_STRING_PRIVATE_DONT_USE(x)	# x
#define	VSTGUI_MAKE_STRING(x)					VSTGUI_MAKE_STRING_PRIVATE_DONT_USE(x)

//----------------------------------------------------
#if DEVELOPMENT
	#ifndef DEBUG
		#define DEBUG	1
	#endif
#else
	#if !defined(NDEBUG) && !defined(DEBUG)
		#define NDEBUG	1
	#endif
#endif

//----------------------------------------------------
#define CLASS_METHODS(name, parent) CBaseObject* newCopy () const override { return new name (*this); }
#define CLASS_METHODS_NOCOPY(name, parent) CBaseObject* newCopy () const override { return nullptr; }
#define CLASS_METHODS_VIRTUAL(name, parent) CBaseObject* newCopy () const override = 0;

//----------------------------------------------------
namespace VSTGUI {

/** coordinate type */
using CCoord = double;
/** ID String pointer */
using IdStringPtr = const char*;
/** UTF8 String pointer */
using UTF8StringPtr = const char*;
/** UTF8 String buffer pointer */
using UTF8StringBuffer = char*;

//-----------------------------------------------------------------------------
// @brief Byte Order
//-----------------------------------------------------------------------------
enum ByteOrder
{
	kBigEndianByteOrder = 0,
	kLittleEndianByteOrder,
#if WINDOWS || defined(__LITTLE_ENDIAN__) || defined(__LITTLE_ENDIAN)
	kNativeByteOrder = kLittleEndianByteOrder
#else
	kNativeByteOrder = kBigEndianByteOrder
#endif
};

//-----------------------------------------------------------------------------
// @brief Message Results
//-----------------------------------------------------------------------------
enum CMessageResult
{
	kMessageUnknown = 0,
	kMessageNotified = 1
};

//------------------------------------------------------------------------
template <typename T, typename B>
inline void setBit (T& storage, B bit, bool state)
{
	static_assert (std::is_integral<T>::value, "only works for integral types");
	static_assert (sizeof (T) >= sizeof (B), "bit type is too big");
	if (state)
		storage |= static_cast<T> (bit);
	else
		storage &= ~(static_cast<T> (bit));
}

//------------------------------------------------------------------------
template <typename T, typename B>
inline constexpr bool hasBit (T storage, B bit)
{
	static_assert (std::is_integral<T>::value, "only works for integral types");
	static_assert (sizeof (T) >= sizeof (B), "bit type is too big");
	return (storage & static_cast<T> (bit)) ? true : false;
}

//-----------------------------------------------------------------------------
template <typename T, typename B>
struct BitScopeToggleT
{
	BitScopeToggleT (T& storage, B bit) : storage (storage), bit (bit) { toggle (); }
	~BitScopeToggleT () noexcept { toggle (); }

private:
	void toggle ()
	{
		bool state = hasBit (storage, bit);
		setBit (storage, bit, !state);
	}
	T& storage;
	B bit;
};

//-----------------------------------------------------------------------------
#define VSTGUI_NEWER_THAN(major, minor) \
	(VSTGUI_VERSION > major || (VSTGUI_VERSION_MAJOR == major && VSTGUI_VERSION_MINOR > minor))

#define VSTGUI_NEWER_THAN_4_10 VSTGUI_NEWER_THAN (4, 10)
#define VSTGUI_NEWER_THAN_4_11 VSTGUI_NEWER_THAN (4, 11)
#define VSTGUI_NEWER_THAN_4_12 VSTGUI_NEWER_THAN (4, 12)
#define VSTGUI_NEWER_THAN_4_13 VSTGUI_NEWER_THAN (4, 13)

} // VSTGUI

//-----------------------------------------------------------------------------
#include "sharedptr.h"
#include "vstguidebug.h"
