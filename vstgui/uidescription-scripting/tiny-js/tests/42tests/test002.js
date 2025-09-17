// function-closure

var a = 40; // a global var

function closure ()
{
	var a = 39; // a local var;
	return function () { return a; };
}

var b = closure (); // the local var a is now hidden

result = b () + 3 == 42 && a + 2 == 42;
