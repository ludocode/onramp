// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// TODO replaced by 

#define GHOST_CONCAT(x, y) GHOST_CONCAT_IMPL(x, y)
#define GHOST_CONCAT_IMPL(x, y) x##y

#define MIRROR_KEY fake_key

#define mirror(...) GHOST_CONCAT(mirror_, 1)(__VA_ARGS__)

#define mirror_1(a) \
    MIRROR_IMPL(a)

#define MIRROR_IMPL(a) \
    MIRROR_IMPL5(\
            GHOST_CONCAT(MIRROR_KEY, __COUNTER__))

#define MIRROR_IMPL5(id) static void GHOST_CONCAT(mirror_TEST_, id)(void);

mirror()
