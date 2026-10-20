#include "tlb.h"

EneDef stg0188Places[] = {
    { 1, 200, 30, 0, ENE_CYCLOPS, 0, 0 },
    { 1, 500, 30, 0, ENE_CYCLOPS, 0, 0 },
    { 0, 0, 0, 0, ENE_SENSYA, 0, 0 },
    { 0, 0, 0, 0, ENE_SENSYA_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_MOUJA, 0, 0 },
    { 0, 0, 0, 0, ENE_MOUJA_ONO, 0, 0 },
    { 0, 0, 0, 0, ENE_MOUJA_KOZ, 0, 0 },
    { 0, 0, 0, 0, ENE_NORU_MARK, 0, 0 },
    { 1, 800, 30, 0, ENE_SENSYA_KOZURE, 0, 0 },
};

void stg0188Build(void) {
    stageBuildBasic(1, 1, 50.0f, 20.0f, 0.0f, (u32) stg0188Places, 9);
}
