#include "tlb.h"

EneDef stg0192Places[] = {
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
};

void stg0192Build(void) {
    stageBuildBasic(6, 1, 100.0f, 30.0f, 0.0f, (u32) stg0192Places, 1);
    camAiSetFar(10000);
}
