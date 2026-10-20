#include "tlb.h"
#include "tlb/tlb_0324.h"

extern Obj *gBattleTask; /* the battle's task (beffect.c) */
extern CamView *gBattleCamView; /* the battle's camera (beffect.c) */

/* rand(n): 0 .. n - 1 (tlb_0324_3). This file calls its own and its neighbours' functions far,
 * through t9, as tlb.h does main's. */
#define btaiRand (*btaiRand)

/* libultra's, through their addresses (code at 0x08000000 cannot jal them). */
#define sinf (*sinf)
#define cosf (*cosf)

/* The task procs this file starts. */
s32 btaiKokeEfProc(Obj *obj, s16 msg, void *arg); /* "KOKEef" */
s32 btaiTbubbleProc(Obj *obj, s16 msg, void *arg); /* "Tbubble" */
s32 btaiUzuInProc(Obj *obj, s16 msg, void *arg); /* "uzu_in" */
s32 btaiTsbubbleProc(Obj *obj, s16 msg, void *arg); /* "Tsbubble" */
s32 btaiU3bubbleProc(Obj *obj, s16 msg, void *arg); /* "U3bubble" */
s32 btaiHagataProc(Obj *obj, s16 msg, void *arg); /* "hagata" */
s32 btaiIkasumiProc(Obj *obj, s16 msg, void *arg); /* "ikasumi" */
s32 btaiTzanzouProc(Obj *obj, s16 msg, void *arg); /* "Tzanzou" */
s32 btaiTzanzou2Proc(Obj *obj, s16 msg, void *arg); /* "Tzanzou" */
s32 btaiKiranProc(Obj *obj, s16 msg, void *arg); /* "kiran" */
s32 btaiBombballProc(Obj *obj, s16 msg, void *arg); /* "bombball" */
s32 btaiFlash2Proc(Obj *obj, s16 msg, void *arg); /* "flash2" */
s32 btaiBarriProc(Obj *obj, s16 msg, void *arg); /* "barri" */
s32 btaiBarriwProc(Obj *obj, s16 msg, void *arg); /* "barriw" */

/* The "SUNAef" (sand dust) task's creation argument, read by sunaEfProc on its init. */
typedef struct SunaArg {
    /* 0x00 */ u8 pad00[4];
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ u8 pad10[0x14];
    /* 0x24 */ f32 pos[3];
    /* 0x30 */ f32 scale;
    /* 0x34 */ u8 pad34[8];
    /* 0x3C */ s32 time;
} SunaArg; /* size 0x40 */

/* The "SUNAef" (sand dust) task's work. */
typedef struct SunaW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ Anim anim;
    /* 0x24 */ u8 pad24[4];
    /* 0x28 */ f32 dy; /* the third's height over its place */
    /* 0x2C */ u8 pad2C[0xC];
    /* 0x38 */ s32 time; /* also the alpha */
    /* 0x3C */ s32 count;
    /* 0x40 */ u8 pad40[2];
    /* 0x42 */ u8 fadeOut; /* 0: the third's alpha rises; 1 (at 255): it falls */
} SunaW;

extern s32 btaiEffectsEnd; /* 1: the battle's effects end (read by every effect here) */
extern AnimFrame btaiSandDustAnim[]; /* the sand dust's texture animation (file 0x123) */
extern u8 btaiSandDustDl[]; /* the sand dust's display list, in file 0x123 */

/* "SUNAef" (sand dust): a puff from file 0x123 that grows, drifts by its velocity (never past
 * z -100) and fades with its time. */
s32 sunaEfProc(Obj *obj, s16 msg, void *arg) {
    SunaW *w = (SunaW *) (obj + 1);
    SunaArg *a = arg;

    switch (msg) {
        case 2:
            obj->name = "SUNAef";
            PRINTF("砂ぼこり\n");
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) btaiSandDustDl, 0x123);
            fadeAnimStart(&w->anim, btaiSandDustAnim, 0, &w->frame->segs[3], floadGetSetFile(0x123, 0));
            w->frame->flags |= 7;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = a->scale;
            w->frame->pos[0] = a->pos[0];
            w->frame->pos[1] = a->pos[1];
            w->frame->pos[2] = a->pos[2];
            w->vel[0] = a->vel[0];
            w->vel[1] = a->vel[1];
            w->vel[2] = a->vel[2];
            w->time = a->time;
            w->fadeOut = 0;
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(0xD1, 0xCC, 0x93, 0xB7, 0xB2, 0x7F, w->time);
            break;
        case 0:
            if (btaiEffectsEnd == 1) {
                return -1;
            }
            if (gGameFrameCount % 3 == 0) {
                fadeAnimStep(&w->anim);
            }
            w->time--;
            if (w->time <= 0) {
                w->time = 0;
                return -1;
            }
            w->frame->scale[0] += 0.1;
            w->frame->scale[1] += 0.1;
            w->frame->scale[2] += 0.1;
            w->frame->pos[0] += w->vel[0];
            w->frame->pos[2] += w->vel[2];
            if (w->frame->pos[2] > -100.0f) {
                w->frame->pos[2] = -100.0f;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A sand-dust puff at (x, y - 850, z), drifting by a fifth of (dx, 0, dz), under the battle's task. */
void sunaEfMake(f32 x, f32 y, f32 z, f32 dx, f32 dz, f32 scale, s32 time) {
    s32 pad;
    SunaArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y + -850.0f;
    arg.pos[2] = z;
    arg.vel[1] = 0.0f;
    arg.vel[0] = dx * 0.2;
    arg.vel[2] = dz * 0.2;
    arg.scale = scale;
    arg.time = time;
    tskCreate(gBattleTask, sunaEfProc, gBattleTask->file, &arg);
}
#define sunaEfMake (*sunaEfMake)

/* The work of a task that kicks up sand: its frame and its drift. */
typedef struct SunaDriftW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
} SunaDriftW;

/* Matched 2026-10-09; it was parked for a dead local (n, computed but unread) that IDO drops,
 * while the game keeps the division live for it; a never-true unsigned test gave it, later
 * written as a real s32 clamp (a kept FAKE, 2026-10-17). */
typedef struct Tlb325MoverW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
} Tlb325MoverW;

void btaiSunaEfTrail(Obj *o) {
    Tlb325MoverW *w = (Tlb325MoverW *) FX(o);
    s32 n;

    if (gGameFrameCount % 5 == 0) {
        n = (s32) (w->vel[2] * w->vel[2] + w->vel[0] * w->vel[0] + 100.0f) / 10;
        /* n is clamped and never read: the clamp keeps the game's div (its mflo goes). */
        if (n < 0) {
            n = 0;
        }
        (*sunaEfMake)(w->frame->pos[0], -500.0f, w->frame->pos[2] + 50.0f, w->vel[0], w->vel[2],
                                  3.0f, 255);
    }
}

/* Two points of this file's data (set by tlb_0324_0, _1 and _3): the third "SUNAef" stands a
 * fifth of the way from the first to the second in x and z, half the way in y. */
extern f32 btaiWhirlPoint1X; /* the first point's x, y, z */
extern f32 btaiWhirlPoint1Y;
extern f32 btaiWhirlPoint1Z;
extern f32 btaiWhirlPoint2; /* the second point's x, y, z */
extern f32 btaiWhirlPoint2Y;
extern f32 btaiWhirlPoint2Z;

/* The third "SUNAef" (sand dust): a lasting puff between the two points (never past z -750),
 * fading in to 255 and then out, growing. */
s32 btaiSunaEf3Proc(Obj *obj, s16 msg, void *arg) {
    SunaW *w = (SunaW *) (obj + 1);

    switch (msg) {
        case 2:
            obj->name = "SUNAef";
            PRINTF("砂ぼこり\n");
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) btaiSandDustDl, 0x123);
            fadeAnimStart(&w->anim, btaiSandDustAnim, 0, &w->frame->segs[3], floadGetSetFile(0x123, 0));
            w->frame->flags |= 7;
            w->frame->scale[2] = 2;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = (btaiWhirlPoint2 - btaiWhirlPoint1X) / 5.0f + btaiWhirlPoint1X;
            w->frame->pos[1] = (btaiWhirlPoint2Y - btaiWhirlPoint1Y) / 2 + btaiWhirlPoint1Y;
            w->frame->pos[2] = (btaiWhirlPoint2Z - btaiWhirlPoint1Z) / 5.0f + btaiWhirlPoint1Z;
            w->time = 128;
            w->dy = -50.0f;
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(0xD1, 0xCC, 0x93, 0xB7, 0xB2, 0x7F, w->time);
            break;
        case 0:
            if (btaiEffectsEnd == 1) {
                return -1;
            }
            if (gGameFrameCount % 3 == 0) {
                fadeAnimStep(&w->anim);
            }
            if (w->fadeOut == 0) {
                w->time++;
                if (w->time == 255) {
                    w->fadeOut = 1;
                }
            } else {
                w->time--;
                if (w->time <= 0) {
                    w->time = 0;
                    return -1;
                }
            }
            w->frame->scale[0] += 0.1;
            w->frame->scale[1] += 0.1;
            w->frame->scale[2] += 0.1;
            w->frame->pos[0] = (btaiWhirlPoint2 - btaiWhirlPoint1X) / 5.0f + btaiWhirlPoint1X;
            w->frame->pos[1] = (btaiWhirlPoint2Y - btaiWhirlPoint1Y) / 2 + btaiWhirlPoint1Y + w->dy;
            w->frame->pos[2] = (btaiWhirlPoint2Z - btaiWhirlPoint1Z) / 5.0f + btaiWhirlPoint1Z;
            if (w->frame->pos[2] > -750.0f) {
                w->frame->pos[2] = -750.0f;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 btaiSunaEf3Proc(Obj *obj, s16 msg, void *arg);

/* Starts a btaiSunaEf3Proc task under the battle's task, in the battle task's file. */
Obj *taisambaCreateTask(void) {
    return tskCreate(gBattleTask, btaiSunaEf3Proc, gBattleTask->file, NULL);
}

/* The second "SUNAef" (sand dust): a puff at a place, standing still, its animation stepped
 * twice a frame, until its count runs out. Its count is never set from the argument. */
s32 btaiSunaEf2Proc(Obj *obj, s16 msg, void *arg) {
    SunaW *w = (SunaW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "SUNAef";
            PRINTF("砂ぼこり\n");
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) btaiSandDustDl, 0x123);
            fadeAnimStart(&w->anim, btaiSandDustAnim, 0, &w->frame->segs[3], floadGetSetFile(0x123, 0));
            w->frame->flags |= 7;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = ((SunaArg *) arg)->scale;
            w->frame->pos[0] = ((SunaArg *) arg)->pos[0];
            w->frame->pos[1] = ((SunaArg *) arg)->pos[1];
            w->frame->pos[2] = ((SunaArg *) arg)->pos[2];
            w->time = 255;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(0xD1, 0xCC, 0x93, 0xB7, 0xB2, 0x7F, w->time);
            break;
        case 0:
            if (btaiEffectsEnd == 1) {
                return -1;
            }
            fadeAnimStep(&w->anim);
            fadeAnimStep(&w->anim);
            w->count--;
            if (w->count <= 0) {
                w->count = 0;
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 btaiSunaEf2Proc(Obj *obj, s16 msg, void *arg);

/* A sand-dust puff at (x, y - 850, z), drifting by a fifth of (dx, 0, dz), under the battle's task. */

void btaiSunaEf2Make(f32 x, f32 y, f32 z, f32 dx, f32 dz, f32 scale, s32 time) {
    s32 pad;
    SunaArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y + -850.0f;
    arg.pos[2] = z;
    arg.vel[1] = 0.0f;
    arg.vel[0] = dx * 0.2;
    arg.vel[2] = dz * 0.2;
    arg.scale = scale;
    arg.time = time;
    tskCreate(gBattleTask, btaiSunaEf2Proc, gBattleTask->file, &arg);
}

/* The "KOKEef" task's creation argument. */
typedef struct KokeArg {
    /* 0x00 */ u8 pad00[0x44];
    /* 0x44 */ f32 pos[3];
    /* 0x50 */ u8 pad50[0x10];
} KokeArg; /* size 0x60 */

/* The "KOKEef" (moss dust) task's work: three puffs. */
typedef struct KokeW {
    /* 0x00 */ PolFrame *frames[3];
    /* 0x0C */ f32 vel[3][3]; /* [axis][puff] */
    /* 0x30 */ u8 pad30[4];
    /* 0x34 */ Anim anim; /* the three puffs' one texture animation */
    /* 0x44 */ u8 pad44[0x10];
    /* 0x54 */ s32 alpha;
} KokeW;

/* "KOKEef" (moss dust): three puffs at a place, thrown up and out at 5 a frame (away from the
 * origin and to both sides), falling back, growing and fading. */
s32 btaiKokeEfProc(Obj *obj, s16 msg, void *arg) {
    KokeW *w = (KokeW *) (obj + 1);
    KokeArg *a = arg;
    s32 i;
    f32 angle;

    switch (msg) {
        case 2:
            obj->name = "KOKEef";
            PRINTF("こけぼこり\n");
            for (i = 0; i < 3; i++) {
                w->frames[i] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
                if (w->frames[i] == NULL) {
                    return -1;
                }
                bossSetPolDL(w->frames[i], (Gfx *) btaiSandDustDl, 0x123);
                fadeAnimStart(&w->anim, btaiSandDustAnim, 0, &w->frames[i]->segs[3], floadGetSetFile(0x123, 0));
                w->frames[i]->flags |= 7;
                w->frames[i]->scale[2] = 4.0f;
                w->frames[i]->scale[0] = w->frames[i]->scale[1] = w->frames[i]->scale[2];
                w->frames[i]->pos[0] = a->pos[0];
                w->frames[i]->pos[1] = a->pos[1];
                w->frames[i]->pos[2] = a->pos[2];
                w->vel[1][i] = 4.0f;
                w->alpha = 255;
            }
            angle = mathAtan2(-a->pos[0], -a->pos[2]);
            w->vel[2][0] = -cosf(angle + 3.1415927f) * 5.0f;
            w->vel[0][0] = -sinf(angle + 3.1415927f) * 5.0f;
            w->vel[2][1] = -cosf(angle + 1.5707964f) * 5.0f;
            w->vel[0][1] = -sinf(angle + 1.5707964f) * 5.0f;
            w->vel[2][2] = -cosf(angle + 4.712389f) * 5.0f;
            w->vel[0][2] = -sinf(angle + 4.712389f) * 5.0f;
            break;
        case 1:
            for (i = 0; i < 3; i++) {
                if (w->frames[i] != NULL) {
                    frmDestroy(&w->frames[i]->f);
                }
            }
            PRINTF("砂ぼこり\n");
            break;
        case 3:
            for (i = 0; i < 3; i++) {
                w->frames[i]->dl = bossPrimEnvColorDL(0xA2, 0x98, 0x84, 0x8E, 0x84, 0x7A, w->alpha);
            }
            break;
        case 0:
            if (btaiEffectsEnd == 1) {
                return -1;
            }
            if (gGameFrameCount % 3 == 0) {
                fadeAnimStep(&w->anim);
            }
            if (w->alpha > 160) {
                w->alpha -= 1;
            } else {
                w->alpha -= 2;
            }
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            for (i = 0; i < 3; i++) {
                w->vel[1][i] -= 0.05;
                w->frames[i]->scale[0] += 0.2;
                w->frames[i]->scale[1] += 0.2;
                w->frames[i]->scale[2] += 0.2;
                w->frames[i]->pos[0] += w->vel[0][i];
                w->frames[i]->pos[1] += w->vel[1][i];
                w->frames[i]->pos[2] += w->vel[2][i];
                if (w->frames[i]->pos[2] > -750.0f) {
                    w->frames[i]->pos[2] = -750.0f;
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "KOKEef" puff at (x, y - 850, z), under the battle's task. */
void btaiKokeEfMake(f32 x, f32 y, f32 z) {
    KokeArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y + -850.0f;
    arg.pos[2] = z;
    tskCreate(gBattleTask, btaiKokeEfProc, gBattleTask->file, &arg);
}

/* rand(n), -n .. n (tlb_0324_3). */
#define btaiRandSigned (*btaiRandSigned)

/* The "uzuSNA" (whirlpool sand) task's work: three puffs. */
typedef struct UzuSnaW {
    /* 0x00 */ PolFrame *frames[3];
    /* 0x0C */ f32 vel[3][3]; /* [axis][puff] */
    /* 0x30 */ u8 pad30[4];
    /* 0x34 */ Anim anim; /* the three puffs' one texture animation */
    /* 0x44 */ u8 pad44[4];
    /* 0x48 */ f32 speed;
    /* 0x4C */ s32 alpha;
    /* 0x50 */ s32 time;
} UzuSnaW;

/* "uzuSNA" (whirlpool sand): three sand puffs scattered around a point 200 behind the second
 * point (seen from the first), drawn to it faster and faster, growing; they fade in for 85
 * frames, then out. */
s32 btaiUzuSnaProc(Obj *obj, s16 msg, void *arg) {
    UzuSnaW *w = (UzuSnaW *) (obj + 1);
    s32 i;
    f32 x;
    f32 z;
    f32 angle;
    s32 pad;

    angle = mathAtan2(btaiWhirlPoint1X - btaiWhirlPoint2, btaiWhirlPoint1Z - btaiWhirlPoint2Z);
    x = btaiWhirlPoint2 - cosf(angle + 3.1415927f) * 200.0f;
    z = btaiWhirlPoint2Z - sinf(angle + 3.1415927f) * 200.0f + 200.0f;
    switch (msg) {
        case 2:
            obj->name = "uzuSNA";
            w->time = 150;
            w->speed = 0.0f;
            w->alpha = 32;
            for (i = 0; i < 3; i++) {
                w->frames[i] = (PolFrame *) frmCreateNext(gFrmLayer8, gFrmKindPol);
                if (w->frames[i] == NULL) {
                    return -1;
                }
                bossSetPolDL(w->frames[i], (Gfx *) btaiSandDustDl, 0x123);
                fadeAnimStart(&w->anim, btaiSandDustAnim, 0, &w->frames[i]->segs[3], floadGetSetFile(0x123, 0));
                w->frames[i]->flags |= 0x27;
                w->frames[i]->scale[2] = 0.3f;
                w->frames[i]->scale[0] = w->frames[i]->scale[1] = w->frames[i]->scale[2];
                w->frames[i]->pos[0] = btaiRandSigned(900) + x;
                w->frames[i]->pos[1] = -850.0f;
                w->frames[i]->pos[2] = btaiRand(800) + z;
                w->vel[0][i] = 0.0f;
                w->vel[1][i] = 0.0f;
                w->vel[2][i] = 0.0f;
            }
            break;
        case 1:
            for (i = 0; i < 3; i++) {
                if (w->frames[i] != NULL) {
                    frmDestroy(&w->frames[i]->f);
                }
            }
            break;
        case 3:
            for (i = 0; i < 3; i++) {
                w->frames[i]->dl = bossPrimEnvColorDL(0xD1, 0xCC, 0x93, 0xB7, 0xB2, 0x7F, w->alpha);
            }
            break;
        case 0:
            if (btaiEffectsEnd == 1) {
                return -1;
            }
            if (gGameFrameCount % 3 == 0) {
                fadeAnimStep(&w->anim);
            }
            w->time--;
            if (w->time > 64) {
                w->alpha += 2;
            } else {
                w->alpha -= 2;
            }
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            if (w->alpha > 255) {
                w->alpha = 255;
            }
            for (i = 0; i < 3; i++) {
                w->speed += 0.3;
                if (w->speed > 20.0f) {
                    w->speed = 20.0f;
                }
                angle = mathAtan2(x - w->frames[i]->pos[0], z - w->frames[i]->pos[2]);
                w->vel[2][i] = -cosf(angle + 3.1415927f) * w->speed;
                w->vel[0][i] = -sinf(angle + 3.1415927f) * w->speed;
                w->frames[i]->scale[0] += 0.2;
                w->frames[i]->scale[1] += 0.2;
                w->frames[i]->scale[2] += 0.2;
                w->frames[i]->pos[0] += w->vel[0][i];
                w->frames[i]->pos[1] += w->vel[1][i];
                w->frames[i]->pos[2] += w->vel[2][i];
                if (w->frames[i]->pos[2] > -750.0f) {
                    w->frames[i]->pos[2] = -750.0f;
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts a btaiUzuSnaProc task under the battle's task, in the battle task's file. */

void btaiUzuSnaMake(void) {
    tskCreate(gBattleTask, btaiUzuSnaProc, gBattleTask->file, NULL);
}

/* The "Tbubble" task's creation argument. */
typedef struct TbubbleArg {
    /* 0x00 */ u8 pad00[0x10];
    /* 0x10 */ f32 vx;
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ f32 vz;
    /* 0x1C */ f32 scale; /* 0.1 when not positive */
    /* 0x20 */ u8 pad20[4];
    /* 0x24 */ f32 pos[3];
    /* 0x30 */ u8 pad30[0xC];
} TbubbleArg; /* size 0x3C */

/* The "Tbubble" task's work. */
typedef struct TbubbleW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 wobX; /* the wobble added to x and z */
    /* 0x08 */ f32 vy;
    /* 0x0C */ f32 wobZ;
    /* 0x10 */ f32 vx; /* slowing by 6% a frame */
    /* 0x14 */ f32 unused14; /* set 0.0f in msg 2 between vx and vz, never read */
    /* 0x18 */ f32 vz;
    /* 0x1C */ f32 scale;
    /* 0x20 */ u8 pad20[4];
    /* 0x24 */ f32 x;
    /* 0x28 */ f32 y;
    /* 0x2C */ f32 z;
    /* 0x30 */ s32 alpha; /* four times the drawn one */
    /* 0x34 */ s32 time;
} TbubbleW;

extern u8 btaiTbubbleDl[]; /* the bubble, in file 0xF8 */

/* "Tbubble": a bubble from file 0xF8 (smaller the farther back it starts) that rises faster and
 * faster up to 3 times its scale a frame, drifts and wobbles, pulses, and fades. */
s32 btaiTbubbleProc(Obj *obj, s16 msg, void *arg) {
    TbubbleW *w = (TbubbleW *) (obj + 1);
    TbubbleArg *a = arg;
    s32 pad[4];

    switch (msg) {
        case 2:
            obj->name = "Tbubble";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) btaiTbubbleDl, 0xF8);
            w->frame->flags |= 0x20;
            w->frame->flags |= 7;
            w->frame->scale[2] = 1.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->alpha = 1020;
            w->time = 0;
            w->wobX = 0.0f;
            w->vy = 0.0f;
            w->wobZ = 0.0f;
            w->vx = a->vx;
            w->unused14 = 0.0f;
            w->vz = a->vz;
            w->frame->pos[0] = w->x = a->pos[0];
            w->frame->pos[1] = w->y = a->pos[1];
            w->frame->pos[2] = w->z = a->pos[2];
            w->scale = a->scale - w->frame->pos[2] / 4000.0f;
            w->frame->scale[0] = w->scale;
            w->frame->scale[1] = w->scale;
            w->frame->scale[2] = w->scale;
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 3:
            w->frame->dl = bossEnvColorDL(0xFF, 0xFF, 0xFF, w->alpha / 4);
            break;
        case 0:
            if (btaiEffectsEnd == 1) {
                return -1;
            }
            w->time++;
            w->alpha -= 6;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            w->vy += 0.1;
            if (w->vy > w->scale * 3.0f) {
                w->vy = w->scale * 3.0f;
            }
            if (((w->vx > 0.0f) ? w->vx : -w->vx) > 0.0f) {
                w->vx *= 0.94;
            }
            if (((w->vz > 0.0f) ? w->vz : -w->vz) > 0.0f) {
                w->vz *= 0.94;
            }
            w->wobZ = (sinf(w->time * 2 * 0.0174532930038800785) + sinf(w->time * 2 * 0.0174532930038800785)) * 34.0f;
            w->wobX = (sinf(w->time * 3 * 0.0174532930038800785) + sinf(w->time * 0.0174532930038800785)) * 34.0f;
            w->frame->scale[1] = (sinf(w->time * 7) * 0.2 + 0.9) * w->scale;
            w->frame->scale[0] = (sinf(w->time * 7 + 1.5707964f) * 0.1 + 1.0) * w->scale;
            w->frame->pos[0] = w->x + w->wobX;
            w->frame->pos[1] += w->vy;
            w->frame->pos[2] = w->z + w->wobZ;
            w->x += w->vx;
            w->z += w->vz;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Tbubble" under parent at (x, y, z), drifting by (vx, vz), with its bubble sound (0x52A)
 * panned from the battle camera. */
void btaiTbubbleMake(Obj *parent, f32 x, f32 y, f32 z, f32 vx, f32 vz, f32 scale) {
    TbubbleArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.vx = vx;
    arg.vz = vz;
    if (scale > 0.0f) {
        arg.scale = scale;
    } else {
        arg.scale = 0.1f;
    }
    sndRequest(0x52A, soundPanGet(gBattleCamView, 300.0f, -200.0f, gBattleCamView->fovy), 0xC0);
    tskCreate(parent, btaiTbubbleProc, parent->file, &arg);
}
#define btaiTbubbleMake (*btaiTbubbleMake)

/* A "Tbubble" under parent at a random x (-2000 .. 1999), y -600, z (-2499 .. -500). */
void btaiTbubbleMakeRand(Obj *parent) {
    f32 x = btaiRand(4000) - 2000;
    s32 pad;

    btaiTbubbleMake(parent, x, -600.0f, -500 - btaiRand(2000), 0.0f, 0.0f, 1.5f);
}

/* The three-bubble tasks' creation argument ("T3bubble", "Tsbubble", "U3bubble"): per bubble
 * a velocity and a position, stored axis by axis. */
typedef struct Bubble3Arg {
    /* 0x00 */ u8 pad00[0xC];
    /* 0x0C */ f32 vel[3][3]; /* [axis][bubble] */
    /* 0x30 */ f32 pos[3][3]; /* [axis][bubble] */
    /* 0x54 */ u8 pad54[0xC];
} Bubble3Arg; /* size 0x60 */

/* The three-bubble tasks' work: the argument's layout, then the alpha and the time. */
typedef struct Bubble3W {
    /* 0x00 */ PolFrame *frames[3];
    /* 0x0C */ f32 vel[3][3]; /* [axis][bubble] */
    /* 0x30 */ f32 pos[3][3]; /* [axis][bubble], before the wobble */
    /* 0x54 */ s32 alpha; /* four times the drawn one */
    /* 0x58 */ s32 time;
} Bubble3W;

/* "T3bubble": three bubbles from file 0xF8 (smaller the farther back they start) that rise
 * faster and faster, drift and wobble, pulse, and fade. */
s32 btaiT3bubbleProc(Obj *obj, s16 msg, void *arg) {
    Bubble3W *w = (Bubble3W *) (obj + 1);
    s32 i;
    f32 wobX;
    f32 wobZ;
    f32 vy;
    Bubble3Arg *a = arg;
    s32 pad[5];

    switch (msg) {
        case 2:
            obj->name = "T3bubble";
            w->alpha = 1020;
            w->time = 0;
            for (i = 0; i < 3; i++) {
                w->frames[i] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
                if (w->frames[i] == NULL) {
                    return -1;
                }
                bossSetPolDL(w->frames[i], (Gfx *) btaiTbubbleDl, 0xF8);
                w->frames[i]->flags |= 0x20;
                w->frames[i]->flags |= 7;
                w->vel[0][i] = a->vel[0][i];
                w->vel[2][i] = a->vel[2][i];
                w->frames[i]->pos[0] = w->pos[0][i] = a->pos[0][i];
                w->frames[i]->pos[1] = w->pos[1][i] = a->pos[1][i];
                w->frames[i]->pos[2] = w->pos[2][i] = a->pos[2][i];
                w->frames[i]->scale[2] = w->frames[i]->scale[1] = w->frames[i]->scale[0] =
                    (btaiRand(31) + 10) / 10 - w->frames[i]->pos[2] / 4000.0f;
            }
            break;
        case 1:
            for (i = 0; i < 3; i++) {
                if (w->frames[i] != NULL) {
                    frmDestroy(&w->frames[i]->f);
                }
            }
            break;
        case 3:
            for (i = 0; i < 3; i++) {
                if (w->frames[i] != NULL) {
                    w->frames[i]->dl = bossEnvColorDL(0xFF, 0xFF, 0xFF, w->alpha / 4);
                }
            }
            break;
        case 0:
            if (btaiEffectsEnd == 1) {
                return -1;
            }
            w->time++;
            w->alpha -= 8;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            vy = w->time * 0.1;
            for (i = 0; i < 3; i++) {
                if (w->frames[i] == NULL) {
                    return -1;
                }
                if (vy > w->frames[i]->scale[2] * 3.0f) {
                    vy = w->frames[i]->scale[2] * 3.0f;
                }
                if (((w->vel[0][i] > 0.0f) ? w->vel[0][i] : -w->vel[0][i]) > 0.0f) {
                    w->vel[0][i] *= 0.94;
                }
                if (((w->vel[2][i] > 0.0f) ? w->vel[2][i] : -w->vel[2][i]) > 0.0f) {
                    w->vel[2][i] *= 0.94;
                }
                wobZ = (sinf(w->time * 2 * 0.0174532930038800785) + sinf(w->time * 3 * 0.0174532930038800785)) * 50.0f;
                wobX = (sinf(w->time * 3 * 0.0174532930038800785) + sinf(w->time * 0.0174532930038800785)) * 50.0f;
                w->frames[i]->scale[1] = w->frames[i]->scale[2] * (sinf(w->time * 7) * 0.2 + 0.9);
                w->frames[i]->scale[0] = w->frames[i]->scale[2] * (sinf(w->time * 7 + 1.5707964f) * 0.1 + 1.0);
                w->frames[i]->pos[0] = w->pos[0][i] + wobX;
                w->frames[i]->pos[1] += vy;
                w->frames[i]->pos[2] = w->pos[2][i] + wobZ;
                w->pos[0][i] += w->vel[0][i];
                w->pos[2][i] += w->vel[2][i];
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "T3bubble": three bubbles at (x, y, z), each drifting by a random x and z (-15 .. 14), with
 * the bubble sound (0x52A). */
void btaiT3bubbleMake(f32 x, f32 y, f32 z) {
    s32 i;
    Bubble3Arg arg;

    for (i = 0; i < 3; i++) {
        arg.pos[0][i] = x;
        arg.pos[1][i] = y;
        arg.pos[2][i] = z;
        arg.vel[0][i] = btaiRand(30) - 15;
        arg.vel[2][i] = btaiRand(30) - 15;
    }
    sndRequest(0x52A, soundPanGet(gBattleCamView, 300.0f, -200.0f, gBattleCamView->fovy), 0xC0);
    tskCreate(gBattleTask, btaiT3bubbleProc, gBattleTask->file, &arg);
}

/* "Tsbubble": three bigger bubbles from file 0xF8 that rise at a steady speed growing with
 * time, drift and wobble wider, pulse, and fade faster.
 * The game loads the abs tests' 0 into $f30 in the prologue and holds case 0's NULL-tested
 * frame in s0. Three changes matched it (2026-10-12, from 41 differing lines): the
 * NULL test through a local `f` (pad[5] to pad[4]); the abs compares against int `0` (a second
 * spelling of 0.0f, IDO_NOTES 365); greedy's FAKE `if (wobZ) {}` moved from the return to
 * before the switch; and a FAKE reader `if (f) {}` after wobZ (f lives across the sinf calls:
 * s0). Misses: `f32 vy = 0.0f;` (the dead store is dropped); `f` for pos[1] += vy, in case 1,
 * or through the scale lines with a sinf temp; `f32 wobZ = 0.0f;`; the FAKE as `if (wobX)` or
 * `if (vy)`; ternary 0 with outer 0.0f.
 * A kept FAKE (2026-10-17): `f32 wobZ = 0;` (the abs compares' own int spelling) in place of
 * `if (wobZ) {}`: real 13 (the zero stays in $f30 but its mtc1 moves out of the prologue); the
 * argument through a typed local set in case 2 in its place: 13. */
s32 btaiTsbubbleProc(Obj *obj, s16 msg, void *arg) {
    Bubble3W *w = (Bubble3W *) (obj + 1);
    s32 i;
    f32 wobX;
    f32 wobZ;
    f32 vy;
    Bubble3Arg *a = arg;
    PolFrame *f;
    s32 pad[4];

    if (wobZ) {} /* FAKE: puts the abs compares' 0 in $f30 from the prologue (empties) */
    switch (msg) {
        case 2:
            obj->name = "Tsbubble";
            w->alpha = 1020;
            w->time = 0;
            for (i = 0; i < 3; i++) {
                w->frames[i] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
                if (w->frames[i] == NULL) {
                    return -1;
                }
                bossSetPolDL(w->frames[i], (Gfx *) btaiTbubbleDl, 0xF8);
                w->frames[i]->flags |= 0x20;
                w->frames[i]->flags |= 7;
                w->vel[0][i] = a->vel[0][i];
                w->vel[2][i] = a->vel[2][i];
                w->vel[1][i] = a->vel[1][i];
                w->frames[i]->pos[0] = w->pos[0][i] = a->pos[0][i];
                w->frames[i]->pos[1] = w->pos[1][i] = a->pos[1][i];
                w->frames[i]->pos[2] = w->pos[2][i] = a->pos[2][i];
                w->frames[i]->scale[2] = w->frames[i]->scale[1] = w->frames[i]->scale[0] =
                    (btaiRand(21) + 20) / 10 - w->frames[i]->pos[2] / 4000.0f;
            }
            break;
        case 1:
            for (i = 0; i < 3; i++) {
                if (w->frames[i] != NULL) {
                    frmDestroy(&w->frames[i]->f);
                }
            }
            break;
        case 3:
            for (i = 0; i < 3; i++) {
                w->frames[i]->dl = bossEnvColorDL(0xFF, 0xFF, 0xFF, w->alpha / 4);
            }
            break;
        case 0:
            if (btaiEffectsEnd == 1) {
                return -1;
            }
            w->time++;
            w->alpha -= 20;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            vy = w->time * 0.2;
            for (i = 0; i < 3; i++) {
                f = w->frames[i];
                if (f == NULL) {
                    return -1;
                }
                if (((w->vel[0][i] > 0) ? w->vel[0][i] : -w->vel[0][i]) > 0) {
                    w->vel[0][i] *= 0.94;
                }
                if (((w->vel[2][i] > 0) ? w->vel[2][i] : -w->vel[2][i]) > 0) {
                    w->vel[2][i] *= 0.94;
                }
                wobZ = (sinf(w->time * 3 * 0.0174532930038800785) + sinf(w->time * 4 * 0.0174532930038800785)) * 70.0f;
                if (f) {} /* FAKE: f lives across the sinf calls, so the NULL test takes s0 */
                wobX = (sinf(w->time * 5 * 0.0174532930038800785) + sinf(w->time * 2 * 0.0174532930038800785)) * 70.0f;
                w->frames[i]->scale[1] = w->frames[i]->scale[2] * (sinf(w->time * 7) * 0.2 + 0.9);
                w->frames[i]->scale[0] = w->frames[i]->scale[2] * (sinf(w->time * 7 + 1.5707964f) * 0.1 + 1.0);
                w->frames[i]->pos[0] = w->pos[0][i] + wobX;
                w->frames[i]->pos[2] = w->pos[2][i] + wobZ;
                w->frames[i]->pos[1] += vy;
                w->pos[0][i] += w->vel[0][i];
                w->pos[2][i] += w->vel[2][i];
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Tsbubble": three bubbles at (x, y, z), each drifting by a random x and z (-15 .. 14), with
 * the bubble sound (0x52A) at (x, y, z). */
void btaiTsbubbleMake(f32 x, f32 y, f32 z) {
    s32 i;
    Bubble3Arg arg;

    for (i = 0; i < 3; i++) {
        arg.pos[0][i] = x;
        arg.pos[1][i] = y;
        arg.pos[2][i] = z;
        arg.vel[0][i] = btaiRand(30) - 15;
        arg.vel[2][i] = btaiRand(30) - 15;
        arg.vel[1][i] = 0.0f;
    }
    SoundLocation2(0x52A, gBattleCamView, x, y, z, 500.0f, 6500.0f, gBattleCamView->fovy);
    tskCreate(gBattleTask, btaiTsbubbleProc, gBattleTask->file, &arg);
}

/* "U3bubble": three bubbles from file 0xF8 that rise faster and faster (up to 30 a frame),
 * drift in x and y, circle 80 round their path, pulse, and fade. */
s32 btaiU3bubbleProc(Obj *obj, s16 msg, void *arg) {
    Bubble3W *w = (Bubble3W *) (obj + 1);
    s32 i;
    f32 wobX;
    f32 wobY;
    f32 vz;
    f32 scale;
    Bubble3Arg *a = arg;
    s32 pad[5];

    switch (msg) {
        case 2:
            obj->name = "U3bubble";
            w->alpha = 512;
            w->time = 0;
            for (i = 0; i < 3; i++) {
                w->frames[i] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
                if (w->frames[i] == NULL) {
                    return -1;
                }
                bossSetPolDL(w->frames[i], (Gfx *) btaiTbubbleDl, 0xF8);
                scale = (btaiRand(21) + 20) / 10;
                w->frames[i]->flags |= 0x20;
                w->frames[i]->flags |= 7;
                w->frames[i]->scale[0] = scale;
                w->frames[i]->scale[1] = scale;
                w->frames[i]->scale[2] = scale;
                w->vel[0][i] = a->vel[0][i];
                w->vel[1][i] = a->vel[1][i];
                w->vel[2][i] = a->vel[2][i];
                w->frames[i]->pos[0] = w->pos[0][i] = a->pos[0][i];
                w->frames[i]->pos[1] = w->pos[1][i] = a->pos[1][i];
                w->frames[i]->pos[2] = w->pos[2][i] = a->pos[2][i];
            }
            break;
        case 1:
            for (i = 0; i < 3; i++) {
                if (w->frames[i] != NULL) {
                    frmDestroy(&w->frames[i]->f);
                }
            }
            break;
        case 3:
            for (i = 0; i < 3; i++) {
                w->frames[i]->dl = bossEnvColorDL(0xFF, 0xFF, 0xFF, w->alpha / 4);
            }
            break;
        case 0:
            if (btaiEffectsEnd == 1) {
                return -1;
            }
            w->time++;
            w->alpha -= 20;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            vz = w->time * 2 + 20;
            for (i = 0; i < 3; i++) {
                if (w->frames[i] == NULL) {
                    return -1;
                }
                if (vz > 30.0f) {
                    vz = 30.0f;
                }
                if (((w->vel[0][i] > 0.0f) ? w->vel[0][i] : -w->vel[0][i]) > 0.0f) {
                    w->vel[0][i] *= 0.94;
                }
                if (((w->vel[1][i] > 0.0f) ? w->vel[1][i] : -w->vel[1][i]) > 0.0f) {
                    w->vel[1][i] *= 0.94;
                }
                wobX = sinf(w->time * 16 * 0.0174532930038800785) * 80.0f;
                wobY = cosf(w->time * 16 * 0.0174532930038800785) * 80.0f;
                w->frames[i]->scale[1] = w->frames[i]->scale[2] * (sinf(w->time * 7) * 0.2 + 0.9);
                w->frames[i]->scale[0] = w->frames[i]->scale[2] * (sinf(w->time * 7 + 1.5707964f) * 0.1 + 1.0);
                w->frames[i]->pos[0] = w->pos[0][i] + wobX;
                w->frames[i]->pos[1] = w->pos[1][i] + wobY;
                w->frames[i]->pos[2] += vz;
                w->pos[0][i] += w->vel[0][i];
                w->pos[1][i] += w->vel[1][i];
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "U3bubble": three bubbles at (x, y, z), each rising or sinking at random (-15 .. 14), with
 * the bubble sound (0x52A) at (x, y, z). */
void btaiU3bubbleMake(f32 x, f32 y, f32 z) {
    s32 i;
    Bubble3Arg arg;

    for (i = 0; i < 3; i++) {
        arg.pos[0][i] = x;
        arg.pos[1][i] = y;
        arg.pos[2][i] = z;
        arg.vel[0][i] = btaiRand(30) - 15;
        arg.vel[1][i] = btaiRand(30) - 15;
    }
    SoundLocation2(0x52A, gBattleCamView, x, y, z, 500.0f, 6500.0f, gBattleCamView->fovy);
    tskCreate(gBattleTask, btaiU3bubbleProc, gBattleTask->file, &arg);
}

/* The whirlpools' state, in this file's data. */
extern s32 btaiWhirl1FadeCount; /* the first whirlpool's count where it fades out */
extern f32 btaiWhirl1Scale; /* its pieces' scale */
extern s32 btaiWhirl1Count; /* its count: angle in 5 degrees, fades in below 128 */
extern s32 btaiWhirl1Life; /* its pieces' life */
extern f32 btaiWhirl2Scale; /* the second whirlpool's pieces' scale */
extern f32 btaiWhirl2Turn; /* its turning speed, in degrees */
extern f32 btaiWhirl1Turn; /* the first's turning speed, in degrees */
extern f32 btaiWhirl1Grow; /* the first's pieces' growth */
extern s32 btaiWhirl2Count; /* the second's count: angle in 5 degrees, fades in below 128, out above 260 */
extern s32 btaiWhirlPiecesEnd; /* tlb_0324_4's: 1 ends the pieces */

/* An "uzu" (whirlpool) piece's work. */
typedef struct UzuW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[4];
    /* 0x08 */ f32 vy;
    /* 0x0C */ u8 pad0C[4];
    /* 0x10 */ f32 pos[3];
    /* 0x1C */ u8 pad1C[0xC];
    /* 0x28 */ f32 scale;
    /* 0x2C */ s32 alpha;
    /* 0x30 */ s32 time;
} UzuW;

extern u8 btaiUzu1Dl[]; /* the whirlpool piece, in file 0xF8 */

/* An "uzu" (whirlpool) piece of the first whirlpool: 60 out from the second point, at the
 * whirlpool's angle; it turns with it, rises faster and faster and grows, fading in its last 16
 * frames. */
s32 btaiUzu1Proc(Obj *obj, s16 msg, void *arg) {
    UzuW *w = (UzuW *) (obj + 1);
    s32 a;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "uzu";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) btaiUzu1Dl, 0xF8);
            w->frame->flags |= 0x20;
            w->pos[0] = w->frame->pos[0] =
                sinf(btaiWhirl1Count * 0.0174532930038800785 * 5.0) * 60.0f + btaiWhirlPoint2;
            w->pos[1] = w->frame->pos[1] = btaiWhirlPoint2Y - 50.0f;
            w->pos[2] = w->frame->pos[2] =
                cosf(btaiWhirl1Count * 0.0174532930038800785 * 5.0) * 60.0f + btaiWhirlPoint2Z;
            w->scale = btaiWhirl1Scale;
            w->frame->scale[0] = w->scale;
            w->frame->scale[2] = w->scale;
            w->frame->scale[1] = 1.0f;
            w->time = 0;
            w->alpha = 255;
            w->vy = 8.0f;
            if (btaiWhirl1Count < 128) {
                w->alpha = btaiWhirl1Count * 2;
            }
            if (btaiWhirl1FadeCount < btaiWhirl1Count) {
                w->alpha = btaiWhirl1FadeCount - btaiWhirl1Count + 256;
            }
            if (w->alpha <= 0) {
                w->alpha = 0;
                PRINTF("??alpha %d\n", w->alpha);
                return -1;
            }
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(0xC8, 0xF0, 0xFF, 0xBE, 0xE6, 0xFF, w->alpha);
            break;
        case 0:
            if (btaiEffectsEnd == 1) {
                return -1;
            }
            w->time++;
            w->frame->rot[1] += btaiWhirl1Turn * 0.0174532930038800785 / 3.0;
            if (w->frame->rot[1] > 6.2831855f) {
                w->frame->rot[1] -= 6.2831855f;
            }
            w->frame->pos[1] += btaiWhirl1Turn * 1.8;
            w->frame->pos[1] += w->vy;
            if (w->time > btaiWhirl1Life) {
                PRINTF("%d %d\n", w->time, btaiWhirl1Life);
                return -1;
            }
            if (btaiWhirlPiecesEnd == 1) {
                return -1;
            }
            if (w->time > btaiWhirl1Life - 16) {
                a = (btaiWhirl1Life - w->time) * 16 - 1;
                if (a > 255) {
                    a = 255;
                }
                if (a < 0) {
                    a = 0;
                }
                if (a < w->alpha) {
                    w->alpha = a;
                }
            }
            w->frame->scale[2] += btaiWhirl1Grow;
            w->frame->scale[0] += btaiWhirl1Grow;
            w->frame->scale[1] += 0.03;
            w->vy += 0.2;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts a btaiUzu1Proc task under the battle's task, in the battle task's file. */

Obj *btaiUzu1Make(void) {
    return tskCreate(gBattleTask, btaiUzu1Proc, gBattleTask->file, NULL);
}

/* The whirlpool's "uzu_in" pieces' path, in this file's data. */
extern s32 btaiWhirl3Count; /* its count: the piece's angle in 2 degrees */
extern s32 btaiWhirlAlphaMax; /* the most alpha a piece starts with */
extern f32 btaiWhirlStartCentre; /* the start's centre */
extern f32 btaiWhirlStartCentreY;
extern f32 btaiWhirlStartCentreZ;
extern f32 btaiWhirlEndCentre; /* the end's centre */
extern f32 btaiWhirlEndCentreY;
extern f32 btaiWhirlEndCentreZ; /* past this z, a piece is gone */

/* The file's .data, in address order (its externs above stay as declarations). */
extern u8 btaiSandDustTex0[];
extern u8 btaiSandDustTex1[];
extern u8 btaiSandDustTex2[];
extern u8 btaiSandDustTex3[];
extern u8 btaiSandDustTex4[];
extern u8 btaiSandDustTex6[];
s32 btaiWhirl1FadeCount = 1; /* the first whirlpool's count where it fades out */
f32 btaiWhirl1Scale = 1.0f; /* its pieces' scale */
s32 btaiWhirl1Count = 1; /* its count: angle in 5 degrees, fades in below 128 */
s32 btaiWhirl1Life = 1; /* its pieces' life */
f32 btaiWhirl2Scale = 1.0f; /* the second whirlpool's pieces' scale */
f32 btaiWhirl2Turn = 1.0f; /* its turning speed, in degrees */
f32 btaiWhirl1Turn = 1.0f; /* the first's turning speed, in degrees */
f32 btaiWhirl1Grow = 1.0f; /* the first's pieces' growth */
s32 btaiWhirl3Count = 1; /* its count: the piece's angle in 2 degrees */
s32 btaiWhirl2Count = 1; /* the second's count: angle in 5 degrees, fades in below 128, out above 260 */
s32 btaiWhirlAlphaMax = 1; /* the most alpha a piece starts with */
u32 btaiWhirlAlphaMaxUnused = 0x00000001;
s32 btaiEffectsEnd = 1; /* 1: the battle's effects end (read by every effect here) */
f32 btaiWhirlPoint1X = 1.0f; /* the first point's x, y, z */
f32 btaiWhirlPoint1Y = 1.0f;
f32 btaiWhirlPoint1Z = 1.0f;
f32 btaiWhirlPoint2 = 1.0f; /* the second point's x, y, z */
f32 btaiWhirlPoint2Y = 1.0f;
f32 btaiWhirlPoint2Z = 1.0f;
f32 btaiWhirlStartCentre = 1.0f; /* the start's centre */
f32 btaiWhirlStartCentreY = 1.0f;
f32 btaiWhirlStartCentreZ = 1.0f;
f32 btaiWhirlEndCentre = 1.0f; /* the end's centre */
f32 btaiWhirlEndCentreY = 1.0f;
f32 btaiWhirlEndCentreZ = 1.0f; /* past this z, a piece is gone */
AnimFrame btaiSandDustAnim[] = { { (u32) btaiSandDustTex0, 6 }, { (u32) btaiSandDustTex1, 6 }, { (u32) btaiSandDustTex2, 6 }, { (u32) btaiSandDustTex3, 6 }, { (u32) btaiSandDustTex4, 6 }, { 0x03002D30, 6 }, { (u32) btaiSandDustTex6, 6 }, { 0x03003D30, 6 }, { 0x03004530, 6 }, { 0x00000000, 0 }, { 0x00000000, 0 } }; /* the sand dust's texture animation (file 0x123) */

/* The "uzu_in" (into the whirlpool) task's work. */
typedef struct UzuInW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3]; /* a hundredth of the way */
    /* 0x10 */ f32 from[3];
    /* 0x1C */ f32 to[3];
    /* 0x28 */ u8 pad28[4];
    /* 0x2C */ f32 rot; /* turning back by 4 degrees a frame */
    /* 0x30 */ s32 alpha;
    /* 0x34 */ u8 pad34[4];
    /* 0x38 */ s32 time;
    /* 0x3C */ s32 hit; /* 1 once the Impact's cannon shot came within 200 */
} UzuInW;

extern u8 btaiUzuInDl[]; /* the piece, in file 0xF8 */

/* "uzu_in" (into the whirlpool): a piece that moves a hundredth of the way a frame from a point
 * on a circle round the start to one round the end, turning, growing and fading in; the
 * Impact's cannon shot passing within 200 makes it swell and fade out. */
s32 btaiUzuInProc(Obj *obj, s16 msg, void *arg) {
    UzuInW *w = (UzuInW *) (obj + 1);
    f32 dx;
    f32 dz;
    s32 pad[4];

    switch (msg) {
        case 2:
            obj->name = "uzu_in";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) btaiUzuInDl, 0xF8);
            w->frame->flags |= 0x20;
            w->from[0] = w->frame->pos[0] =
                sinf(btaiWhirl3Count * 0.0174532930038800785 * 2) * 90.0f + btaiWhirlStartCentre;
            w->from[1] = w->frame->pos[1] =
                cosf(btaiWhirl3Count * 0.0174532930038800785 * 2) * 90.0f + btaiWhirlStartCentreY;
            w->frame->pos[2] = btaiWhirlStartCentreZ;
            w->from[2] = w->frame->pos[2];
            w->to[0] = sinf(btaiWhirl3Count * 0.0174532930038800785 * 1 / 2) * 200.0f + btaiWhirlEndCentre;
            w->to[1] = cosf(btaiWhirl3Count * 0.0174532930038800785 * 1) * 30.0f + btaiWhirlEndCentreY;
            w->to[2] = btaiWhirlEndCentreZ;
            w->frame->scale[0] = 0.02f;
            w->frame->scale[2] = 0.02f;
            w->frame->scale[1] = 0.02f;
            w->alpha = 0;
            w->alpha = btaiWhirl3Count / 2;
            if (w->alpha > 112) {
                w->alpha = 112;
            }
            if (btaiWhirl3Count < 600) {
                w->to[2] = -700.0f;
            }
            if (btaiWhirlAlphaMax < 112) {
                w->alpha = btaiWhirlAlphaMax;
            }
            w->vel[2] = (w->to[2] - w->from[2]) / 100.0f;
            w->vel[0] = (w->to[0] - w->from[0]) / 100.0f;
            w->vel[1] = (w->to[1] - w->from[1]) / 100.0f;
            w->time = 0;
            w->hit = 0;
            w->rot = 0.0f;
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, 255);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            if (btaiEffectsEnd == 1) {
                return -1;
            }
            w->rot -= 0.0698131720155203139;
            if (w->rot < 0.0f) {
                w->rot = 6.2831855f;
            }
            w->time++;
            if (w->hit == 1 || btaiWhirlPiecesEnd == 1) {
                w->frame->scale[1] += 0.2;
                w->frame->scale[0] += 0.2;
                w->alpha -= 4;
                if (w->alpha < 0) {
                    w->alpha = 0;
                    return -1;
                }
            } else {
                w->frame->scale[1] = w->time * 0.015f + 0.05;
                w->frame->scale[0] = w->time * 0.015f + 0.05;
                if (w->frame->pos[2] > 0.0f) {
                    w->alpha = w->frame->pos[2] / 2;
                }
                if (w->alpha > 112) {
                    w->alpha = 112;
                }
            }
            w->frame->pos[0] += w->vel[0];
            w->frame->pos[1] += w->vel[1];
            w->frame->pos[2] += w->vel[2];
            if (w->frame->pos[1] < w->to[1]) {
                w->frame->pos[1] = w->to[1];
            }
            if (w->frame->pos[2] > btaiWhirlEndCentreZ) {
                return -1;
            }
            if (w->hit == 0) {
                dx = gMegaShotPosX - w->frame->pos[0];
                dz = gMegaShotPosZ - w->frame->pos[2];
                PRINTF("んが砲が渦にあたりました----->%d \n", w->hit);
                if (dx * dx + dz * dz < 40000.0f) {
                    w->hit = 1;
                }
            }
            w->frame->scale[2] = 4.0f;
            w->frame->pos[0] *= 0.98;
            w->frame->rot[2] = w->rot;
            w->frame->rot[1] = mathAtan2(w->frame->pos[0], w->frame->pos[2]) - 3.1415927f;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts a btaiUzuInProc task under the battle's task, in the battle task's file. */

Obj *btaiUzuInMake(void) {
    return tskCreate(gBattleTask, btaiUzuInProc, gBattleTask->file, NULL);
}

/* An "uzu" (whirlpool) piece: 60 out from the second point, at the whirlpool's angle; it turns
 * with the whirlpool, rises faster and faster and grows, fading after 45 frames, gone after
 * 61. */
s32 btaiUzu2Proc(Obj *obj, s16 msg, void *arg) {
    UzuW *w = (UzuW *) (obj + 1);
    s32 a;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "uzu";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) btaiUzu1Dl, 0xF8);
            w->frame->flags |= 0x20;
            w->pos[0] = w->frame->pos[0] = sinf(btaiWhirl2Count * 0.0174532930038800785 * 5.0) * 60.0f;
            w->pos[1] = w->frame->pos[1] = btaiWhirlPoint2Y - 50.0f;
            w->pos[2] = w->frame->pos[2] =
                cosf(btaiWhirl2Count * 0.0174532930038800785 * 5.0) * 60.0f + btaiWhirlPoint2Z;
            w->scale = btaiWhirl2Scale;
            w->frame->scale[0] = w->scale;
            w->frame->scale[2] = w->scale;
            w->frame->scale[1] = 1.0f;
            w->time = 0;
            w->alpha = 255;
            w->vy = 8.0f;
            if (btaiWhirl2Count < 128) {
                w->alpha = btaiWhirl2Count * 2;
            }
            if (btaiWhirl2Count > 260) {
                w->alpha = 516 - btaiWhirl2Count;
            }
            if (w->alpha <= 0) {
                return -1;
            }
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(0xC8, 0xF0, 0xFF, 0xBE, 0xE6, 0xFF, w->alpha);
            break;
        case 0:
            if (btaiEffectsEnd == 1) {
                return -1;
            }
            w->time++;
            w->frame->rot[1] += btaiWhirl2Turn * 0.0174532930038800785 / 3.0;
            if (w->frame->rot[1] > 6.2831855f) {
                w->frame->rot[1] -= 6.2831855f;
            }
            w->frame->pos[1] += btaiWhirl2Turn * 1.8;
            w->frame->pos[1] += w->vy;
            if (w->time > 60) {
                return -1;
            }
            if (btaiWhirlPiecesEnd == 1) {
                return -1;
            }
            if (w->time >= 45) {
                a = 959 - w->time * 16;
                if (a > 255) {
                    a = 255;
                }
                if (a < 0) {
                    a = 0;
                }
                if (a < w->alpha) {
                    w->alpha = a;
                }
            }
            w->frame->scale[2] += 0.005;
            w->frame->scale[0] += 0.005;
            w->frame->scale[1] += 0.03;
            w->vy += 0.2;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 btaiUzu2Proc(Obj *obj, s16 msg, void *arg);

/* Starts a btaiUzu2Proc task under the battle's task, in the battle task's file. */

void btaiUzu2Make(void) {
    tskCreate(gBattleTask, btaiUzu2Proc, gBattleTask->file, NULL);
}

/* The "hagata" (bite marks) task's creation argument. */
typedef struct HagataArg {
    /* 0x00 */ u8 pad00[4];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ u8 pad10[8];
} HagataArg; /* size 0x18 */

/* The "hagata" (bite marks) task's work. */
typedef struct HagataW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0xC];
    /* 0x10 */ s32 alpha;
    /* 0x14 */ s32 time;
} HagataW;

extern u8 btaiHagataDl[]; /* the bite marks, in file 0xF8 */

/* "hagata" (bite marks) from file 0xF8: shows up by 20 a frame for 10 frames, then fades by 6;
 * gone after 61 frames or when faded out. */
s32 btaiHagataProc(Obj *obj, s16 msg, void *arg) {
    HagataW *w = (HagataW *) (obj + 1);
    HagataArg *a = arg;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "hagata";
            PRINTF("歯形\n");
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) btaiHagataDl, 0xF8);
            w->frame->flags |= 0x20;
            w->frame->flags |= 7;
            w->frame->pos[0] = a->pos[0];
            w->frame->pos[1] = a->pos[1];
            w->frame->pos[2] = a->pos[2];
            w->frame->scale[0] = 0.4f;
            w->frame->scale[2] = 0.4f;
            w->frame->scale[1] = 0.4f;
            w->alpha = 128;
            w->time = 0;
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, 255);
            gDPSetEnvColor(gGfxDlHead++, 0, 0, 0, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            if (btaiEffectsEnd == 1) {
                return -1;
            }
            if (w->time++ > 60) {
                return -1;
            }
            if (w->time < 10) {
                w->alpha += 20;
            } else {
                w->alpha -= 6;
            }
            if (w->alpha > 255) {
                w->alpha = 255;
            }
            if (w->alpha < 0) {
                w->alpha = 0;
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "hagata" under parent at (x, y, z + 50). */
void btaiHagataMake(Obj *parent, f32 x, f32 y, f32 z) {
    HagataArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z + 50.0f;
    tskCreate(parent, btaiHagataProc, parent->file, &arg);
}

/* The "ikasumi" (squid ink) task's creation argument. */
typedef struct IkasumiArg {
    /* 0x00 */ u8 pad00[0x18];
    /* 0x18 */ f32 pos[3];
    /* 0x24 */ f32 scale;
    /* 0x28 */ u8 pad28[8];
    /* 0x30 */ s32 time; /* how long the screen stays inked */
    /* 0x34 */ u8 pad34[8];
} IkasumiArg; /* size 0x3C */

/* The "ikasumi" (squid ink) task's work. */
typedef struct IkasumiW {
    /* 0x00 */ PolFrame *frame; /* the ink blob */
    /* 0x04 */ PolFrame *screen; /* the black over the cockpit */
    /* 0x08 */ Anim anim;
    /* 0x18 */ f32 pos[3];
    /* 0x24 */ u8 pad24[4];
    /* 0x28 */ s32 alpha; /* the blob's */
    /* 0x2C */ s32 screenAlpha;
    /* 0x30 */ s32 time;
    /* 0x34 */ s32 count;
    /* 0x38 */ u8 dark; /* the darkest the screen goes: 60, 40 or 20 by gBossPhase */
} IkasumiW;

extern s32 btaiInkGone; /* 1: the ink is gone */
extern s32 btaiAction; /* tlb_0324_4's: 13-20 end the ink */
extern u8 btaiIkasumiTintDl[]; /* the full-screen tint, in file 0xEA (beffect.c's filter) */

/* Matched 2026-10-16; it was parked for case 2's float colouring (which chain takes f0, which
 * f2) and the ugen float queue after mathAtan2's call; which the game needs is still open, so
 * an angle carrier and a never-true test (both FAKE, kept) give it. */
/* "ikasumi" (squid ink): a blob of ink thrown at the cockpit that swells after 50 frames while
 * the screen goes dark (no darker than its limit) for the ink's time, then clears. */
s32 btaiIkasumiProc(Obj *obj, s16 msg, void *arg) {
    IkasumiW *w = (IkasumiW *) (obj + 1);
    IkasumiArg *a = arg;
    f32 angle;
    f32 x;
    f32 z;

    switch (msg) {
        case 2:
            obj->name = "ikasumi";
            PRINTF("\n");
            w->frame =(PolFrame *) frmCreateNext(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) btaiSandDustDl, 0x123);
            fadeAnimStart(&w->anim, btaiSandDustAnim, 0, &w->frame->segs[3], floadGetSetFile(0x123, 0));
            w->screen = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->screen == NULL) {
                return -1;
            }
            bossSetPolDL(w->screen, (Gfx *) btaiIkasumiTintDl, 0xEA);
            bossSetFrameFlags((AnimeFrame *) w->screen, 0x20);
            w->screen->scale[2] = 0.1f;
            w->screen->scale[0] = w->screen->scale[1] = w->screen->scale[2];
            w->screen->pos[2] = 0.0f;
            w->screen->pos[0] = w->screen->pos[1] = w->screen->pos[2];
            w->frame->flags |= 0x20;
            /* FAKE: angle carries a->scale's chain so the never-true test below can read it */
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = angle = a->scale;
            w->frame->pos[0] = w->pos[0] = a->pos[0];
            w->frame->pos[1] = w->pos[1] = a->pos[1];
            w->frame->pos[2] = w->pos[2] = a->pos[2];
            z = w->frame->pos[2];
            x = w->frame->pos[0];
            /* FAKE: a never-true unsigned test (IDO_NOTES 284, shopYadoya_proc's lever). Its 14
             * reads of angle lift a->scale's range above the shared temp's (save 9.0 over 7.5), so
             * it takes f0 as in the game; its 13 adds and the trunc spend 14 float temps (two past
             * the six-register ring) and its nine integer ops the whole integer ring. as1 deletes
             * the branch and every dead op: no instruction is left. */
            if ((((((((((u32) (s32) (angle + angle + angle + angle + angle + angle + angle +
                                      angle + angle + angle + angle + angle + angle + angle) ^
                        1) + 1) ^ 1) + 1) ^ 1) + 1) ^ 1) + 1) < 0) {
                break;
            }
            angle = mathAtan2(x, z);
            w->frame->pos[2] += 100.0f;
            w->frame->pos[0] *= 0.7;
            w->count = 0;
            w->time = a->time;
            w->alpha = 255;
            w->screenAlpha = 0;
            switch (gBossPhase) {
                case 0:
                    w->dark = 60;
                    break;
                case 1:
                    w->dark = 40;
                    break;
                case 2:
                    w->dark = 20;
                    break;
            }
            break;
        case 1:
            /* one line each, the game's style (IDO_NOTES 324; tlb_0081's destroys) */
            if (w->frame != NULL) frmDestroy(&w->frame->f);
            if (w->screen != NULL) frmDestroy(&w->screen->f);
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(1, 1, 1, 1, 1, 1, w->alpha);
            w->screen->dl = bossXluPrimEnvColorDL(1, 1, 1, w->screenAlpha);
            break;
        case 0:
            if (btaiEffectsEnd == 1) {
                return -1;
            }
            if (btaiInkGone == 1) {
                return -1;
            }
            if (gBattleFlags & 1) {
                return -1;
            }
            if (btaiAction == 13 || btaiAction == 14 || btaiAction == 15 ||
                btaiAction == 16 || btaiAction == 17 || btaiAction == 18 ||
                btaiAction == 19 || btaiAction == 20) {
                return -1;
            }
            w->count++;
            w->screen->rot[0] = gCockpitRotX;
            w->screen->rot[1] = gCockpitRotY;
            w->screen->rot[2] = gCockpitRotZ;
            if (gGameFrameCount % 3 == 0) {
                fadeAnimStep(&w->anim);
            }
            if (w->count == 50) {
                w->frame->f.flags &= ~2;
            }
            if (w->count >= 50) {
                w->frame->scale[0] += 0.1;
                w->frame->scale[1] += 0.1;
                w->frame->scale[2] += 0.1;
                w->time--;
                w->alpha -= 3;
                if (w->dark < w->time) {
                    w->screenAlpha++;
                } else {
                    w->screenAlpha -= 3;
                }
                if (w->dark < w->screenAlpha) {
                    w->screenAlpha = w->dark;
                }
            } else {
                w->alpha -= 2;
                if (w->alpha < w->dark) {
                    w->alpha = w->dark;
                }
                w->frame->scale[0] += 0.8;
                w->frame->scale[1] += 0.8;
                w->frame->scale[2] += 0.8;
            }
            if (gBattleFlags & 1) {
                if (w->dark < w->time) {
                    w->time = w->dark;
                } else {
                    w->alpha -= 4;
                    w->screenAlpha -= 4;
                }
            }
            if (w->screenAlpha <= 0 && w->count >= 50) {
                w->screenAlpha = 0;
                PRINTF("ikasumi dead ----> %d\n", w->count);
                return -1;
            }
            if (gBattleFlags & 0x10) {
                return -1;
            }
            if (w->alpha <= 0) {
                PRINTF("ikasumi alpha --\n");
                w->alpha = 0;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An "ikasumi" (squid ink) at (x, y, z) and scale under the battle's task, darkening the
 * screen for time. */
void btaiIkasumiMake(f32 x, f32 y, f32 z, f32 scale, s32 time) {
    IkasumiArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.scale = scale;
    arg.time = time;
    tskCreate(gBattleTask, btaiIkasumiProc, gBattleTask->file, &arg);
}

/* The two "Tzanzou" (afterimage) tasks' creation argument: the model's place, turn and
 * animation frame. */
typedef struct TzanzouArg {
    /* 0x00 */ u8 pad00[0x10];
    /* 0x10 */ f32 pos[3];
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 frame;
    /* 0x2C */ u8 pad2C[4];
    /* 0x30 */ s32 mode; /* the second's (w->mode) */
} TzanzouArg; /* size 0x34 */

/* The "Tzanzou" (afterimage) tasks' work. */
typedef struct TzanzouW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ u8 pad04[0x28];
    /* 0x2C */ s32 alpha;
    /* 0x30 */ s32 mode; /* the second's: 1 grows faster */
} TzanzouW;

extern u8 btaiTzanzouAnim[]; /* the afterimage's model animation (file 0xF8) */

/* "Tzanzou": an afterimage, the model of file 0xF8 at the argument's place, turn and frame,
 * fading out from 80 by 10 a frame, tinted (200, 40, 20). */
s32 btaiTzanzouProc(Obj *obj, s16 msg, void *arg) {
    TzanzouW *w = (TzanzouW *) (obj + 1);
    TzanzouArg *a = arg;
    s32 pad[4];

    switch (msg) {
        case 2:
            obj->name = "Tzanzou";
            PRINTF("残像作ります\n");
            w->frame = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetAnimAt(w->frame, btaiTzanzouAnim, 0xF8, 0.0f);
            w->frame->flags |= 0x20;
            w->frame->scale[2] = 1.7f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->alpha = 80;
            w->frame->pos[0] = a->pos[0];
            w->frame->pos[1] = a->pos[1];
            w->frame->pos[2] = a->pos[2];
            w->frame->rot[0] = a->rot[0];
            w->frame->rot[1] = a->rot[1];
            w->frame->rot[2] = a->rot[2];
            w->frame->frame = a->frame;
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, 255);
            gDPSetEnvColor(gGfxDlHead++, 200, 40, 20, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            if (btaiEffectsEnd == 1) {
                return -1;
            }
            w->alpha -= 10;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Tzanzou" (btaiTzanzouProc) under parent: the model at (x, y, z), turned
 * (rx, ry, rz), at its animation's frame. */
void btaiTzanzouMake(Obj *parent, f32 x, f32 y, f32 z, f32 rx, f32 ry, f32 rz, f32 frame) {
    TzanzouArg arg;
    s32 pad1;
    s32 pad2;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.rot[0] = rx;
    arg.rot[1] = ry;
    arg.rot[2] = rz;
    arg.frame = frame;
    tskCreate(parent, btaiTzanzouProc, parent->file, &arg);
}

extern u8 btaiTzanzou2Anim[]; /* the second afterimage's model animation (file 0xF8) */
extern s32 gBossPhase; /* bmove.c's */

/* The second "Tzanzou": an afterimage of file 0xF8's other model, fading out from 70 by 9 a
 * frame, tinted (200, 40, 30), growing by (3 + 3 * gBossPhase) / 100 a frame (by 0.1
 * more in mode 1). */
s32 btaiTzanzou2Proc(Obj *obj, s16 msg, void *arg) {
    TzanzouW *w = (TzanzouW *) (obj + 1);
    TzanzouArg *a = arg;
    s32 pad[4];

    switch (msg) {
        case 2:
            obj->name = "Tzanzou";
            PRINTF("残像作ります\n");
            w->frame = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetAnimAt(w->frame, btaiTzanzou2Anim, 0xF8, 0.0f);
            w->frame->flags |= 0x20;
            w->frame->scale[2] = 1.7f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->alpha = 70;
            w->frame->pos[0] = a->pos[0];
            w->frame->pos[1] = a->pos[1];
            w->frame->pos[2] = a->pos[2];
            w->frame->rot[0] = a->rot[0];
            w->frame->rot[1] = a->rot[1];
            w->frame->rot[2] = a->rot[2];
            w->frame->frame = a->frame;
            w->mode = a->mode;
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, 255);
            gDPSetEnvColor(gGfxDlHead++, 200, 40, 30, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            if (btaiEffectsEnd == 1) {
                return -1;
            }
            w->alpha -= 9;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            if (w->mode == 1) {
                w->frame->scale[0] += 0.1;
            }
            w->frame->scale[0] += (f32) ((3.0 + gBossPhase * 3.0) / 100.0);
            w->frame->scale[1] = w->frame->scale[2] = w->frame->scale[0];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Tzanzou" (btaiTzanzou2Proc) under parent: the model at (x, y, z), turned
 * (rx, ry, rz), at its animation's frame, with n. */
void btaiTzanzou2Make(Obj *parent, f32 x, f32 y, f32 z, f32 rx, f32 ry, f32 rz, f32 frame,
                            s32 n) {
    TzanzouArg arg;
    s32 pad1;
    s32 pad2;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.rot[0] = rx;
    arg.rot[1] = ry;
    arg.rot[2] = rz;
    arg.frame = frame;
    arg.mode = n;
    tskCreate(parent, btaiTzanzou2Proc, parent->file, &arg);
}

/* The "kiran" (sparkle) task's creation argument. */
typedef struct KiranArg {
    /* 0x00 */ u8 pad00[8];
    /* 0x08 */ f32 pos[3];
    /* 0x14 */ u8 pad14[8];
} KiranArg; /* size 0x1C */

/* The "kiran" (sparkle) task's work. */
typedef struct KiranW {
    /* 0x00 */ PolFrame *frame; /* the sparkle */
    /* 0x04 */ PolFrame *glow; /* a quarter of its width */
    /* 0x08 */ u8 pad08[0xC];
    /* 0x14 */ s32 alpha;
    /* 0x18 */ s32 state; /* 0 opens, 1 glows, 2 closes */
} KiranW;

extern u8 btaiKiranDl[]; /* the sparkle, in file 0xF8 */
extern u8 btaiKiranGlowDl[]; /* its glow, in file 0xF8 */

/* "kiran" (sparkle): two frames from file 0xF8 that open wide by 16 a frame up to 224, glow
 * up to 255, then close by 20 a frame.
 * msg 1's destroys are one-line ifs (test and call on one line, no braces): that puts w's addiu
 * in the first beqz's delay slot as the game does (IDO_NOTES 324, 347). They replace the FAKE
 * layout of before (the first block and the next test on one line), a kept FAKE, 2026-10-17. */
s32 btaiKiranProc(Obj *obj, s16 msg, void *arg) {
    KiranW *w = (KiranW *) (obj + 1);
    KiranArg *a = arg;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "kiran";
            PRINTF("きらん\n");
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->glow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->glow == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) btaiKiranDl, 0xF8);
            bossSetPolDL(w->glow, (Gfx *) btaiKiranGlowDl, 0xF8);
            w->frame->flags |= 7;
            w->glow->flags |= 7;
            w->frame->scale[2] = 0.1f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->glow->scale[2] = 0.1f;
            w->glow->scale[0] = w->glow->scale[1] = w->glow->scale[2];
            w->frame->pos[0] = a->pos[0];
            w->frame->pos[1] = a->pos[1];
            w->frame->pos[2] = a->pos[2];
            w->glow->pos[0] = a->pos[0];
            w->glow->pos[1] = a->pos[1];
            w->glow->pos[2] = a->pos[2];
            w->alpha = 64;
            w->state = 0;
            break;
        case 1:
            /* one line each, the game's style (IDO_NOTES 324; tlb_0081's destroys) */
            if (w->frame != NULL) frmDestroy(&w->frame->f);
            if (w->glow != NULL) frmDestroy(&w->glow->f);
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(0xFF, 0xFF, 0xFF, 0xDE, 0xA2, 0x52, w->alpha);
            w->glow->dl = bossPrimEnvColorDL(0xFF, 0xFF, 0xFF, 0xDE, 0xA2, 0x52, w->alpha / 6);
            break;
        case 0:
            if (btaiEffectsEnd == 1) {
                return -1;
            }
            switch (w->state) {
                case 0:
                    w->alpha += 16;
                    if (w->alpha > 224) {
                        w->state = 1;
                        w->alpha = 224;
                        break;
                    }
                    w->frame->scale[0] += 1.6;
                    w->frame->scale[1] = w->frame->scale[0];
                    w->glow->scale[0] = w->frame->scale[0] / 4;
                    w->glow->scale[1] = w->glow->scale[0];
                    break;
                case 1:
                    w->alpha++;
                    if (w->alpha > 255) {
                        w->alpha = 255;
                        w->state = 2;
                        break;
                    }
                    w->frame->scale[0] += 0.2;
                    w->frame->scale[1] = w->frame->scale[0];
                    w->glow->scale[0] = w->frame->scale[0] / 4;
                    w->glow->scale[1] = w->glow->scale[0];
                    break;
                case 2:
                    w->alpha -= 20;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        PRINTF("きらんを消します\n");
                        return -1;
                    }
                    if (w->frame->scale[0] > 0.01) {
                        w->frame->scale[0] -= 0.4;
                        w->frame->scale[1] = w->frame->scale[0];
                        w->glow->scale[0] = w->frame->scale[0] / 4;
                        w->glow->scale[1] = w->glow->scale[0];
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "kiran" (sparkle) at (x, y, z) under the battle's task. */
void btaiKiranMake(f32 x, f32 y, f32 z) {
    KiranArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gBattleTask, btaiKiranProc, gBattleTask->file, &arg);
}

/* The "bombball" task's creation argument. */
typedef struct BombballArg {
    /* 0x00 */ u8 pad00[4];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ f32 size; /* the scale it swells to */
    /* 0x14 */ u8 pad14[8];
    /* 0x1C */ s32 time; /* how long it holds there */
} BombballArg; /* size 0x20 */

/* The "bombball" task's work. */
typedef struct BombballW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0xC];
    /* 0x10 */ f32 size;
    /* 0x14 */ s32 alpha;
    /* 0x18 */ s32 state; /* 0 swells, 1 holds, 2 swells and fades */
    /* 0x1C */ s32 time;
} BombballW;

extern u8 btaiBombballDl[]; /* the ball, in file 0x120 */

/* "bombball": a yellow ball from file 0x120, spinning, swelling by 3% a frame to its size,
 * holding for its time, then swelling by 4% a frame while it fades. */
s32 btaiBombballProc(Obj *obj, s16 msg, void *arg) {
    BombballW *w = (BombballW *) (obj + 1);
    BombballArg *a = arg;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "bombball";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) btaiBombballDl, 0x120);
            w->frame->scale[2] = 1.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->size = a->size;
            w->frame->pos[0] = a->pos[0];
            w->frame->pos[1] = a->pos[1];
            w->frame->pos[2] = a->pos[2];
            w->alpha = 255;
            w->time = a->time;
            w->state = 0;
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 0:
            w->frame->rot[1] += 0.0523598790116402354;
            w->frame->rot[2] += 0.034906586007760157;
            switch (w->state) {
                case 0:
                    w->frame->scale[0] *= 1.03;
                    w->frame->scale[1] *= 1.03;
                    w->frame->scale[2] *= 1.03;
                    if (w->size < w->frame->scale[0]) {
                        w->state = 1;
                        PRINTF("<ball>step1\n");
                    }
                    break;
                case 1:
                    if (w->time-- < 0) {
                        w->state = 2;
                        PRINTF("<ball>step2\n");
                    }
                    break;
                case 2:
                    w->frame->scale[0] *= 1.04;
                    w->frame->scale[1] *= 1.04;
                    w->frame->scale[2] *= 1.04;
                    w->alpha--;
                    break;
            }
            if (w->alpha <= 0) {
                w->alpha = 0;
                PRINTF("<ball>destroy\n");
                return -1;
            }
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(0xFF, 0xFF, 0, 0xFF, 0, 0, w->alpha);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "bombball" at (x, y, z) under the battle's task, swelling to size and holding for time. */
void btaiBombballMake(f32 x, f32 y, f32 z, f32 size, s32 time) {
    BombballArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.size = size;
    arg.time = time;
    tskCreate(gBattleTask, btaiBombballProc, gBattleTask->file, &arg);
}

/* The "flash2" task's creation argument. */
typedef struct Flash2Arg {
    /* 0x00 */ u8 pad00[4];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ u8 pad10[8];
    /* 0x18 */ s32 time; /* how long it stays lit */
} Flash2Arg; /* size 0x1C */

/* The "flash2" task's work. */
typedef struct Flash2W {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0xC];
    /* 0x10 */ s32 alpha;
    /* 0x14 */ s32 state; /* 0 lights up, 1 stays, 2 fades out */
    /* 0x18 */ s32 time;
} Flash2W;

extern u8 btaiFlash2Dl[]; /* the flash, in file 0x120 (beffect.c) */

/* "flash2" ("pika 2"): a white flash from file 0x120 at 500 times its size, lighting up by 5,
 * staying for its time, then fading out by 4. */
s32 btaiFlash2Proc(Obj *obj, s16 msg, void *arg) {
    Flash2W *w = (Flash2W *) (obj + 1);
    Flash2Arg *a = arg;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "flash2";
            PRINTF("ぴか2\n");
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) btaiFlash2Dl, 0x120);
            w->frame->flags |= 7;
            w->frame->scale[2] = 500.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = a->pos[0];
            w->frame->pos[1] = a->pos[1];
            w->frame->pos[2] = a->pos[2];
            w->alpha = 1;
            w->state = 0;
            w->time = a->time;
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, w->alpha);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->alpha += 5;
                    if (w->alpha >= 250) {
                        w->alpha = 255;
                        w->state = 1;
                    }
                    break;
                case 1:
                    w->time--;
                    if (w->time < 0) {
                        w->state = 2;
                    }
                    break;
                case 2:
                    w->alpha -= 4;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        PRINTF("フラッシュ2を消しました\n");
                        return -1;
                    }
                    break;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "flash2" at (x, y, z) under the battle's task. */
void btaiFlash2Make(f32 x, f32 y, f32 z, s32 a) {
    Flash2Arg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.time = a;
    tskCreate(gBattleTask, btaiFlash2Proc, gBattleTask->file, &arg);
}

/* The "barri" (barrier) tasks' creation argument. */
typedef struct BarriArg {
    /* 0x00 */ u8 pad00[0x28];
    /* 0x28 */ s32 no; /* which of the five */
    /* 0x2C */ u8 pad2C[0x10];
} BarriArg; /* size 0x3C */

/* The "barri" and "barriw" (barrier) tasks' work. */
typedef struct BarriW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0xC];
    /* 0x10 */ s32 alpha;
    /* 0x14 */ s32 unused14; /* set 220 with the alpha in "barri"'s msg 2, never read */
    /* 0x18 */ u8 pad18[8];
    /* 0x20 */ s32 time; /* 0-60, again and again */
    /* 0x24 */ s32 delay; /* 12 frames per number */
    /* 0x28 */ s32 no;
    /* 0x2C */ s32 done; /* 1 once the boss's flag 0x4000 is off: it fades out */
    /* 0x30 */ u8 pad30[4];
    /* 0x34 */ u8 unused34; /* set 1 in "barri"'s msg 2, never read */
} BarriW;

extern u8 btaiBarriRingDl[]; /* a barrier ring, in file 0xF8 */
extern u8 btaiBarriwDl[]; /* the barrier's wall, in file 0xEE (beffect.c's ring) */

/* "barri" (barrier): ring number no of five, from file 0xF8, 600 in front of the boss while
 * its flag 0x4000 is on; after 12 frames per number it pulses every 61 frames, growing and
 * spinning. */
s32 btaiBarriProc(Obj *obj, s16 msg, void *arg) {
    BarriW *w = (BarriW *) (obj + 1);
    s32 pad[2];

    if (1) {} /* FAKE: a block boundary (a compiled-out print?) keeps w out of s0 */
    switch (msg) {
        case 2:
            w->no = ((BarriArg *) arg)->no;
            obj->name = "barri";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) btaiBarriRingDl, 0xF8);
            w->frame->flags |= 0x22;
            w->frame->scale[2] = 0.01f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->rot[2] = 0.0f;
            w->frame->rot[0] = w->frame->rot[2];
            w->frame->rot[1] = gBossYaw;
            w->alpha = 220;
            w->unused14 = 220;
            w->delay = w->no * 12;
            w->time = 0;
            w->done = 0;
            w->unused34 = 1;
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 0:
            w->delay--;
            if (w->delay <= 0) {
                w->time++;
                if (w->time > 60) {
                    w->time = 0;
                    w->alpha = 0;
                    w->frame->scale[2] = 0.1f;
                    w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
                }
                if (w->done == 0) {
                    w->alpha = (w->time / 2) * (w->time / 2) + 1;
                } else {
                    w->alpha -= 6;
                }
                if (w->alpha > 255) {
                    w->alpha = 255;
                }
                if (w->alpha < 0) {
                    w->alpha = 0;
                    return -1;
                }
                w->frame->scale[0] += 0.042;
                w->frame->scale[1] += 0.042;
                w->frame->scale[2] += 0.042;
                w->frame->rot[2] += 0.349065860077601542;
                if (gBossFlags & 0x4000) {
                    w->frame->pos[0] = gBossPosX - sinf(gBossYaw + 3.1415927f) * 600.0f;
                    w->frame->pos[1] = 0.0f;
                    w->frame->pos[2] = gBossPosZ - cosf(gBossYaw + 3.1415927f) * 600.0f;
                } else {
                    w->done = 1;
                }
            }
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(w->alpha, w->alpha, 0xFF, 0, 0, 100, w->alpha);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "barriw" (barrier wall): the wall from file 0xEE, 600 in front of the boss while its flag
 * 0x4000 is on, spinning; puffs (effectPompomCreate) at the boss every 20 frames; fades out
 * once the flag is off. */
s32 btaiBarriwProc(Obj *obj, s16 msg, void *arg) {
    BarriW *w = (BarriW *) (obj + 1);
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "barriw";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) btaiBarriwDl, 0xEE);
            w->frame->flags |= 0x22;
            w->frame->scale[2] = 0.6f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->rot[1] = 0.0f;
            w->frame->rot[0] = w->frame->rot[2] = w->frame->rot[1];
            w->alpha = 1;
            w->done = 0;
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 0:
            if (w->done == 0) {
                w->alpha += 2;
            } else {
                w->alpha -= 8;
            }
            if (w->alpha > 255) {
                w->alpha = 255;
            }
            if (w->alpha < 0) {
                w->alpha = 0;
                return -1;
            }
            w->frame->rot[2] += 0.0698131720155203139;
            if (w->frame->rot[2] > 6.2831855f) {
                w->frame->rot[2] -= 6.2831855f;
            }
            w->frame->rot[1] = gBossYaw;
            if (gBossFlags & 0x4000) {
                w->frame->pos[0] = gBossPosX - sinf(gBossYaw + 3.1415927f) * 600.0f;
                w->frame->pos[1] = 0.0f;
                w->frame->pos[2] = gBossPosZ - cosf(gBossYaw + 3.1415927f) * 600.0f;
            } else {
                w->done = 1;
            }
            if (gGameFrameCount % 20 == 0) {
                effectPompomCreate(gBossPosX, 0.0f, gBossPosZ + 400.0f, 100.0f, 100.0f, 100.0f, 12, 20, 6, 1.8f,
                                   1.0f);
            }
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(0xFF, 0xFF, 0xFF, 0, 0, 0xFF, w->alpha);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The barrier: five "barri" tasks, numbered 0-4, and a "barriw", under the battle's task. */
void btaiBarriMake(void) {
    BarriArg arg;
    s32 i;

    for (i = 0; i < 5; i++) {
        arg.no = i;
        tskCreate(gBattleTask, btaiBarriProc, gBattleTask->file, &arg);
    }
    tskCreate(gBattleTask, btaiBarriwProc, gBattleTask->file, &arg);
}

/* Three slots (positions in tlb_0324_7), each with an in-use flag and its eel. */
typedef struct BarriSlot {
    /* 0x00 */ f32 pos[3];
    /* 0x0C */ s32 used;
    /* 0x10 */ Obj *obj; /* its eel (btaiUtsuboProc, tlb_0324_7) */
} BarriSlot; /* size 0x14 */

extern BarriSlot btaiBarriSlots[3];

/* Takes the first free slot of btaiBarriSlots and returns its number, or -1 when all three are taken. */
s32 btaiUtsuboTakeSlot(void) {
    s32 i;

    for (i = 0; i < 3; i++) {
        if (btaiBarriSlots[i].used == 0) {
            btaiBarriSlots[i].used = 1;
            return i;
        }
    }
    return -1;
}
