/* https://github.com/protopopov1122/kefir/issues/1 */
#define foo []
#define cat(x, y) x##y
int cat(foo,_);
