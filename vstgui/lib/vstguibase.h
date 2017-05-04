// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
		#ifndef MAC_OS_X_VERSION_10_9
			#error you need at least OSX SDK 10.9 to build vstgui
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
	#if _WIN32_WINNT < 0x601
		#error unsupported Platform SDK you need at least the Windows 7 Platform SDK to compile VSTGUI
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
    #include <cstdint>
    #include <type_traits>
    #include <algorithm>
    #include <climits>
    using std::min;
    using std::max;
	#ifndef LINUX
		#define LINUX 1
	#endif

#else
	#error unsupported compiler
#endif

#include <atomic>

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
	
using CCoord = double;					///< coordinate type
using IdStringPtr = const char*;		///< ID String pointer
using UTF8StringPtr = const char*;		///< UTF8 String pointer
using UTF8StringBuffer = char*;			///< UTF8 String buffer pointer

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
class IReference
{
public:
	virtual void forget () = 0;
	virtual void remember () = 0;
};

//-----------------------------------------------------------------------------
template <typename T>
class ReferenceCounted : public IReference
{
public:
	ReferenceCounted () = default;
	virtual ~ReferenceCounted () noexcept = default;
	
	ReferenceCounted (const ReferenceCounted&) {};
	ReferenceCounted& operator= (const ReferenceCounted&) { return *this; }

	//-----------------------------------------------------------------------------
	/// @name Reference Counting Methods
	//-----------------------------------------------------------------------------
	//@{
	void forget () override { if (--nbReference == 0) { beforeDelete (); delete this; } }	///< decrease refcount and delete object if refcount == 0
	void remember () override { nbReference++; }										///< increase refcount
	virtual int32_t getNbReference () const { return nbReference; }					///< get refcount
	virtual void beforeDelete () {}
	//@}
private:
	T nbReference {1};
};

using AtomicReferenceCounted = ReferenceCounted<std::atomic<int32_t>>;
using NonAtomicReferenceCounted = ReferenceCounted<int32_t>;

//-----------------------------------------------------------------------------
// CBaseObject Declaration
//! @brief Base Object with reference counter
//-----------------------------------------------------------------------------
class CBaseObject : public NonAtomicReferenceCounted
{
public:
	CBaseObject () = default;
	~CBaseObject () noexcept override = default;

	CBaseObject (const CBaseObject& o) {};
	CBaseObject& operator= (const CBaseObject& obj) { return *this; }
	
	//-----------------------------------------------------------------------------
	/// @name Message Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual CMessageResult notify (CBaseObject* sender, IdStringPtr message) { return kMessageUnknown; }
	//@}

	/// @cond ignore
	virtual CBaseObject* newCopy () const { return 0; }
	/// @endcond
};

//------------------------------------------------------------------------
template <class I>
class SharedPointer
{
public:
//------------------------------------------------------------------------
	inline SharedPointer (I* ptr, bool remember = true) noexcept;
	inline SharedPointer (const SharedPointer&) noexcept;
	inline SharedPointer () noexcept;
	inline ~SharedPointer () noexcept;

	inline I* operator=(I* ptr) noexcept;
	inline SharedPointer<I>& operator=(const SharedPointer<I>& ) noexcept;

	inline operator I* ()  const noexcept { return ptr; }      // act as I*
	inline I* operator->() const noexcept { return ptr; }      // act as I*

	inline I* get () const noexcept { return ptr; }

	template<class T> T* cast () const { return dynamic_cast<T*> (ptr); }

	inline SharedPointer (SharedPointer<I>&& mp) noexcept;
	inline SharedPointer<I>& operator=(SharedPointer<I>&& mp) noexcept;

	template<typename T>
	inline SharedPointer (const SharedPointer<T>& op) noexcept
	{
		*this = static_cast<I*> (op.get ());
	}

	template<typename T>
	inline SharedPointer& operator= (const SharedPointer<T>& op) noexcept
	{
		*this = static_cast<I*> (op.get ());
		return *this;
	}
	
	template<typename T>
	inline SharedPointer (SharedPointer<T>&& op) noexcept
	{
		*this = std::move (op);
	}

	template<typename T>
	inline SharedPointer& operator= (SharedPointer<T>&& op) noexcept
	{
		if (ptr)
			ptr->forget ();
		ptr = op.ptr;
		op.ptr = nullptr;
		return *this;
	}
	
//------------------------------------------------------------------------
protected:
	template<typename T>
	friend class SharedPointer;

	I* ptr {nullptr};
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
inline SharedPointer<I>::SharedPointer (I* _ptr, bool remember) noexcept
: ptr (_ptr)
{
	if (ptr && remember)
		ptr->remember ();
}

//------------------------------------------------------------------------
template <class I>
inline SharedPointer<I>::SharedPointer (const SharedPointer<I>& other) noexcept
: ptr (other.ptr)
{
	if (ptr)
		ptr->remember ();
}

//------------------------------------------------------------------------
template <class I>
inline SharedPointer<I>::SharedPointer () noexcept
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
inline I* SharedPointer<I>::operator=(I* _ptr) noexcept
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
inline SharedPointer<I>& SharedPointer<I>::operator=(const SharedPointer<I>& _ptr) noexcept
{
	operator= (_ptr.ptr);
	return *this;
}

//------------------------------------------------------------------------
template <class I>
inline SharedPointer<I> owned (I* p) noexcept { return SharedPointer<I> (p, false); }

//------------------------------------------------------------------------
template <class I>
inline SharedPointer<I> shared (I* p) noexcept { return SharedPointer<I> (p, true); }

//------------------------------------------------------------------------
template <class I, typename ...Args>
inline SharedPointer<I> makeOwned (Args&& ...args)
{
	return SharedPointer<I> (new I (std::forward<Args>(args)...), false);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
/** An owning pointer. \deprecated
 *
 *	This class is now deprecated. Please change your code from
 *	@code{.cpp} OwningPointer<MyClass> obj = new MyClass () @endcode
 *	to
 *	@code{.cpp} SharedPointer<MyClass> obj = makeOwned<MyClass> (); @endcode
 */
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

} // namespace

//-----------------------------------------------------------------------------
#include "vstguidebug.h"

#endif
