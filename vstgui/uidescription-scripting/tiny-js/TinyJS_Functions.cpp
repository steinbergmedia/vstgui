/*
 * TinyJS
 *
 * A single-file Javascript-alike engine
 *
 * - Useful language functions
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

#include "TinyJS_Functions.h"
#include <cmath>
#include <cstdlib>
#include <sstream>

//------------------------------------------------------------------------
namespace TJS {

using namespace std;
using namespace std::literals;
// ----------------------------------------------- Actual Functions
void scTrace (CScriptVar* c, void* userdata)
{
	CTinyJS* js = (CTinyJS*)userdata;
	js->root->trace ();
}

void scObjectDump (CScriptVar* c) { c->getParameter ("this"sv)->trace ("> "); }

void scObjectClone (CScriptVar* c)
{
	CScriptVar* obj = c->getParameter ("this"sv);
	c->getReturnVar ()->copyValue (obj);
}

void scMathRand (CScriptVar* c) { c->getReturnVar ()->setDouble ((double)rand () / RAND_MAX); }

void scMathRandInt (CScriptVar* c)
{
	auto min = c->getParameter ("min"sv)->getInt ();
	auto max = c->getParameter ("max"sv)->getInt ();
	auto val = min + (int64_t)(rand () % (1 + max - min));
	c->getReturnVar ()->setInt (val);
}

void scCharToInt (CScriptVar* c)
{
	string str = c->getParameter ("ch"sv)->getString ();
	;
	int val = 0;
	if (str.length () > 0)
		val = (int)str.c_str ()[0];
	c->getReturnVar ()->setInt (val);
}

void scStringIndexOf (CScriptVar* c)
{
	string str = c->getParameter ("this"sv)->getString ();
	string search = c->getParameter ("search"sv)->getString ();
	size_t p = str.find (search);
	auto val = (p == string::npos) ? -1 : p;
	c->getReturnVar ()->setInt (val);
}

void scStringSubstring (CScriptVar* c)
{
	string str = c->getParameter ("this"sv)->getString ();
	auto lo = c->getParameter ("lo"sv)->getInt ();
	auto hi = c->getParameter ("hi"sv)->getInt ();

	auto l = hi - lo;
	if (l > 0 && lo >= 0 && lo + l <= static_cast<int64_t> (str.length ()))
		c->getReturnVar ()->setString (str.substr (lo, l));
	else
		c->getReturnVar ()->setString ("");
}

void scStringCharAt (CScriptVar* c)
{
	string str = c->getParameter ("this"sv)->getString ();
	auto p = c->getParameter ("pos"sv)->getInt ();
	if (p >= 0 && p < static_cast<int64_t> (str.length ()))
		c->getReturnVar ()->setString (str.substr (p, 1));
	else
		c->getReturnVar ()->setString ("");
}

void scStringCharCodeAt (CScriptVar* c)
{
	string str = c->getParameter ("this"sv)->getString ();
	auto p = c->getParameter ("pos"sv)->getInt ();
	if (p >= 0 && p < static_cast<int64_t> (str.length ()))
		c->getReturnVar ()->setInt (str.at (p));
	else
		c->getReturnVar ()->setInt (0);
}

void scStringSplit (CScriptVar* c)
{
	string str = c->getParameter ("this"sv)->getString ();
	string sep = c->getParameter ("separator"sv)->getString ();
	CScriptVar* result = c->getReturnVar ();
	result->setArray ();
	int length = 0;

	size_t pos = str.find (sep);
	while (pos != string::npos)
	{
		result->setArrayIndex (length++, new CScriptVar (str.substr (0, pos)));
		str = str.substr (pos + 1);
		pos = str.find (sep);
	}

	if (str.size () > 0)
		result->setArrayIndex (length++, new CScriptVar (str));
}

void scStringFromCharCode (CScriptVar* c)
{
	char str[2];
	str[0] = static_cast<char> (c->getParameter ("char"sv)->getInt ());
	str[1] = 0;
	c->getReturnVar ()->setString (str);
}

void scIntegerParseInt (CScriptVar* c)
{
	string str = c->getParameter ("str"sv)->getString ();
	auto val = stringToInteger (str);
	c->getReturnVar ()->setInt (val);
}

void scIntegerValueOf (CScriptVar* c)
{
	string str = c->getParameter ("str"sv)->getString ();

	int val = 0;
	if (str.length () == 1)
		val = str[0];
	c->getReturnVar ()->setInt (val);
}

void scJSONStringify (CScriptVar* c)
{
	std::ostringstream result;
	c->getParameter ("obj"sv)->getJSON (result);
	c->getReturnVar ()->setString (result.str ());
}

void scExec (CScriptVar* c, void* data)
{
	CTinyJS* tinyJS = (CTinyJS*)data;
	std::string str = c->getParameter ("jsCode"sv)->getString ();
	tinyJS->execute (str);
}

void scEval (CScriptVar* c, void* data)
{
	CTinyJS* tinyJS = (CTinyJS*)data;
	std::string str = c->getParameter ("jsCode"sv)->getString ();
	c->setReturnVar (tinyJS->evaluateComplex (str).var);
}

void scArrayContains (CScriptVar* c)
{
	CScriptVar* obj = c->getParameter ("obj"sv);
	CScriptVarLink* v = c->getParameter ("this"sv)->firstChild;

	bool contains = false;
	while (v)
	{
		if (v->var->equals (obj))
		{
			contains = true;
			break;
		}
		v = v->nextSibling;
	}

	c->getReturnVar ()->setInt (contains);
}

void scArrayRemove (CScriptVar* c)
{
	CScriptVar* obj = c->getParameter ("obj"sv);
	vector<int> removedIndices;
	CScriptVarLink* v;
	// remove
	v = c->getParameter ("this"sv)->firstChild;
	while (v)
	{
		if (v->var->equals (obj))
		{
			removedIndices.push_back (v->getIntName ());
		}
		v = v->nextSibling;
	}
	// renumber
	v = c->getParameter ("this"sv)->firstChild;
	while (v)
	{
		int n = v->getIntName ();
		int newn = n;
		for (size_t i = 0; i < removedIndices.size (); i++)
			if (n >= removedIndices[i])
				newn--;
		if (newn != n)
			v->setIntName (newn);
		v = v->nextSibling;
	}
}

void scArrayJoin (CScriptVar* c)
{
	string sep = c->getParameter ("separator"sv)->getString ();
	CScriptVar* arr = c->getParameter ("this"sv);

	ostringstream sstr;
	int l = arr->getArrayLength ();
	for (int i = 0; i < l; i++)
	{
		if (i > 0)
			sstr << sep;
		sstr << arr->getArrayIndex (i)->getString ();
	}

	c->getReturnVar ()->setString (sstr.str ());
}

// ----------------------------------------------- Register Functions
void registerFunctions (CTinyJS* tinyJS)
{

	tinyJS->addNative ("function exec(jsCode)"sv, [=] (auto scriptVar) {
		scExec (scriptVar, tinyJS);
	}); // execute the given code
	tinyJS->addNative ("function eval(jsCode)"sv, [=] (auto scriptVar) {
		scEval (scriptVar, tinyJS);
	}); // execute the given string (an expression) and return the result
	tinyJS->addNative ("function trace()"sv, [=] (auto scriptVar) { scTrace (scriptVar, tinyJS); });
	tinyJS->addNative ("function Object.dump()"sv, scObjectDump);
	tinyJS->addNative ("function Object.clone()"sv, scObjectClone);
	tinyJS->addNative ("function Math.rand()"sv, scMathRand);
	tinyJS->addNative ("function Math.randInt(min, max)"sv, scMathRandInt);
	tinyJS->addNative ("function charToInt(ch)"sv,
					   scCharToInt); //  convert a character to an int - get its value
	tinyJS->addNative ("function String.indexOf(search)"sv,
					   scStringIndexOf); // find the position of a string in a string, -1 if not
	tinyJS->addNative ("function String.substring(lo,hi)"sv, scStringSubstring);
	tinyJS->addNative ("function String.charAt(pos)"sv, scStringCharAt);
	tinyJS->addNative ("function String.charCodeAt(pos)"sv, scStringCharCodeAt);
	tinyJS->addNative ("function String.fromCharCode(char)"sv, scStringFromCharCode);
	tinyJS->addNative ("function String.split(separator)"sv, scStringSplit);
	tinyJS->addNative ("function Integer.parseInt(str)"sv, scIntegerParseInt); // string to int
	tinyJS->addNative ("function Integer.valueOf(str)"sv,
					   scIntegerValueOf); // value of a single character
	tinyJS->addNative ("function JSON.stringify(obj, replacer)"sv,
					   scJSONStringify); // convert to JSON. replacer is ignored at the moment
	// JSON.parse is left out as you can (unsafely!) use eval instead
	tinyJS->addNative ("function Array.contains(obj)"sv, scArrayContains);
	tinyJS->addNative ("function Array.remove(obj)"sv, scArrayRemove);
	tinyJS->addNative ("function Array.join(separator)"sv, scArrayJoin);
}

//------------------------------------------------------------------------
} // TJS
