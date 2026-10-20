#include "tlb.h"

EneDef stg0191Places[] = {
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 1, 220, 50, 0, ENE_GYUKI, 2, 0 },
};

void stg0191Build(void) {
    stageBuildBasic(1, 1, 50.0f, 20.0f, 0.0f, (u32) stg0191Places, 4);
}
