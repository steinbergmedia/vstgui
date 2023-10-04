// built-in functions

foo = "foo bar stuff";
r = Math.rand();

parsed = Integer.parseInt("42");
parsedHex = Integer.parseInt("0xFF");
parsedOct = Integer.parseInt("011");
parsedBig = Integer.parseInt("4294967296");

aStr = "ABCD";
aChar = aStr.charAt(0);

obj1 = new Object ();
obj1.food = "cake";
obj1.desert = "pie";

obj2 = obj1.clone();
obj2.food = "kittens";

result = foo.length == 13 && foo.indexOf("bar") == 4 && foo.substring(8, 13) == "stuff" &&
		 parsed == 42 && Integer.valueOf(aChar) == 65 && obj1.food == "cake" &&
		 obj2.desert == "pie" && parsedHex == 255 && parsedOct == 9 && parsedBig == 4294967296;
