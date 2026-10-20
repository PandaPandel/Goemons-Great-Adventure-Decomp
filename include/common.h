#ifndef COMMON_H
#define COMMON_H

/* libultra 2.0I (lib/ultralib), with the F3DEX2 GBI. The build defines BUILD_VERSION=VERSION_I and
 * F3DEX_GBI_2. */
#include <ultra64.h>

/* Pi as a float (gu.h's M_PI is a double): the game's constants are 3.141592741 (0x40490FDB)
 * and its half. */
#define PI 3.14159265358979323846f

/* The game's debug prints are compiled out, but their strings stay in .rodata (e.g. "Warning:
 * in aicamDeleteLookat !\n", which no code loads). */
#define PRINTF if (0) osSyncPrintf

/* fabsf is IDO's intrinsic: an inline abs.s, as the game has it. */
float fabsf(float);
#pragma intrinsic (fabsf)
/* fabs, the double one: the files that inline it say so with their own #pragma intrinsic (fabs). */
double fabs(double);

typedef struct Vec3f {
    f32 x, y, z;
} Vec3f;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

/* A KSEG0 address as the RCP sees it (physical). */
#define VIRTUAL_TO_PHYSICAL(addr) ((u32) (addr) - 0x80000000)

#endif
