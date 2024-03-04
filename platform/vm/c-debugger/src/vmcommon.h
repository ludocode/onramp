#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

/**
 * This Onramp VM uses Ghost for portability and utilities. See:
 *
 *     https://github.com/ludocode/ghost
 *
 * Regenerate it in with:
 *
 *     [path/to/ghost]/tools/amalgamate.py -i vm.c -o vm_ghost.h -p vm
 *
 * (or just `make ghost` if you've cloned it in the right place)
 */
#if 0
#include "ghost/debug/ghost_debugbreak.h"
#include "ghost/debug/ghost_static_assert.h"
#include "ghost/header/c/ghost_stdint_h.h"
#include "ghost/header/c/ghost_stdio_h.h"
#include "ghost/header/c/ghost_string_h.h"
#include "ghost/header/c/ghost_stdlib_h.h"
#include "ghost/language/ghost_always_inline.h"
#include "ghost/language/ghost_array_count.h"
#include "ghost/language/ghost_noinline.h"
#include "ghost/language/ghost_noreturn.h"
#include "ghost/language/ghost_unreachable.h"
#include "ghost/language/ghost_null.h"
#include "ghost/language/ghost_const_cast.h"
#include "ghost/malloc/ghost_alloc_array.h"
#include "ghost/malloc/ghost_alloc_zero.h"
#include "ghost/malloc/ghost_strdup.h"
#include "ghost/math/min/ghost_min_u32.h"
#include "ghost/math/rotr/ghost_rotr_u32.h"
#include "ghost/serialization/load/ghost_load_le_u32.h"
#include "ghost/serialization/store/ghost_store_le_u32.h"
#include "ghost/string/ghost_strlcat.h"
#include "ghost/string/ghost_strlcpy.h"
#endif
#include "vm_ghost.h"

#include <time.h>
#include <ctype.h>
#include <sys/stat.h>

#include "libo-util.h"
#include "libo-error.h"

void common_init(void);
void common_destroy(void);

#endif
