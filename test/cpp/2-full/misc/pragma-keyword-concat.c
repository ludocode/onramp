// No compiler that I've tested supports concatenation of string literals in
// _Pragma. They all mangle the below.
_Pragma("onramp" "test" " " "\"a" "b\"")
