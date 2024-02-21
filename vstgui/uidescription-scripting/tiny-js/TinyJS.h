/*
 * TinyJS
 *
 * A single-file Javascript-alike engine
 *
 * Authored By Gordon Williams <gw@pur3.co.uk>
 *
 * Copyright (C) 2009 Pur3 Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

// If defined, this keeps a note of all calls and where from in memory. This is slower, but good for
// debugging
#define TINYJS_CALL_STACK

#include <string>
#include <vector>
#include <functional>
#include <variant>
#include <limits>

#ifndef TRACE
#define TRACE printf
#endif // TRACE

//------------------------------------------------------------------------
namespace TJS {

constexpr int TINYJS_LOOP_MAX_ITERATIONS = 8192;

enum class LexType : int
{
	Eof = 0,
	ID = 256,
	INT,
	FLOAT,
	STR,

	EQUAL,
	TYPEEQUAL,
	NEQUAL,
	NTYPEEQUAL,
	LEQUAL,
	LSHIFT,
	LSHIFTEQUAL,
	GEQUAL,
	RSHIFT,
	RSHIFTUNSIGNED,
	RSHIFTEQUAL,
	PLUSEQUAL,
	MINUSEQUAL,
	PLUSPLUS,
	MINUSMINUS,
	ANDEQUAL,
	ANDAND,
	OREQUAL,
	OROR,
	XOREQUAL,

	// reserved words
	R_LIST_START,
	R_IF = R_LIST_START,
	R_ELSE,
	R_DO,
	R_WHILE,
	R_FOR,
	R_BREAK,
	R_CONTINUE,
	R_FUNCTION,
	R_RETURN,
	R_VAR,
	R_TRUE,
	R_FALSE,
	R_NULL,
	R_UNDEFINED,
	R_NEW,

	LIST_END /* always the last entry */
};
inline constexpr int asInteger (LexType t) { return static_cast<int> (t); }
inline constexpr LexType asLexType (int t)
{
	if (t < asInteger (LexType::ID) || t > (asInteger (LexType::LIST_END)))
		return LexType::Eof;
	return static_cast<LexType> (t);
}

enum SCRIPTVAR_FLAGS
{
	SCRIPTVAR_UNDEFINED = 0,
	SCRIPTVAR_FUNCTION = 1,
	SCRIPTVAR_OBJECT = 2,
	SCRIPTVAR_ARRAY = 4,
	SCRIPTVAR_DOUBLE = 8,	// floating point double
	SCRIPTVAR_INTEGER = 16, // integer number
	SCRIPTVAR_STRING = 32,	// string
	SCRIPTVAR_NULL = 64,	// it seems null is its own data type

	SCRIPTVAR_NATIVE = 128, // to specify this is a native function
	SCRIPTVAR_NUMERICMASK = SCRIPTVAR_NULL | SCRIPTVAR_DOUBLE | SCRIPTVAR_INTEGER,
	SCRIPTVAR_VARTYPEMASK = SCRIPTVAR_DOUBLE | SCRIPTVAR_INTEGER | SCRIPTVAR_STRING |
							SCRIPTVAR_FUNCTION | SCRIPTVAR_OBJECT | SCRIPTVAR_ARRAY |
							SCRIPTVAR_NULL,

};

static constexpr auto TINYJS_RETURN_VAR = "return";
static constexpr auto TINYJS_PROTOTYPE_CLASS = "prototype";
static constexpr auto TINYJS_TEMP_NAME = "";
static constexpr auto TINYJS_BLANK_DATA = "";

//------------------------------------------------------------------------
// Custom memory allocator
using AllocatorFunc = std::function<void*(size_t)>;
using DeallocatorFunc = std::function<void (void*, size_t)>;
extern AllocatorFunc allocator;
extern DeallocatorFunc deallocator;

void setCustomAllocator (AllocatorFunc&& allocator, DeallocatorFunc&& deallocator);

//------------------------------------------------------------------------
template<typename T>
struct Allocator
{
	using value_type = T;
	using propagate_on_container_move_assignment = std::true_type;

	Allocator () = default;

	template<class U>
	constexpr Allocator (const Allocator<U>&) noexcept
	{
	}

	[[nodiscard]] T* allocate (std::size_t n)
	{
		return static_cast<T*> (allocator (n * sizeof (T)));
	}
	void deallocate (T* p, std::size_t n) noexcept { deallocator (p, n); }

	bool operator== (const Allocator& other) const { return &other == this; }
	bool operator!= (const Allocator& other) const { return &other != this; }
};

using string = std::basic_string<char, std::char_traits<char>, Allocator<char>>;
using ostringstream = std::basic_ostringstream<char, std::char_traits<char>, Allocator<char>>;

/** convert the given string into a quoted string suitable for javascript */
string getJSString (std::string_view str);
/** convert the given string to an 64 bit integer supporting hex and octal written numbers */
int64_t stringToInteger (std::string_view str);

class CScriptException
{
public:
	string text;
	CScriptException (const string& exceptionText);
	CScriptException (string&& exceptionText);
	CScriptException (const CScriptException&) = default;
	CScriptException (CScriptException&&) = default;
	~CScriptException () noexcept;

	static void* operator new (std::size_t count);
	static void operator delete (void* ptr, std::size_t size);
};

class CScriptLex
{
public:
	CScriptLex (std::string_view input);
	~CScriptLex (void);

	/** Get the string representation of the given token */
	static string getTokenStr (int token);

	/** Lexical match wotsit */
	void match (int expected_tk);
	void match (LexType expected_tk);
	/** Reset this lex so we can start again */
	void reset ();

	int getToken () const { return token; }
	size_t getTokenStart () const { return tokenStart; }
	size_t getTokenEnd () const { return tokenEnd; }
	const string& getTokenString () const { return tkStr; }

	/** Return a sub-string from the given position up until right now */
	string getSubString (size_t pos) const;
	/** Return a sub-lexer from the given position up until right now */
	CScriptLex* getSubLex (size_t lastPosition) const;
	/** Return a string representing the position in lines and columns of the character pos given */
	string getPosition (size_t pos = std::numeric_limits<size_t>::max ()) const;

	static void* operator new (std::size_t count);
	static void operator delete (void* ptr, std::size_t size);

private:
	void getNextCh ();
	/** Get the text token from our text string */
	void getNextToken ();

	/** The type of the token that we have */
	int token;
	/** Position in the data at the beginning of the token we have here */
	size_t tokenStart;
	/** Position in the data at the last character of the token we have here */
	size_t tokenEnd;
	/** Position in the data at the last character of the last token */
	size_t tokenLastEnd;
	/** Data contained in the token we have here */
	string tkStr;

	char currCh, nextCh;

	/** Data string to get tokens from */
	const char* data;
	/** Start and end position in data string */
	size_t dataEnd;
	/** Position in data (we CAN go past the end of the string here) */
	size_t dataPos;

	std::vector<size_t, Allocator<size_t>> newLinePositions;
};

class CScriptVar;

using JSCallback = std::function<void (CScriptVar* var)>;

class CScriptVarLink
{
public:
	CScriptVarLink (CScriptVar* var, const string& name = TINYJS_TEMP_NAME, bool own = false);
	/** Copy constructor */
	CScriptVarLink (const CScriptVarLink& link);
	~CScriptVarLink ();
	/** Replace the Variable pointed to */
	void replaceWith (CScriptVar* newVar);
	/** Replace the Variable pointed to (just dereferences) */
	void replaceWith (CScriptVarLink* newVar);
	/** Get the name as an integer (for arrays) */
	int getIntName () const;
	/** Set the name as an integer (for arrays) */
	void setIntName (int n);

	const string& getName () const { return name; }

	void setNextSibling (CScriptVarLink* s) { nextSibling = s; }
	void setPrevSibling (CScriptVarLink* s) { prevSibling = s; }
	CScriptVarLink* getNextSibling () const { return nextSibling; }
	CScriptVarLink* getPrevSibling () const { return prevSibling; }

	void setVar (CScriptVar* v);
	CScriptVar* getVar () const { return var; }

	bool owned () const { return isOwned; }

	static void* operator new (std::size_t count);
	static void operator delete (void* ptr, std::size_t size);

private:
	string name;
	CScriptVarLink* nextSibling {nullptr};
	CScriptVarLink* prevSibling {nullptr};
	CScriptVar* var {nullptr};
	bool isOwned {false};
};

struct IScriptVarLifeTimeObserver
{
	virtual ~IScriptVarLifeTimeObserver () noexcept = default;

	virtual void onDestroy (CScriptVar* var) = 0;
};

/** Variable class (containing a doubly-linked list of children) */
class CScriptVar
{
public:
	/** Create undefined */
	CScriptVar ();
	/** User defined */
	CScriptVar (const string& varData, int varFlags);
	/** Create a string */
	CScriptVar (std::string_view str);
	/** Create a double */
	CScriptVar (double varData);
	/** Create an integer */
	CScriptVar (int64_t val);
	/** Create an integer */
	CScriptVar (bool val);
	virtual ~CScriptVar (void);

	/** If this is a function, get the result value (for use by native functions) */
	CScriptVar* getReturnVar ();
	/** Set the result value. Use this when setting complex return data as it avoids a deepCopy() */
	void setReturnVar (CScriptVar* var);
	/** If this is a function, get the parameter with the given name (for use by native functions)
	 */
	CScriptVar* getParameter (std::string_view name);

	/** Tries to find a child with the given name, may return 0 */
	CScriptVarLink* findChild (std::string_view childName);
	/** Tries to find a child with the given name, or will create it with the given flags */
	CScriptVarLink* findChildOrCreate (std::string_view childName,
									   int varFlags = SCRIPTVAR_UNDEFINED);
	/** Tries to find a child with the given path (separated by dots) */
	CScriptVarLink* findChildOrCreateByPath (const string& path);
	/** add a child if not already exist */
	CScriptVarLink* addChild (std::string_view childName, CScriptVar* child = NULL);
	/** add a child overwriting any with the same name */
	CScriptVarLink* addChildNoDup (std::string_view childName, CScriptVar* child = NULL);
	/** remove the child */
	void removeChild (CScriptVar* child);
	/** Remove a specific link (this is faster than finding via a child) */
	void removeLink (CScriptVarLink* link);
	void removeAllChildren ();
	/** The the value at an array index */
	CScriptVar* getArrayIndex (int idx);
	/** Set the value at an array index */
	void setArrayIndex (int idx, CScriptVar* value);
	/** If this is an array, return the number of items in it (else 0) */
	int getArrayLength ();
	/** Get the number of children */
	int getChildren ();

	int64_t getInt ();
	bool getBool () { return getInt () != 0; }
	double getDouble ();
	const string& getString ();
	/** get Data as a parsable javascript string */
	string getParsableString ();
	void setInt (int64_t num);
	void setDouble (double val);
	void setString (std::string_view str);
	void setUndefined ();
	void setArray ();
	bool equals (CScriptVar* v);

	bool isInt () { return (flags & SCRIPTVAR_INTEGER) != 0; }
	bool isDouble () { return (flags & SCRIPTVAR_DOUBLE) != 0; }
	bool isString () { return (flags & SCRIPTVAR_STRING) != 0; }
	bool isNumeric () { return (flags & SCRIPTVAR_NUMERICMASK) != 0; }
	bool isFunction () { return (flags & SCRIPTVAR_FUNCTION) != 0; }
	bool isObject () { return (flags & SCRIPTVAR_OBJECT) != 0; }
	bool isArray () { return (flags & SCRIPTVAR_ARRAY) != 0; }
	bool isNative () { return (flags & SCRIPTVAR_NATIVE) != 0; }
	bool isUndefined () { return (flags & SCRIPTVAR_VARTYPEMASK) == SCRIPTVAR_UNDEFINED; }
	bool isNull () { return (flags & SCRIPTVAR_NULL) != 0; }
	/** Is this *not* an array/object/etc */
	bool isBasic () { return firstChild == 0; }

	/** do a maths op with another script variable */
	CScriptVar* mathsOp (CScriptVar* b, int op);
	/** copy the value from the value given */
	void copyValue (CScriptVar* val);
	/** deep copy this node and return the result */
	CScriptVar* deepCopy ();

	/** Dump out the contents of this using trace */
	void trace (string indentStr = "", const string& name = "");
	/** For debugging - just dump a string version of the flags */
	string getFlagsAsString ();
	/** Write out all the JS code needed to recreate this script variable to the stream (as JSON) */
	void getJSON (std::ostream& destination, const string linePrefix = "");
	/** Set the callback for native functions */
	void setCallback (const JSCallback& callback);
	/** Moves in the callback for native functions */
	void setCallback (JSCallback&& callback);

	void callCallback (CScriptVar* var);

	void setFunctionScript (std::string_view str);

	/// For memory management/garbage collection
	/** Add reference to this variable */
	CScriptVar* addRef ();
	/** Remove a reference, and delete this variable if required */
	void release ();
	/** Get the number of references to this script variable */
	int getRefs ();

	void setLifeTimeObserver (IScriptVarLifeTimeObserver* obs) { lifeTimeObserver = obs; }

	CScriptVarLink* getFirstChild () const { return firstChild; }
	CScriptVarLink* getLastChild () const { return lastChild; }

	static void* operator new (std::size_t count);
	static void operator delete (void* ptr, std::size_t size);

protected:
	CScriptVarLink* firstChild {nullptr};
	CScriptVarLink* lastChild {nullptr};

	/** The number of references held to this - used for garbage collection */
	int refs {0};
	/** the flags determine the type of the variable - int/double/string/etc */
	int flags {0};
	std::variant<string, int64_t, double, JSCallback> variant;
	string dataStr;

	/** Copy the basic data and flags from the variable given, with no
	 * children. Should be used internally only - by copyValue and deepCopy */
	void copySimpleData (CScriptVar* val);

private:
	IScriptVarLifeTimeObserver* lifeTimeObserver {nullptr};
};

class CTinyJS
{
public:
	CTinyJS ();
	~CTinyJS ();

	void execute (const string& code);
	/** Evaluate the given code and return a link to a javascript object,
	 * useful for (dangerous) JSON parsing. If nothing to return, will return
	 * 'undefined' variable type. CScriptVarLink is returned as this will
	 * automatically release the result as it goes out of scope. If you want to
	 * keep it, you must use addRef() and release() */
	CScriptVarLink evaluateComplex (std::string_view code);
	/** Evaluate the given code and return a string. If nothing to return, will return
	 * 'undefined' */
	string evaluate (std::string_view code);

	/** add a native function to be called from TinyJS
	   example:
	   \code
		   void scRandInt(CScriptVar *c, void *userdata) { ... }
		   tinyJS->addNative("function randInt(min, max)", scRandInt, 0);
	   \endcode

	   or

	   \code
		   void scSubstring(CScriptVar *c, void *userdata) { ... }
		   tinyJS->addNative("function String.substring(lo, hi)", scSubstring, 0);
	   \endcode
	*/
	void addNative (std::string_view funcDesc, const JSCallback& ptr);

	/** Get the given variable specified by a path (var1.var2.etc), or return 0 */
	CScriptVar* getScriptVariable (const string& path) const;
	/** Get the value of the given variable, or return 0 */
	const string* getVariable (const string& path) const;
	/** set the value of the given variable, return trur if it exists and gets set */
	bool setVariable (const string& path, const string& varData);

	/** Send all variables to stdout */
	void trace ();

	CScriptVar* getRoot () const { return root; }

	static void* operator new (std::size_t count);
	static void operator delete (void* ptr, std::size_t size);

private:
	/** root of symbol table */
	CScriptVar* root {nullptr};
	/** current lexer */
	CScriptLex* lexer {nullptr};
	/** stack of scopes when parsing */
	std::vector<CScriptVar*> scopes;
#ifdef TINYJS_CALL_STACK
	/** Names of places called so we can show when erroring */
	std::vector<string> call_stack;
#endif

	/** Built in string class */
	CScriptVar* stringClass {nullptr};
	/** Built in object class */
	CScriptVar* objectClass {nullptr};
	/** Built in array class */
	CScriptVar* arrayClass {nullptr};

	// parsing - in order of precedence
	CScriptVarLink* functionCall (bool& execute, CScriptVarLink* function, CScriptVar* parent);
	CScriptVarLink* factor (bool& execute);
	CScriptVarLink* unary (bool& execute);
	CScriptVarLink* term (bool& execute);
	CScriptVarLink* expression (bool& execute);
	CScriptVarLink* shift (bool& execute);
	CScriptVarLink* condition (bool& execute);
	CScriptVarLink* logic (bool& execute);
	CScriptVarLink* ternary (bool& execute);
	CScriptVarLink* base (bool& execute);
	void block (bool& execute);
	void statement (bool& execute);
	// parsing utility functions
	CScriptVarLink* parseFunctionDefinition ();
	void parseFunctionArguments (CScriptVar* funcVar) const;

	/** Finds a child, looking recursively up the scopes */
	CScriptVarLink* findInScopes (const string& childName) const;
	/** Look up in any parent classes of the given object */
	CScriptVarLink* findInParentClasses (CScriptVar* object, const string& name) const;
};

//------------------------------------------------------------------------
} // TJS
