/* https://github.com/protopopov1122/kefir/issues/1 */
#define foo []
#define stringify(x) #x
const char* f = stringify(foo);
