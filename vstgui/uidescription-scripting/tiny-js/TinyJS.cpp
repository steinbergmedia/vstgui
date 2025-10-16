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

/* Version 0.1  :  (gw) First published on Google Code
   Version 0.11 :  Making sure the 'root' variable never changes
				   'symbol_base' added for the current base of the sybmbol table
   Version 0.12 :  Added findChildOrCreate, changed string passing to use references
				   Fixed broken string encoding in getJSString()
				   Removed getInitCode and added getJSON instead
				   Added nil
				   Added rough JSON parsing
				   Improved example app
   Version 0.13 :  Added tokenEnd/tokenLastEnd to lexer to avoid parsing whitespace
				   Ability to define functions without names
				   Can now do "var mine = function(a,b) { ... };"
				   Slightly better 'trace' function
				   Added findChildOrCreateByPath function
				   Added simple test suite
				   Added skipping of blocks when not executing
   Version 0.14 :  Added parsing of more number types
				   Added parsing of string defined with '
				   Changed nil to null as per spec, added 'undefined'
				   Now set variables with the correct scope, and treat unknown
							  as 'undefined' rather than failing
				   Added proper (I hope) handling of null and undefined
				   Added === check
   Version 0.15 :  Fix for possible memory leaks
   Version 0.16 :  Removal of un-needed findRecursive calls
				   symbol_base removed and replaced with 'scopes' stack
				   Added reference counting a proper tree structure
					   (Allowing pass by reference)
				   Allowed JSON output to output IDs, not strings
				   Added get/set for array indices
				   Changed Callbacks to include user data pointer
				   Added some support for objects
				   Added more Java-esque builtin functions
   Version 0.17 :  Now we don't deepCopy the parent object of the class
				   Added JSON.stringify and eval()
				   Nicer JSON indenting
				   Fixed function output in JSON
				   Added evaluateComplex
				   Fixed some reentrancy issues with evaluate/execute
   Version 0.18 :  Fixed some issues with code being executed when it shouldn't
   Version 0.19 :  Added array.length
				   Changed '__parent' to 'prototype' to bring it more in line with javascript
   Version 0.20 :  Added '%' operator
   Version 0.21 :  Added array type
				   String.length() no more - now String.length
				   Added extra constructors to reduce confusion
				   Fixed checks against undefined
   Version 0.22 :  First part of ardi's changes:
					   sprintf -> sprintf_s
					   extra tokens parsed
					   array memory leak fixed
				   Fixed memory leak in evaluateComplex
				   Fixed memory leak in FOR loops
				   Fixed memory leak for unary minus
   Version 0.23 :  Allowed evaluate[Complex] to take in semi-colon separated
					 statements and then only return the value from the last one.
					 Also checks to make sure *everything* was parsed.
				   Ints + doubles are now stored in binary form (faster + more precise)
   Version 0.24 :  More useful error for maths ops
				   Don't dump everything on a match error.
   Version 0.25 :  Better string escaping
   Version 0.26 :  Add CScriptVar::equals
				   Add built-in array functions
   Version 0.27 :  Added OZLB's TinyJS.setVariable (with some tweaks)
				   Added OZLB's Maths Functions
   Version 0.28 :  Ternary operator
				   Rudimentary call stack on error
				   Added String Character functions
				   Added shift operators
   Version 0.29 :  Added new object via functions
				   Fixed getString() for double on some platforms
   Version 0.30 :  Rlyeh Mario's patch for Math Functions on VC++
   Version 0.31 :  Add exec() to TinyJS functions
				   Now print quoted JSON that can be read by PHP/Python parsers
				   Fixed postfix increment operator
   Version 0.32 :  Fixed Math.randInt on 32 bit PCs, where it was broken
   Version 0.33 :  Fixed Memory leak + brokenness on === comparison

	NOTE:
		  Constructing an array with an initial length 'Array(5)' doesn't work
		  Recursive loops of data such as a.foo = a; fail to be garbage collected
		  length variable cannot be set
		  The postfix increment operator returns the current value, not the previous as it should.
		  There is no prefix increment operator
		  Arrays are implemented as a linked list - hence a lookup time is O(n)

	TODO:
		  Utility va-args style function in TinyJS for executing a function directly
		  Merge the parsing of expressions/statements so eval("statement") works like we'd expect.
		  Move 'shift' implementation into mathsOp

 */

#include "TinyJS.h"
#include <cassert>
#include <string>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <array>
#include <charconv>
#include <vector>

//------------------------------------------------------------------------
namespace TJS {

using namespace std::string_view_literals;

#ifdef __GNUC__
#define vsprintf_s vsnprintf
#define sprintf_s snprintf
#define _strdup strdup
#endif

// ----------------------------------------------------------------------------------- Memory Debug

#define DEBUG_MEMORY 0

#if DEBUG_MEMORY

vector<CScriptVar*> allocatedVars;
vector<CScriptVarLink*> allocatedLinks;

void mark_allocated (CScriptVar* v) { allocatedVars.push_back (v); }

void mark_deallocated (CScriptVar* v)
{
	for (size_t i = 0; i < allocatedVars.size (); i++)
	{
		if (allocatedVars[i] == v)
		{
			allocatedVars.erase (allocatedVars.begin () + i);
			break;
		}
	}
}

void mark_allocated (CScriptVarLink* v) { allocatedLinks.push_back (v); }

void mark_deallocated (CScriptVarLink* v)
{
	for (size_t i = 0; i < allocatedLinks.size (); i++)
	{
		if (allocatedLinks[i] == v)
		{
			allocatedLinks.erase (allocatedLinks.begin () + i);
			break;
		}
	}
}

void show_allocated ()
{
	for (size_t i = 0; i < allocatedVars.size (); i++)
	{
		printf ("ALLOCATED, %d refs\n", allocatedVars[i]->getRefs ());
		allocatedVars[i]->trace ("  ");
	}
	for (size_t i = 0; i < allocatedLinks.size (); i++)
	{
		printf ("ALLOCATED LINK %s, allocated[%d] to \n", allocatedLinks[i]->name.c_str (),
				allocatedLinks[i]->var->getRefs ());
		allocatedLinks[i]->var->trace ("  ");
	}
	allocatedVars.clear ();
	allocatedLinks.clear ();
}
#endif

#define ASSERT(X) assert (X)

/* Frees the given link IF it isn't owned by anything else */
static inline void CLEAN (CScriptVarLink* x)
{
	CScriptVarLink* __v = x;
	if (__v && !__v->owned ())
	{
		delete __v;
	}
}

/* Create a LINK to point to VAR and free the old link.
 * BUT this is more clever - it tries to keep the old link if it's not owned to save allocations */
static inline void CREATE_LINK (CScriptVarLink*& link, CScriptVar* var)
{
	if (!link || link->owned ())
		link = new CScriptVarLink (var);
	else
		link->replaceWith (var);
}

// ----------------------------------------------------------------------------------- Utils
bool isWhitespace (char ch) { return (ch == ' ') || (ch == '\t') || (ch == '\n') || (ch == '\r'); }

bool isNumeric (char ch) { return (ch >= '0') && (ch <= '9'); }
bool isNumber (const string& str)
{
	for (size_t i = 0; i < str.size (); i++)
		if (!isNumeric (str[i]))
			return false;
	return true;
}
bool isHexadecimal (char ch)
{
	return ((ch >= '0') && (ch <= '9')) || ((ch >= 'a') && (ch <= 'f')) ||
		   ((ch >= 'A') && (ch <= 'F'));
}
bool isAlpha (char ch)
{
	return ((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')) || ch == '_';
}

/// convert the given string into a quoted string suitable for javascript
string getJSString (std::string_view str)
{
	string nStr {str.data (), str.size ()};
	for (size_t i = 0; i < nStr.size (); i++)
	{
		string replaceWith = "";
		bool replace = true;

		switch (nStr[i])
		{
			case '\\':
				replaceWith = "\\\\";
				break;
			case '\n':
				replaceWith = "\\n";
				break;
			case '\r':
				replaceWith = "\\r";
				break;
			case '\a':
				replaceWith = "\\a";
				break;
			case '"':
				replaceWith = "\\\"";
				break;
			default:
			{
				int nCh = ((int)nStr[i]) & 0xFF;
				if (nCh < 32 || nCh > 127)
				{
					char buffer[5];
					sprintf_s (buffer, 5, "\\x%02X", nCh);
					replaceWith = buffer;
				}
				else
					replace = false;
			}
		}

		if (replace)
		{
			nStr = nStr.substr (0, i) + replaceWith + nStr.substr (i + 1);
			i += replaceWith.size () - 1;
		}
	}
	return "\"" + nStr + "\"";
}

int64_t stringToInteger (std::string_view str)
{
	int base = 10;
	if (str.size () >= 2)
	{
		if (str[0] == '0')
		{
			if (str[1] == 'x')
			{
				base = 16;
				str = {str.data () + 2, str.size () - 2};
			}
			else
			{
				if (str.find_first_of ('.') == string::npos)
				{
					base = 8;
				}
			}
		}
	}
	int64_t result {0};
	auto res = std::from_chars (str.data (), str.data () + str.size (), result, base);
	if (res.ec == std::errc ())
		return result;
	return {};
}

static double stringToDouble (std::string_view str)
{
	double value;
	std::istringstream sstream (std::string {str.data (), str.size ()});
	sstream.imbue (std::locale::classic ());
	sstream >> value;
	if (sstream.fail ())
		return {};
	return value;
}

//------------------------------------------------------------------------
AllocatorFunc allocator = [] (size_t size) {
	return operator new (size);
};

//------------------------------------------------------------------------
DeallocatorFunc deallocator = [] (void* ptr, size_t size) {
	return operator delete (ptr, size);
};

//------------------------------------------------------------------------
void setCustomAllocator (AllocatorFunc&& _allocator, DeallocatorFunc&& _deallocator)
{
	allocator = std::move (_allocator);
	deallocator = std::move (_deallocator);
}

// -----------------------------------------------------------------------------------
// CSCRIPTEXCEPTION

CScriptException::CScriptException (const string& exceptionText) { text = exceptionText; }
CScriptException::CScriptException (string&& exceptionText) { text = std::move (exceptionText); }
CScriptException::~CScriptException () noexcept {}

void* CScriptException::operator new (std::size_t count) { return allocator (count); }
void CScriptException::operator delete (void* ptr, std::size_t size) { deallocator (ptr, size); }

struct LexTokenDef
{
	LexType type;
	std::string_view name;
};

static std::array<LexTokenDef, asInteger (LexType::LIST_END)> gLexTokens = {{
	{LexType::Eof, "EOF"sv},
	{LexType::ID, "ID"sv},
	{LexType::INT, "INT"sv},
	{LexType::FLOAT, "FLOAT"sv},
	{LexType::STR, "STRING"sv},
	{LexType::EQUAL, "=="sv},
	{LexType::TYPEEQUAL, "==="sv},
	{LexType::NEQUAL, "!="sv},
	{LexType::NTYPEEQUAL, "!=="sv},
	{LexType::LEQUAL, "<="sv},
	{LexType::LSHIFT, "<<"sv},
	{LexType::LSHIFTEQUAL, "<<="sv},
	{LexType::GEQUAL, ">="sv},
	{LexType::RSHIFT, ">>"sv},
	{LexType::RSHIFTUNSIGNED, ">>"sv},
	{LexType::RSHIFTEQUAL, ">>="sv},
	{LexType::PLUSEQUAL, "+="sv},
	{LexType::MINUSEQUAL, "-="sv},
	{LexType::PLUSPLUS, "++"sv},
	{LexType::MINUSMINUS, "--"sv},
	{LexType::ANDEQUAL, "&="sv},
	{LexType::ANDAND, "&&"sv},
	{LexType::OREQUAL, "|="sv},
	{LexType::OROR, "||"sv},
	{LexType::XOREQUAL, "^="sv},

	{LexType::R_IF, "if"sv},
	{LexType::R_ELSE, "else"sv},
	{LexType::R_DO, "do"sv},
	{LexType::R_WHILE, "while"sv},
	{LexType::R_FOR, "for"sv},
	{LexType::R_BREAK, "break"sv},
	{LexType::R_CONTINUE, "continue"sv},
	{LexType::R_FUNCTION, "function"sv},
	{LexType::R_RETURN, "return"sv},
	{LexType::R_VAR, "var"sv},
	{LexType::R_TRUE, "true"sv},
	{LexType::R_FALSE, "false"sv},
	{LexType::R_NULL, "null"sv},
	{LexType::R_UNDEFINED, "undefined"sv},
	{LexType::R_NEW, "new"sv},
}};

inline constexpr std::string_view findTokenName (LexType t)
{
	for (const auto& el : gLexTokens)
	{
		if (el.type == t)
			return el.name;
	}
	return {};
}

inline constexpr LexType findTokenType (std::string_view token, LexType notFound = LexType::ID)
{
	for (const auto& el : gLexTokens)
	{
		if (el.name == token)
			return el.type;
	}
	return notFound;
}

// ----------------------------------------------------------------------------------- CSCRIPTLEX

CScriptLex::CScriptLex (std::string_view input)
{
	data = input.data ();
	dataEnd = input.size ();
	reset ();
}

CScriptLex::~CScriptLex (void) {}

void* CScriptLex::operator new (std::size_t count) { return allocator (count); }
void CScriptLex::operator delete (void* ptr, std::size_t size) { deallocator (ptr, size); }

void CScriptLex::reset ()
{
	dataPos = 0;
	tokenStart = 0;
	tokenEnd = 0;
	tokenLastEnd = 0;
	token = 0;
	tkStr = "";
	getNextCh ();
	getNextCh ();
	getNextToken ();
}

void CScriptLex::match (int expected_tk)
{
	if (token != expected_tk)
	{
		ostringstream errorString;
		errorString << "Got " << getTokenStr (token) << " expected " << getTokenStr (expected_tk)
					<< " at " << getPosition (tokenStart);
		throw CScriptException (errorString.str ());
	}
	getNextToken ();
}

void CScriptLex::match (LexType expected_tk)
{
	if (token != asInteger (expected_tk))
	{
		ostringstream errorString;
		errorString << "Got " << getTokenStr (token) << " expected " << findTokenName (expected_tk)
					<< " at " << getPosition (tokenStart);
		throw CScriptException (errorString.str ());
	}
	getNextToken ();
}

string CScriptLex::getTokenStr (int token)
{
	if (token > 32 && token < 128)
	{
		string str ("' '");
		str[1] = static_cast<char> (token);
		return str;
	}
	auto tokenString = findTokenName (asLexType (token));
	if (!tokenString.empty ())
		return {tokenString.data (), tokenString.size ()};

	ostringstream msg;
	msg << "?[" << token << "]";
	return msg.str ();
}

void CScriptLex::getNextCh ()
{
	currCh = nextCh;
	if (dataPos < dataEnd)
		nextCh = data[dataPos];
	else
		nextCh = 0;
	if (nextCh == '\n')
		newLinePositions.push_back (dataPos);
	dataPos++;
}

void CScriptLex::getNextToken ()
{
	token = asInteger (LexType::Eof);
	tkStr.clear ();
	while (currCh && isWhitespace (currCh))
		getNextCh ();
	// newline comments
	if (currCh == '/' && nextCh == '/')
	{
		while (currCh && currCh != '\n')
			getNextCh ();
		getNextCh ();
		getNextToken ();
		return;
	}
	// block comments
	if (currCh == '/' && nextCh == '*')
	{
		while (currCh && (currCh != '*' || nextCh != '/'))
			getNextCh ();
		getNextCh ();
		getNextCh ();
		getNextToken ();
		return;
	}
	// record beginning of this token
	tokenStart = dataPos - 2;
	// tokens
	if (isAlpha (currCh))
	{ //  IDs
		while (isAlpha (currCh) || isNumeric (currCh))
		{
			tkStr += currCh;
			getNextCh ();
		}
		token = asInteger (findTokenType (tkStr));
	}
	else if (isNumeric (currCh))
	{ // Numbers
		bool isHex = false;
		if (currCh == '0')
		{
			tkStr += currCh;
			getNextCh ();
		}
		if (currCh == 'x')
		{
			isHex = true;
			tkStr += currCh;
			getNextCh ();
		}
		token = asInteger (LexType::INT);
		while (isNumeric (currCh) || (isHex && isHexadecimal (currCh)))
		{
			tkStr += currCh;
			getNextCh ();
		}
		if (!isHex && currCh == '.')
		{
			token = asInteger (LexType::FLOAT);
			tkStr += '.';
			getNextCh ();
			while (isNumeric (currCh))
			{
				tkStr += currCh;
				getNextCh ();
			}
		}
		// do fancy e-style floating point
		if (!isHex && (currCh == 'e' || currCh == 'E'))
		{
			token = asInteger (LexType::FLOAT);
			tkStr += currCh;
			getNextCh ();
			if (currCh == '-')
			{
				tkStr += currCh;
				getNextCh ();
			}
			while (isNumeric (currCh))
			{
				tkStr += currCh;
				getNextCh ();
			}
		}
	}
	else if (currCh == '"')
	{
		// strings...
		getNextCh ();
		while (currCh && currCh != '"')
		{
			if (currCh == '\\')
			{
				getNextCh ();
				switch (currCh)
				{
					case 'n':
						tkStr += '\n';
						break;
					case '"':
						tkStr += '"';
						break;
					case '\\':
						tkStr += '\\';
						break;
					default:
						tkStr += currCh;
				}
			}
			else
			{
				tkStr += currCh;
			}
			getNextCh ();
		}
		getNextCh ();
		token = asInteger (LexType::STR);
	}
	else if (currCh == '\'')
	{
		// strings again...
		getNextCh ();
		while (currCh && currCh != '\'')
		{
			if (currCh == '\\')
			{
				getNextCh ();
				switch (currCh)
				{
					case 'n':
						tkStr += '\n';
						break;
					case 'a':
						tkStr += '\a';
						break;
					case 'r':
						tkStr += '\r';
						break;
					case 't':
						tkStr += '\t';
						break;
					case '\'':
						tkStr += '\'';
						break;
					case '\\':
						tkStr += '\\';
						break;
					case 'x':
					{ // hex digits
						char buf[3] = "??";
						getNextCh ();
						buf[0] = currCh;
						getNextCh ();
						buf[1] = currCh;
						tkStr += (char)strtol (buf, 0, 16);
					}
					break;
					default:
						if (currCh >= '0' && currCh <= '7')
						{
							// octal digits
							char buf[4] = "???";
							buf[0] = currCh;
							getNextCh ();
							buf[1] = currCh;
							getNextCh ();
							buf[2] = currCh;
							tkStr += (char)strtol (buf, 0, 8);
						}
						else
							tkStr += currCh;
				}
			}
			else
			{
				tkStr += currCh;
			}
			getNextCh ();
		}
		getNextCh ();
		token = asInteger (LexType::STR);
	}
	else
	{
		// single chars
		token = currCh;
		if (currCh)
			getNextCh ();
		if (token == '=' && currCh == '=')
		{ // ==
			token = asInteger (LexType::EQUAL);
			getNextCh ();
			if (currCh == '=')
			{ // ===
				token = asInteger (LexType::TYPEEQUAL);
				getNextCh ();
			}
		}
		else if (token == '!' && currCh == '=')
		{ // !=
			token = asInteger (LexType::NEQUAL);
			getNextCh ();
			if (currCh == '=')
			{ // !==
				token = asInteger (LexType::NTYPEEQUAL);
				getNextCh ();
			}
		}
		else if (token == '<' && currCh == '=')
		{
			token = asInteger (LexType::LEQUAL);
			getNextCh ();
		}
		else if (token == '<' && currCh == '<')
		{
			token = asInteger (LexType::LSHIFT);
			getNextCh ();
			if (currCh == '=')
			{ // <<=
				token = asInteger (LexType::LSHIFTEQUAL);
				getNextCh ();
			}
		}
		else if (token == '>' && currCh == '=')
		{
			token = asInteger (LexType::GEQUAL);
			getNextCh ();
		}
		else if (token == '>' && currCh == '>')
		{
			token = asInteger (LexType::RSHIFT);
			getNextCh ();
			if (currCh == '=')
			{ // >>=
				token = asInteger (LexType::RSHIFTEQUAL);
				getNextCh ();
			}
			else if (currCh == '>')
			{ // >>>
				token = asInteger (LexType::RSHIFTUNSIGNED);
				getNextCh ();
			}
		}
		else if (token == '+' && currCh == '=')
		{
			token = asInteger (LexType::PLUSEQUAL);
			getNextCh ();
		}
		else if (token == '-' && currCh == '=')
		{
			token = asInteger (LexType::MINUSEQUAL);
			getNextCh ();
		}
		else if (token == '+' && currCh == '+')
		{
			token = asInteger (LexType::PLUSPLUS);
			getNextCh ();
		}
		else if (token == '-' && currCh == '-')
		{
			token = asInteger (LexType::MINUSMINUS);
			getNextCh ();
		}
		else if (token == '&' && currCh == '=')
		{
			token = asInteger (LexType::ANDEQUAL);
			getNextCh ();
		}
		else if (token == '&' && currCh == '&')
		{
			token = asInteger (LexType::ANDAND);
			getNextCh ();
		}
		else if (token == '|' && currCh == '=')
		{
			token = asInteger (LexType::OREQUAL);
			getNextCh ();
		}
		else if (token == '|' && currCh == '|')
		{
			token = asInteger (LexType::OROR);
			getNextCh ();
		}
		else if (token == '^' && currCh == '=')
		{
			token = asInteger (LexType::XOREQUAL);
			getNextCh ();
		}
	}
	/* This isn't quite right yet */
	tokenLastEnd = tokenEnd;
	tokenEnd = dataPos - 3;
}

string CScriptLex::getSubString (size_t lastPosition) const
{
	size_t lastCharIdx = tokenLastEnd + 1;
	if (lastCharIdx < dataEnd)
	{
		string value (&data[lastPosition], lastCharIdx - lastPosition);
		return value;
	}
	else
	{
		return string (&data[lastPosition]);
	}
}

CScriptLex* CScriptLex::getSubLex (size_t lastPosition) const
{
	size_t lastCharIdx = tokenLastEnd + 1;
	if (lastCharIdx < dataEnd)
		return new CScriptLex ({data + lastPosition, lastCharIdx - lastPosition});
	else
		return new CScriptLex ({data + lastPosition, dataEnd - lastPosition});
}

string CScriptLex::getPosition (size_t pos) const
{
	if (pos == std::numeric_limits<size_t>::max ())
		pos = tokenLastEnd;
	size_t lineNo = 1, colNo = 1;

	size_t lastLinePos = 0;
	for (const auto& nlPos : newLinePositions)
	{
		if (pos < nlPos)
			break;
		++lineNo;
		lastLinePos = nlPos + 1;
	}
	colNo = pos - lastLinePos;

	return string ("(line: ") + std::to_string (lineNo).data () + string (", col: ") +
		   std::to_string (colNo).data () + string (")");
}

// -----------------------------------------------------------------------------------
// CSCRIPTVARLINK

CScriptVarLink::CScriptVarLink (CScriptVar* inVar, const string& inName, bool own)
: name (inName), var (owning (inVar)), isOwned (own)
{
#if DEBUG_MEMORY
	mark_allocated (this);
#endif
}

CScriptVarLink::CScriptVarLink (const CScriptVarLink& link)
: name (link.name), var (owning (link.var))
{
#if DEBUG_MEMORY
	mark_allocated (this);
#endif
}

CScriptVarLink::~CScriptVarLink ()
{
#if DEBUG_MEMORY
	mark_deallocated (this);
#endif
	var->release ();
}

void* CScriptVarLink::operator new (std::size_t count) { return allocator (count); }
void CScriptVarLink::operator delete (void* ptr, std::size_t size) { deallocator (ptr, size); }

void CScriptVarLink::replaceWith (CScriptVar* newVar)
{
	CScriptVar* oldVar = var;
	var = owning (newVar);
	oldVar->release ();
}

void CScriptVarLink::replaceWith (CScriptVarLink* newVar)
{
	if (newVar)
		replaceWith (newVar->var);
	else
	{
		auto v = owning (new CScriptVar ());
		replaceWith (v);
		v->release ();
	}
}

int CScriptVarLink::getIntName () const { return atoi (name.c_str ()); }

void CScriptVarLink::setIntName (int n)
{
	char sIdx[64];
	sprintf_s (sIdx, sizeof (sIdx), "%d", n);
	name = sIdx;
}

void CScriptVarLink::setVar (CScriptVar* v)
{
	ASSERT (v != nullptr);
	replaceWith (v);
}

// ----------------------------------------------------------------------------------- CSCRIPTVAR

CScriptVar::CScriptVar ()
{
#if DEBUG_MEMORY
	mark_allocated (this);
#endif
	flags = SCRIPTVAR_UNDEFINED;
}

CScriptVar::CScriptVar (std::string_view str)
{
#if DEBUG_MEMORY
	mark_allocated (this);
#endif
	flags = SCRIPTVAR_STRING;
	variant = string (str.data (), str.size ());
}

CScriptVar::CScriptVar (const string& varData, int varFlags)
{
#if DEBUG_MEMORY
	mark_allocated (this);
#endif
	flags = varFlags;
	if (varFlags & SCRIPTVAR_INTEGER)
	{
		variant = stringToInteger (varData);
	}
	else if (varFlags & SCRIPTVAR_DOUBLE)
	{
		variant = stringToDouble (varData);
	}
	else if (varFlags & SCRIPTVAR_STRING)
	{
		variant = varData;
	}
	else if (!varData.empty ())
	{
		ASSERT (false);
	}
}

CScriptVar::CScriptVar (double val)
{
#if DEBUG_MEMORY
	mark_allocated (this);
#endif
	setDouble (val);
}

CScriptVar::CScriptVar (int64_t val)
{
#if DEBUG_MEMORY
	mark_allocated (this);
#endif
	setInt (val);
}

CScriptVar::CScriptVar (bool val)
{
#if DEBUG_MEMORY
	mark_allocated (this);
#endif
	setInt (val);
}

CScriptVar::~CScriptVar (void)
{
	if (lifeTimeObserver)
		lifeTimeObserver->onDestroy (this);
#if DEBUG_MEMORY
	mark_deallocated (this);
#endif
	removeAllChildren ();
}

void* CScriptVar::operator new (std::size_t count) { return allocator (count); }
void CScriptVar::operator delete (void* ptr, std::size_t size) { deallocator (ptr, size); }

CScriptVar* CScriptVar::getReturnVar () { return getParameter (TINYJS_RETURN_VAR); }

void CScriptVar::setReturnVar (CScriptVar* var)
{
	findChildOrCreate (TINYJS_RETURN_VAR)->replaceWith (var);
}

CScriptVar* CScriptVar::getParameter (std::string_view name)
{
	return findChildOrCreate (name)->getVar ();
}

CScriptVarLink* CScriptVar::findChild (std::string_view childName)
{
	CScriptVarLink* v = firstChild;
	while (v)
	{
		if (v->getName ().compare (childName) == 0)
			return v;
		v = v->getNextSibling ();
	}
	return 0;
}

CScriptVarLink* CScriptVar::findChildOrCreate (std::string_view childName, int varFlags)
{
	CScriptVarLink* l = findChild (childName);
	if (l)
		return l;

	return addChild (childName, new CScriptVar (TINYJS_BLANK_DATA, varFlags));
}

CScriptVarLink* CScriptVar::findChildOrCreateByPath (const string& path)
{
	size_t p = path.find ('.');
	if (p == string::npos)
		return findChildOrCreate (path);

	return findChildOrCreate (path.substr (0, p), SCRIPTVAR_OBJECT)
		->getVar ()
		->findChildOrCreateByPath (path.substr (p + 1));
}

CScriptVarLink* CScriptVar::addChild (std::string_view childName, CScriptVar* child)
{
	if (isUndefined ())
	{
		flags = SCRIPTVAR_OBJECT;
	}
	// if no child supplied, create one
	if (!child)
		child = new CScriptVar ();

	CScriptVarLink* link = new CScriptVarLink (child, {childName.data (), childName.size ()}, true);
	if (lastChild)
	{
		lastChild->setNextSibling (link);
		link->setPrevSibling (lastChild);
		lastChild = link;
	}
	else
	{
		firstChild = link;
		lastChild = link;
	}
	return link;
}

CScriptVarLink* CScriptVar::addChildNoDup (std::string_view childName, CScriptVar* child)
{
	// if no child supplied, create one
	if (!child)
		child = new CScriptVar ();

	CScriptVarLink* v = findChild (childName);
	if (v)
	{
		v->replaceWith (child);
	}
	else
	{
		v = addChild (childName, child);
	}

	return v;
}

void CScriptVar::removeChild (CScriptVar* child)
{
	CScriptVarLink* link = firstChild;
	while (link)
	{
		if (link->getVar () == child)
			break;
		link = link->getNextSibling ();
	}
	ASSERT (link);
	removeLink (link);
}

void CScriptVar::removeLink (CScriptVarLink* link)
{
	if (!link)
		return;
	if (auto sibling = link->getNextSibling ())
		sibling->setPrevSibling (link->getPrevSibling ());
	if (auto sibling = link->getPrevSibling ())
		sibling->setNextSibling (link->getNextSibling ());
	if (lastChild == link)
		lastChild = link->getPrevSibling ();
	if (firstChild == link)
		firstChild = link->getNextSibling ();
	delete link;
}

void CScriptVar::removeAllChildren ()
{
	CScriptVarLink* c = firstChild;
	while (c)
	{
		CScriptVarLink* t = c->getNextSibling ();
		delete c;
		c = t;
	}
	firstChild = nullptr;
	lastChild = nullptr;
}

CScriptVar* CScriptVar::getArrayIndex (int idx)
{
	char sIdx[64];
	sprintf_s (sIdx, sizeof (sIdx), "%d", idx);
	CScriptVarLink* link = findChild (sIdx);
	if (link)
		return link->getVar ();
	else
		return new CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_NULL); // undefined
}

void CScriptVar::setArrayIndex (int idx, CScriptVar* value)
{
	char sIdx[64];
	sprintf_s (sIdx, sizeof (sIdx), "%d", idx);
	CScriptVarLink* link = findChild (sIdx);

	if (link)
	{
		if (value->isUndefined ())
			removeLink (link);
		else
			link->replaceWith (value);
	}
	else
	{
		if (!value->isUndefined ())
			addChild (sIdx, value);
	}
}

int CScriptVar::getArrayLength ()
{
	int highest = -1;
	if (!isArray ())
		return 0;

	CScriptVarLink* link = firstChild;
	while (link)
	{
		if (isNumber (link->getName ()))
		{
			int val = atoi (link->getName ().c_str ());
			if (val > highest)
				highest = val;
		}
		link = link->getNextSibling ();
	}
	return highest + 1;
}

int CScriptVar::getChildren ()
{
	int n = 0;
	CScriptVarLink* link = firstChild;
	while (link)
	{
		n++;
		link = link->getNextSibling ();
	}
	return n;
}

int64_t CScriptVar::getInt ()
{
	if (isInt ())
		return std::get<int64_t> (variant);
	if (isNull ())
		return 0;
	if (isUndefined ())
		return 0;
	if (isDouble ())
		return static_cast<int> (std::get<double> (variant));
	return 0;
}

double CScriptVar::getDouble ()
{
	if (isDouble ())
		return std::get<double> (variant);
	if (isInt ())
		return static_cast<double> (std::get<int64_t> (variant));
	if (isNull ())
		return 0;
	if (isUndefined ())
		return 0;
	return 0; /* or NaN? */
}

const string& CScriptVar::getString ()
{
	/* Because we can't return a string that is generated on demand.
	 * I should really just use char* :) */
	static string s_null = "null";
	static string s_undefined = "undefined";
	if (isInt ())
	{
		dataStr = std::to_string (std::get<int64_t> (variant));
		return dataStr;
	}
	if (isDouble ())
	{
		char buffer[32];
		sprintf_s (buffer, sizeof (buffer), "%f", std::get<double> (variant));
		dataStr = buffer;
		return dataStr;
	}
	if (isNull ())
		return s_null;
	if (isUndefined ())
		return s_undefined;
	if (auto s = std::get_if<string> (&variant))
		return *s;
	return s_undefined;
}

void CScriptVar::setInt (int64_t val)
{
	flags = (flags & ~SCRIPTVAR_VARTYPEMASK) | SCRIPTVAR_INTEGER;
	variant = val;
}

void CScriptVar::setDouble (double val)
{
	flags = (flags & ~SCRIPTVAR_VARTYPEMASK) | SCRIPTVAR_DOUBLE;
	variant = val;
}

void CScriptVar::setString (std::string_view str)
{
	// name sure it's not still a number or integer
	flags = (flags & ~SCRIPTVAR_VARTYPEMASK) | SCRIPTVAR_STRING;
	variant = string (str);
}

void CScriptVar::setUndefined ()
{
	// name sure it's not still a number or integer
	flags = (flags & ~SCRIPTVAR_VARTYPEMASK) | SCRIPTVAR_UNDEFINED;
	variant = TINYJS_BLANK_DATA;
	removeAllChildren ();
}

void CScriptVar::setArray ()
{
	// name sure it's not still a number or integer
	flags = (flags & ~SCRIPTVAR_VARTYPEMASK) | SCRIPTVAR_ARRAY;
	variant = TINYJS_BLANK_DATA;
	removeAllChildren ();
}

bool CScriptVar::equals (CScriptVar* v)
{
	CScriptVar* resV = mathsOp (v, asInteger (LexType::EQUAL));
	bool res = resV->getBool ();
	delete resV;
	return res;
}

CScriptVar* CScriptVar::mathsOp (CScriptVar* b, int op)
{
	CScriptVar* a = this;
	// Type equality check
	if (op == asInteger (LexType::TYPEEQUAL) || op == asInteger (LexType::NTYPEEQUAL))
	{
		// check type first, then call again to check data
		bool eql = ((a->flags & SCRIPTVAR_VARTYPEMASK) == (b->flags & SCRIPTVAR_VARTYPEMASK));
		if (eql)
		{
			CScriptVar* contents = a->mathsOp (b, asInteger (LexType::EQUAL));
			if (!contents->getBool ())
				eql = false;
			if (!contents->refs)
				delete contents;
		};
		if (op == asInteger (LexType::TYPEEQUAL))
			return new CScriptVar (eql);
		else
			return new CScriptVar (!eql);
	}
	// do maths...
	if (a->isUndefined () && b->isUndefined ())
	{
		if (op == asInteger (LexType::EQUAL))
			return new CScriptVar (true);
		else if (op == asInteger (LexType::NEQUAL))
			return new CScriptVar (false);
		else
			return new CScriptVar (); // undefined
	}
	else if ((a->isNumeric () || a->isUndefined ()) && (b->isNumeric () || b->isUndefined ()))
	{
		if (!a->isDouble () && !b->isDouble ())
		{
			// use ints
			auto da = a->getInt ();
			auto db = b->getInt ();
			switch (op)
			{
				case '+':
					return new CScriptVar (da + db);
				case '-':
					return new CScriptVar (da - db);
				case '*':
					return new CScriptVar (da * db);
				case '/':
					return new CScriptVar (da / db);
				case '&':
					return new CScriptVar (da & db);
				case '|':
					return new CScriptVar (da | db);
				case '^':
					return new CScriptVar (da ^ db);
				case '%':
					return new CScriptVar (da % db);
				case asInteger (LexType::EQUAL):
					return new CScriptVar (da == db);
				case asInteger (LexType::NEQUAL):
					return new CScriptVar (da != db);
				case '<':
					return new CScriptVar (da < db);
				case asInteger (LexType::LEQUAL):
					return new CScriptVar (da <= db);
				case '>':
					return new CScriptVar (da > db);
				case asInteger (LexType::GEQUAL):
					return new CScriptVar (da >= db);
				default:
					throw CScriptException ("Operation " + CScriptLex::getTokenStr (op) +
											" not supported on the Int datatype");
			}
		}
		else
		{
			// use doubles
			double da = a->getDouble ();
			double db = b->getDouble ();
			switch (op)
			{
				case '+':
					return new CScriptVar (da + db);
				case '-':
					return new CScriptVar (da - db);
				case '*':
					return new CScriptVar (da * db);
				case '/':
					return new CScriptVar (da / db);
				case asInteger (LexType::EQUAL):
					return new CScriptVar (da == db);
				case asInteger (LexType::NEQUAL):
					return new CScriptVar (da != db);
				case '<':
					return new CScriptVar (da < db);
				case asInteger (LexType::LEQUAL):
					return new CScriptVar (da <= db);
				case '>':
					return new CScriptVar (da > db);
				case asInteger (LexType::GEQUAL):
					return new CScriptVar (da >= db);
				default:
					throw CScriptException ("Operation " + CScriptLex::getTokenStr (op) +
											" not supported on the Double datatype");
			}
		}
	}
	else if (a->isArray ())
	{
		/* Just check pointers */
		switch (op)
		{
			case asInteger (LexType::EQUAL):
				return new CScriptVar (a == b);
			case asInteger (LexType::NEQUAL):
				return new CScriptVar (a != b);
			default:
				throw CScriptException ("Operation " + CScriptLex::getTokenStr (op) +
										" not supported on the Array datatype");
		}
	}
	else if (a->isObject ())
	{
		/* Just check pointers */
		switch (op)
		{
			case asInteger (LexType::EQUAL):
				return new CScriptVar (a == b);
			case asInteger (LexType::NEQUAL):
				return new CScriptVar (a != b);
			default:
				throw CScriptException ("Operation " + CScriptLex::getTokenStr (op) +
										" not supported on the Object datatype");
		}
	}
	else
	{
		string da = a->getString ();
		string db = b->getString ();
		// use strings
		switch (op)
		{
			case '+':
				return new CScriptVar (da + db, SCRIPTVAR_STRING);
			case asInteger (LexType::EQUAL):
				return new CScriptVar (da == db);
			case asInteger (LexType::NEQUAL):
				return new CScriptVar (da != db);
			case '<':
				return new CScriptVar (da < db);
			case asInteger (LexType::LEQUAL):
				return new CScriptVar (da <= db);
			case '>':
				return new CScriptVar (da > db);
			case asInteger (LexType::GEQUAL):
				return new CScriptVar (da >= db);
			default:
				throw CScriptException ("Operation " + CScriptLex::getTokenStr (op) +
										" not supported on the string datatype");
		}
	}
	ASSERT (0);
	return 0;
}

void CScriptVar::copySimpleData (CScriptVar* val)
{
	variant = val->variant;
	customData = val->customData;
	flags = (flags & ~SCRIPTVAR_VARTYPEMASK) | (val->flags & SCRIPTVAR_VARTYPEMASK);
}

void CScriptVar::copyValue (CScriptVar* val)
{
	if (val)
	{
		copySimpleData (val);
		// remove all current children
		removeAllChildren ();
		// copy children of 'val'
		CScriptVarLink* child = val->firstChild;
		while (child)
		{
			CScriptVar* copied;
			// don't copy the 'parent' object...
			if (child->getName () != TINYJS_PROTOTYPE_CLASS)
				copied = child->getVar ()->deepCopy ();
			else
				copied = child->getVar ();

			addChild (child->getName (), copied);

			child = child->getNextSibling ();
		}
	}
	else
	{
		setUndefined ();
	}
}

CScriptVar* CScriptVar::deepCopy ()
{
	CScriptVar* newVar = new CScriptVar ();
	newVar->copySimpleData (this);
	// copy children
	CScriptVarLink* child = firstChild;
	while (child)
	{
		CScriptVar* copied;
		// don't copy the 'parent' object...
		if (child->getName () != TINYJS_PROTOTYPE_CLASS)
			copied = child->getVar ()->deepCopy ();
		else
			copied = child->getVar ();

		newVar->addChild (child->getName (), copied);
		child = child->getNextSibling ();
	}
	return newVar;
}

void CScriptVar::trace (string indentStr, const string& name)
{
	TRACE ("%s'%s' = '%s' %s\n", indentStr.c_str (), name.c_str (), getString ().c_str (),
		   getFlagsAsString ().c_str ());
	string indent = indentStr + " ";
	CScriptVarLink* link = firstChild;
	while (link)
	{
		link->getVar ()->trace (indent, link->getName ());
		link = link->getNextSibling ();
	}
}

string CScriptVar::getFlagsAsString ()
{
	string flagstr = "";
	if (flags & SCRIPTVAR_FUNCTION)
		flagstr = flagstr + "FUNCTION ";
	if (flags & SCRIPTVAR_OBJECT)
		flagstr = flagstr + "OBJECT ";
	if (flags & SCRIPTVAR_ARRAY)
		flagstr = flagstr + "ARRAY ";
	if (flags & SCRIPTVAR_NATIVE)
		flagstr = flagstr + "NATIVE ";
	if (flags & SCRIPTVAR_DOUBLE)
		flagstr = flagstr + "DOUBLE ";
	if (flags & SCRIPTVAR_INTEGER)
		flagstr = flagstr + "INTEGER ";
	if (flags & SCRIPTVAR_STRING)
		flagstr = flagstr + "STRING ";
	return flagstr;
}

string CScriptVar::getParsableString ()
{
	// Numbers can just be put in directly
	if (isNumeric ())
		return getString ();
	if (isFunction ())
	{
		ostringstream funcStr;
		funcStr << "function (";
		// get list of parameters
		CScriptVarLink* link = firstChild;
		while (link)
		{
			funcStr << link->getName ();
			if (link->getNextSibling ())
				funcStr << ",";
			link = link->getNextSibling ();
		}
		// add function body
		funcStr << ") " << (isNative () ? "{ /* native code */ }" : getString ());
		return funcStr.str ();
	}
	// if it is a string then we quote it
	if (isString ())
		return getJSString (getString ());
	if (isNull ())
		return "null";
	return "undefined";
}

void CScriptVar::getJSON (std::ostream& destination, const string linePrefix)
{
	if (isObject ())
	{
		string indentedLinePrefix = linePrefix + "  ";
		// children - handle with bracketed list
		destination << "{ \n";
		CScriptVarLink* link = firstChild;
		while (link)
		{
			destination << indentedLinePrefix;
			destination << getJSString (link->getName ());
			destination << " : ";
			link->getVar ()->getJSON (destination, indentedLinePrefix);
			link = link->getNextSibling ();
			if (link)
			{
				destination << ",\n";
			}
		}
		destination << "\n" << linePrefix << "}";
	}
	else if (isArray ())
	{
		string indentedLinePrefix = linePrefix + "  ";
		destination << "[\n";
		int len = getArrayLength ();
		if (len > 10000)
			len = 10000; // we don't want to get stuck here!

		for (int i = 0; i < len; i++)
		{
			getArrayIndex (i)->getJSON (destination, indentedLinePrefix);
			if (i < len - 1)
				destination << ",\n";
		}

		destination << "\n" << linePrefix << "]";
	}
	else
	{
		// no children or a function... just write value directly
		destination << getParsableString ();
	}
}

void CScriptVar::setCallback (const JSCallback& callback) { variant = callback; }

void CScriptVar::setCallback (JSCallback&& callback) { variant = std::move (callback); }

void CScriptVar::callCallback (CScriptVar* var)
{
	if (auto call = std::get_if<JSCallback> (&variant))
	{
		(*call) (var);
		return;
	}
	ASSERT (false); // calling a non existing native function. callers must check this first
}

void CScriptVar::setFunctionScript (std::string_view str)
{
	ASSERT (isFunction ());
	variant = string {str.data (), str.size ()};
}

CScriptVar* CScriptVar::addRef ()
{
	refs++;
	return this;
}

void CScriptVar::release ()
{
	if (refs <= 0)
		printf ("OMFG, we have unreffed too far!\n");
	if ((--refs) == 0)
	{
		delete this;
	}
}

int CScriptVar::getRefs () { return refs; }

// ----------------------------------------------------------------------------------- CSCRIPT

CTinyJS::CTinyJS ()
{
	root = owning (new CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT));
	// Add built-in classes
	stringClass = owning (new CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT));
	arrayClass = owning (new CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT));
	objectClass = owning (new CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT));
	root->addChild ("String", stringClass);
	root->addChild ("Array", arrayClass);
	root->addChild ("Object", objectClass);
}

CTinyJS::~CTinyJS ()
{
	ASSERT (!lexer);
	scopes.clear ();
	stringClass->release ();
	arrayClass->release ();
	objectClass->release ();
	root->release ();

#if DEBUG_MEMORY
	show_allocated ();
#endif
}

void* CTinyJS::operator new (std::size_t count) { return allocator (count); }
void CTinyJS::operator delete (void* ptr, std::size_t size) { deallocator (ptr, size); }

void CTinyJS::trace () { root->trace (); }

void CTinyJS::execute (const string& code)
{
	CScriptLex* oldLex = lexer;
	std::vector<CScriptVar*> oldScopes = std::move (scopes);
	lexer = new CScriptLex (code);
#ifdef TINYJS_CALL_STACK
	call_stack.clear ();
#endif
	scopes.clear ();
	scopes.push_back (root);
	try
	{
		bool execute = true;
		while (lexer->getToken ())
			statement (execute);
	}
	catch (CScriptException& e)
	{
		ostringstream msg;
		msg << "Error " << e.text;
#ifdef TINYJS_CALL_STACK
		for (int i = (int)call_stack.size () - 1; i >= 0; i--)
			msg << "\n" << i << ": " << call_stack.at (i);
#else
		msg << " at " << lexer->getPosition ();
#endif
		delete lexer;
		lexer = oldLex;

		throw CScriptException (msg.str ());
	}
	delete lexer;
	lexer = oldLex;
	scopes = std::move (oldScopes);
}

CScriptVarLink CTinyJS::evaluateComplex (std::string_view code)
{
	CScriptLex* oldLex = lexer;
	std::vector<CScriptVar*> oldScopes = std::move (scopes);

	lexer = new CScriptLex (code);
#ifdef TINYJS_CALL_STACK
	call_stack.clear ();
#endif
	scopes.clear ();
	scopes.push_back (root);
	CScriptVarLink* v = nullptr;
	try
	{
		bool execute = true;
		do
		{
			CLEAN (v);
			v = base (execute);
			if (lexer->getToken () != asInteger (LexType::Eof))
				lexer->match (';');
		} while (lexer->getToken () != asInteger (LexType::Eof));
	}
	catch (CScriptException& e)
	{
		ostringstream msg;
		msg << "Error: " << e.text;
#ifdef TINYJS_CALL_STACK
		for (int i = (int)call_stack.size () - 1; i >= 0; i--)
			msg << "\n" << i << ": " << call_stack.at (i);
#else
		msg << " at " << lexer->getPosition ();
#endif
		delete lexer;
		lexer = oldLex;

		throw CScriptException (msg.str ());
	}
	delete lexer;
	lexer = oldLex;
	scopes = std::move (oldScopes);

	if (v)
	{
		CScriptVarLink r = *v;
		CLEAN (v);
		return r;
	}
	// return undefined...
	return CScriptVarLink (new CScriptVar ());
}

string CTinyJS::evaluate (std::string_view code)
{
	return evaluateComplex (code).getVar ()->getString ();
}

void CTinyJS::parseFunctionArguments (CScriptVar* funcVar) const
{
	lexer->match ('(');
	while (lexer->getToken () != ')')
	{
		funcVar->addChildNoDup (lexer->getTokenString ());
		lexer->match (asInteger (LexType::ID));
		if (lexer->getToken () != ')')
			lexer->match (',');
	}
	lexer->match (')');
}

void CTinyJS::addNative (std::string_view funcDesc, const JSCallback& ptr)
{
	CScriptLex* oldLex = lexer;
	lexer = new CScriptLex (funcDesc);

	CScriptVar* base = root;

	lexer->match (LexType::R_FUNCTION);
	string funcName = lexer->getTokenString ();
	lexer->match (LexType::ID);
	/* Check for dots, we might want to do something like function String.substring ... */
	while (lexer->getToken () == '.')
	{
		lexer->match ('.');
		CScriptVarLink* link = base->findChild (funcName);
		// if it doesn't exist, make an object class
		if (!link)
			link = base->addChild (funcName, new CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT));
		base = link->getVar ();
		funcName = lexer->getTokenString ();
		lexer->match (LexType::ID);
	}

	CScriptVar* funcVar = new CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE);
	funcVar->setCallback (ptr);
	parseFunctionArguments (funcVar);
	delete lexer;
	lexer = oldLex;

	base->addChild (funcName, funcVar);
}

CScriptVarLink* CTinyJS::parseFunctionDefinition ()
{
	// actually parse a function...
	lexer->match (LexType::R_FUNCTION);
	string funcName = TINYJS_TEMP_NAME;
	/* we can have functions without names */
	if (lexer->getToken () == asInteger (LexType::ID))
	{
		funcName = lexer->getTokenString ();
		lexer->match (LexType::ID);
	}
	CScriptVarLink* funcVar =
		new CScriptVarLink (new CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION), funcName);
	parseFunctionArguments (funcVar->getVar ());
	size_t funcBegin = lexer->getTokenStart ();
	bool noexecute = false;
	block (noexecute);
	funcVar->getVar ()->setFunctionScript (lexer->getSubString (funcBegin));
	return funcVar;
}

/** Handle a function call (assumes we've parsed the function name and we're
 * on the start bracket). 'parent' is the object that contains this method,
 * if there was one (otherwise it's just a normnal function).
 */
CScriptVarLink* CTinyJS::functionCall (bool& execute, CScriptVarLink* function, CScriptVar* parent)
{
	if (execute)
	{
		if (!function->getVar ()->isFunction ())
		{
			string errorMsg = "Expecting '";
			errorMsg = errorMsg + function->getName () + "' to be a function";
			throw CScriptException (errorMsg.c_str ());
		}
		lexer->match ('(');
		// create a new symbol table entry for execution of this function
		CScriptVar* functionRoot = new CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION);
		if (parent)
			functionRoot->addChildNoDup ("this", parent);
		// grab in all parameters
		CScriptVarLink* v = function->getVar ()->getFirstChild ();
		while (v)
		{
			CScriptVarLink* value = base (execute);
			if (execute)
			{
				if (value->getVar ()->isBasic ())
				{
					// pass by value
					functionRoot->addChild (v->getName (), value->getVar ()->deepCopy ());
				}
				else
				{
					// pass by reference
					functionRoot->addChild (v->getName (), value->getVar ());
				}
			}
			CLEAN (value);
			v = v->getNextSibling ();
			if (lexer->getToken () == ')')
				break;
			lexer->match (',');
		}
		lexer->match (')');
		// execute function!
		// add the function's execute space to the symbol table so we can recurse
		CScriptVarLink* returnVarLink = functionRoot->addChild (TINYJS_RETURN_VAR);
		scopes.push_back (functionRoot);
#ifdef TINYJS_CALL_STACK
		call_stack.push_back (function->getName () + " " + lexer->getPosition ());
#endif

		string exceptionText;
		if (function->getVar ()->isNative ())
		{
			try
			{
				function->getVar ()->callCallback (functionRoot);
			}
			catch (CScriptException& e)
			{
				exceptionText = std::move (e.text);
			}
		}
		else
		{
			/* we just want to execute the block, but something could
			 * have messed up and left us with the wrong ScriptLex, so
			 * we want to be careful here... */
			CScriptLex* oldLex = lexer;
			CScriptLex* newLex = new CScriptLex (function->getVar ()->getString ());
			lexer = newLex;
			try
			{
				block (execute);
				// because return will probably have called this, and set execute to false
				execute = true;
			}
			catch (CScriptException& e)
			{
				exceptionText = std::move (e.text);
			}
			delete newLex;
			lexer = oldLex;
		}
		if (!exceptionText.empty ())
		{
			delete functionRoot;
			throw CScriptException (std::move (exceptionText));
		}

#ifdef TINYJS_CALL_STACK
		if (!call_stack.empty ())
			call_stack.pop_back ();
#endif
		scopes.pop_back ();
		/* get the real return var before we remove it from our function */
		auto returnVar = new CScriptVarLink (returnVarLink->getVar ());
		functionRoot->removeLink (returnVarLink);
		delete functionRoot;
		if (returnVar)
			return returnVar;
		else
			return new CScriptVarLink (new CScriptVar ());
	}
	else
	{
		// function, but not executing - just parse args and be done
		lexer->match ('(');
		while (lexer->getToken () != ')')
		{
			CScriptVarLink* value = base (execute);
			CLEAN (value);
			if (lexer->getToken () != ')')
				lexer->match (',');
		}
		lexer->match (')');
		if (lexer->getToken () == '{')
		{ // TODO: why is this here?
			block (execute);
		}
		/* function will be a blank scriptvarlink if we're not executing,
		 * so just return it rather than an alloc/free */
		return function;
	}
}

CScriptVarLink* CTinyJS::factor (bool& execute)
{
	if (lexer->getToken () == '(')
	{
		lexer->match ('(');
		CScriptVarLink* a = base (execute);
		lexer->match (')');
		return a;
	}
	if (lexer->getToken () == asInteger (LexType::R_TRUE))
	{
		lexer->match (LexType::R_TRUE);
		return new CScriptVarLink (new CScriptVar (true));
	}
	if (lexer->getToken () == asInteger (LexType::R_FALSE))
	{
		lexer->match (LexType::R_FALSE);
		return new CScriptVarLink (new CScriptVar (false));
	}
	if (lexer->getToken () == asInteger (LexType::R_NULL))
	{
		lexer->match (LexType::R_NULL);
		return new CScriptVarLink (new CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_NULL));
	}
	if (lexer->getToken () == asInteger (LexType::R_UNDEFINED))
	{
		lexer->match (LexType::R_UNDEFINED);
		return new CScriptVarLink (new CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_UNDEFINED));
	}
	if (lexer->getToken () == asInteger (LexType::ID))
	{
		CScriptVarLink* a = execute ? findInScopes (lexer->getTokenString ())
									: new CScriptVarLink (new CScriptVar ());
		// printf("0x%08X for %s at %s\n", (unsigned int)a, l->tkStr.c_str(),
		// l->getPosition().c_str());
		/* The parent if we're executing a method call */
		CScriptVar* parent = nullptr;

		if (execute && !a)
		{
			/* Variable doesn't exist! JavaScript says we should create it
			 * (we won't add it here. This is done in the assignment operator)*/
			a = new CScriptVarLink (new CScriptVar (), lexer->getTokenString ());
		}
		lexer->match (LexType::ID);
		while (lexer->getToken () == '(' || lexer->getToken () == '.' || lexer->getToken () == '[')
		{
			if (lexer->getToken () == '(')
			{ // ------------------------------------- Function Call
				a = functionCall (execute, a, parent);
			}
			else if (lexer->getToken () == '.')
			{ // ------------------------------------- Record Access
				lexer->match ('.');
				if (execute)
				{
					const string& name = lexer->getTokenString ();
					CScriptVarLink* child = a->getVar ()->findChild (name);
					if (!child)
						child = findInParentClasses (a->getVar (), name);
					if (!child)
					{
						/* if we haven't found this defined yet, use the built-in
						   'length' properly */
						if (a->getVar ()->isArray () && name == "length")
						{
							int64_t l = a->getVar ()->getArrayLength ();
							child = new CScriptVarLink (new CScriptVar (l));
						}
						else if (a->getVar ()->isString () && name == "length")
						{
							int64_t l = a->getVar ()->getString ().size ();
							child = new CScriptVarLink (new CScriptVar (l));
						}
						else
						{
							child = a->getVar ()->addChild (name);
						}
					}
					parent = a->getVar ();
					a = child;
				}
				lexer->match (LexType::ID);
			}
			else if (lexer->getToken () == '[')
			{ // ------------------------------------- Array Access
				lexer->match ('[');
				CScriptVarLink* index = base (execute);
				lexer->match (']');
				if (execute)
				{
					CScriptVarLink* child =
						a->getVar ()->findChildOrCreate (index->getVar ()->getString ());
					parent = a->getVar ();
					a = child;
				}
				CLEAN (index);
			}
			else
				ASSERT (0);
		}
		return a;
	}
	if (lexer->getToken () == asInteger (LexType::INT) ||
		lexer->getToken () == asInteger (LexType::FLOAT))
	{
		CScriptVar* a =
			new CScriptVar (lexer->getTokenString (),
							((lexer->getToken () == asInteger (LexType::INT)) ? SCRIPTVAR_INTEGER
																			  : SCRIPTVAR_DOUBLE));
		lexer->match (lexer->getToken ());
		return new CScriptVarLink (a);
	}
	if (lexer->getToken () == asInteger (LexType::STR))
	{
		CScriptVar* a = new CScriptVar (lexer->getTokenString (), SCRIPTVAR_STRING);
		lexer->match (LexType::STR);
		return new CScriptVarLink (a);
	}
	if (lexer->getToken () == '{')
	{
		CScriptVar* contents = new CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT);
		/* JSON-style object definition */
		lexer->match ('{');
		while (lexer->getToken () != '}')
		{
			string id = lexer->getTokenString ();
			// we only allow strings or IDs on the left hand side of an initialisation
			if (lexer->getToken () == asInteger (LexType::STR))
				lexer->match (LexType::STR);
			else
				lexer->match (LexType::ID);
			lexer->match (':');
			if (execute)
			{
				CScriptVarLink* a = base (execute);
				contents->addChild (id, a->getVar ());
				CLEAN (a);
			}
			// no need to clean here, as it will definitely be used
			if (lexer->getToken () != '}')
				lexer->match (',');
		}

		lexer->match ('}');
		return new CScriptVarLink (contents);
	}
	if (lexer->getToken () == '[')
	{
		CScriptVar* contents = new CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_ARRAY);
		/* JSON-style array */
		lexer->match ('[');
		int idx = 0;
		while (lexer->getToken () != ']')
		{
			if (execute)
			{
				char idx_str[16]; // big enough for 2^32
				sprintf_s (idx_str, sizeof (idx_str), "%d", idx);

				CScriptVarLink* a = base (execute);
				contents->addChild (idx_str, a->getVar ());
				CLEAN (a);
			}
			// no need to clean here, as it will definitely be used
			if (lexer->getToken () != ']')
				lexer->match (',');
			idx++;
		}
		lexer->match (']');
		return new CScriptVarLink (contents);
	}
	if (lexer->getToken () == asInteger (LexType::R_FUNCTION))
	{
		CScriptVarLink* funcVar = parseFunctionDefinition ();
		if (funcVar->getName () != TINYJS_TEMP_NAME)
			TRACE ("Functions not defined at statement-level are not meant to have a name");
		return funcVar;
	}
	if (lexer->getToken () == asInteger (LexType::R_NEW))
	{
		// new -> create a new object
		lexer->match (LexType::R_NEW);
		const string& className = lexer->getTokenString ();
		if (execute)
		{
			CScriptVarLink* objClassOrFunc = findInScopes (className);
			if (!objClassOrFunc)
			{
				TRACE ("%s is not a valid class name", className.c_str ());
				return new CScriptVarLink (new CScriptVar ());
			}
			lexer->match (LexType::ID);
			CScriptVar* obj = new CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT);
			CScriptVarLink* objLink = new CScriptVarLink (obj);
			if (objClassOrFunc->getVar ()->isFunction ())
			{
				CLEAN (functionCall (execute, objClassOrFunc, obj));
			}
			else
			{
				obj->addChild (TINYJS_PROTOTYPE_CLASS, objClassOrFunc->getVar ());
				if (lexer->getToken () == '(')
				{
					lexer->match ('(');
					lexer->match (')');
				}
			}
			return objLink;
		}
		else
		{
			lexer->match (LexType::ID);
			if (lexer->getToken () == '(')
			{
				lexer->match ('(');
				lexer->match (')');
			}
		}
	}
	// Nothing we can do here... just hope it's the end...
	lexer->match (LexType::Eof);
	return 0;
}

CScriptVarLink* CTinyJS::unary (bool& execute)
{
	CScriptVarLink* a;
	if (lexer->getToken () == '!')
	{
		lexer->match ('!'); // binary not
		a = factor (execute);
		if (execute)
		{
			CScriptVar zero (static_cast<int64_t> (0));
			CScriptVar* res = a->getVar ()->mathsOp (&zero, asInteger (LexType::EQUAL));
			CREATE_LINK (a, res);
		}
	}
	else
		a = factor (execute);
	return a;
}

CScriptVarLink* CTinyJS::term (bool& execute)
{
	CScriptVarLink* a = unary (execute);
	while (lexer->getToken () == '*' || lexer->getToken () == '/' || lexer->getToken () == '%')
	{
		int op = lexer->getToken ();
		lexer->match (lexer->getToken ());
		CScriptVarLink* b = unary (execute);
		if (execute)
		{
			CScriptVar* res = a->getVar ()->mathsOp (b->getVar (), op);
			CREATE_LINK (a, res);
		}
		CLEAN (b);
	}
	return a;
}

CScriptVarLink* CTinyJS::expression (bool& execute)
{
	bool negate = false;
	if (lexer->getToken () == '-')
	{
		lexer->match ('-');
		negate = true;
	}
	CScriptVarLink* a = term (execute);
	if (negate)
	{
		CScriptVar zero (static_cast<int64_t> (0));
		CScriptVar* res = zero.mathsOp (a->getVar (), '-');
		CREATE_LINK (a, res);
	}

	while (lexer->getToken () == '+' || lexer->getToken () == '-' ||
		   lexer->getToken () == asInteger (LexType::PLUSPLUS) ||
		   lexer->getToken () == asInteger (LexType::MINUSMINUS))
	{
		int op = lexer->getToken ();
		lexer->match (lexer->getToken ());
		if (op == asInteger (LexType::PLUSPLUS) || op == asInteger (LexType::MINUSMINUS))
		{
			if (execute)
			{
				CScriptVar one (static_cast<int64_t> (1));
				CScriptVar* res =
					a->getVar ()->mathsOp (&one, op == asInteger (LexType::PLUSPLUS) ? '+' : '-');
				CScriptVarLink* oldValue = new CScriptVarLink (a->getVar ());
				// in-place add/subtract
				a->replaceWith (res);
				CLEAN (a);
				a = oldValue;
			}
		}
		else
		{
			CScriptVarLink* b = term (execute);
			if (execute)
			{
				// not in-place, so just replace
				CScriptVar* res = a->getVar ()->mathsOp (b->getVar (), op);
				CREATE_LINK (a, res);
			}
			CLEAN (b);
		}
	}
	return a;
}

CScriptVarLink* CTinyJS::shift (bool& execute)
{
	CScriptVarLink* a = expression (execute);
	if (lexer->getToken () == asInteger (LexType::LSHIFT) ||
		lexer->getToken () == asInteger (LexType::RSHIFT) ||
		lexer->getToken () == asInteger (LexType::RSHIFTUNSIGNED))
	{
		int op = lexer->getToken ();
		lexer->match (op);
		CScriptVarLink* b = base (execute);
		auto shift = execute ? b->getVar ()->getInt () : 0;
		CLEAN (b);
		if (execute)
		{
			if (op == asInteger (LexType::LSHIFT))
				a->getVar ()->setInt (a->getVar ()->getInt () << shift);
			if (op == asInteger (LexType::RSHIFT))
				a->getVar ()->setInt (a->getVar ()->getInt () >> shift);
			if (op == asInteger (LexType::RSHIFTUNSIGNED))
				a->getVar ()->setInt (((unsigned int)a->getVar ()->getInt ()) >> shift);
		}
	}
	return a;
}

CScriptVarLink* CTinyJS::condition (bool& execute)
{
	CScriptVarLink* a = shift (execute);
	CScriptVarLink* b;
	while (lexer->getToken () == asInteger (LexType::EQUAL) ||
		   lexer->getToken () == asInteger (LexType::NEQUAL) ||
		   lexer->getToken () == asInteger (LexType::TYPEEQUAL) ||
		   lexer->getToken () == asInteger (LexType::NTYPEEQUAL) ||
		   lexer->getToken () == asInteger (LexType::LEQUAL) ||
		   lexer->getToken () == asInteger (LexType::GEQUAL) || lexer->getToken () == '<' ||
		   lexer->getToken () == '>')
	{
		int op = lexer->getToken ();
		lexer->match (lexer->getToken ());
		b = shift (execute);
		if (execute)
		{
			CScriptVar* res = a->getVar ()->mathsOp (b->getVar (), op);
			CREATE_LINK (a, res);
		}
		CLEAN (b);
	}
	return a;
}

CScriptVarLink* CTinyJS::logic (bool& execute)
{
	CScriptVarLink* a = condition (execute);
	CScriptVarLink* b;
	while (lexer->getToken () == '&' || lexer->getToken () == '|' || lexer->getToken () == '^' ||
		   lexer->getToken () == asInteger (LexType::ANDAND) ||
		   lexer->getToken () == asInteger (LexType::OROR))
	{
		bool noexecute = false;
		int op = lexer->getToken ();
		lexer->match (lexer->getToken ());
		bool shortCircuit = false;
		bool boolean = false;
		// if we have short-circuit ops, then if we know the outcome
		// we don't bother to execute the other op. Even if not
		// we need to tell mathsOp it's an & or |
		if (op == asInteger (LexType::ANDAND))
		{
			op = '&';
			shortCircuit = !a->getVar ()->getBool ();
			boolean = true;
		}
		else if (op == asInteger (LexType::OROR))
		{
			op = '|';
			shortCircuit = a->getVar ()->getBool ();
			boolean = true;
		}
		b = condition (shortCircuit ? noexecute : execute);
		if (execute && !shortCircuit)
		{
			if (boolean)
			{
				CScriptVar* newa = new CScriptVar (a->getVar ()->getBool ());
				CScriptVar* newb = new CScriptVar (b->getVar ()->getBool ());
				CREATE_LINK (a, newa);
				CREATE_LINK (b, newb);
			}
			CScriptVar* res = a->getVar ()->mathsOp (b->getVar (), op);
			CREATE_LINK (a, res);
		}
		CLEAN (b);
	}
	return a;
}

CScriptVarLink* CTinyJS::ternary (bool& execute)
{
	CScriptVarLink* lhs = logic (execute);
	bool noexec = false;
	if (lexer->getToken () == '?')
	{
		lexer->match ('?');
		if (!execute)
		{
			CLEAN (lhs);
			CLEAN (base (noexec));
			lexer->match (':');
			CLEAN (base (noexec));
		}
		else
		{
			bool first = lhs->getVar ()->getBool ();
			CLEAN (lhs);
			if (first)
			{
				lhs = base (execute);
				lexer->match (':');
				CLEAN (base (noexec));
			}
			else
			{
				CLEAN (base (noexec));
				lexer->match (':');
				lhs = base (execute);
			}
		}
	}

	return lhs;
}

CScriptVarLink* CTinyJS::base (bool& execute)
{
	CScriptVarLink* lhs = ternary (execute);
	if (lexer->getToken () == '=' || lexer->getToken () == asInteger (LexType::PLUSEQUAL) ||
		lexer->getToken () == asInteger (LexType::MINUSEQUAL))
	{
		/* If we're assigning to this and we don't have a parent,
		 * add it to the symbol table root as per JavaScript. */
		if (execute && !lhs->owned ())
		{
			if (lhs->getName ().length () > 0)
			{
				CScriptVarLink* realLhs = root->addChildNoDup (lhs->getName (), lhs->getVar ());
				CLEAN (lhs);
				lhs = realLhs;
			}
			else
				TRACE ("Trying to assign to an un-named type\n");
		}

		int op = lexer->getToken ();
		lexer->match (lexer->getToken ());
		CScriptVarLink* rhs = base (execute);
		if (execute)
		{
			if (op == '=')
			{
				lhs->replaceWith (rhs);
			}
			else if (op == asInteger (LexType::PLUSEQUAL))
			{
				CScriptVar* res = lhs->getVar ()->mathsOp (rhs->getVar (), '+');
				lhs->replaceWith (res);
			}
			else if (op == asInteger (LexType::MINUSEQUAL))
			{
				CScriptVar* res = lhs->getVar ()->mathsOp (rhs->getVar (), '-');
				lhs->replaceWith (res);
			}
			else
				ASSERT (0);
		}
		CLEAN (rhs);
	}
	return lhs;
}

void CTinyJS::block (bool& execute)
{
	lexer->match ('{');
	if (execute)
	{
		while (lexer->getToken () && lexer->getToken () != '}')
			statement (execute);
		lexer->match ('}');
	}
	else
	{
		// fast skip of blocks
		int brackets = 1;
		while (lexer->getToken () && brackets)
		{
			if (lexer->getToken () == '{')
				brackets++;
			if (lexer->getToken () == '}')
				brackets--;
			lexer->match (lexer->getToken ());
		}
	}
}

void CTinyJS::statement (bool& execute)
{
	if (lexer->getToken () == asInteger (LexType::ID) ||
		lexer->getToken () == asInteger (LexType::INT) ||
		lexer->getToken () == asInteger (LexType::FLOAT) ||
		lexer->getToken () == asInteger (LexType::STR) || lexer->getToken () == '-')
	{
		/* Execute a simple statement that only contains basic arithmetic... */
		CLEAN (base (execute));
		lexer->match (';');
	}
	else if (lexer->getToken () == '{')
	{
		/* A block of code */
		block (execute);
	}
	else if (lexer->getToken () == ';')
	{
		/* Empty statement - to allow things like ;;; */
		lexer->match (';');
	}
	else if (lexer->getToken () == asInteger (LexType::R_VAR))
	{
		/* variable creation. TODO - we need a better way of parsing the left
		 * hand side. Maybe just have a flag called can_create_var that we
		 * set and then we parse as if we're doing a normal equals.*/
		lexer->match (LexType::R_VAR);
		while (lexer->getToken () != ';')
		{
			CScriptVarLink* a = nullptr;
			if (execute)
				a = scopes.back ()->findChildOrCreate (lexer->getTokenString ());
			lexer->match (LexType::ID);
			// now do stuff defined with dots
			while (lexer->getToken () == '.')
			{
				lexer->match ('.');
				if (execute)
				{
					CScriptVarLink* lastA = a;
					a = lastA->getVar ()->findChildOrCreate (lexer->getTokenString ());
				}
				lexer->match (LexType::ID);
			}
			// sort out initialiser
			if (lexer->getToken () == '=')
			{
				lexer->match ('=');
				CScriptVarLink* var = base (execute);
				if (execute)
					a->replaceWith (var);
				CLEAN (var);
			}
			if (lexer->getToken () != ';')
				lexer->match (',');
		}
		lexer->match (';');
	}
	else if (lexer->getToken () == asInteger (LexType::R_IF))
	{
		lexer->match (LexType::R_IF);
		lexer->match ('(');
		CScriptVarLink* var = base (execute);
		lexer->match (')');
		bool cond = execute && var->getVar ()->getBool ();
		CLEAN (var);
		bool noexecute = false; // because we need to be abl;e to write to it
		statement (cond ? execute : noexecute);
		if (lexer->getToken () == asInteger (LexType::R_ELSE))
		{
			lexer->match (LexType::R_ELSE);
			statement (cond ? noexecute : execute);
		}
	}
	else if (lexer->getToken () == asInteger (LexType::R_WHILE))
	{
		// We do repetition by pulling out the string representing our statement
		// there's definitely some opportunity for optimisation here
		lexer->match (LexType::R_WHILE);
		lexer->match ('(');
		size_t whileCondStart = lexer->getTokenStart ();
		bool noexecute = false;
		CScriptVarLink* cond = base (execute);
		bool loopCond = execute && cond->getVar ()->getBool ();
		CLEAN (cond);
		CScriptLex* whileCond = lexer->getSubLex (whileCondStart);
		lexer->match (')');
		size_t whileBodyStart = lexer->getTokenStart ();
		statement (loopCond ? execute : noexecute);
		CScriptLex* whileBody = lexer->getSubLex (whileBodyStart);
		CScriptLex* oldLex = lexer;
		int loopCount = TINYJS_LOOP_MAX_ITERATIONS;
		while (loopCond && loopCount-- > 0)
		{
			whileCond->reset ();
			lexer = whileCond;
			cond = base (execute);
			loopCond = execute && cond->getVar ()->getBool ();
			CLEAN (cond);
			if (loopCond)
			{
				whileBody->reset ();
				lexer = whileBody;
				statement (execute);
			}
		}
		lexer = oldLex;
		delete whileCond;
		delete whileBody;

		if (loopCount <= 0)
		{
			root->trace ();
			TRACE ("WHILE Loop exceeded %d iterations at %s\n", TINYJS_LOOP_MAX_ITERATIONS,
				   lexer->getPosition ().c_str ());
			throw CScriptException ("LOOP_ERROR");
		}
	}
	else if (lexer->getToken () == asInteger (LexType::R_FOR))
	{
		lexer->match (LexType::R_FOR);
		lexer->match ('(');
		statement (execute); // initialisation
		// l->match(';');
		size_t forCondStart = lexer->getTokenStart ();
		bool noexecute = false;
		CScriptVarLink* cond = base (execute); // condition
		bool loopCond = execute && cond->getVar ()->getBool ();
		CLEAN (cond);
		CScriptLex* forCond = lexer->getSubLex (forCondStart);
		lexer->match (';');
		size_t forIterStart = lexer->getTokenStart ();
		CLEAN (base (noexecute)); // iterator
		CScriptLex* forIter = lexer->getSubLex (forIterStart);
		lexer->match (')');
		size_t forBodyStart = lexer->getTokenStart ();
		statement (loopCond ? execute : noexecute);
		CScriptLex* forBody = lexer->getSubLex (forBodyStart);
		CScriptLex* oldLex = lexer;
		if (loopCond)
		{
			forIter->reset ();
			lexer = forIter;
			CLEAN (base (execute));
		}
		int loopCount = TINYJS_LOOP_MAX_ITERATIONS;
		while (execute && loopCond && loopCount-- > 0)
		{
			forCond->reset ();
			lexer = forCond;
			cond = base (execute);
			loopCond = cond->getVar ()->getBool ();
			CLEAN (cond);
			if (execute && loopCond)
			{
				forBody->reset ();
				lexer = forBody;
				statement (execute);
			}
			if (execute && loopCond)
			{
				forIter->reset ();
				lexer = forIter;
				CLEAN (base (execute));
			}
		}
		lexer = oldLex;
		delete forCond;
		delete forIter;
		delete forBody;
		if (loopCount <= 0)
		{
			root->trace ();
			TRACE ("FOR Loop exceeded %d iterations at %s\n", TINYJS_LOOP_MAX_ITERATIONS,
				   lexer->getPosition ().c_str ());
			throw CScriptException ("LOOP_ERROR");
		}
	}
	else if (lexer->getToken () == asInteger (LexType::R_RETURN))
	{
		lexer->match (LexType::R_RETURN);
		CScriptVarLink* result = nullptr;
		if (lexer->getToken () != ';')
			result = base (execute);
		if (execute)
		{
			CScriptVarLink* resultVar = scopes.back ()->findChild (TINYJS_RETURN_VAR);
			if (resultVar)
				resultVar->replaceWith (result);
			else
				TRACE ("RETURN statement, but not in a function.\n");
			execute = false;
		}
		CLEAN (result);
		lexer->match (';');
	}
	else if (lexer->getToken () == asInteger (LexType::R_FUNCTION))
	{
		CScriptVarLink* funcVar = parseFunctionDefinition ();
		if (execute)
		{
			if (funcVar->getName () == TINYJS_TEMP_NAME)
				TRACE ("Functions defined at statement-level are meant to have a name\n");
			else
				scopes.back ()->addChildNoDup (funcVar->getName (), funcVar->getVar ());
		}
		CLEAN (funcVar);
	}
	else
		lexer->match (LexType::Eof);
}

/// Get the given variable specified by a path (var1.var2.etc), or return 0
CScriptVar* CTinyJS::getScriptVariable (const string& path) const
{
	// traverse path
	size_t prevIdx = 0;
	size_t thisIdx = path.find ('.');
	if (thisIdx == string::npos)
		thisIdx = path.length ();
	CScriptVar* var = root;
	while (var && prevIdx < path.length ())
	{
		string el = path.substr (prevIdx, thisIdx - prevIdx);
		CScriptVarLink* varl = var->findChild (el);
		var = varl ? varl->getVar () : 0;
		prevIdx = thisIdx + 1;
		thisIdx = path.find ('.', prevIdx);
		if (thisIdx == string::npos)
			thisIdx = path.length ();
	}
	return var;
}

/// Get the value of the given variable, or return 0
const string* CTinyJS::getVariable (const string& path) const
{
	CScriptVar* var = getScriptVariable (path);
	// return result
	if (var)
		return &var->getString ();
	else
		return 0;
}

/// set the value of the given variable, return trur if it exists and gets set
bool CTinyJS::setVariable (const string& path, const string& varData)
{
	CScriptVar* var = getScriptVariable (path);
	// return result
	if (var)
	{
		if (var->isInt ())
			var->setInt (stringToInteger (varData));
		else if (var->isDouble ())
			var->setDouble (stringToDouble (varData));
		else
			var->setString (varData);
		return true;
	}
	else
		return false;
}

/// Finds a child, looking recursively up the scopes
CScriptVarLink* CTinyJS::findInScopes (const string& childName) const
{
	for (int64_t s = scopes.size () - 1; s >= 0; s--)
	{
		CScriptVarLink* v = scopes[s]->findChild (childName);
		if (v)
			return v;
	}
	return NULL;
}

/// Look up in any parent classes of the given object
CScriptVarLink* CTinyJS::findInParentClasses (CScriptVar* object, const string& name) const
{
	// Look for links to actual parent classes
	CScriptVarLink* parentClass = object->findChild (TINYJS_PROTOTYPE_CLASS);
	while (parentClass)
	{
		CScriptVarLink* implementation = parentClass->getVar ()->findChild (name);
		if (implementation)
			return implementation;
		parentClass = parentClass->getVar ()->findChild (TINYJS_PROTOTYPE_CLASS);
	}
	// else fake it for strings and finally objects
	if (object->isString ())
	{
		CScriptVarLink* implementation = stringClass->findChild (name);
		if (implementation)
			return implementation;
	}
	if (object->isArray ())
	{
		CScriptVarLink* implementation = arrayClass->findChild (name);
		if (implementation)
			return implementation;
	}
	CScriptVarLink* implementation = objectClass->findChild (name);
	if (implementation)
		return implementation;

	return 0;
}

//------------------------------------------------------------------------
} // TJS
