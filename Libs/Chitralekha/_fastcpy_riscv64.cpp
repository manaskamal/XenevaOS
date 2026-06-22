#include "_fastcpy.h"
#include <string.h>

extern "C" void* _fastcpy(void* targ, void* src, size_t len) {
    return memcpy(targ, src, len);
}
