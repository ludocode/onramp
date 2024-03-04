// A maximum of 15 indirections are supported.
typedef int******** foo;
typedef foo******** bar; // ERROR: too many indirections
