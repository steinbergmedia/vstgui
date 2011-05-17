//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//-----------------------------------------------------------------------------
// VSTGUI Version
//-----------------------------------------------------------------------------
#define VSTGUI_VERSION_MAJOR  4
#define VSTGUI_VERSION_MINOR  0

//-----------------------------------------------------------------------------
// Platform definitions
//-----------------------------------------------------------------------------
#if WIN32
	#define WINDOWS 1
#elif __APPLE_CC__
	#include <stdint.h>
	#include <AvailabilityMacros.h>
	#ifndef MAC_OS_X_VERSION_10_5
		#define MAC_OS_X_VERSION_10_5 1050
	#endif
	#ifndef MAC_COCOA
		#define MAC_COCOA (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5)
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

#if WINDOWS
	#include <sdkddkver.h>
	#if _WIN32_WINNT < 0x600
		#error unsupported Platform SDK you need at least the Vista Platform SDK to compile VSTGUI
	#endif
	#if defined (_WIN32_WINNT_WIN7) && !defined (VSTGUI_DIRECT2D_SUPPORT)
		#define VSTGUI_DIRECT2D_SUPPORT	1
	#endif
	typedef char				int8_t;
	typedef unsigned char		uint8_t;
	typedef short				int16_t;
	typedef unsigned short		uint16_t;
	typedef long				int32_t;
	typedef unsigned long		uint32_t;
	typedef __int64				int64_t;
	typedef unsigned __int64	uint64_t;
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

#ifndef DEPRECATED_ATTRIBUTE
#define DEPRECATED_ATTRIBUTE
#endif

#if VSTGUI_ENABLE_DEPRECATED_METHODS
#define VSTGUI_DEPRECATED(x)	DEPRECATED_ATTRIBUTE	x
#else
#define VSTGUI_DEPRECATED(x)
#endif

//----------------------------------------------------
// Helper makros
//----------------------------------------------------
#define	VSTGUI_MAKE_STRING_PRIVATE_DONT_USE(x)	# x
#define	VSTGUI_MAKE_STRING(x)					VSTGUI_MAKE_STRING_PRIVATE_DONT_USE(x)

#if DEVELOPMENT
	#ifndef DEBUG
		#define DEBUG	1
	#endif
#else
	#if !defined(NDEBUG) && !defined(DEBUG)
		#define NDEBUG	1
	#endif
#endif

namespace VSTGUI {

#if DEBUG
#define CLASS_METHODS(name, parent)             \
	virtual bool isTypeOf (IdStringPtr s) const \
		{ return (!strcmp (s, (#name))) ? true : parent::isTypeOf (s); }\
	virtual IdStringPtr getClassName () const { return (#name); } \
	virtual CBaseObject* newCopy () const { return new name (*this); }
#define CLASS_METHODS_NOCOPY(name, parent)             \
	virtual bool isTypeOf (IdStringPtr s) const \
		{ return (!strcmp (s, (#name))) ? true : parent::isTypeOf (s); }\
	virtual IdStringPtr getClassName () const { return (#name); } \
	virtual CBaseObject* newCopy () const { return 0; }
#else
#define CLASS_METHODS(name, parent)             \
	virtual bool isTypeOf (IdStringPtr s) const \
		{ return (!strcmp (s, (#name))) ? true : parent::isTypeOf (s); } \
	virtual CBaseObject* newCopy () const { return (CBaseObject*)new name (*this); }
#define CLASS_METHODS_NOCOPY(name, parent)             \
	virtual bool isTypeOf (IdStringPtr s) const \
		{ return (!strcmp (s, (#name))) ? true : parent::isTypeOf (s); } \
	virtual CBaseObject* newCopy () const { return 0; }
#endif
#define CLASS_METHODS_VIRTUAL(name, parent)             \
	virtual bool isTypeOf (IdStringPtr s) const \
		{ return (!strcmp (s, (#name))) ? true : parent::isTypeOf (s); } \
	virtual CBaseObject* newCopy () const = 0;

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
	CBaseObject () : nbReference (1) {}
	virtual ~CBaseObject () {}

	//-----------------------------------------------------------------------------
	/// @name Reference Counting Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void forget () { nbReference--; if (nbReference == 0) delete this; }	///< decrease refcount and delete object if refcount == 0
	virtual void remember () { nbReference++; }										///< increase refcount
	virtual int32_t getNbReference () const { return nbReference; }					///< get refcount
	//@}
	
	//-----------------------------------------------------------------------------
	/// @name Message Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual CMessageResult notify (CBaseObject* sender, IdStringPtr message) { return kMessageUnknown; }
	//@}

	/// @cond ignore
	virtual bool isTypeOf (IdStringPtr s) const { return (!strcmp (s, "CBaseObject")); }
	virtual CBaseObject* newCopy () const { return 0; }
	/// @endcond

	#if DEBUG
	virtual IdStringPtr getClassName () const { return "CBaseObject"; }
	#endif
	
private:
	int32_t nbReference;
};

//-----------------------------------------------------------------------------
class CBaseObjectGuard
{
public:
	CBaseObjectGuard (CBaseObject* _obj) : obj (_obj) { if (obj) obj->remember (); }
	~CBaseObjectGuard () { if (obj) obj->forget (); }
protected:
	CBaseObject* obj;
};

} // namespace

//-----------------------------------------------------------------------------
#include "vstguidebug.h"

#endif
