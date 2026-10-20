#include "tlb.h"

/* File 139: Hagure town's gate ("gate", townGate_proc) and the rivers and waves ("kawa", "nami",
 * "kaiki", "derokawa1/2", "kirikawa1/2/5"): scrolling-texture water, each with its display list. */

/* The gate's work ("gate"), at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct GateW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit; /* 1 on the ground */
    /* 0x0B */ s8 hp;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ ACollFrame *atk; /* the box that opens it when hit */
    /* 0x38 */ s32 timer; /* ticks in the state */
} GateW;

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

extern u8 townGateAnim[]; /* the gate's animation */

/* "gate": Hagure town's gate. Hit, it opens, the screen fades and the game goes to the map with
 * the next stage and areas open. */
s32 townGate_proc(Obj *o, s16 msg, void *arg) {
    GateW *w = (GateW *) FX(o);

    switch (msg) {
        case 2:
            o->name = "gate";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) townGateAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->atk = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 20, 0, 20, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x66;
            w->atk->ofs[2] = -10;
            if (!(gSaveData.stageFlags[101] & 0x80)) {
                PRINTF(">>>>>>>>>>>>>  townGate_proc: sas 入閣デモまだ\n");
                w->state = 4;
            }
            if (eneSetOnEarth(o) == -1) {
                PRINTF("townGate_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            PRINTF("townGate_proc: はぐれ町の門ＯＫ\n");
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (acollAtkWasHit(w->atk) == 1) {
                        soundIncamCam(700, w->model->pos[0], w->model->pos[1], w->model->pos[2]);
                        demoPlFreezeAll();
                        w->state = 1;
                    }
                    break;
                case 1:
                    if (eneAnimPlayOnce(o, w->model, 0.05f)) {
                        w->timer = 0;
                        w->state = 2;
                    }
                    break;
                case 2:
                    if (w->timer == 1) {
                        fadeSetAll(10.0f, 0.0f, 255.0f, 0, 0, 0);
                        sndPlay(10);
                    }
                    if (w->timer > 10) {
                        w->state = 3;
                    }
                    break;
                case 3:
                    gSaveData.stageFlags[2] |= 1;
                    gSaveData.areaFlags[2] |= 1;
                    gSaveData.areaFlags[3] |= 1;
                    stageGotoAtlas(0, 2);
                    break;
                case 4: /* waits: the sasuke-joins demo has not been seen */
                    break;
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
            eneStepHookPost(o);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* A 64x32 CI4 texture loaded into TMEM at tmem for tile rtile, as gDPLoadMultiBlock_4b, with its
 * tile size at (uls, ult): one macro (one source line: IDO schedules by line), as tlb_0050's. */
#define gDPLoadScrollBlock_4b(pkt, timg, tmem, rtile, uls, ult)                                       \
    {                                                                                                  \
        gDPSetTextureImage(pkt, G_IM_FMT_CI, G_IM_SIZ_16b, 1, timg);                                   \
        gDPSetTile(pkt, G_IM_FMT_CI, G_IM_SIZ_16b, 0, tmem, G_TX_LOADTILE, 0,                          \
                   G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 6, G_TX_NOLOD); \
        gDPLoadSync(pkt);                                                                              \
        gDPLoadBlock(pkt, G_TX_LOADTILE, 0, 0, ((64 * 32 + 3) >> 2) - 1, CALC_DXT_4b(64));             \
        gDPPipeSync(pkt);                                                                              \
        gDPSetTile(pkt, G_IM_FMT_CI, G_IM_SIZ_4b, 4, tmem, rtile, 0, G_TX_WRAP | G_TX_NOMIRROR, 5,     \
                   G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 6, G_TX_NOLOD);                              \
        gDPSetTileSize(pkt, rtile, uls, ult, (64 - 1) << 2, (32 - 1) << 2);                            \
    }

/* A 32x64 I8 texture, the same way. */
#define gDPLoadScrollBlock_I8(pkt, timg, tmem, rtile, uls, ult)                                       \
    {                                                                                                  \
        gDPSetTextureImage(pkt, G_IM_FMT_I, G_IM_SIZ_16b, 1, timg);                                    \
        gDPSetTile(pkt, G_IM_FMT_I, G_IM_SIZ_16b, 0, tmem, G_TX_LOADTILE, 0,                           \
                   G_TX_WRAP | G_TX_NOMIRROR, 6, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD); \
        gDPLoadSync(pkt);                                                                              \
        gDPLoadBlock(pkt, G_TX_LOADTILE, 0, 0, ((32 * 64 + 1) >> 1) - 1, CALC_DXT(32, 1));             \
        gDPPipeSync(pkt);                                                                              \
        gDPSetTile(pkt, G_IM_FMT_I, G_IM_SIZ_8b, 4, tmem, rtile, 0, G_TX_WRAP | G_TX_NOMIRROR, 6,      \
                   G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);                              \
        gDPSetTileSize(pkt, rtile, uls, ult, (32 - 1) << 2, (64 - 1) << 2);                            \
    }

extern u8 kawaKawaRiverTex[]; /* the river's texture */

/* "kawa"'s display list: two layers of one 64x32 CI4 texture, scrolled against each other. */
Gfx *kawaKawaDraw(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 128, 128, 128, 128);
    gDPLoadScrollBlock_4b(gfx++, kawaKawaRiverTex, 0, G_TX_RENDERTILE, scroll, scroll);
    gDPLoadScrollBlock_4b(gfx++, kawaKawaRiverTex, 0x100, 1, -scroll, scroll + 10);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* A river's work ("kawa" and the rest), at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct KawaW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ Frame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *pol;
    /* 0x38 */ RideFrame *ride; /* "derokawa1"'s */
    /* 0x3C */ s32 scroll; /* its display list's (read as a u16) */
} KawaW;

extern Gfx kawaKawaData0Dl[];
extern Gfx kawaKaikiDl[]; /* the kaiki's (kind 0x1BF's file set 0xAC): kawaKawaData0Dl's address in another file */
extern Gfx kawaKawaData1Dl[];
extern Gfx kawaKawaData2Dl[];

/* "kawa": a river, one of three shapes by its placement's data (the first a tenth larger), drawn
 * with kawaKawaDraw's scrolling layers. */
s32 kawaKawaProc(Obj *o, s16 msg, void *arg) {
    KawaW *w = (KawaW *) FX(o);
    s32 pad;
    s32 data;

    switch (msg) {
        case 2:
            o->name = "kawa";
            data = eneSetup(o, arg);
            switch (data) {
                case 0:
                default:
                    w->pol = eneNewPol(o, kawaKawaData0Dl);
                    break;
                case 1:
                    w->pol = eneNewPol(o, kawaKawaData1Dl);
                    break;
                case 2:
                    w->pol = eneNewPol(o, kawaKawaData2Dl);
                    break;
            }
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayers[6]);
            if (data == 0) {
                w->pol->scale[2] = 1.1f;
                w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            } else {
                w->pol->scale[2] = 1.0f;
                w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            }
            break;
        case 1:
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = kawaKawaDraw(gGfxDlHead, w->scroll);
            w->scroll--;
            break;
        case 0:
            break;
    }
    eneSetModel(o, &w->pol->f);
    return tskDefaultProc(o, msg, arg);
}

extern u8 kawaFallsTex1[]; /* the falls' texture */

/* "derokawa2"'s display list: no fog, two-cycle translucent, two layers of one 32x64 I8 texture
 * scrolled down, the second twice as fast. */
Gfx *kawaDerokawa2Draw(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gSPClearGeometryMode(gfx++, G_FOG);
    gDPSetRenderMode(gfx++, G_RM_PASS, G_RM_AA_ZB_XLU_SURF2);
    gDPSetCombineLERP(gfx++, 0, 0, 0, 1, TEXEL0, TEXEL1, ENVIRONMENT, TEXEL1, PRIMITIVE, ENVIRONMENT,
                      COMBINED_ALPHA, ENVIRONMENT, COMBINED, 0, SHADE, 0);
    gDPSetPrimColor(gfx++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor(gfx++, 128, 160, 255, 128);
    gDPLoadScrollBlock_I8(gfx++, kawaFallsTex1, 0, G_TX_RENDERTILE, 0, scroll);
    gDPLoadScrollBlock_I8(gfx++, kawaFallsTex1, 0x100, 1, 10, scroll * 2);
    gSPEndDisplayList(gfx++);
    return gfx;
}

extern u8 kawaNamiWavesTex[]; /* the waves' texture */

/* "nami"'s display list: as "kawa"'s, with the waves' texture. */
Gfx *kawaNamiDraw(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 128, 128, 128, 128);
    gDPLoadScrollBlock_4b(gfx++, kawaNamiWavesTex, 0, G_TX_RENDERTILE, scroll, scroll);
    gDPLoadScrollBlock_4b(gfx++, kawaNamiWavesTex, 0x100, 1, -scroll, scroll + 10);
    gSPEndDisplayList(gfx++);
    return gfx;
}

extern Gfx kawaNamiDl[];

/* "nami": waves, drawn with kawaNamiDraw's scrolling layers. */
s32 kawaNamiProc(Obj *o, s16 msg, void *arg) {
    KawaW *w = (KawaW *) FX(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "nami";
            eneSetup(o, arg);
            w->pol = eneNewPol(o, kawaNamiDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayers[6]);
            w->pol->scale[2] = 1.0f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            break;
        case 1:
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = kawaNamiDraw(gGfxDlHead, w->scroll);
            w->scroll--;
            eneSetModelOnCourse(o, &w->pol->f);
            break;
        case 0:
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, w->model);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 kawaKaikiSeaTex[]; /* the sea's texture */

/* "kaiki"'s display list: as "kawa"'s, with the sea's texture, more opaque. */
Gfx *kawaKaikiDraw(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 128, 128, 128, 192);
    gDPLoadScrollBlock_4b(gfx++, kawaKaikiSeaTex, 0, G_TX_RENDERTILE, scroll, scroll);
    gDPLoadScrollBlock_4b(gfx++, kawaKaikiSeaTex, 0x100, 1, -scroll, scroll + 10);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* "kaiki": the sea, drawn with kawaKaikiDraw's scrolling layers. */
s32 kawaKaikiProc(Obj *o, s16 msg, void *arg) {
    KawaW *w = (KawaW *) FX(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "kaiki";
            eneSetup(o, arg);
            w->pol = eneNewPol(o, kawaKaikiDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayers[6]);
            w->pol->scale[2] = 1.0f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            break;
        case 1:
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = kawaKaikiDraw(gGfxDlHead, w->scroll);
            w->scroll--;
            eneSetModelOnCourse(o, &w->pol->f);
            break;
        case 0:
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, w->model);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 kawaFallsTex0[]; /* the falls' texture */

/* "derokawa1" and "kirikawa2"'s display list: as "kawa"'s, with another texture. */
Gfx *kawaDerokawa1Draw(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 128, 128, 128, 128);
    gDPLoadScrollBlock_4b(gfx++, kawaFallsTex0, 0, G_TX_RENDERTILE, scroll, scroll);
    gDPLoadScrollBlock_4b(gfx++, kawaFallsTex0, 0x100, 1, -scroll, scroll + 10);
    gSPEndDisplayList(gfx++);
    return gfx;
}

extern Gfx kawaDerokawa1Dl[];
extern u8 kawaDerokawa1Ride[]; /* its ride's collision (RideData) */

/* "derokawa1": a muddy river with a ride on it, drawn with kawaDerokawa1Draw's scrolling
 * layers. */
s32 kawaDerokawa1Proc(Obj *o, s16 msg, void *arg) {
    KawaW *w = (KawaW *) FX(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "derokawa1";
            eneSetup(o, arg);
            w->pol = eneNewPol(o, kawaDerokawa1Dl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayers[6]);
            w->pol->scale[2] = 1.0f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            w->ride = rideCreate(gRideTree2, (s32) o, &w->pol->f,
                                 (RideData *) eneFileAddr(o, kawaDerokawa1Ride, 0));
            break;
        case 1:
            frmDestroy(&w->ride->f);
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = kawaDerokawa1Draw(gGfxDlHead, w->scroll);
            w->scroll--;
            eneSetModelOnCourse(o, &w->pol->f);
            break;
        case 0:
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, w->model);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern Gfx kawaDerokawa2Dl[];

/* "derokawa2": a muddy fall, a pol frame of its own at its place drawn after
 * kawaDerokawa2Draw's scrolling layers. */
s32 kawaDerokawa2Proc(Obj *o, s16 msg, void *arg) {
    KawaW *w = (KawaW *) FX(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "derokawa2";
            eneSetup(o, arg);
            w->pol = (PolFrame *) frmCreateNext(gFrmLayers[6], gFrmKindPol);
            if (w->pol == NULL) {
                return -1;
            }
            eneGetSegs(o, w->pol->segs);
            w->pol->dl2 = kawaDerokawa2Dl;
            w->pol->pos[0] = w->x;
            w->pol->pos[1] = w->y;
            w->pol->pos[2] = w->z;
            w->pol->scale[2] = 1.0f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            break;
        case 1:
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = kawaDerokawa2Draw(gGfxDlHead, w->scroll);
            w->scroll -= 4;
            eneSetModelOnCourse(o, &w->pol->f);
            break;
        case 0:
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, w->model);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 kawaClearRiverTex[]; /* the clear river's texture */

/* "kirikawa1" and "kirikawa5"'s display list: as "kawa"'s, with another texture. */
Gfx *kawaKirikawa1Draw(Gfx *gfx, u16 scroll) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 128, 128, 128, 128);
    gDPLoadScrollBlock_4b(gfx++, kawaClearRiverTex, 0, G_TX_RENDERTILE, scroll, scroll);
    gDPLoadScrollBlock_4b(gfx++, kawaClearRiverTex, 0x100, 1, -scroll, scroll + 10);
    gSPEndDisplayList(gfx++);
    return gfx;
}

extern Gfx kawaKirikawa1Dl[];

/* "kirikawa1": a river, drawn with kawaKirikawa1Draw's scrolling layers. */
s32 kawaKirikawa1Proc(Obj *o, s16 msg, void *arg) {
    KawaW *w = (KawaW *) FX(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "kirikawa1";
            eneSetup(o, arg);
            w->pol = eneNewPol(o, kawaKirikawa1Dl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayers[6]);
            w->pol->scale[2] = 1.0f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            break;
        case 1:
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = kawaKirikawa1Draw(gGfxDlHead, w->scroll);
            w->scroll--;
            eneSetModelOnCourse(o, &w->pol->f);
            break;
        case 0:
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, w->model);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern Gfx kawaKirikawa2Dl[];

/* "kirikawa2": a river, drawn with kawaDerokawa1Draw's scrolling layers. */
s32 kawaKirikawa2Proc(Obj *o, s16 msg, void *arg) {
    KawaW *w = (KawaW *) FX(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "kirikawa2";
            eneSetup(o, arg);
            w->pol = eneNewPol(o, kawaKirikawa2Dl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayers[6]);
            w->pol->scale[2] = 1.0f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            break;
        case 1:
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = kawaDerokawa1Draw(gGfxDlHead, w->scroll);
            w->scroll--;
            eneSetModelOnCourse(o, &w->pol->f);
            break;
        case 0:
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, w->model);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern Gfx kawaKirikawa2SmallDl[];

/* The second "kirikawa2" kind: another river at a fifth size, drawn with
 * kawaDerokawa1Draw's scrolling layers. */
s32 kawaKirikawa2SmallProc(Obj *o, s16 msg, void *arg) {
    KawaW *w = (KawaW *) FX(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "kirikawa2";
            eneSetup(o, arg);
            w->pol = eneNewPol(o, kawaKirikawa2SmallDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayers[6]);
            w->pol->scale[2] = 0.2f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            break;
        case 1:
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = kawaDerokawa1Draw(gGfxDlHead, w->scroll);
            w->scroll--;
            eneSetModelOnCourse(o, &w->pol->f);
            break;
        case 0:
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, w->model);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern Gfx kawaKirikawa5Dl[];

/* "kirikawa5": a river, drawn with kawaKirikawa1Draw's scrolling layers. */
s32 kawaKirikawa5Proc(Obj *o, s16 msg, void *arg) {
    KawaW *w = (KawaW *) FX(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "kirikawa5";
            eneSetup(o, arg);
            w->pol = eneNewPol(o, kawaKirikawa5Dl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, gFrmLayers[6]);
            w->pol->scale[2] = 1.0f;
            w->pol->scale[0] = w->pol->scale[1] = w->pol->scale[2];
            break;
        case 1:
            if (w->pol != NULL) {
                frmDestroy(&w->pol->f);
            }
            break;
        case 3:
            w->pol->dl = gGfxDlHead;
            gGfxDlHead = kawaKirikawa1Draw(gGfxDlHead, w->scroll);
            w->scroll--;
            eneSetModelOnCourse(o, &w->pol->f);
            break;
        case 0:
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, w->model);
            }
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
