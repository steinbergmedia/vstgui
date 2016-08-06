//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins :
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef __vstguibase__
#define __vstguibase__

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>

//-----------------------------------------------------------------------------
// VSTGUI Version
//-----------------------------------------------------------------------------
#define VSTGUI_VERSION_MAJOR  4
#define VSTGUI_VERSION_MINOR  5

//-----------------------------------------------------------------------------
// Platform definitions
//-----------------------------------------------------------------------------
#if __APPLE_CC__
	#ifndef __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES
		#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
	#endif
	#include <stdint.h>
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
		#ifndef MAC_OS_X_VERSION_10_7
			#error you need at least OSX SDK 10.7 to build vstgui
		#endif
		#ifndef MAC_OS_X_VERSION_10_8
			#define MAC_OS_X_VERSION_10_8 1080
		#endif
		#ifndef MAC_OS_X_VERSION_10_9
			#define MAC_OS_X_VERSION_10_9 1090
		#endif
		#ifndef MAC_COCOA
			#define MAC_COCOA 1
		#endif
		#ifndef MAC
			#define MAC 1
		#endif
		#if !__LP64__ && !defined (MAC_CARBON)
			#define MAC_CARBON 1
			#ifndef TARGET_API_MAC_CARBON
				#define TARGET_API_MAC_CARBON 1
			#endif
			#ifndef __CF_USE_FRAMEWORK_INCLUDES__
				#define __CF_USE_FRAMEWORK_INCLUDES__ 1
			#endif
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
	#include <type_traits>

	#if defined (__clang__) && __clang_major__ > 4
		#if defined (VSTGUI_WARN_EVERYTHING) && VSTGUI_WARN_EVERYTHING == 1
			#pragma clang diagnostic warning "-Weverything"
			#pragma clang diagnostic warning "-Wconversion"
			#pragma clang diagnostic ignored "-Wreorder"
		#else
			#pragma clang diagnostic warning "-Wunreachable-code"
		#endif
	#endif

#elif WIN32 || WINDOWS || defined(_WIN32)
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <sdkddkver.h>
	#if _WIN32_WINNT < 0x600
		#error unsupported Platform SDK you need at least the Vista Platform SDK to compile VSTGUI
	#endif
	#ifdef __GNUC__
		#if __cplusplus < 201103L
			#error compiler not supported
		#endif
	#elif _MSC_VER < 1800
		#error Visual Studio 2013 or newer needed
	#endif
	#include <type_traits>
	#include <stdint.h>
	#ifndef WINDOWS
		#define WINDOWS 1
	#endif
	#if !defined(__GNUC__) && _MSC_VER == 1800
		#define noexcept		// only supported since VS 2015
	#endif
	#define VSTGUI_DIRECT2D_SUPPORT	1
	#define DEPRECATED_ATTRIBUTE __declspec(deprecated)
	#pragma warning(3 : 4189) // local variable is initialized but not referenced
	#pragma warning(3 : 4702) // unreachable code
	#pragma warning(3 : 4995) // deprecated
	#pragma warning(3 : 4431) // missing type specifier - int assumed. Note: C no longer supports default-int
	#pragma warning(3 : 4254) // conversion from 'type1' to 'type2', possible loss of data
	#pragma warning(3 : 4388) // signed/unsigned mismatch
	#include <algorithm>
	using std::min;
	using std::max;

	#if defined (__clang__) && __clang__
		#if defined (VSTGUI_WARN_EVERYTHING) && VSTGUI_WARN_EVERYTHING == 1
			#pragma clang diagnostic warning "-Wconversion"
			#pragma clang diagnostic ignored "-Wreorder"
		#else
			#pragma clang diagnostic warning "-Wunreachable-code"
		#endif
	#endif

#else
	#error unsupported compiler
#endif

#ifdef UNICODE
	#undef UNICODE
#endif
#define UNICODE 1

//----------------------------------------------------
// Deprecation setting
//----------------------------------------------------
#ifndef VSTGUI_ENABLE_DEPRECATED_METHODS
	#define VSTGUI_ENABLE_DEPRECATED_METHODS 0
#endif

#ifndef DEPRECATED_ATTRIBUTE
	#define DEPRECATED_ATTRIBUTE
#endif

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	#define VSTGUI_DEPRECATED(x)	DEPRECATED_ATTRIBUTE	x
#else
	#define VSTGUI_DEPRECATED(x)
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
#define CLASS_METHODS_NOCOPY(name, parent) CBaseObject* newCopy () const override { return 0; }
#define CLASS_METHODS_VIRTUAL(name, parent) CBaseObject* newCopy () const override = 0;

//----------------------------------------------------
namespace VSTGUI {
	
typedef double		CCoord;				///< coordinate type
typedef const char* IdStringPtr;		///< ID String pointer
typedef const char* UTF8StringPtr;		///< UTF8 String pointer
typedef char*		UTF8StringBuffer;	///< UTF8 String buffer pointer

//-----------------------------------------------------------------------------
// @brief Byte Order
//-----------------------------------------------------------------------------
enum ByteOrder {
	kBigEndianByteOrder = 0,
	kLittleEndianByteOrder,
#if WINDOWS || defined (__LITTLE_ENDIAN__)
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

//-----------------------------------------------------------------------------
// CBaseObject Declaration
//! @brief Base Object with reference counter
//-----------------------------------------------------------------------------
class CBaseObject
{
public:
	CBaseObject () = default;
	virtual ~CBaseObject () noexcept = default;

	//-----------------------------------------------------------------------------
	/// @name Reference Counting Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void forget () { nbReference--; if (nbReference == 0) { beforeDelete (); delete this; } }	///< decrease refcount and delete object if refcount == 0
	virtual void remember () { nbReference++; }										///< increase refcount
	virtual int32_t getNbReference () const { return nbReference; }					///< get refcount
	//@}

	//-----------------------------------------------------------------------------
	/// @name Message Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual CMessageResult notify (CBaseObject* sender, IdStringPtr message) { return kMessageUnknown; }
	//@}

	virtual void beforeDelete () {}

	/// @cond ignore
	virtual CBaseObject* newCopy () const { return 0; }
	/// @endcond

private:
	int32_t nbReference {1};
};

//------------------------------------------------------------------------
template <class I>
class SharedPointer
{
public:
//------------------------------------------------------------------------
	inline SharedPointer (I* ptr, bool remember = true);
	inline SharedPointer (const SharedPointer&);
	inline SharedPointer ();
	inline ~SharedPointer () noexcept;

	inline I* operator=(I* ptr);
	inline SharedPointer<I>& operator=(const SharedPointer<I>& );

	inline operator I* ()  const { return ptr; }      // act as I*
	inline I* operator->() const { return ptr; }      // act as I*

	template<class T> T* cast () const { return dynamic_cast<T*> (ptr); }

	inline SharedPointer (SharedPointer<I>&& mp) noexcept;
	inline SharedPointer<I>& operator=(SharedPointer<I>&& mp) noexcept;
//------------------------------------------------------------------------
protected:
	I* ptr;
};

//-----------------------------------------------------------------------------
class CBaseObjectGuard
{
public:
	explicit CBaseObjectGuard (CBaseObject* _obj) : obj (_obj) {}
protected:
	SharedPointer<CBaseObject> obj;
};


//------------------------------------------------------------------------
template <class I>
inline SharedPointer<I>::SharedPointer (I* _ptr, bool remember)
: ptr (_ptr)
{
	if (ptr && remember)
		ptr->remember ();
}

//------------------------------------------------------------------------
template <class I>
inline SharedPointer<I>::SharedPointer (const SharedPointer<I>& other)
: ptr (other.ptr)
{
	if (ptr)
		ptr->remember ();
}

//------------------------------------------------------------------------
template <class I>
inline SharedPointer<I>::SharedPointer ()
: ptr (0)
{}

//------------------------------------------------------------------------
template <class I>
inline SharedPointer<I>::~SharedPointer () noexcept
{
	if (ptr)
		ptr->forget ();
}

//------------------------------------------------------------------------
template <class I>
inline SharedPointer<I>::SharedPointer (SharedPointer<I>&& mp) noexcept
: ptr (nullptr)
{
	*this = std::move (mp);
}

//------------------------------------------------------------------------
template <class I>
inline SharedPointer<I>& SharedPointer<I>::operator=(SharedPointer<I>&& mp) noexcept
{
	if (ptr)
		ptr->forget ();
	ptr = mp.ptr;
	mp.ptr = nullptr;
	return *this;
}

//------------------------------------------------------------------------
template <class I>
inline I* SharedPointer<I>::operator=(I* _ptr)
{
	if (_ptr != ptr)
	{
		if (ptr)
			ptr->forget ();
		ptr = _ptr;
		if (ptr)
			ptr->remember ();
	}
	return ptr;
}

//------------------------------------------------------------------------
template <class I>
inline SharedPointer<I>& SharedPointer<I>::operator=(const SharedPointer<I>& _ptr)
{
	operator= (_ptr.ptr);
	return *this;
}

//------------------------------------------------------------------------
template <class I>
inline SharedPointer<I> owned (I* p) { return SharedPointer<I> (p, false); }

//------------------------------------------------------------------------
template <class I>
inline SharedPointer<I> shared (I* p) { return SharedPointer<I> (p, true); }

//------------------------------------------------------------------------
template <class I, typename ...Args>
inline SharedPointer<I> makeOwned (Args&& ...args)
{
	return SharedPointer<I> (new I (std::forward<Args>(args)...), false);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
template <class I>
class OwningPointer : public SharedPointer<I>
{
public:
//------------------------------------------------------------------------
	inline OwningPointer (I* p) : SharedPointer<I> (p, false) {}
	inline OwningPointer (const SharedPointer<I>& p) : SharedPointer<I> (p) {}
	inline OwningPointer (const OwningPointer<I>& p) : SharedPointer<I> (p) {}
	inline OwningPointer () : SharedPointer<I> () {}
	inline I* operator=(I* _ptr)
	{
		if (_ptr != this->ptr)
		{
			if (this->ptr)
				this->ptr->forget ();
			this->ptr = _ptr;
		}
		return this->ptr;
	}
};
#endif

} // namespace

//-----------------------------------------------------------------------------
#include "vstguidebug.h"

#endif
