#include "tlb.h"

/* File 117: two officials ("yakunin", townspeople of person.c) and the Karasu Tengu's two
 * events ("karaE1", "karaE2": deroKarasuEve1_proc), demo talkers who give the player a pass
 * (kind 0x1C3) if he takes on their errand (flag 0x93). */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* A Karasu Tengu event's work, at its task + 0x20: a demo talker's (PersonObj) up to 0x44. */
typedef struct KaraW {
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
    /* 0x34 */ PersonData *data;
    /* 0x38 */ ACollFrame *box; /* its talk box */
    /* 0x3C */ u8 flags3C;
    /* 0x3D */ u8 count;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ Obj *talker; /* the player talking to it */
    /* 0x44 */ MPolFrame *shadow;
    /* 0x48 */ u8 pad48[4];
    /* 0x4C */ Obj *pass; /* the pass (kind 0x1C3), made between it and the player */
    /* 0x50 */ s32 timer; /* ticks in the state */
    /* 0x54 */ u8 flags; /* 1: the sky's tunes change 300 ticks after the pass */
    /* 0x55 */ u8 tuneTimer;
} KaraW;

#define KARA(obj) ((KaraW *) FX(obj))

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

void deroKarasuTegataGive(Obj *o, Obj *player);

extern u8 deroKarasuYakuninAnim0[];
extern u8 deroKarasuYakunin2Anim[];
extern u8 deroKarasuYakuninAnim1[];
extern u8 deroKarasuEve2Anim[];
u32 deroKarasuYakuninAnims[] = { (u32) deroKarasuYakuninAnim0, (u32) deroKarasuYakuninAnim1 }; /* the first official's animations */
u32 deroKarasuYakunin2Anims[] = { (u32) deroKarasuYakunin2Anim, (u32) deroKarasuYakunin2Anim }; /* the second's */
u32 deroKarasuEve1Model[] = { (u32) deroKarasuYakunin2Anim }; /* karaE1's model */
u32 deroKarasuEve2Model[] = { (u32) deroKarasuEve2Anim, 0x00000000, 0x00000000 }; /* karaE2's model */

/* 0x00-0x2B: the four small functions tlb_0114 also has (0x134-0x15F there; see there). */
void deroKarasuTegataDummy(void) {
}

/* The pass goes to the player (state 3). */
void deroKarasuTegataGive(Obj *o, Obj *player) {
    PassW *w = (PassW *) FX(o);

    w->state = 3;
    w->player = player;
    w->timer = 0;
}

void deroKarasuTegataSdSet(void) {
    DPRINTF("&&&& eventTegataSdSet  pl D がないn");
}

void deroKarasuTegataSdClear(void) {
    DPRINTF("&&&& SD_G_CLEAR1n");
    DPRINTF("&&&& SD_E_CLEAR1n");
    DPRINTF("&&&& SD_S_CLEAR1n");
    DPRINTF("&&&& SD_Y_CLEAR1n");
}

/* "yakunin": an official, a townsperson. */
s32 deroKarasuYakuninProc(Obj *o, s16 msg, void *arg) {
    switch (msg) {
        case 2:
            o->name = "yakunin";
            if (personInitMainTable(o, deroKarasuYakuninAnims, arg) == -1) {
                return -1;
            }
            break;
        case 8:
            return personTalk(o, arg);
        case 1:
            personDelete(o);
            break;
        case 0:
            personAct(o);
            break;
        case 3: /* nothing: four cases over 0-8 give a compare chain, five the game's jump table
                 * (its words 3-7 all go to tskDefaultProc) */
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "yakunin": the other official, an "anime" model at 0.65 scale, lit. */
s32 deroKarasuYakunin2Proc(Obj *o, s16 msg, void *arg) {
    PersonObj *p = PERSON(o);
    s32 pad[2];
    u16 r;

    switch (msg) {
        case 2:
            o->name = "yakunin";
            personFlagOn(o, 0x40);
            if (personInitMainTable(o, deroKarasuYakunin2Anims, arg) == -1) {
                return -1;
            }
            ((AnimeFrame *) p->model)->scale[0] = ((AnimeFrame *) p->model)->scale[1] =
                ((AnimeFrame *) p->model)->scale[2] = 0.65f;
            /* The radius read into a local (inline, it takes a t-register where the game has a3). */
            r = p->box->radius;
            p->box->radius = r * 0.65f * 0.1 * 3.0;
            p->box->ofs[1] = 0x13;
            personFlagOn(o, 0x20);
            break;
        case 8:
            return personTalk(o, arg);
        case 3:
            ((AnimeFrame *) p->model)->dl = eneDlEnvColorLit(o, 0, 0, 0, 0, 1);
            break;
        case 1:
            personDelete(o);
            break;
        case 0:
            personAct(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "karaE1": the Karasu Tengu's first event. Talked to, it turns to the player and talks
 * (scenario 0x2D6); if the player takes on its errand (0x93) it leaves a pass between them that
 * sparks and goes to the player (scenario 0x20, then 0x2D6 again).
 *
 * Matched 2026-10-12: the radius in a u16 local r (in pad1's word) and an empty
 * "case 4: break;". Diagnostic first (o taken out of case 3's or case 1's call, so one use fewer
 * at the case heads): o reloads everywhere and, with r, real 1. So o's a0 piece is marginal; the
 * empty case (one more edge into the tail, the jump table unchanged) tips it. A K&R definition
 * (a probe): no change. w set in each case instead of at its declaration: 176 (o
 * takes s1).
 * Was NON_MATCHING: 92 differing lines, mix distance 8; the frame and every stack slot are the
 * game's. The one difference: the game reads o from its home at every use (case 2's name store
 * through t8, each case's first call's a0, the tail's own "lw a0"). Here IDO splits o's live
 * range and keeps it in a0 from the entry to each case's first call and into the tail ("sw a0"
 * at each case head, the tail's reload in each predecessor's delay slot), which also leaves t8
 * free so case 2's temps come one register early. tlb_0114's g_shonin, the same shape, compiles
 * with the game's reloads. Measured, none changing it: the prints as PRINTF (if (0)), the step's
 * locals at function scope, "default: break;", eight FAKE splits before the tail, "(&o);".
 * "Obj *volatile o" gives the reloads but builds w at the entry (398). The radius read into a
 * u16 local first (deroKarasuYakunin2Proc's lever, which matched it there) gives the game's a1 for it
 * here too, but with o in a0 the t-registers fall one short: 314 (a hill: with the reload fixed
 * it should be the game's form). karaE2 (deroKarasuEve2_proc) has the same one difference.
 * 2026-10-11: o's address taken ("if (&o) {}", tlb_0131 14C8's lever): w built at the
 * entry as with volatile, frame 8 short; with two pad words more 423.
 * 2026-10-12 (second try): msg declared u16 with a plain "switch (msg)" (rule 27's homed narrow
 * parameter, no FAKE): 41 real (msg masked into a1 and spilled to 0x38; o still in a0).
 * tlb_0109's shopTenguTurn_proc (the same case 2 head, matched) keeps o in s2: here the game
 * gives o no register at all and s0 to w.
 * 2026-10-12 (third try), probe (e0 is this C, the same code): with state 0's
 * body cut to the two hooks, o gets s1 for the whole function; with case 2 cut too, o reloads.
 * So the a0 pieces come from o losing the saved register to the size of case 0, then being
 * split; the game gives o neither. Diagnostic only, no candidate yet.
 * 2026-10-12 (fourth try): the tail's call as tskDefaultProc(*(Obj **) &o, ...) (tlb_0071 bloodw's
 * lever, which matched there): 224, w built at the entry and the frame 8 short (as "if (&o)").
 * w as a macro, KARA(o) at every use, with that &o: 295 (length +40, o+0x20 no longer CSE'd);
 * without the &o: 214 (o takes s0). A K&R definition: the check tool does not find the function
 * (not measured). The splits sweep (224 variants, 1 and 8 boundaries): none below 92. */
s32 deroKarasuEve1_proc(Obj *o, s16 msg, void *arg) {
    KaraW *w = KARA(o);
    u16 r;
    u32 data;
    s32 pad2[2];

    switch (msg) {
        case 2:
            o->name = "karaE1";
            data = eneSetup(o, arg);
            w->model = eneAnimeModelMake(o, deroKarasuEve1Model[0], NULL, 0);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            demoTalkInit(o, data);
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.65f;
            r = w->box->radius;
            w->box->radius = r * 0.65f * 0.1 * 3.0;
            w->box->ofs[1] = 0x13;
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("deroKarasuEve1_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            DPRINTF("$$$$   deroKarasuEve1_proc: カラテン イベント１セット\n");
            break;
        case 8:
            return demoTalk(o, arg);
        case 3:
            w->model->dl = eneDlEnvColorLit(o, 0, 0, 0, 0, 1);
            break;
        case 4: /* nothing (message 4, the position query of other procs). Its jump-table word
                 * is the default's; it keeps o out of a0 at the case heads. */
            break;
        case 1:
            demoTalkerFree(o);
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            break;
        case 0: {
            Vec3f pos;
            s32 i;
            Vec3f at;
            s32 pad3;
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
                        personTalkCamAt(o, w->talker, 80);
                        w->state = 1;
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
                    scRunStart(0x2D6, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    w->state = 3;
                    break;
                case 3:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x93)) {
                            w->state = 4;
                            swWorkReset(0x93);
                        } else {
                            w->state = 7;
                        }
                    }
                    break;
                case 4:
                    if (w->talker == NULL) {
                        return -1;
                    }
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
                        deroKarasuTegataGive(w->pass, w->talker);
                        w->flags |= 1;
                    }
                    if (w->timer == 260 / gRetracesPerFrame) {
                        scRunStart(0x20, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (!scIsRunning()) {
                        scRunStart(0x2D6, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        w->state = 7;
                    }
                    break;
                case 7:
                    if (!scIsRunning()) {
                        demoFinish(o);
                        camSunSetClockStop(0);
                        camAiSetHold(0);
                        demoPlReleaseAll();
                        w->state = 0;
                    }
                    break;
            }
            if (w->flags & 1) {
                if (300 / gRetracesPerFrame == w->tuneTimer++) {
                    w->flags &= ~1;
                    camSunClearTunes();
                    camSunSetTunes4(0x30, 0x2D, 0x2E, 0x2F);
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
    }
    return tskDefaultProc(o, msg, arg);
}

/* "karaE2": the Karasu Tengu's second event, as the first with its own scenario (0x2D7,
 * SC_A6_M_MIN3_TENGU), the close-in talk camera, and facing 210 degrees while it waits.
 *
 * Matched 2026-10-12 with karaE1's two changes (r, the empty case 4); it was parked for karaE1's
 * one difference, o kept in a0 where the game reloads it from its home. */
s32 deroKarasuEve2_proc(Obj *o, s16 msg, void *arg) {
    KaraW *w = KARA(o);
    u16 r;
    u32 data;
    s32 pad2[2];

    switch (msg) {
        case 2:
            o->name = "karaE2";
            data = eneSetup(o, arg);
            w->model = eneAnimeModelMake(o, deroKarasuEve2Model[0], NULL, 0);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, &w->model->f, 100, 0, 100, 0, 0);
            demoTalkInit(o, data);
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2] = 0.65f;
            r = w->box->radius;
            w->box->radius = r * 0.65f * 0.1 * 3.0;
            w->box->ofs[1] = 0x13;
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("deroKarasuEve1_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            DPRINTF("$$$$   deroKarasuEve1_proc: カラテン イベント１セット\n");
            break;
        case 8:
            return demoTalk(o, arg);
        case 3:
            w->model->dl = eneDlEnvColorLit(o, 0, 0, 0, 0, 1);
            break;
        case 4: /* nothing, as in karaE1 */
            break;
        case 1:
            demoTalkerFree(o);
            if (w->shadow != NULL) {
                frmDestroy((Frame *) w->shadow);
            }
            break;
        case 0: {
            Vec3f at;
            s32 pad3;
            f32 x;
            f32 y;
            f32 z;
            Vec3f spark;

            eneStepHookPre(o);
            switch (w->state) {
                case 0:
                    if (demoTalkEndCheck(o)) {
                        camSunSetClockStop(1);
                        demoCamZoom_CL(w->talker);
                        w->state = 1;
                    }
                    eneTurnToAngle(o, 210.0f, 4.0f);
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
                    DPRINTF("talkNo [SC_A6_M_MIN3_TENGU]\n");
                    scRunStart(0x2D7, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    w->state = 3;
                    break;
                case 3:
                    if (!scIsRunning()) {
                        if (swWorkTest(0x93)) {
                            w->state = 4;
                            swWorkReset(0x93);
                        } else {
                            w->state = 7;
                        }
                    }
                    break;
                case 4:
                    if (w->talker == NULL) {
                        return -1;
                    }
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
                        deroKarasuTegataGive(w->pass, w->talker);
                        w->flags |= 1;
                    }
                    if (w->timer == 260 / gRetracesPerFrame) {
                        scRunStart(0x20, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        w->state = 6;
                    }
                    break;
                case 6:
                    if (!scIsRunning()) {
                        scRunStart(0x2D7, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                        w->state = 7;
                    }
                    break;
                case 7:
                    if (!scIsRunning()) {
                        DPRINTF("######kara: 会話終了\n");
                        personTalkCamOff();
                        demoPlReleaseAll();
                        demoFinish(o);
                        w->state = 0;
                    }
                    break;
            }
            if (w->flags & 1) {
                if (300 / gRetracesPerFrame == w->tuneTimer++) {
                    w->flags &= ~1;
                    camSunClearTunes();
                    camSunSetTunes4(0x30, 0x2D, 0x2E, 0x2F);
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
    }
    return tskDefaultProc(o, msg, arg);
}
