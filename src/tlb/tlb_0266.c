#include "tlb.h"

extern u32 mechChamisePlacesEne19FParamPath;
extern PersonData mechChamisePlacesEne19FParam[];
u32 mechChamisePlacesEne19FParamPath = 0x00010000;
PersonData mechChamisePlacesEne19FParam[] = {
    { 39, (u32) &mechChamisePlacesEne19FParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef mechChamisePlaces[] = {
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -20, 14, 48, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -39, 7, 0, ENE_TRANSFER, 0, 0 },
    { 0, -17, 0, 30, ENE_SHOP_CHA_BABA, 2, (u32) &mechChamisePlacesEne19FParam },
    { 0, 0, 1, 0, ENE_CHAMISE, 0, 0 },
};

void mechChamiseInit(void) {
    stageBuildFixedCam(0x6F, 0, 0.0f, 0.0f, 0.0f, (u32) mechChamisePlaces, 5, 115.0f, 20.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(20.0f, 0.0f, 0.628318548f);
}
