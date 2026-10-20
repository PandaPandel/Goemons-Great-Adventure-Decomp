#include "tlb.h"

EneDef stg0185Places[] = {
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
};

void stg0185Build(void) {
    stageBuildBasic(2, 1, 255.0f, 100.0f, 0.0f, (u32) stg0185Places, 1);
    PRINTF("敵テストステージセット成功\n");
}
