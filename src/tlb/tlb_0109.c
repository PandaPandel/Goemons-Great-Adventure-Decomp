#include "tlb.h"

/* The shop clerks and the town's shop customers: Cha-ya's old woman ("shopChaBaba"), the
 * Ganso and Honke shops' girls and boy, the Bannya master and his criminal, the inn, the mat
 * and take and ume shop people, the sleepy customers, Omitsu of the Mitsuya. Each is a task
 * kind with a switch(msg) proc; the ones that talk keep a demo talker's work (DEMO_CLERK_W),
 * walk a path of commands (as person.c's personActStepRun) and run their scenario. */

/* The game's debug print, compiled to nothing but its arguments (demo.c's form). */
#define DPRINTF

/* A talking clerk's work ("struct DEMO_CLERK_W"), at its task + 0x20: a demo talker's
 * (PersonObj) up to 0x44. */
typedef struct DemoClerkW {
    /* 0x00 */ u8 state; /* 0 walking, 1 talked to, 2 turning, 3 talking */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PersonData *data;
    /* 0x38 */ ACollFrame *box;
    /* 0x3C */ u8 flags;
    /* 0x3D */ u8 count;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ Obj *talker;
    /* 0x44 */ MPolFrame *shadow;
    /* 0x48 */ Anim anim; /* its face (segment 3 of its model) */
    /* 0x58 */ u16 timer;
    /* 0x5A */ u16 step; /* frames into the command */
    /* 0x5C */ u16 *path; /* the command it is at */
} DemoClerkW;

#define CLERK(obj) ((DemoClerkW *) FX(obj))

void shopAnimReset(Anim *a);

/* The talk is over (demo.c's demoFinish): the talker is sent message 8, the talk box is on
 * again. */
void shopClerkFinish(Obj *o) {
    DemoClerkW *p = CLERK(o);
    s32 pad;
    struct {
        Obj *o;
        s32 one;
    } arg;

    arg.o = o;
    arg.one = 1;
    if (p->talker != NULL) {
        tskSendMsg(p->talker, 8, &arg);
    } else {
        DPRINTF("Please set playerD at damage frame. Please use taskSet\n");
    }
    p->flags &= ~2;
    p->flags &= ~1;
    p->box->flags |= 4;
    eneSetForward(o, 0);
}

/* Its path, from its table entry. */
void shopClerkPathInit(Obj *o, PersonData *d) {
    CLERK(o)->path = (u16 *) stageAddrToRam(d->path);
}

/* Runs a path command (personActStepRun's, without the animations); the next one.
 * 0 x z speed: walk there; 1 n: go to command word n; 2 deg: stop and turn to deg;
 * 4 n: stand for n frames; 5 se: a sound at it; 6: stop. */
u16 *shopClerkActStep(Obj *o, u16 *base, u16 *path) {
    DemoClerkW *p = CLERK(o);
    s32 pad[2];

    switch (path[0]) {
        case 0: {
            f32 x = (s16) path[1];
            f32 z = (s16) path[2];
            f32 speed = (s16) path[3];

            if (eneMoveToPos(o, x, z, speed) == 1) {
                path += 4;
            }
            eneTurnToPoint(o, x, z, 13.0f);
            break;
        }
        case 1:
            path = base + (s16) path[1];
            break;
        case 2: {
            f32 deg = (s16) path[1];

            p->vel[0] = 0.0f;
            p->vel[2] = 0.0f;
            if (eneTurnToAngle(o, deg, 13.0f) == 1) {
                path += 2;
            }
            break;
        }
        case 3:
            DPRINTF("超人に行動パターンはいってません\n");
            break;
        case 4: {
            s16 n = path[1];

            if (p->step == 0) {
                p->vel[0] = 0.0f;
                p->vel[2] = 0.0f;
            }
            if (n < p->step++) {
                p->step = 0;
                path += 2;
            }
            break;
        }
        case 5: {
            s16 se = path[1];

            soundLocationCam(se, p->x, p->y, p->z);
            path += 2;
            break;
        }
        case 6:
            path += 1;
            p->vel[2] = 0.0f;
            p->vel[1] = 0.0f;
            p->vel[0] = 0.0f;
            break;
        default:
            DPRINTF("actStep がへんです");
            break;
    }
    return path;
}

/* The talk starts: the players stand, the camera closes in on the talker, pausing is off. */
void shopClerkTalkStart(Obj *o) {
    demoPlStandAll();
    demoCamZoom_CL(CLERK(o)->talker);
    eneSetForward(o, 1);
    gGameWork.scenarioRunning = 2;
}

/* The talk ends: the players go, the camera back, pausing on. */
void shopClerkTalkEnd(Obj *o) {
    (*shopClerkFinish)(o);
    demoPlReleaseAll();
    personTalkCamOff();
    camAiSetHold(0);
    gGameWork.scenarioRunning = 0;
}

/* An animation back to its first frame. */
void shopAnimReset(Anim *a) {
    a->index = 0;
    a->tick = 0;
    *a->out = fadeSegToRam(a->frames[0].addr, a->file);
}

/* The procs call these five through their addresses (lui/addiu + jalr), shopAnimReset with a
 * jal. */
#define shopClerkFinish (*shopClerkFinish)
#define shopClerkPathInit (*shopClerkPathInit)
#define shopClerkActStep (*shopClerkActStep)
#define shopClerkTalkStart (*shopClerkTalkStart)
#define shopClerkTalkEnd (*shopClerkTalkEnd)

/* acollAtkAddXYZ returns its box (main declares it void). */
typedef ACollFrame *(*AtkAddXYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 ox, s16 oy,
                                     s16 oz, s32 a);

extern u8 shopAnimOmituTex0[];
extern u8 shopAnimChaBabaTex0[];
extern u8 shopAnimChaBabaTex1[];
extern u8 shopAnimGirlTex0[];
AnimFrame shopAnimChaBaba[] = {
    { (u32) shopAnimChaBabaTex0, 15 },
    { (u32) shopAnimChaBabaTex1, 15 },
    { 0x00000000, 0 },
};
extern u8 shopChaBabaAnim[];

/* "cha_baba": the tea house's old woman. */
s32 shopChaBaba_proc(Obj *o, s16 msg, void *arg) {
    DemoClerkW *w = CLERK(o);
    s32 pad;
    u32 data;
    Vec3f pos;
    s32 pad2;
    u16 *path;
    u16 *base;

    switch (msg) {
        case 2:
            o->name = "cha_baba";
            data = eneSetup(o, arg);
            DPRINTF("sizeof struct DEMO_CLERK_W [%d]\n", sizeof(DemoClerkW));
            w->model = eneNewAnime(o, (u32) shopChaBabaAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, (Frame *) w->model, 100, 0, 100, 0,
                                                      0);
            demoTalkInit(o, data);
            shopClerkPathInit(o, w->data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopChaBaba_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimChaBaba, 0, &w->model->segs[3], eneGetFile(o, 1));
            w->timer = 0;
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            demoTalkerFree(o);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->path != NULL) {
                        path = w->path;
                        base = (u16 *) stageAddrToRam(w->data->path);
                        w->path = shopClerkActStep(o, base, path);
                    }
                    eneAnimLoop(o, w->model, 0.2f);
                    if (demoTalkEndCheck(o)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n", w->data->talk);
                    demoCamZoom_CL(w->talker);
                    eneSetForward(o, 1);
                    w->state = 2;
                    break;
                case 2:
                    w->vel[0] = 0.0f;
                    w->vel[2] = 0.0f;
                    if (w->talker != NULL) {
                        tskSendMsg(w->talker, 4, &pos);
                        if (eneTurnToPoint(o, pos.x, pos.z, 18.0f) == 1) {
                            w->state = 3;
                        }
                    }
                    break;
                case 3:
                    w->model->frame = 0.0f;
                    fadeAnimStep(&w->anim);
                    if (!scIsRunning()) {
                        shopAnimReset(&w->anim);
                        shopClerkFinish(o);
                        personTalkCamOff();
                        camAiSetHold(0);
                        w->state = 0;
                    }
                    break;
                case 4:
                    break;
            }
            personPlBlockCheck(o);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* A shop's own clerk's work ("struct SHOP_CLERK_W"), at its task + 0x20: an enemy's
 * (EneObj) up to 0x34. */
typedef struct ShopClerkW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ u32 dl; /* shopClerkDl in RAM */
    /* 0x38 */ MPolFrame *shadow;
    /* 0x3C */ Anim anim; /* its face */
    /* 0x4C */ s32 timer;
} ShopClerkW;

#define SHOPCLERK(obj) ((ShopClerkW *) FX(obj))

AnimFrame shopAnimGirl[] = {
    { (u32) shopAnimGirlTex0, 15 },
    { 0x04000C60, 15 },
    { 0x00000000, 0 },
};
u32 shopClerkDl[] = { 0x00000000, 0x00000000, 0x05080000, 0x00000000, 0x00000000 };
extern u8 shopGansoGirlAnim[];

/* "gansogirl": the Ganso shop's girl, who bows (once through) when a player is near. */
s32 shopGansoGirl_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "gansogirl";
            eneSetup(o, arg);
            DPRINTF("struct SHOP_CLERK_W size[%d]\n", sizeof(ShopClerkW));
            w->model = eneNewAnime(o, (u32) shopGansoGirlAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->rot[1] += 3.141592741f;
            w->dl = fadeSegToRam(shopClerkDl, o->file);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopGansoGirl_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimGirl, 0, &w->model->segs[3], eneGetFile(o, 1));
            w->timer = 0;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            if (!scIsRunning()) {
                eneTurnToPoint(o, 0.0f, 0, 2.0f);
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            if (shimPlNearCheckWorld(o, 40.0f)) {
                eneAnimPlayOnce(o, w->model, 0.4f);
                fadeAnimStep(&w->anim);
            } else {
                w->model->frame = 0.0f;
                shopAnimReset(&w->anim);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            if (w->timer == 15) {
                sndPlay(0x30E);
            }
            w->timer++;
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

AnimFrame shopAnimIta[] = { { 0x04002940, 15 }, { 0x04002D60, 15 }, { 0x00000000, 0 } };
u32 shopItaUnusedDl[] = { 0x00000000, 0x00000000, 0x05080000, 0x00000000, 0x00000000 }; /* as shopClerkDl; nothing reads it */
extern u8 shopGansoItaAnim[];

/* "h_ita": the Ganso shop's boy (shopGansoIta). */
s32 shopGansoIta_proc(Obj *o, s16 msg, void *arg) {
    DemoClerkW *w = CLERK(o);
    s32 pad;
    u32 data;
    s32 pad2;
    u16 *path;
    u16 *base;

    switch (msg) {
        case 2:
            o->name = "h_ita";
            data = eneSetup(o, arg);
            DPRINTF("sizeof struct DEMO_CLERK_W [%d]\n", sizeof(DemoClerkW));
            w->model = eneNewAnime(o, (u32) shopGansoItaAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, (Frame *) w->model, 100, 0, 100, 0,
                                                      0);
            demoTalkInit(o, data);
            shopClerkPathInit(o, w->data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopGansoIta_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimIta, 0, &w->model->segs[3], eneGetFile(o, 1));
            w->timer = 0;
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            demoTalkerFree(o);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->path != NULL) {
                        path = w->path;
                        base = (u16 *) stageAddrToRam(w->data->path);
                        w->path = shopClerkActStep(o, base, path);
                    }
                    if (demoTalkEndCheck(o)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n", w->data->talk);
                    shopClerkTalkStart(o);
                    w->state = 3;
                    break;
                case 3:
                    fadeAnimStep(&w->anim);
                    if (!scIsRunning()) {
                        shopAnimReset(&w->anim);
                        shopClerkTalkEnd(o);
                        w->state = 0;
                    }
                    break;
            }
            eneAnimLoop(o, w->model, 0.4f);
            personPlBlockCheck(o);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopHonkeGirlAnim[];

/* "h_girl": the Honke shop's girl (shopHonkeGirl). */
s32 shopHonkeGirl_proc(Obj *o, s16 msg, void *arg) {
    DemoClerkW *w = CLERK(o);
    s32 pad;
    u32 data;
    Vec3f pos;
    s32 pad2;
    u16 *path;
    u16 *base;

    switch (msg) {
        case 2:
            o->name = "h_girl";
            data = eneSetup(o, arg);
            DPRINTF("sizeof struct DEMO_CLERK_W [%d]\n", sizeof(DemoClerkW));
            w->model = eneNewAnime(o, (u32) shopHonkeGirlAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, (Frame *) w->model, 100, 0, 100, 0,
                                                      0);
            demoTalkInit(o, data);
            shopClerkPathInit(o, w->data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopHonkeGirl_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimGirl, 0, &w->model->segs[3], eneGetFile(o, 1));
            shimEneSetLowFlags(o, 3);
            w->timer = 0;
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            demoTalkerFree(o);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->path != NULL) {
                        path = w->path;
                        base = (u16 *) stageAddrToRam(w->data->path);
                        w->path = shopClerkActStep(o, base, path);
                    } else {
                        eneTurnToPoint(o, 64.0f, 40.0f, 18.0f);
                    }
                    if (demoTalkEndCheck(o)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n", w->data->talk);
                    shopClerkTalkStart(o);
                    w->state = 2;
                    break;
                case 2:
                    w->vel[0] = 0.0f;
                    w->vel[2] = 0.0f;
                    if (w->talker != NULL) {
                        tskSendMsg(w->talker, 4, &pos);
                        if (eneTurnToPoint(o, pos.x, pos.z, 18.0f) == 1) {
                            w->state = 3;
                        }
                    }
                    break;
                case 3:
                    eneAnimLoop(o, w->model, 0.4f);
                    fadeAnimStep(&w->anim);
                    if (!scIsRunning()) {
                        shopAnimReset(&w->anim);
                        shopClerkTalkEnd(o);
                        w->state = 0;
                    }
                    break;
                case 4:
                    break;
            }
            personPlBlockCheck(o);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            if (w->timer == 15) {
                sndPlay(0x30F);
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopBabaAnim[];

/* "baba": an old woman in a shop, who moves (and her face) while a player is near. Turned a
 * quarter round with no table entry, else to 0. */
s32 shopBaba_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);
    s32 pad;
    u32 data;

    switch (msg) {
        case 2:
            o->name = "baba";
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopBabaAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (data == 0) {
                w->rot[1] += 1.570796371f;
            } else {
                w->rot[1] = 0.0f;
            }
            fadeAnimStart(&w->anim, shopAnimChaBaba, 0, &w->model->segs[3], eneGetFile(o, 1));
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            if (shimPlNearCheckWorld(o, 30.0f)) {
                eneAnimLoop(o, w->model, 0.2f);
                fadeAnimStep(&w->anim);
            }
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

AnimFrame shopAnimDebu[] = {
    { 0x040018C0, 15 },
    { 0x04001CE0, 15 },
    { 0x00000000, 0 },
};
extern u8 shopYorozuDebuAnim[];

/* "yorozudebu": the general store's fat man, who moves while a player is near. */
s32 shopYorozuDebu_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "yorozudebu";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopYorozuDebuAnim);
            if (w->model == NULL) {
                return -1;
            }
            fadeAnimStart(&w->anim, shopAnimDebu, 0, &w->model->segs[3], eneGetFile(o, 1));
            w->timer = 0;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            if (shimPlNearCheckWorld(o, 40.0f)) {
                eneAnimLoop(o, w->model, 0.8f);
                fadeAnimStep(&w->anim);
            }
            w->timer++;
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "pura"'s work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct PuraW {
    /* 0x00 */ u8 state; /* 0 down, 1 coming down, 2 waiting, 3 turning its frame, 4 turning,
                          * 5 going up, 6 done */
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 hitKind;
    /* 0x0A */ u8 hit;
    /* 0x0B */ s8 hp;
    /* 0x0C */ AnimeFrame *model;
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ f32 speed; /* its animation's step a frame */
    /* 0x38 */ u32 timer;
} PuraW;

#define PURA(obj) ((PuraW *) FX(obj))

extern u8 shopPuraAnim[];

/* "pura": a thing (a sign) that comes down from 80 to 11, waits, turns (its animation to
 * frame 5.7 if it is in the first half, else 18.4, in 20 frames) and goes back up. */
s32 shopPura_proc(Obj *o, s16 msg, void *arg) {
    PuraW *w = PURA(o);
    s32 pad;
    f32 f;

    switch (msg) {
        case 2:
            o->name = "pura";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopPuraAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->rot[1] -= 1.570796371f;
            w->state = 6;
            w->y = 80.0f;
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
                    w->vel[1] = (11.0f - w->y) / 60.0f;
                    w->timer = 0;
                    w->state++;
                    break;
                case 1:
                    if (w->y <= 11.0f) {
                        w->y = 11.0f;
                        w->vel[1] = 0.0f;
                        w->state++;
                    }
                    w->model->frame = 5.699999809f;
                    break;
                case 2:
                    if (++w->timer > 30) {
                        eneAnimLoop(o, w->model, 0.2f);
                    }
                    break;
                case 3:
                    w->timer = 0;
                    f = w->model->frame;
                    if (f < polAnimeLastFrame(w->model) / 2) {
                        w->speed = (5.699999809f - f) / 20.0f;
                    } else {
                        w->speed = (18.39999962f - f) / 20.0f;
                    }
                    w->state++;
                    break;
                case 4:
                    w->model->frame += w->speed;
                    if (++w->timer == 20) {
                        w->model->frame = 5.699999809f;
                        w->timer = 0;
                        w->state++;
                        w->vel[1] = (80.0f - w->y) / 60.0f;
                    }
                    break;
                case 5:
                    if (80.0f <= w->y) {
                        w->y = 80.0f;
                        w->vel[1] = 0.0f;
                        w->state++;
                    }
                    break;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopDancerAnim[];

/* "dancer": a dancer, dancing on the spot. */
s32 shopDancer_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "dancer";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopDancerAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->rot[1] -= 1.570796371f;
            w->shadow = shadowCreate();
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.3f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 2.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

AnimFrame shopAnimOyaji[] = {
    { 0x04001080, 15 },
    { 0x040014A0, 15 },
    { 0x00000000, 0 },
};
extern u8 shopBannyaOyaziAnim[];

/* "bannya": the Bannya master (shopBannyaOyazi). */
s32 shopBannyaOyazi_proc(Obj *o, s16 msg, void *arg) {
    DemoClerkW *w = CLERK(o);
    s32 pad;
    u32 data;
    s32 pad2;

    switch (msg) {
        case 2:
            o->name = "bannya";
            data = eneSetup(o, arg);
            DPRINTF("sizeof struct DEMO_CLERK_W [%d]\n", sizeof(DemoClerkW));
            w->model = eneNewAnime(o, (u32) shopBannyaOyaziAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, (Frame *) w->model, 100, 0, 100, 0,
                                                      0);
            demoTalkInit(o, data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopBannyaOyazi_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimOyaji, 0, &w->model->segs[3], eneGetFile(o, 1));
            w->timer = 0;
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            demoTalkerFree(o);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (demoTalkEndCheck(o)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n", w->data->talk);
                    shopClerkTalkStart(o);
                    w->state = 3;
                    break;
                case 3:
                    eneAnimLoop(o, w->model, 0.4f);
                    fadeAnimStep(&w->anim);
                    if (!scIsRunning()) {
                        shopAnimReset(&w->anim);
                        shopClerkTalkEnd(o);
                        w->state = 0;
                    }
                    break;
            }
            personPlBlockCheck(o);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            if (w->timer == 15) {
                sndPlay(0x30D);
            }
            if (w->timer >= 6 && w->timer < 36) {
                fadeAnimStep(&w->anim);
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "ban_cri": the Bannya's criminal, an unseen talker who makes its master (kind 0x1A8 on
 * stages 0xB7 and 0xE2, else 0x1A4) where it stands. */
s32 shopBannyaCriminal_proc(Obj *o, s16 msg, void *arg) {
    DemoClerkW *w = CLERK(o);
    s32 pad;
    u32 data;
    s32 pad2;
    s32 kind;
    s32 pad3;

    switch (msg) {
        case 2:
            o->name = "ban_cri";
            data = eneSetup(o, arg);
            DPRINTF("sizeof struct DEMO_CLERK_W [%d]\n", sizeof(DemoClerkW));
            w->model = eneNewAnime(o, 0);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, (Frame *) w->model, 200, 0, 100, 0,
                                                      0);
            demoTalkInit(o, data);
            switch (gGameWork.stage) {
                case 0xA2:
                    kind = 0x1A4;
                    break;
                case 0xB7:
                    kind = 0x1A8;
                    break;
                case 0xE2:
                    kind = 0x1A8;
                    break;
                default:
                    kind = 0x1A4;
                    break;
            }
            if (eneMakeEx(o, kind, 0, w->x, w->y, w->z, 2, (u32 (*)(void *, void *, void *)) 1) == NULL) {
                DPRINTF("shopBannyaCriminal_proc: syujin NULL!!\n");
                return -1;
            }
            w->timer = 0;
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            demoTalkerFree(o);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (demoTalkEndCheck(o)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n", w->data->talk);
                    shopClerkTalkStart(o);
                    w->state = 3;
                    break;
                case 3:
                    if (!scIsRunning()) {
                        shopClerkTalkEnd(o);
                        w->state = 0;
                    }
                    break;
            }
            personPlBlockCheck(o);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

AnimFrame shopAnimYadoya[] = {
    { 0x04002100, 15 },
    { 0x04002520, 15 },
    { 0x00000000, 0 },
};
extern u8 shopYadoyaAnim[];

/* "yadoya": the inn's keeper, who moves while a player is near.
 * Matched 2026-10-16; it was parked for eneMove's load scheduled ahead of w's addiu (its twin
 * shopYorozuDebu_proc already had the game's order); a never-true unsigned test spending all
 * nine of ugen's integer temps, marked FAKE below, rolls the free list back to the game's. */
s32 shopYadoya_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "yadoya";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopYadoyaAnim);
            if (w->model == NULL) {
                return -1;
            }
            fadeAnimStart(&w->anim, shopAnimYadoya, 0, &w->model->segs[3], eneGetFile(o, 1));
            w->timer = 0;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            /* FAKE: a never-true unsigned test (IDO_NOTES 284) puts a block boundary after w's
             * addiu, so as1 orders it before eneMove's lui; its nine ops spend ugen's whole
             * nine-register temp ring (t8..t7), so the timer's temps come back to t8/t0. as1
             * deletes the branch and the dead chain: no instruction is left. */
            if (((((((((((u32) w + 1) ^ 1) + 1) ^ 1) + 1) ^ 1) + 1) ^ 1) + 1) < 0) {
                break;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            if (shimPlNearCheckWorld(o, 40.0f)) {
                eneAnimLoop(o, w->model, 0.8f);
                fadeAnimStep(&w->anim);
            }
            w->timer++;
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

AnimFrame shopAnimGoemon[] = {
    { 0x030215B0, 50 },
    { 0x030219D0, 50 },
    { 0x00000000, 0 },
};
extern u8 shopMatGRunAnim[];

/* "matgoe": Goemon at the mat shop (shopMatG_run), running on the spot. */
s32 shopMatG_run_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "matgoe";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopMatGRunAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopMatG_run_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->rot[1] += 0.3926990926f;
            fadeAnimStart(&w->anim, shopAnimGoemon, 0, &w->model->segs[3], eneGetFile(o, 0));
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.4f);
            fadeAnimStep(&w->anim);
            if (w->timer == 50) {
                sndPlay(0x13D);
            }
            w->timer++;
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopMatEAnim[];

/* "matE": Ebisumaru at the mat shop (shopMatE). */
s32 shopMatE_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "matE";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopMatEAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopMatE_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.2f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopMatSAnim[];

/* "matS": Sasuke at the mat shop (shopMatS). */
s32 shopMatS_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "matS";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopMatSAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->rot[1] -= 2.19911491870880127;
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopMatS_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.3f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            if (w->timer == 3) {
                sndPlay(0xC2);
            }
            w->timer++;
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopMatYAnim[];

/* "matY": Yae at the mat shop (shopMatY). */
s32 shopMatY_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "matY";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopMatYAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopMatY_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.4f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopTakGAnim[];

/* "takG": Goemon at the take (bamboo) shop (shopTakG). */
s32 shopTakG_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "takG";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopTakGAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopTakG_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimGoemon, 0, &w->model->segs[3], eneGetFile(o, 0));
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.1f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            fadeAnimStep(&w->anim);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

AnimFrame shopAnimEbisu[] = {
    { 0x03021DF0, 50 },
    { 0x03022210, 50 },
    { 0x00000000, 0 },
};
extern u8 shopTakEAnim[];

/* "takE": Ebisumaru at the take shop (shopTakE). */
s32 shopTakE_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "takE";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopTakEAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopTakE_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimEbisu, 0, &w->model->segs[3], eneGetFile(o, 0));
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.08f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            fadeAnimStep(&w->anim);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

AnimFrame shopAnimSasuke[] = {
    { 0x03022630, 50 },
    { 0x03022A50, 50 },
    { 0x00000000, 0 },
};
extern u8 shopTakSAnim[];

/* "taksas": Sasuke at the take shop (shopTakS). */
s32 shopTakS_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "taksas";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopTakSAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopTakS_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimSasuke, 0, &w->model->segs[3], eneGetFile(o, 0));
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.1f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            fadeAnimStep(&w->anim);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

AnimFrame shopAnimYae[] = {
    { 0x03022E70, 50 },
    { 0x03023290, 50 },
    { 0x00000000, 0 },
};
extern u8 shopTakYAnim[];

/* "cha_baba": Yae at the take shop (shopTakY; the old woman's name). */
s32 shopTakY_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "cha_baba";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopTakYAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopTakY_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimYae, 0, &w->model->segs[3], eneGetFile(o, 0));
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.1f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            fadeAnimStep(&w->anim);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopUmeGAnim[];

/* "umegoe": Goemon at the ume (plum) shop (shopUmeG), lying down (turned a quarter round on
 * x and y). */
s32 shopUmeG_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "umegoe";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopUmeGAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopUmeG_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->rot[0] -= 1.570796371f;
            w->rot[1] -= 1.570796371f;
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimGoemon, 0, &w->model->segs[3], eneGetFile(o, 0));
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.1f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            fadeAnimStep(&w->anim);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopUmeEAnim[];

/* "umeebi": Ebisumaru at the ume shop (shopUmeE), lying down. */
s32 shopUmeE_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "umeebi";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopUmeEAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->rot[0] -= 1.570796371f;
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopUmeE_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimEbisu, 0, &w->model->segs[3], eneGetFile(o, 0));
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.08f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            fadeAnimStep(&w->anim);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopUmeSAnim[];

/* "umesas": Sasuke at the ume shop (shopUmeS). */
s32 shopUmeS_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "umesas";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopUmeSAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopUmeS_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimSasuke, 0, &w->model->segs[3], eneGetFile(o, 0));
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.1f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            fadeAnimStep(&w->anim);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopUmeYAnim[];

/* "umeyae": Yae at the ume shop (shopUmeY). */
s32 shopUmeY_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "umeyae";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopUmeYAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopUmeY_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimYae, 0, &w->model->segs[3], eneGetFile(o, 0));
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.1f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            fadeAnimStep(&w->anim);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopUmePAnim[];

/* "umepura": a thing at the ume shop (shopUmeP), turned an eighth round. */
s32 shopUmeP_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "umepura";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopUmePAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopUmeP_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->rot[1] += 0.7853981853f;
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimGoemon, 0, &w->model->segs[3], eneGetFile(o, 0));
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.1f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            fadeAnimStep(&w->anim);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopUmeOAnim[];

/* "umeoku": the ume shop's mistress (shopUmeO). */
s32 shopUmeO_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "umeoku";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopUmeOAnim);
            if (w->model == NULL) {
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopUmeO_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneFall(o, 0.8f);
            w->hit &= ~1;
            if (eneLand((EneObj *) w, 10.0f) == 1) {
                w->hit |= 1;
                w->vel[1] = 0.0f;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.1f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopKyakuOyajiAnim[];

/* "oyaji": a sleepy customer, an old man (out at its table entry's hours; on stage 0x98 its
 * low flags 3), who nods off every 120 frames. */
s32 shopKyakuOyaji_proc(Obj *o, s16 msg, void *arg) {
    DemoClerkW *w = CLERK(o);
    s32 pad;
    u32 data;
    s32 pad2;

    switch (msg) {
        case 2:
            o->name = "oyaji";
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopKyakuOyajiAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, (Frame *) w->model, 100, 0, 100, 0,
                                                      0);
            demoTalkInit(o, data);
            shopClerkPathInit(o, w->data);
            if (!personHoursCheck(o, w->data->hours)) {
                DPRINTF("ねむいよ〜〜\n");
                eneReleaseSpawn(w->spawn);
                return -1;
            }
            if (gGameWork.stage == 0x98) {
                shimEneSetLowFlags(o, 3);
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimOyaji, 0, &w->model->segs[3], eneGetFile(o, 1));
            w->timer = 0;
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            demoTalkerFree(o);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->timer % 120 < 60) {
                        if (eneAnimPlayOnce(o, w->model, 0.2f) == 1) {
                            w->model->frame = 0.0f;
                            w->timer = 60;
                        }
                    }
                    if (demoTalkEndCheck(o)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n", w->data->talk);
                    shopClerkTalkStart(o);
                    w->state = 3;
                    break;
                case 3:
                    w->model->frame = 0.0f;
                    if (!scIsRunning()) {
                        shopClerkTalkEnd(o);
                        w->timer = 0;
                        w->state = 0;
                    }
                    break;
            }
            fadeAnimStep(&w->anim);
            personPlBlockCheck(o);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopKyakuDebuAnim[];

/* "debu": a sleepy customer, a fat man (on stage 0xD8 its low flags 3). */
s32 shopKyakuDebu_proc(Obj *o, s16 msg, void *arg) {
    DemoClerkW *w = CLERK(o);
    s32 pad;
    u32 data;
    s32 pad2;

    switch (msg) {
        case 2:
            o->name = "debu";
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopKyakuDebuAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, (Frame *) w->model, 100, 0, 100, 0,
                                                      0);
            demoTalkInit(o, data);
            shopClerkPathInit(o, w->data);
            if (!personHoursCheck(o, w->data->hours)) {
                DPRINTF("ねむいよ〜〜\n");
                eneReleaseSpawn(w->spawn);
                return -1;
            }
            if (gGameWork.stage == 0xD8) {
                shimEneSetLowFlags(o, 3);
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimDebu, 0, &w->model->segs[3], eneGetFile(o, 1));
            w->timer = 0;
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            demoTalkerFree(o);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (demoTalkEndCheck(o)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n", w->data->talk);
                    shopClerkTalkStart(o);
                    w->state = 3;
                    break;
                case 3:
                    if (!scIsRunning()) {
                        shopClerkTalkEnd(o);
                        w->state = 0;
                    }
                    break;
            }
            eneAnimLoop(o, w->model, 0.2f);
            fadeAnimStep(&w->anim);
            personPlBlockCheck(o);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopKyakuObasanAnim[];

/* "obasan": a sleepy customer, a middle-aged woman, who nods off every 60 frames while she
 * talks. */
s32 shopKyakuObasan_proc(Obj *o, s16 msg, void *arg) {
    DemoClerkW *w = CLERK(o);
    s32 pad;
    u32 data;
    s32 pad2;

    switch (msg) {
        case 2:
            o->name = "obasan";
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopKyakuObasanAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, (Frame *) w->model, 100, 0, 100, 0,
                                                      0);
            demoTalkInit(o, data);
            shopClerkPathInit(o, w->data);
            if (!personHoursCheck(o, w->data->hours)) {
                DPRINTF("ねむいよ〜〜\n");
                eneReleaseSpawn(w->spawn);
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimYadoya, 0, &w->model->segs[3], eneGetFile(o, 1));
            w->timer = 0;
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            demoTalkerFree(o);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->model->frame = 0.0f;
                    if (demoTalkEndCheck(o)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n", w->data->talk);
                    shopClerkTalkStart(o);
                    w->state = 3;
                    w->timer = 0;
                    break;
                case 3:
                    fadeAnimStep(&w->anim);
                    if (w->timer % 60 < 30) {
                        if (eneAnimPlayOnce(o, w->model, 0.4f) == 1) {
                            w->model->frame = 0.0f;
                            w->timer = 30;
                        }
                    }
                    if (!scIsRunning()) {
                        shopAnimReset(&w->anim);
                        shopClerkTalkEnd(o);
                        w->state = 0;
                    }
                    break;
            }
            personPlBlockCheck(o);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopKyakuBouzuAnim[];

/* "Bozu": a sleepy customer, a monk (shopKyakuBouzu), who walks its path. */
s32 shopKyakuBouzu_proc(Obj *o, s16 msg, void *arg) {
    DemoClerkW *w = CLERK(o);
    s32 pad;
    u32 data;
    s32 pad2;
    u16 *path;
    u16 *base;
    Vec3f pos;

    switch (msg) {
        case 2:
            o->name = "Bozu";
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopKyakuBouzuAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, (Frame *) w->model, 100, 0, 100, 0,
                                                      0);
            demoTalkInit(o, data);
            shopClerkPathInit(o, w->data);
            if (!personHoursCheck(o, w->data->hours)) {
                DPRINTF("ねむいよ〜〜\n");
                eneReleaseSpawn(w->spawn);
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopKyakuBouzu_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimGirl, 0, &w->model->segs[3], eneGetFile(o, 1));
            w->timer = 0;
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            demoTalkerFree(o);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->path != NULL) {
                        path = w->path;
                        base = (u16 *) stageAddrToRam(w->data->path);
                        w->path = shopClerkActStep(o, base, path);
                    }
                    eneAnimLoop(o, w->model, 0.2f);
                    if (demoTalkEndCheck(o)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n", w->data->talk);
                    shopClerkTalkStart(o);
                    w->state = 2;
                    break;
                case 2:
                    w->vel[0] = 0.0f;
                    w->vel[2] = 0.0f;
                    if (w->talker != NULL) {
                        tskSendMsg(w->talker, 4, &pos);
                        if (eneTurnToPoint(o, pos.x, pos.z, 18.0f) == 1) {
                            w->state = 3;
                        }
                    }
                    break;
                case 3:
                    w->model->frame = 0.0f;
                    fadeAnimStep(&w->anim);
                    if (!scIsRunning()) {
                        shopAnimReset(&w->anim);
                        shopClerkTalkEnd(o);
                        w->state = 0;
                    }
                    break;
            }
            personPlBlockCheck(o);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopKyakuOtokoAnim[];

/* "otoko": a sleepy customer, a man (its print still "shopKyakuBouzu"), who walks its
 * path. */
s32 shopKyakuOtoko_proc(Obj *o, s16 msg, void *arg) {
    DemoClerkW *w = CLERK(o);
    s32 pad;
    u32 data;
    s32 pad2;
    u16 *path;
    u16 *base;
    Vec3f pos;

    switch (msg) {
        case 2:
            o->name = "otoko";
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopKyakuOtokoAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, (Frame *) w->model, 100, 0, 100, 0,
                                                      0);
            demoTalkInit(o, data);
            shopClerkPathInit(o, w->data);
            if (!personHoursCheck(o, w->data->hours)) {
                DPRINTF("ねむいよ〜〜\n");
                eneReleaseSpawn(w->spawn);
                return -1;
            }
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopKyakuBouzu_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimIta, 0, &w->model->segs[3], eneGetFile(o, 1));
            w->timer = 0;
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            demoTalkerFree(o);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->path != NULL) {
                        path = w->path;
                        base = (u16 *) stageAddrToRam(w->data->path);
                        w->path = shopClerkActStep(o, base, path);
                    }
                    eneAnimLoop(o, w->model, 0.2f);
                    if (demoTalkEndCheck(o)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n", w->data->talk);
                    shopClerkTalkStart(o);
                    w->state = 2;
                    break;
                case 2:
                    w->vel[0] = 0.0f;
                    w->vel[2] = 0.0f;
                    if (w->talker != NULL) {
                        tskSendMsg(w->talker, 4, &pos);
                        if (eneTurnToPoint(o, pos.x, pos.z, 18.0f) == 1) {
                            w->state = 3;
                        }
                    }
                    break;
                case 3:
                    w->model->frame = 0.0f;
                    fadeAnimStep(&w->anim);
                    if (!scIsRunning()) {
                        shopAnimReset(&w->anim);
                        shopClerkTalkEnd(o);
                        w->state = 0;
                    }
                    break;
            }
            personPlBlockCheck(o);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopAwaTex[];
extern u8 shopMituyaOmituAnim[]; /* Omitsu's at the Mituya (file set 0xC7): shopChaBabaAnim's address in another file */
extern u8 shopSarayaManiaAnim[]; /* the Saraya mania's (file set 0x49): shopAwaTex's address in another file */

/* The bubbles' display list: one 64x32 CI4 texture (shopAwaTex) in two tiles, tile 0
 * scrolled by t along both axes, tile 1 by (-t, t + 10), blended through the environment
 * colour (as tlb_0071's water). IDO schedules by line: the first tile's load and render (one
 * macro of the original's, likely) on one line gives the game's order of its gfx++ and the
 * 0xF2000000. */
Gfx *shopAwaDraw(Gfx *gfx, u16 t) {
    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 128, 128, 128, 128);
    gDPSetTextureImage(gfx++, G_IM_FMT_CI, G_IM_SIZ_16b, 1, shopAwaTex); gDPSetTile(gfx++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 6, G_TX_NOLOD); gDPLoadSync(gfx++); gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, ((64 * 32 + 3) >> 2) - 1, CALC_DXT_4b(64)); gDPPipeSync(gfx++); gDPSetTile(gfx++, G_IM_FMT_CI, G_IM_SIZ_4b, ((64 >> 1) + 7) >> 3, 0, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 6, G_TX_NOLOD); gDPSetTileSize(gfx++, G_TX_RENDERTILE, t, t, (64 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC);
    gDPSetTextureImage(gfx++, G_IM_FMT_CI, G_IM_SIZ_16b, 1, shopAwaTex);
    gDPSetTile(gfx++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0x100, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD,
               G_TX_NOMIRROR | G_TX_WRAP, 6, G_TX_NOLOD);
    gDPLoadSync(gfx++);
    gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, ((64 * 32 + 3) >> 2) - 1, CALC_DXT_4b(64));
    gDPPipeSync(gfx++); gDPSetTile(gfx++, G_IM_FMT_CI, G_IM_SIZ_4b, ((64 >> 1) + 7) >> 3, 0x100, 1, 0, G_TX_NOMIRROR | G_TX_WRAP, 5, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 6, G_TX_NOLOD); gDPSetTileSize(gfx++, 1, -t, t + 10, (64 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC);
    gSPEndDisplayList(gfx++);
    return gfx;
}

/* "awa"'s work, at its task + 0x20: an enemy's (EneObj) up to 0x34. */
typedef struct AwaW {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 zone;
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
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
    /* 0x38 */ s32 scroll; /* down one a draw */
} AwaW;

#define AWA(obj) ((AwaW *) FX(obj))

extern Gfx shopAwaDl[];

/* "awa": bubbles, a "pol" drawn in layer 4's third frame with a scrolling display list
 * (shopAwaDraw) made each draw (message 3). */
s32 shopAwa_proc(Obj *o, s16 msg, void *arg) {
    AwaW *w = AWA(o);
    s32 pad[2];

    switch (msg) {
        case 2:
            o->name = "awa";
            eneSetup(o, arg);
            w->pol = eneNewPol(o, shopAwaDl);
            if (w->pol == NULL) {
                return -1;
            }
            frmMove(&w->pol->f, ((Frame **) &gFrmLayer4)[2]);
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
            gGfxDlHead = shopAwaDraw(gGfxDlHead, w->scroll);
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

AnimFrame shopAnimOmitu[] = {
    { (u32) shopAnimOmituTex0, 10 },
    { 0x03005DE0, 10 },
    { 0x00000000, 0 },
};

/* "omitu": Omitsu of the Mitsuya tea house (shopMituyaOmitu), out once the area is cleared
 * (switch 0xBD). */
s32 shopMituyaOmitu_proc(Obj *o, s16 msg, void *arg) {
    DemoClerkW *w = CLERK(o);
    s32 pad;
    u32 data;
    Vec3f pos;
    s32 pad2;
    u16 *path;
    s32 pad3;

    switch (msg) {
        case 2:
            o->name = "omitu";
            DPRINTF("sizeof struct DEMO_CLERK_W [%d]\n", sizeof(DemoClerkW));
            if (swWorkTest(0xBD) == 0) {
                DPRINTF("エリアクリアーしてないのでおみっちゃんでません\n");
                return -1;
            }
            data = eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopMituyaOmituAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[2] = 0.1099999994f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, (Frame *) w->model, 100, 0, 100, 0,
                                                      0);
            demoTalkInit(o, data);
            shopClerkPathInit(o, w->data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopMituyaOmitu_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimOmitu, 0, &w->model->segs[3], eneGetFile(o, 0));
            w->timer = 0;
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            demoTalkerFree(o);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->path != NULL) {
                        path = w->path;
                        w->path = shopClerkActStep(o, (u16 *) stageAddrToRam(w->data->path), path);
                    }
                    eneAnimLoop(o, w->model, 0.2f);
                    if (demoTalkEndCheck(o)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n", w->data->talk);
                    shopClerkTalkStart(o);
                    w->state = 2;
                    break;
                case 2:
                    w->vel[0] = 0.0f;
                    w->vel[2] = 0.0f;
                    if (w->talker != NULL) {
                        tskSendMsg(w->talker, 4, &pos);
                        if (eneTurnToPoint(o, pos.x, pos.z, 18.0f) == 1) {
                            w->state = 3;
                        }
                    }
                    break;
                case 3:
                    w->model->frame = 0.0f;
                    fadeAnimStep(&w->anim);
                    if (!scIsRunning()) {
                        shopAnimReset(&w->anim);
                        shopClerkTalkEnd(o);
                        w->state = 0;
                    }
                    break;
                case 4:
                    break;
            }
            personPlBlockCheck(o);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

/* "omitu" again: the plate shop's collector (shopSarayaMania; its name is Omitsu's), its face
 * from the set file 0x74's first. */
s32 shopSarayaMania_proc(Obj *o, s16 msg, void *arg) {
    DemoClerkW *w = CLERK(o);
    s32 pad;
    u32 data;
    Vec3f pos;
    s32 pad2;
    u16 *path;

    switch (msg) {
        case 2:
            o->name = "omitu";
            data = eneSetup(o, arg);
            DPRINTF("sizeof struct DEMO_CLERK_W [%d]\n", sizeof(DemoClerkW));
            w->model = eneNewAnime(o, (u32) shopSarayaManiaAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->box = ((AtkAddXYZFunc) acollAtkAddXYZ)(ene_atk_root_frame, (Frame *) w->model, 100, 0, 100, 0,
                                                      0);
            demoTalkInit(o, data);
            shopClerkPathInit(o, w->data);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopSarayaMania_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            fadeAnimStart(&w->anim, shopAnimGirl, 0, &w->model->segs[3], floadGetSetFile(0x74, 0));
            w->timer = 0;
            break;
        case 8:
            return demoTalk(o, arg);
        case 1:
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            demoTalkerFree(o);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->path != NULL) {
                        path = w->path;
                        w->path = shopClerkActStep(o, (u16 *) stageAddrToRam(w->data->path), path);
                    }
                    eneAnimLoop(o, w->model, 0.2f);
                    if (demoTalkEndCheck(o)) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    scRunStart(w->data->talk, plGetIndex(w->talker) + 1, plGetChara(w->talker));
                    DPRINTF("talkNo[%d]\n", w->data->talk);
                    shopClerkTalkStart(o);
                    w->state = 2;
                    break;
                case 2:
                    w->vel[0] = 0.0f;
                    w->vel[2] = 0.0f;
                    if (w->talker != NULL) {
                        tskSendMsg(w->talker, 4, &pos);
                        if (eneTurnToPoint(o, pos.x, pos.z, 18.0f) == 1) {
                            w->state = 3;
                        }
                    }
                    break;
                case 3:
                    w->model->frame = 0.0f;
                    fadeAnimStep(&w->anim);
                    if (!scIsRunning()) {
                        shopClerkTalkEnd(o);
                        w->state = 0;
                        shopAnimReset(&w->anim);
                    }
                    break;
                case 4:
                    break;
            }
            personPlBlockCheck(o);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            w->timer++;
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopTenguAnim[];

/* "tengu": a tengu mask (scaled 0.65), lit with no colour at each draw (message 3), that
 * moves while a player is near. */
s32 shopTengu_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);
    s32 pad;

    switch (msg) {
        case 2:
            o->name = "tengu";
            eneSetup(o, arg);
            w->model = eneNewAnime(o, (u32) shopTenguAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[2] = 0.6499999762f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->timer = 0;
            break;
        case 3:
            w->model->dl = eneDlEnvColorLit(o, 0, 0, 0, 0, 1);
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 0:
            eneStepHookPre(o);
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            if (shimPlNearCheckWorld(o, 40.0f)) {
                eneAnimLoop(o, w->model, 0.8f);
            }
            w->timer++;
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}

extern u8 shopTenguTurnAnim[];

/* "tengu" again: a tengu (scaled 0.65, turned round) made from the Ganso girl's proc (its print
 * still "shopGansoGirl"), lit with no colour at each draw, turning to the middle while no
 * scenario runs. */
s32 shopTenguTurn_proc(Obj *o, s16 msg, void *arg) {
    ShopClerkW *w = SHOPCLERK(o);

    switch (msg) {
        case 2:
            o->name = "tengu";
            eneSetup(o, arg);
            DPRINTF("struct SHOP_CLERK_W size[%d]\n", sizeof(ShopClerkW));
            w->model = eneNewAnime(o, (u32) shopTenguTurnAnim);
            if (w->model == NULL) {
                return -1;
            }
            w->model->scale[2] = 0.6499999762f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            w->rot[1] += 3.141592741f;
            w->dl = fadeSegToRam(shopClerkDl, o->file);
            if (eneSetOnEarth(o) == -1) {
                DPRINTF("shopGansoGirl_proc: Error: 地面にセット失敗\n");
                return -1;
            }
            w->shadow = shadowCreate();
            w->timer = 0;
            break;
        case 1:
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->shadow != NULL) {
                frmDestroy(&w->shadow->f);
            }
            break;
        case 3:
            w->model->dl = eneDlEnvColorLit(o, 0, 0, 0, 0, 1);
            break;
        case 0:
            eneStepHookPre(o);
            if (!scIsRunning()) {
                eneTurnToPoint(o, 0.0f, 0.0f, 2.0f);
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(o, &w->model->f);
            }
            eneAnimLoop(o, w->model, 0.4f);
            shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.0f, NULL);
            w->timer++;
            eneStepHookPost(o);
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
