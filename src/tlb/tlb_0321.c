#include "tlb.h"

EneDef stg02bPlaces[] = {
    { 0, 0, 0, 0, ENE_GASYA_BODY, 0, 0 },
    { 0, 0, 0, 0, ENE_GASYA_HAND, 0, 0 },
    { 0, 0, 0, 0, ENE_GASYA_WALL, 0, 0 },
    { 0, 0, 0, 0, ENE_GASYA_FLAME, 0, 0 },
    { 0, 0, 0, 0, ENE_GASYA_BOMB, 0, 0 },
    { 0, 0, 0, 0, ENE_GASYA_NANIWA, 0, 0 },
    { 2, 1750, 180, 0, ENE_GASYA_MAIN, 0, 0 },
    { 2, 1970, 149, 0, ENE_GASYA_HASI, 0, 0 },
};

void stg02bBuild(void) {
    stageBuildBasic(2, 2, 1400.0f, 210.0f, 0.0f, (u32) stg02bPlaces, 8);
    PRINTF("敵テストステージセット成功\n");
}
