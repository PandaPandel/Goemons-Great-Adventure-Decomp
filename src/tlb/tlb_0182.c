#include "tlb.h"

extern PersonData stg0182PlacesEne197Param[];
u32 stg0182Ene176Param[] = { 0x00000252, 0x00000000, 0x04080000, 0x00000000, 0x00000000, 0x00000252, 0x00000000, 0x04080000, 0x00000000, 0x00000000 };
PersonData stg0182PlacesEne197Param[] = {
    { 0, 0x00000000, { 15, 70 }, 0, 0.0f, 0 },
};
EneDef stg0182Places[] = {
    { 1, 1000, 90, 0, ENE_SUISHA, 2, 0 },
    { 2, 840, 150, 0, ENE_SUISHA, 2, 0 },
    { 2, 2752, 105, 0, ENE_SUISHA, 2, 1 },
    { 4, 746, -32, 0, ENE_SUISHA, 2, 1 },
    { 0, 600, 83, 1367, ENE_WATER_STREAM_FLAGS_2, 2, -2031358 },
    { 0, 324, 71, 1560, ENE_WATER_STREAM_FLAGS_1, 2, 15270404 },
    { 0, 427, 165, 1419, ENE_WATER_STREAM_FLAGS_1, 2, 17498628 },
    { 0, 1168, 101, -685, ENE_WATER_STREAM_FLAGS_1, 2, 27197956 },
    { 0, 155, 223, 1400, ENE_WATER_STREAM_FLAGS_1, 2, -2949115 },
    { 0, 302, 193, 1368, ENE_WATER_STREAM_FLAGS_1, 2, 1638405 },
    { 0, 218, 95, 1488, ENE_WATER_STREAM_FLAGS_1, 2, -1638395 },
    { 0, 826, 60, -237, ENE_WATER_STREAM_FLAGS_1, 2, 10027013 },
    { 0, 1039, 142, -562, ENE_WATER_STREAM_FLAGS_1, 2, 9830405 },
    { 0, 1058, 127, -623, ENE_WATER_STREAM_FLAGS_1, 2, 10027013 },
    { 0, 116, 199, 1484, ENE_WATER_STREAM_FLAGS_1, 2, -19857147 },
    { 0, 890, 36, -162, ENE_WATER_STREAM_FLAGS_1, 2, -6487547 },
    { 0, 849, 12, -31, ENE_WATER_STREAM_FLAGS_1, 2, -6094843 },
    { 0, 1099, 120, -489, ENE_WATER_STREAM_FLAGS_1, 2, -6881275 },
    { 0, 1049, 95, -359, ENE_WATER_STREAM_FLAGS_1, 2, -7208699 },
    { 0, -339, 181, 1482, ENE_WATER_STREAM_FLAGS_1, 2, 6095367 },
    { 0, 639, 219, 312, ENE_WATER_STREAM_FLAGS_1, 2, 3866887 },
    { 0, -255, 154, 1540, ENE_WATER_FALLS, 2, 164 },
    { 0, 43, 192, 1484, ENE_WATER_FALLS_RIDE, 2, -256 },
    { 0, 156, 216, 1474, ENE_WATER_FALLS_GROUND, 2, 2 },
    { 0, 335, 186, 1435, ENE_WATER_FALLS_GROUND, 2, 71 },
    { 0, 229, 88, 1561, ENE_WATER_FALLS_GROUND, 2, 22 },
    { 0, 527, 152, 1352, ENE_WATER_FALLS, 2, 361 },
    { 0, 435, 59, 1514, ENE_WATER_FALLS, 2, 334 },
    { 0, 449, 67, 1503, ENE_WATER_FALLS, 2, -174 },
    { 0, 710, 192, 387, ENE_WATER_FALLS, 2, 130 },
    { 0, 894, 53, -213, ENE_WATER_FALLS_GROUND, 2, 201 },
    { 0, 866, 29, -92, ENE_WATER_FALLS_GROUND, 2, -56 },
    { 0, 828, 6, 40, ENE_WATER_FALLS, 2, -48 },
    { 0, 1108, 135, -537, ENE_WATER_FALLS_GROUND, 2, 199 },
    { 0, 1075, 113, -420, ENE_WATER_FALLS_GROUND, 2, -59 },
    { 0, 1022, 88, -290, ENE_WATER_FALLS, 2, -65 },
    { 0, 1128, 120, -598, ENE_WATER_FALLS_GROUND, 2, 199 },
    { 0, 1180, 89, -805, ENE_WATER_FALLS, 2, 511 },
    { 1, 700, 150, 0, ENE_HIDUTU, 0, 0 },
    { 0, -11, 10, -22, ENE_KENSUKE, 2, (u32) &stg0182PlacesEne197Param },
    { 1, 588, 115, 0, ENE_IMP_OBJ_BALL, 2, 0 },
};

void stg0182Build(void) {
    s32 pad;

    if (!(joyGetPadHist(0, 0)->button & Z_TRIG)) {
        stageBuildBasic(2, 1, 270.0f, 100.0f, 0.0f, (u32) stg0182Places, 0x29);
    } else {
        stageBuildZoneCam(2, 0, -1195.0f, 95.0f, 1555.0f, (u32) stg0182Places, 0x29);
    }
    camAiSetFar(600);
    eneMakeEx(NULL, ENE_DEMO_TSUCHI, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
    eneMakeEx(NULL, ENE_DEMO_JIZOU_FILE_130, 0, 0.0f, 0.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) stg0182Ene176Param);
    cambgCreate(gStageTask, 1, 800.0f);
}
