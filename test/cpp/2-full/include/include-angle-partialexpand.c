// All compilers fail this with trying to include <foo/a/foo-path.h>
#define include foo
#define FOO <include/a/include-path.h>
#include FOO
