#pragma once

#include <hx/libhatchet.h>

#if HX_CPLUSPLUS
extern "C" {
#endif

bool hxctest_all(void);
bool hxctest_libhatchet_h(void);
bool hxctest_math(void);
bool hxctest_clamp(void);
bool hxctest_swap(void);
bool hxctest_memory(void);

#if HX_CPLUSPLUS
}
#endif
