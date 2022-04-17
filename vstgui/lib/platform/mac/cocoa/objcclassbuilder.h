// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#import <objc/runtime.h>
#import <objc/message.h>
#import <tuple>
#import <string>
#import <optional>

//------------------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------------------
template<typename T>
struct ObjCVariable
{
	ObjCVariable (id obj, Ivar ivar) : obj (obj), ivar (ivar) {}

	T get () const { return static_cast<T> (object_getIvar (obj, ivar)); }

	void set (const T& value) { object_setIvar (obj, ivar, static_cast<id> (value)); }

private:
	id obj;
	Ivar ivar {nullptr};
};

//------------------------------------------------------------------------------------
struct ObjCInstance
{
	ObjCInstance (id obj) : obj (obj) {}

	template<typename T>
	std::optional<ObjCVariable<T>> getVariable (const char* name) const
	{
		if (auto ivar = class_getInstanceVariable ([obj class], name))
		{
			return {ObjCVariable<T> (obj, ivar)};
		}
		return {};
	}

	template<typename Func, typename... T>
	void callSuper (SEL selector, T... args) const
	{
		void (*f) (id, SEL, T...) = (void (*) (id, SEL, T...))objc_msgSendSuper;
		f (getSuper (), selector, args...);
	}

	template<typename Func, typename R, typename... T>
	R callSuper (SEL selector, T... args) const
	{
		R (*f) (id, SEL, T...) = (R (*) (id, SEL, T...))objc_msgSendSuper;
		return f (getSuper (), selector, args...);
	}

private:
	id getSuper () const
	{
		if (os.receiver == nullptr)
		{
			os.receiver = obj;
			os.super_class = class_getSuperclass ([obj class]);
		}
		return static_cast<id> (&os);
	}

	id obj;
	mutable objc_super os {};
};

//------------------------------------------------------------------------------------
struct ObjCClassBuilder
{
	ObjCClassBuilder& init (const char* name, Class baseClass);

	template<typename Func>
	ObjCClassBuilder& addMethod (SEL selector, Func imp);
	template<typename T>
	ObjCClassBuilder& addIvar (const char* name);

	ObjCClassBuilder& addProtocol (const char* name);
	ObjCClassBuilder& addProtocol (Protocol* proto);

	Class finalize ();

private:
	static Class generateUniqueClass (const std::string& inClassName, Class baseClass);

	template<typename Func>
	ObjCClassBuilder& addMethod (SEL selector, Func imp, const char* types);

	ObjCClassBuilder& addIvar (const char* name, size_t size, uint8_t alignment, const char* types);

	template<typename R, typename... T>
	static constexpr std::tuple<R, T...> functionArgs (R (*) (T...))
	{
		return std::tuple<R, T...> ();
	}

	template<typename... T>
	static constexpr std::tuple<T...> functionArgs (void (*) (T...))
	{
		return std::tuple<T...> ();
	}

	template<typename R, typename... T>
	static constexpr bool isVoidReturnType (R (*) (T...))
	{
		return false;
	}

	template<typename... T>
	static constexpr bool isVoidReturnType (void (*) (T...))
	{
		return true;
	}

	template<typename Proc>
	static std::string encodeFunction (Proc proc)
	{
		std::string result;
		if (isVoidReturnType (proc))
			result = "v";
		std::apply ([&] (auto&&... args) { ((result += @encode (decltype (args))), ...); },
					functionArgs (proc));
		return result;
	}

	Class cl {nullptr};
	Class baseClass {nullptr};
};

//------------------------------------------------------------------------
inline ObjCClassBuilder& ObjCClassBuilder::init (const char* name, Class bc)
{
	baseClass = bc;
	cl = generateUniqueClass (name, baseClass);
	return *this;
}

//------------------------------------------------------------------------------------
inline Class ObjCClassBuilder::generateUniqueClass (const std::string& inClassName, Class baseClass)
{
	std::string className (inClassName);
	int32_t iteration = 0;
	while (objc_lookUpClass (className.data ()) != nil)
	{
		iteration++;
		className = inClassName + "_" + std::to_string (iteration);
	}
	Class resClass = objc_allocateClassPair (baseClass, className.data (), 0);
	return resClass;
}

//-----------------------------------------------------------------------------
inline Class ObjCClassBuilder::finalize ()
{
	objc_registerClassPair (cl);

	auto res = cl;
	baseClass = cl = nullptr;
	return res;
}

//-----------------------------------------------------------------------------
template<typename Func>
inline ObjCClassBuilder& ObjCClassBuilder::addMethod (SEL selector, Func imp, const char* types)
{
	auto res = class_addMethod (cl, selector, IMP (imp), types);
	vstgui_assert (res == true);
	return *this;
}

//-----------------------------------------------------------------------------
template<typename Func>
ObjCClassBuilder& ObjCClassBuilder::addMethod (SEL selector, Func imp)
{
	return addMethod (selector, imp, encodeFunction (imp).data ());
}

//------------------------------------------------------------------------
template<typename T>
inline ObjCClassBuilder& ObjCClassBuilder::addIvar (const char* name)
{
	return addIvar (name, sizeof (T), static_cast<uint8_t> (log2 (sizeof (T))), @encode (T));
}

//-----------------------------------------------------------------------------
inline ObjCClassBuilder& ObjCClassBuilder::addIvar (const char* name, size_t size,
													uint8_t alignment, const char* types)
{
	auto res = class_addIvar (cl, name, size, alignment, types);
	vstgui_assert (res == true);
	return *this;
}

//-----------------------------------------------------------------------------
inline ObjCClassBuilder& ObjCClassBuilder::addProtocol (const char* name)
{
	if (auto protocol = objc_getProtocol (name))
		return addProtocol (protocol);
	return *this;
}

//-----------------------------------------------------------------------------
inline ObjCClassBuilder& ObjCClassBuilder::addProtocol (Protocol* proto)
{
	auto res = class_addProtocol (cl, proto);
	vstgui_assert (res == true);
	return *this;
}

//------------------------------------------------------------------------------------
} // VSTGUI
