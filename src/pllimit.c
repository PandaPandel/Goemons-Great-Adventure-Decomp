#include "common.h"
#include "pllimit.h"

/* Speed limits (overlay 9). The file's .rodata holds one dead print, "falldead: %08x\n"; its
 * place in the code is not known. */

#pragma intrinsic (sqrtf)

/* Limits a speed across (x and z) to max. */
void plLimitSpeedXZ(f32 *v, f32 max) {
    f32 x = v[0];
    f32 z = v[2];
    f32 d = sqrtf(x * x + z * z);
    f32 k;

    PRINTF("falldead: %08x\n", v);
    if (max < d) {
        k = max / d;
        v[0] = x * k;
        v[2] = z * k;
    }
}

/* Limits a speed up or down (y) to max. */
void plLimitSpeedY(f32 *v, f32 max) {
    f32 y = v[1];
    f32 a = fabsf(y);
    f32 k;

    if (max < a) {
        k = max / a;
        v[1] = y * k;
    }
}
