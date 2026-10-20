#include "tlb.h"

/* File 114: the merchant ("shounin"), a townsperson (person.c), and his ghost ("g_shonin"),
 * a demo talker who appears at night with a will-o'-the-wisp ("gOnibi", three flames) until
 * he has given the player his pass (tegata, flag 0xB5); then he rests in peace. */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* The ghost merchant's work, at its task + 0x20: a demo talker's (PersonObj) up to 0x44. */
typedef struct GShoninW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 pad09[3];
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
    /* 0x44 */ u8 flags; /* 1: the sky's tunes change 300 ticks after the pass */
    /* 0x48 */ s32 alpha; /* its env colour's alpha (0xE0 when shown) */
    /* 0x4C */ s32 timer; /* ticks in the state */
    /* 0x50 */ u8 wave; /* its bobbing's phase, 0-39 */
    /* 0x54 */ Obj *pass; /* the pass (kind 0x1C3), made between it and the player */
    /* 0x58 */ f32 amp; /* the bobbing's height */
    /* 0x5C */ Obj *onibi; /* its will-o'-the-wisp (kind 0x1C6) */
    /* 0x60 */ u8 tuneTimer;
} GShoninW;

#define GSHONIN(obj) ((GShoninW *) FX(obj))

/* The will-o'-the-wisp's work ("gOnibi"), at its task + 0x20. */
typedef struct GOnibiW {
    /* 0x00 */ u8 unused00;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 pad09[3];
    /* 0x0C */ PolFrame *model; /* where it is (no list of its own) */
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *flames[3];
    /* 0x40 */ Anim anim[3]; /* each flame's texture */
    /* 0x70 */ s32 wave; /* the bobbing's phase, 0-39 */
    /* 0x74 */ s32 alpha;
    /* 0x78 */ u8 state; /* 0 brightening, 1 dimming, 2 going out */
} GOnibiW;

#define GONIBI(obj) ((GOnibiW *) FX(obj))

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

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

void shouninTegataGive(Obj *o, Obj *player);
void shouninOnibiOut(Obj *o);

extern u8 shouninAnim0[];
extern u8 shouninOnibiTex0[];
extern u8 shouninOnibiTex2[];
extern u8 shouninOnibiTex5[];
u32 shouninAnims[] = { (u32) shouninAnim0, 0x0300056C }; /* the merchant's animations: { 0x030001D8, 0x0300056C } */
AnimFrame shouninOnibiTexAnim[] = {
    { (u32) shouninOnibiTex0, 2 },
    { 0x03000D80, 2 },
    { (u32) shouninOnibiTex2, 2 },
    { 0x03001D80, 2 },
    { 0x03002580, 2 },
    { (u32) shouninOnibiTex5, 2 },
    { 0x03003580, 2 },
    { 0x03003D80, 2 },
    { 0x03004580, 2 },
    { 0x03004D80, 2 },
    { 0x03005580, 2 },
    { 0x03005D80, 2 },
    { 0x03006580, 2 },
    { 0x03006D80, 2 },
    { 0x03007580, 2 },
    { 0x03007D80, 2 },
    { 0, 0 },
}; /* the flames' texture animation */
extern Gfx shouninOnibiFlameDl[];

/* "shounin": the merchant, a townsperson. */
s32 shouninProc(Obj *o, s16 msg, void *arg) {
    switch (msg) {
        case 2:
            o->name = "shounin";
            if (personInitMainTable(o, shouninAnims, arg) == -1) {
                return -1;
            }
            break;
        case 3:
            ((AnimeFrame *) PERSON(o)->model)->dl = gGfxDlHead;
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, 255);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 8:
            return personTalk(o, arg);
        case 1:
            personDelete(o);
            break;
        case 0:
            personAct(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* 0x134-0x15F: a block of four small functions that file 117 also has at its start (an empty
 * one, the pass setter, and two emptied ones whose prints remain: the pass's "eventTegataSdSet"
 * and the four characters' SD_x_CLEAR1 flags). splat put the empty ones in their neighbours'
 * .s files; which empty one held which print is not known (no code is left, only the strings'
 * order). The prints' "n" is the game's (a typo for "\n"). */
void shouninTegataDummy(void) {
}

/* The pass goes to the player (state 3). */
void shouninTegataGive(Obj *o, Obj *player) {
    PassW *w = (PassW *) FX(o);

    w->state = 3;
    w->player = player;
    w->timer = 0;
}

void shouninTegataSdSet(void) {
    DPRINTF("&&&& eventTegataSdSet  pl D がないn");
}

void shouninTegataSdClear(void) {
    DPRINTF("&&&& SD_G_CLEAR1n");
    DPRINTF("&&&& SD_E_CLEAR1n");
    DPRINTF("&&&& SD_S_CLEAR1n");
    DPRINTF("&&&& SD_Y_CLEAR1n");
}

/* "g_shonin": the ghost merchant. Made only while the pass is not given and not during the
 * courier mission (flag 0x29 without 0xB6); shown at night (before 4:00 or from 21:00), hidden by
 * day. Talked to, he turns to the player, talks (scenario 0x29B), and if the player takes on his
 * errand (0x93) leaves a pass between them that sparks, goes to the player (scenarios 0x20,
 * 0x29C), fades out and his wisp goes out; then he rests (0xB5) or waits for night. */
s32 shouninGhostProc(Obj *o, s16 msg, void *arg) {
    GShoninW *w = GSHONIN(o);
    u32 data;
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "g_shonin";
            if (swWorkTest(0xB5)) {
                DPRINTF(">>>  商人の幽霊すでに手形をもらっています  終了\n");
                return -1;
            }
            if (swWorkTest(0x29) && !swWorkTest(0xB6)) {
                DPRINTF("****  商人の幽霊飛脚ミッション中  終了\n");
                return -1;
            }
            data = eneSetup(o, arg);
            w->model = eneAnimeModelAlpha(o, shouninAnims[0], 0);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 0xA0, 0, 0xA0, 0, 0);
            if (w->box == NULL) {
                return -1;
            }
            demoTalkInit(o, data);
            if (gGameWork.time < 0x150000 && gGameWork.time > 0x40000) {
                DPRINTF(">>>>>>  幽霊商人発生しません\n");
                w->alpha = 0;
                w->model->f.flags &= ~2;
                break;
            }
            DPRINTF(">>>>>>  幽霊商人発生する\n");
            w->onibi = eneMakeEx(NULL, ENE_SHOUNIN_ONIBI, 0, w->model->pos[0], w->model->pos[1], w->model->pos[2], 0, NULL);
            w->alpha = 0xE0;
            break;
        case 3:
            w->model->dl = gGfxDlHead;
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            demoTalkerFree(o);
            break;
        case 0: {
            /* Block-scoped: the draw case's two gfx macros' locals take the slots above these. */
            Vec3f pos;
            s32 i;
            Vec3f at;
            f32 x;
            f32 y;
            f32 z;
            Vec3f spark;

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
                        camSunSetClockStop(1);
                        gGameWork.partake = 1;
                        personTalkCamAt(o, w->talker, 80);
                        w->timer = 0;
                        w->state = 1;
                    }
                    if (gGameWork.time < 0x150000 && gGameWork.time > 0x40000) {
                        DPRINTF(">>>    商人消える\n");
                        w->timer = 0;
                        w->state = 8;
                    }
                    break;
                case 1:
                    if (w->talker != NULL) {
                        tskSendMsg(w->talker, 4, &at);
                        if (eneTurnToPoint(o, at.x, at.z, 10.0f) == 1) {
                            w->state = 2;
                        }
                    }
                    break;
                case 2:
                    scRunStart(0x29B, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    w->state = 3;
                    break;
                case 3:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x93)) {
                            swWorkReset(0x93);
                            w->state = 4;
                        } else {
                            camSunSetClockStop(0);
                            camAiSetHold(0);
                            gGameWork.partake = 0;
                            demoFinish(o);
                            w->state = 0;
                        }
                    }
                    break;
                case 4:
                    plGetModelPos(w->talker, &x, &y, &z);
                    w->pass = eneMakeEx(NULL, ENE_DEMO_TEGATA, 0, (w->model->pos[0] + x) / 2, w->model->pos[1],
                                        (w->model->pos[2] + z) / 2, 0, NULL);
                    w->timer = 0;
                    w->state = 5;
                    break;
                case 5:
                    if (w->timer == 200 / gRetracesPerFrame) {
                        demoGetEneModelPos(w->pass, &spark);
                        ptclSpawnSparksFall(spark.x, spark.y, spark.z);
                        eneMakeEx(NULL, ENE_DEMO_KIRAKIRA, 0, spark.x, spark.y, spark.z, 0, NULL);
                        shouninTegataGive(w->pass, w->talker);
                        w->flags |= 1;
                    }
                    if (w->timer == 260 / gRetracesPerFrame) {
                        scRunStart(0x20, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (!scIsRunning()) {
                        scRunStart(0x29C, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        w->state = 7;
                    }
                    break;
                case 7:
                    if (!scIsRunning()) {
                        camSunSetClockStop(0);
                        camAiSetHold(0);
                        demoFinish(o);
                        w->state = 8;
                    }
                    break;
                case 8:
                    if (w->alpha-- < 1) {
                        w->alpha = 0;
                        if (w->onibi != NULL) {
                            shouninOnibiOut(w->onibi);
                        }
                        if (swWorkTest(0xB5)) {
                            DPRINTF("成仏\n");
                            gGameWork.partake = 0;
                            camSunSetClockStop(0);
                            return -1;
                        }
                        DPRINTF("夜待ち\n");
                        w->model->f.flags &= ~2;
                        w->state = 9;
                    }
                    break;
                case 9:
                    if (gGameWork.time > 0x150000) {
                        DPRINTF(">>>    出現\n");
                        w->wave = 0;
                        w->y = 0.0f;
                        w->model->f.flags |= 2;
                        w->state = 10;
                    }
                    break;
                case 10:
                    if (w->alpha++ >= 0xE0) {
                        w->alpha = 0xE0;
                        w->onibi = eneMakeEx(NULL, ENE_SHOUNIN_ONIBI, 0, w->model->pos[0], w->model->pos[1],
                                             w->model->pos[2], 0, NULL);
                        w->state = 0;
                    }
                    break;
            }
            if (w->flags & 1) {
                if (300 / gRetracesPerFrame == w->tuneTimer++) {
                    w->flags &= ~1;
                    camSunClearTunes();
                    camSunSetTunes4(0xBE, 0xBB, 0xBC, 0xBD);
                }
            }
            if (w->wave == 40) {
                w->wave = 0;
            }
            if (w->wave == 0) {
                w->amp = 0.2f;
            }
            w->vel[1] = mathCos(w->wave * 9 * 3.1415927f / 180.0f) * w->amp;
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.5f);
            eneStepHookPost(o);
            w->timer++;
            w->wave++;
            break;
        }
    }
    return tskDefaultProc(o, msg, arg);
}

/* "gOnibi": the ghost merchant's will-o'-the-wisp, three blue flames (pol frames of one
 * animated texture, file 0x125) bobbing beside him; they brighten and dim in turn and go out
 * when he leaves. */
s32 shouninOnibiProc(Obj *o, s16 msg, void *arg) {
    GOnibiW *w = GONIBI(o);
    s32 i;
    s32 j;
    s32 k;
    s32 m;

    switch (msg) {
        case 2:
            o->name = "gOnibi";
            eneSetup(o, arg);
            w->model = eneNewPol(o, NULL);
            if (w->model == NULL) {
                DPRINTF(">>>  鬼火本体でまへん\n");
                return -1;
            }
            for (j = 0; j < 3; j++) {
                w->flames[j] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
                if (w->flames[j] == NULL) {
                    DPRINTF(">>>  鬼火[%i]でない\n", j);
                    return -1;
                }
                w->flames[j]->dl2 = shouninOnibiFlameDl;
                w->flames[j]->flags |= 0x27;
                floadGetSetRam((void **) w->flames[j]->segs, 0x125);
                w->flames[j]->scale[0] = w->flames[j]->scale[1] = w->flames[j]->scale[2] = 0.1f;
                w->flames[j]->pos[0] = w->model->pos[0] - 5.0f + j * 7;
                w->flames[j]->pos[1] = w->model->pos[1] + 5.0f + j * 3;
                w->flames[j]->pos[2] = w->model->pos[2] - 5.0f + j * 5;
                fadeAnimStart(&w->anim[j], shouninOnibiTexAnim, 0, &w->flames[j]->segs[3],
                              floadGetSetFile(0x125, 0));
            }
            DPRINTF(">>>  鬼火セットＯＫ\n");
            w->alpha = 0xA0;
            w->wave = 0;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            for (i = 0; i < 3; i++) {
                if (w->flames[i] != NULL) {
                    frmDestroy(&w->flames[i]->f);
                }
            }
            break;
        case 3:
            for (k = 0; k < 3; k++) {
                w->flames[k]->dl = eneDlEnvPrimColor(o, 0x80, 0x80, 0xFF, 0, 0, 0xE0, w->alpha);
            }
            break;
        case 0:
            for (m = 0; m < 3; m++) {
                fadeAnimStep(&w->anim[m]);
            }
            switch (w->state) {
                case 0:
                    w->alpha += 5;
                    if (w->alpha >= 0x100) {
                        w->alpha = 0xFF;
                        w->state++;
                    }
                    break;
                case 1:
                    w->alpha -= 5;
                    if (w->alpha <= 0x20) {
                        w->state = 0;
                    }
                    break;
                case 2:
                    w->alpha -= 5;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        return -1;
                    }
                    break;
            }
            if (w->wave == 40) {
                w->wave = 0;
            }
            for (m = 0; m < 3; m++) {
                w->flames[m]->pos[1] += 0.4 * mathCos((w->wave + m * 15) * 9 * 3.1415927f / 180.0f);
            }
            w->wave++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* The will-o'-the-wisp goes out (state 2). */
void shouninOnibiOut(Obj *o) {
    GONIBI(o)->state = 2;
}
