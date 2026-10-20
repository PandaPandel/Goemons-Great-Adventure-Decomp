#include "tlb.h"

/* File 126: the tsuchikorobi hunt. "tsuchijisan", the old man who asks the player to rid him of
 * the tsuchikorobi (earth-rollers) and gives a pass (tegata) when they are all gone;
 * "demoTsuchi", the hunt's manager; "tsuchikorobi", the rolling enemies themselves; "tsuchiDisp",
 * the count of them on the screen (a cross, "batten", on each one killed); and "tHasi". */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* The old man's work, at its task + 0x20: a demo talker's (PersonObj) up to 0x44. */
typedef struct TsuchiJisanW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 pad09;
    /* 0x0A */ u8 hit;
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PersonData *data;
    /* 0x38 */ ACollFrame *box; /* its talk box */
    /* 0x3C */ u8 flags3C;
    /* 0x3D */ u8 count;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ Obj *talker; /* the player talking to it */
    /* 0x44 */ MPolFrame *shadow;
    /* 0x48 */ Camera *cam;
    /* 0x4C */ Obj *pass; /* the pass (kind 0x1C3), made between it and the player */
    /* 0x50 */ s32 timer; /* ticks in the state */
    /* 0x54 */ u8 flags; /* 1: the sky's tunes change 300 ticks after the pass */
    /* 0x55 */ u8 tuneTimer;
} TsuchiJisanW;

#define TSUCHIJISAN(obj) ((TsuchiJisanW *) FX(obj))

/* The pass's work (kind 0x1C3, another file's), as far as this file sets it. */
typedef struct PassW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *pol;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *glow;
    /* 0x38 */ u8 rgb[3];
    /* 0x3B */ u8 pad3B;
    /* 0x3C */ s16 alpha;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ s32 timer;
    /* 0x44 */ Obj *player; /* the one it goes to */
} PassW;

/* The hunt's manager's work ("demoTsuchi_proc"), at its task + 0x20. */
typedef struct DemoTsuchiW {
    /* 0x00 */ u8 pad00[4];
    /* 0x04 */ u8 state;
    /* 0x08 */ s32 timer; /* ticks in the state */
    /* 0x0C */ Obj *tsuchi; /* the tsuchikorobi (kind 0x1C2) */
    /* 0x10 */ Camera *cam;
    /* 0x14 */ s8 partake; /* gGameWork.partake while scenario 0x1F5 runs */
} DemoTsuchiW;

#define DEMOTSUCHI(obj) ((DemoTsuchiW *) FX(obj))

/* A tsuchikorobi's work, at its task + 0x20: an enemy's (EneObj) up to 0x40. */
typedef struct TsuchiW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ s8 hp;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk;
    /* 0x38 */ ACollFrame *dmg;
    /* 0x3C */ MPolFrame *shadow;
    /* 0x40 */ u32 halves[2]; /* its two halves' display lists, for its death */
    /* 0x48 */ s8 life; /* hits it takes (3; 5 for the big one) */
    /* 0x4C */ s32 way; /* the nearest player's way (eneNearestPlWay) */
    /* 0x50 */ s16 alpha; /* its env colour's alpha (a flash) */
    /* 0x52 */ u8 count; /* rolls left */
    /* 0x53 */ u8 flags; /* 1: in the air (no fall) */
    /* 0x54 */ f32 floorY; /* below this it has fallen too far and dies */
    /* 0x58 */ s32 timer; /* ticks in the state */
    /* 0x5C */ u8 act; /* what it does near a player: 0, 1 roll, 2 jump, 3 throw */
    /* 0x5D */ u8 id; /* which of the seven (its kinoko bit) */
    /* 0x60 */ u32 *anims; /* its animations (tsuchiKorobiAnims) */
} TsuchiW;

#define TSUCHI(obj) ((TsuchiW *) FX(obj))

/* The count's work ("tsuchiDisp"), at its task + 0x20. */
typedef struct TsuchiDispW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *faces[7]; /* the seven tsuchikorobi */
    /* 0x50 */ PolFrame *battens[7]; /* a cross on each one killed */
    /* 0x6C */ u8 n; /* the crosses made */
    /* 0x6D */ u8 pad6D[3];
    /* 0x70 */ s32 timer;
} TsuchiDispW;

#define TSUCHIDISP(obj) ((TsuchiDispW *) FX(obj))

/* The bridge's work ("tHasi"), at its task + 0x20. */
typedef struct THasiW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[7];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ AnimeFrame *parts[2];
} THasiW;

#define THASI(obj) ((THasiW *) FX(obj))

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

/* fadeSegToRam as this file calls it: the display list's address passed as a pointer (the
 * file's own prototype must have taken one). As a u32 ("(u32) tsuchiHasiDl") IDO treats the
 * address as a value worth keeping: the bridge's two calls share it in a stack temp and the
 * count's loop hoists it into a saved register, where the game builds it at each call. */
typedef Gfx *(*SegToRamFunc)(Gfx *addr, u32 file);

/* acollDmgAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*DmgAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height,
                                     s16 ox, s16 oy, s16 oz, s32 a);

void tsuchiFadeOut(f32 speed);
void tsuchiPassToPlayer(Obj *o, Obj *player);
u8 tsuchiIsKilled(u8 id);
void tsuchiSetKilled(u8 id);
void tsuchiClearKilled(void);
void battenAdd_proc(Obj *o);

extern Gfx tsuchiDispDl[]; /* the count's faces and crosses: their render modes */
extern u32 tsuchiKorobiAnims[]; /* the tsuchikorobi's five animations */
extern f32 tsuchiDispPos[7][3]; /* the count's seven places */
extern Gfx tsuchiHasiDl[]; /* the bridge's render modes */
extern Gfx battenDl[];
extern u8 etskTsuchikorobiAnim[];
extern u8 tsuchiHasiPartAnim0[];
extern u8 tsuchiHasiPartAnim1[];
extern u8 etskTsuchikorobiHalf0Dl[];
extern u8 etskTsuchikorobiHalf1Dl[];
extern Gfx tsuchiDispFaceDl[];

extern Unk88282 gSaveCopyPlayers[4]; /* the players' entries for a continue (stage.c) */
extern CamView *tsuchiDispCam; /* the count's camera (an orthographic one) */
extern Obj *tsuchiDispTask; /* the count's task ("tsuchiDisp") */

/* The file's .data, in address order (its externs above stay as declarations). */
extern u8 tsuchiKorobiAnim1[];
extern u8 tsuchiKorobiAnim2[];
extern u8 tsuchiKorobiAnim4[];
Gfx tsuchiDispDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsDPSetRenderMode(CVG_DST_CLAMP | ZMODE_OPA | CVG_X_ALPHA | FORCE_BL | G_RM_PASS, CVG_DST_CLAMP | ZMODE_OPA | CVG_X_ALPHA | FORCE_BL | GBL_c2(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1)),
    gsDPSetCombineLERP(PRIMITIVE, 0, SHADE, 0, 0, 0, 0, PRIMITIVE, PRIMITIVE, 0, SHADE, 0, 0, 0, 0, PRIMITIVE),
    gsDPPipeSync(),
    gsDPSetBlendColor(0x00, 0x00, 0x00, 0x01),
    gsDPSetAlphaCompare(G_AC_THRESHOLD),
    gsSPEndDisplayList(),
}; /* the count's faces and crosses: their render modes */
CamView *tsuchiDispCam = (CamView *) 0x12345670; /* the count's camera (an orthographic one) */
Obj *tsuchiDispTask = (Obj *) 0x12345670; /* the count's task ("tsuchiDisp") */
u32 tsuchiKorobiAnims[] = { (u32) etskTsuchikorobiAnim, (u32) tsuchiKorobiAnim1, (u32) tsuchiKorobiAnim2, 0x03000748, (u32) tsuchiKorobiAnim4 }; /* the tsuchikorobi's five animations */
f32 tsuchiDispPos[7][3] = { 86.0f, 36.0f, 0.0f, 111.0f, 36.0f, 0.0f, 136.0f, 36.0f, 0.0f, 161.0f, 36.0f, 0.0f, 186.0f, 36.0f, 0.0f, 211.0f, 36.0f, 0.0f, 236.0f, 36.0f, 0.0f }; /* the count's seven places */
Gfx tsuchiHasiDl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPPipelineMode(G_PM_NPRIMITIVE),
    gsDPSetAlphaDither(G_AD_DISABLE),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsDPSetCombineKey(G_CK_NONE),
    gsDPSetTextureLOD(G_TL_TILE),
    gsDPSetTextureDetail(G_TD_CLAMP),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsDPSetTextureConvert(G_TC_FILT),
    gsDPSetTextureLUT(G_TT_NONE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsDPSetRenderMode(G_RM_RA_ZB_OPA_SURF, G_RM_RA_ZB_OPA_SURF2),
    gsDPSetAlphaCompare(G_AC_NONE),
    gsSPEndDisplayList(),
    gsDPNoOp(),
}; /* the bridge's render modes */
extern u8 tsuchiJisanAnim[];

/* The screen fades to black at the given speed. The first call is on the brace's line: IDO
 * schedules by line, and only there does "sw ra" come after the call's address, as the game has
 * it (file 127's beatFadeOut, measured there). */
void tsuchiFadeOut(f32 speed) { fadeSetSpeed(speed);
    fadeSetColor(0, 0, 0);
    fadeSetTarget(255.0f);
}

/* The pass goes to the player (state 3). */
void tsuchiPassToPlayer(Obj *o, Obj *player) {
    PassW *w = (PassW *) FX(o);

    w->state = 3;
    w->player = player;
    w->timer = 0;
}

void tsuchiEventTegataSdSet(void) {
    DPRINTF("&&&& eventTegataSdSet  pl D がないn");
}

void tsuchiEventSdClear(void) {
    DPRINTF("&&&& SD_G_CLEAR1n");
    DPRINTF("&&&& SD_E_CLEAR1n");
    DPRINTF("&&&& SD_S_CLEAR1n");
    DPRINTF("&&&& SD_Y_CLEAR1n");
}

/* "tsuchijisan": the old man. Talked to, he turns to the player and talks; if the player takes
 * on the hunt (0x94) the screen fades and the game goes to the atlas with the hunt's stage open;
 * back from it (0x92), he gives the pass between them (kind 0x1C3, sparks and kind 0x1C1), the
 * pass goes to the player (scenario 0x20), and the sky's tunes change. */
s32 tsuchiJisanProc(Obj *o, s16 msg, void *arg) {
    TsuchiJisanW *w = TSUCHIJISAN(o);
    Obj *player;
    u32 data;
    CamView *view;
    s32 pad;
    Vec3f pos;
    s32 i;
    Vec3f at;
    s32 pad2[3];
    f32 x;
    f32 y;
    f32 z;
    Vec3f spark;

    switch (msg) {
        case 2:
            o->name = "tsuchijisan";
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) tsuchiJisanAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->rot[1] = 0.2454369962f;
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            demoTalkInit(o, data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("tsuchiJisan_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            if (swWorkTest(0x92)) {
                swWorkReset(0x92);
                gGameWork.scenarioRunning = 2;
                camSunSetClockStop(1);
                w->state = 8;
            }
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            demoTalkerFree(o);
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    for (i = 0; i < 4; i++) {
                        if (plIsInPlay(i) && !(gGameWork.players[i]->flags & 2)) {
                            tskSendMsg(gGameWork.players[i], 4, &pos);
                            eneTurnToPoint(o, pos.x, pos.z, 10.0f);
                            break;
                        }
                    }
                    if (demoTalkEndCheck(o)) {
                        demoCamZoom_CL(w->talker);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (w->talker != NULL) {
                        tskSendMsg(w->talker, 4, &at);
                        if (eneTurnToPoint(o, at.x, at.z, 10.0f) == 1) {
                            w->state = 3;
                        }
                    }
                    break;
                case 3:
                    scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n", w->data->talk);
                    w->state = 4;
                    break;
                case 4:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x94)) {
                            swWorkReset(0x94);
                            gGameWork.scenarioRunning = 2;
                            w->state = 7;
                            w->timer = 0;
                        } else {
                            personTalkCamOff();
                            demoFinish(o);
                            camSunSetClockStop(1);
                            w->state = 0;
                        }
                    }
                    break;
                case 7:
                    if (w->timer == 1) {
                        sndPlay(10);
                        tsuchiFadeOut(16 / gRetracesPerFrame);
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        view = camGetView(w->cam);
                        camSetSpeed(w->cam, 20 / gRetracesPerFrame);
                        camMoveAtY(w->cam, view->at[1] + 50.0f);
                        gGameWork.scenarioRunning = 2;
                        for (i = 0; i < 4; i++) {
                            gSaveCopyPlayers[i].chara = -1;
                        }
                    }
                    if (w->timer == 20 / gRetracesPerFrame) {
                        gGameWork.scenarioRunning = 0;
                        camAiSetHold(0);
                        demoAreaFlagSave();
                        demoAreaFlagSetBit7(0);
                        gSaveData.stageFlags[3] |= 1;
                        gSaveData.stageFlags[3] &= ~0x80;
                        gSaveData.areaFlags[3] |= 1;
                        gSaveData.areaFlags[3] &= ~0x80;
                        stageGotoAtlas(0, 2);
                    }
                    break;
                case 8:
                    if (w->timer == 1) {
                        demoPlStandAll();
                    }
                    if (w->timer == 8) {
                        if (scKinokoCheck() < 7) {
                            camSunSetTunes4(0x79, 0x76, 0x77, 0x78);
                            sndPlay(0x51F);
                        }
                        personCamZoomUp(plCheck());
                        swWorkReset(0x92);
                        demoAreaFlagRestore();
                        scRunStart(w->data->talk, 0, 0);
                        w->state = 10;
                    }
                    break;
                case 10:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x93)) {
                            w->state = 5;
                            swWorkReset(0x93);
                        } else {
                            w->state = 9;
                        }
                    }
                    break;
                case 5:
                    player = plCheck();
                    if (player == NULL) {
                        return -1;
                    }
                    plGetModelPos(player, &x, &y, &z);
                    w->pass = eneMakeEx(NULL, ENE_DEMO_TEGATA, 0, (w->model->pos[0] + x) / 2, w->model->pos[1],
                                        (w->model->pos[2] + z) / 2, 0, NULL);
                    w->timer = 0;
                    w->state = 6;
                    break;
                case 6:
                    if (w->timer == 200 / gRetracesPerFrame) {
                        demoGetEneModelPos(w->pass, &spark);
                        ptclSpawnSparksFall(spark.x, spark.y, spark.z);
                        eneMakeEx(NULL, ENE_DEMO_KIRAKIRA, 0, spark.x, spark.y, spark.z, 0, NULL);
                        tsuchiPassToPlayer(w->pass, plCheck());
                        w->flags |= 1;
                    }
                    if (w->timer == 260 / gRetracesPerFrame) {
                        scRunStart(0x20, 0, 0);
                        w->state = 9;
                    }
                    break;
                case 9:
                    if (!scIsRunning()) {
                        personTalkCamOff();
                        demoPlReleaseAll();
                        camSunSetClockStop(1);
                        w->state = 0;
                    }
                    break;
            }
            if (w->flags & 1) {
                if (300 / gRetracesPerFrame == w->tuneTimer++) {
                    w->flags &= ~1;
                    camSunSetTunes4(0x79, 0x76, 0x77, 0x78);
                }
            }
            eneFall(o, 0.4f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.5f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "demoTsuchi_proc": the hunt's manager. Made only while the hunt is on (flag 5) and the pass
 * not yet given (0x97). The first time (0x1B) the clock goes to 6:00. It makes the count's
 * camera and, once the players stand, runs scenario 0x1F5, then makes kind 0x160 and the
 * tsuchikorobi (kind 0x1C2). When all seven are found (scKinokoCheck) at night's end it runs
 * scenario 0x21 and the players watch; then (or when the mission is over, 2) the screen fades
 * and the game goes back to the old man (spawn 0xA6) with flag 0x92 set. */
s32 demoTsuchi_proc(Obj *o, s16 msg, void *arg) {
    DemoTsuchiW *w = DEMOTSUCHI(o);
    s32 pad[4];
    s32 i;
    CamView *view;

    switch (msg) {
        case 2:
            o->name = "demoTsuchi_proc";
            if (!swWorkTest(5)) {
                DPRINTF("demoTsuchi_proc: 土転び抹殺計画は開始されません\n");
                return -1;
            }
            if (swWorkTest(0x97)) {
                DPRINTF("demoTsuchi_proc: 土転び抹殺計画は開始されません 手形もらってるで\n");
                return -1;
            }
            if (!swWorkTest(0x1B)) {
                swWorkSet(0x1B);
                gGameWork.time = 0x60000;
                w->state = 0;
            } else {
                w->state = 1;
            }
            tsuchiDispCam = (CamView *) frmCreate((Frame *) gFramePool10, gFrmKindCamera);
            if (tsuchiDispCam == NULL) {
                return -1;
            }
            tsuchiDispCam->ortho |= 1;
            tsuchiDispCam->zNear = -200.0f;
            tsuchiDispCam->zFar = 200.0f;
            gGameWork.mission = 1;
            gGameWork.scenarioRunning = 2;
            gGameWork.areaStage = 2;
            tsuchiDispTask = NULL;
            DPRINTF("demoTsuchi セット完了\n");
            break;
        case 1:
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->timer == 1) {
                        demoPlStandAll();
                    }
                    if (w->timer == 8) {
                        camSunSetClockStop(1);
                        w->partake = gGameWork.partake;
                        gGameWork.partake = 1;
                        scRunStart2(0x1F5, 0, 0);
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (!scIsRunning()) {
                        gGameWork.partake = w->partake;
                        camSunSetClockStop(0);
                        demoPlReleaseAll();
                        tsuchiClearKilled();
                        eneMakeEx(NULL, ENE_TSUCHI_DISP, 0, 0.0f, 0.0f, 0.0f, 2, NULL);
                        w->tsuchi = eneMakeEx(NULL, ENE_DEMO_TIMER, 0, 0.0f, 0.0f, 0.0f, 2,
                                              (u32 (*)(void *, void *, void *)) 0x12);
                        DPRINTF("demoTsuchi_proc: 土ころび出現\n");
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (gGameWork.time >= 0x120000 && gGameWork.time < gGameWork.rate + 0x120000) {
                        w->timer = 0;
                        w->state = 3;
                    }
                    if (scKinokoCheck() == 7) {
                        scRunStart2(0x21, 0, 0);
                        gGameWork.partake = 1;
                        for (i = 0; i < 4; i++) {
                            if (plIsInPlay(i) == 1) {
                                plEnterDemo(gGameWork.players[i]);
                                PL(gGameWork.players[i])->flags |= 0x10000000;
                            }
                        }
                        camAiSetHold(1);
                        fadeSetColor(0, 0, 0);
                        fadeSetLevel(128.0f);
                        w->timer = 0;
                        w->state = 4;
                    }
                    break;
                case 3:
                    if (40 / gRetracesPerFrame < w->timer && !scIsRunning()) {
                        gGameWork.scenarioRunning = 2;
                        sndPlay(10);
                        w->timer = 0;
                        w->state = 7;
                    }
                    break;
                case 4:
                    if (40 / gRetracesPerFrame < w->timer && !scIsRunning()) {
                        gGameWork.scenarioRunning = 2;
                        w->timer = 0;
                        w->state = 7;
                    }
                    break;
                case 7:
                    if (w->timer == 1) {
                        for (i = 0; i < 4; i++) {
                            contPakInitRumble(i);
                            contPakRumbleRequestStop(i);
                        }
                        swWorkReset(0x1B);
                        tsuchiFadeOut(16 / gRetracesPerFrame);
                        if (w->tsuchi != NULL) {
                            w->tsuchi = tskDestroy(w->tsuchi);
                        }
                        if (tsuchiDispTask != NULL) {
                            tsuchiDispTask = tskDestroy(tsuchiDispTask);
                        }
                        camAiSetHold(1);
                        w->cam = camAiGetCam();
                        view = camGetView(w->cam);
                        camSetSpeed(w->cam, 20 / gRetracesPerFrame);
                        camMoveAtY(w->cam, view->at[1] + 50.0f);
                        gGameWork.scenarioRunning = 2;
                        if (scKinokoCheck() < 7) {
                            sndPlay(0x45F);
                        }
                        sndPlay(0x8107);
                    }
                    if (w->timer == 20 / gRetracesPerFrame) {
                        gGameWork.areaStage = 2;
                        gGameWork.scenarioRunning = 0;
                        gGameWork.mission = 0;
                        gGameWork.partake = 0;
                        swWorkSet(0x92);
                        stageGotoSpawn(gStageTask->parent, 0xA6, 0, -15.0f, 10.0f, 12.0f, 3.141592741f, 0xD7);
                    }
                    break;
            }
            if (gGameWork.mission == 2) {
                if (w->state != 7) {
                    DPRINTF("%%%%   event manager: PLAYER 死にました信号をうけとったぜ！\n");
                    gGameWork.scenarioRunning = 2;
                    sndPlay(10);
                    demoPlStandAll();
                    w->timer = 0;
                    w->state = 7;
                }
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "tsuchikorobi": an earth-roller (id 0-6 from its placement; 3 is the big one). Made while the
 * hunt is on and it has not been killed (gGameWork.kinoko). Near a player it acts by its act:
 * rolls toward the player (1), jumps (2), or throws (kind 0xDA, 4 and 5). Hit, it flashes; killed
 * it marks itself, crosses itself out on the count and dies (no item for the last one). At
 * night's end it jumps away. */
s32 Etsk_tsuchikorobi_proc(Obj *o, s16 msg, void *arg) {
    TsuchiW *w = TSUCHI(o);
    s32 data;
    f32 dx;
    f32 dy;
    s32 r;

    switch (msg) {
        case 2:
            o->name = "tsuchikorobi";
            if (!swWorkTest(5)) {
                DPRINTF(">>>>   土転びでない ミッションたっていない\n");
                return -1;
            }
            if (swWorkTest(0x97)) {
                DPRINTF(">>>>    土転びでない 手形もらってる\n");
                return -1;
            }
            data = eneSetup(o, arg);
            w->act = data & 0xFF;
            w->id = (data >> 8) & 0xFF;
            if (tsuchiIsKilled(w->id)) {
                DPRINTF(">>>>  土転び(id=[%d])は既に殺・さ・れ・た\n", w->id);
                return -1;
            }
            w->model = eneAnimeModelMake(o, (u32) etskTsuchikorobiAnim, NULL, 0);
            if (w->model == NULL) {
                return -1;
            }
            w->anims = tsuchiKorobiAnims;
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("Etsk_tsuchikorobi_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->atk = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 80, 0, 80, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x78;
            w->dmg = ((DmgAddXYZFunc) acollDmgAddXYZ)(ene_dmg_root_frame, &w->model->f, 110, 180, 0, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->halves[0] = (u32) etskTsuchikorobiHalf0Dl;
            w->halves[1] = (u32) etskTsuchikorobiHalf1Dl;
            w->alpha = 0;
            w->life = 3;
            w->floorY = w->model->pos[1] - 100.0f;
            DPRINTF(">>>>  土転び(id=[%d] act=[%d])セットＯＫ\n", w->id, w->act);
            if (w->id == 3) {
                w->model->scale[2] = 0.15f;
                w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
                w->life = 5;
            }
            break;
        case 3:
            if (w->state == 4 || w->state == 5) {
                w->model->dl = eneDlEnvColorLit(o, w->alpha, 255, 0, 150, 1);
            } else if (w->state == 8 || w->state == 9 || w->state == 10) {
                w->model->dl = eneDlEnvColorLit(o, w->alpha, 255, 255, 255, 1);
            } else if (w->state == 1) {
                w->model->dl = eneDlEnvColorLit(o, w->alpha, 255, 255, 0, 1);
            } else {
                w->model->dl = eneDlEnvColorLit(o, w->alpha, 0, 0, 0, 1);
            }
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (w->timer == 1) {
                        w->model->anim = w->anims[0];
                        w->model->frame = 0.0f;
                        w->vel[0] = 0.0f;
                    }
                    eneAnimLoop(o, w->model, 0.5f);
                    if (eneNearerPl12(o, 90.0f)) {
                        switch (w->act) {
                            default:
                            case 0:
                                w->state = 1;
                                w->act = 0;
                                break;
                            case 1:
                                w->state = 2;
                                w->act = 0;
                                break;
                            case 2:
                                w->act = 0;
                                w->state = 4;
                                break;
                            case 3:
                                w->state = 4;
                                break;
                        }
                        w->timer = 0;
                    }
                    break;
                case 1:
                    if (w->timer == 1) {
                        w->model->anim = w->anims[1];
                        w->model->frame = 0.0f;
                        if (eneNearestPlWay(o) == 4) {
                            w->vel[0] = -2.0f;
                        } else {
                            w->vel[0] = 2.0f;
                        }
                        w->way = eneNearestPlWay(o);
                        w->count = 5;
                    }
                    w->alpha = 80;
                    if (eneAnimPlayOnce(o, w->model, 2.0f) == 1) {
                        w->model->frame = 0.0f;
                        w->count--;
                    }
                    eneTurnToAngle(o, eneCourseAngle(o, w->way) * 360.0f / 6.283185482f, 20.0f);
                    if (w->timer % 3 == 0) {
                        itexPuffWhite(w->model->pos[0], w->model->pos[1] + 3.0f, w->model->pos[2], 0.025f);
                    }
                    if (w->timer % (15 / gRetracesPerFrame) == 0) {
                        soundIncamCam(0x23D, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
                    }
                    if (w->count == 0) {
                        w->dmg->flags |= 2;
                        w->alpha = 0;
                        w->state = 2;
                        w->timer = 0;
                    }
                    break;
                case 2:
                    if (w->timer == 1) {
                        w->model->anim = w->anims[2];
                        w->model->frame = 0.0f;
                        w->vel[0] = 0.0f;
                    }
                    if ((s32) w->model->frame == 5) {
                        soundIncamCam(0x23A, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
                        DPRINTF("Etsk_tsuchikorobi_proc: ジャンプするで\n");
                        if (eneNearestPlWay(o) == 4) {
                            w->vel[0] = -1.0f;
                        } else {
                            w->vel[0] = 1.0f;
                        }
                        if (!eneWallCheckNoPush(o, 10.0f, 20.0f, w->z, 25.0f)) {
                            DPRINTF("Etsk_tsuchikorobi_proc: ジャンプ着地地点があたりない\n");
                            w->vel[0] = 0.0f;
                        }
                        w->flags |= 1;
                        w->vel[1] = 4.5f;
                        w->way = eneNearestPlWay(o);
                        w->timer = 0;
                        w->state = 3;
                    }
                    w->way = eneNearestPlWay(o);
                    eneTurnToAngle(o, eneCourseAngle(o, w->way) * 360.0f / 6.283185482f, 20.0f);
                    eneAnimPlayOnce(o, w->model, 0.4f);
                    break;
                case 3:
                    eneTurnToAngle(o, eneCourseAngle(o, w->way) * 360.0f / 6.283185482f, 20.0f);
                    if ((s32) w->model->frame == 8) {
                        w->flags &= ~1;
                    }
                    if (eneAnimPlayOnce(o, w->model, 0.4f) == 1 && (w->hit & 1)) {
                        itexPuff(w->model->pos[0], w->model->pos[1] - 5.0f, w->model->pos[2], 0.12f);
                        soundIncamCam(0x2EE, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
                        w->state = 4;
                        w->timer = 0;
                    }
                    break;
                case 4:
                    if (w->timer == 1) {
                        w->model->anim = w->anims[3];
                        w->model->frame = 0.0f;
                        w->vel[0] = 0.0f;
                    }
                    w->way = eneNearestPlWay(o);
                    if ((s32) w->model->frame == 15) {
                        DPRINTF("Etsk_tsuchikorobi_proc: pldir[%d]\n", w->way);
                        if (w->way == 1) {
                            dx = 20.0f;
                        } else {
                            dx = -20.0f;
                        }
                        if (w->id == 3) {
                            dy = 15.0f;
                        } else {
                            dy = 9.0f;
                        }
                        eneMakeEx(NULL, ENE_TB_HANI_TAMA_MAIN, w->zone, w->x + dx, w->y + dy, w->z, 2,
                                  (u32 (*)(void *, void *, void *)) w->way);
                        soundIncamCam(0x22F, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
                        w->state = 5;
                    }
                    w->alpha += 5;
                    if (w->alpha > 80) {
                        w->alpha = 80;
                    }
                case 5:
                    eneTurnToAngle(o, eneCourseAngle(o, w->way) * 360.0f / 6.283185482f, 20.0f);
                    if (w->state == 5) {
                        w->alpha -= 80 / (polAnimeLastFrame(w->model) - 14);
                        if (w->alpha <= 0) {
                            w->alpha = 0;
                        }
                    }
                    if (eneAnimPlayOnce(o, w->model, 0.6f) == 1) {
                        w->alpha = 0;
                        w->state = 6;
                        w->timer = 0;
                        if (w->act == 3) {
                            w->state = 4;
                        }
                    }
                    break;
                case 6:
                    if (w->timer == 1) {
                        w->model->anim = w->anims[2];
                        w->model->frame = 0.0f;
                        w->vel[0] = 0.0f;
                    }
                    if ((s32) w->model->frame == 5) {
                        soundIncamCam(0x23A, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
                        DPRINTF("Etsk_tsuchikorobi_proc: ジャンプするで\n");
                        if (eneNearestPlWay(o) == 4) {
                            w->vel[0] = -2.0f;
                        } else {
                            w->vel[0] = 2.0f;
                        }
                        if (!eneWallCheckNoPush(o, 10.0f, 60.0f, w->z, 10.0f)) {
                            DPRINTF("Etsk_tsuchikorobi_proc: ジャンプ着地地点があたりない\n");
                            w->vel[0] = 0.0f;
                        }
                        w->flags |= 1;
                        w->vel[1] = 4.5f;
                        w->way = eneNearestPlWay(o);
                        w->timer = 0;
                        w->state = 7;
                    }
                    w->way = eneNearestPlWay(o);
                    eneTurnToAngle(o, eneCourseAngle(o, w->way) * 360.0f / 6.283185482f, 20.0f);
                    eneAnimPlayOnce(o, w->model, 0.4f);
                    break;
                case 7:
                    eneTurnToAngle(o, eneCourseAngle(o, w->way) * 360.0f / 6.283185482f, 20.0f);
                    if ((s32) w->model->frame == 8) {
                        w->flags &= ~1;
                    }
                    if (eneAnimPlayOnce(o, w->model, 0.4f) == 1 && (w->hit & 1)) {
                        if (!eneNearerPl12(o, 90.0f) || w->act != 0) {
                            soundIncamCam(0x2EE, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
                        }
                        itexPuff(w->model->pos[0], w->model->pos[1] - 5.0f, w->model->pos[2], 0.12f);
                        w->state = 0;
                        w->timer = 0;
                    }
                    break;
                case 8:
                    if (w->timer == 1) {
                        w->life--;
                        w->model->anim = w->anims[4];
                        w->model->frame = 0.0f;
                        w->vel[0] = 0.0f;
                        w->flags &= ~1;
                        w->vel[1] = 0.0f;
                        w->atk->flags &= ~2;
                        w->dmg->flags &= ~2;
                        w->way = eneNearestPlWay(o);
                        DPRINTF("Etsk_tsuchikorobi: hp [%d]\n", w->life);
                        if (w->life <= 0) {
                            w->state = 11;
                            break;
                        }
                        eneHitSoundHeavy(w->dmg);
                        eneMakeHitSpark(w->zone, w->x, w->y + 10.0f, w->z, 2, w->rot[1]);
                    }
                    if ((s32) w->model->frame == 1) {
                        w->timer = 0;
                        w->state = 9;
                    }
                    eneAnimPlayOnce(o, w->model, 1.0f);
                    break;
                case 9:
                    if (w->timer == 1) {
                        if (!eneWallCheckPush(o, 10.0f, 20.0f, w->z, 4.0f)) {
                            w->vel[0] = 0.0f;
                        } else if (w->way == 1) {
                            w->vel[0] = -1.0f;
                        } else {
                            w->vel[0] = 1.0f;
                        }
                        w->flags |= 1;
                        w->vel[1] = 1.5f;
                    }
                    if (!(w->timer & 1)) {
                        w->alpha = 255;
                    } else {
                        w->alpha = 0;
                    }
                    if (w->timer == 16 / gRetracesPerFrame) {
                        w->flags &= ~1;
                        w->hit &= ~1;
                    }
                    if (!(w->flags & 1) && (w->hit & 1)) {
                        w->state = 10;
                    }
                    break;
                case 10:
                    if (w->timer % 3 == 0) {
                        w->alpha = 255;
                    } else {
                        w->alpha = 0;
                    }
                    if (eneAnimPlayOnce(o, w->model, 0.3f) == 1) {
                        w->alpha = 0;
                        w->atk->flags |= 2;
                        w->dmg->flags |= 2;
                        w->state = 0;
                        w->timer = 0;
                    }
                    break;
                case 11:
                    if (scKinokoCheck() == 6) {
                        DPRINTF("______Etsk_tsuchikorobi: 最後だからアイテムなしだ\n");
                        demoEneDeathAct(o, w->dmg, w->halves[0], w->halves[1], 0);
                    } else {
                        demoEneDeathAct(o, w->dmg, w->halves[0], w->halves[1], 1);
                    }
                    tsuchiSetKilled(w->id);
                    battenAdd_proc(tsuchiDispTask);
                    DPRINTF("Etsk_tsuchikorobi: 土ころび  [%d]匹目を殺った\n", scKinokoCheck());
                    DPRINTF("さようなら id[%d]\n", w->id);
                    return -1;
                case 12:
                    if (w->timer == 1) {
                        w->dmg->flags &= ~2;
                        w->model->anim = w->anims[2];
                        w->model->frame = 0.0f;
                        w->vel[0] = 0.0f;
                    }
                    if ((s32) w->model->frame == 5) {
                        DPRINTF("Etsk_tsuchikorobi_proc: ジャンプするで\n");
                        w->flags |= 1;
                        w->vel[2] = 1.5f;
                        w->vel[1] = 4.5f;
                    }
                    eneTurnToAngle(o, eneCourseAngle(o, 8) * 360.0f / 6.283185482f, 20.0f);
                    if ((s32) w->model->frame == 8) {
                        w->flags &= ~1;
                        w->timer = 0;
                        w->state = 13;
                    }
                    eneAnimPlayOnce(o, w->model, 0.4f);
                    break;
                case 13:
                    if (w->timer == 200 / gRetracesPerFrame) {
                        return -1;
                    }
                    break;
            }
            if (acollAtkWasHit(w->atk) == 1) {
                if ((Obj *) w->atk->hitPower != NULL) {
                    tskSendMsg((Obj *) w->atk->hitPower, 7, o);
                }
            }
            if (acollDmgWasHit(w->dmg) == 1) {
                if (w->state != 8 && w->state != 9 && w->state != 10) {
                    w->state = 8;
                    w->timer = 0;
                }
            }
            if (gGameWork.time >= 0x120000 && gGameWork.time < gGameWork.rate + 0x120000) {
                w->timer = 0;
                w->state = 12;
            }
            r = eneWallCheckPush(o, 10.0f, 8.0f, w->z, 4.0f);
            if (r == -1 || r == 0) {
                DPRINTF("かべがけチェック\n");
                if (w->state != 9 && w->state != 2 && w->state != 3 && w->state != 6 && w->state != 7 &&
                    w->state != 12 && w->state != 13) {
                    w->vel[0] = 0.0f;
                }
            }
            if (!(w->flags & 1)) {
                eneFall(o, 0.4f);
                w->hit &= ~1;
                if (eneLand((EneObj *) w, 10.0f) == 1) {
                    w->hit |= 1;
                    w->vel[1] = 0.0f;
                }
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 2.0f, NULL);
            if (w->model->pos[1] < w->floorY) {
                DPRINTF("Etsk_tsuchikorobi_proc: 落ちすぎて死ぬる id[%d]\n", w->id);
                eneReleaseSpawn(w->spawn);
                return -1;
            }
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* Whether tsuchikorobi id has been killed (its bit in gGameWork.kinoko). */
u8 tsuchiIsKilled(u8 id) {
    return gGameWork.kinoko & (u8) (1 << (u8) (id % 8));
}

/* Tsuchikorobi id has been killed. */
void tsuchiSetKilled(u8 id) {
    gGameWork.kinoko |= (u8) (1 << (u8) (id % 8));
}

void tsuchiDummy(void) {
}

/* None killed yet. */
void tsuchiClearKilled(void) {
    gGameWork.kinoko = 0;
}

/* "tsuchiDisp": the count on the screen, seven faces in a row under the hunt's camera, each
 * crossed out (a "batten", grown in) when its tsuchikorobi is killed. */
s32 tsuchiDispProc(Obj *o, s16 msg, void *arg) {
    TsuchiDispW *w = TSUCHIDISP(o);
    s32 i;
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "tsuchiDisp";
            tsuchiDispTask = o;
            eneSetup(o, arg);
            for (i = 0; i < 7; i++) {
                w->faces[i] = (PolFrame *) frmCreate(&tsuchiDispCam->f, gFrmKindPol);
                if (w->faces[i] == NULL) {
                    return -1;
                }
                w->faces[i]->dl = ((SegToRamFunc) fadeSegToRam)(tsuchiDispDl, o->file);
                w->faces[i]->dl2 = tsuchiDispFaceDl;
                floadGetSetRam((void **) w->faces[i]->segs, 0x82);
                w->faces[i]->rot[0] = 3.141592741f;
                w->faces[i]->rot[1] = 1.570796371f;
                w->faces[i]->pos[0] = tsuchiDispPos[i][0];
                w->faces[i]->pos[1] = tsuchiDispPos[i][1];
                /* A double literal: written 0.08f, IDO shares one 0.08f with message 0's (f20 for
                 * both, which also moves pi and 90 degrees to f22 and f24). */
                w->faces[i]->scale[2] = 0.08;
                w->faces[i]->scale[0] = w->faces[i]->scale[1] = w->faces[i]->scale[2];
            }
            floadLoadSet(0x83);
            break;
        case 1:
            for (i = 0; i < 7; i++) {
                if (w->faces[i] != NULL) {
                    frmDestroy(&w->faces[i]->f);
                }
                if (w->battens[i] != NULL) {
                    frmDestroy(&w->battens[i]->f);
                }
            }
            tsuchiDispTask = NULL;
            break;
        case 0:
            for (i = 0; i < 7; i++) {
                if (w->battens[i] != NULL) {
                    if (w->battens[i]->scale[0] < 0.07999999821f) {
                        w->battens[i]->scale[0] += 0.01999999955f;
                        w->battens[i]->scale[1] += 0.01999999955f;
                        w->battens[i]->scale[2] += 0.01999999955f;
                    } else {
                        w->battens[i]->scale[2] = 0.07999999821f;
                        w->battens[i]->scale[0] = w->battens[i]->scale[1] = w->battens[i]->scale[2];
                    }
                }
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "battenAdd_proc": a cross over the next face of the count. */
void battenAdd_proc(Obj *o) {
    TsuchiDispW *w = TSUCHIDISP(o);

    w->battens[w->n] = (PolFrame *) frmCreate(&tsuchiDispCam->f, gFrmKindPol);
    if (w->battens[w->n] == NULL) {
        DPRINTF("battenAdd_proc:Error: not enough frame memory\n");
    }
    w->battens[w->n]->dl = ((SegToRamFunc) fadeSegToRam)(tsuchiDispDl, o->file);
    w->battens[w->n]->dl2 = battenDl;
    floadGetSetRam((void **) w->battens[w->n]->segs, 0x83);
    w->battens[w->n]->rot[0] = 3.141592741f;
    w->battens[w->n]->pos[0] = tsuchiDispPos[w->n][0];
    w->battens[w->n]->pos[1] = tsuchiDispPos[w->n][1];
    w->battens[w->n]->scale[2] = 0.009999999776f;
    w->battens[w->n]->scale[0] = w->battens[w->n]->scale[1] = w->battens[w->n]->scale[2];
    w->n++;
    DPRINTF("battenAdd_proc:Error: batten set!\n");
}

/* "tHasi": a bridge in two animated parts. */
s32 tsuchiHasiProc(Obj *o, s16 msg, void *arg) {
    THasiW *w = THASI(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "tHasi";
            eneSetup(o, arg);
            w->parts[0] = eneNewAnime2(o, (u32) tsuchiHasiPartAnim0, ((SegToRamFunc) fadeSegToRam)(tsuchiHasiDl, o->file));
            w->parts[0]->scale[2] = 0.2520000041f;
            w->parts[0]->scale[0] = w->parts[0]->scale[1] = w->parts[0]->scale[2];
            eneSetModel(o, &w->parts[0]->f);
            w->parts[1] = eneNewAnime2(o, (u32) tsuchiHasiPartAnim1, ((SegToRamFunc) fadeSegToRam)(tsuchiHasiDl, o->file));
            w->parts[1]->scale[2] = 0.2520000041f;
            w->parts[1]->scale[0] = w->parts[1]->scale[1] = w->parts[1]->scale[2];
            eneSetModel(o, &w->parts[1]->f);
            break;
        case 1:
            if (w->parts[0] != NULL) {
                frmDestroy(&w->parts[0]->f);
            }
            if (w->parts[1] != NULL) {
                frmDestroy(&w->parts[1]->f);
            }
            break;
        case 0:
            eneAnimLoop(o, w->parts[0], 0.1f);
            eneAnimLoop(o, w->parts[1], 0.1f);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
