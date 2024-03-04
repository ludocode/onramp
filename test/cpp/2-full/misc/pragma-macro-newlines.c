// Tests that the preprocessor will correctly insert newlines when converting
// _Pragma to #pragma
#define FOO A _Pragma("mark") B
FOO
