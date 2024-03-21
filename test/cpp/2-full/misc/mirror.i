# 1 "./misc/mirror.c"
# 609 "./misc/mirror.c"
static void mirror_TEST_fake_key0(void); static void mirror_THUNK_fake_key0(void* vfixture, void* vparam) { mirror_TEST_fake_key0(); } MIRROR_REGISTRATION_BLOCK( fake_key0) { static mirror_test_t test; test.id = "fake_key0"; test.fn = mirror_THUNK_fake_key0; test.file = "./misc/mirror.c"; test.line = 609; test.name = "<anonymous>"; mirror_register_test(&test); } static void mirror_TEST_fake_key0(void) {
