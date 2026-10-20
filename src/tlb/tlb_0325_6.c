#include "tlb.h"
#include "tlb/tlb_0325.h"

#define sinf (*sinf)
#define cosf (*cosf)

/* The "SUNAef" (sand dust) tasks' creation argument (as tlb_0324_6's sunaEfProc). */
typedef struct SunaArg {
    /* 0x00 */ u8 pad00[4];
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ u8 pad10[0x14];
    /* 0x24 */ f32 pos[3];
    /* 0x30 */ f32 scale;
    /* 0x34 */ u8 pad34[8];
    /* 0x3C */ s32 time;
} SunaArg;

/* The "SUNAef" tasks' work. */
typedef struct SunaW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ Anim anim;
    /* 0x24 */ u8 pad24[0x14];
    /* 0x38 */ s32 time; /* also the alpha */
    /* 0x3C */ s32 count;
    /* 0x40 */ u8 pad40[2];
    /* 0x42 */ u8 fadeOut; /* tlb_0324_6's: its third puff fades out at 1; nothing here reads it */
} SunaW;

/* This file's .data (0x08017B80, IDO_NOTES 343/349), in the game's order: two flags, the sand
 * dust's and the cloud's texture animations (file 0x123), then the sky's scrolls, the statics of
 * bfriSkyProc (last). */
extern u8 bfriSandDustTex0[];
extern u8 bfriSandDustTex1[];
extern u8 bfriSandDustTex2[];
extern u8 bfriSandDustTex3[];
extern u8 bfriSandDustTex4[];
extern u8 bfriSandDustTex6[];
s32 bfriFlashFrames = 1; /* frames of lightning flash left */
s32 bfriEffectsEnd = 1; /* 1: the battle's effects end */
AnimFrame bfriSandDustAnim[] = { /* the sand dust's texture animation (file 0x123) */
    { (u32) bfriSandDustTex0, 6 }, { (u32) bfriSandDustTex1, 6 }, { (u32) bfriSandDustTex2, 6 }, { (u32) bfriSandDustTex3, 6 },
    { (u32) bfriSandDustTex4, 6 }, { 0x03002D30, 6 },      { (u32) bfriSandDustTex6, 6 }, { 0x03003D30, 6 },
    { 0x03004530, 6 },      { 0, 0 },
};
extern u8 bfriSandDustDl[]; /* the sand dust's display list, in file 0x123 */

/* "SUNAef" (sand dust), this file's first: as tlb_0324_6's sunaEfProc, but twice life size,
 * its animation stepped every frame and its time falling by 2. The time is stored before fadeOut
 * as in sunaEfProc. The argument is read through a typed local set in case 2 (as tlb_0323_1's
 * bisuHpunchProc): that keeps it in s1 for the whole function as the game does. It replaces a
 * FAKE empty "if (a) {}" before the position copies with `a` set at its declaration (a kept
 * FAKE, 2026-10-17). Also measured then: no copy, every read through ((SunaArg *) arg) (real
 * 19, obj takes s1); the copy passed on, tskDefaultProc(obj, msg, a) (19). */
s32 bfriSunaEfProc(Obj *obj, s16 msg, void *arg) {
    SunaArg *a;
    SunaW *w = (SunaW *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "SUNAef";
            PRINTF("砂ぼこり\n");
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bfriSandDustDl, 0x123);
            fadeAnimStart(&w->anim, bfriSandDustAnim, 0, &w->frame->segs[3], floadGetSetFile(0x123, 0));
            w->frame->flags |= 7;
            w->frame->scale[2] = 2.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            a = (SunaArg *) arg;
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
            if (w->frame != NULL) {
                w->frame->dl = bossPrimEnvColorDL(0xD1, 0xCC, 0x93, 0xB7, 0xB2, 0x7F, w->time);
            }
            break;
        case 0:
            if (bfriEffectsEnd == 1) {
                return -1;
            }
            fadeAnimStep(&w->anim);
            w->time -= 2;
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

s32 bfriSunaEfProc(Obj *obj, s16 msg, void *arg);
extern Obj *gBattleTask; /* the battle's task (beffect.c) */

/* A sand-dust puff at (x, y - 850, z), drifting by a fifth of (dx, 0, dz), under the battle's task. */

void bfriSunaEfMake(f32 x, f32 y, f32 z, f32 dx, f32 dz, f32 scale, s32 time) {
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
    tskCreate(gBattleTask, bfriSunaEfProc, gBattleTask->file, &arg);
}

/* A moving task's work: its frame, then its velocity. */
typedef struct Tlb325MoverW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
} Tlb325MoverW;

/* Every 5 frames, a sand-dust puff under the moving task: at its frame's x and z (+50), at
 * -500, drifting with it, for 255 frames. The game works out a tenth of (speed squared + 100)
 * (the div stays, its mflo never read) and does not use it. */
void bfriSunaEfTrail(Obj *o) {
    Tlb325MoverW *w = (Tlb325MoverW *) FX(o);
    s32 n;

    if (gGameFrameCount % 5 == 0) {
        n = (s32) (w->vel[2] * w->vel[2] + w->vel[0] * w->vel[0] + 100.0f) / 10;
        /* n is clamped and never read: the clamp keeps the game's div (its mflo goes); an s32
         * clamp, as its twin btaiSunaEfTrail (tlb_0324_6), for the u32 never-true test of before. */
        if (n < 0) {
            n = 0;
        }
        (*bfriSunaEfMake)(w->frame->pos[0], -500.0f, w->frame->pos[2] + 50.0f, w->vel[0], w->vel[2],
                                  3.0f, 255);
    }
}

/* The second "SUNAef": a puff of the size given at its place, its animation stepped twice a
 * frame, white-sand coloured at 255; it ends when its count runs out. */
s32 bfriSunaEf2Proc(Obj *obj, s16 msg, void *arg) {
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
            bossSetPolDL(w->frame, (Gfx *) bfriSandDustDl, 0x123);
            fadeAnimStart(&w->anim, bfriSandDustAnim, 0, &w->frame->segs[3], floadGetSetFile(0x123, 0));
            w->frame->flags |= 7;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = a->scale;
            w->frame->pos[0] = a->pos[0];
            w->frame->pos[1] = a->pos[1];
            w->frame->pos[2] = a->pos[2];
            w->time = 255;
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 3:
            if (w->frame != NULL) {
                w->frame->dl = bossPrimEnvColorDL(0xD1, 0xCC, 0x93, 0xB7, 0xB2, 0x7F, w->time);
            }
            break;
        case 0:
            if (bfriEffectsEnd == 1) {
                return -1;
            }
            fadeAnimStep(&w->anim);
            fadeAnimStep(&w->anim);
            if (--w->count <= 0) {
                w->count = 0;
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A sand-dust puff at (x, y - 850, z), drifting by a fifth of (dx, 0, dz), under the battle's task. */

void bfriSunaEf2Make(f32 x, f32 y, f32 z, f32 dx, f32 dz, f32 scale, s32 time) {
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
    tskCreate(gBattleTask, bfriSunaEf2Proc, gBattleTask->file, &arg);
}

/* "KOKE2ef"'s (bfriKoke2EfProc) argument: only the position its starter sets. */
typedef struct Koke2Arg {
    /* 0x00 */ u8 pad00[0x44];
    /* 0x44 */ f32 pos[3];
    /* 0x50 */ u8 pad50[0x10];
} Koke2Arg;

/* "KOKE2ef"'s work: three puffs (as tlb_0324_6's "KOKEef"). */
typedef struct Koke2W {
    /* 0x00 */ PolFrame *frames[3];
    /* 0x0C */ f32 vel[3][3]; /* [axis][puff] */
    /* 0x30 */ u8 pad30[4];
    /* 0x34 */ Anim anim; /* the three puffs' one texture animation */
    /* 0x44 */ u8 pad44[0x10];
    /* 0x54 */ s32 alpha;
} Koke2W;

/* "KOKE2ef" (moss dust, as tlb_0324_6's "KOKEef"): three puffs at a place, six times life
 * size, thrown up and out at 5 a frame (away from the origin and to both sides), falling back,
 * growing and fading. */
s32 bfriKoke2EfProc(Obj *obj, s16 msg, void *arg) {
    f32 angle;
    Koke2W *w = (Koke2W *) (obj + 1);
    Koke2Arg *a = arg;
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "KOKE2ef";
            PRINTF("こけぼこり\n");
            for (i = 0; i < 3; i++) {
                w->frames[i] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
                if (w->frames[i] == NULL) {
                    return -1;
                }
                bossSetPolDL(w->frames[i], (Gfx *) bfriSandDustDl, 0x123);
                fadeAnimStart(&w->anim, bfriSandDustAnim, 0, &w->frames[i]->segs[3], floadGetSetFile(0x123, 0));
                w->frames[i]->flags |= 7;
                w->frames[i]->scale[2] = 6.0f;
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
                if (w->frames[i] != NULL) {
                    w->frames[i]->dl = bossPrimEnvColorDL(0xA2, 0x98, 0x84, 0x8E, 0x84, 0x7A, w->alpha);
                }
            }
            break;
        case 0:
            if (bfriEffectsEnd == 1) {
                return -1;
            }
            fadeAnimStep(&w->anim);
            if (w->alpha > 160) {
                w->alpha -= 2;
            } else {
                w->alpha -= 4;
            }
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            for (i = 0; i < 3; i++) {
                w->vel[1][i] -= 0.1;
                w->frames[i]->scale[0] += 0.02;
                w->frames[i]->scale[1] += 0.02;
                w->frames[i]->scale[2] += 0.02;
                w->frames[i]->pos[0] += w->vel[0][i];
                w->frames[i]->pos[1] += w->vel[1][i];
                w->frames[i]->pos[2] += w->vel[2][i];
                if (w->frames[i]->pos[2] > -810.0f) {
                    w->frames[i]->pos[2] = -810.0f;
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "KOKE2ef" at (x, y - 850, z) under the battle's task. */
void bfriKoke2EfMake(f32 x, f32 y, f32 z) {
    Koke2Arg arg;

    arg.pos[0] = x;
    arg.pos[1] = y + -850.0f;
    arg.pos[2] = z;
    tskCreate(gBattleTask, bfriKoke2EfProc, gBattleTask->file, &arg);
}

/* "kiran2"'s (bfriKiran2Proc) argument: only the position its starter sets. */
typedef struct Kiran2Arg {
    /* 0x00 */ u8 pad00[0x8];
    /* 0x08 */ f32 pos[3];
    /* 0x14 */ u8 pad14[0x8];
} Kiran2Arg;

/* "kiran2"'s work (as tlb_0324_6's "kiran"). */
typedef struct Kiran2W {
    /* 0x00 */ PolFrame *frame; /* the sparkle */
    /* 0x04 */ PolFrame *glow; /* a quarter of its width */
    /* 0x08 */ u8 pad08[0xC];
    /* 0x14 */ s32 alpha;
    /* 0x18 */ s32 state; /* 0 opens, 1 glows, 2 closes */
} Kiran2W;

/* "kiran2" (sparkle, as tlb_0324_6's "kiran" but with no display lists set): two frames that
 * open wide by 16 a frame up to 224, glow up to 255, then close by 20 a frame. Case 1's first
 * test is on one line: that puts w's addiu in its beqz's delay slot, as the game has it. */
s32 bfriKiran2Proc(Obj *obj, s16 msg, void *arg) {
    Kiran2W *w = (Kiran2W *) FX(obj);
    Kiran2Arg *a = arg;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "kiran2";
            PRINTF("きらん\n");
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->glow = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->glow == NULL) {
                return -1;
            }
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
            if (w->frame != NULL) { frmDestroy(&w->frame->f); }
            if (w->glow != NULL) {
                frmDestroy(&w->glow->f);
            }
            break;
        case 3:
            if (w->frame != NULL) {
                w->frame->dl = bossPrimEnvColorDL(0xFF, 0xFF, 0xFF, 0xDE, 0xA2, 0x52, w->alpha);
            }
            if (w->glow != NULL) {
                w->glow->dl = bossPrimEnvColorDL(0xFF, 0xFF, 0xFF, 0xDE, 0xA2, 0x52, w->alpha / 6);
            }
            break;
        case 0:
            if (bfriEffectsEnd == 1) {
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

/* A "kiran2" at x, y, z under the battle's task. */
void bfriKiran2Make(f32 x, f32 y, f32 z) {
    Kiran2Arg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gBattleTask, bfriKiran2Proc, gBattleTask->file, &arg);
}

/* "Kumo"'s work: three puffs of cloud. */
typedef struct KumoW {
    /* 0x00 */ PolFrame *frames[3];
    /* 0x0C */ Anim anim; /* the three puffs' one texture animation */
    /* 0x1C */ f32 ofs[3][3]; /* [axis][puff]: each puff's place about the boss */
    /* 0x40 */ f32 vel[3];
    /* 0x4C */ s32 alpha;
    /* 0x50 */ s32 time;
} KumoW;

AnimFrame bfriCloudAnim[] = { /* the cloud's texture animation (file 0x123) */
    { (u32) bfriSandDustTex0, 4 }, { (u32) bfriSandDustTex1, 4 },  { (u32) bfriSandDustTex2, 6 }, { (u32) bfriSandDustTex3, 8 },
    { (u32) bfriSandDustTex4, 12 }, { (u32) bfriSandDustTex3, 8 }, { (u32) bfriSandDustTex2, 6 }, { (u32) bfriSandDustTex1, 4 },
    { (u32) bfriSandDustTex0, 6 }, { 0, 0 },
};
extern s32 bfriCloudsGather; /* tlb_0325_1's: 1 while the clouds gather */

/* rand(n), -n .. n - 1 (tlb_0325_3). */
#define bfriRandSigned (*bfriRandSigned)

/* "Kumo" (cloud): three puffs scattered about the boss, drawn to the boss's place: while
 * bfriCloudsGather is 1 by a thirtieth of the distance a frame, fading by 3; else by a
 * tenth, fading in by 4 a frame and out by 2 after 100 frames. They grow wider than tall. */
s32 bfriKumoProc(Obj *obj, s16 msg, void *arg) {
    KumoW *w = (KumoW *) (obj + 1);
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "Kumo";
            for (i = 0; i < 3; i++) {
                w->frames[i] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
                if (w->frames[i] == NULL) {
                    return -1;
                }
                bossSetPolDL(w->frames[i], (Gfx *) bfriSandDustDl, 0x123);
                fadeAnimStart(&w->anim, bfriCloudAnim, 0, &w->frames[i]->segs[3], floadGetSetFile(0x123, 0));
                w->frames[i]->flags |= 7;
                w->frames[i]->scale[2] = 5.0f;
                w->frames[i]->scale[0] = w->frames[i]->scale[1] = w->frames[i]->scale[2];
                w->ofs[0][i] = bfriRandSigned(500);
                w->ofs[1][i] = bfriRandSigned(70) - 470;
                w->ofs[2][i] = bfriRandSigned(300);
                w->frames[i]->pos[0] = w->ofs[0][i] + gBossPosX;
                w->frames[i]->pos[1] = w->ofs[1][i] + gBossPosY;
                w->frames[i]->pos[2] = w->ofs[2][i] + gBossPosZ;
            }
            w->alpha = 1;
            w->time = 0;
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
                    w->frames[i]->dl = bossPrimEnvColorDL(0xFF, 0xFF, 0xFF, 0x5A, 0x5A, 0x78, w->alpha);
                }
            }
            break;
        case 0:
            if (bfriEffectsEnd == 1) {
                return -1;
            }
            if (gGameFrameCount % 5 == 0) {
                fadeAnimStep(&w->anim);
            }
            w->time++;
            if (bfriCloudsGather == 1) {
                if (w->time * 4 < 255) {
                    w->time = w->time * 4; /* the game's: the else branch sets alpha here */
                }
                w->alpha -= 3;
                w->vel[0] = (gBossPosX + w->ofs[0][0] - w->frames[0]->pos[0]) / 30.0f;
                w->vel[2] = (gBossPosZ + w->ofs[2][0] - w->frames[0]->pos[2]) / 30.0f;
            } else {
                if (w->time * 4 < 255) {
                    w->alpha = w->time * 4;
                } else if (w->time > 100) {
                    w->alpha -= 2;
                }
                w->vel[0] = (gBossPosX + w->ofs[0][0] - w->frames[0]->pos[0]) / 10.0f;
                w->vel[2] = (gBossPosZ + w->ofs[2][0] - w->frames[0]->pos[2]) / 10.0f;
            }
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            for (i = 0; i < 3; i++) {
                w->frames[i]->scale[0] += 0.03;
                w->frames[i]->scale[1] += 0.01;
                w->frames[i]->scale[2] += 0.03;
                w->frames[i]->pos[0] += w->vel[0];
                w->frames[i]->pos[2] += w->vel[2];
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "Kumo2"'s (bfriKumo2Proc) argument: only the position its starter sets. */
typedef struct Kumo2Arg {
    /* 0x00 */ u8 pad00[0x40];
    /* 0x40 */ f32 pos[3]; /* "Kumo2" takes it as its velocity */
    /* 0x4C */ u8 pad4C[0x8];
} Kumo2Arg;

/* "Kumo2" (cloud): three puffs scattered about the boss, drifting by the argument's vector,
 * fading in by 20 a frame to 128 and out by 6 after 20 frames, growing wider than tall. */
s32 bfriKumo2Proc(Obj *obj, s16 msg, void *arg) {
    KumoW *w = (KumoW *) (obj + 1);
    Kumo2Arg *a = arg;
    s32 i;

    switch (msg) {
        case 2:
            obj->name = "Kumo2";
            for (i = 0; i < 3; i++) {
                w->frames[i] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
                if (w->frames[i] == NULL) {
                    return -1;
                }
                bossSetPolDL(w->frames[i], (Gfx *) bfriSandDustDl, 0x123);
                fadeAnimStart(&w->anim, bfriCloudAnim, 0, &w->frames[i]->segs[3], floadGetSetFile(0x123, 0));
                w->frames[i]->flags |= 7;
                w->frames[i]->scale[2] = 5.0f;
                w->frames[i]->scale[0] = w->frames[i]->scale[1] = w->frames[i]->scale[2];
                w->ofs[0][i] = bfriRandSigned(300);
                w->ofs[1][i] = bfriRandSigned(70) - 470;
                w->ofs[2][i] = bfriRandSigned(200);
                w->frames[i]->pos[0] = w->ofs[0][i] + gBossPosX;
                w->frames[i]->pos[1] = w->ofs[1][i] + gBossPosY;
                w->frames[i]->pos[2] = w->ofs[2][i] + gBossPosZ;
            }
            w->alpha = 1;
            w->time = 0;
            w->vel[0] = a->pos[0];
            w->vel[1] = a->pos[1];
            w->vel[2] = a->pos[2];
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
                    w->frames[i]->dl = bossPrimEnvColorDL(0xFF, 0xFF, 0xFF, 0x5A, 0x5A, 0x78, w->alpha);
                }
            }
            break;
        case 0:
            if (bfriEffectsEnd == 1) {
                return -1;
            }
            if (gGameFrameCount % 5 == 0) {
                fadeAnimStep(&w->anim);
            }
            w->time++;
            if (w->time * 20 < 128) {
                w->alpha = w->time * 20;
            } else if (w->time > 20) {
                w->alpha -= 6;
            }
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            for (i = 0; i < 3; i++) {
                w->frames[i]->scale[0] += 0.03;
                w->frames[i]->scale[1] += 0.01;
                w->frames[i]->scale[2] += 0.03;
                w->frames[i]->pos[0] += w->vel[0];
                w->frames[i]->pos[2] += w->vel[2];
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

extern Obj *gBattleTask; /* the battle's task (beffect.c) */
s32 bfriKumoProc(Obj *obj, s16 msg, void *arg);

/* Starts a bfriKumoProc task under the battle's task, in the battle task's file. */

Obj *bfriKumoMake(void) {
    return tskCreate(gBattleTask, bfriKumoProc, gBattleTask->file, NULL);
}

/* A "Kumo2" (cloud) drifting by half of x, y, z under the battle's task. */
void bfriKumo2Make(f32 x, f32 y, f32 z) {
    Kumo2Arg arg;

    arg.pos[0] = x / 2;
    arg.pos[1] = y / 2;
    arg.pos[2] = z / 2;
    tskCreate(gBattleTask, bfriKumo2Proc, gBattleTask->file, &arg);
}

extern s32 bfriCloudsRun; /* 1 while the clouds run */

void bfriKumo2Make(f32 x, f32 y, f32 z);

/* While bfriCloudsRun is 1, every 30 frames: at 15 a bfriKumoProc task, at 0
 * a "Kumo2" at half of x, y, z. Both starters are called through their addresses. */
void bfriCloudsMake(f32 x, f32 y, f32 z) {
    if (bfriCloudsRun == 1) {
        if (gGameFrameCount % 30 == 15) {
            (*bfriKumoMake)();
        }
        if (gGameFrameCount % 30 == 0) {
            (*bfriKumo2Make)(x, y, z);
        }
    }
}

/* "tat_TEST"'s work. */
typedef struct TatTestW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ PolFrame *inner; /* 0.7 of the frame's width, at its place */
    /* 0x08 */ f32 vel[3]; /* a sixtieth of the way to the origin */
    /* 0x14 */ s32 alpha;
    /* 0x18 */ s32 time;
    /* 0x1C */ f32 hit; /* 1.0 once the Impact's cannon shot came within 632 */
    /* 0x20 */ f32 spin;
} TatTestW;

extern s32 bfriTornadoAlpha; /* tlb_0325_1's: the tornado's alpha */
extern u8 bfriTatTestDl[]; /* the tornado, in file 0xFB */

/* "tat_TEST" (a tornado, its prints' "竜巻"): two of file 0xFB's tornado at the boss, moving to
 * the origin over 60 frames, spinning by 4 degrees a frame and growing; the Impact's cannon
 * shot passing within about 632 makes it swell and fade out by 4. */
s32 bfriTatTestProc(Obj *obj, s16 msg, void *arg) {
    TatTestW *w = (TatTestW *) (obj + 1);
    f32 dx;
    f32 dz;
    s32 pad[4];

    switch (msg) {
        case 2:
            obj->name = "tat_TEST";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bfriTatTestDl, 0xFB);
            w->inner = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->inner == NULL) {
                return -1;
            }
            bossSetPolDL(w->inner, (Gfx *) bfriTatTestDl, 0xFB);
            w->frame->flags |= 0x22;
            w->inner->flags |= 0x22;
            w->frame->pos[0] = w->inner->pos[0] = gBossPosX;
            w->frame->pos[1] = w->inner->pos[1] = gBossPosY;
            w->frame->pos[2] = w->inner->pos[2] = gBossPosZ;
            w->frame->scale[0] = w->inner->scale[0] = 1.0f;
            w->frame->scale[2] = w->inner->scale[2] = 1.0f;
            w->frame->scale[1] = w->inner->scale[1] = 1.0f;
            w->frame->rot[0] = w->inner->rot[0] = 0.0f;
            w->frame->rot[2] = w->inner->rot[2] = 0.0f;
            w->frame->rot[1] = w->inner->rot[1] = 0.0f;
            w->alpha = bfriTornadoAlpha;
            w->vel[2] = -w->frame->pos[2] / 60.0f;
            w->vel[0] = -w->frame->pos[0] / 60.0f;
            w->vel[1] = -w->frame->pos[1] / 60.0f;
            w->time = 0;
            w->spin = 0.0f;
            w->hit = 0.0f;
            break;
        case 1:
            if (w->frame != NULL) { frmDestroy(&w->frame->f); }
            if (w->inner != NULL) {
                frmDestroy(&w->inner->f);
            }
            break;
        case 3:
            if (w->frame != NULL) {
                w->frame->dl = bossPrimEnvColorDL(0, 0xFF, 0x90, 0, 0xF0, 0x40, w->alpha);
            }
            if (w->inner != NULL) {
                w->inner->dl = bossPrimEnvColorDL(0, 0xFF, 0x90, 0, 0xF0, 0x40, w->alpha);
            }
            break;
        case 0:
            if (bfriEffectsEnd == 1) {
                return -1;
            }
            w->spin += 0.0698131720155203139;
            if (w->spin > 6.2831855f) {
                w->spin -= 6.2831855f;
            }
            w->time++;
            if (w->hit == 1.0f) {
                w->frame->scale[1] += 0.3;
                w->frame->scale[0] += 0.3;
                w->alpha -= 4;
                if (w->alpha < 0) {
                    w->alpha = 0;
                    return -1;
                }
            } else {
                w->frame->scale[1] = w->time * 0.016666668f + 0.05;
                w->frame->scale[0] = w->time * 0.016666668f + 0.05;
            }
            w->inner->scale[0] = w->frame->scale[0] * 0.7;
            w->inner->scale[1] = w->frame->scale[1] * 0.7;
            w->frame->pos[0] += w->vel[0];
            w->frame->pos[1] += w->vel[1];
            w->frame->pos[2] += w->vel[2];
            w->inner->pos[0] = w->frame->pos[0];
            w->inner->pos[1] = w->frame->pos[1];
            w->inner->pos[2] = w->frame->pos[2];
            if (w->frame->pos[2] > -100.0f) {
                return -1;
            }
            if (w->hit == 0.0f) {
                dx = gMegaShotPosX - w->frame->pos[0];
                dz = gMegaShotPosZ - w->frame->pos[2];
                if (dx * dx + dz * dz < 400000.0f) {
                    PRINTF("んが砲が竜巻にあたりました\n");
                    w->hit = 1.0f;
                }
            }
            w->frame->pos[0] *= 0.98;
            w->frame->rot[2] = w->spin;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts a bfriTatTestProc task under the battle's task, in the battle task's file. */

Obj *bfriTatTestMake(void) {
    return tskCreate(gBattleTask, bfriTatTestProc, gBattleTask->file, NULL);
}

/* "Puropera"'s work. */
typedef struct PuroperaW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0xC];
    /* 0x10 */ s32 time; /* frames since it began slowing */
    /* 0x14 */ u8 pad14[0x4];
    /* 0x18 */ f32 spin;
} PuroperaW;

extern s32 bfriPropellerFull; /* 1: the propeller turns at full speed */
extern u8 bfriPuroperaDl[]; /* the propeller's display list, in file 0xFB */

/* "Puropera" (the boss's propeller): file 0xFB's propeller, a tenth of life size, kept on the
 * boss and turned with it. While bfriPropellerFull is 1 it spins 12 degrees a frame; else it
 * slows to a stop over 30 frames and ends. */
s32 bfriPuroperaProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    PuroperaW *w = (PuroperaW *) FX(obj);
    s32 pad2[5];

    switch (msg) {
        case 2:
            obj->name = "Puropera";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bfriPuroperaDl, 0xFB);
            w->frame->flags |= 0x20;
            w->frame->pos[0] = gBossPosX;
            w->frame->pos[1] = gBossPosY;
            w->frame->pos[2] = gBossPosZ;
            w->frame->scale[0] = 0.1f;
            w->frame->scale[2] = 0.1f;
            w->frame->scale[1] = 0.1f;
            w->frame->rot[0] = 0.0f;
            w->frame->rot[2] = 0.0f;
            w->frame->rot[1] = 0.0f;
            w->time = 0;
            w->spin = 0.0f;
            break;
        case 3:
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 0:
            if (bfriEffectsEnd == 1) {
                return -1;
            }
            w->frame->rot[1] = gBossYaw;
            w->frame->pos[0] = gBossPosX;
            w->frame->pos[1] = gBossPosY;
            w->frame->pos[2] = gBossPosZ;
            if (bfriPropellerFull == 1) {
                w->spin += 0.20943951604656094;
                if (w->spin > 6.2831855f) {
                    w->spin -= 6.2831855f;
                }
            } else {
                if (++w->time == 30) {
                    return -1;
                }
                w->spin += (30 - w->time) / 30.0f * 12.0f * 0.01745329300388008;
                if (w->spin > 6.2831855f) {
                    w->spin -= 6.2831855f;
                }
            }
            w->frame->rot[2] = w->spin;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 bfriPuroperaProc(Obj *obj, s16 msg, void *arg);

/* Starts a "Puropera" (bfriPuroperaProc) under the effect task. */
void bfriPuroperaMake(void) {
    tskCreate(gEffectTask, bfriPuroperaProc, gBattleTask->file, NULL);
}

/* "baria1"'s work. */
typedef struct Baria1W {
    /* 0x00 */ PolFrame *frame; /* the frame (file 0xFB's bfriBaria1Dl), its vertices rebuilt */
    /* 0x04 */ PolFrame *inner; /* file 0xFB's bfriBaria1InsideDl */
    /* 0x08 */ s32 alpha;
    /* 0x0C */ s32 innerAlpha;
} Baria1W;

extern s32 bfriBarrierState; /* tlb_0325_1's: the boss's barrier state, 4 while it stands */
extern u8 bfriBaria1Dl[]; /* the barrier's frame, in file 0xFB */
extern u8 bfriBaria1InsideDl[]; /* its inside, in file 0xFB */
extern u8 bfriBaria1Vtx[]; /* the frame's 11 vertices, in file 0xFB */
extern f32 bfriBitX[5]; /* the barrier's five corners: x */
extern f32 bfriBitY[5]; /* y */
extern f32 bfriBitZ[5]; /* z */

/* "baria1" (the barrier's frame): file 0xFB's barrier at the origin, its frame's vertices set
 * each draw from the five corners in bfriBitX (their middle, then each corner twice); it fades
 * in while bfriBarrierState is 4 and ends when it is not. */
s32 bfriBaria1Proc(Obj *obj, s16 msg, void *arg) {
    Baria1W *w = (Baria1W *) (obj + 1);
    s32 pad[3];
    f32 x;
    f32 y;
    f32 z;
    Vtx *vtx;

    switch (msg) {
        case 2:
            obj->name = "baria1";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                PRINTF("\n\n\n*******  !!! フレームが足りません !!! ******\n\n\n");
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bfriBaria1Dl, 0xFB);
            w->inner = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->inner == NULL) {
                PRINTF("\n\n\n*******  !!! フレームが足りません !!! ******\n\n\n");
                return -1;
            }
            bossSetPolDL(w->inner, (Gfx *) bfriBaria1InsideDl, 0xFB);
            w->frame->flags |= 0x20;
            w->inner->flags |= 0x20;
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            w->frame->scale[0] = 1.0f;
            w->frame->scale[2] = 1.0f;
            w->frame->scale[1] = 1.0f;
            w->frame->rot[0] = 0.0f;
            w->frame->rot[2] = 0.0f;
            w->frame->rot[1] = 0.0f;
            w->inner->pos[0] = 0.0f;
            w->inner->pos[1] = 0.0f;
            w->inner->pos[2] = 0.0f;
            w->inner->scale[0] = 1.0f;
            w->inner->scale[2] = 1.0f;
            w->inner->scale[1] = 1.0f;
            w->inner->rot[0] = 0.0f;
            w->inner->rot[2] = 0.0f;
            w->inner->rot[1] = 0.0f;
            w->alpha = 1;
            w->innerAlpha = 1;
            PRINTF("バリア枠を作りました。\n");
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            if (w->inner != NULL) {
                frmDestroy(&w->inner->f);
            }
            break;
        case 3:
            if (w->frame != NULL) {
                w->frame->dl = bossPrimEnvColorDL(0xFF, 0x46, 0x64, 0x64, 0, 0x28, w->alpha);
            }
            if (w->inner != NULL) {
                w->inner->dl = bossPrimEnvColorDL(0xFF, 0, 0x64, 0x64, 0, 0x28, w->innerAlpha);
            }
            vtx = (Vtx *) gGfxDlHead;
            gGfxDlHead += 22;
            w->frame->segs[5] = (u32) vtx;
            heapMemCopy((u8 *) vtx, (u8 *) (w->frame->segs[0] + ((u32) bfriBaria1Vtx & 0xFFFFFF)), 0xB0);
            x = (bfriBitX[0] + bfriBitX[1] + bfriBitX[2] + bfriBitX[3] + bfriBitX[4]) / 5.0f;
            y = (bfriBitY[0] + bfriBitY[1] + bfriBitY[2] + bfriBitY[3] + bfriBitY[4]) / 5.0f;
            z = (bfriBitZ[0] + bfriBitZ[1] + bfriBitZ[2] + bfriBitZ[3] + bfriBitZ[4]) / 5.0f;
            vtx[1].v.ob[0] = x;
            vtx[1].v.ob[1] = y;
            vtx[1].v.ob[2] = z;
            vtx[0].v.ob[0] = vtx[9].v.ob[0] = bfriBitX[0];
            vtx[0].v.ob[1] = vtx[9].v.ob[1] = bfriBitY[0];
            vtx[0].v.ob[2] = vtx[9].v.ob[2] = bfriBitZ[0];
            vtx[2].v.ob[0] = vtx[8].v.ob[0] = bfriBitX[1];
            vtx[2].v.ob[1] = vtx[8].v.ob[1] = bfriBitY[1];
            vtx[2].v.ob[2] = vtx[8].v.ob[2] = bfriBitZ[1];
            vtx[3].v.ob[0] = vtx[7].v.ob[0] = bfriBitX[2];
            vtx[3].v.ob[1] = vtx[7].v.ob[1] = bfriBitY[2];
            vtx[3].v.ob[2] = vtx[7].v.ob[2] = bfriBitZ[2];
            vtx[4].v.ob[0] = vtx[6].v.ob[0] = bfriBitX[3];
            vtx[4].v.ob[1] = vtx[6].v.ob[1] = bfriBitY[3];
            vtx[4].v.ob[2] = vtx[6].v.ob[2] = bfriBitZ[3];
            vtx[5].v.ob[0] = vtx[10].v.ob[0] = bfriBitX[4];
            vtx[5].v.ob[1] = vtx[10].v.ob[1] = bfriBitY[4];
            vtx[5].v.ob[2] = vtx[10].v.ob[2] = bfriBitZ[4];
            break;
        case 0:
            if (bfriBarrierState != 4) {
                return -1;
            }
            w->alpha += 3;
            if (w->alpha > 255) {
                w->alpha = 255;
            }
            if (gGameFrameCount % 2 == 0) {
                w->innerAlpha++;
            }
            if (w->innerAlpha > 160) {
                w->innerAlpha = 160;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* "baria2"'s (bfriBaria2Proc) argument: only the index its starter sets. */
typedef struct Baria2Arg {
    /* 0x00 */ u8 pad00[0x18];
    /* 0x18 */ s32 index;
} Baria2Arg;

/* "baria2"'s work. */
typedef struct Baria2W {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[4];
    /* 0x08 */ s32 alpha;
    /* 0x0C */ u8 pad0C[4];
    /* 0x10 */ s32 step; /* 0 to 60: how far from the corners to their middle */
    /* 0x14 */ s32 delay; /* frames before it starts: 130, 150 or 170 */
    /* 0x18 */ s32 index; /* which of the three */
} Baria2W;

extern u8 bfriBaria2Vtx[]; /* the effect's 11 vertices, in file 0xFB */
extern f32 bfriBitTrailX[3][7]; /* each effect's five corners' offsets: x */
extern f32 bfriBitTrailY[3][7]; /* y */
extern f32 bfriBitTrailZ[3][7]; /* z */

/* "baria2" (a barrier effect, one of three): file 0xFB's barrier inside at the origin, its
 * vertices set each draw between the barrier's five corners (bfriBitX) and their middle, moved
 * by the effect's own offsets; after its delay it sweeps over 60 frames again and again,
 * brightening as it goes, and ends when bfriBarrierState is not 4. */
s32 bfriBaria2Proc(Obj *obj, s16 msg, void *arg) {
    Baria2W *w = (Baria2W *) (obj + 1);
    s32 wait;
    s32 pad[3];
    f32 x;
    f32 y;
    f32 z;
    Vtx *vtx;

    switch (msg) {
        case 2:
            obj->name = "baria2";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                PRINTF("\n\n\n*************  !!! フレームが足りません !!! *********\n\n\n");
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bfriBaria1InsideDl, 0xFB);
            w->frame->flags |= 0x20;
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 0.0f;
            w->frame->pos[2] = 0.0f;
            w->frame->scale[0] = 1.0f;
            w->frame->scale[2] = 1.0f;
            w->frame->scale[1] = 1.0f;
            w->frame->rot[0] = 0.0f;
            w->frame->rot[2] = 0.0f;
            w->frame->rot[1] = 0.0f;
            w->alpha = 1;
            w->step = 0;
            w->index = ((Baria2Arg *) arg)->index;
            w->delay = w->index * 20 + 130;
            PRINTF("バリアエフェクト[%d]を作りました\n", w->index);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(0xFF, 0x46, 0x64, 0x64, 0, 0x28, w->alpha);
            vtx = (Vtx *) gGfxDlHead;
            gGfxDlHead += 22;
            w->frame->segs[5] = (u32) vtx;
            heapMemCopy((u8 *) vtx, (u8 *) (w->frame->segs[0] + ((u32) bfriBaria2Vtx & 0xFFFFFF)), 0xB0);
            x = (bfriBitX[0] + bfriBitX[1] + bfriBitX[2] + bfriBitX[3] + bfriBitX[4]) / 5.0f;
            y = (bfriBitY[0] + bfriBitY[1] + bfriBitY[2] + bfriBitY[3] + bfriBitY[4]) / 5.0f;
            z = (bfriBitZ[0] + bfriBitZ[1] + bfriBitZ[2] + bfriBitZ[3] + bfriBitZ[4]) / 5.0f;
            vtx[1].v.ob[0] = x;
            vtx[1].v.ob[1] = y;
            vtx[1].v.ob[2] = z;
            vtx[0].v.ob[0] = vtx[9].v.ob[0] = (bfriBitX[0] - bfriBitTrailX[w->index][0]) / 60.0f * w->step + x;
            vtx[0].v.ob[1] = vtx[9].v.ob[1] = (bfriBitY[0] - bfriBitTrailY[w->index][0]) / 60.0f * w->step + y;
            vtx[0].v.ob[2] = vtx[9].v.ob[2] = (bfriBitZ[0] - bfriBitTrailZ[w->index][0]) / 60.0f * w->step + z;
            vtx[2].v.ob[0] = vtx[8].v.ob[0] = (bfriBitX[1] - bfriBitTrailX[w->index][1]) / 60.0f * w->step + x;
            vtx[2].v.ob[1] = vtx[8].v.ob[1] = (bfriBitY[1] - bfriBitTrailY[w->index][1]) / 60.0f * w->step + y;
            vtx[2].v.ob[2] = vtx[8].v.ob[2] = (bfriBitZ[1] - bfriBitTrailZ[w->index][1]) / 60.0f * w->step + z;
            vtx[3].v.ob[0] = vtx[7].v.ob[0] = (bfriBitX[2] - bfriBitTrailX[w->index][2]) / 60.0f * w->step + x;
            vtx[3].v.ob[1] = vtx[7].v.ob[1] = (bfriBitY[2] - bfriBitTrailY[w->index][2]) / 60.0f * w->step + y;
            vtx[3].v.ob[2] = vtx[7].v.ob[2] = (bfriBitZ[2] - bfriBitTrailZ[w->index][2]) / 60.0f * w->step + z;
            vtx[4].v.ob[0] = vtx[6].v.ob[0] = (bfriBitX[3] - bfriBitTrailX[w->index][3]) / 60.0f * w->step + x;
            vtx[4].v.ob[1] = vtx[6].v.ob[1] = (bfriBitY[3] - bfriBitTrailY[w->index][3]) / 60.0f * w->step + y;
            vtx[4].v.ob[2] = vtx[6].v.ob[2] = (bfriBitZ[3] - bfriBitTrailZ[w->index][3]) / 60.0f * w->step + z;
            vtx[5].v.ob[0] = vtx[10].v.ob[0] = (bfriBitX[4] - bfriBitTrailX[w->index][4]) / 60.0f * w->step + x;
            vtx[5].v.ob[1] = vtx[10].v.ob[1] = (bfriBitY[4] - bfriBitTrailY[w->index][4]) / 60.0f * w->step + y;
            vtx[5].v.ob[2] = vtx[10].v.ob[2] = (bfriBitZ[4] - bfriBitTrailZ[w->index][4]) / 60.0f * w->step + z;
            break;
        case 0:
            wait = w->delay-- > 0;
            if (!wait) {
                w->step++;
                if (w->step > 60) {
                    w->step = 0;
                }
                if (bfriBarrierState != 4) {
                    return -1;
                }
                w->alpha = w->step + 30;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 bfriBaria1Proc(Obj *obj, s16 msg, void *arg);
s32 bfriBaria2Proc(Obj *obj, s16 msg, void *arg);

/* A barrier: one "baria1" and three "baria2" (0 to 2) under the battle's task. */
void bfriBariaMake(void) {
    s32 i;
    Baria2Arg arg;

    tskCreate(gBattleTask, bfriBaria1Proc, gBattleTask->file, NULL);
    for (i = 0; i < 3; i++) {
        arg.index = i;
        tskCreate(gBattleTask, bfriBaria2Proc, gBattleTask->file, &arg);
    }
}


/* "Fzanzou"'s (bfriFzanzouProc, an afterimage) argument: only what its starter sets. */
typedef struct FzanzouArg {
    /* 0x00 */ u8 pad00[0x10];
    /* 0x10 */ f32 pos[3];
    /* 0x1C */ f32 rot[3];
    /* 0x28 */ f32 frame; /* the model animation's frame */
    /* 0x2C */ u8 pad2C[0x4];
    /* 0x30 */ s32 mode; /* w->mode: 1 grows faster */
} FzanzouArg;

/* "Fzanzou"'s work. */
typedef struct FzanzouW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ u8 pad04[0x28];
    /* 0x2C */ s32 alpha;
    /* 0x30 */ s32 mode; /* 1 grows faster */
} FzanzouW;

extern u8 bfriFzanzouAnim[]; /* the boss's model animation (file 0xFB) */
extern s32 gBossPhase; /* bmove.c's */

/* "Fzanzou": an afterimage of file 0xFB's model at the argument's place, turn and frame (as
 * tlb_0324_6's second "Tzanzou"), fading out from 80 by 7 a frame, its tint shifting from blue
 * to green as it fades, growing by (3 + 2 * gBossPhase) / 100 a frame (by 0.02 more in
 * mode 1). */
s32 bfriFzanzouProc(Obj *obj, s16 msg, void *arg) {
    FzanzouW *w = (FzanzouW *) (obj + 1);
    FzanzouArg *a = arg;
    s32 pad[4];

    switch (msg) {
        case 2:
            obj->name = "Fzanzou";
            PRINTF("残像作ります\n");
            w->frame = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetAnimAt(w->frame, bfriFzanzouAnim, 0xFB, 0.0f);
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
            w->mode = a->mode;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, 255, 255, 255, 255);
            gDPSetEnvColor(gGfxDlHead++, 40, w->alpha + 160, 110 - w->alpha, w->alpha);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            if (bfriEffectsEnd == 1) {
                return -1;
            }
            w->alpha -= 7;
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            if (w->mode == 1) {
                w->frame->scale[0] += 0.02;
            }
            w->frame->scale[0] += (f32) ((3.0 + gBossPhase * 2.0) / 100.0);
            w->frame->scale[1] = w->frame->scale[2] = w->frame->scale[0];
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An "Fzanzou" at x, y, z under the battle's task (obj is not used). */
void bfriFzanzouMake(Obj *obj, f32 x, f32 y, f32 z, f32 a, f32 b, f32 c, f32 d, s32 n) {
    FzanzouArg arg;
    s32 pad[2];

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.rot[0] = a;
    arg.rot[1] = b;
    arg.rot[2] = c;
    arg.frame = d;
    arg.mode = n;
    tskCreate(gBattleTask, bfriFzanzouProc, gBattleTask->file, &arg);
}

/* "flash3"'s (bfriFlash3Proc) argument: only what its starter sets. */
typedef struct Flash3Arg {
    /* 0x00 */ u8 pad00[0x4];
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ u8 pad10[0x8];
    /* 0x18 */ s32 type; /* how long it holds */
} Flash3Arg;

/* "flash3"'s work. */
typedef struct Flash3W {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0xC];
    /* 0x10 */ s32 alpha;
    /* 0x14 */ s32 state; /* 0: brightening; 1: holding; 2: fading */
    /* 0x18 */ s32 timer;
} Flash3W;

extern u8 bfriFlash3Dl[]; /* the spark's display list, in file 0x120 */

/* "flash3" (its prints call it the second flash, "ぴか2"): file 0x120's spark a hundred times
 * life size at the position given, that brightens by 5 to white, holds for the frames given,
 * then fades by 4. */
s32 bfriFlash3Proc(Obj *obj, s16 msg, void *arg) {
    Flash3W *w = (Flash3W *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "flash3";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bfriFlash3Dl, 0x120);
            w->frame->flags |= 7;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 100.0f;
            w->frame->pos[0] = ((Flash3Arg *) arg)->pos[0];
            w->frame->pos[1] = ((Flash3Arg *) arg)->pos[1];
            w->frame->pos[2] = ((Flash3Arg *) arg)->pos[2];
            w->alpha = 1;
            w->state = 0;
            w->timer = ((Flash3Arg *) arg)->type;
            PRINTF("ぴか2\n");
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->alpha);
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
                    if (--w->timer < 0) {
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

/* A "flash3" of a type at x, y, z under the battle's task. */
void bfriFlash3Make(f32 x, f32 y, f32 z, s32 type) {
    Flash3Arg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.type = type;
    tskCreate(gBattleTask, bfriFlash3Proc, gBattleTask->file, &arg);
}

/* "SKY"'s colours. */
typedef struct SkyColor {
    /* 0x00 */ s16 r;
    /* 0x02 */ s16 g;
    /* 0x04 */ s16 b;
} SkyColor;

/* "SKY"'s work. */
typedef struct SkyW {
    /* 0x00 */ PolFrame *frame; /* the near sky (file 0xFB's bfriSkyNearDl) */
    /* 0x04 */ PolFrame *far; /* the far sky (bfriSkyFarDl) */
    /* 0x08 */ s32 alpha; /* both skies' */
    /* 0x0C */ s32 alpha2; /* counted, never drawn */
    /* 0x10 */ SkyColor prim; /* the near sky's */
    /* 0x16 */ SkyColor env;
    /* 0x1C */ SkyColor farPrim; /* the far sky's: flashes while bfriFlashFrames runs */
    /* 0x22 */ SkyColor farEnv;
    /* 0x28 */ u8 pad28[8];
    /* 0x30 */ s32 unused30; /* set 0 in msg 2, never read */
    /* 0x34 */ s32 unused34; /* set 0 in msg 2, never read */
} SkyW;

extern s32 bfriFlashFrames; /* frames of lightning flash left */
extern s32 bfriSkyShown; /* 0: the sky fades out */
extern u8 bfriSkyNearDl[]; /* the near sky, in file 0xFB */
extern u8 bfriSkyFarDl[]; /* the far sky, in file 0xFB */
extern u8 bfriSkyNearTex[]; /* the near sky's clouds (32x32 I4), in file 0xFB */
extern u8 bfriSkyFarTex[]; /* the far sky's clouds (32x32 I4) */

/* "SKY" (the sky, "空"): two of file 0xFB's sky domes with scrolling cloud textures, turned
 * round while gBattleFlags has 0x04000000; while bfriFlashFrames counts down the far one
 * flashes, and at its fifth frame a "THUNDER" bolt falls (near on every third flash while
 * gBattleFlags2 has 0x400). It fades in to 96, and out once bfriSkyShown is 0.
 * The five scroll bytes (0x08017C28-0x08017C38, 4 bytes apart) are this function's own statics:
 * the game reaches them with "lui at; sb/lbu %lo(sym)(at)" at every use, which IDO gives only a
 * function-scope static.
 * Matched 2026-10-11; it was parked until its statics' .data was split into this file. */
s32 bfriSkyProc(Obj *obj, s16 msg, void *arg) {
    static u8 bfriSkyNearScrollS = 1; /* the near sky's texture scroll, s */
    static u8 bfriSkyNearScrollT = 1; /* t */
    static u8 bfriSkyFarScrollS = 1; /* the far sky's, s */
    static u8 bfriSkyFarScrollT = 1; /* t */
    static u8 bfriSkyFlashCount = 1; /* counts the flashes: every third makes a near bolt */
    SkyW *w = (SkyW *) (obj + 1);
    s32 pad[3];

    switch (msg) {
        case 2:
            obj->name = "SKY";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bfriSkyNearDl, 0xFB);
            w->far = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->far == NULL) {
                return -1;
            }
            bossSetPolDL(w->far, (Gfx *) bfriSkyFarDl, 0xFB);
            w->frame->flags |= 0x20;
            w->far->flags |= 0x20;
            w->frame->scale[2] = 1.211f;
            w->frame->scale[1] = w->frame->scale[2];
            w->frame->scale[0] = 1.211f;
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = 1910.0f;
            w->frame->pos[2] = 1000.0f;
            w->far->scale[2] = 1.211f;
            w->far->scale[1] = w->far->scale[2];
            w->far->scale[0] = 1.211f;
            w->far->pos[0] = 0.0f;
            w->far->pos[1] = 3410.0f;
            w->far->pos[2] = 1000.0f;
            w->unused30 = 0;
            w->unused34 = 0;
            w->alpha = 0;
            w->alpha2 = 0;
            w->prim.r = 255;
            w->prim.g = 0x80;
            w->prim.b = 0x80;
            w->env.r = 0x80;
            w->env.g = 0x10;
            w->env.b = 0x10;
            w->farPrim.r = 255;
            w->farPrim.g = 0x80;
            w->farPrim.b = 0x80;
            w->farEnv.r = 0x80;
            w->farEnv.g = 0x10;
            w->farEnv.b = 0x10;
            bfriSkyNearScrollS = 0;
            bfriSkyNearScrollT = 0;
            bfriSkyFlashCount = 0;
            bfriSkyFarScrollS = 0;
            bfriSkyFarScrollT = 0;
            PRINTF("空\n");
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->far->f);
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetTexturePersp(gGfxDlHead++, G_TP_PERSP);
            gSPClearGeometryMode(gGfxDlHead++, G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                                                   G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH);
            gSPSetGeometryMode(gGfxDlHead++, G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);
            gDPSetAlphaCompare(gGfxDlHead++, G_AC_THRESHOLD);
            gDPPipeSync(gGfxDlHead++);
            gDPSetBlendColor(gGfxDlHead++, 0, 0, 0, 1);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->prim.r, w->prim.g, w->prim.b, 255);
            gDPSetEnvColor(gGfxDlHead++, w->env.r, w->env.g, w->env.b, w->alpha);
            gDPSetTextureImage(gGfxDlHead++, G_IM_FMT_I, G_IM_SIZ_16b, 1, bfriSkyNearTex);
            gDPSetTile(gGfxDlHead++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 5,
                       G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);
            gDPLoadSync(gGfxDlHead++);
            gDPLoadBlock(gGfxDlHead++, G_TX_LOADTILE, 0, 0, ((32 * 32 + 3) >> 2) - 1, CALC_DXT_4b(32));
            gDPPipeSync(gGfxDlHead++);
            gDPSetTile(gGfxDlHead++, G_IM_FMT_I, G_IM_SIZ_4b, 2, 0, G_TX_RENDERTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 5,
                       G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);
            gDPSetTileSize(gGfxDlHead++, G_TX_RENDERTILE, bfriSkyNearScrollS, bfriSkyNearScrollT,
                           (bfriSkyNearScrollS + 31) << 2, (bfriSkyNearScrollT + 31) << 2);
            gSPEndDisplayList(gGfxDlHead++);
            w->far->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetTexturePersp(gGfxDlHead++, G_TP_PERSP);
            gSPClearGeometryMode(gGfxDlHead++, G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                                                   G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH);
            gSPSetGeometryMode(gGfxDlHead++, G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH);
            gDPSetCombineLERP(gGfxDlHead++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);
            gDPSetAlphaCompare(gGfxDlHead++, G_AC_THRESHOLD);
            gDPPipeSync(gGfxDlHead++);
            gDPSetBlendColor(gGfxDlHead++, 0, 0, 0, 1);
            gDPSetPrimColor(gGfxDlHead++, 0, 0, w->farPrim.r, w->farPrim.g, w->farPrim.b, 255);
            gDPSetEnvColor(gGfxDlHead++, w->farEnv.r, w->farEnv.g, w->farEnv.b, w->alpha);
            gDPSetTextureImage(gGfxDlHead++, G_IM_FMT_I, G_IM_SIZ_16b, 1, bfriSkyFarTex);
            gDPSetTile(gGfxDlHead++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 5,
                       G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);
            gDPLoadSync(gGfxDlHead++);
            gDPLoadBlock(gGfxDlHead++, G_TX_LOADTILE, 0, 0, ((32 * 32 + 3) >> 2) - 1, CALC_DXT_4b(32));
            gDPPipeSync(gGfxDlHead++);
            gDPSetTile(gGfxDlHead++, G_IM_FMT_I, G_IM_SIZ_4b, 2, 0, G_TX_RENDERTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 5,
                       G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD);
            gDPSetTileSize(gGfxDlHead++, G_TX_RENDERTILE, bfriSkyFarScrollS, bfriSkyFarScrollT,
                           (bfriSkyFarScrollS + 31) << 2, (bfriSkyFarScrollT + 31) << 2);
            gSPEndDisplayList(gGfxDlHead++);
            break;
        case 0:
            if (bfriEffectsEnd == 1) {
                return -1;
            }
            if (gBattleFlags & 0x04000000) {
                w->frame->rot[1] = 3.1415927f;
                w->far->rot[1] = 3.1415927f;
            } else {
                w->frame->rot[1] = 0.0f;
                w->far->rot[1] = 0.0f;
            }
            if (bfriSkyShown == 0) {
                if (gGameFrameCount % 2 == 0) {
                    bfriSkyNearScrollS++;
                }
                if (gGameFrameCount % 3 == 0) {
                    bfriSkyFarScrollS--;
                }
            } else {
                if (gGameFrameCount % 4 == 0) {
                    bfriSkyNearScrollS++;
                    bfriSkyNearScrollT++;
                }
                if (gGameFrameCount % 6 == 0) {
                    bfriSkyFarScrollS++;
                    bfriSkyFarScrollT++;
                }
            }
            if (bfriFlashFrames != 0) {
                w->farPrim.r = 255;
                w->farPrim.g = 255;
                w->farPrim.b = 0xE0;
                w->farEnv.r = bfriFlashFrames * 51;
                w->farEnv.g = bfriFlashFrames * 51;
                w->farEnv.b = bfriFlashFrames * 44;
                if (bfriFlashFrames == 5) {
                    if (bfriSkyFlashCount++ >= 2) {
                        bfriSkyFlashCount = 0;
                    }
                    if (gBattleFlags2 & 0x400) {
                        if (bfriSkyFlashCount == 0) {
                            (*bfriThunderMake)(0);
                        } else {
                            (*bfriThunderMake)(1);
                        }
                    } else {
                        (*bfriThunderMake)(1);
                    }
                }
                bfriFlashFrames--;
            } else {
                w->farPrim.r = 255;
                w->farPrim.g = 0x80;
                w->farPrim.b = 0x80;
                w->farEnv.r = 0x80;
                w->farEnv.g = 0x10;
                w->farEnv.b = 0x10;
            }
            if (bfriSkyShown == 0) {
                if (--w->alpha < 0) {
                    w->alpha = 0;
                    return -1;
                }
            } else {
                w->alpha++;
                w->alpha2++;
                if (w->alpha2 > 255) {
                    w->alpha2 = 255;
                }
                if (w->alpha > 96) {
                    w->alpha = 96;
                }
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 bfriSkyProc(Obj *obj, s16 msg, void *arg);

/* Starts a bfriSkyProc task under the battle's task, in the battle task's file. */

Obj *bfriSkyMake(void) {
    return tskCreate(gBattleTask, bfriSkyProc, gBattleTask->file, NULL);
}

typedef struct Tlb325Arg13F7C {
    /* 0x00 */ u8 pad00[0x18];
    /* 0x18 */ s32 value; /* "THUNDER"'s type */
    /* 0x1C */ u8 pad1C[0xC];
} Tlb325Arg13F7C;

/* "THUNDER"'s work. */
typedef struct ThunderW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0xC];
    /* 0x10 */ s32 alpha;
    /* 0x14 */ s32 anim; /* which of its four bolts it shows, stepped every 3 frames */
    /* 0x18 */ s32 type; /* 0: near; 1: far; 2: straight down, tilted at random */
    /* 0x1C */ s16 color[6]; /* prim r, g, b, env r, g, b */
} ThunderW;

extern u8 bfriThunderBolt0Dl[]; /* the bolts' display lists, in file 0xFB */
extern u8 bfriThunderBolt1Dl[];
extern u8 bfriThunderBolt2Dl[];
extern u8 bfriThunderBolt3Dl[];

/* "THUNDER" (a lightning bolt): file 0xFB's bolt flickering through its four shapes, at a random
 * place in the distance (type 0 or 1) or tilted over the arena (type 2), its height growing by
 * 1% a frame as it fades. */
s32 bfriThunderProc(Obj *obj, s16 msg, void *arg) {
    ThunderW *w = (ThunderW *) (obj + 1);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "THUNDER";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bfriThunderBolt0Dl, 0xFB);
            w->type = ((Tlb325Arg13F7C *) arg)->value;
            w->frame->flags |= 0x22;
            w->frame->scale[1] = 1.0f;
            w->frame->scale[0] = 2.5f;
            w->frame->scale[2] = w->frame->scale[0];
            switch (w->type) {
                case 0:
                    w->frame->pos[0] = bfriRandSigned(600);
                    w->frame->pos[1] = 1920.0f;
                    w->frame->pos[2] = bfriRandSigned(200) - 3500;
                    w->anim = 0;
                    w->alpha = 208;
                    w->frame->scale[1] = 2.0f;
                    break;
                case 1:
                    w->frame->pos[0] = bfriRandSigned(2000);
                    w->frame->pos[1] = 1920.0f;
                    w->frame->pos[2] = bfriRandSigned(2000) - 4500;
                    w->anim = 0;
                    w->alpha = 208;
                    w->frame->scale[1] = 2.0f;
                    break;
                case 2:
                    w->frame->pos[0] = 0.0f;
                    w->frame->pos[1] = 0.0f;
                    w->frame->pos[2] = -2800.0f;
                    w->frame->rot[0] = bfriRandSigned(40) * 0.0174532930038800785;
                    w->frame->rot[1] = bfriRandSigned(40) * 0.0174532930038800785;
                    w->frame->rot[2] = bfriRandSigned(60) * 0.0174532930038800785 + 3.14159274101257324;
                    w->frame->scale[1] = 2.5f;
                    w->alpha = 208;
                    break;
            }
            w->color[0] = 255;
            w->color[1] = 255;
            w->color[2] = 0xE0;
            w->color[3] = 0xA0;
            w->color[4] = 0x60;
            w->color[5] = 0x20;
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            break;
        case 3:
            if (w->frame != NULL) {
                w->frame->dl = bossPrimEnvColorDL(w->color[0], w->color[1], w->color[2], w->color[3], w->color[4],
                                                  w->color[5], w->alpha);
            }
            break;
        case 0:
            if (bfriEffectsEnd == 1) {
                return -1;
            }
            if (gGameFrameCount % 3 == 0) {
                w->anim++;
                if (w->anim >= 5) {
                    w->anim = 0;
                }
                switch (w->anim) {
                    case 0:
                        bossSetPolDL(w->frame, (Gfx *) bfriThunderBolt0Dl, 0xFB);
                        break;
                    case 1:
                        bossSetPolDL(w->frame, (Gfx *) bfriThunderBolt1Dl, 0xFB);
                        break;
                    case 2:
                        bossSetPolDL(w->frame, (Gfx *) bfriThunderBolt2Dl, 0xFB);
                        break;
                    case 3:
                        bossSetPolDL(w->frame, (Gfx *) bfriThunderBolt3Dl, 0xFB);
                        break;
                }
            }
            switch (w->type) {
                case 0:
                    w->frame->scale[1] *= 1.01;
                    if (w->alpha > 192) {
                        w->alpha -= 1;
                    } else {
                        w->alpha -= 20;
                    }
                    break;
                case 1:
                    w->frame->scale[1] *= 1.01;
                    w->alpha -= 16;
                    break;
                case 2:
                    w->frame->scale[1] *= 1.01;
                    w->frame->scale[0] = 2.0f;
                    w->frame->scale[2] = w->frame->scale[0];
                    if (w->alpha > 192) {
                        w->alpha -= 1;
                    } else {
                        w->alpha -= 20;
                    }
                    break;
            }
            if (w->alpha <= 0) {
                w->alpha = 0;
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Starts a bfriThunderProc task under the battle's task, with value in its argument. */
void bfriThunderMake(s32 value) {
    Tlb325Arg13F7C arg;

    arg.value = value;
    tskCreate(gBattleTask, bfriThunderProc, gBattleTask->file, &arg);
}

/* "plasma"'s (bfriPlasmaProc) argument: only what its starter sets. */
typedef struct PlasmaArg {
    /* 0x00 */ u8 pad00[0x14];
    /* 0x14 */ f32 pos[3];
    /* 0x20 */ u8 pad20[0xC];
    /* 0x2C */ s32 type; /* how long it holds */
} PlasmaArg;

/* "plasma"'s work. */
typedef struct PlasmaW {
    /* 0x00 */ PolFrame *frame; /* the spark (file 0x120) */
    /* 0x04 */ u8 pad04[8];
    /* 0x0C */ PolFrame *ring; /* file 0xEE's bfriPlasmaRingDl */
    /* 0x10 */ PolFrame *ring2; /* the same; the game never places it */
    /* 0x14 */ u8 pad14[0xC];
    /* 0x20 */ s32 alpha;
    /* 0x24 */ s32 ringAlpha;
    /* 0x28 */ s32 state; /* 0: grows; 1: flickers; 2: shakes; 3: the rings spread; 4: fades */
    /* 0x2C */ s32 timer;
} PlasmaW;

extern u8 bfriPlasmaRingDl[]; /* a ring, in file 0xEE */

/* rumbles the cockpit's controller (tlb_0325_4) */
#define bfriRumble (*bfriRumble)

void bfriItexGlowYellow(f32 x, f32 y, f32 z, f32 size);

/* "plasma" (its prints call it the second flash, "ぴか2"): file 0x120's spark at the place
 * given, that grows to 40 times life size, flickers for the frames given, shakes for 100 frames
 * (two "Meffect"s below it as it ends), then spreads file 0xEE's rings to 200 and fades; it
 * scatters yellow glows behind it every 6 frames and rumbles the controller. */
s32 bfriPlasmaProc(Obj *obj, s16 msg, void *arg) {
    PlasmaW *w = (PlasmaW *) (obj + 1);
    PlasmaArg *a = arg;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "plasma";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            w->ring = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ring == NULL) {
                return -1;
            }
            w->ring2 = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ring2 == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bfriFlash3Dl, 0x120);
            bossSetPolDL(w->ring, (Gfx *) bfriPlasmaRingDl, 0xEE);
            bossSetPolDL(w->ring2, (Gfx *) bfriPlasmaRingDl, 0xEE);
            w->frame->flags |= 7;
            w->frame->scale[2] = 4.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->scale[2] = 4.0f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = a->pos[0];
            w->frame->pos[1] = a->pos[1];
            w->frame->pos[2] = a->pos[2];
            w->alpha = 1;
            w->state = 0;
            w->timer = a->type;
            w->ringAlpha = 0;
            w->ring->pos[0] = w->frame->pos[0];
            w->ring->pos[1] = w->frame->pos[1] + 50.0f;
            w->ring->pos[2] = w->frame->pos[2];
            w->ring->pos[0] = w->frame->pos[0];
            w->ring->pos[1] = w->frame->pos[1];
            w->ring->pos[2] = w->frame->pos[2] - 50.0f;
            PRINTF("ぴか2\n");
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            if (w->ring != NULL) {
                frmDestroy(&w->ring->f);
            }
            if (w->ring2 != NULL) {
                frmDestroy(&w->ring2->f);
            }
            break;
        case 3:
            if (w->frame != NULL) {
                w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 0x7F, w->alpha);
            }
            if (w->ring != NULL) {
                w->ring->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 0x7F, w->ringAlpha);
            }
            if (w->ring2 != NULL) {
                w->ring2->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 0x7F, w->ringAlpha);
            }
            break;
        case 0:
            if (bfriEffectsEnd == 1) {
                return -1;
            }
            switch (w->state) {
                case 0:
                    w->alpha += 20;
                    if (w->alpha > 255) {
                        w->alpha = 255;
                    }
                    w->frame->scale[0] += 10.0f;
                    if (w->frame->scale[0] > 40.0f) {
                        w->frame->scale[0] = 40.0f;
                        w->frame->scale[1] = w->frame->scale[0];
                        w->state = 1;
                    }
                    if (gGameFrameCount % 6 == 0) {
                        w->frame->scale[1] = w->frame->scale[0];
                        (*bfriItexGlowYellow)(bfriRandSigned(20), bfriRandSigned(20), -2900.0f,
                                                  1.0f);
                    }
                    break;
                case 1:
                    w->alpha += 20;
                    if (w->alpha > 255) {
                        w->alpha = 255;
                    }
                    if (gGameFrameCount % 2 == 0) {
                        w->frame->scale[0] = 30.0f;
                    } else {
                        w->frame->scale[0] = 25.0f;
                    }
                    w->frame->scale[1] = w->frame->scale[0];
                    if (--w->timer < 0) {
                        w->frame->scale[0] = 40.0f;
                        w->state = 2;
                        w->timer = 100;
                    }
                    if (gGameFrameCount % 6 == 0) {
                        w->frame->scale[1] = w->frame->scale[0];
                        (*bfriItexGlowYellow)(bfriRandSigned(200), bfriRandSigned(200), -2900.0f,
                                                  2.0f);
                        bfriRumble(0x88888888, 6);
                    }
                    break;
                case 2:
                    w->timer--;
                    if (gGameFrameCount % 2 == 0) {
                        w->frame->scale[0] += 3.0f;
                    } else {
                        w->frame->scale[0] -= 3.0f;
                    }
                    w->frame->scale[1] = w->frame->scale[0];
                    if (w->timer < 0) {
                        w->state = 3;
                        w->timer = 100;
                        beMeffectCreate(w->frame->pos[0], w->frame->pos[1] - 100.0f, w->frame->pos[2]);
                        beMeffectCreate(w->frame->pos[0], w->frame->pos[1] - 200.0f, w->frame->pos[2]);
                        bfriRumble(0xFFFFFFFF, 60);
                    } else {
                        bfriRumble(0x88888888, 60);
                    }
                    if (gGameFrameCount % 6 == 0) {
                        w->frame->scale[1] = w->frame->scale[0];
                        (*bfriItexGlowYellow)(bfriRandSigned(300), bfriRandSigned(300), -2900.0f,
                                                  3.0f);
                        bfriRumble(0xBBBBBBBB, 6);
                    }
                    break;
                case 3:
                    w->frame->scale[0] += 10.0f;
                    w->ring->scale[0] += 10.0f;
                    w->ring->scale[1] += 10.0f;
                    w->ring2->scale[0] += 10.0f;
                    w->ring2->scale[1] += 10.0f;
                    w->ringAlpha += 10;
                    if (w->ringAlpha > 255) {
                        w->ringAlpha = 255;
                    }
                    if (w->frame->scale[0] > 200.0f) {
                        w->frame->scale[0] = 200.0f;
                        w->state = 4;
                    }
                    w->frame->scale[1] = w->frame->scale[0];
                    break;
                case 4:
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

/* A "plasma" of a type at x, y, z under the battle's task. */
void bfriPlasmaMake(f32 x, f32 y, f32 z, s32 type) {
    PlasmaArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    arg.type = type;
    tskCreate(gBattleTask, bfriPlasmaProc, gBattleTask->file, &arg);
}

/* A yellow glow (file 0x121's glow animation) at x, y, z of a size, growing by 1% a frame,
 * under the effect task. */
void bfriItexGlowYellow(f32 x, f32 y, f32 z, f32 size) {
    ItexInit init;

    init = gItexTemplate;
    init.file = 0x121;
    init.frames = gItexGlowAnim;
    init.flags = 3;
    init.env.r = init.env.g = 0xFF;
    init.env.b = 0;
    init.pos[0] = x;
    init.pos[1] = y;
    init.pos[2] = z;
    init.grow.x = init.grow.y = init.grow.z = 1.01f;
    init.scale[2] = size;
    init.scale[1] = size;
    init.scale[0] = size;
    tskCreate(gEffectTask, itex, gEffectTask->file, &init);
}
