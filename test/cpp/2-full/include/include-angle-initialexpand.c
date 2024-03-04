// Clang and MSVC include the correct file.
// GCC, LCC, chibicc and Kefir try to include <foo/a/foo-path.h>
#define include foo
#define FOO /*nothing*/
#include FOO <include/a/include-path.h>
