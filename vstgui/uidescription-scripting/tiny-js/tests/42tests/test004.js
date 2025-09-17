// generator-test

function fibonacci ()
{
	var fn1 = 1;
	var fn2 = 1;
	while (1)
	{
		var current = fn2;
		fn2 = fn1;
		fn1 = fn1 + current;
		var reset = yield current;
		if (reset)
		{
			fn1 = 1;
			fn2 = 1;
		}
	}
}

var generator = fibonacci ();

generator.next(); // 1
generator.next(); // 1
generator.next(); // 2
generator.next(); // 3
generator.next(); // 5

result = generator.next() == 8 && generator.send(true) == 1;
