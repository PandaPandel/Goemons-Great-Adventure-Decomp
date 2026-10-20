#include "tlb.h"

EneDef stg0193Places[] = {
    { 0, 0, -14, 0, ENE_OBJ22_WATER_FLAGS_3, 2, 3 },
};

void stg0193Build(void) {
    stageBuildBasic(0x1A, 1, 170.0f, 40.0f, 0.0f, (u32) stg0193Places, 1);
    camAiSetFar(800);
}
